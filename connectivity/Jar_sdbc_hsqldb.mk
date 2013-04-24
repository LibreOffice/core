# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

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
