# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
$(eval $(call gb_Executable_Executable,LibreOffice))

$(eval $(call gb_Executable_use_api,LibreOffice,\
    udkapi \
    offapi \
))

$(eval $(call gb_Executable_use_externals,LibreOffice,\
    boost_headers \
    iconv \
    zlib \
))

$(eval $(call gb_Executable_use_system_darwin_frameworks,LibreOffice,\
    Foundation \
    CoreFoundation \
    CoreGraphics \
    CoreText \
    UIKit \
))

$(eval $(call gb_Executable_add_objcxxobjects,LibreOffice,\
    ios/experimental/LibreOffice/LibreOffice/lo \
))

$(eval $(call gb_Executable_add_objcobjects,LibreOffice,\
    ios/experimental/LibreOffice/LibreOffice/AppDelegate \
    ios/experimental/LibreOffice/LibreOffice/main \
    ios/experimental/LibreOffice/LibreOffice/View \
    ios/experimental/LibreOffice/LibreOffice/ViewController \
))

# Mark the executable as depending on all libraries so that it gets
# rebuilt if any library has been rebuilt. Avoids need for "make
# ios.clean".

# Yeah, this isn't the full list. I couldn't be bothered. Do we want to
# list everything here, too (as in gb_LinkTarget__command_dynamiclink in
# solenv/gbuild/platform/IOS_ARM_GCC.mk)? Should we have a global gb_
# variable for that list? (To be used for Android builds, too.)

$(WORKDIR)/LinkTarget/Executable/LibreOffice : \
    $(wildcard $(INSTDIR)/$(LIBO_LIB_FOLDER)/lib*.a) \
    $(wildcard $(WORKDIR)/LinkTarget/StaticLibrary/lib*.a)

# vim: set ts=4 sw=4 et:
