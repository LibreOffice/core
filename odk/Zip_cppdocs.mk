# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,cppdocs,$(call gb_CustomTarget_get_workdir,odk/odkcommon)))

$(eval $(call gb_Zip_add_files,cppdocs,\
	docs/cpp/ref \
))

$(eval $(call gb_Zip_add_commandoptions,cppdocs,-r))

$(call gb_Zip_get_target,cppdocs) : $(call gb_CustomTarget_get_target,odk/odkcommon/docs/cpp/ref)

# vim: set noet sw=4 ts=4:
