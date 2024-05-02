# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,static/unoembind))

$(eval $(call gb_CustomTarget_register_targets,static/unoembind, \
    bindings_uno.cxx \
    bindings_uno.js \
))

$(gb_CustomTarget_workdir)/static/unoembind/bindings_uno.cxx \
$(gb_CustomTarget_workdir)/static/unoembind/bindings_uno.hxx \
$(gb_CustomTarget_workdir)/static/unoembind/bindings_uno.js: \
        $(call gb_Executable_get_target_for_build,embindmaker) $(call gb_UnoApi_get_target,udkapi) \
        $(call gb_UnoApi_get_target,offapi)
	$(call gb_Executable_get_command,embindmaker) uno \
        $(gb_CustomTarget_workdir)/static/unoembind/bindings_uno.cxx \
        $(gb_CustomTarget_workdir)/static/unoembind/bindings_uno.hxx \
        $(gb_CustomTarget_workdir)/static/unoembind/bindings_uno.js \
        +$(call gb_UnoApi_get_target,udkapi) +$(call gb_UnoApi_get_target,offapi)

# vim: set noet sw=4 ts=4:
