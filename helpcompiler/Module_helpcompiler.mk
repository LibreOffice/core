# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,helpcompiler))

$(eval $(call gb_Module_add_targets,helpcompiler,\
    $(call gb_Helper_optionals_or,HELPTOOLS XMLHELP,Library_helplinker) \
))

$(eval $(call gb_Module_add_targets_for_build,helpcompiler,\
    $(if $(DISABLE_DYNLOADING),,$(call gb_Helper_optional,HELPTOOLS, \
        Executable_HelpIndexer \
        Executable_HelpLinker \
    )) \
))

# vim:set noet sw=4 ts=4:
