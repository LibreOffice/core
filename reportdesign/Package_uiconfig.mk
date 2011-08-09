# -*- Mode: makefile-gmake; tab-width: 4; indent-tabs-mode: t -*-
# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License or as specified alternatively below. You may obtain a copy of
# the License at http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
#       David Tardon, Red Hat Inc. <dtardon@redhat.com>
# Portions created by the Initial Developer are Copyright (C) 2010 the
# Initial Developer. All Rights Reserved.
#
# Major Contributor(s):
#
# For minor contributions see the git repository.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Package_Package,reportdesign_uiconfig,$(SRCDIR)/reportdesign/uiconfig/dbreport))

$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/menubar/menubar.xml,menubar/menubar.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/statusbar/statusbar.xml,statusbar/statusbar.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/alignmentbar.xml,toolbar/alignmentbar.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/arrowshapes.xml,toolbar/arrowshapes.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/basicshapes.xml,toolbar/basicshapes.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/calloutshapes.xml,toolbar/calloutshapes.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/drawbar.xml,toolbar/drawbar.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/flowchartshapes.xml,toolbar/flowchartshapes.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/Formatting.xml,toolbar/Formatting.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/reportcontrols.xml,toolbar/reportcontrols.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/resizebar.xml,toolbar/resizebar.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/sectionalignmentbar.xml,toolbar/sectionalignmentbar.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/sectionshrinkbar.xml,toolbar/sectionshrinkbar.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/starshapes.xml,toolbar/starshapes.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/symbolshapes.xml,toolbar/symbolshapes.xml))
$(eval $(call gb_Package_add_file,reportdesign_uiconfig,xml/uiconfig/modules/dbreport/toolbar/toolbar.xml,toolbar/toolbar.xml))

# vim: set noet sw=4 ts=4:
