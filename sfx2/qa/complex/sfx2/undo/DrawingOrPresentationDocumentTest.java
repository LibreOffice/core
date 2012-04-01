/*
 * To change this template, choose Tools | Templates
 * and open the template in the editor.
 */

package complex.sfx2.undo;

import com.sun.star.awt.Rectangle;
import com.sun.star.document.XUndoManager;
import com.sun.star.document.XUndoManagerSupplier;
import com.sun.star.document.XUndoAction;
import com.sun.star.awt.Point;
import com.sun.star.awt.Size;
import com.sun.star.beans.XPropertySet;
import com.sun.star.drawing.CircleKind;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XShape;
import com.sun.star.drawing.XShapes;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.uno.UnoRuntime;
import org.openoffice.test.tools.DocumentType;
import static org.junit.Assert.*;

/**
 * implements the {@link DocumentTest} interface on top of a drawing document
 * @author frank.schoenheit@oracle.com
 */
public abstract class DrawingOrPresentationDocumentTest extends DocumentTestBase
{
    public DrawingOrPresentationDocumentTest( XMultiServiceFactory i_orb, final DocumentType i_docType ) throws com.sun.star.uno.Exception
    {
        super( i_orb, i_docType );
    }

    public void initializeDocument() throws com.sun.star.uno.Exception
    {
        // remove all shapes - Impress has two default shapes in a new doc; just get rid of them
        final XShapes firstPageShapes = getFirstPageShapes();
        while ( firstPageShapes.getCount() > 0 )
            firstPageShapes.remove( UnoRuntime.queryInterface( XShape.class, firstPageShapes.getByIndex( 0 ) ) );
    }

    public void doSingleModification() throws com.sun.star.uno.Exception
    {
        // add a simple centered shape to the first page
        Rectangle pagePlayground = impl_getFirstPagePlayground();
        impl_createCircleShape(
            ( pagePlayground.X + ( pagePlayground.Width - BIG_CIRCLE_SIZE ) / 2 ),
            ( pagePlayground.Y + ( pagePlayground.Height - BIG_CIRCLE_SIZE ) / 2 ),
            BIG_CIRCLE_SIZE,
            FILL_COLOR
        );
    }

    public void verifyInitialDocumentState() throws com.sun.star.uno.Exception
    {
        final XShapes firstPageShapes = getFirstPageShapes();
        assertEquals( "there should be no shapes at all", 0, firstPageShapes.getCount() );
    }

    public void verifySingleModificationDocumentState() throws com.sun.star.uno.Exception
    {
        final XShapes firstPageShapes = getFirstPageShapes();
        assertEquals( "there should be one shape, not more, not less", 1, firstPageShapes.getCount() );

        final Object shape = firstPageShapes.getByIndex(0);
        verifyShapeGeometry( shape, BIG_CIRCLE_SIZE, BIG_CIRCLE_SIZE );
        final XPropertySet shapeProps = UnoRuntime.queryInterface( XPropertySet.class, shape );
        assertEquals( "wrong circle tpye", CIRCLE_TYPE.getValue(), ((CircleKind)shapeProps.getPropertyValue( "CircleKind" )).getValue() );
        //assertEquals( "wrong circle fill color", FILL_COLOR, ((Integer)shapeProps.getPropertyValue( "FillColor" )).intValue() );
            // disable this particular check: A bug in the drawing layer API restores the FillColor to its
            // default value upon re-insertion. This is issue #i115080#
    }

    public int doMultipleModifications() throws com.sun.star.uno.Exception
    {
        // add a simple centered shape to the first page
        Rectangle pagePlayground = impl_getFirstPagePlayground();
        impl_createCircleShape(
            pagePlayground.X,
            pagePlayground.Y,
            SMALL_CIRCLE_SIZE,
            ALTERNATE_FILL_COLOR
        );
        impl_createCircleShape(
            pagePlayground.X + pagePlayground.Width - SMALL_CIRCLE_SIZE,
            pagePlayground.Y,
            SMALL_CIRCLE_SIZE,
            ALTERNATE_FILL_COLOR
        );
        impl_createCircleShape(
            pagePlayground.X,
            pagePlayground.Y + pagePlayground.Height - SMALL_CIRCLE_SIZE,
            SMALL_CIRCLE_SIZE,
            ALTERNATE_FILL_COLOR
        );
        impl_createCircleShape(
            pagePlayground.X + pagePlayground.Width - SMALL_CIRCLE_SIZE,
            pagePlayground.Y + pagePlayground.Height - SMALL_CIRCLE_SIZE,
            SMALL_CIRCLE_SIZE,
            ALTERNATE_FILL_COLOR
        );
        return 4;
    }

    private void impl_createCircleShape( final int i_x, final int i_y, final int i_size, final int i_color ) throws com.sun.star.uno.Exception
    {
        final XPropertySet shapeProps = getDocument().createInstance( "com.sun.star.drawing.EllipseShape", XPropertySet.class );
        shapeProps.setPropertyValue( "CircleKind", CIRCLE_TYPE );
        shapeProps.setPropertyValue( "FillColor", i_color );

        final XShape shape = UnoRuntime.queryInterface( XShape.class, shapeProps );
        final Size shapeSize = new Size( i_size, i_size );
        shape.setSize( shapeSize );
        final Point shapePos = new Point( i_x, i_y );
        shape.setPosition( shapePos );

        final XShapes pageShapes = UnoRuntime.queryInterface( XShapes.class, getFirstPageShapes() );
        pageShapes.add( shape );

        // Sadly, Draw/Impress currently do not create Undo actions for programmatic changes to the document.
        // Which renders the test here slightly useless ... unless we fake the Undo actions ourself.
        final XUndoManagerSupplier suppUndoManager = UnoRuntime.queryInterface( XUndoManagerSupplier.class, getDocument().getDocument() );
        final XUndoManager undoManager = suppUndoManager.getUndoManager();
        undoManager.addUndoAction( new ShapeInsertionUndoAction( shape, pageShapes ) );
    }

    private Rectangle impl_getFirstPagePlayground() throws com.sun.star.uno.Exception
    {
        final XShapes firstPageShapes = getFirstPageShapes();
        final XPropertySet firstPageProps = UnoRuntime.queryInterface( XPropertySet.class, firstPageShapes );
        final int pageWidth = ((Integer)firstPageProps.getPropertyValue( "Width" )).intValue();
        final int pageHeight = ((Integer)firstPageProps.getPropertyValue( "Height" )).intValue();
        final int borderLeft = ((Integer)firstPageProps.getPropertyValue( "BorderLeft" )).intValue();
        final int borderTop = ((Integer)firstPageProps.getPropertyValue( "BorderTop" )).intValue();
        final int borderRight = ((Integer)firstPageProps.getPropertyValue( "BorderRight" )).intValue();
        final int borderBottom = ((Integer)firstPageProps.getPropertyValue( "BorderBottom" )).intValue();
        return new Rectangle( borderLeft, borderTop, pageWidth - borderLeft - borderRight, pageHeight - borderTop - borderBottom );
    }

    /**
     * returns the XShapes interface of the first page of our drawing document
     */
    private XShapes getFirstPageShapes() throws com.sun.star.uno.Exception
    {
        final XDrawPagesSupplier suppPages = UnoRuntime.queryInterface( XDrawPagesSupplier.class, getDocument().getDocument() );
        final XDrawPages pages = suppPages.getDrawPages();
        return UnoRuntime.queryInterface( XShapes.class, pages.getByIndex( 0 ) );
    }

    /**
     * verifies the given shape has the given size
     */
    private void verifyShapeGeometry( final Object i_shapeObject, final int i_expectedWidth, final int i_expectedHeight )
         throws com.sun.star.uno.Exception
    {
        final XShape shape = UnoRuntime.queryInterface( XShape.class, i_shapeObject );
        final Size shapeSize = shape.getSize();
        assertEquals( "unexpected shape width", i_expectedWidth, shapeSize.Width );
        assertEquals( "unexpected shape height", i_expectedHeight, shapeSize.Height );
    }

    private static class ShapeInsertionUndoAction implements XUndoAction
    {
        ShapeInsertionUndoAction( final XShape i_shape, final XShapes i_shapeCollection )
        {
            m_shape = i_shape;
            m_shapeCollection = i_shapeCollection;
        }

        public String getTitle()
        {
            return "insert shape";
        }

        public void undo()
        {
            m_shapeCollection.remove( m_shape );
        }

        public void redo()
        {
            m_shapeCollection.add( m_shape );
        }

        private final XShape m_shape;
        private final XShapes m_shapeCollection;
    }

    private static CircleKind   CIRCLE_TYPE = CircleKind.FULL;
    private static int          FILL_COLOR = 0xCC2244;
    private static int          ALTERNATE_FILL_COLOR = 0x44CC22;
    private static int          BIG_CIRCLE_SIZE = 5000;
    private static int          SMALL_CIRCLE_SIZE = 2000;
}
