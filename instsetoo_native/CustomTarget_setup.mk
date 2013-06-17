# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,instsetoo_native/setup))

$(eval $(call gb_CustomTarget_register_targets,instsetoo_native/setup,\
	ooenv \
	$(if $(filter TRUE,$(DISABLE_PYTHON)),,pythonloader.unorc) \
	versionrc \
))

$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/ooenv :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	( \
		echo 'java_path=`$(INSTDIR)/ure/bin/javaldx 2>/dev/null`' && \
		echo 'export LD_LIBRARY_PATH="$(INSTDIR)/program:$$java_path$${LD_LIBRARY_PATH:+:$$LD_LIBRARY_PATH}"' && \
		echo 'ulimit -c unlimited' && \
		echo 'export PATH="$(INSTDIR)/program:$(INSTDIR)/ure/bin:$$PATH"' && \
		echo 'export GNOME_DISABLE_CRASH_DIALOG=1' && \
		echo '# debugging assistance' && \
		echo 'export SAL_DISABLE_FLOATGRAB=1' && \
		echo 'export G_SLICE=always-malloc' && \
		echo 'export MALLOC_CHECK_=2' && \
		echo 'export MALLOC_PERTURB_=153' && \
		echo 'export OOO_DISABLE_RECOVERY=1' \
	) > $@

$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/pythonloader.unorc :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	( \
		echo '[Bootstrap]' && \
		$(if $(filter YES,$(SYSTEM_PYTHON)),\
			echo PYUNO_LOADER_PYTHONPATH='$$ORIGIN',\
			echo PYUNO_LOADER_PYTHONHOME='$$ORIGIN/python-core-$(PYTHON_VERSION)' && \
			echo PYUNO_LOADER_PYTHONPATH='$(foreach dir,lib lib/lib-dynload lib/lib-tk lib/site-packages,$$ORIGIN/python-core-$(PYTHON_VERSION)/$(dir)) $$ORIGIN' \
		) \
	) > $@

$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/versionrc :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),ECH,1)
	( \
		echo '[Version]' && \
		echo 'AllLanguages=en-US' && \
		echo 'buildid=$(shell git log -1 --format=%H)' && \
		echo 'ProductMajor=$(LIBO_VERSION_MAJOR)$(LIBO_VERSION_MINOR)$(LIBO_VERSION_MICRO)' && \
		echo 'ProductMinor=$(LIBO_VERSION_PATCH)' \
	) > $@

# vim: set noet sw=4 ts=4:
