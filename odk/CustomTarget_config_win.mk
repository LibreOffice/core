# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/config))

$(eval $(call gb_CustomTarget_register_targets,odk/config,\
    setsdkenv_windows.bat \
))

$(call gb_CustomTarget_get_workdir,odk/config)/setsdkenv_windows.bat : $(SRCDIR)/odk/config/setsdkenv_windows.bat
	sed $< -e 's#__SDKNAME__#libreoffice$(PRODUCTVERSION)_sdk#' > $@

# vim: set noet sw=4 ts=4:
