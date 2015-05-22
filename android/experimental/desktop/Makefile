ifeq ($(BUILDDIR),)
include ../../../config_host.mk
endif

# The default target just builds.
all: build-ant

# The package of this app
APP_PACKAGE=org.libreoffice.experimental.desktop

BOOTSTRAPDIR=../../Bootstrap
include $(BOOTSTRAPDIR)/Makefile.shared

native-code.cxx: $(SRCDIR)/solenv/bin/native-code.py
	$< -j \
		-g core -g edit -g writer \
		> $@

# The name of the Bootstrap activity class
BOOTSTRAP=org.libreoffice.android.Bootstrap

# The default target just builds.

copy-stuff-desktop:
# tango artwork / icons
	mkdir -p assets/share/config
	cp -R $(INSTDIR)/$(LIBO_SHARE_FOLDER)/config/images_tango.zip assets/share/config
# shell / splash images
	mkdir -p assets/program
	cp -r $(SRC_ROOT)/icon-themes/galaxy/brand/* assets/program
# presets - becomes the users home directory, .stamp forces .zip dir creation
	for D in $(strip autocorr autotext backup basic config gallery \
	         psprint/driver template \
		 uno_packages/cache wordbook); do \
		mkdir -p assets/presets/$$D ; \
		echo "content" > assets/presets/$$D/stamp; \
	done
# lofficerc
	mkdir -p assets/program/
	echo "[Bootstrap]" > assets/program/lofficerc
	echo "Logo=1" >> assets/program/lofficerc
	echo "NativeProgress=1" >> assets/program/lofficerc
	echo "URE_BOOTSTRAP=file:///assets/program/fundamentalrc" >> assets/program/lofficerc
	echo "HOME=$(APP_DATA_PATH)/files" >> assets/program/lofficerc
	echo "TMPDIR=$(APP_DATA_PATH)/files" >> assets/program/lofficerc
# - this looks useful but breaks more than it fixes ...
#	echo "DISABLE_EXTENSION_SYNCHRONIZATION=1" >> assets/program/lofficerc

setup-jars:
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

build-ant: android_version_setup copy-stuff copy-stuff-desktop link-so properties setup-jars
	unset JAVA_HOME && $(ANT) $(if $(VERBOSE)$(verbose),,-quiet) debug

run:
	$(ANDROID_SDK_HOME)/platform-tools/adb shell am start -n $(APP_PACKAGE)/.Desktop
