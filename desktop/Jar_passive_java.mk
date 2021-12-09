# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Jar_Jar,passive_java))

$(eval $(call gb_Jar_add_sourcefiles,passive_java, \
    desktop/test/deployment/passive/com/sun/star/comp/test/deployment/Dispatch \
    desktop/test/deployment/passive/com/sun/star/comp/test/deployment/Provider \
    desktop/test/deployment/passive/com/sun/star/comp/test/deployment/Services \
))

$(eval $(call gb_Jar_set_componentfile,passive_java,desktop/test/deployment/passive/passive_java,OXT,passive_generic))

$(eval $(call gb_Jar_set_manifest,passive_java,$(SRCDIR)/desktop/test/deployment/passive/MANIFEST.MF))

$(eval $(call gb_Jar_set_packageroot,passive_java,com))

$(eval $(call gb_Jar_use_jars,passive_java, \
    libreoffice \
))

# vim: set noet sw=4 ts=4:
