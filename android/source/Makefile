ifeq ($(BUILDDIR),)
gb_Side:=host
include ../../config_host.mk
endif

# The default target just builds.

all: build-gradle

DISABLE_UI=TRUE
BOOTSTRAPDIR=../Bootstrap
include $(BOOTSTRAPDIR)/Makefile.shared

native-code.cxx: $(SRCDIR)/solenv/bin/native-code.py
	$< -j -g core -g writer -g calc -g draw -g edit > $@

install:
	./gradlew $(if $(verbose),--info) $(if $(versionCode),-PcmdVersionCode=$(versionCode)) install$(if $(DISABLE_UI),StrippedUI,FullUI)Debug
	@echo
	@echo 'Run it with "make run"'
	@echo

uninstall:
	$(ANDROID_SDK_HOME)/platform-tools/adb uninstall $(ANDROID_PACKAGE_NAME)

clean:
	rm -rf assets assets_fullUI assets_strippedUI jniLibs jniLibs_debug $(OBJLOCAL)
	rm -f native-code.cxx
	rm -f liboSettings.gradle

build-gradle: liboSettings.gradle local.properties link-so
	./gradlew $(if $(verbose),--info) $(if $(versionCode),-PcmdVersionCode=$(versionCode)) assemble$(if $(DISABLE_UI),StrippedUI,FullUI)$(if $(ENABLE_RELEASE_BUILD),Release,Debug)

run:
	$(ANDROID_SDK_HOME)/platform-tools/adb shell am start -n $(ANDROID_PACKAGE_NAME)/.ui.LibreOfficeUIActivity

debugrun:
	$(SYSBASE)/../../../ndk-gdb --start
