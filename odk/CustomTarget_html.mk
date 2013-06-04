# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,odk/html))

$(eval $(call gb_CustomTarget_register_targets,odk/html,\
	docs/install.html \
	docs/tools.html \
	examples/DevelopersGuide/examples.html \
	examples/examples.html \
	index.html \
))

$(call gb_CustomTarget_get_workdir,odk/html)/%.html : $(SRCDIR)/odk/%.html
	$(call gb_Output_announce,$*.html,$(true),SED,1)
	cat $< | tr -d "\015" | sed -e 's/%PRODUCT_RELEASE%/$(PRODUCTVERSION)/g' > $@

# vim: set noet sw=4 ts=4:
