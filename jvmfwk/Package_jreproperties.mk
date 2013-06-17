# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,jvmfwk_jreproperties,$(call gb_CustomTarget_get_workdir,jvmfwk/jreproperties)))

$(eval $(call gb_Package_set_outdir,jvmfwk_jreproperties,$(INSTDIR)))

$(eval $(call gb_Package_add_file,jvmfwk_jreproperties,$(LIBO_URE_LIB_FOLDER)/JREProperties.class,JREProperties.class))

# vim:set noet sw=4 ts=4:
