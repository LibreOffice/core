/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: views.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 16:09:23 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#include <cppunit/simpleheader.hxx>
#include <cppuhelper/compbase1.hxx>
#include <comphelper/broadcasthelper.hxx>

#include "unoview.hxx"
#include "unoviewcontainer.hxx"
#include "shape.hxx"
#include "layermanager.hxx"
#include "com/sun/star/presentation/XSlideShowView.hpp"

namespace target = slideshow::internal;
using namespace ::com::sun::star;

namespace
{

class UnoViewContainerTest : public CppUnit::TestFixture
{
public:
    // our test view subject
    class TestUnoView : public target::UnoView
    {
        bool mbDisposed;

    public:
        TestUnoView() : mbDisposed( false )
        {
        }

        virtual ~TestUnoView()
        {
        }

        virtual ::cppcanvas::CanvasSharedPtr getCanvas() const
        {
            return ::cppcanvas::CanvasSharedPtr();
        }

        virtual ::cppcanvas::CustomSpriteSharedPtr createSprite( const ::basegfx::B2DSize& rSpriteSizePixel ) const
        {
            return ::cppcanvas::CustomSpriteSharedPtr();
        }

        virtual double getSpritePriority( double nSpritePrio ) const
        {
            return 0.0;
        }

        virtual void setPriority( double nPrio )
        {
        }

        virtual target::ViewLayerSharedPtr createViewLayer() const
        {
            return target::ViewLayerSharedPtr();
        }

        virtual void clear() const
        {
        }

        virtual bool isContentDestroyed() const
        {
            return true;
        }

        virtual bool updateScreen() const
        {
            return true;
        }

        virtual ::basegfx::B2DHomMatrix getTransformation() const
        {
            return ::basegfx::B2DHomMatrix();
        }

        virtual void setViewSize( const ::basegfx::B2DSize& )
        {
        }

        virtual void setClip( const ::basegfx::B2DPolyPolygon& rClip )
        {
        }

        virtual void setMouseCursor( sal_Int16 nPointerShape )
        {
        }

        virtual uno::Reference< presentation::XSlideShowView > getUnoView() const
        {
            return uno::Reference< presentation::XSlideShowView >();
        }

        virtual void _dispose()
        {
            mbDisposed = true;
        }

        bool isDisposed() { return mbDisposed; }
    };

    // our test shape subject
    typedef ::cppu::WeakComponentImplHelper1< drawing::XShape > ShapeBase;
    class TestShape : public target::Shape, public comphelper::OBaseMutex, public ShapeBase
    {
        const basegfx::B2DRectangle maRect;
        const double                mnPrio;

        virtual ::rtl::OUString SAL_CALL getShapeType(  ) throw (uno::RuntimeException)
        {
            return ::rtl::OUString();
        }

        virtual awt::Point SAL_CALL getPosition(  ) throw (uno::RuntimeException)
        {
            return awt::Point();
        }

        virtual void SAL_CALL setPosition( const awt::Point& aPosition ) throw (uno::RuntimeException)
        {
        }

        virtual awt::Size SAL_CALL getSize(  ) throw (uno::RuntimeException)
        {
            return awt::Size();
        }

        virtual void SAL_CALL setSize( const awt::Size& aSize ) throw (beans::PropertyVetoException, uno::RuntimeException)
        {
        }

    public:
        TestShape( const basegfx::B2DRectangle& rRect,
                   double                       nPrio ) :
            ShapeBase( m_aMutex ),
            maRect( rRect ),
            mnPrio( nPrio )
        {}

        virtual uno::Reference< drawing::XShape > getXShape() const
        {
            return uno::Reference< drawing::XShape >( const_cast<TestShape*>(this) );
        }

        virtual void addViewLayer( const target::ViewLayerSharedPtr&    rNewLayer,
                                   bool                                 bRedrawLayer )
        {
        }
        virtual bool removeViewLayer( const target::ViewLayerSharedPtr& rNewLayer )
        {
            return true;
        }
        virtual bool clearAllViewLayers()
        {
            return true;
        }

        virtual bool update() const
        {
            return true;
        }
        virtual bool render() const
        {
            return true;
        }
        virtual bool isUpdateNecessary() const
        {
            return true;
        }

        virtual ::basegfx::B2DRectangle getPosSize() const
        {
            return maRect;
        }
        virtual ::basegfx::B2DRectangle getDOMBounds() const
        {
            return maRect;
        }
        virtual ::basegfx::B2DRectangle getUpdateArea() const
        {
            return maRect;
        }

        virtual bool isVisible() const
        {
            return true;
        }
        virtual double getPriority() const
        {
            return mnPrio;
        }
        virtual bool isBackgroundDetached() const
        {
            return false;
        }
        virtual bool hasIntrinsicAnimation() const
        {
            return false;
        }
        virtual bool hasHyperlinks() const
        {
            return false;
        }
        virtual HyperLinkRegions getHyperlinkRegions() const
        {
            return HyperLinkRegions();
        }
    };

    void testContainer()
    {
        target::UnoViewContainer aContainer;

        boost::shared_ptr<TestUnoView> pView( new TestUnoView() );
        aContainer.addView( pView );

        CPPUNIT_ASSERT_MESSAGE( "Testing container size",
                                1 == std::distance( aContainer.begin(),
                                                    aContainer.end() ));
        aContainer.dispose();
        CPPUNIT_ASSERT_MESSAGE( "Testing dispose: container must be empty",
                                0 == std::distance( aContainer.begin(),
                                                    aContainer.end() ));
        CPPUNIT_ASSERT_MESSAGE( "Testing dispose: all elements must receive dispose",
                                pView->isDisposed() );
    }

    void testLayerManager()
    {
        basegfx::B2DRectangle aRect(0,0,1600,1200);
        const basegfx::B2DRectangle aRect2( aRect );
        target::LayerManager aLayerManager( aRect );
        boost::shared_ptr<TestUnoView> pView( new TestUnoView() );

        aRect.reset();

        CPPUNIT_ASSERT_MESSAGE( "Testing page bounds",
                                aRect2 == aLayerManager.getPageBounds());

        aLayerManager.addView( pView );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Testing view removal",
                                      true,
                                      aLayerManager.removeView( pView ));

        boost::shared_ptr<TestShape> pShape( new TestShape(basegfx::B2DRectangle(10,10,100,100),
                                                           1.0) );
        aLayerManager.addShape( pShape );
        CPPUNIT_ASSERT_EQUAL_MESSAGE( "Testing shape removal",
                                      true,
                                      aLayerManager.removeShape( pShape ));
    }

    // hook up the test
    CPPUNIT_TEST_SUITE(UnoViewContainerTest);
    CPPUNIT_TEST(testContainer);
    CPPUNIT_TEST(testLayerManager);
    CPPUNIT_TEST_SUITE_END();

}; // class UnoViewContainerTest

// -----------------------------------------------------------------------------
CPPUNIT_TEST_SUITE_NAMED_REGISTRATION(UnoViewContainerTest, "UnoViewContainerTest");
} // namespace


// -----------------------------------------------------------------------------

// this macro creates an empty function, which will called by the RegisterAllFunctions()
// to let the user the possibility to also register some functions by hand.
NOADDITIONAL;

