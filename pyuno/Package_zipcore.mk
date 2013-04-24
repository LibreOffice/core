# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,python_zipcore,$(call gb_CustomTarget_get_workdir,pyuno/zipcore)))

pyuno_PYTHON_ARCHIVE_NAME:=python-core-$(PYTHON_VERSION).zip

$(eval $(call gb_Package_add_file,python_zipcore,bin/$(pyuno_PYTHON_ARCHIVE_NAME),$(pyuno_PYTHON_ARCHIVE_NAME)))

# vim: set noet sw=4 ts=4:
