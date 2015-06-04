# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,liborcus))

$(eval $(call gb_Module_add_targets,liborcus,\
	UnpackedTarball_liborcus \
))

ifeq ($(COM),MSC)

$(eval $(call gb_Module_add_targets,liborcus,\
	Library_orcus \
	Library_orcus-parser \
))

else # !MSC

$(eval $(call gb_Module_add_targets,liborcus,\
	ExternalPackage_liborcus \
	ExternalProject_liborcus \
))

endif

# vim: set noet sw=4 ts=4:
