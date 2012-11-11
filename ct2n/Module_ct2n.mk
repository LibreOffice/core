# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,ConvertTextToNumber))

ifneq (,$(filter CT2N,$(BUILD_TYPE)))

$(eval $(call gb_Module_add_targets,ConvertTextToNumber,\
	UnpackedTarball_ct2n \
	Zip_ct2n \
))

endif

# vim:set shiftwidth=4 softtabstop=4 expandtab:
