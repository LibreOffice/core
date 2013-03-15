# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,uno_loader_classes,$(call gb_CustomTarget_get_workdir,odk/odkcommon/classes)))

$(eval $(call gb_Zip_add_commandoptions,uno_loader_classes,-r))

$(eval $(call gb_Zip_add_files,uno_loader_classes,\
		com/sun/star/lib/loader \
		win/unowinreg.dll \
))

$(call gb_Zip_get_target,uno_loader_classes) : $(call gb_CustomTarget_get_target,odk/odkcommon/classes)

# vim: set noet sw=4 ts=4:
