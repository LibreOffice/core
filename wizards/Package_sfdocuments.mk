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

$(eval $(call gb_Package_Package,wizards_basicsrvsfdocuments,$(SRCDIR)/wizards/source/sfdocuments))

$(eval $(call gb_Package_add_files,wizards_basicsrvsfdocuments,$(LIBO_SHARE_FOLDER)/basic/SFDocuments,\
	SF_Base.xba \
	SF_Calc.xba \
	SF_Document.xba \
	SF_Form.xba \
	SF_FormControl.xba \
	SF_Register.xba \
	__License.xba \
	dialog.xlb \
	script.xlb \
))

# vim: set noet sw=4 ts=4:
