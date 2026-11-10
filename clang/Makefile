# Build this without automake, so that the plugin is not used when building the plugin itself.

CXX=clang++
LLVM_CONFIG=llvm-config

CLANG_CXXFLAGS = $(shell $(LLVM_CONFIG) --cxxflags)
CLANG_LDFLAGS = $(shell $(LLVM_CONFIG) --ldflags)

ifeq ($(shell uname),Darwin)
DARWIN_SDK := $(shell xcrun --show-sdk-path 2>/dev/null)
ifneq ($(DARWIN_SDK),)
DARWIN_SYSROOT_FLAG := -isysroot $(DARWIN_SDK)
CLANG_CXXFLAGS += $(DARWIN_SYSROOT_FLAG)
endif
CLANG_LDFLAGS += -undefined dynamic_lookup
endif

plugin.so: plugin.o Makefile
	$(CXX) -shared $(CLANG_LDFLAGS) $(DARWIN_SYSROOT_FLAG) plugin.o -o plugin.so

plugin.o: plugin.cpp Makefile
	$(CXX) -O2 -g $(CLANG_CXXFLAGS) -Wall -Werror plugin.cpp -fPIC -c -o plugin.o

run: | plugin.so
	$(CXX) $(DARWIN_SYSROOT_FLAG) -Werror -Xclang -load -Xclang ./plugin.so -Xclang -add-plugin -Xclang coplugin -c -o test/capture.o test/capture.cpp

check: | plugin.so
	$(CXX) $(DARWIN_SYSROOT_FLAG) -Werror -Xclang -verify -Xclang -load -Xclang ./plugin.so -Xclang -add-plugin -Xclang coplugin -c test/capture.cpp
	$(CXX) $(DARWIN_SYSROOT_FLAG) -Werror -Xclang -verify -Xclang -load -Xclang ./plugin.so -Xclang -add-plugin -Xclang coplugin -c test/refcounting.cpp

clean:
	rm -f plugin.so plugin.o
