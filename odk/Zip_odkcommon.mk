# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,odkcommon,$(call gb_CustomTarget_get_workdir,odk/odkcommon)))

$(eval $(call gb_Zip_add_files,odkcommon,\
	$(odkcommon_ZIPLIST) \
))

$(eval $(call gb_Zip_add_commandoptions,odkcommon,-r))

# Additional dependencies for the zip. Because we are zipping whole
# directories, we cannot just depend on make to figure out from file
# dependencies which custom targets need to be built.
$(call gb_Zip_get_target,odkcommon) : $(odkcommon_ZIPDEPS)

# vim: set noet sw=4 ts=4:
