# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,odkexamples,$(SRCDIR)/odk))

$(eval $(call gb_Zip_add_commandoptions,odkexamples,\
	-r \
	-x "*makefile.mk" \
	-x "*Container1*" \
	-x "*Storage*" \
	-x "*register_component*" \
	-x "*examples.html" \
	-x "*ConverterServlet*" \
	-x "*NotesAccess*" \
	-x "*delphi*" \
))

$(eval $(call gb_Zip_add_files,odkexamples,\
	examples \
))

#FIXME: scp2 seems to require the zip to be in bin
$(call gb_Zip_get_final_target,odkexamples) : $(OUTDIR)/bin/odkexamples.zip | $(OUTDIR)/bin/.dir
$(call gb_Zip_get_clean_target,odkexamples) : clean_odkexamples_zip

$(OUTDIR)/bin/odkexamples.zip : $(call gb_Zip_get_target,odkexamples)
	$(call gb_Deliver_deliver,$<,$@)

.PHONY : clean_odkexamples_zip
clean_odkexamples_zip:
	rm -f $(OUTDIR)/bin/odkexamples.zip

# vim: set noet sw=4 ts=4:
