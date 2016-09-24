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
	sed -e 's|%PRODUCT_RELEASE%|$(PRODUCTVERSION)|g' \
	    -e 's|%DOXYGEN_PREFIX0%|$(if $(DOXYGEN),.,http://api.libreoffice.org)|g' \
	    -e 's|%DOXYGEN_PREFIX1%|$(if $(DOXYGEN),..,http://api.libreoffice.org)|g' \
	    -e 's|%DOXYGEN_PREFIX2%|$(if $(DOXYGEN),../..,http://api.libreoffice.org)|g' \
	    -e 's|%JAVADOC_PREFIX0%|$(if $(ENABLE_JAVA),.,http://api.libreoffice.org)|g' \
	    -e 's|%JAVADOC_PREFIX1%|$(if $(ENABLE_JAVA),..,http://api.libreoffice.org)|g' \
	    < $< > $@

# vim: set noet sw=4 ts=4:
