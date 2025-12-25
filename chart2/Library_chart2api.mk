# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,chart2api))

$(eval $(call gb_Library_set_include,chart2api,\
    $$(INCLUDE) \
))

$(eval $(call gb_Library_set_precompiled_header,chart2api,chart2/inc/pch/precompiled_chart2api))

$(eval $(call gb_Library_add_defs,chart2api,\
       -DCHART2API_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_sdk_api,chart2api))

$(eval $(call gb_Library_use_libraries,chart2api,\
))

$(eval $(call gb_Library_add_exception_objects,chart2api,\
    chart2/source/api/AbstractPivotTableDataProvider \
))


# vim: set noet sw=4 ts=4:
