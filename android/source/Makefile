ifeq ($(BUILDDIR),)
gb_Side:=host
include ../../config_host.mk
endif

# The default target just builds.

all: build-ant

# The package of this app
# The setting from configure (ANDROID_PACKAGE_NAME) is applied in later stages.
APP_PACKAGE=org.libreoffice

DISABLE_UI=TRUE
BOOTSTRAPDIR=../Bootstrap
include $(BOOTSTRAPDIR)/Makefile.shared

native-code.cxx: $(SRCDIR)/solenv/bin/native-code.py
	$< -j -g core -g writer -g calc -g draw -g edit > $@

build-ant: android_version_setup copy-stuff prepare-appcompat link-so properties
#
# Copy jar files we need
#
	for F in java_uno \
		 juh \
		 jurt \
		 ridl \
		 unoloader; do \
	    $(call COPYJAR,$(INSTDIR)/$(LIBO_URE_SHARE_JAVA_FOLDER)/$${F}.jar); \
	done
	for F in unoil; do \
	    $(call COPYJAR,$(INSTDIR)/$(LIBO_SHARE_JAVA_FOLDER)/$${F}.jar); \
	done
	#ownCloud lib dependency
	$(call COPYJAR,$(WORKDIR)/UnpackedTarball/owncloud_android_lib/bin/owncloud-android-library.jar)
#
	unset JAVA_HOME && $(ANT) $(if $(verbose),,-quiet) $(if $(ENABLE_RELEASE_BUILD),release,debug)

run:
	adb shell am start -n $(APP_PACKAGE)/.ui.LibreOfficeUIActivity

debugrun:
	$(SYSBASE)/../../../ndk-gdb --start
