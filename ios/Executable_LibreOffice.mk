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

$(call gb_LinkTarget_get_target,Executable/LibreOffice) : $(call gb_Postprocess_get_target,AllModulesButInstsetNative)

# Mark the executable as depending on all libraries so that it gets
# rebuilt if any library has been rebuilt. Avoids need for "make
# ios.clean".

$(call gb_LinkTarget_get_target,Executable/LibreOffice) : $(wildcard $(OUTDIR)/lib/lib*.a)

# vim: set ts=4 sw=4 et:
