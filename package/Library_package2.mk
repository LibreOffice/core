# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,package2))

$(eval $(call gb_Library_set_componentfile,package2,package/util/package2,services))

$(eval $(call gb_Library_set_include,package2,\
	$$(INCLUDE) \
	-I$(SRCDIR)/package/inc \
))

$(eval $(call gb_Library_use_sdk_api,package2))

$(eval $(call gb_Library_add_defs,package2,\
	-DDLLIMPLEMENTATION_PACKAGE \
))

$(eval $(call gb_Library_set_precompiled_header,package2,package/inc/pch/precompiled_package2))

$(eval $(call gb_Library_use_libraries,package2,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sax \
	salhelper \
	ucbhelper \
	tl \
	utl \
))

$(eval $(call gb_Library_use_externals,package2,\
	boost_headers \
	zlib \
))

$(eval $(call gb_Library_add_exception_objects,package2,\
	package/source/manifest/ManifestExport \
	package/source/manifest/ManifestImport \
	package/source/manifest/ManifestReader \
	package/source/manifest/ManifestWriter \
	package/source/zipapi/blowfishcontext \
	package/source/zipapi/ByteChucker \
	package/source/zipapi/ByteGrabber \
	package/source/zipapi/CRC32 \
	package/source/zipapi/Deflater \
	package/source/zipapi/Inflater \
	package/source/zipapi/sha1context \
	package/source/zipapi/ThreadedDeflater \
	package/source/zipapi/XBufferedThreadedStream \
	package/source/zipapi/XUnbufferedStream \
	package/source/zipapi/ZipEnumeration \
	package/source/zipapi/ZipFile \
	package/source/zipapi/ZipOutputEntry \
	package/source/zipapi/ZipOutputStream \
	package/source/zippackage/wrapstreamforshare \
	package/source/zippackage/zipfileaccess \
	package/source/zippackage/ZipPackageBuffer \
	package/source/zippackage/ZipPackage \
	package/source/zippackage/ZipPackageEntry \
	package/source/zippackage/ZipPackageFolder \
	package/source/zippackage/ZipPackageFolderEnumeration \
	package/source/zippackage/ZipPackageSink \
	package/source/zippackage/ZipPackageStream \
))

ifneq ($(SYSTEM_ZLIB),)
$(eval $(call gb_Library_add_defs,package2,\
	-DSYSTEM_ZLIB \
))
endif

# vim: set noet sw=4 ts=4:
