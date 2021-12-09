# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# Rdb_saxparser duplicates some of the Rdb_services content, as needed during the build in
# CustomTarget_i18npool/localedata:

$(eval $(call gb_Rdb_Rdb,saxparser))

$(eval $(call gb_Rdb_add_components,saxparser,\
	sax/source/expatwrap/expwrap \
))

# vim:set noet sw=4 ts=4:
