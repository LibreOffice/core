/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include <sal/types.h>
#include <cppunit/TestAssert.h>
#include <cppunit/TestFixture.h>
#include <cppunit/extensions/HelperMacros.h>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <comphelper/make_shared_from_uno.hxx>

#include <basegfx/matrix/b2dhommatrix.hxx>
#include <basegfx/range/b2drange.hxx>

#include "shape.hxx"
#include "tests.hxx"
#include "com/sun/star/presentation/XSlideShowView.hpp"

namespace target = slideshow::internal;
using namespace ::com::sun::star;

// our test shape subject
typedef ::cppu::WeakComponentImplHelper< drawing::XShape > ShapeBase;
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

    // XShape
    virtual OUString SAL_CALL getShapeType(  ) throw (uno::RuntimeException)
    {
        CPPUNIT_ASSERT_MESSAGE( "TestShape::getShapeType: unexpected method call", false );
        return OUString();
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
        if( std::none_of(
                maViewLayers.begin(),
                maViewLayers.end(),
                [&rNewLayer]
                ( const ViewVector::value_type& cp )
                { return cp.first == rNewLayer; } ) )
            throw std::exception();

        maViewLayers.erase(
            std::remove_if(
                maViewLayers.begin(),
                maViewLayers.end(),
                [&rNewLayer]
                ( const ViewVector::value_type& cp )
                { return cp.first == rNewLayer; } ) );
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
