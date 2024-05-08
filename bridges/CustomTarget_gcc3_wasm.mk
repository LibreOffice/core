# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,bridges/gcc3_wasm))

$(eval $(call gb_CustomTarget_register_targets,bridges/gcc3_wasm, \
    callvirtualfunction-wrapper.cxx \
    callvirtualfunction-impls.s \
))

$(gb_CustomTarget_workdir)/bridges/gcc3_wasm/callvirtualfunction-impls.s \
$(gb_CustomTarget_workdir)/bridges/gcc3_wasm/callvirtualfunction-wrapper.cxx: \
        $(call gb_Executable_get_target_for_build,wasmcallgen) $(call gb_UnoApi_get_target,udkapi) \
        $(call gb_UnoApi_get_target,offapi)
	$(call gb_Executable_get_command,wasmcallgen) \
        $(gb_CustomTarget_workdir)/bridges/gcc3_wasm/callvirtualfunction-wrapper.cxx \
        $(gb_CustomTarget_workdir)/bridges/gcc3_wasm/callvirtualfunction-impls.s \
        +$(call gb_UnoApi_get_target,udkapi) +$(call gb_UnoApi_get_target,offapi)

# vim: set noet sw=4 ts=4:
