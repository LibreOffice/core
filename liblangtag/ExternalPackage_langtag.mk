# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,langtag,langtag))
$(eval $(call gb_ExternalPackage_use_external_project,langtag,langtag))
ifeq ($(GUI),WNT)
ifeq ($(COM),GCC)
$(eval $(call gb_ExternalPackage_add_file,langtag,lib/liblangtag-1.dll,liblangtag/.libs/liblangtag-1.dll))
$(eval $(call gb_ExternalPackage_add_file,langtag,lib/liblangtag.dll.a,liblangtag/.libs/liblangtag.dll.a))
else
$(eval $(call gb_ExternalPackage_add_file,langtag,lib/langtag.lib,liblangtag/.libs/langtag.lib))
endif
else
ifeq ($(OS),MACOSX)
$(eval $(call gb_ExternalPackage_add_file,langtag,lib/liblangtag.1.0.0.dylib,liblangtag/.libs/liblangtag.1.0.0.dylib))
else
$(eval $(call gb_ExternalPackage_add_file,langtag,lib/liblangtag.so.1.0.0,liblangtag/.libs/liblangtag.so.1.0.0))
endif
endif

# vim: set noet sw=4 ts=4:
