ifeq ($(BUILDDIR),)
include ../../../config_host.mk
endif

# The default target just builds.

all: build-ant

# The package of this app
APP_PACKAGE=org.libreoffice

DISABLE_UI=TRUE
BOOTSTRAPDIR=../../Bootstrap
include $(BOOTSTRAPDIR)/Makefile.shared

native-code.cxx: $(SRCDIR)/solenv/bin/native-code.py
	$< -j -g core -g writer -g calc -g draw -g edit > $@

build-ant: android_version_setup copy-stuff link-so properties
#
# Copy jar files we need
#
	cp $(ANDROID_SDK_HOME)/extras/android/support/v4/android-support-v4.jar libs

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
#
	unset JAVA_HOME && $(ANT) $(if $(VERBOSE)$(verbose),,-quiet) debug

run:
	adb shell am start -n $(APP_PACKAGE)/.ui.LibreOfficeUIActivity -e input /assets/test1.odt
