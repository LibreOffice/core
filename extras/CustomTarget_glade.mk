# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_CustomTarget_CustomTarget,extras/glade))

#
# Rules
#

$(call gb_CustomTarget_get_workdir,extras/source/glade)/libreoffice-catalog.xml : \
        $(SRCDIR)/extras/source/glade/libreoffice-catalog.xml.in \
        $(SRCDIR)/extras/source/glade/makewidgetgroup.xslt \
        | $(call gb_ExternalExecutable_get_dependencies,xsltproc)
	$(call gb_Output_announce,$(subst $(WORKDIR)/,,$@),$(true),XSLT,4)
	$(call gb_Helper_abbreviate_dirs, \
	mkdir -p $(dir $@) && \
	$(call gb_ExternalExecutable_get_command,xsltproc) --nonet \
		-o $@ $(SRCDIR)/extras/source/glade/makewidgetgroup.xslt $< \
	)

# vim: set noet sw=4 ts=4:
