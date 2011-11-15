NDK_HOME:=$(shell type -p ndk-build)
NDK_HOME:=$(shell dirname $(NDK_HOME))

all:
	ndk-build V=1
# Copy shared libraries we need to libs/armeabi-v7a so that ant will
# include them in the .apk
	cp ../../unxandr.pro/bin/cppunittester libs/armeabi-v7a/libcppunittester.so
	cp ../../../solver/unxandr.pro/lib/libcppunit-1.12.so libs/armeabi-v7a
	cp ../../unxandr.pro/lib/*.so libs/armeabi-v7a
	cp $(NDK_HOME)/sources/cxx-stl/gnu-libstdc++/libs/armeabi-v7a/libgnustl_shared.so libs/armeabi-v7a
# Copy them to obj/local/armeabi-v7a, too, where gdb will look for
# them.  Not sure if this is useful or not; I have great problems with
# ndk-gdb.
	cp ../../unxandr.pro/bin/cppunittester obj/local/armeabi-v7a/libcppunittester.so
	cp ../../../solver/unxandr.pro/lib/libcppunit-1.12.so obj/local/armeabi-v7a
	cp ../../unxandr.pro/lib/*.so obj/local/armeabi-v7a
	cp $(NDK_HOME)/sources/cxx-stl/gnu-libstdc++/libs/armeabi-v7a/libgnustl_shared.so obj/local/armeabi-v7a
	unset JAVA_HOME && ant debug
	@echo 'Install it on the device with ant debug install'
	@echo 'Then run it with something like what "make run" does (see Makefile)'

run: all
	unset JAVA_HOME && ant debug install
#
# Note: this is of course just an example. The full path the the test
# .so needs to be supplied, unfortunately, I guess cppunittester
# checks its existance using the pathname instead of just
# osl_loadModule'ing it.
	adb shell am start -n org.libreoffice.android/.Bootstrap -e lo-main-library libcppunittester -e lo-main-cmdline "cppunittester /data/data/org.libreoffice.android/lib/libqa_sal_types.so"
