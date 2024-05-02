# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,readlicense_oo_readmes,$(gb_CustomTarget_workdir)/readlicense_oo/readme))

$(eval $(call gb_Package_add_files,readlicense_oo_readmes,$(LIBO_SHARE_READMES_FOLDER), \
    $(foreach lang,$(readlicense_oo_LANGS),$(call gb_README,$(lang))) \
))
    # readlicense_oo_LANGS from readlicense_oo/CustomTarget_readme.mk

# vim: set noet sw=4 ts=4:
