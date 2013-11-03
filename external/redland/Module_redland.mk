# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Module_Module,redland))

ifeq ($(SYSTEM_REDLAND),NO)
$(eval $(call gb_Module_add_targets,redland,\
	UnpackedTarball_raptor \
	UnpackedTarball_rasqal \
	UnpackedTarball_redland \
))
ifeq ($(COM),MSC)
$(eval $(call gb_Module_add_targets,redland,\
	Library_raptor \
	Library_rasqal \
	Library_rdf \
))
else
$(eval $(call gb_Module_add_targets,redland,\
	ExternalPackage_raptor \
	ExternalPackage_rasqal \
	ExternalPackage_redland \
	ExternalProject_raptor \
	ExternalProject_rasqal \
	ExternalProject_redland \
))
endif
endif

# vim: set noet sw=4 ts=4:
