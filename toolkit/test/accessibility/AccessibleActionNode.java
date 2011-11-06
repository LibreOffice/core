/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/

import javax.swing.JOptionPane;
import com.sun.star.accessibility.XAccessibleAction;

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
