#**************************************************************
#  
#  Licensed to the Apache Software Foundation (ASF) under one
#  or more contributor license agreements.  See the NOTICE file
#  distributed with this work for additional information
#  regarding copyright ownership.  The ASF licenses this file
#  to you under the Apache License, Version 2.0 (the
#  "License"); you may not use this file except in compliance
#  with the License.  You may obtain a copy of the License at
#  
#    http://www.apache.org/licenses/LICENSE-2.0
#  
#  Unless required by applicable law or agreed to in writing,
#  software distributed under the License is distributed on an
#  "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
#  KIND, either express or implied.  See the License for the
#  specific language governing permissions and limitations
#  under the License.
#  
#**************************************************************



$(eval $(call gb_Library_Library,package2))

$(eval $(call gb_Library_add_precompiled_header,package2,$(SRCDIR)/package/inc/pch/precompiled_package))

$(eval $(call gb_Library_set_componentfile,package2,package/util/package2))

$(eval $(call gb_Library_set_include,package2,\
        $$(INCLUDE) \
	-I$(SRCDIR)/package/inc \
	-I$(SRCDIR)/package/inc/pch \
))

$(eval $(call gb_Library_add_defs,package2,\
	-DPACKAGE_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_add_api,package2,\
	offapi \
	udkapi \
))

$(eval $(call gb_Library_add_linked_libs,package2,\
	comphelper \
	cppuhelper \
	cppu \
	sal \
	sax \
	ucbhelper \
	$(gb_STDLIBS) \
))

$(call gb_Library_use_external,package2,zlib)

$(eval $(call gb_Library_add_exception_objects,package2,\
	package/source/package/manifest/ManifestReader \
	package/source/package/manifest/ManifestWriter \
	package/source/package/manifest/ManifestImport \
	package/source/package/manifest/ManifestExport \
	package/source/package/manifest/UnoRegister \
	package/source/package/zipapi/CRC32 \
	package/source/package/zipapi/ByteChucker \
	package/source/package/zipapi/ByteGrabber \
	package/source/package/zipapi/blowfishcontext \
	package/source/package/zipapi/Inflater \
	package/source/package/zipapi/Deflater \
	package/source/package/zipapi/sha1context \
	package/source/package/zipapi/ZipEnumeration \
	package/source/package/zipapi/ZipFile \
	package/source/package/zipapi/ZipOutputStream \
	package/source/package/zipapi/XUnbufferedStream \
	package/source/package/zippackage/ZipPackage \
	package/source/package/zippackage/ZipPackageBuffer \
	package/source/package/zippackage/ZipPackageEntry \
	package/source/package/zippackage/ZipPackageFolder \
	package/source/package/zippackage/ZipPackageFolderEnumeration \
	package/source/package/zippackage/ZipPackageSink \
	package/source/package/zippackage/ZipPackageStream \
	package/source/package/zippackage/wrapstreamforshare \
	package/source/package/zippackage/zipfileaccess \
))

# vim: set noet sw=4 ts=4:
