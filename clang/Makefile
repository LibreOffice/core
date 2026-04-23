# Build this without automake, so that the plugin is not used when building the plugin itself.

CXX=clang++
LLVM_CONFIG=llvm-config

CLANG_CXXFLAGS = $(shell $(LLVM_CONFIG) --cxxflags)

plugin.so: plugin.o Makefile
	$(CXX) -shared plugin.o -o plugin.so

plugin.o: plugin.cpp Makefile
	$(CXX) -O2 -g $(CLANG_CXXFLAGS) -Wall -Werror plugin.cpp -fPIC -c -o plugin.o

run: | plugin.so
	$(CXX) -Werror -Xclang -load -Xclang ./plugin.so -Xclang -add-plugin -Xclang coplugin -c -o test/capture.o test/capture.cpp

check: | plugin.so
	$(CXX) -Werror -Xclang -verify -Xclang -load -Xclang ./plugin.so -Xclang -add-plugin -Xclang coplugin -c test/capture.cpp
	$(CXX) -Werror -Xclang -verify -Xclang -load -Xclang ./plugin.so -Xclang -add-plugin -Xclang coplugin -c test/refcounting.cpp

clean:
	rm -f plugin.so plugin.o
