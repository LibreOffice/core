
import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.AccessibleRelation;
import drafts.com.sun.star.accessibility.XAccessibleRelationSet;
import drafts.com.sun.star.accessibility.AccessibleRelationType;
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
                switch( aRelation.RelationType )
                {
                    case AccessibleRelationType.INVALID:
                        aBuffer.append( "INVALID" );
                        break;
                    case AccessibleRelationType.CONTROLLED_BY:
                        aBuffer.append( "CONTROLLED_BY" );
                        break;
                    case AccessibleRelationType.CONTROLLED_BY_PROPERTY:
                        aBuffer.append( "CONTROLLED_BY_PROPERTY" );
                        break;
                    case AccessibleRelationType.CONTROLLER_FOR:
                        aBuffer.append( "CONTROLLER_FOR" );
                        break;
                    case AccessibleRelationType.CONTROLLER_FOR_PROPERTY:
                        aBuffer.append( "CONTROLLER_FOR_PROPERTY" );
                        break;
                    case AccessibleRelationType.LABEL_FOR:
                        aBuffer.append( "LABEL_FOR" );
                        break;
                    case AccessibleRelationType.LABEL_FOR_PROPERTY:
                        aBuffer.append( "LABEL_FOR_PROPERTY" );
                        break;
                    case AccessibleRelationType.LABELED_BY:
                        aBuffer.append( "LABELED_BY" );
                        break;
                    case AccessibleRelationType.LABELED_BY_PROPERTY:
                        aBuffer.append( "LABELED_BY_PROPERTY" );
                        break;
                    case AccessibleRelationType.MEMBER_OF:
                        aBuffer.append( "MEMBER_OF" );
                        break;
                    case AccessibleRelationType.MEMBER_OF_PROPERTY:
                        aBuffer.append( "MEMBER_OF_PROPERTY" );
                        break;
                    default:
                        aBuffer.append( aRelation.RelationType );
                        break;
                }
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
