# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Jar_Jar,unoloader,org.libreoffice.unoloader))

$(eval $(call gb_Jar_set_packageroot,unoloader,com))

$(eval $(call gb_Jar_set_manifest,unoloader,$(SRCDIR)/ridljar/source/unoloader/com/sun/star/lib/unoloader/manifest))

# the module-info.class is manually added here since it's not in "com" dir
$(if $(MODULAR_JAVA),$(eval $(call gb_Jar_add_packagedirs,unoloader,\
	$(call gb_JavaClassSet_get_classdir,$(call gb_Jar_get_classsetname,unoloader))/module-info.class \
)))

$(if $(MODULAR_JAVA),$(eval $(call gb_Jar_add_sourcefiles_java9,unoloader,\
    ridljar/source/unoloader/module-info \
)))

$(eval $(call gb_Jar_add_sourcefiles,unoloader,\
    ridljar/source/unoloader/com/sun/star/lib/unoloader/UnoClassLoader \
    ridljar/source/unoloader/com/sun/star/lib/unoloader/UnoLoader \
))

# vim:set noet sw=4 ts=4:
