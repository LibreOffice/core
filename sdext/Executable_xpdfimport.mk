# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,xpdfimport))

$(eval $(call gb_Executable_use_externals,xpdfimport,\
    boost_headers \
    expat \
    freetype \
    libpng \
    poppler \
    $(if $(filter-out WNT MACOSX,$(OS)),fontconfig) \
    zlib \
))

$(eval $(call gb_Executable_add_exception_objects,xpdfimport,\
    sdext/source/pdfimport/xpdfwrapper/pdfioutdev_gpl \
    sdext/source/pdfimport/xpdfwrapper/pnghelper \
    sdext/source/pdfimport/xpdfwrapper/wrapper_gpl \
))

$(eval $(call gb_Executable_use_system_win32_libs,xpdfimport,\
	shell32 \
))

$(eval $(call gb_Executable_add_default_nativeres,xpdfimport))

ifneq ($(SYSTEM_POPPLER),)
# Using system poppler
$(eval $(call gb_Executable_add_defs,xpdfimport,\
    -DSYSTEM_POPPLER \
))
endif

# vim:set noet sw=4 ts=4:
