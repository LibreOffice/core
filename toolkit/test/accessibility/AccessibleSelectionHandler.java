
import com.sun.star.uno.UnoRuntime;
import drafts.com.sun.star.accessibility.XAccessible;
import drafts.com.sun.star.accessibility.XAccessibleContext;
import drafts.com.sun.star.accessibility.XAccessibleSelection;
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


//     virtual void SAL_CALL selectAccessibleChild(
//         sal_Int32 nChildIndex )
//         throw ( ::com::sun::star::lang::IndexOutOfBoundsException,
//                 ::com::sun::star::uno::RuntimeException );
//     virtual void SAL_CALL clearAccessibleSelection(  )
//         throw ( ::com::sun::star::uno::RuntimeException );
//     virtual void SAL_CALL selectAllAccessible(  )
//         throw ( ::com::sun::star::uno::RuntimeException );
//         throw ( ::com::sun::star::uno::RuntimeException );
//     virtual void SAL_CALL deselectSelectedAccessibleChild(



/**
 * Display a dialog with a list-box of children and select/deselect buttons
 */
class SelectionDialog extends JDialog
    implements ActionListener
{
    JList aChildren;
    XAccessibleSelection xSelection;

    public SelectionDialog( AccTreeNode aNode )
    {
        super( AccessibilityWorkBench.get() );

        xSelection = aNode.getSelection();

        XAccessibleContext xContext = aNode.getContext();
        int nCount = xContext.getAccessibleChildCount();
        Vector aChildVector = new Vector();
        for(int i = 0; i < nCount; i++)
        {
            try
            {
                aChildVector.add(
                    xContext.getAccessibleChild(i).
                    getAccessibleContext().getAccessibleDescription() );
            }
            catch( IndexOutOfBoundsException e )
            {
                aChildVector.add( "ERROR: IndexOutOfBoundsException" );
            }
        }

        aChildren = new JList( aChildVector );

        init();
    }

    /** build dialog */
    protected void init()
    {
        setTitle( "Select" );

        // vertical stacking of the elements
        Container aContent = getContentPane();

        // label with explanation
        aContent.add( new JLabel( "Select/Deselect child elements" ),
                      BorderLayout.NORTH );

        // the JListBox
        aContent.add( aChildren, BorderLayout.CENTER );
        aChildren.setSelectionMode( ListSelectionModel.SINGLE_SELECTION );

        JPanel aButtons = new JPanel();
        aButtons.setLayout( new FlowLayout() );

        JButton aButton;

        aButton = new JButton( "Select" );
        aButton.setActionCommand( "Select" );
        aButton.addActionListener( this );
        aButtons.add( aButton );

        aButton = new JButton( "Deselect" );
        aButton.setActionCommand( "Deselect" );
        aButton.addActionListener( this );
        aButtons.add( aButton );

        aButton = new JButton( "Select all" );
        aButton.setActionCommand( "Select all" );
        aButton.addActionListener( this );
        aButtons.add( aButton );

        aButton = new JButton( "Clear Selection" );
        aButton.setActionCommand( "Clear Selection" );
        aButton.addActionListener( this );
        aButtons.add( aButton );

        aButton = new JButton( "Cancel" );
        aButton.setActionCommand( "Cancel" );
        aButton.addActionListener( this );
        aButtons.add( aButton );

        // add Panel with buttons
        aContent.add( aButtons, BorderLayout.SOUTH );

        setSize( getPreferredSize() );
    }

    void cancel()
    {
        hide();
        dispose();
    }

    void select()
    {
        try
        {
            xSelection.selectAccessibleChild( aChildren.getSelectedIndex() );
        }
        catch( IndexOutOfBoundsException e )
        {
            JOptionPane.showMessageDialog( AccessibilityWorkBench.get(),
                                           "Can't select: IndexOutofBounds",
                                           "Error in selectAccessibleChild",
                                           JOptionPane.ERROR_MESSAGE);
        }
    }

    void deselect()
    {
        try
        {
            xSelection.deselectSelectedAccessibleChild(
                aChildren.getSelectedIndex() );
        }
        catch( IndexOutOfBoundsException e )
        {
            JOptionPane.showMessageDialog( AccessibilityWorkBench.get(),
                                           "Can't deselect: IndexOutofBounds",
                                           "Error in deselectAccessibleChild",
                                           JOptionPane.ERROR_MESSAGE);
        }
    }

    void selectAll()
    {
        xSelection.selectAllAccessible();
    }

    void clearSelection()
    {
        xSelection.clearAccessibleSelection();
    }



    public void actionPerformed(ActionEvent e)
    {
        String sCommand = e.getActionCommand();

        if( "Cancel".equals( sCommand ) )
            cancel();
        else if ( "Select".equals( sCommand ) )
            select();
        else if ( "Deselect".equals( sCommand ) )
            deselect();
        else if ( "Clear Selection".equals( sCommand ) )
            clearSelection();
        else if ( "Select all".equals( sCommand ) )
            selectAll();
    }
}
