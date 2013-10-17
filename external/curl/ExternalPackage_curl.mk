# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalPackage_ExternalPackage,curl,curl))

$(eval $(call gb_ExternalPackage_use_external_project,curl,curl))

ifneq ($(OS),WNT)

ifeq ($(COM),GCC)
$(eval $(call gb_ExternalPackage_add_file,curl,lib/pkgconfig/libcurl.pc,libcurl.pc))
endif

ifeq ($(DISABLE_DYNLOADING),TRUE)
$(eval $(call gb_ExternalPackage_add_file,curl,lib/libcurl.a,lib/.libs/libcurl.a))
else ifeq ($(OS),ANDROID)
$(eval $(call gb_ExternalPackage_add_file,curl,lib/libcurl.so,lib/.libs/libcurl.so))
else ifeq ($(OS),AIX)
$(eval $(call gb_ExternalPackage_add_library_for_install,curl,lib/libcurl.so,lib/.libs/libcurl.so.4))
else
# TODO: Do we really want the unversioned library in the installation?
# And if that's true, why do we need to deliver the versioned one at all?
$(eval $(call gb_ExternalPackage_add_library_for_install,curl,lib/libcurl.so,lib/.libs/libcurl.so.4.2.0))
$(eval $(call gb_ExternalPackage_add_file,curl,lib/libcurl.so.4,lib/.libs/libcurl.so.4.2.0))
endif

else ifeq ($(OS)$(COM),WNTGCC)

$(eval $(call gb_ExternalPackage_add_file,curl,lib/libcurl.a,lib/.libs/libcurl.a))
$(eval $(call gb_ExternalPackage_add_library_for_install,curl,bin/libcurl.dll,lib/.libs/libcurl.dll))

else ifeq ($(COM),MSC)

$(eval $(call gb_ExternalPackage_add_file,curl,lib/libcurl.lib,lib/libcurl.lib))
$(eval $(call gb_ExternalPackage_add_library_for_install,curl,bin/libcurl.dll,lib/libcurl.dll))

endif

# vim: set noet sw=4 ts=4:
