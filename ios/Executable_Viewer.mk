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

# The executables built for iOS link to all LO libs statically. The
# link command just uses the wildcard function to list all of them.
# Instead of tediously here declare dependencies on the transitive
# closure of those from which some object ends up being linked in, we
# list a few libraries that are high in the dependency forest to make
# it likely that all necessary libraries are built before this
# executable is.
$(eval $(call gb_Executable_use_libraries,Viewer,\
	msword \
	scfilt \
	sdfilt \
	swui \
))

# vim: set ts=4 sw=4 et:
