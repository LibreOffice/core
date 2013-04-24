# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,LibreOfficePython.framework,$(call gb_UnpackedTarball_get_dir,python3)/python-inst/@__________________________________________________OOO))

$(eval $(call gb_Zip_add_dependencies,LibreOfficePython.framework,\
              $(call gb_CustomTarget_get_target,python3/fixscripts) \
              $(call gb_CustomTarget_get_target,python3/fixinstallnames) \
              $(call gb_CustomTarget_get_target,python3/executables) \
))

$(eval $(call gb_Zip_add_file,LibreOfficePython.framework,LibreOfficePython.framework))

# preserve links to not duplicate stuff in the zip...
# links are not preserved when unpacking, need to be created in scp2
$(eval $(call gb_Zip_add_commandoptions,LibreOfficePython.framework,-y))

# vim: set noet sw=4 ts=4:
