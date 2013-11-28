# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,postprocess_registry,$(patsubst %/,%,$(call gb_XcdTarget_get_target,))))

$(eval $(call gb_Package_add_files,postprocess_registry,$(LIBO_SHARE_FOLDER)/registry,\
	$(postprocess_XCDS) \
	$(foreach lang,$(gb_Configuration_LANGS), \
		Langpack-$(lang).xcd \
	) \
))

$(eval $(foreach lang,$(gb_CJK_LANGS),\
	$(call gb_Package_add_file,postprocess_registry,$(LIBO_SHARE_FOLDER)/registry/cjk_$(lang).xcd,cjk.xcd) \
))

$(eval $(foreach lang,$(gb_CTL_LANGS),\
	$(call gb_Package_add_file,postprocess_registry,$(LIBO_SHARE_FOLDER)/registry/ctl_$(lang).xcd,ctl.xcd) \
))

$(eval $(foreach lang,$(gb_CTLSEQCHECK_LANGS),\
	$(call gb_Package_add_file,postprocess_registry,$(LIBO_SHARE_FOLDER)/registry/ctlseqcheck_$(lang).xcd,ctlseqcheck.xcd) \
))

$(eval $(call gb_Package_add_files,postprocess_registry,$(LIBO_SHARE_FOLDER)/registry/res,\
	$(foreach lang,$(gb_Configuration_LANGS), \
		fcfg_langpack_$(lang).xcd \
		registry_$(lang).xcd \
	) \
))

# vim: set noet sw=4 ts=4:
