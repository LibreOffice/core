# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,pyuno/pythonloader_ini))

$(eval $(call gb_CustomTarget_register_targets,pyuno/pythonloader_ini, \
    $(call gb_Helper_get_rcfile,pythonloader.uno) \
))

$(call gb_CustomTarget_get_workdir,pyuno/pythonloader_ini)/$(call gb_Helper_get_rcfile,pythonloader.uno): \
			$(BUILDDIR)/config_$(gb_Side)/config_python.h \
            $(SRCDIR)/pyuno/CustomTarget_pyuno_pythonloader_ini.mk
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	(   printf '[Bootstrap]\n' && \
            $(if $(SYSTEM_PYTHON),, \
                printf 'PYUNO_LOADER_PYTHONHOME=%s\n' \
                    $(if $(filter MACOSX,$(OS)), \
                        '$$ORIGIN/../Frameworks/LibreOfficePython.framework', \
                        '$$ORIGIN/python-core-$(PYTHON_VERSION)') &&) \
            printf 'PYUNO_LOADER_PYTHONPATH=%s$$ORIGIN\n' \
                $(if $(SYSTEM_PYTHON), \
                    '', \
                    $(if $(filter MACOSX,$(OS)), \
                        '$(foreach dir,/ /lib-dynload /lib-tk /site-packages,$(patsubst %/,%,$$ORIGIN/../Frameworks/LibreOfficePython.framework/Versions/Current/lib/python$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)$(dir))) ', \
                    $(if $(filter WNT,$(OS)), \
                        '$(foreach dir,/ /site-packages,$(patsubst %/,%,$$ORIGIN/python-core-$(PYTHON_VERSION)/lib$(dir))) ', \
                        '$(foreach dir,/ /lib-dynload /lib-tk /site-packages,$(patsubst %/,%,$$ORIGIN/python-core-$(PYTHON_VERSION)/lib$(dir))) '))) \
        ) > $@

# vim: set noet sw=4 ts=4:
