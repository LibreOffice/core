# Build this without automake, so that the plugin is not used when building the plugin itself.

CLANG_CXXFLAGS = $(shell llvm-config --cxxflags)

plugin.so: plugin.o Makefile
	clang++ -shared plugin.o -o plugin.so

plugin.o: plugin.cpp Makefile
	clang++ -O2 -g $(CLANG_CXXFLAGS) -Wall -Werror plugin.cpp -fPIC -c -o plugin.o

run:
	clang++ -Werror -Xclang -load -Xclang ./plugin.so -Xclang -add-plugin -Xclang coplugin -c -o test/capture.o test/capture.cpp

check:
	clang++ -Werror -Xclang -verify -Xclang -load -Xclang ./plugin.so -Xclang -add-plugin -Xclang coplugin -c test/capture.cpp
	clang++ -Werror -Xclang -verify -Xclang -load -Xclang ./plugin.so -Xclang -add-plugin -Xclang coplugin -c test/refcounting.cpp

clean:
	rm -f plugin.so plugin.o
