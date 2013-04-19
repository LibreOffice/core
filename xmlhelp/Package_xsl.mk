# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#

$(eval $(call gb_Package_Package,xmlhelp_xsl,$(SRCDIR)/xmlhelp/util))

$(eval $(call gb_Package_add_file,xmlhelp_xsl,bin/compact.xsl,compact.xsl))
$(eval $(call gb_Package_add_file,xmlhelp_xsl,bin/embed.xsl,embed.xsl))
$(eval $(call gb_Package_add_file,xmlhelp_xsl,bin/idxcaption.xsl,idxcaption.xsl))
$(eval $(call gb_Package_add_file,xmlhelp_xsl,bin/idxcontent.xsl,idxcontent.xsl))
$(eval $(call gb_Package_add_file,xmlhelp_xsl,bin/main_transform.xsl,main_transform.xsl))

# vim: set shiftwidth=4 tabstop=4 noexpandtab:
