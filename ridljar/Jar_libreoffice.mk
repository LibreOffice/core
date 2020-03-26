# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Jar_Jar,libreoffice))

$(eval $(call gb_Jar_set_packageroot,libreoffice,com))

$(eval $(call gb_Jar_set_manifest,libreoffice,$(SRCDIR)/ridljar/source/libreoffice/manifest))

$(eval $(call gb_Jar_add_sourcefiles,libreoffice,\
    ridljar/source/libreoffice/module-info \
    ridljar/source/libreoffice/com/sun/star/Dummy \
))

$(eval $(call gb_Jar_add_packagedir,libreoffice,\
    module-info.class \
))

$(eval $(call gb_Jar_add_manifest_classpath,libreoffice, \
    juh.jar \
    ridl.jar \
))

# vim:set noet sw=4 ts=4:
