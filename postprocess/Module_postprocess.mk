# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,postprocess))

# UGLY: postprocess_XCDS variable from CustomTarget_registry
# is reused in Package_registry!

$(eval $(call gb_Module_add_targets,postprocess,\
	CustomTarget_images \
	CustomTarget_registry \
	Package_images \
	Rdb_services \
))
$(eval $(call gb_Module_add_l10n_targets,postprocess,\
	Package_registry \
))

ifeq ($(WINDOWS_BUILD_SIGNING),TRUE)
$(eval $(call gb_Module_add_targets,postprocess,\
	CustomTarget_signing \
))
endif

# vim: set noet sw=4 ts=4:
