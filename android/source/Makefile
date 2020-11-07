ifeq ($(BUILDDIR),)
gb_Side:=host
include ../../config_host.mk
endif

# The default target just builds.

all: build-gradle

DISABLE_UI=TRUE
BOOTSTRAPDIR=../Bootstrap
include $(BOOTSTRAPDIR)/Makefile.shared

$(WORKDIR)/android/native-code.cxx: $(SRCDIR)/solenv/bin/native-code.py
	@echo "[build PY ] $@"
	$< -j -g core -g writer -g calc -g draw -g edit > $@

install:
	if test "$$ENABLE_ANDROID_LOK" != "TRUE" ; then GRADLE_OPTS=-Dorg.gradle.project.buildDir=$(WORKDIR)/gradle-build ./gradlew -PbuildCache=$(WORKDIR)/build-cache $(if $(verbose),--info) $(if $(versionCode),-PcmdVersionCode=$(versionCode)) install$(if $(DISABLE_UI),StrippedUI,FullUI)$(if $(ENABLE_ANDROID_EDITING),Editing)Debug ; fi
	@if test "$$ENABLE_ANDROID_LOK" != "TRUE" ; then echo ; echo 'Run it with "make run"' ; echo ; fi

uninstall:
	$(ANDROID_SDK_HOME)/platform-tools/adb uninstall $(ANDROID_PACKAGE_NAME)

clean:
	rm -rf $(WORKDIR)/android/assets $(WORKDIR)/android/assets_fullUI $(WORKDIR)/android/assets_strippedUI $(WORKDIR)/android/jniLibs $(WORKDIR)/android/jniLibs_debug $(OBJLOCAL)
	rm -f $(WORKDIR)/android/native-code.cxx
	rm -f $(WORKDIR)/android/liboSettings.gradle

build-gradle: $(WORKDIR)/android/liboSettings.gradle $(WORKDIR)/android/local.properties $(WORKDIR)/android/link-so
ifeq ($(ENABLE_JAVA),TRUE)
	if test "$$ENABLE_ANDROID_LOK" != "TRUE" ; then GRADLE_OPTS=-Dorg.gradle.project.buildDir=$(WORKDIR)/gradle-build ./gradlew -PliboWorkdir=$(WORKDIR) -PbuildCache=$(WORKDIR)/build-cache $(if $(verbose),--info) $(if $(versionCode),-PcmdVersionCode=$(versionCode)) assemble$(if $(DISABLE_UI),StrippedUI,FullUI)$(if $(ENABLE_ANDROID_EDITING),Editing)$(if $(ENABLE_RELEASE_BUILD),Release,Debug) ; fi
endif

run:
	if test "$$ENABLE_ANDROID_LOK" != "TRUE" ; then $(ANDROID_SDK_HOME)/platform-tools/adb shell am start -n $(ANDROID_PACKAGE_NAME)/org.libreoffice.ui.LibreOfficeUIActivity ; fi

debugrun:
	@echo "please debug with lldb from within Android Studio, or setup ndk-gdb manually (see android/README for details)"
