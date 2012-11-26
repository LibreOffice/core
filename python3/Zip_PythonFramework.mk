# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# Major Contributor(s):
# Copyright (C) 2012 Christian Lohmaier <lohmaier+LibreOffice@gmail.com>
#  (initial developer)
#
# All Rights Reserved.
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

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
