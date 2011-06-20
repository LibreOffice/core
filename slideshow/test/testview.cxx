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
#include <basegfx/range/b1drange.hxx>
#include <basegfx/range/b2drectangle.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <cppcanvas/spritecanvas.hxx>

#include "tests.hxx"
#include "view.hxx"
#include "unoview.hxx"
#include "com/sun/star/presentation/XSlideShowView.hpp"

#include <vector>
#include <exception>


namespace target = slideshow::internal;
using namespace ::com::sun::star;

// our test view subject
typedef ::cppu::WeakComponentImplHelper1< presentation::XSlideShowView > ViewBase;
class ImplTestView : public TestView,
                     private cppu::BaseMutex,
                     public ViewBase
{
    mutable std::vector<std::pair<basegfx::B2DVector,double> > maCreatedSprites;
    mutable std::vector<TestViewSharedPtr>                     maViewLayers;
    basegfx::B2DRange                                  maBounds;
    basegfx::B1DRange                                  maPriority;
    bool                                               mbIsClipSet;
    bool                                               mbIsClipEmptied;
    bool                                               mbIsClearCalled;
    bool                                               mbDisposed;


public:
    ImplTestView() :
        ViewBase(m_aMutex),
        maCreatedSprites(),
        maViewLayers(),
        maBounds(),
        maPriority(),
        mbIsClipSet(false),
        mbIsClipEmptied(false),
        mbIsClearCalled(false),
        mbDisposed( false )
    {
    }

    virtual ~ImplTestView()
    {
    }

    // XSlideShowView
    virtual uno::Reference< rendering::XSpriteCanvas > SAL_CALL getCanvas(  ) throw (uno::RuntimeException)
    {
        return uno::Reference< rendering::XSpriteCanvas >();
    }

    virtual void SAL_CALL clear(  ) throw (uno::RuntimeException)
    {
    }

    virtual geometry::AffineMatrix2D SAL_CALL getTransformation(  ) throw (uno::RuntimeException)
    {
        return geometry::AffineMatrix2D();
    }

    virtual void SAL_CALL addTransformationChangedListener( const uno::Reference< util::XModifyListener >& ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL removeTransformationChangedListener( const uno::Reference< util::XModifyListener >& ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL addPaintListener( const uno::Reference< awt::XPaintListener >& ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL removePaintListener( const uno::Reference< awt::XPaintListener >& ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL addMouseListener( const uno::Reference< awt::XMouseListener >& ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL removeMouseListener( const uno::Reference< awt::XMouseListener >& ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL addMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL removeMouseMotionListener( const uno::Reference< awt::XMouseMotionListener >& ) throw (uno::RuntimeException)
    {
    }

    virtual void SAL_CALL setMouseCursor( ::sal_Int16 ) throw (uno::RuntimeException)
    {
    }

    virtual awt::Rectangle SAL_CALL getCanvasArea(  ) throw (uno::RuntimeException)
    {
        return awt::Rectangle(0,0,100,100);
    }

    // TestView
    virtual bool isClearCalled() const
    {
        return mbIsClearCalled;
    }

    virtual std::vector<std::pair<basegfx::B2DVector,double> > getCreatedSprites() const
    {
        return maCreatedSprites;
    }

    virtual basegfx::B1DRange getPriority() const
    {
        return maPriority;
    }

    virtual bool wasClipSet() const
    {
        return mbIsClipEmptied;
    }

    virtual basegfx::B2DRange getBounds() const
    {
        return maBounds;
    }

    virtual std::vector<boost::shared_ptr<TestView> > getViewLayers() const
    {
        return maViewLayers;
    }

    // ViewLayer
    virtual bool isOnView(target::ViewSharedPtr const& /*rView*/) const
    {
        return true;
    }

    virtual ::cppcanvas::CanvasSharedPtr getCanvas() const
    {
        return ::cppcanvas::CanvasSharedPtr();
    }

    virtual ::cppcanvas::CustomSpriteSharedPtr createSprite( const ::basegfx::B2DSize& rSpriteSizePixel,
                                                             double                    nPriority ) const
    {
        maCreatedSprites.push_back( std::make_pair(rSpriteSizePixel,nPriority) );

        return ::cppcanvas::CustomSpriteSharedPtr();
    }

    virtual void setPriority( const basegfx::B1DRange& rRange )
    {
        maPriority = rRange;
    }

    virtual ::basegfx::B2DHomMatrix getTransformation() const
    {
        return ::basegfx::B2DHomMatrix();
    }

    virtual ::basegfx::B2DHomMatrix getSpriteTransformation() const
    {
        return ::basegfx::B2DHomMatrix();
    }

    virtual void setClip( const ::basegfx::B2DPolyPolygon& rClip )
    {
        if( !mbIsClipSet )
        {
            if( rClip.count() > 0 )
                mbIsClipSet = true;
        }
        else if( !mbIsClipEmptied )
        {
            if( rClip.count() == 0 )
                mbIsClipEmptied = true;
        }
        else if( rClip.count() > 0 )
        {
            mbIsClipSet = true;
            mbIsClipEmptied = false;
        }
        else
        {
            // unexpected call
            throw std::exception();
        }
    }

    virtual bool resize( const basegfx::B2DRange& rArea )
    {
        const bool bRet( maBounds != rArea );
        maBounds = rArea;
        return bRet;
    }

    virtual target::ViewLayerSharedPtr createViewLayer(
        const basegfx::B2DRange& rLayerBounds ) const
    {
        maViewLayers.push_back( TestViewSharedPtr(new ImplTestView()));
        maViewLayers.back()->resize( rLayerBounds );

        return maViewLayers.back();
    }

    virtual bool updateScreen() const
    {
        // misusing updateScreen for state reporting
        return !mbDisposed;
    }

    virtual bool paintScreen() const
    {
        // misusing updateScreen for state reporting
        return !mbDisposed;
    }

    virtual void clear() const
    {
    }

    virtual void clearAll() const
    {
    }

    virtual void setViewSize( const ::basegfx::B2DSize& )
    {
    }

    virtual void setCursorShape( sal_Int16 /*nPointerShape*/ )
    {
    }

    virtual uno::Reference< presentation::XSlideShowView > getUnoView() const
    {
        return uno::Reference< presentation::XSlideShowView >( const_cast<ImplTestView*>(this) );
    }

    virtual void _dispose()
    {
        mbDisposed = true;
    }

    virtual bool isSoundEnabled (void) const
    {
        return true;
    }

    virtual void setIsSoundEnabled (const bool /*bValue*/)
    {
    }
};


TestViewSharedPtr createTestView()
{
    return TestViewSharedPtr(
        comphelper::make_shared_from_UNO(
            new ImplTestView()) );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
