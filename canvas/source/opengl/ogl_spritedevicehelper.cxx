/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "ogl_spritedevicehelper.hxx"
#include "ogl_spritecanvas.hxx"
#include "ogl_canvasbitmap.hxx"
#include "ogl_canvastools.hxx"
#include "ogl_canvascustomsprite.hxx"
#include "ogl_texturecache.hxx"

#include <canvas/verbosetrace.hxx>
#include <basegfx/tools/canvastools.hxx>
#include <basegfx/tools/unopolypolygon.hxx>

#include <osl/mutex.hxx>
#include <rtl/instance.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/rendering/XColorSpace.hpp>
#include <com/sun/star/rendering/XIntegerBitmapColorSpace.hpp>

#include <vcl/sysdata.hxx>
#include <vcl/syschild.hxx>
#include <vcl/canvastools.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <vcl/opengl/OpenGLHelper.hxx>

using namespace ::com::sun::star;

static void initContext()
{
    // need the backside for mirror effects
    glDisable(GL_CULL_FACE);


    // misc preferences
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
    glShadeModel(GL_FLAT);
}

static void initTransformation(const ::Size& rSize, bool bMirror=false)
{
    // use whole window
    glViewport( 0,0,
                (GLsizei)rSize.Width(),
                (GLsizei)rSize.Height() );
    // model coordinate system is already in device pixel
    // clear to black
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

namespace oglcanvas
{

    SpriteDeviceHelper::SpriteDeviceHelper() :
        mpDevice(NULL),
        mpSpriteCanvas(NULL),
        maActiveSprites(),
        maLastUpdate(),
        mpTextureCache(new TextureCache())
    {}

    SpriteDeviceHelper::~SpriteDeviceHelper()
    {}

    void SpriteDeviceHelper::init( vcl::Window&               rWindow,
                                   SpriteCanvas&         rSpriteCanvas,
                                   const awt::Rectangle& rViewArea )
    {
        mpSpriteCanvas = &rSpriteCanvas;

        rSpriteCanvas.setWindow(
            uno::Reference<awt::XWindow2>(
                VCLUnoHelper::GetInterface(&rWindow),
                uno::UNO_QUERY_THROW) );

        mpContext.reset(new OpenGLContext);
        mpContext->requestLegacyContext();
        mpContext->init(&rWindow);
        // init window context
        initContext();
        mRenderHelper.InitOpenGL();
        mpContext->makeCurrent();

        notifySizeUpdate(rViewArea);
        // TODO(E3): check for GL_ARB_imaging extension
    }

    void SpriteDeviceHelper::disposing()
    {
        // release all references
        mpSpriteCanvas = NULL;
        mpDevice = NULL;
        mpTextureCache.reset();
        mpContext.reset();
        mRenderHelper.dispose();
    }

    geometry::RealSize2D SpriteDeviceHelper::getPhysicalResolution()
    {
        if( !mpContext->isInitialized() )
            return ::canvas::tools::createInfiniteSize2D(); // we're disposed

        // Map a one-by-one millimeter box to pixel
        SystemChildWindow* pChildWindow = mpContext->getChildWindow();
        const MapMode aOldMapMode( pChildWindow->GetMapMode() );
        pChildWindow->SetMapMode( MapMode(MAP_MM) );
        const Size aPixelSize( pChildWindow->LogicToPixel(Size(1,1)) );
        pChildWindow->SetMapMode( aOldMapMode );

        return ::vcl::unotools::size2DFromSize( aPixelSize );
    }

    geometry::RealSize2D SpriteDeviceHelper::getPhysicalSize()
    {
        if( !mpContext->isInitialized() )
            return ::canvas::tools::createInfiniteSize2D(); // we're disposed

        // Map the pixel dimensions of the output window to millimeter
        SystemChildWindow* pChildWindow = mpContext->getChildWindow();
        const MapMode aOldMapMode( pChildWindow->GetMapMode() );
        pChildWindow->SetMapMode( MapMode(MAP_MM) );
        const Size aLogSize( pChildWindow->PixelToLogic(pChildWindow->GetOutputSizePixel()) );
        pChildWindow->SetMapMode( aOldMapMode );

        return ::vcl::unotools::size2DFromSize( aLogSize );
    }

    uno::Reference< rendering::XLinePolyPolygon2D > SpriteDeviceHelper::createCompatibleLinePolyPolygon(
        const uno::Reference< rendering::XGraphicDevice >&              /*rDevice*/,
        const uno::Sequence< uno::Sequence< geometry::RealPoint2D > >&  points )
    {
        // disposed?
        if( !mpSpriteCanvas )
            return uno::Reference< rendering::XLinePolyPolygon2D >(); // we're disposed

        return uno::Reference< rendering::XLinePolyPolygon2D >(
            new ::basegfx::unotools::UnoPolyPolygon(
                ::basegfx::unotools::polyPolygonFromPoint2DSequenceSequence( points )));
    }

    uno::Reference< rendering::XBezierPolyPolygon2D > SpriteDeviceHelper::createCompatibleBezierPolyPolygon(
        const uno::Reference< rendering::XGraphicDevice >&                      /*rDevice*/,
        const uno::Sequence< uno::Sequence< geometry::RealBezierSegment2D > >&  points )
    {
        // disposed?
        if( !mpSpriteCanvas )
            return uno::Reference< rendering::XBezierPolyPolygon2D >(); // we're disposed

        return uno::Reference< rendering::XBezierPolyPolygon2D >(
            new ::basegfx::unotools::UnoPolyPolygon(
                ::basegfx::unotools::polyPolygonFromBezier2DSequenceSequence( points ) ) );
    }

    uno::Reference< rendering::XBitmap > SpriteDeviceHelper::createCompatibleBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
        const geometry::IntegerSize2D&                      size )
    {
        // disposed?
        if( !mpSpriteCanvas )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        return uno::Reference< rendering::XBitmap >(
            new CanvasBitmap( size,
                              mpSpriteCanvas,
                              *this,
                              false ) );
    }

    uno::Reference< rendering::XVolatileBitmap > SpriteDeviceHelper::createVolatileBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
        const geometry::IntegerSize2D&                      /*size*/ )
    {
        return uno::Reference< rendering::XVolatileBitmap >();
    }

    uno::Reference< rendering::XBitmap > SpriteDeviceHelper::createCompatibleAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
        const geometry::IntegerSize2D&                      size )
    {
        // disposed?
        if( !mpSpriteCanvas )
            return uno::Reference< rendering::XBitmap >(); // we're disposed

        return uno::Reference< rendering::XBitmap >(
            new CanvasBitmap( size,
                              mpSpriteCanvas,
                              *this,
                              true ) );
    }

    uno::Reference< rendering::XVolatileBitmap > SpriteDeviceHelper::createVolatileAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
        const geometry::IntegerSize2D&                      /*size*/ )
    {
        return uno::Reference< rendering::XVolatileBitmap >();
    }

    bool SpriteDeviceHelper::hasFullScreenMode()
    {
        // TODO(F3): offer fullscreen mode the XCanvas way
        return false;
    }

    bool SpriteDeviceHelper::enterFullScreenMode( bool /*bEnter*/ )
    {
        // TODO(F3): offer fullscreen mode the XCanvas way
        return false;
    }

    ::sal_Int32 SpriteDeviceHelper::createBuffers( ::sal_Int32 /*nBuffers*/ )
    {
        // TODO(F3): implement XBufferStrategy interface. For now, we
        // _always_ will have exactly one backbuffer
        return 1;
    }

    void SpriteDeviceHelper::destroyBuffers()
    {
        // TODO(F3): implement XBufferStrategy interface. For now, we
        // _always_ will have exactly one backbuffer
    }

    namespace
    {
        /** Functor providing a StrictWeakOrdering for XSprites (over
            priority)
         */
        struct SpriteComparator
        {
            bool operator()( const ::rtl::Reference<CanvasCustomSprite>& rLHS,
                             const ::rtl::Reference<CanvasCustomSprite>& rRHS ) const
            {
                const double nPrioL( rLHS->getPriority() );
                const double nPrioR( rRHS->getPriority() );

                // if prios are equal, tie-break on ptr value
                return nPrioL == nPrioR ? rLHS.get() < rRHS.get() : nPrioL < nPrioR;
            }
        };
    }

    bool SpriteDeviceHelper::showBuffer( bool bIsVisible, bool /*bUpdateAll*/ )
    {
        // hidden or disposed?
        if( !bIsVisible || !mpContext->isInitialized() || !mpSpriteCanvas )
            return false;

        if( !activateWindowContext() )
            return false;

        SystemChildWindow* pChildWindow = mpContext->getChildWindow();
        const ::Size& rOutputSize = pChildWindow->GetSizePixel();
        initTransformation(rOutputSize);

        // render the actual spritecanvas content
        mpSpriteCanvas->renderRecordedActions();

        // render all sprites (in order of priority) on top of that
        std::vector< ::rtl::Reference<CanvasCustomSprite> > aSprites;
        std::copy(maActiveSprites.begin(),
                  maActiveSprites.end(),
                  std::back_insert_iterator<
                       std::vector< ::rtl::Reference< CanvasCustomSprite > > >(aSprites));
        std::sort(aSprites.begin(),
                  aSprites.end(),
                  SpriteComparator());
        std::for_each(aSprites.begin(),
                      aSprites.end(),
                      boost::mem_fn(&CanvasCustomSprite::renderSprite));

#ifdef DEBUG_RENDERING
        // frame counter, other info
        const double denominator( maLastUpdate.getElapsedTime() );
        maLastUpdate.reset();


        const double fps(denominator == 0.0 ? 100.0 : 1.0/denominator);


        std::vector<double> aVec;
        aVec.push_back(mfAlpha);
        aVec.push_back(mfPriority);
        aVec.push_back(maCanvasHelper.getRecordedActionCount());

        renderOSD( aVec, 20, pRenderHelper);
#endif


        /*
         * TODO: moggi: fix it!
        // switch buffer, sync etc.
        const unx::Window aXWindow=pChildWindow->GetSystemData()->aWindow;
        unx::glXSwapBuffers(reinterpret_cast<unx::Display*>(mpDisplay),
                            aXWindow);
        pChildWindow->Show();
        unx::glXWaitGL();
        XSync( reinterpret_cast<unx::Display*>(mpDisplay), false );
        */
        mpContext->swapBuffers();

        // flush texture cache, such that it does not build up
        // indefinitely.
        // TODO: have max cache size/LRU time in config, prune only on
        // demand
        mpTextureCache->prune();

        return true;
    }

    bool SpriteDeviceHelper::switchBuffer( bool bIsVisible, bool bUpdateAll )
    {
        // no difference for VCL canvas
        return showBuffer( bIsVisible, bUpdateAll );
    }

    uno::Any SpriteDeviceHelper::isAccelerated() const
    {
        return ::com::sun::star::uno::makeAny(false);
    }

    uno::Any SpriteDeviceHelper::getDeviceHandle() const
    {
        const SystemChildWindow* pChildWindow = mpContext->getChildWindow();
        return uno::makeAny( reinterpret_cast< sal_Int64 >(pChildWindow) );
    }

    uno::Any SpriteDeviceHelper::getSurfaceHandle() const
    {
        return uno::Any();
    }

    uno::Reference<rendering::XColorSpace> SpriteDeviceHelper::getColorSpace() const
    {
        // always the same
        return uno::Reference<rendering::XColorSpace>(
            ::canvas::tools::getStdColorSpace(),
            uno::UNO_QUERY);
    }

    void SpriteDeviceHelper::notifySizeUpdate( const awt::Rectangle& rBounds )
    {
        if( mpContext->isInitialized() )
        {
            SystemChildWindow* pChildWindow = mpContext->getChildWindow();
            pChildWindow->setPosSizePixel(
                0,0,rBounds.Width,rBounds.Height);
            mRenderHelper.SetVP(rBounds.Width, rBounds.Height);
        }
    }

    void SpriteDeviceHelper::dumpScreenContent() const
    {
        SAL_INFO("canvas.ogl", BOOST_CURRENT_FUNCTION );
    }

    void SpriteDeviceHelper::show( const ::rtl::Reference< CanvasCustomSprite >& xSprite )
    {
        maActiveSprites.insert(xSprite);
    }

    void SpriteDeviceHelper::hide( const ::rtl::Reference< CanvasCustomSprite >& xSprite )
    {
        maActiveSprites.erase(xSprite);
    }


    bool SpriteDeviceHelper::activateWindowContext()
    {
        mpContext->makeCurrent();
        return true;
    }

    namespace
    {

        class BufferContextImpl : public IBufferContext
        {
            glm::vec2       maSize;
            GLuint mnFrambufferId;
            GLuint mnDepthId;
            GLuint mnTextureId;

            virtual bool startBufferRendering() SAL_OVERRIDE
            {
                glBindFramebuffer(GL_FRAMEBUFFER, mnFrambufferId);
                return true;
            }

            virtual bool endBufferRendering() SAL_OVERRIDE
            {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
                return true;
            }

            virtual GLuint getTextureId() SAL_OVERRIDE
            {
                return mnTextureId;
            }

        public:
            BufferContextImpl(const glm::vec2& rSize) :
                maSize(rSize),
                mnFrambufferId(0),
                mnDepthId(0),
                mnTextureId(0)
            {
                OpenGLHelper::createFramebuffer(maSize.x, maSize.y, mnFrambufferId,
                        mnDepthId, mnTextureId, false);
            }

            virtual ~BufferContextImpl()
            {
                glDeleteTextures(1, &mnTextureId);
                glDeleteRenderbuffers(1, &mnDepthId);
                glDeleteFramebuffers(1, &mnFrambufferId);
            }
        };
    }

    IBufferContextSharedPtr SpriteDeviceHelper::createBufferContext(const glm::vec2& rSize) const
    {
        return IBufferContextSharedPtr(new BufferContextImpl(rSize));
    }

    TextureCache& SpriteDeviceHelper::getTextureCache() const
    {
        return *mpTextureCache;
    }

    RenderHelper* SpriteDeviceHelper::getRenderHelper()
    {
        return &    mRenderHelper;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
