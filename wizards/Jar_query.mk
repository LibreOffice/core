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

$(eval $(call gb_Jar_Jar,query))

$(eval $(call gb_Jar_use_jars,query,\
	libreoffice \
	java_uno \
	commonwizards \
))
$(eval $(call gb_Jar_set_manifest,query,$(SRCDIR)/wizards/com/sun/star/wizards/query/MANIFEST.MF))

$(eval $(call gb_Jar_set_packageroot,query,com))

$(eval $(call gb_Jar_add_sourcefiles,query,\
	wizards/com/sun/star/wizards/query/CallQueryWizard \
	wizards/com/sun/star/wizards/query/Finalizer \
	wizards/com/sun/star/wizards/query/QuerySummary \
	wizards/com/sun/star/wizards/query/QueryWizard \
))

$(eval $(call gb_Jar_set_componentfile,query,wizards/com/sun/star/wizards/query/query,OOO,services))

# vim: set noet sw=4 ts=4:
