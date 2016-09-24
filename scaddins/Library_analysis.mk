# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
# This file incorporates work covered by the following license notice:
#
#   Licensed to the Apache Software Foundation (ASF) under one or more
#   contributor license agreements. See the NOTICE file distributed
#   with this work for additional information regarding copyright
#   ownership. The ASF licenses this file to you under the Apache
#   License, Version 2.0 (the "License"); you may not use this file
#   except in compliance with the License. You may obtain a copy of
#   the License at http://www.apache.org/licenses/LICENSE-2.0 .
#

$(eval $(call gb_Library_Library,analysis))

$(eval $(call gb_Library_set_componentfile,analysis,scaddins/source/analysis/analysis))

$(eval $(call gb_Library_use_external,analysis,boost_headers))

$(eval $(call gb_Library_use_internal_comprehensive_api,analysis,\
	offapi \
	scaddins \
	udkapi \
))

$(eval $(call gb_Library_use_libraries,analysis,\
	comphelper \
	cppu \
	cppuhelper \
	sal \
	tl \
	i18nlangtag \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,analysis,\
	scaddins/source/analysis/analysis \
	scaddins/source/analysis/analysishelper \
	scaddins/source/analysis/bessel \
	scaddins/source/analysis/financial \
))

# Runtime dependency for unit-tests
$(eval $(call gb_Library_use_restarget,analysis,analysis))

# vim: set noet sw=4 ts=4:
