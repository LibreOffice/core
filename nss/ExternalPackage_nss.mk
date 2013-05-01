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
		mozilla/dist/out/lib/libcrmf.a \
		mozilla/dist/out/lib/libfreebl3.dylib \
		mozilla/dist/out/lib/libnspr4.dylib \
		mozilla/dist/out/lib/libnss3.dylib \
		mozilla/dist/out/lib/libnssckbi.dylib \
		mozilla/dist/out/lib/libnssdbm3.dylib \
		mozilla/dist/out/lib/libnssutil3.dylib \
		mozilla/dist/out/lib/libplc4.dylib \
		mozilla/dist/out/lib/libplds4.dylib \
		mozilla/dist/out/lib/libsmime3.dylib \
		mozilla/dist/out/lib/libsoftokn3.dylib \
		mozilla/dist/out/lib/libssl3.dylib \
))
$(eval $(call gb_ExternalPackage_add_files,nss,lib/sqlite,\
	        $(if $(filter 1060 1070 1080,$(MAC_OS_X_VERSION_MIN_REQUIRED)),,mozilla/dist/out/lib/libsqlite3.dylib) \
))
else ifeq ($(OS),WNT)
ifeq ($(COM),MSC)
$(eval $(call gb_ExternalPackage_add_files,nss,lib,\
		mozilla/dist/out/lib/nspr4.lib \
		mozilla/dist/out/lib/nss3.lib \
		mozilla/dist/out/lib/nssdbm3.lib \
		mozilla/dist/out/lib/nssutil3.lib \
		mozilla/dist/out/lib/plc4.lib \
		mozilla/dist/out/lib/plds4.lib \
		mozilla/dist/out/lib/smime3.lib \
		mozilla/dist/out/lib/softokn3.lib \
		mozilla/dist/out/lib/sqlite3.lib \
		mozilla/dist/out/lib/ssl3.lib \
))
else
$(eval $(call gb_ExternalPackage_add_files,nss,lib,\
		mozilla/nsprpub/out/pr/src/libnspr4.dll.a \
		mozilla/dist/out/lib/libnss3.dll.a \
		mozilla/dist/out/lib/libnssdbm3.dll.a \
		mozilla/dist/out/lib/libnssutil3.dll.a \
		mozilla/nsprpub/out/lib/libc/src/libplc4.dll.a \
		mozilla/nsprpub/out/lib/ds/libplds4.dll.a \
		mozilla/dist/out/lib/libsmime3.dll.a \
		mozilla/dist/out/lib/libsoftokn3.dll.a \
		mozilla/dist/out/lib/libsqlite3.dll.a \
		mozilla/dist/out/lib/libssl3.dll.a \
))
endif
$(eval $(call gb_ExternalPackage_add_libraries_for_install,nss,bin,\
		mozilla/dist/out/lib/freebl3.dll \
		mozilla/dist/out/lib/nspr4.dll \
		mozilla/dist/out/lib/nss3.dll \
		mozilla/dist/out/lib/nssckbi.dll \
		mozilla/dist/out/lib/nssdbm3.dll \
		mozilla/dist/out/lib/nssutil3.dll \
		mozilla/dist/out/lib/plc4.dll \
		mozilla/dist/out/lib/plds4.dll \
		mozilla/dist/out/lib/smime3.dll \
		mozilla/dist/out/lib/softokn3.dll \
		mozilla/dist/out/lib/sqlite3.dll \
		mozilla/dist/out/lib/ssl3.dll \
))
else # OS!=WNT/MACOSX
$(eval $(call gb_ExternalPackage_add_files,nss,lib,\
		mozilla/dist/out/lib/libcrmf.a \
))
$(eval $(call gb_ExternalPackage_add_libraries_for_install,nss,lib,\
		mozilla/dist/out/lib/libfreebl3.so \
		mozilla/dist/out/lib/libnspr4.so \
		mozilla/dist/out/lib/libnss3.so \
		mozilla/dist/out/lib/libnssckbi.so \
		mozilla/dist/out/lib/libnssdbm3.so \
		mozilla/dist/out/lib/libnsssysinit.so \
		mozilla/dist/out/lib/libnssutil3.so \
		mozilla/dist/out/lib/libplc4.so \
		mozilla/dist/out/lib/libplds4.so \
		mozilla/dist/out/lib/libsmime3.so \
		mozilla/dist/out/lib/libsoftokn3.so \
		mozilla/dist/out/lib/libssl3.so \
))
$(eval $(call gb_ExternalPackage_add_files,nss,lib/sqlite,\
	        mozilla/dist/out/lib/libsqlite3.so \
))
endif

# vim: set noet sw=4 ts=4:
