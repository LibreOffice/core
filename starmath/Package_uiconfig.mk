# Version: MPL 1.1 / GPLv3+ / LGPLv3+
#
# The contents of this file are subject to the Mozilla Public License Version
# 1.1 (the "License"); you may not use this file except in compliance with
# the License. You may obtain a copy of the License at
# http://www.mozilla.org/MPL/
#
# Software distributed under the License is distributed on an "AS IS" basis,
# WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
# for the specific language governing rights and limitations under the
# License.
#
# The Initial Developer of the Original Code is
# Norbert Thiebaud <nthiebaud@gmail.com> (C) 2010, All Rights Reserved.
#
# Alternatively, the contents of this file may be used under the terms of
# either the GNU General Public License Version 3 or later (the "GPLv3+"), or
# the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
# in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
# instead of those above.

$(eval $(call gb_Package_Package,sm_uiconfig,$(realpath $(SRCDIR)/starmath/uiconfig)))
$(eval $(call gb_Package_add_file,sm_uiconfig,xml/uiconfig/modules/smath/menubar/menubar.xml,smath/menubar/menubar.xml))
$(eval $(call gb_Package_add_file,sm_uiconfig,xml/uiconfig/modules/smath/statusbar/statusbar.xml,smath/statusbar/statusbar.xml))
$(eval $(call gb_Package_add_file,sm_uiconfig,xml/uiconfig/modules/smath/toolbar/toolbar.xml,smath/toolbar/toolbar.xml))
$(eval $(call gb_Package_add_file,sm_uiconfig,xml/uiconfig/modules/smath/toolbar/standardbar.xml,smath/toolbar/standardbar.xml))
$(eval $(call gb_Package_add_file,sm_uiconfig,xml/uiconfig/modules/smath/toolbar/fullscreenbar.xml,smath/toolbar/fullscreenbar.xml))

