# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
$(eval $(call gb_Executable_Executable,Viewer))

$(eval $(call gb_Executable_use_api,Viewer,\
    udkapi \
    offapi \
))

$(eval $(call gb_Executable_use_externals,Viewer,\
    iconv \
    zlib \
))

$(eval $(call gb_Executable_set_include,Viewer,\
    $$(INCLUDE) \
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

# vim: set ts=4 sw=4 et:
