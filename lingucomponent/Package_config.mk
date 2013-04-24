# -*- Mode: makefile; tab-width: 4; indent-tabs-mode: t -*-
#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,lingucomponent_config,$(SRCDIR)/lingucomponent/config))

$(eval $(call gb_Package_add_file,lingucomponent_config,xml/registry/data/org/openoffice/Office/Linguistic-lingucomponent-hyphenator.xcu,Linguistic-lingucomponent-hyphenator.xcu))
$(eval $(call gb_Package_add_file,lingucomponent_config,xml/registry/data/org/openoffice/Office/Linguistic-lingucomponent-spellchecker.xcu,Linguistic-lingucomponent-spellchecker.xcu))
$(eval $(call gb_Package_add_file,lingucomponent_config,xml/registry/data/org/openoffice/Office/Linguistic-lingucomponent-thesaurus.xcu,Linguistic-lingucomponent-thesaurus.xcu))

# vim: set noet sw=4 ts=4:
