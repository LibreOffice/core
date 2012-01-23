# lo-main-library - parameter denoting which shlib to load and execute
# 'lo_main' from

include ../../../config_host.mk

# The package of this app
APP_PACKAGE=org.libreoffice.android.libo

# The name of the Bootstrap activity class
BOOTSTRAP=org.libreoffice.android.Bootstrap

# The full path the the app's directory needs to be supplied in a
# couple of places, unfortunately.

APP_DATA_PATH=/data/data/$(APP_PACKAGE)

SODEST=libs/armeabi-v7a
OBJLOCAL=obj/local/armeabi-v7a

define COPY
cp $(1) $(SODEST)$(if $(2),/$(2)) && $(STRIP) --strip-debug $(SODEST)$(if $(2),/$(2),/$(notdir $(1))) && \
cp $(1) $(OBJLOCAL)$(if $(2),/$(2))
endef

# The default target just builds.

all: build-ant

buildrcs:
# main sofficerc ini ...
	mkdir -p assets/program/
	echo "[Bootstrap]" > assets/program/lofficerc
	echo "Logo=1" >> assets/program/lofficerc
	echo "NativeProgress=1" >> assets/program/lofficerc
	echo "URE_BOOTSTRAP=file:///assets/program/fundamentalrc" >> assets/program/lofficerc
# fundamentalrc ini ...
	echo "[Bootstrap]" > assets/program/fundamentalrc
	echo "LO_LIB_DIR=file://$(APP_DATA_PATH)/lib/" >> assets/program/fundamentalrc
	echo "URE_LIB_DIR=file://$(APP_DATA_PATH)/lib/" >> assets/program/fundamentalrc # checkme - is this used to find configs ?
	echo "BRAND_BASE_DIR=file:///assets" >> assets/program/fundamentalrc
	echo 'CONFIGURATION_LAYERS=xcsxcu:$${BRAND_BASE_DIR}/share/registry module:$${BRAND_BASE_DIR}/share/registry/modules res:$${BRAND_BASE_DIR}/share/registry bundledext:$${$${BRAND_BASE_DIR}/program/unorc:BUNDLED_EXTENSIONS_USER}/registry/com.sun.star.comp.deployment.configuration.PackageRegistryBackend/configmgr.ini sharedext:$${$${BRAND_BASE_DIR}/program/unorc:SHARED_EXTENSIONS_USER}/registry/com.sun.star.comp.deployment.configuration.PackageRegistryBackend/configmgr.ini xcsxcu:$${$${BRAND_BASE_DIR}/program/unorc:UNO_USER_PACKAGES_CACHE}/registry/com.sun.star.comp.deployment.configuration.PackageRegistryBackend/registry userext:$${$${BRAND_BASE_DIR}/program/unorc:UNO_USER_PACKAGES_CACHE}/registry/com.sun.star.comp.deployment.configuration.PackageRegistryBackend/configmgr.ini user:$${$$BRAND_BASE_DIR/program/bootstraprc:UserInstallation}/user/registrymodifications.xcu' >> assets/program/fundamentalrc
	echo 'URE_BIN_DIR=file:///assets/ure/bin/dir/not-here/can-we/exec-anyway' >> assets/program/fundamentalrc
	echo 'URE_MORE_TYPES=file:///assets/ure/share/misc/types.rdb file:///assets/program/types/types.rdb' >> assets/program/fundamentalrc
	echo 'URE_MORE_SERVICES=file:///assets/ure/share/misc/services.rdb file:///assets/program/services/services.rdb <$$ORIGIN/services>*' >> assets/program/fundamentalrc
# unorc ini ...
	echo "[Bootstrap]" > assets/program/unorc
	echo "URE_INTERNAL_LIB_DIR=file://$(APP_DATA_PATH)/lib/" >> assets/program/unorc
	echo 'UNO_TYPES=file:///assets/ure/share/misc/types.rdb file:///assets/program/types/types.rdb $${URE_MORE_TYPES}' >> assets/program/unorc
	echo 'UNO_SERVICES=file:///assets/ure/share/misc/services.rdb file:///assets/program/services/services.rdb $${URE_MORE_SERVICES}' >> assets/program/unorc

copy-stuff: buildrcs
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
	for F in sc_ucalc sc_filters_test; do \
	    $(call COPY,$(WORKDIR)/LinkTarget/CppunitTest/libtest_$${F}.so); \
	done
#
# Libs and UNO components that the tests need.
#
	for F in $(strip \
		  basebmplo \
		  basegfxlo \
		  bootstrap.uno \
		  comphelpgcc3 \
		  configmgr.uno \
		  deploymentmisclo \
		  fileacc \
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
		  localedata_en \
		  localedata_es \
		  localedata_euro \
		  localedata_others \
		  mergedlo \
		  msfilterlo \
		  ooxlo \
		  reg \
		  saxlo \
		  sblo \
		  sclo \
		  scfiltlo \
		  sfxlo \
		  sotlo \
		  stocservices.uno \
		  store \
		  svllo \
		  svtlo \
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
		  ); do \
	    $(call COPY,$(OUTDIR)/lib/lib$${F}.so); \
	done
#
# Then the shared GNU C++ library
	$(call COPY,$(ANDROID_NDK_HOME)/sources/cxx-stl/gnu-libstdc++/libs/armeabi-v7a/libgnustl_shared.so)
#
# Then other "assets" that can be left in the .apk. Let the directory
# structure under assets mimic the install set - except for the fact
# that program/ is renamed to lib/ and we put the ure/lib .so's into
# the assets/lib directory too.
	mkdir -p assets/program/types assets/program/services
	mkdir -p assets/program/resource
	mkdir -p assets/ure/share/misc assets/share/registry/res
	cp $(OUTDIR)/bin/types.rdb assets/program/types/
	cp $(OUTDIR)/bin/ure/types.rdb assets/ure/share/misc/
	cp $(OUTDIR)/xml/services.rdb assets/program/services
	cp $(OUTDIR)/xml/ure/services.rdb assets/ure/share/misc/
	cp -R $(OUTDIR)/xml/*.xcd assets/share/registry/
	mv assets/share/registry/fcfg_langpack_en-US.xcd assets/share/registry/res/
# .res files
	cp $(OUTDIR)/bin/*en-US.res assets/program/resource/
# Then assets that are unpacked at run-time into the app's data directory.
	mkdir -p assets/unpack/etc/fonts
	cp fonts.conf assets/unpack/etc/fonts
#
# Then gdbserver and gdb.setup so that we can debug with ndk-gdb.
#
	cp $(ANDROID_NDK_HOME)/toolchains/arm-linux-androideabi-4.4.3/prebuilt/gdbserver $(SODEST)
	echo set solib-search-path ./obj/local/armeabi-v7a >$(SODEST)/gdb.setup


build-ant: copy-stuff
	unset JAVA_HOME && ant debug

install: copy-stuff
	adb shell rm -r $(APP_DATA_PATH)
	unset JAVA_HOME && ant debug install
	@echo
	@echo 'Run it with "make run"'
	@echo

uninstall:
	adb uninstall $(APP_PACKAGE)

run:
	echo "FONTCONFIG_FILE=$(APP_DATA_PATH)/etc/fonts/fonts.conf -env:INIFILENAME=file:///assets/program/sofficerc" > cmdline
	adb push cmdline $(APP_DATA_PATH)/cmdline
	adb shell am start -n $(APP_PACKAGE)/$(BOOTSTRAP) -e lo-main-library libmergedlo -e lo-main-indirect-cmdline "$(APP_DATA_PATH)/cmdline"
# add -e lo-strace yes # if you want that

# If you reinstall an app several times, even if you uninstall it
# between, disk space seems to leak that won't get recycled until you
# stop and start...
stop-start-cycle:
	adb shell stop && adb shell start && sleep 10

clean:
	ant clean
	rm -rf assets $(SODEST) $(OBJLOCAL)

