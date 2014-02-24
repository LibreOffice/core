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

ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_files,nss,$(LIBO_LIB_FOLDER),\
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
		$(if $(filter 1050,$(MAC_OS_X_VERSION_MIN_REQUIRED)),dist/out/lib/libsqlite3.dylib) \
))
else ifeq ($(OS),WNT)
$(eval $(call gb_ExternalPackage_add_files,nss,$(LIBO_LIB_FOLDER),\
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
$(eval $(call gb_ExternalPackage_add_files,nss,$(LIBO_LIB_FOLDER),\
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
		dist/out/lib/libsqlite3.so \
))
endif

# vim: set noet sw=4 ts=4:
