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

$(eval $(call gb_Module_Module,wizards))

$(eval $(call gb_Module_add_targets,wizards,\
	CustomTarget_share \
	CustomTarget_wizards \
	Package_access2base \
	Package_depot \
	Package_euro \
	Package_form \
	Package_gimmicks \
	Package_import \
	Package_share \
	Package_standard \
	Package_template \
	Package_tools \
	Package_tutorials \
	Package_usr \
	Package_wizards \
	Package_wizards_properties \
	Pyuno_fax \
	Pyuno_letter \
	Pyuno_agenda \
	Pyuno_commonwizards \
))

$(eval $(call gb_Module_add_l10n_targets,wizards,\
	AllLangMoTarget_wiz \
))

ifeq ($(ENABLE_JAVA),TRUE)
$(eval $(call gb_Module_add_targets,wizards,\
	Jar_commonwizards \
	Jar_form \
	Jar_query \
	Jar_report \
	Jar_reportbuilder \
	Jar_table \
))
endif

# vim: set noet sw=4 ts=4:
