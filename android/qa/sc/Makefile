# The package of this app
APP_PACKAGE=org.libreoffice.android.qa.sc

# The name of the Bootstrap activity class
BOOTSTRAP=org.libreoffice.android.Bootstrap

# The full path the the app's directory needs to be supplied in a
# couple of places, unfortunately.

APP_DATA_PATH=/data/data/$(APP_PACKAGE)

NDK_HOME:=$(shell type -p ndk-build)
NDK_HOME:=$(shell dirname $(NDK_HOME))

SODEST=libs/armeabi-v7a
OBJLOCAL=obj/local/armeabi-v7a

define COPY
cp $(1) $(SODEST)$(if $(2),/$(2)) && \
arm-linux-androideabi-strip --strip-debug $(SODEST)$(if $(2),/$(2),/$(notdir $(1))) && \
cp $(1) $(OBJLOCAL)$(if $(2),/$(2))
endef

# The default target just builds.

all: build-ant

copy-stuff:
#
# Copy shared libraries we need to libs/armeabi-v7a so that ant will
# include them in the .apk.
#
# Copy them to obj/local/armeabi-v7a, too, where gdb will look for
# them.
#
	mkdir -p $(SODEST) $(OBJLOCAL)
#
# First the most important one, liblo-bootstrap.so
	$(call COPY,$(OUTDIR)/lib/liblo-bootstrap.so)
#
# Then the cppunittester "program". As all "program"
# files we build for Android actually is a shared object.
	$(call COPY,$(OUTDIR)/bin/cppunit/cppunittester,libcppunittester.so)
#
# The cppunit library
#
	$(call COPY,$(OUTDIR)/lib/libcppunit-1.12.so)
#
# The tests we run here
#
	for F in sc_ucalc; do \
	    $(call COPY,$(WORKDIR)/LinkTarget/CppunitTest/libtest_$${F}.so); \
	done
#
# Libs and UNO components that the tests need.
#
	for F in $(strip \
		  avmedialo \
		  basebmplo \
		  basegfxlo \
		  bootstrap.uno \
		  canvastoolslo \
		  comphelpgcc3 \
		  configmgr.uno \
		  cppcanvaslo \
		  drawinglayerlo \
		  editenglo \
		  fontconfig \
		  forlo \
		  foruilo \
		  freetype \
		  fwelo \
		  fwilo \
		  fwklo \
		  gcc3_uno \
		  i18nisolang1gcc3 \
		  i18npaperlo \
		  i18npool.uno \
		  i18nutilgcc3 \
		  icudatalo \
		  icui18nlo \
		  iculelo \
		  icuuclo \
		  jvmaccessgcc3 \
		  jvmfwk \
		  lnglo \
		  localedata_en \
		  localedata_others \
		  msfilterlo \
		  reg \
		  saxlo \
		  sblo \
		  sfxlo \
		  sotlo \
		  store \
		  svllo \
		  svtlo \
		  svxcorelo \
		  svxlo \
		  test \
		  tklo \
		  tllo \
		  ucb1 \
		  ucbhelper4gcc3 \
		  ucpfile1 \
		  uno_cppu \
		  uno_cppuhelpergcc3 \
		  uno_sal \
		  uno_salhelpergcc3 \
		  unoexceptionprotector \
		  unotest \
		  unoxmllo \
		  utllo \
		  vbahelperlo \
		  vcllo \
		  vclplug_svplo \
		  xcrlo \
		  xml2 \
		  xmlreader \
		  xolo \
		  ); do \
	    $(call COPY,$(OUTDIR)/lib/lib$${F}.so); \
	done
#
# Then the shared GNU C++ library
	$(call COPY,$(NDK_HOME)/sources/cxx-stl/gnu-libstdc++/libs/armeabi-v7a/libgnustl_shared.so)
#
# Then other "assets". Let the directory structure under assets mimic
# that under solver for now.
	mkdir -p assets/bin assets/lib assets/xml/ure
	cp $(OUTDIR)/bin/udkapi.rdb assets/bin
	cp $(OUTDIR)/bin/types.rdb assets/bin
	cp $(OUTDIR)/xml/ure/services.rdb assets/xml/ure
	for F in framework/util/fwk i18npool/util/i18npool sfx2/util/sfx unoxml/source/service/unoxml configmgr/source/configmgr; do \
		mkdir -p assets/ComponentTarget/`dirname $$F`; \
	        cp $(WORKDIR)/ComponentTarget/$$F.component assets/ComponentTarget/`dirname $$F`; \
	done
	for F in ucb1 ucpfile1; do \
		cp $(OUTDIR)/xml/$$F.component assets/xml; \
	done
	cp -R $(OUTDIR)/xml/registry assets/xml
	for F in uno_services uno_types; do \
		sed -e 's!uri="./!uri="$(APP_DATA_PATH)/lib/!g' <$(SRC_ROOT)/testtools/$(INPATH)/lib/$${F}.rdb >assets/lib/$${F}.rdb; \
	done
#
# Then gdbserver and gdb.setup so that we can debug with ndk-gdb.
#
	cp $(NDK_HOME)/toolchains/arm-linux-androideabi-4.4.3/prebuilt/gdbserver $(SODEST)
	echo set solib-search-path ./obj/local/armeabi-v7a >$(SODEST)/gdb.setup


build-ant: copy-stuff
	unset JAVA_HOME && ant debug

install: copy-stuff
	unset JAVA_HOME && ant debug install
	@echo
	@echo 'Run it with something like what "make run" does (see Makefile)'
	@echo

uninstall:
	adb uninstall $(APP_PACKAGE)

run:
	echo "$(APP_DATA_PATH)/lib/libtest_sc_ucalc.so --headless --protector libunoexceptionprotector.so unoexceptionprotector '-env:CONFIGURATION_LAYERS=xcsxcu:file:///assets/xml/registry' '-env:UNO_TYPES=file:///assets/bin/udkapi.rdb file:///assets/bin/types.rdb'  '-env:UNO_SERVICES=file:///assets/xml/ure/services.rdb  file:///assets/ComponentTarget/framework/util/fwk.component file:///assets/ComponentTarget/i18npool/util/i18npool.component file:///assets/ComponentTarget/sfx2/util/sfx.component file:///assets/ComponentTarget/unoxml/source/service/unoxml.component file:///assets/ComponentTarget/configmgr/source/configmgr.component file:///assets/xml/ucb1.component file:///assets/xml/ucpfile1.component' -env:URE_INTERNAL_LIB_DIR=file://$(APP_DATA_PATH)/lib -env:LO_LIB_DIR=file://$(APP_DATA_PATH)/lib" >cmdline
	adb push cmdline $(APP_DATA_PATH)/cmdline
	adb shell am start -n $(APP_PACKAGE)/$(BOOTSTRAP) -e lo-main-library libcppunittester -e lo-main-indirect-cmdline "$(APP_DATA_PATH)/cmdline" 

clean:
	ant clean
	rm -rf assets $(SODEST) $(OBJLOCAL)

