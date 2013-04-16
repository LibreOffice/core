# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,tooltip_properties,$(call gb_CustomTarget_get_workdir,tooltip/tooltip_properties)))

$(eval $(call gb_Package_set_outdir,tooltip_properties,$(INSTDIR)))

$(eval $(call gb_Package_add_files,tooltip_properties,help/tooltip/,\
	$(foreach lang,$(subst -,_,$(gb_TRANS_LANGS)),shared_$(lang).properties) \
))
# vim: set noet sw=4 ts=4:
