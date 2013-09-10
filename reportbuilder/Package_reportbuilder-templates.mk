# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

# There is currently only a single, en-US, template that needs to be included in
# any installation set regardless of language settings, so keep this as a
# Package instead of an AllLangPackage (whose content would need to go into a
# language pack in scp2) for now.

$(eval $(call gb_Package_Package,reportbuilder-templates,$(SRCDIR)/reportbuilder/template))

$(eval $(call gb_Package_set_outdir,reportbuilder-templates,$(gb_INSTROOT)))

$(eval $(call gb_Package_add_files_with_dir,reportbuilder-templates,$(LIBO_SHARE_FOLDER)/template, \
    common/wizard/report/default.otr \
))

# vim: set noet sw=4 ts=4:
