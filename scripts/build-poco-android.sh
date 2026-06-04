#!/bin/bash

if [ $# -eq 0 ]; then
    echo "build-poco-android.sh <abs-path-to-ndk-toplevel> <abs-path-to-build-top-dir>"
    exit;
fi

NDK_PATH=$1 # e.g. ~/Android/Sdk/ndk/android-ndk-r20b
BUILD_PATH=$2 # e.g. ~/Android/

mkdir -p $BUILD_PATH
cd $BUILD_PATH
# The official POCO C++ Libraries repository is on GitHub.
if ! test -f android-poco/.git/config; then
    git clone https://github.com/pocoproject/poco.git android-poco
fi
cd android-poco
git checkout -f
git pull -r
git checkout poco-1.12.5p2-release
patch -p1 << 'EOF'
diff --git a/Makefile b/Makefile
index 217637409..860514959 100644
--- a/Makefile
+++ b/Makefile
@@ -88,7 +88,7 @@ CppUnit-clean:

 install: libexecs
 	mkdir -p $(INSTALLDIR)/include/Poco
-	mkdir -p $(INSTALLDIR)/lib
+	mkdir -p $(INSTALLDIR)/$(OSARCH)/lib
 	mkdir -p $(INSTALLDIR)/bin
 	for comp in $(filter-out $(foreach f,$(OMIT),$f%),$(COMPONENTS)) ; do \
 		if [ -d "$(POCO_BASE)/$$comp/include" ] ; then \
@@ -102,8 +102,8 @@ ifeq ($(OSNAME), CYGWIN)
 	find $(POCO_BUILD)/lib/$(OSNAME)/$(OSARCH) -name "cygPoco*" -type f -exec cp -f  {} $(INSTALLDIR)/bin \;
 	find $(POCO_BUILD)/lib/$(OSNAME)/$(OSARCH) -name "cygPoco*" -type l -exec cp -Rf {} $(INSTALLDIR)/bin \;
 endif
-	find $(POCO_BUILD)/lib/$(OSNAME)/$(OSARCH) -name "libPoco*" -type f -exec cp -f  {} $(INSTALLDIR)/lib \;
-	find $(POCO_BUILD)/lib/$(OSNAME)/$(OSARCH) -name "libPoco*" -type l -exec cp -Rf {} $(INSTALLDIR)/lib \;
+	find $(POCO_BUILD)/lib/$(OSNAME)/$(OSARCH) -name "libPoco*" -type f -exec cp -f  {} $(INSTALLDIR)/$(OSARCH)/lib \;
+	find $(POCO_BUILD)/lib/$(OSNAME)/$(OSARCH) -name "libPoco*" -type l -exec cp -Rf {} $(INSTALLDIR)/$(OSARCH)/lib \;

 uninstall:
 	[ -d $(INSTALLDIR)/include/Poco ] && rm -rf $(INSTALLDIR)/include/Poco || echo "No installed Poco headers found";
diff --git a/build/config/Android b/build/config/Android
index 95a472787..66577ca39 100644
--- a/build/config/Android
+++ b/build/config/Android
@@ -12,29 +12,35 @@ ANDROID_ABI        ?= armeabi
 POCO_TARGET_OSNAME  = Android
 POCO_TARGET_OSARCH  = $(ANDROID_ABI)

-ifeq ($(ANDROID_ABI),armeabi)
-TOOL      = arm-linux-androideabi
-ARCHFLAGS = -mthumb
-else
 ifeq ($(ANDROID_ABI),armeabi-v7a)
 TOOL      = arm-linux-androideabi
 ARCHFLAGS = -march=armv7-a -mfloat-abi=softfp
 LINKFLAGS = -Wl,--fix-cortex-a8
 else
+ifeq ($(ANDROID_ABI),arm64-v8a)
+TOOL      = aarch64-linux-android
+ARCHFLAGS = -march=armv8-a
+LINKFLAGS = -Wl,--fix-cortex-a8
+else
 ifeq ($(ANDROID_ABI),x86)
 TOOL      = i686-linux-android
 ARCHFLAGS = -march=i686 -msse3 -mstackrealign -mfpmath=sse
 else
+ifeq ($(ANDROID_ABI),x86_64)
+TOOL      = x86_64-linux-android
+ARCHFLAGS = -march=x86-64 -msse3 -mstackrealign -mfpmath=sse
+else
 $(error Invalid ABI specified in ANDROID_ABI)
 endif
 endif
 endif
+endif

 #
 # Define Tools
 #
-CC      = $(TOOL)-gcc
-CXX     = $(TOOL)-g++
+CC      = $(TOOL)21-clang
+CXX     = $(TOOL)21-clang++
 LINK    = $(CXX)
 STRIP   = $(TOOL)-strip
 LIB     = $(TOOL)-ar -cr
@@ -56,10 +62,10 @@ SHAREDLIBLINKEXT = .so
 #
 # Compiler and Linker Flags
 #
-CFLAGS          = $(ARCHFLAGS) -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64
+CFLAGS          = $(ARCHFLAGS) -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing
 CFLAGS32        =
 CFLAGS64        =
-CXXFLAGS        = $(ARCHFLAGS) -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -finline-limit=64 -frtti -fexceptions
+CXXFLAGS        = $(ARCHFLAGS) -fpic -ffunction-sections -funwind-tables -fstack-protector -fno-strict-aliasing -frtti -fexceptions
 CXXFLAGS32      =
 CXXFLAGS64      =
 LINKFLAGS      +=
@@ -86,4 +92,4 @@ SYSFLAGS = -DPOCO_ANDROID -DPOCO_NO_FPENVIRONMENT -DPOCO_NO_WSTRING -DPOCO_NO_SH
 #
 # System Specific Libraries
 #
-SYSLIBS  = -lstdc++ -lsupc++
+SYSLIBS  = -static-libstdc++
EOF

export PATH="$PATH":$NDK_PATH/toolchains/llvm/prebuilt/linux-x86_64/bin/
export SYSLIBS=-static-libstdc++
INSTDIR=$BUILD_PATH/android-poco/install
mkdir -p $INSTDIR

./configure \
   --config=Android \
   --prefix=$INSTDIR \
   --no-samples \
   --no-tests \
   --omit=ActiveRecord,Crypto,NetSSL_OpenSSL,Zip,Data,Data/SQLite,Data/ODBC,Data/MySQL,MongoDB,PDF,CppParser,PageCompiler,JWT,Prometheus,Redis

make -sj12 ANDROID_ABI=armeabi-v7a CC=armv7a-linux-androideabi21-clang CXX=armv7a-linux-androideabi21-clang++ install
make -sj12 ANDROID_ABI=arm64-v8a LIB="llvm-ar -cr" RANLIB=llvm-ranlib STRIP=llvm-strip install
make -sj12 ANDROID_ABI=x86       install
make -sj12 ANDROID_ABI=x86_64    install

