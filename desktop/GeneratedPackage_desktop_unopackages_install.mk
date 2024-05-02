# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_GeneratedPackage_GeneratedPackage,desktop_unopackages_install,$(gb_CustomTarget_workdir)/desktop/unopackages_install))

$(eval $(call gb_GeneratedPackage_use_customtarget,desktop_unopackages_install,desktop/unopackages_install))

$(eval $(call gb_GeneratedPackage_add_dir,desktop_unopackages_install,$(INSTROOT)/$(LIBO_SHARE_FOLDER)/uno_packages,uno_packages))

# vim: set noet sw=4 ts=4:
