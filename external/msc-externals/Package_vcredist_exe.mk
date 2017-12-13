# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,vcredist_exe,$(VCREDIST_DIR)))

$(eval $(call gb_Package_add_files,vcredist_exe,$(LIBO_ETC_FOLDER),\
	$(VCREDIST_EXE) \
))

# vim:set shiftwidth=4 tabstop=4 noexpandtab:
