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

$(eval $(call gb_Jar_Jar,form))

$(eval $(call gb_Jar_use_jars,form,\
	libreoffice \
	java_uno \
	commonwizards \
))

$(eval $(call gb_Jar_set_manifest,form,$(SRCDIR)/wizards/com/sun/star/wizards/form/MANIFEST.MF))

$(eval $(call gb_Jar_set_packageroot,form,com))

$(eval $(call gb_Jar_add_sourcefiles,form,\
	wizards/com/sun/star/wizards/form/CallFormWizard \
	wizards/com/sun/star/wizards/form/DataEntrySetter \
	wizards/com/sun/star/wizards/form/FieldLinker \
	wizards/com/sun/star/wizards/form/Finalizer \
	wizards/com/sun/star/wizards/form/FormConfiguration \
	wizards/com/sun/star/wizards/form/FormControlArranger \
	wizards/com/sun/star/wizards/form/FormDocument \
	wizards/com/sun/star/wizards/form/FormWizard \
	wizards/com/sun/star/wizards/form/StyleApplier \
	wizards/com/sun/star/wizards/form/UIControlArranger \
))

$(eval $(call gb_Jar_set_componentfile,form,wizards/com/sun/star/wizards/form/form,OOO,services))

# vim: set noet sw=4 ts=4:
