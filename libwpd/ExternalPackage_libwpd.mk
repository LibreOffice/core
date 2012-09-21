# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,libwpd_inc,wpd))

$(eval $(call gb_ExternalPackage_add_unpacked_files,libwpd_inc,inc/external/libwpd,\
	libwpd.h \
	WPDocument.h \
	WPXBinaryData.h \
	WPXDocumentInterface.h \
	WPXProperty.h \
	WPXPropertyList.h \
	WPXString.h \
	WPXPropertyListVector.h \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,libwpd_inc,inc/external/libwpd-stream,\
	libwpd-stream.h \
	WPXStream.h \
	WPXStreamImplementation.h \
))

# vim: set noet sw=4 ts=4:
