# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,postprocess_registry_install,$(call gb_CustomTarget_get_workdir,postprocess/registry)))

$(eval $(call gb_Package_set_outdir,postprocess_registry_install,$(gb_INSTROOT)))

$(eval $(call gb_Package_add_files,postprocess_registry_install,$(LIBO_SHARE_FOLDER)/registry,\
	$(postprocess_XCDS) \
	$(foreach lang,$(gb_Configuration_LANGS), \
		Langpack-$(lang).xcd \
	) \
))

$(eval $(call gb_Package_add_files,postprocess_registry_install,$(LIBO_SHARE_FOLDER)/registry/res,\
	$(foreach lang,$(gb_Configuration_LANGS), \
		fcfg_langpack_$(lang).xcd \
		registry_$(lang).xcd \
	) \
))

# vim: set noet sw=4 ts=4:
