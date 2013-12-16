# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Jar_Jar,active_java))

$(eval $(call gb_Jar_add_sourcefiles,active_java, \
    desktop/test/deployment/active/com/sun/star/comp/test/deployment/Dispatch \
    desktop/test/deployment/active/com/sun/star/comp/test/deployment/Provider \
    desktop/test/deployment/active/com/sun/star/comp/test/deployment/Services \
))

$(eval $(call gb_Jar_set_manifest,active_java,$(SRCDIR)/desktop/test/deployment/active/MANIFEST.MF))

$(eval $(call gb_Jar_set_packageroot,active_java,com))

$(eval $(call gb_Jar_use_jars,active_java, \
    juh \
    ridl \
    unoil \
))

# vim: set noet sw=4 ts=4:
