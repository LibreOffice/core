# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# Major Contributor(s):
# Copyright (C) 2012 Radu Ioan <ioan.radu.g@gmail.com> (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.

$(eval $(call gb_CppunitTest_CppunitTest,sal_rtl_uuid))

$(eval $(call gb_CppunitTest_add_exception_objects,sal_rtl_uuid,\
    sal/qa/rtl/uuid/rtl_Uuid \
))

$(eval $(call gb_CppunitTest_use_libraries,sal_rtl_uuid,\
    sal \
	$(gb_UWINAPI) \
    $(gb_STDLIBS) \
))

# vim: set noet sw=4 ts=4:
