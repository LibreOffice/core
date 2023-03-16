# -*- tab-width: 4; indent-tabs-mode: nil; py-indent-offset: 4 -*-
#
# This Source Code Form is subject to the terms of the Mozilla Public
# License, v. 2.0. If a copy of the MPL was not distributed with this
# file, You can obtain one at http://mozilla.org/MPL/2.0/.
#
from uitest.framework import UITestCase
from uitest.uihelper.common import get_state_as_dict
from uitest.uihelper.common import select_pos
from libreoffice.uno.propertyvalue import mkPropertyValues
#Bug 152722 - NatNum12 number format (spell out): add/delete decimal places is losing format

class tdf152722(UITestCase):
    def test_tdf152722_NatNum_modifier_decimal_value(self):
        #numberingformatpage.ui
        with self.ui_test.create_doc_in_start_center("calc"):
            #format - cell
            with self.ui_test.execute_dialog_through_command(".uno:FormatCellDialog") as xDialog:
                xTabs = xDialog.getChild("tabcontrol")
                select_pos(xTabs, "0")  #tab Numbers
                xdecimalsed  = xDialog.getChild( "decimalsed" )
                xleadzerosed = xDialog.getChild( "leadzerosed" )
                xnegnumred   = xDialog.getChild( "negnumred" )
                xthousands   = xDialog.getChild( "thousands" )
                xformatted   = xDialog.getChild( "formatted" )

                xformatted.executeAction( "CLEAR", tuple() )  #clear textbox
                xformatted.executeAction( "TYPE", mkPropertyValues({"TEXT":"[NatNum12 cardinal]0"}) )
                xdecimalsed.executeAction( "UP", tuple() )
                self.assertEqual( get_state_as_dict(xformatted)["Text"], "[NatNum12 cardinal]0.0" )
                xnegnumred.executeAction( "CLICK", tuple() )
                self.assertEqual( get_state_as_dict(xformatted)["Text"], "[NatNum12 cardinal]0.0;[RED][NatNum12 cardinal]-0.0" )
                #Bug 153023: disable Thousand separator for NatNum12 modifier
                self.assertEqual(get_state_as_dict(xthousands)["Enabled"], "false")
                #Leading zeroes can be 0 or 1
                self.assertEqual(get_state_as_dict(xleadzerosed)["Enabled"], "true")

# vim: set shiftwidth=4 softtabstop=4 expandtab:
