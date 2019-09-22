# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_StaticLibrary_StaticLibrary,precompiled_system))

$(eval $(call gb_StaticLibrary_set_precompiled_header,precompiled_system,pch/inc/pch/precompiled_system))

$(eval $(call gb_StaticLibrary_use_external,precompiled_system,boost_headers))

$(eval $(call gb_StaticLibrary_add_exception_objects,precompiled_system,\
	pch/system_empty \
))

# vim: set noet sw=4 ts=4:
