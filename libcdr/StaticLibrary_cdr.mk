# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,cdrlib))

$(eval $(call gb_StaticLibrary_use_unpacked,cdrlib,cdr))

$(eval $(call gb_StaticLibrary_use_package,cdrlib,libcdr_inc))

$(eval $(call gb_StaticLibrary_use_externals,cdrlib,\
	lcms2 \
	wpd \
	wpg \
	zlib \
))

$(eval $(call gb_StaticLibrary_set_generated_cxx_suffix,cdrlib,cpp))

$(eval $(call gb_StaticLibrary_add_generated_exception_objects,cdrlib,\
    UnpackedTarball/cdr/src/lib/CDRCollector \
    UnpackedTarball/cdr/src/lib/CDRContentCollector \
    UnpackedTarball/cdr/src/lib/CDRDocument \
    UnpackedTarball/cdr/src/lib/CDRInternalStream \
    UnpackedTarball/cdr/src/lib/CDROutputElementList \
    UnpackedTarball/cdr/src/lib/CDRParser \
    UnpackedTarball/cdr/src/lib/CDRPath \
    UnpackedTarball/cdr/src/lib/CDRStringVector \
    UnpackedTarball/cdr/src/lib/CDRStylesCollector \
    UnpackedTarball/cdr/src/lib/CDRSVGGenerator \
    UnpackedTarball/cdr/src/lib/CDRTypes \
    UnpackedTarball/cdr/src/lib/CDRZipStream \
    UnpackedTarball/cdr/src/lib/CMXDocument \
    UnpackedTarball/cdr/src/lib/CMXParser \
    UnpackedTarball/cdr/src/lib/CommonParser \
    UnpackedTarball/cdr/src/lib/libcdr_utils \
))

# vim: set noet sw=4 ts=4:
