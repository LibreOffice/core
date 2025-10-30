# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t; fill-column: 100 -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Extension_Extension,rust_uno-example,rust_uno/example,nodeliver))

$(eval $(call gb_Extension_add_file,rust_uno-example,platform.components,$(call gb_Rdb_get_target,rust_uno-example)))

$(eval $(call gb_Extension_add_files,rust_uno-example,, \
    $(SRCDIR)/rust_uno/example/Addons.xcu \
    $(SRCDIR)/rust_uno/example/ProtocolHandler.xcu \
    $(SRCDIR)/rust_uno/target/release/$(if $(filter WNT,$(OS_FOR_BUILD)),rust_uno.dll,librust_uno.so) \
))

$(eval $(call gb_Extension_add_libraries,rust_uno-example, \
    rust_uno-example \
))

$(SRCDIR)/rust_uno/target/release/$(if $(filter WNT,$(OS_FOR_BUILD)),rust_uno.dll,librust_uno.so): $(call gb_CustomTarget_get_target,rust_uno/cargo)

# vim: set noet sw=4 ts=4:
