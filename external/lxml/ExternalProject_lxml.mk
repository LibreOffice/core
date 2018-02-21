# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_ExternalProject_ExternalProject,lxml))

$(eval $(call gb_ExternalProject_use_external_project,lxml,python3))
$(eval $(call gb_ExternalProject_use_external_project,lxml,libxml2))
$(eval $(call gb_ExternalProject_use_external_project,lxml,libxslt))
$(eval $(call gb_ExternalProject_use_external_project,lxml,zlib))

$(eval $(call gb_ExternalProject_register_targets,lxml,\
	build \
))

ifeq ($(SYSTEM_PYTHON),TRUE)
lxml_PYTHONCOMMAND := $(PYTHON_FOR_BUILD)
else
lxml_PYTHONCOMMAND := $(call gb_UnpackedTarball_get_dir,python3)/python
endif

$(call gb_ExternalProject_get_state_target,lxml,build):
	$(call gb_ExternalProject_run,build,\
		$(lxml_PYTHONCOMMAND) setup.py build && \
		$(lxml_PYTHONCOMMAND) setup.py install --install-lib install \
	)

# vim: set noet sw=4 ts=4:
