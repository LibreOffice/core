# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,python_shell,$(gb_CustomTarget_workdir)/pyuno/python_shell))

ifeq ($(OS),MACOSX)
$(eval $(call gb_Package_add_file,python_shell,$(LIBO_ETC_FOLDER)/python,python.sh))
else
$(eval $(call gb_Package_add_file,python_shell,$(LIBO_BIN_FOLDER)/python,python.sh))
endif

# vim: set noet sw=4 ts=4:
