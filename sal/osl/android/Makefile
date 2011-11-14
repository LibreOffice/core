NDK_HOME:=$(shell type -p ndk-build)
NDK_HOME:=$(shell dirname $(NDK_HOME))

all:
	ndk-build V=1
	cp ../../unxandr.pro/bin/cppunittester libs/armeabi-v7a/libcppunittester.so
	cp ../../../solver/unxandr.pro/lib/libcppunit-1.12.so libs/armeabi-v7a
	cp ../../unxandr.pro/lib/*.so libs/armeabi-v7a
	cp $(NDK_HOME)/sources/cxx-stl/gnu-libstdc++/libs/armeabi-v7a/libgnustl_shared.so libs/armeabi-v7a
	unset JAVA_HOME && ant debug
	@echo 'Install it on the device with ant debug install'
	@echo 'Then run it with something like what "make run" does (see Makefile)'

run: all
	unset JAVA_HOME && ant debug install
#
# Note: this is of course just an example
	adb shell am start -a org.libreoffice.android -n org.libreoffice.android/.Bootstrap -e lo-main-library libcppunittester -e lo-main-cmdline "cppunittester /data/data/org.libreoffice.android/lib/libqa_sal_types.so"
