# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,pyuno/python_bin))

# do we need it here: $(gb_Executable_EXT)?
$(call gb_CustomTarget_get_target,pyuno/python_bin) : \
    $(call gb_CustomTarget_get_workdir,pyuno/python_bin)/python.bin

$(call gb_CustomTarget_get_workdir,pyuno/python_bin)/python.bin : $(OUTDIR)/bin/python | \
    $(call gb_CustomTarget_get_workdir,pyuno/python_bin)/.dir
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),COP,1)
	cp $< $@.tmp
	strip $@.tmp
	mv $@.tmp $@

# vim: set noet sw=4 ts=4:
