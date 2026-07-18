# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the Collabora Office project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_UnpackedTarball_UnpackedTarball,ecma376_opc))

$(eval $(call gb_UnpackedTarball_set_tarball,ecma376_opc,$(ECMA376_OPC_TARBALL),0))

# The archive holds the specification text and two electronic annexes
# side by side; the RELAX NG annex unpacks into the same directory.
$(eval $(call gb_UnpackedTarball_set_pre_action,ecma376_opc,\
	unzip -qq OpenPackagingConventions-RELAXNG.zip \
))

# vim: set noet sw=4 ts=4:
