all: jni/LibreOfficeBootstrap-debug.apk

jni/LibreOfficeBootstrap-debug.apk : jni/*.c src/org/libreoffice/android/*.java
	ndk-build V=1
	cp ../../../solver/unxandr.pro/bin/cppunit/cppunittester libs/armeabi-v7a/libcppunittester.so
	cp ../../../solver/unxandr.pro/lib/libcppunit-1.12.so libs/armeabi-v7a
	cp ../../../solver/unxandr.pro/lib/libuno_sal.so libs/armeabi-v7a
	ant debug
	@echo Install it on the device with ant debug install
	@echo Then run it with something like:
	@echo adb shell am start -a org.libreoffice.android -n org.libreoffice.android/.Bootstrap -e lo-main-library libcppunittester

