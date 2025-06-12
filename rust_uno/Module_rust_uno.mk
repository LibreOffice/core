# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,rust_uno))

ifeq ($(ENABLE_RUST_UNO),TRUE)
$(eval $(call gb_Module_add_targets,rust_uno, \
    CustomTarget_cargo \
    CustomTarget_rustmaker \
    Extension_rust_uno-example \
    Library_rust_uno-cpp \
    Library_rust_uno-example \
))
endif

# vim: set noet sw=4 ts=4:
