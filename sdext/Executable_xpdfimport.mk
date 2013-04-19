# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,xpdfimport))

$(eval $(call gb_Executable_use_packages,xpdfimport,\
))

$(eval $(call gb_Executable_use_externals,xpdfimport,\
    boost_headers \
    poppler \
    zlib \
))

$(eval $(call gb_Executable_add_exception_objects,xpdfimport,\
    sdext/source/pdfimport/xpdfwrapper/pdfioutdev_gpl \
    sdext/source/pdfimport/xpdfwrapper/pnghelper \
    sdext/source/pdfimport/xpdfwrapper/wrapper_gpl \
))

# vim:set noet sw=4 ts=4:
