import com.sun.star.uno.UnoRuntime;
import com.sun.star.accessibility.XAccessible;
import com.sun.star.accessibility.XAccessibleContext;
import com.sun.star.accessibility.AccessibleRelation;
import com.sun.star.accessibility.XAccessibleRelationSet;
import com.sun.star.accessibility.AccessibleRelationType;
import com.sun.star.lang.IndexOutOfBoundsException;


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
