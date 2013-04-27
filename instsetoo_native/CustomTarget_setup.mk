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
	ure-link \
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

$(call gb_CustomTarget_get_workdir,instsetoo_native/setup)/ure-link :
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),LN ,1)
	rm -f $@ && \
	mkdir -p $(dir $@)/ure && \
	cd $(dir $@) && \
	ln -s ure ure-link

# vim: set noet sw=4 ts=4:
