# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#*************************************************************************
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#*************************************************************************

$(eval $(call sd_export_test,-ooxml4))

$(call gb_CppunitTest_get_target,sd_export_tests-ooxml4): \
    $(call gb_Package_get_target,postprocess_images)

# vim: set noet sw=4 ts=4:
