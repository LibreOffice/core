ifeq ($(BUILDDIR),)
include ../../../config_host.mk
endif

# The default target just builds.

all: build-ant

# The package of this app
APP_PACKAGE=org.libreoffice

BOOTSTRAPDIR=../../Bootstrap
include $(BOOTSTRAPDIR)/Makefile.shared

native-code.cxx: $(SRCDIR)/solenv/bin/native-code.py
	$< \
		-g core -g writer \
		> $@

build-ant: android_version_setup copy-stuff link-so properties
	echo sdk.dir=$(ANDROID_SDK_HOME) >../../abs-lib/local.properties
# ActionBarSherlock uses the v4 support library, apparently one needs to copy
# it like this for it to be found
	mkdir -p ../../abs-lib/libs
	cp $(ANDROID_SDK_HOME)/extras/android/support/v4/android-support-v4.jar ../../abs-lib/libs
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
#
	unset JAVA_HOME && $(ANT) $(if $(VERBOSE)$(verbose),,-quiet) debug

run:
	adb shell am start -n $(APP_PACKAGE)/.ui.LibreOfficeUIActivity -e input /assets/test1.odt
