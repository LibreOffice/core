# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Jar_Jar,unoloader))

$(eval $(call gb_Jar_set_packageroot,unoloader,com))

$(eval $(call gb_Jar_set_manifest,unoloader,$(SRCDIR)/ridljar/source/unoloader/com/sun/star/lib/unoloader/manifest))

$(eval $(call gb_Jar_add_sourcefiles,unoloader,\
    ridljar/source/unoloader/com/sun/star/lib/unoloader/UnoClassLoader \
    ridljar/source/unoloader/com/sun/star/lib/unoloader/UnoLoader \
))

# vim:set noet sw=4 ts=4:
