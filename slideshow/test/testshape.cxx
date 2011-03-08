/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <cppuhelper/compbase1.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/make_shared_from_uno.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drange.hxx>

#include "shape.hxx"
#include "tests.hxx"
#include "com/sun/star/presentation/XSlideShowView.hpp"

#include <o3tl/compat_functional.hxx>

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
                             boost::bind( o3tl::select1st<ViewVector::value_type>(),
                                          _1 ))) == maViewLayers.end() )
            throw std::exception();

        maViewLayers.erase(
            std::remove_if(
                maViewLayers.begin(),
                maViewLayers.end(),
                boost::bind( std::equal_to< target::ViewLayerSharedPtr >(),
                             boost::cref( rNewLayer ),
                             boost::bind( o3tl::select1st<ViewVector::value_type>(),
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
