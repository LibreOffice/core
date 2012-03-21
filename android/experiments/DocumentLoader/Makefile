include ../../../config_host.mk

# The package of this app
APP_PACKAGE=org.libreoffice.android.examples

SODEST=libs/armeabi-v7a
OBJLOCAL=obj/local/armeabi-v7a

define COPYSO
cp $(1) $(SODEST)$(if $(2),/$(2)) && $(STRIP) --strip-debug $(SODEST)$(if $(2),/$(2),/$(notdir $(1))) && \
cp $(1) $(OBJLOCAL)$(if $(2),/$(2))
endef

define COPYJAR
cp $(1) libs
endef

# The default target just builds.

all: build-ant

copy-stuff: 
# First always clean 
	rm -rf libs $(OBJLOCAL)
	mkdir -p $(SODEST) $(OBJLOCAL)
#
# Copy shared libraries (including UNO components) we need to
# libs/armeabi-v7a so that ant will include them in the .apk.
#
# Copy them to obj/local/armeabi-v7a, too, where gdb will look for
# them.
#
	for F in $(strip \
		   comphelpgcc3 \
		   gcc3_uno \
		   i18nisolang1gcc3 \
		   i18nutilgcc3 \
		   icudatalo \
		   icui18nlo \
		   icuuclo \
		   java_uno \
		   juh \
		   juhx \
		   jvmaccessgcc3 \
		   lo-bootstrap \
		   localedata_en \
		   localedata_others \
		   mergedlo \
		   reg \
		   store \
		   ucbhelper4gcc3 \
		   uno_cppu \
		   uno_sal \
		   uno_salhelpergcc3 \
		   uno_cppuhelpergcc3 \
		   xml2 \
		   xmlreader \
		   bootstrap.uno \
		   i18npool.uno \
		  ); do \
	    $(call COPYSO,$(OUTDIR)/lib/lib$${F}.so); \
	done
#
# Then the shared GNU C++ library
	$(call COPYSO,$(ANDROID_NDK_HOME)/sources/cxx-stl/gnu-libstdc++/libs/armeabi-v7a/libgnustl_shared.so)
#
# Then other "assets". Let the directory structure under assets mimic
# that under solver for now.
	mkdir -p assets/bin assets/lib assets/xml/ure assets/ComponentTarget/i18npool/util
	cp $(OUTDIR)/bin/udkapi.rdb assets/bin
	cp $(OUTDIR)/bin/types.rdb assets/bin
	cp $(OUTDIR)/bin/uno.ini assets
	cp $(OUTDIR)/xml/ure/services.rdb assets/xml/ure
	cp $(SRC_ROOT)/odk/examples/java/DocumentHandling/test/test1.odt assets
	cp $(WORKDIR)/ComponentTarget/i18npool/util/i18npool.component assets/ComponentTarget/i18npool/util
#
# Then gdbserver and gdb.setup so that we can debug with ndk-gdb.
#
	cp $(ANDROID_NDK_HOME)/toolchains/arm-linux-androideabi-4.4.3/prebuilt/gdbserver $(SODEST)
	echo set solib-search-path ./obj/local/armeabi-v7a >$(SODEST)/gdb.setup

build-ant: copy-stuff
#
# Copy jar files we need, and even construct one.
#
	for F in $(strip \
		   java_uno \
		   juh \
		   jurt \
		   ridl \
		   unoloader \
		  ); do \
	    $(call COPYJAR,$(OUTDIR)/bin/$${F}.jar); \
	done
#
# com.sun.star.frame.XComponentLoader is not in any jar
#
	cd libs && \
	    LD_LIBRARY_PATH=$(OUTDIR_FOR_BUILD)/lib \
	    DYLD_LIBRARY_PATH=$(OUTDIR_FOR_BUILD)/lib \
	    $(OUTDIR_FOR_BUILD)/bin/javamaker -BUCR -nD \
		$(OUTDIR)/bin/udkapi.rdb $(OUTDIR)/bin/offapi.rdb \
		-Tcom.sun.star.frame.XComponentLoader && \
	    jar cvf more.jar com
#
	unset JAVA_HOME && $(ANT) debug

install: copy-stuff
	unset JAVA_HOME && $(ANT) debug install
	@echo
	@echo 'Run it with something like what "make run" does (see Makefile)'
	@echo

uninstall:
	$(ANDROID_SDK_HOME)/platform-tools/adb uninstall $(APP_PACKAGE)

run:
	adb shell am start -n org.libreoffice.android.examples/.DocumentLoader -e input /assets/test1.odt

clean:
	$(ANT) clean
	rm -rf assets libs $(SODEST) $(OBJLOCAL)
