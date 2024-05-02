# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,scp2/macros))

$(call gb_CustomTarget_get_target,scp2/macros) : $(gb_CustomTarget_workdir)/scp2/macros/langmacros.inc

$(gb_CustomTarget_workdir)/scp2/macros/langmacros.inc :| $(gb_CustomTarget_workdir)/scp2/macros/.dir

$(gb_CustomTarget_workdir)/scp2/macros/langmacros.inc : $(SRCDIR)/scp2/macros/macro.pl $(BUILDDIR)/config_host.mk.stamp
	$(call gb_Helper_abbreviate_dirs,\
		export COMPLETELANGISO_VAR='$(gb_ScpTemplateTarget_LANGS)' && \
		export HELP_LANGS='$(gb_HELP_LANGS)' && \
		$(PERL) $< -verbose -o $@ -c $(BUILDDIR)/config_host.mk.stamp \
	)

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
