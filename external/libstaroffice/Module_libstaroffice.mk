# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,libstaroffice))

$(eval $(call gb_Module_add_targets,libstaroffice,\
	UnpackedTarball_libstaroffice \
))

ifeq ($(COM),MSC)

$(eval $(call gb_Module_add_targets,libstaroffice,\
	Library_staroffice \
))

else

$(eval $(call gb_Module_add_targets,libstaroffice,\
	ExternalPackage_libstaroffice \
	ExternalProject_libstaroffice \
))

endif

# vim: set noet sw=4 ts=4:
