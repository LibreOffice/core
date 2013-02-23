# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,moz))

ifeq ($(WITH_MOZAB4WIN),YES)

$(eval $(call gb_Module_add_targets,moz,\
	ExternalPackage_moz_inc \
	ExternalPackage_moz_lib \
	CustomTarget_runtime \
	Package_runtime \
	UnpackedTarball_moz_inc \
	UnpackedTarball_moz_lib \
))

endif

# vim: set noet sw=4 ts=4:
