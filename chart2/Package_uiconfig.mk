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

$(eval $(call gb_Package_Package,chart2_uiconfig,$(SRCDIR)/chart2/uiconfig))

# TODO: this was not listed in d.lst
# $(eval $(call gb_Package_add_file,chart2_uiconfig,xml/uiconfig/modules/schart/accelerator/en-US/default.xml,accelerator/en-US/default.xml))
$(eval $(call gb_Package_add_file,chart2_uiconfig,xml/uiconfig/modules/schart/menubar/menubar.xml,menubar/menubar.xml))
$(eval $(call gb_Package_add_file,chart2_uiconfig,xml/uiconfig/modules/schart/statusbar/statusbar.xml,statusbar/statusbar.xml))
$(eval $(call gb_Package_add_file,chart2_uiconfig,xml/uiconfig/modules/schart/toolbar/arrowshapes.xml,toolbar/arrowshapes.xml))
$(eval $(call gb_Package_add_file,chart2_uiconfig,xml/uiconfig/modules/schart/toolbar/basicshapes.xml,toolbar/basicshapes.xml))
$(eval $(call gb_Package_add_file,chart2_uiconfig,xml/uiconfig/modules/schart/toolbar/calloutshapes.xml,toolbar/calloutshapes.xml))
$(eval $(call gb_Package_add_file,chart2_uiconfig,xml/uiconfig/modules/schart/toolbar/drawbar.xml,toolbar/drawbar.xml))
$(eval $(call gb_Package_add_file,chart2_uiconfig,xml/uiconfig/modules/schart/toolbar/flowchartshapes.xml,toolbar/flowchartshapes.xml))
$(eval $(call gb_Package_add_file,chart2_uiconfig,xml/uiconfig/modules/schart/toolbar/standardbar.xml,toolbar/standardbar.xml))
$(eval $(call gb_Package_add_file,chart2_uiconfig,xml/uiconfig/modules/schart/toolbar/starshapes.xml,toolbar/starshapes.xml))
$(eval $(call gb_Package_add_file,chart2_uiconfig,xml/uiconfig/modules/schart/toolbar/symbolshapes.xml,toolbar/symbolshapes.xml))
$(eval $(call gb_Package_add_file,chart2_uiconfig,xml/uiconfig/modules/schart/toolbar/toolbar.xml,toolbar/toolbar.xml))

# vim: set noet ts=4 sw=4:
