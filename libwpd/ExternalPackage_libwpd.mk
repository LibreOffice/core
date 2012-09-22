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
	src/lib/libwpd.h \
	src/lib/WPDocument.h \
	src/lib/WPXBinaryData.h \
	src/lib/WPXDocumentInterface.h \
	src/lib/WPXProperty.h \
	src/lib/WPXPropertyList.h \
	src/lib/WPXString.h \
	src/lib/WPXPropertyListVector.h \
))

$(eval $(call gb_ExternalPackage_add_unpacked_files,libwpd_inc,inc/external/libwpd-stream,\
	src/lib/libwpd-stream.h \
	src/lib/WPXStream.h \
	src/lib/WPXStreamImplementation.h \
))

# vim: set noet sw=4 ts=4:
