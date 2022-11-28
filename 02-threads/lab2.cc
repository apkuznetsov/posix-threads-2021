#include <cstdlib>
#include <iostream>
#include <string.h>
#include <semaphore.h>
#include <pthread.h>

// ������� ��� ������������ M � T1
sem_t SEMAPHORE;
// �������� ���������� � ������ ��� ������������ T1 � T2
pthread_mutex_t COND_MUTEX;
pthread_cond_t COND_VAR;
bool HAS_T1_FINISHED = false;
// ������ ���������� ������� � �����.
int TEXT_INDEX_FOR_SYMBOL = 0;

// "����������" ������� � ����� ������
void append_symbol_to_text(char* text, char symbol) {
	// ��� ������ ��������
	for (int i = 0; i < 1000; i++)
		;
	// ������ �� ������ ������, ����� ������������� ���������
	*(text + TEXT_INDEX_FOR_SYMBOL) = symbol;
}

void* t1_func(void* args) {
	char* text = (char*) args;
	const char T1_TEXT[] = "Text1, ";

	// T1 ���, ����� M �������� �������, ����� ������� ���� ������
	std::cout << "T1 waiting M" << std::endl;
	sem_wait(&SEMAPHORE);

	// ������ ������� ��� ������������ � T2
	pthread_mutex_lock(&COND_MUTEX);

	std::cout << "T1 writing" << std::endl;
	for (unsigned int i = 0; i < strlen(T1_TEXT); i++) {
		append_symbol_to_text(text, T1_TEXT[i]);
		TEXT_INDEX_FOR_SYMBOL++;
	}
	// ������� ���� ��������� ���� ����� ��������.
	// ��������� ������� �������� �� 1 ��� ������������ ���� ���� � ����� M
	sem_post(&SEMAPHORE);

	// ������������ ������� ��� T2
	HAS_T1_FINISHED = true;
	// �������� ����������� �� �������� ����������
	pthread_cond_signal(&COND_VAR);
	pthread_mutex_unlock(&COND_MUTEX);

	// ���������� � ����� ������ F1
	std::cout << "T1 ending" << std::endl;
	return EXIT_SUCCESS;
}

void* t2_func(void* args) {
	char* text = (char*) args;
	const char T2_TEXT[] = "Text2.\n\0";

	// T2 ���, ����� T1 ��������� ������
	std::cout << "T2 waiting T1" << std::endl;
	pthread_mutex_lock(&COND_MUTEX);
	// �����, ���� T1 ���� �� �������� ������
	while (!HAS_T1_FINISHED) {
		pthread_cond_wait(&COND_VAR, &COND_MUTEX);
	}

	std::cout << "T2 writing" << std::endl;
	for (unsigned int i = 0; i < strlen(T2_TEXT) + 1; i++) {
		append_symbol_to_text(text, T2_TEXT[i]);
		TEXT_INDEX_FOR_SYMBOL++;
	}
	// �������� ����������� �� �������� ����������
	pthread_cond_signal(&COND_VAR);
	pthread_mutex_unlock(&COND_MUTEX);

	std::cout << "T2 ending" << std::endl;
	return EXIT_SUCCESS;
}

int main(int argc, char *argv[]) {
	// "�����" ������ ��� �����.
	// � ����� ��������� ������ ��������� "Text0, Text1, Text2.\n".
	// 22 ��������, ������ ��� ������ � ��������� ����� ������ '\0'.
	char text[100];

	// �������� ��������.
	// 0 � ������� �� ����������� ������ ������ ���������,
	// 0 � �������� ��������.
	sem_init(&SEMAPHORE, 0, 0);
	// �������� ������� � �������� ����������
	pthread_mutex_init(&COND_MUTEX, NULL);
	pthread_cond_init(&COND_VAR, NULL);

	// �������� � ������ ���� T1
	pthread_t t2_thread;
	pthread_create(&t2_thread, NULL, (void*(*)(void*)) t2_func, (void*) text);
	pthread_t t1_thread;
	pthread_create(&t1_thread, // ���������� ����
			NULL, // ���������� ������
			(void*(*)(void*)) t1_func, // ������� ����
			(void*) text); // ��������� ������� ����
	// �������� � ������ ���� T2


	// �������, ������� ������ ������� � text ������� ���� main
	const char M_TEXT[] = "Text0, ";
	// ������ �������� � ����� ����� M.
	// ���� �� ���-�� �������� � M_TEXT ("Text0, ")
	std::cout << "M writing" << std::endl;
	for (unsigned int i = 0; i < strlen(M_TEXT); i++) {
		append_symbol_to_text(text, M_TEXT[i]);
		// � ����. ��� ������� � ����. �������
		TEXT_INDEX_FOR_SYMBOL++;
	}
	// ������� ���� ��������� ���� ����� ��������,
	// ��������� ������� �������� �� 1
	sem_post(&SEMAPHORE);

	// M ���, ����� T1 ������ ���� �������
	std::cout << "M waiting T1" << std::endl;
	sem_wait(&SEMAPHORE);
	// ������� �������
	sem_destroy(&SEMAPHORE);

	// � ���, ����� T2 ������ �������,
	// ������������ M � T2
	void *value_ret;
	std::cout << "M waiting T2" << std::endl;
	pthread_join(t2_thread, &value_ret);

	// ����� ���������� ������������� ������ ��� �����
	std::cout << text << std::endl;
	// ���������� � ����� ������ ���� M
	std::cout << "M ending" << std::endl;
	return EXIT_SUCCESS;
}
