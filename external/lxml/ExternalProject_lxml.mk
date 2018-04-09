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

lxml_PYTHON := $(call gb_ExternalExecutable_get_command,python)

$(call gb_ExternalProject_get_state_target,lxml,build): \
	$(call gb_ExternalExecutable_get_dependencies,python)
	$(call gb_ExternalProject_run,build,\
		PYPATH=$${PYPATH:+$$PYPATH:}$(call gb_UnpackedTarball_get_dir,lxml)/install && \
		$(if $(PYTHON_FOR_BUILD), \
			unset MACOSX_DEPLOYMENT_TARGET && , \
			CFLAGS="$$CFLAGS -I$(call gb_UnpackedTarball_get_dir,python3)" && \
			CFLAGS="$$CFLAGS -I$(call gb_UnpackedTarball_get_dir,python3)/Include" && \
			LDFLAGS="$$LDFLAGS -L$(call gb_UnpackedTarball_get_dir,python3)" && \
			_PYTHON_PROJECT_BASE=$(call gb_UnpackedTarball_get_dir,python3) && \
			export CFLAGS LDFLAGS _PYTHON_PROJECT_BASE && ) \
		$(lxml_PYTHON) setup.py build \
			$(if $(SYSTEM_LIBXML),,--with-xml2-config=$(call gb_UnpackedTarball_get_dir,libxml2)/xml2-config) \
			$(if $(SYSTEM_LIBXSLT),,--with-xslt-config=$(call gb_UnpackedTarball_get_dir,libxslt)/xslt-config) && \
		rm -fr install && \
		mkdir install && \
		$(lxml_PYTHON) setup.py install \
			--install-lib install \
	)

# vim: set noet sw=4 ts=4:
