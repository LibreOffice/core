# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Zip_Zip,odkexamples,$(SRCDIR)/odk))

odkexamples_ZIPLIST := $(subst $(SRCDIR)/odk/,,$(shell find $(SRCDIR)/odk/examples \
	-not \( -name "makefile.mk" -o -name "*Inspector*" \
	-o -name "*Container1*" -o -name "*Storage*" \
	-o -name "*register_component*" -o -name "examples.html" \
	-o -name "*ConverterServlet*" -o -name "*NotesAccess*" \
	-o -name "*delphi*" \)))

$(eval $(call gb_Zip_add_files,odkexamples,\
		$(odkexamples_ZIPLIST) \
))

#FIXME: scp2 seems to require the zip to be in bin
$(call gb_Zip_get_final_target,odkexamples) : $(OUTDIR)/bin/odkexamples.zip
$(call gb_Zip_get_clean_target,odkexamples) : clean_odkexamples_zip

$(OUTDIR)/bin/odkexamples.zip : $(call gb_Zip_get_target,odkexamples)
	$(call gb_Deliver_deliver,$<,$@)

.PHONY : clean_odkexamples_zip
clean_odkexamples_zip:
	rm -f $(OUTDIR)/bin/odkexamples.zip

# vim: set noet sw=4 ts=4:
