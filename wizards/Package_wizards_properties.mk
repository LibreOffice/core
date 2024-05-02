# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,wizards_properties,$(gb_CustomTarget_workdir)/wizards/locproperties))

$(eval $(call gb_Package_add_files,wizards_properties,$(LIBO_SHARE_FOLDER)/wizards,\
	$(foreach lang,$(subst -,_,$(wizards_LANGS)),resources_$(lang).properties) \
))

# vim:set noet sw=4 ts=4:
