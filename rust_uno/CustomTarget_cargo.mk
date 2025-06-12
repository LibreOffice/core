# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,rust_uno/cargo))

.PHONY: $(call gb_CustomTarget_get_target,rust_uno/cargo)
$(call gb_CustomTarget_get_target,rust_uno/cargo): \
        $(call gb_Library_get_target,rust_uno-cpp) \
        $(gb_CustomTarget_workdir)/rust_uno/rustmaker/cpp
	cd $(SRCDIR)/rust_uno && cargo build $(if $(verbose),--verbose,) --release

# vim: set noet sw=4 ts=4:
