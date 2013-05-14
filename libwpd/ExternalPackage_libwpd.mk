# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libwpd,libwpd))

$(eval $(call gb_ExternalPackage_add_unpacked_files,libwpd,inc/external/libwpd,\
	inc/libwpd/libwpd.h \
	inc/libwpd/WPDocument.h \
	inc/libwpd/WPXBinaryData.h \
	inc/libwpd/WPXDocumentInterface.h \
	inc/libwpd/WPXProperty.h \
	inc/libwpd/WPXPropertyList.h \
	inc/libwpd/WPXString.h \
	inc/libwpd/WPXPropertyListVector.h \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,libwpd,inc/external/libwpd-stream,\
	inc/libwpd-stream/libwpd-stream.h \
	inc/libwpd-stream/WPXStream.h \
	inc/libwpd-stream/WPXStreamImplementation.h \
))

$(eval $(call gb_ExternalPackage_use_external_project,libwpd,libwpd))

ifeq ($(OS)$(COM),WNTMSC)
$(eval $(call gb_ExternalPackage_add_file,libwpd,lib/wpd-0.9.lib,src/lib/.libs/libwpd-0.9.lib))
else
$(eval $(call gb_ExternalPackage_add_file,libwpd,lib/libwpd-0.9.a,src/lib/.libs/libwpd-0.9.a))
endif

# vim: set noet sw=4 ts=4:
