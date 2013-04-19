# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Library_Library,forui))

$(eval $(call gb_Library_set_include,forui,\
    $$(INCLUDE) \
    -I$(SRCDIR)/formula/inc \
    -I$(SRCDIR)/formula/source/ui/inc \
))

$(eval $(call gb_Library_add_defs,forui,\
    -DFORMULA_DLLIMPLEMENTATION \
))

$(eval $(call gb_Library_use_external,forui,boost_headers))

$(eval $(call gb_Library_use_sdk_api,forui))

$(eval $(call gb_Library_use_libraries,forui,\
    comphelper \
    cppu \
    cppuhelper \
    for \
    sal \
    sfx \
    svl \
    svt \
    tl \
    utl \
    vcl \
	i18nlangtag \
	$(gb_UWINAPI) \
))

$(eval $(call gb_Library_add_exception_objects,forui,\
    formula/source/ui/dlg/formula \
    formula/source/ui/dlg/FormulaHelper \
    formula/source/ui/dlg/funcpage \
    formula/source/ui/dlg/funcutl \
    formula/source/ui/dlg/omoduleclient \
    formula/source/ui/dlg/parawin \
    formula/source/ui/dlg/structpg \
    formula/source/ui/resource/ModuleHelper \
))

# vim: set noet sw=4 ts=4:
