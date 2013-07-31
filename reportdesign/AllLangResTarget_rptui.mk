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

$(eval $(call gb_AllLangResTarget_AllLangResTarget,rptui))

$(eval $(call gb_AllLangResTarget_set_reslocation,rptui,reportdesign))

$(eval $(call gb_AllLangResTarget_add_srs,rptui,reportdesign/rptui))

$(eval $(call gb_SrsTarget_SrsTarget,reportdesign/rptui))

$(eval $(call gb_SrsTarget_use_srstargets,reportdesign/rptui,\
	svx/res \
))

$(eval $(call gb_SrsTarget_set_include,reportdesign/rptui,\
	$$(INCLUDE) \
	-I$(SRCDIR)/reportdesign/source/ui/inc \
	-I$(SRCDIR)/reportdesign/inc \
    -I$(call gb_SrsTemplateTarget_get_include_dir,) \
))


$(eval $(call gb_SrsTarget_add_files,reportdesign/rptui,\
	reportdesign/source/ui/dlg/dlgpage.src	\
	reportdesign/source/ui/dlg/PageNumber.src	\
	reportdesign/source/ui/dlg/CondFormat.src	\
	reportdesign/source/ui/dlg/Navigator.src	\
	reportdesign/source/ui/dlg/GroupsSorting.src \
	reportdesign/source/ui/inspection/inspection.src \
	reportdesign/source/ui/report/report.src \
))

# vim: set noet sw=4 ts=4:
