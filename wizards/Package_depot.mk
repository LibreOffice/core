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

$(eval $(call gb_Package_Package,wizards_basicsrvdepot,$(SRCDIR)/wizards/source/depot))

$(eval $(call gb_Package_set_outdir,wizards_basicsrvdepot,$(INSTROOT)))

$(eval $(call gb_Package_add_files,wizards_basicsrvdepot,$(LIBO_SHARE_FOLDER)/basic/Depot,\
	CommonLang.xba \
	Currency.xba \
	Depot.xba \
	dialog.xlb \
	Dialog2.xdl \
	Dialog3.xdl \
	Dialog4.xdl \
	Internet.xba \
	Lang_de.xba \
	Lang_en.xba \
	Lang_es.xba \
	Lang_fr.xba \
	Lang_it.xba \
	Lang_ja.xba \
	Lang_ko.xba \
	Lang_sv.xba \
	Lang_tw.xba \
	Lang_zh.xba \
	script.xlb \
	tools.xba \
))

# vim: set noet sw=4 ts=4:
