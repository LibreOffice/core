package complex.embedding;

import com.sun.star.uno.XInterface;
import com.sun.star.lang.XMultiServiceFactory;
import com.sun.star.lang.XSingleServiceFactory;

import com.sun.star.bridge.XUnoUrlResolver;
import com.sun.star.uno.UnoRuntime;
import com.sun.star.uno.XInterface;
import com.sun.star.beans.XPropertySet;
import com.sun.star.frame.XLoadable;
import com.sun.star.drawing.XDrawPagesSupplier;
import com.sun.star.drawing.XDrawPages;
import com.sun.star.drawing.XDrawPage;
import com.sun.star.drawing.XShape;
import com.sun.star.graphic.XGraphic;

import com.sun.star.embed.*;

import share.LogWriter;
import complex.embedding.TestHelper;
import complex.embedding.EmbeddingTest;

public class Test01 implements EmbeddingTest {

    XMultiServiceFactory m_xMSF;
    TestHelper m_aTestHelper;

    public Test01( XMultiServiceFactory xMSF, LogWriter aLogWriter )
    {
        m_xMSF = xMSF;
        m_aTestHelper = new TestHelper( aLogWriter, "Test01: " );
    }

    public boolean test()
    {
        try
        {
            Object oDoc = m_xMSF.createInstance( "com.sun.star.comp.Draw.DrawingDocument" );
            XLoadable xLoad = (XLoadable) UnoRuntime.queryInterface( XLoadable.class, oDoc );
            if ( xLoad == null )
            {
                m_aTestHelper.Error( "Can not get XLoadable!" );
                return false;
            }

            xLoad.initNew();

            XDrawPagesSupplier xDPSupply = (XDrawPagesSupplier) UnoRuntime.queryInterface( XDrawPagesSupplier.class, oDoc );
            if ( xDPSupply == null )
            {
                m_aTestHelper.Error( "Can not get XDrawPagesSupplier!" );
                return false;
            }

            XDrawPages xDrawPages = xDPSupply.getDrawPages();
            if ( xDrawPages == null )
            {
                m_aTestHelper.Error( "Can not get XDrawPages object!" );
                return false;
            }

            if ( xDrawPages.getCount() == 0 )
            {
                m_aTestHelper.Error( "There must be at least one page in the document!" );
                return false;
            }

            Object oPage = xDrawPages.getByIndex( 0 );
            XDrawPage xPage = (XDrawPage) UnoRuntime.queryInterface( XDrawPage.class, oPage );
            if ( xPage == null )
            {
                m_aTestHelper.Error( "Can not get access to drawing page!" );
                return false;
            }

            XMultiServiceFactory xDrFactory = ( XMultiServiceFactory ) UnoRuntime.queryInterface( XMultiServiceFactory.class,
                                                                                                  oDoc );
            if ( xDrFactory == null )
            {
                m_aTestHelper.Error( "Can not get drawing factory!" );
                return false;
            }

            Object oShape = xDrFactory.createInstance( "com.sun.star.drawing.OLE2Shape" );
            XShape xShape = ( XShape ) UnoRuntime.queryInterface( XShape.class, oShape );
            if ( xShape == null )
            {
                m_aTestHelper.Error( "Can not create new shape!" );
                return false;
            }

            XPropertySet xShapeProps = ( XPropertySet ) UnoRuntime.queryInterface( XPropertySet.class, oShape );
            if ( xShapeProps == null )
            {
                m_aTestHelper.Error( "Can not get access to shapes properties!" );
                return false;
            }

            xPage.add( xShape );
            xShapeProps.setPropertyValue( "CLSID", "078B7ABA-54FC-457F-8551-6147e776a997" );

            Object oEmbObj = xShapeProps.getPropertyValue( "EmbeddedObject" );
            XEmbeddedObject xEmbObj = ( XEmbeddedObject ) UnoRuntime.queryInterface( XEmbeddedObject.class, oEmbObj );
            if ( xEmbObj == null )
            {
                m_aTestHelper.Error( "Embedded object can not be accessed!" );
                return false;
            }

            XEmbeddedClient xClient = xEmbObj.getClientSite();
            if ( xClient == null )
            {
                m_aTestHelper.Error( "The client for the object must be set!" );
                return false;
            }

            Object oReplacement = xShapeProps.getPropertyValue( "Graphic" );
            XGraphic xReplGraph = ( XGraphic ) UnoRuntime.queryInterface( XGraphic.class, oReplacement );
            if ( xReplGraph == null )
            {
                m_aTestHelper.Error( "The replacement graphic should be available!" );
                return false;
            }

            return true;
        }
        catch( Exception e )
        {
            m_aTestHelper.Error( "Exception: " + e );
            return false;
        }
    }
}

