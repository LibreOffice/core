#
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
# 	Peter Foley <pefoley2@verizon.net>
# Portions created by the Initial Developer are Copyright (C) 2011 the
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
#

$(eval $(call gb_Package_Package,testautomation_win,$(SRCDIR)/testautomation/global/win))

$(eval $(call gb_Package_add_file,testautomation_win,bin/win/bars.win,bars.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/dial_a_c.win,dial_a_c.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/dial_d_h.win,dial_d_h.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/dial_i_o.win,dial_i_o.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/dial_p_s.win,dial_p_s.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/dial_t_z.win,dial_t_z.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/dokument.win,dokument.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/e_mathop.win,e_mathop.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/edia_a_c.win,edia_a_c.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/edia_d_h.win,edia_d_h.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/edia_i_o.win,edia_i_o.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/edia_p_s.win,edia_p_s.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/edia_t_z.win,edia_t_z.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/etab_a_d.win,etab_a_d.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/etab_e_g.win,etab_e_g.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/etab_h_o.win,etab_h_o.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/etab_p_s.win,etab_p_s.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/etab_t_z.win,etab_t_z.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/etoolbox.win,etoolbox.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/mathop.win,mathop.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/piloten.win,piloten.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/reportdesigner.win,reportdesigner.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/spadmin.win,spadmin.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/sys_dial.win,sys_dial.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/tab_a_d.win,tab_a_d.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/tab_e_g.win,tab_e_g.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/tab_h_o.win,tab_h_o.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/tab_p_s.win,tab_p_s.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/tab_t_z.win,tab_t_z.win))
$(eval $(call gb_Package_add_file,testautomation_win,bin/win/w_autop.win,w_autop.win))
