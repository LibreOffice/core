# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       Matúš Kukan <matus.kukan@gmail.com>
# Portions created by the Initial Developer are Copyright (C) 2011 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Library_Library,package2))

$(eval $(call gb_Library_use_package,package2,package_inc))

$(eval $(call gb_Library_set_componentfile,package2,package/util/package2))

$(eval $(call gb_Library_set_include,package2,\
	$$(INCLUDE) \
	-I$(SRCDIR)/package/inc \
))

$(eval $(call gb_Library_use_sdk_api,package2))

$(eval $(call gb_Library_add_defs,package2,\
	-DDLLIMPLEMENTATION_PACKAGE \
))

$(eval $(call gb_Library_use_libraries,package2,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	sax \
	ucbhelper \
	$(gb_UWINAPI) \
	$(gb_STDLIBS) \
))

$(eval $(call gb_Library_use_externals,package2,\
	zlib \
))

$(eval $(call gb_Library_add_exception_objects,package2,\
	package/source/manifest/ManifestExport \
	package/source/manifest/ManifestImport \
	package/source/manifest/ManifestReader \
	package/source/manifest/ManifestWriter \
	package/source/manifest/UnoRegister \
	package/source/zipapi/blowfishcontext \
	package/source/zipapi/ByteChucker \
	package/source/zipapi/ByteGrabber \
	package/source/zipapi/CRC32 \
	package/source/zipapi/Deflater \
	package/source/zipapi/Inflater \
	package/source/zipapi/sha1context \
	package/source/zipapi/XUnbufferedStream \
	package/source/zipapi/ZipEnumeration \
	package/source/zipapi/ZipFile \
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

ifeq ($(SYSTEM_ZLIB),YES)
$(eval $(call gb_Library_add_defs,package2,\
	-DSYSTEM_ZLIB \
))
endif

# vim: set noet sw=4 ts=4:
