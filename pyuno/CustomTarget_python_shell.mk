# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,pyuno/python_shell))

$(eval $(call gb_CustomTarget_register_targets,pyuno/python_shell,\
	os.sh \
	python.sh \
))

ifeq ($(OS),MACOSX)
pyuno_PYTHON_SHELL_VERSION:=$(PYTHON_VERSION_MAJOR).$(PYTHON_VERSION_MINOR)
else
pyuno_PYTHON_SHELL_VERSION:=$(PYTHON_VERSION)
endif

$(call gb_CustomTarget_get_workdir,pyuno/python_shell)/python.sh : \
		$(SRCDIR)/pyuno/zipcore/python.sh \
		$(call gb_CustomTarget_get_workdir,pyuno/python_shell)/os.sh
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),CAT,1)
	cat $^ > $@ && chmod +x $@

$(call gb_CustomTarget_get_workdir,pyuno/python_shell)/os.sh : \
		$(SRCDIR)/pyuno/zipcore/$(if $(filter MACOSX,$(OS)),mac,nonmac).sh
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),SED,1)
	sed -e "s/%%PYVERSION%%/$(pyuno_PYTHON_SHELL_VERSION)/g" \
        -e "s,%%PYTHON_FRAMEWORK_RELATIVE_PATH%%,$(if $(filter YES,$(ENABLE_MACOSX_MACLIKE_APP_STRUCTURE)),../Frameworks/),g" \
		$< > $@

# vim: set noet sw=4 ts=4:
