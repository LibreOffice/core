# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,desktop/unopackages_install))

$(call gb_CustomTarget_get_target,desktop/unopackages_install) : | \
    $(gb_CustomTarget_workdir)/desktop/unopackages_install/uno_packages/cache/uno_packages

$(gb_CustomTarget_workdir)/desktop/unopackages_install/uno_packages/cache/uno_packages : | \
    $(gb_CustomTarget_workdir)/desktop/unopackages_install/.dir
	mkdir -p $@

# vim: set noet sw=4 ts=4:
