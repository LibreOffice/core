#
#
# This file is part of the LibreOffice project.
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
#

$(eval $(call gb_Package_Package,MathMLDTD_bin,$(SRCDIR)/MathMLDTD))

$(eval $(call gb_Package_add_file,MathMLDTD_bin,bin/math.dtd,math.dtd))
$(eval $(call gb_Package_add_file,MathMLDTD_bin,bin/w3c_ipr_software_notice.html,w3c_ipr_software_notice.html))

