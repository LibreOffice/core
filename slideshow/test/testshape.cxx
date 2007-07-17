/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: testshape.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2007-07-17 15:21:51 $
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
#include <cppuhelper/basemutex.hxx>
#include <comphelper/make_shared_from_uno.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drange.hxx>

#include "shape.hxx"
#include "tests.hxx"
#include "com/sun/star/presentation/XSlideShowView.hpp"

#include <boost/bind.hpp>

namespace target = slideshow::internal;
using namespace ::com::sun::star;

// our test shape subject
typedef ::cppu::WeakComponentImplHelper1< drawing::XShape > ShapeBase;
class ImplTestShape : public TestShape,
                      private cppu::BaseMutex,
                      public ShapeBase
{
    typedef std::vector<std::pair<target::ViewLayerSharedPtr,bool> > ViewVector;
    ViewVector               maViewLayers;
    const basegfx::B2DRange  maRect;
    const double             mnPrio;
    sal_Int32                mnAnimated;
    mutable sal_Int32        mnNumUpdates;
    mutable sal_Int32        mnNumRenders;

public:
    ImplTestShape( const basegfx::B2DRange& rRect,
                   double                   nPrio ) :
        ShapeBase( m_aMutex ),
        maViewLayers(),
        maRect( rRect ),
        mnPrio( nPrio ),
        mnAnimated(0),
        mnNumUpdates(0),
        mnNumRenders(0)
    {}


private:
    // TestShape
    virtual std::vector<std::pair<target::ViewLayerSharedPtr,bool> > getViewLayers() const
    {
        return maViewLayers;
    }
    virtual sal_Int32 getNumUpdates() const
    {
        return mnNumUpdates;
    }
    virtual sal_Int32 getNumRenders() const
    {
        return mnNumRenders;
    }
    virtual sal_Int32 getAnimationCount() const
    {
        return mnAnimated;
    }


    // XShape
    virtual ::rtl::OUString SAL_CALL getShapeType(  ) throw (uno::RuntimeException)
    {
        CPPUNIT_ASSERT_MESSAGE( "TestShape::getShapeType: unexpected method call", false );
        return ::rtl::OUString();
    }

    virtual awt::Point SAL_CALL getPosition(  ) throw (uno::RuntimeException)
    {
        CPPUNIT_ASSERT_MESSAGE( "TestShape::getPosition: unexpected method call", false );
        return awt::Point();
    }

    virtual void SAL_CALL setPosition( const awt::Point& ) throw (uno::RuntimeException)
    {
        CPPUNIT_ASSERT_MESSAGE( "TestShape::setPosition: unexpected method call", false );
    }

    virtual awt::Size SAL_CALL getSize(  ) throw (uno::RuntimeException)
    {
        CPPUNIT_ASSERT_MESSAGE( "TestShape::getSize: unexpected method call", false );
        return awt::Size();
    }

    virtual void SAL_CALL setSize( const awt::Size& /*aSize*/ ) throw (beans::PropertyVetoException, uno::RuntimeException)
    {
        CPPUNIT_ASSERT_MESSAGE( "TestShape::setSize: unexpected method call", false );
    }


    //////////////////////////////////////////////////////////////////////////


    // Shape
    virtual uno::Reference< drawing::XShape > getXShape() const
    {
        return uno::Reference< drawing::XShape >( const_cast<ImplTestShape*>(this) );
    }
    virtual void addViewLayer( const target::ViewLayerSharedPtr& rNewLayer,
                               bool                              bRedrawLayer )
    {
        maViewLayers.push_back( std::make_pair(rNewLayer,bRedrawLayer) );
    }
    virtual bool removeViewLayer( const target::ViewLayerSharedPtr& rNewLayer )
    {
        if( std::find_if(
                maViewLayers.begin(),
                maViewLayers.end(),
                boost::bind( std::equal_to< target::ViewLayerSharedPtr >(),
                             boost::cref( rNewLayer ),
                             boost::bind( std::select1st<ViewVector::value_type>(),
                                          _1 ))) == maViewLayers.end() )
            throw std::exception();

        maViewLayers.erase(
            std::remove_if(
                maViewLayers.begin(),
                maViewLayers.end(),
                boost::bind( std::equal_to< target::ViewLayerSharedPtr >(),
                             boost::cref( rNewLayer ),
                             boost::bind( std::select1st<ViewVector::value_type>(),
                                          _1 ))));
        return true;
    }
    virtual bool clearAllViewLayers()
    {
        maViewLayers.clear();
        return true;
    }

    virtual bool update() const
    {
        ++mnNumUpdates;
        return true;
    }
    virtual bool render() const
    {
        ++mnNumRenders;
        return true;
    }
    virtual bool isContentChanged() const
    {
        return true;
    }
    virtual ::basegfx::B2DRectangle getBounds() const
    {
        return maRect;
    }
    virtual ::basegfx::B2DRectangle getDomBounds() const
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
        return mnAnimated != 0;
    }

    // AnimatableShape
    virtual void enterAnimationMode()
    {
        ++mnAnimated;
    }

    virtual void leaveAnimationMode()
    {
        --mnAnimated;
    }
};


TestShapeSharedPtr createTestShape(const basegfx::B2DRange& rRect,
                                   double                   nPrio)
{
    return TestShapeSharedPtr(
        comphelper::make_shared_from_UNO(
            new ImplTestShape(rRect,nPrio)) );
}
