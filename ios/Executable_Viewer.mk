# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
$(eval $(call gb_Executable_Executable,Viewer))

$(eval $(call gb_Executable_use_api,Viewer,\
    udkapi \
    offapi \
))

$(eval $(call gb_Executable_use_externals,Viewer,\
    boost_headers \
    iconv \
    zlib \
))

$(eval $(call gb_Executable_use_system_darwin_frameworks,Viewer,\
    Foundation \
    CoreFoundation \
    CoreGraphics \
    CoreText \
    UIKit \
))

$(eval $(call gb_Executable_add_objcxxobjects,Viewer,\
    ios/experimental/Viewer/Viewer/lo-viewer \
    ios/experimental/Viewer/Viewer/LOViewerAppDelegate \
    ios/experimental/Viewer/Viewer/main \
))

$(call gb_Executable_get_target,Viewer) : $(call gb_Postprocess_get_target,AllModulesButInstsetNative)

# vim: set ts=4 sw=4 et:
