# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Executable_Executable,genindex_data))

$(eval $(call gb_Executable_use_libraries,genindex_data,\
	sal \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Executable_use_externals,genindex_data,\
	icudata \
	icui18n \
	icuuc \
))

$(eval $(call gb_Executable_add_exception_objects,genindex_data,\
	i18npool/source/indexentry/genindex_data \
))

# vim: set noet sw=4 ts=4:
