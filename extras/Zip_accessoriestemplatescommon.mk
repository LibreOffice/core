# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,accessoriestemplatescommon,$(call gb_UnpackedTarball_get_dir,templates-pack)/templates/common))

$(eval $(call gb_Zip_add_dependencies,accessoriestemplatescommon,\
	$(call gb_UnpackedTarball_get_target,templates-pack) \
))

$(eval $(call gb_Zip_add_files,accessoriestemplatescommon,\
	dummy_common_templates.txt \
	svn-commit.2.tmp \
	svn-commit.tmp \
))

# vim: set noet sw=4 ts=4:
