# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,msc-externals))

ifneq ($(BUILD_X64),)

$(eval $(call gb_Module_add_targets,msc-externals,\
	Package_msvc_dlls \
))

endif

# Install the universal crts and VC runtimes (tdf#108580)
ifneq ($(VCREDIST_DIR),)

$(eval $(call gb_Module_add_targets,msc-externals,\
	Package_vcredist_exe \
))

endif

# vim: set noet sw=4 ts=4:
