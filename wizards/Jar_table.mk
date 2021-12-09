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

$(eval $(call gb_Jar_Jar,table))

$(eval $(call gb_Jar_use_jars,table,\
	libreoffice \
	java_uno \
	commonwizards \
))

$(eval $(call gb_Jar_set_packageroot,table,com))

$(eval $(call gb_Jar_set_manifest,table,$(SRCDIR)/wizards/com/sun/star/wizards/table/MANIFEST.MF))

$(eval $(call gb_Jar_add_sourcefiles,table,\
	wizards/com/sun/star/wizards/table/CallTableWizard \
	wizards/com/sun/star/wizards/table/CGCategory \
	wizards/com/sun/star/wizards/table/CGTable \
	wizards/com/sun/star/wizards/table/FieldDescription \
	wizards/com/sun/star/wizards/table/FieldFormatter \
	wizards/com/sun/star/wizards/table/Finalizer \
	wizards/com/sun/star/wizards/table/PrimaryKeyHandler \
	wizards/com/sun/star/wizards/table/ScenarioSelector \
	wizards/com/sun/star/wizards/table/TableWizard \
))

$(eval $(call gb_Jar_set_componentfile,table,wizards/com/sun/star/wizards/table/table,OOO,services))

# vim: set noet sw=4 ts=4:
