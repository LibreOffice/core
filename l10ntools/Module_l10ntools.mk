# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,l10ntools))

$(eval $(call gb_Module_add_targets_for_build,l10ntools,\
    Executable_helpex \
    Executable_idxdict \
    Executable_ulfex \
    Executable_cfgex \
    Executable_uiex \
    Executable_xrmex \
    Executable_localize \
    Executable_transex3 \
    Executable_pocheck \
    Executable_propex \
    Executable_treex \
    Executable_stringex \
    StaticLibrary_transex \
))

$(eval $(call gb_Module_add_targets,l10ntools,\
    Package_ulfconv \
))

# vim:set noet sw=4 ts=4:
