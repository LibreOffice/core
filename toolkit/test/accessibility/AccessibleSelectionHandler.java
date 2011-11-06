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


import com.sun.star.uno.UnoRuntime;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.XAccessibleSelection;
import com.sun.star.lang.IndexOutOfBoundsException;

import javax.swing.*;
import java.awt.*;
import java.util.Vector;
import java.awt.event.ActionListener;
import java.awt.event.ActionEvent;



class AccessibleSelectionHandler
    extends NodeHandler
{
    public NodeHandler createHandler( XAccessibleContext xContext )
    {
        XAccessibleSelection xSelection =
            (XAccessibleSelection) UnoRuntime.queryInterface(
                XAccessibleSelection.class, xContext);
        return (xSelection == null) ? null :
            new AccessibleSelectionHandler(xSelection);
    }

    public AccessibleSelectionHandler()
    {
    }

    public AccessibleSelectionHandler( XAccessibleSelection xSelection )
    {
        if (xSelection != null)
            maChildList.setSize( 2 );
    }

    public AccessibleTreeNode createChild( AccessibleTreeNode aParent,
                                           int nIndex )
    {
        AccessibleTreeNode aChild = null;

        if( aParent instanceof AccTreeNode )
        {
            XAccessibleSelection xSelection =
                ((AccTreeNode)aParent).getSelection();
            if( xSelection != null )
            {
                switch( nIndex )
                {
                    case 0:
                        aChild = new StringNode(
                            "getSelectedAccessibleChildCount: " +
                            xSelection.getSelectedAccessibleChildCount(),
                            aParent );
                        break;
                    case 1:
                    {
                        VectorNode aVNode =
                            new VectorNode( "Selected Children", aParent);
                        int nSelected = 0;
                        int nCount = ((AccTreeNode)aParent).getContext().
                            getAccessibleChildCount();
                        try
                        {
                            for( int i = 0; i < nCount; i++ )
                            {
                                try
                                {
                                    if( xSelection.isAccessibleChildSelected( i ) )
                                    {
                                        XAccessible xSelChild = xSelection.
                                            getSelectedAccessibleChild(nSelected);
                                        XAccessible xNChild =
                                            ((AccTreeNode)aParent).
                                            getContext().getAccessibleChild( i );
                                        aVNode.addChild( new StringNode(
                                            i + ": " +
                                            xNChild.getAccessibleContext().
                                            getAccessibleDescription() + " (" +
                                            (xSelChild.equals(xNChild) ? "OK" : "XXX") +
                                            ")", aParent ) );
                                    }
                                }
                                catch (com.sun.star.lang.DisposedException e)
                                {
                                    aVNode.addChild( new StringNode(
                                        i + ": caught DisposedException while creating",
                                        aParent ));
                                }
                            }
                            aChild = aVNode;
                        }
                        catch( IndexOutOfBoundsException e )
                        {
                            aChild = new StringNode( "IndexOutOfBounds",
                                                     aParent );
                        }
                    }
                    break;
                    default:
                        aChild = new StringNode( "ERROR", aParent );
                        break;
                }
            }
        }

        return aChild;
    }


    public String[] getActions (AccessibleTreeNode aNode)
    {
        if( aNode instanceof AccTreeNode )
        {
            XAccessibleSelection xSelection =
                ((AccTreeNode)aNode).getSelection();
            if( xSelection != null )
            {
                return new String[] { "Select..." };
            }
        }
        return new String[0];
    }

    public void performAction (AccessibleTreeNode aNode, int nIndex)
    {
        new SelectionDialog( (AccTreeNode)aNode ).show();
    }
}
