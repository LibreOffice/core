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

import com.sun.star.uno.UnoRuntime;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.AccessibleRelation;
import com.sun.star.accessibility.XAccessibleRelationSet;
import com.sun.star.lang.IndexOutOfBoundsException;

import tools.NameProvider;

class AccessibleRelationHandler
    extends NodeHandler
{
    public NodeHandler createHandler( XAccessibleContext xContext )
    {
        AccessibleRelationHandler aHandler = null;
        if (xContext != null)
        {
            XAccessibleRelationSet xRelation = xContext.getAccessibleRelationSet();
            if (xRelation != null)
                aHandler = new AccessibleRelationHandler(xContext);
        }
        return aHandler;
    }

    public AccessibleRelationHandler()
    {
    }

    public AccessibleRelationHandler( XAccessibleContext xContext )
    {
        XAccessibleRelationSet xRelation = xContext.getAccessibleRelationSet();
        if (xRelation != null)
            maChildList.setSize( 1 );
    }

    public AccessibleTreeNode createChild( AccessibleTreeNode aParent,
                                           int nIndex )
    {
        XAccessibleRelationSet xRelation = null;
        AccessibleTreeNode aChild = null;

        if( aParent instanceof AccTreeNode )
        {
            xRelation =
                ((AccTreeNode)aParent).getContext().getAccessibleRelationSet();
        }
        if( xRelation == null )
            return aChild;


        VectorNode aVNode = new VectorNode( "RelationSet", aParent);
        int nCount = xRelation.getRelationCount();
        try
        {
            for( int i = 0; i < nCount; i++ )
            {
                AccessibleRelation aRelation = xRelation.getRelation( i );

                StringBuffer aBuffer = new StringBuffer();
                aBuffer.append (NameProvider.getRelationName (aRelation.RelationType));
                aBuffer.append( ": " );

                for( int j = 0; j < aRelation.TargetSet.length; j++ )
                {
                    Object aTarget = aRelation.TargetSet[j];
                    XAccessible xAccTarget =
                        (XAccessible)UnoRuntime.queryInterface(
                             XAccessible.class, aTarget );
                    if( xAccTarget == null )
                    {
                        aBuffer.append( aTarget.toString() );
                    }
                    else
                    {
                        aBuffer.append( xAccTarget.getAccessibleContext().
                                         getAccessibleName() );
                    }
                    aBuffer.append( ", " );
                }
                aBuffer.delete( aBuffer.length() - 2, aBuffer.length() );

                aVNode.addChild( new StringNode( aBuffer.toString(),
                                                 aParent ) );
            }

            aChild = aVNode;
        }
        catch( IndexOutOfBoundsException e )
        {
            aChild = new StringNode( "IndexOutOfBounds", aParent );
        }

        return aChild;
    }
}
