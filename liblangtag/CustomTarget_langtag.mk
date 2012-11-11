# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,liblangtag/langtag))

$(call gb_CustomTarget_get_target,liblangtag/langtag) : $(call gb_Zip_get_target,liblangtag_data)
	rm -rf $(OUTDIR)/share/liblangtag \
	&& mkdir -p $(OUTDIR)/share/liblangtag \
	&& unzip $< -d $(OUTDIR)/share/liblangtag \
	&& touch $(call gb_CustomTarget_get_workdir,liblangtag/langtag)/.dir 


# vim: set noet sw=4 ts=4:
