/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

import javax.swing.JOptionPane;

/**
    Base class for all tree nodes.
 */
class AccessibleActionNode
    extends StringNode
{
    public AccessibleActionNode (String aDisplayObject,
        AccessibleTreeNode aParent,
        int nActionIndex)
    {
        super (aDisplayObject, aParent);
        mnActionIndex = nActionIndex;
    }

    public String[] getActions ()
    {
        return new String[] {"Perform Action"};
    }

    /** perform action */
    public void performAction (int nIndex)
    {
        if (nIndex != 0)
            return;
        boolean bResult = false;
        if (getParent() instanceof AccTreeNode)
            try
            {
                bResult = AccessibleActionHandler.getAction(
                    (AccTreeNode)getParent()).doAccessibleAction (
                        mnActionIndex);
            }
        catch (com.sun.star.lang.IndexOutOfBoundsException e)
        {
        }

        JOptionPane.showMessageDialog (null,
            "performed action " + mnActionIndex
            + (bResult?" with":" without") + " success",
            "Action " + mnActionIndex,
            JOptionPane.INFORMATION_MESSAGE);
    }

    private int mnActionIndex;
}
