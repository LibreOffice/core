# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,solenv/gdb))

$(call gb_CustomTarget_get_target,solenv/gdb) : \
	$(SRCDIR)/solenv/bin/install-gdb-printers \
	$(SRCDIR)/solenv/gdb/autoload.template \
	| $(call gb_CustomTarget_get_workdir,solenv/gdb)/.dir

$(call gb_CustomTarget_get_target,solenv/gdb) :
	$(call gb_Output_announce,solenv/gdb,$(true),SH ,1)
	install-gdb-printers -a $(call gb_CustomTarget_get_workdir,solenv/gdb) -f && \
	touch $@

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
