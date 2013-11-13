# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,nss,nss))

$(eval $(call gb_ExternalPackage_use_external_project,nss,nss))

$(eval $(call gb_ExternalPackage_add_files,nss,bin,\
	config/nspr-config \
	config/nss-config \
))

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_libraries_for_install,nss,lib,\
		dist/out/lib/libfreebl3.dylib \
		dist/out/lib/libnspr4.dylib \
		dist/out/lib/libnss3.dylib \
		dist/out/lib/libnssckbi.dylib \
		dist/out/lib/libnssdbm3.dylib \
		dist/out/lib/libnssutil3.dylib \
		dist/out/lib/libplc4.dylib \
		dist/out/lib/libplds4.dylib \
		dist/out/lib/libsmime3.dylib \
		dist/out/lib/libsoftokn3.dylib \
		dist/out/lib/libssl3.dylib \
))
else ifeq ($(OS),WNT)
ifeq ($(COM),MSC)
$(eval $(call gb_ExternalPackage_add_files,nss,lib,\
		dist/out/lib/nspr4.lib \
		dist/out/lib/nss3.lib \
		dist/out/lib/nssdbm3.lib \
		dist/out/lib/nssutil3.lib \
		dist/out/lib/plc4.lib \
		dist/out/lib/plds4.lib \
		dist/out/lib/smime3.lib \
		dist/out/lib/softokn3.lib \
		dist/out/lib/sqlite3.lib \
		dist/out/lib/ssl3.lib \
))
else
$(eval $(call gb_ExternalPackage_add_files,nss,lib,\
		nsprpub/out/pr/src/libnspr4.dll.a \
		dist/out/lib/libnss3.dll.a \
		dist/out/lib/libnssdbm3.dll.a \
		dist/out/lib/libnssutil3.dll.a \
		nsprpub/out/lib/libc/src/libplc4.dll.a \
		nsprpub/out/lib/ds/libplds4.dll.a \
		dist/out/lib/libsmime3.dll.a \
		dist/out/lib/libsoftokn3.dll.a \
		dist/out/lib/libsqlite3.dll.a \
		dist/out/lib/libssl3.dll.a \
))
endif
$(eval $(call gb_ExternalPackage_add_libraries_for_install,nss,bin,\
		dist/out/lib/freebl3.dll \
		dist/out/lib/nspr4.dll \
		dist/out/lib/nss3.dll \
		dist/out/lib/nssckbi.dll \
		dist/out/lib/nssdbm3.dll \
		dist/out/lib/nssutil3.dll \
		dist/out/lib/plc4.dll \
		dist/out/lib/plds4.dll \
		dist/out/lib/smime3.dll \
		dist/out/lib/softokn3.dll \
		dist/out/lib/sqlite3.dll \
		dist/out/lib/ssl3.dll \
))
else # OS!=WNT/MACOSX
$(eval $(call gb_ExternalPackage_add_libraries_for_install,nss,lib,\
		dist/out/lib/libfreebl3.so \
		dist/out/lib/libnspr4.so \
		dist/out/lib/libnss3.so \
		dist/out/lib/libnssckbi.so \
		dist/out/lib/libnssdbm3.so \
		dist/out/lib/libnssutil3.so \
		dist/out/lib/libplc4.so \
		dist/out/lib/libplds4.so \
		dist/out/lib/libsmime3.so \
		dist/out/lib/libsoftokn3.so \
		dist/out/lib/libssl3.so \
))
$(eval $(call gb_ExternalPackage_add_files,nss,lib/sqlite,\
	    dist/out/lib/libsqlite3.so \
))
endif

# vim: set noet sw=4 ts=4:
