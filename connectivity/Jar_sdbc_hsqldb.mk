# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
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
# Major Contributor(s):
# Copyright (C) 2011 Peter Foley <pefoley2@verizon.net> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Jar_Jar,sdbc_hsqldb))

$(eval $(call gb_Jar_use_externals,sdbc_hsqldb,\
	hsqldb \
))

# FIXME: is this really necessary?
$(eval $(call gb_Jar_add_manifest_classpath,sdbc_hsqldb,\
	.. \
))

$(eval $(call gb_Jar_set_packageroot,sdbc_hsqldb,com))

$(eval $(call gb_Jar_add_packagedir,sdbc_hsqldb,org))

$(eval $(call gb_Jar_add_sourcefiles,sdbc_hsqldb,\
	connectivity/org/hsqldb/lib/FileSystemRuntimeException \
	connectivity/com/sun/star/sdbcx/comp/hsqldb/NativeInputStreamHelper \
	connectivity/com/sun/star/sdbcx/comp/hsqldb/NativeLibraries \
	connectivity/com/sun/star/sdbcx/comp/hsqldb/NativeOutputStreamHelper \
	connectivity/com/sun/star/sdbcx/comp/hsqldb/NativeStorageAccess \
	connectivity/com/sun/star/sdbcx/comp/hsqldb/StorageAccess \
	connectivity/com/sun/star/sdbcx/comp/hsqldb/StorageFileAccess \
	connectivity/com/sun/star/sdbcx/comp/hsqldb/StorageNativeInputStream \
	connectivity/com/sun/star/sdbcx/comp/hsqldb/StorageNativeOutputStream \
))

# vim: set noet sw=4 ts=4:
