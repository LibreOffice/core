# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

ifeq ($(OS),WNT)
jvmfwk_Package_jreproperties_solver_LIBDIR := bin
else
jvmfwk_Package_jreproperties_solver_LIBDIR := lib
endif

$(eval $(call gb_Package_Package,jvmfwk_jreproperties_solver,$(call gb_CustomTarget_get_workdir,jvmfwk/jreproperties)))

$(eval $(call gb_Package_add_file,jvmfwk_jreproperties_solver,$(jvmfwk_Package_jreproperties_solver_LIBDIR)/JREProperties.class,JREProperties.class))

# vim:set noet sw=4 ts=4:
