/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/config.h>
#include <sal/log.hxx>

#include <basegfx/utils/canvastools.hxx>
#include <basegfx/utils/unopolypolygon.hxx>
#include <com/sun/star/awt/XTopWindow.hpp>
#include <com/sun/star/lang/NoSupportException.hpp>
#include <com/sun/star/rendering/XColorSpace.hpp>
#include <com/sun/star/rendering/XIntegerBitmapColorSpace.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <rtl/instance.hxx>
#include <toolkit/helper/vclunohelper.hxx>
#include <vcl/canvastools.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>
#include <vcl/syschild.hxx>
#include <vcl/sysdata.hxx>

#include "ogl_spritedevicehelper.hxx"
#include "ogl_spritecanvas.hxx"
#include "ogl_canvasbitmap.hxx"
#include "ogl_canvastools.hxx"
#include "ogl_canvascustomsprite.hxx"
#include "ogl_texturecache.hxx"

#include <iterator>

using namespace ::com::sun::star;

static void initContext()
{
    // need the backside for mirror effects
    glDisable(GL_CULL_FACE);

    // no perspective, we're 2D
    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    // misc preferences
    glEnable(GL_POINT_SMOOTH);
    glEnable(GL_LINE_SMOOTH);
    glEnable(GL_POLYGON_SMOOTH);
    glHint(GL_POINT_SMOOTH_HINT,GL_NICEST);
    glHint(GL_LINE_SMOOTH_HINT,GL_NICEST);
    glHint(GL_POLYGON_SMOOTH_HINT,GL_NICEST);
    glShadeModel(GL_FLAT);
}

static void initTransformation(const ::Size& rSize)
{
    // use whole window
    glViewport( 0,0,
                static_cast<GLsizei>(rSize.Width()),
                static_cast<GLsizei>(rSize.Height()) );

    // model coordinate system is already in device pixel
    glMatrixMode(GL_MODELVIEW);
    glLoadIdentity();
    glTranslated(-1.0, 1.0, 0.0);
    glScaled( 2.0  / rSize.Width(),
              -2.0 / rSize.Height(),
              1.0 );

    // clear to black
    glClearColor(0,0,0,0);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

namespace oglcanvas
{

    SpriteDeviceHelper::SpriteDeviceHelper() :
        mpSpriteCanvas(nullptr),
        maActiveSprites(),
        maLastUpdate(),
        mpTextureCache(new TextureCache()),
        mnLinearTwoColorGradientProgram(0),
        mnLinearMultiColorGradientProgram(0),
        mnRadialTwoColorGradientProgram(0),
        mnRadialMultiColorGradientProgram(0),
        mnRectangularTwoColorGradientProgram(0),
        mnRectangularMultiColorGradientProgram(0),
        mxContext(OpenGLContext::Create())
    {}

    SpriteDeviceHelper::~SpriteDeviceHelper()
    { mxContext->dispose(); }

    void SpriteDeviceHelper::init( vcl::Window&               rWindow,
                                   SpriteCanvas&         rSpriteCanvas,
                                   const awt::Rectangle& rViewArea )
    {
        mpSpriteCanvas = &rSpriteCanvas;

        rSpriteCanvas.setWindow(
            uno::Reference<awt::XWindow2>(
                VCLUnoHelper::GetInterface(&rWindow),
                uno::UNO_QUERY_THROW) );

        mxContext->requestLegacyContext();
        mxContext->init(&rWindow);
        // init window context
        initContext();

        mnLinearMultiColorGradientProgram =
            OpenGLHelper::LoadShaders("dummyVertexShader", "linearMultiColorGradientFragmentShader");

        mnLinearTwoColorGradientProgram =
            OpenGLHelper::LoadShaders("dummyVertexShader", "linearTwoColorGradientFragmentShader");

        mnRadialMultiColorGradientProgram =
            OpenGLHelper::LoadShaders("dummyVertexShader", "radialMultiColorGradientFragmentShader");

        mnRadialTwoColorGradientProgram =
            OpenGLHelper::LoadShaders("dummyVertexShader", "radialTwoColorGradientFragmentShader");

        mnRectangularMultiColorGradientProgram =
            OpenGLHelper::LoadShaders("dummyVertexShader", "rectangularMultiColorGradientFragmentShader");

        mnRectangularTwoColorGradientProgram =
            OpenGLHelper::LoadShaders("dummyVertexShader", "rectangularTwoColorGradientFragmentShader");

        mxContext->makeCurrent();

        notifySizeUpdate(rViewArea);
        // TODO(E3): check for GL_ARB_imaging extension
    }

    void SpriteDeviceHelper::disposing()
    {
        // release all references
        mpSpriteCanvas = nullptr;
        mpTextureCache.reset();

        if( mxContext->isInitialized() )
        {
            glDeleteProgram( mnRectangularTwoColorGradientProgram );
            glDeleteProgram( mnRectangularMultiColorGradientProgram );
            glDeleteProgram( mnRadialTwoColorGradientProgram );
            glDeleteProgram( mnRadialMultiColorGradientProgram );
            glDeleteProgram( mnLinearTwoColorGradientProgram );
            glDeleteProgram( mnLinearMultiColorGradientProgram );
        }
    }

    geometry::RealSize2D SpriteDeviceHelper::getPhysicalResolution()
    {
        if( !mxContext->isInitialized() )
            return ::canvas::tools::createInfiniteSize2D(); // we're disposed

        // Map a one-by-one millimeter box to pixel
        SystemChildWindow* pChildWindow = mxContext->getChildWindow();
        const MapMode aOldMapMode( pChildWindow->GetMapMode() );
        pChildWindow->SetMapMode( MapMode(MapUnit::MapMM) );
        const Size aPixelSize( pChildWindow->LogicToPixel(Size(1,1)) );
        pChildWindow->SetMapMode( aOldMapMode );

        return vcl::unotools::size2DFromSize( aPixelSize );
    }

    geometry::RealSize2D SpriteDeviceHelper::getPhysicalSize()
    {
        if( !mxContext->isInitialized() )
            return ::canvas::tools::createInfiniteSize2D(); // we're disposed

        // Map the pixel dimensions of the output window to millimeter
        SystemChildWindow* pChildWindow = mxContext->getChildWindow();
        const MapMode aOldMapMode( pChildWindow->GetMapMode() );
        pChildWindow->SetMapMode( MapMode(MapUnit::MapMM) );
        const Size aLogSize( pChildWindow->PixelToLogic(pChildWindow->GetOutputSizePixel()) );
        pChildWindow->SetMapMode( aOldMapMode );

        return vcl::unotools::size2DFromSize( aLogSize );
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
                              *this ) );
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
                              *this ) );
    }

    uno::Reference< rendering::XVolatileBitmap > SpriteDeviceHelper::createVolatileAlphaBitmap(
        const uno::Reference< rendering::XGraphicDevice >&  /*rDevice*/,
        const geometry::IntegerSize2D&                      /*size*/ )
    {
        return uno::Reference< rendering::XVolatileBitmap >();
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

    bool SpriteDeviceHelper::showBuffer( bool bIsVisible, SAL_UNUSED_PARAMETER bool /*bUpdateAll*/ )
    {
        // hidden or disposed?
        if( !bIsVisible || !mxContext->isInitialized() || !mpSpriteCanvas )
            return false;

        mxContext->makeCurrent();

        SystemChildWindow* pChildWindow = mxContext->getChildWindow();
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
        for( const auto& rSprite : aSprites )
            rSprite->renderSprite();


        // frame counter, other info
        glMatrixMode(GL_MODELVIEW);
        glLoadIdentity();
        glTranslated(-1.0, 1.0, 0.0);
        glScaled( 2.0  / rOutputSize.Width(),
                  -2.0 / rOutputSize.Height(),
                  1.0 );

        const double denominator( maLastUpdate.getElapsedTime() );
        maLastUpdate.reset();

        const double fps(denominator == 0.0 ? 100.0 : 1.0/denominator);
        std::vector<double> aVec; aVec.push_back(fps);
        aVec.push_back(maActiveSprites.size());
        aVec.push_back(mpTextureCache->getCacheSize());
        aVec.push_back(mpTextureCache->getCacheMissCount());
        aVec.push_back(mpTextureCache->getCacheHitCount());
        renderOSD( aVec, 20 );

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
        mxContext->swapBuffers();

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
        return css::uno::Any(false);
    }

    uno::Any SpriteDeviceHelper::getDeviceHandle() const
    {
        const SystemChildWindow* pChildWindow = mxContext->getChildWindow();
        return uno::Any( reinterpret_cast< sal_Int64 >(pChildWindow) );
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
        if( mxContext->isInitialized() )
        {
            SystemChildWindow* pChildWindow = mxContext->getChildWindow();
            pChildWindow->setPosSizePixel(
                0,0,rBounds.Width,rBounds.Height);
        }
    }

    void SpriteDeviceHelper::dumpScreenContent() const
    {
        SAL_INFO("canvas.ogl", OSL_THIS_FUNC );
    }

    void SpriteDeviceHelper::show( const ::rtl::Reference< CanvasCustomSprite >& xSprite )
    {
        maActiveSprites.insert(xSprite);
    }

    void SpriteDeviceHelper::hide( const ::rtl::Reference< CanvasCustomSprite >& xSprite )
    {
        maActiveSprites.erase(xSprite);
    }

    static void setupUniforms( unsigned int                                 nProgramId,
                               const ::basegfx::B2DHomMatrix&               rTexTransform )
    {
        const GLint nTransformLocation = glGetUniformLocation(nProgramId,
                                                             "m_transform" );
        // OGL is column-major
        float aTexTransform[] =
            {
                float(rTexTransform.get(0,0)), float(rTexTransform.get(1,0)),
                float(rTexTransform.get(0,1)), float(rTexTransform.get(1,1)),
                float(rTexTransform.get(0,2)), float(rTexTransform.get(1,2))
            };
        glUniformMatrix3x2fv(nTransformLocation,1,false,aTexTransform);
    }

    static void setupUniforms( unsigned int                   nProgramId,
                               const rendering::ARGBColor*    pColors,
                               const uno::Sequence< double >& rStops,
                               const ::basegfx::B2DHomMatrix& rTexTransform )
    {
        glUseProgram(nProgramId);

        GLuint nColorsTexture;
        glActiveTexture(GL_TEXTURE0);
        glGenTextures(1, &nColorsTexture);
        glBindTexture(GL_TEXTURE_1D, nColorsTexture);

        const sal_Int32 nColors=rStops.getLength();
        glTexImage1D( GL_TEXTURE_1D, 0, GL_RGBA, nColors, 0, GL_RGBA, GL_DOUBLE, pColors );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

        GLuint nStopsTexture;
        glActiveTexture(GL_TEXTURE1);
        glGenTextures(1, &nStopsTexture);
        glBindTexture(GL_TEXTURE_1D, nStopsTexture);

        glTexImage1D( GL_TEXTURE_1D, 0, GL_ALPHA, nColors, 0, GL_ALPHA, GL_DOUBLE, rStops.getConstArray() );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MIN_FILTER, GL_NEAREST );
        glTexParameteri( GL_TEXTURE_1D, GL_TEXTURE_MAG_FILTER, GL_NEAREST );

        const GLint nColorArrayLocation = glGetUniformLocation(nProgramId,
                                                               "t_colorArray4d" );
        glUniform1i( nColorArrayLocation, 0 ); // unit 0

        const GLint nStopArrayLocation = glGetUniformLocation(nProgramId,
                                                              "t_stopArray1d" );
        glUniform1i( nStopArrayLocation, 1 ); // unit 1

        const GLint nNumColorLocation = glGetUniformLocation(nProgramId,
                                                             "i_nColors" );
        glUniform1i( nNumColorLocation, nColors-1 );

        setupUniforms(nProgramId,rTexTransform);
    }

    static void setupUniforms( unsigned int                   nProgramId,
                               const rendering::ARGBColor&    rStartColor,
                               const rendering::ARGBColor&    rEndColor,
                               const ::basegfx::B2DHomMatrix& rTexTransform )
    {
        glUseProgram(nProgramId);

        const GLint nStartColorLocation = glGetUniformLocation(nProgramId,
                                                               "v_startColor4d" );
        glUniform4f(nStartColorLocation,
                    rStartColor.Red,
                    rStartColor.Green,
                    rStartColor.Blue,
                    rStartColor.Alpha);

        const GLint nEndColorLocation = glGetUniformLocation(nProgramId,
                                                             "v_endColor4d" );
        glUniform4f(nEndColorLocation,
                    rEndColor.Red,
                    rEndColor.Green,
                    rEndColor.Blue,
                    rEndColor.Alpha);

        setupUniforms(nProgramId,rTexTransform);
    }

    void SpriteDeviceHelper::useLinearGradientShader( const rendering::ARGBColor*    pColors,
                                                      const uno::Sequence< double >& rStops,
                                                      const ::basegfx::B2DHomMatrix& rTexTransform )
    {
        if( rStops.getLength() > 2 )
            setupUniforms(mnLinearMultiColorGradientProgram, pColors, rStops, rTexTransform);
        else
            setupUniforms(mnLinearTwoColorGradientProgram, pColors[0], pColors[1], rTexTransform);
    }

    void SpriteDeviceHelper::useRadialGradientShader( const rendering::ARGBColor*    pColors,
                                                      const uno::Sequence< double >& rStops,
                                                      const ::basegfx::B2DHomMatrix& rTexTransform )
    {
        if( rStops.getLength() > 2 )
            setupUniforms(mnRadialMultiColorGradientProgram, pColors, rStops, rTexTransform);
        else
            setupUniforms(mnRadialTwoColorGradientProgram, pColors[0], pColors[1], rTexTransform);
    }

    void SpriteDeviceHelper::useRectangularGradientShader( const rendering::ARGBColor*    pColors,
                                                           const uno::Sequence< double >& rStops,
                                                           const ::basegfx::B2DHomMatrix& rTexTransform )
    {
        if( rStops.getLength() > 2 )
            setupUniforms(mnRectangularMultiColorGradientProgram, pColors, rStops, rTexTransform);
        else
            setupUniforms(mnRectangularTwoColorGradientProgram, pColors[0], pColors[1], rTexTransform);
    }

    namespace
    {

        class BufferContextImpl : public IBufferContext
        {
            GLuint mnFrambufferId;
            GLuint mnDepthId;
            GLuint mnTextureId;

            virtual void startBufferRendering() override
            {
                glBindFramebuffer(GL_FRAMEBUFFER, mnFrambufferId);
            }

            virtual void endBufferRendering() override
            {
                glBindFramebuffer(GL_FRAMEBUFFER, 0);
            }

            virtual GLuint getTextureId() override
            {
                return mnTextureId;
            }

        public:
            explicit BufferContextImpl(const ::basegfx::B2IVector& rSize) :
                mnFrambufferId(0),
                mnDepthId(0),
                mnTextureId(0)
            {
                OpenGLHelper::createFramebuffer(rSize.getX(), rSize.getY(), mnFrambufferId,
                        mnDepthId, mnTextureId);
            }

            virtual ~BufferContextImpl() override
            {
                glDeleteTextures(1, &mnTextureId);
                glDeleteRenderbuffers(1, &mnDepthId);
                glDeleteFramebuffers(1, &mnFrambufferId);
            }
        };
    }

    IBufferContextSharedPtr SpriteDeviceHelper::createBufferContext(const ::basegfx::B2IVector& rSize) const
    {
        return IBufferContextSharedPtr(new BufferContextImpl(rSize));
    }

    TextureCache& SpriteDeviceHelper::getTextureCache() const
    {
        return *mpTextureCache;
    }
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
