/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
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

#include <sal/types.h>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>

#include <memory>

#include <com/sun/star/beans/XFastPropertySet.hpp>
#include <com/sun/star/rendering/IntegerBitmapLayout.hpp>
#include <com/sun/star/rendering/ColorComponentTag.hpp>
#include <com/sun/star/rendering/ColorSpaceType.hpp>
#include <com/sun/star/rendering/RenderingIntent.hpp>
#include <com/sun/star/util/Endianness.hpp>
#include <com/sun/star/animations/TransitionType.hpp>
#undef IN
#undef OUT
#include <com/sun/star/animations/TransitionSubType.hpp>
#include <com/sun/star/presentation/XTransitionFactory.hpp>
#include <com/sun/star/presentation/XTransition.hpp>
#include <com/sun/star/presentation/XSlideShowView.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/rendering/XIntegerBitmap.hpp>
#include <com/sun/star/geometry/IntegerSize2D.hpp>

#include <cppuhelper/compbase.hxx>
#include <cppuhelper/basemutex.hxx>
#include <cppuhelper/factory.hxx>
#include <rtl/ref.hxx>
#include <sal/log.hxx>

#include <comphelper/servicedecl.hxx>

#include <canvas/canvastools.hxx>

#include <tools/diagnose_ex.h>

#include <vcl/canvastools.hxx>
#include <vcl/opengl/OpenGLContext.hxx>
#include <vcl/opengl/OpenGLHelper.hxx>
#include <vcl/window.hxx>

#include "TransitionImpl.hxx"

#include <vcl/sysdata.hxx>

#if OSL_DEBUG_LEVEL > 0
#include <chrono>
#endif

using namespace ::com::sun::star;
using ::com::sun::star::beans::XFastPropertySet;
using ::com::sun::star::uno::Any;
using ::com::sun::star::uno::Reference;
using ::com::sun::star::uno::Sequence;
using ::com::sun::star::uno::UNO_QUERY;
using ::com::sun::star::uno::UNO_QUERY_THROW;

namespace
{

typedef cppu::WeakComponentImplHelper<presentation::XTransition> OGLTransitionerImplBase;

#if OSL_DEBUG_LEVEL > 0
class TimerContext
{
public:
    explicit TimerContext(OUString const& rWhat)
        : m_aWhat(rWhat)
        , m_StartTime(std::chrono::steady_clock::now())
    {
    }
    ~TimerContext()
    {
        auto const aDuration(std::chrono::steady_clock::now() - m_StartTime);
        SAL_INFO("slideshow.opengl", m_aWhat << " took: " << std::chrono::duration_cast<std::chrono::microseconds>(aDuration).count());
    }
private:
    OUString const m_aWhat;
    std::chrono::steady_clock::time_point const m_StartTime;
};
#endif

struct OGLFormat
{
    GLint const  nInternalFormat;
    GLenum const eFormat;
    GLenum const eType;
};

/* channel ordering: (0:rgba, 1:bgra, 2:argb, 3:abgr)
 */
int calcComponentOrderIndex(const uno::Sequence<sal_Int8>& rTags)
{
    using namespace rendering::ColorComponentTag;

    static const sal_Int8 aOrderTable[] =
    {
        RGB_RED, RGB_GREEN, RGB_BLUE, ALPHA,
        RGB_BLUE, RGB_GREEN, RGB_RED, ALPHA,
        ALPHA, RGB_RED, RGB_GREEN, RGB_BLUE,
        ALPHA, RGB_BLUE, RGB_GREEN, RGB_RED,
    };

    const sal_Int32 nNumComps(rTags.getLength());
    const sal_Int8* pLine=aOrderTable;
    for(int i=0; i<4; ++i)
    {
        int j=0;
        while( j<4 && j<nNumComps && pLine[j] == rTags[j] )
            ++j;

        // all of the line passed, this is a match!
        if( j==nNumComps )
            return i;

        pLine+=4;
    }

    return -1;
}

/** This is the Transitioner class for OpenGL 3D transitions in
 * slideshow. This class is implicitly
 * constructed from XTransitionFactory.
*/
class OGLTransitionerImpl : private cppu::BaseMutex, public OGLTransitionerImplBase
{
public:
    OGLTransitionerImpl();
    OGLTransitionerImpl(const OGLTransitionerImpl&) = delete;
    OGLTransitionerImpl& operator=(const OGLTransitionerImpl&) = delete;
    bool setTransition( const std::shared_ptr<OGLTransitionImpl>& pOGLTransition );
    bool initialize( const Reference< presentation::XSlideShowView >& xView,
            const Reference< rendering::XBitmap >& xLeavingSlide,
            const Reference< rendering::XBitmap >& xEnteringSlide );

    // XTransition
    virtual void SAL_CALL update( double nTime ) override;
    virtual void SAL_CALL viewChanged( const Reference< presentation::XSlideShowView >& rView,
                       const Reference< rendering::XBitmap >& rLeavingBitmap,
                       const Reference< rendering::XBitmap >& rEnteringBitmap ) override;

protected:
    void disposeTextures();

    // WeakComponentImplHelperBase
    virtual void SAL_CALL disposing() override;

    bool isDisposed() const
    {
        return (rBHelper.bDisposed || rBHelper.bInDispose);
    }

    void createTexture( GLuint* texID,
            bool useMipmap,
            uno::Sequence<sal_Int8>& data,
            const OGLFormat* pFormat );
    const OGLFormat* chooseFormats();

private:
    void impl_initializeFlags( bool const bGLXPresent );

    void impl_dispose();

    void setSlides( const Reference< rendering::XBitmap >& xLeavingSlide , const uno::Reference< rendering::XBitmap >& xEnteringSlide );
    void impl_prepareSlides();

    void impl_createTexture( bool useMipmap, uno::Sequence<sal_Int8>& data, const OGLFormat* pFormat );

    bool initWindowFromSlideShowView( const uno::Reference< presentation::XSlideShowView >& xView );
    /** After the window has been created, and the slides have been set, we'll initialize the slides with OpenGL.
    */
    void GLInitSlides();

    bool impl_prepareTransition();

private:
    rtl::Reference<OpenGLContext> mpContext;

    /** OpenGL handle to the leaving slide's texture
    */
    GLuint maLeavingSlideGL;
    /** OpenGL handle to the entering slide's texture
    */
    GLuint maEnteringSlideGL;

    Reference< presentation::XSlideShowView > mxView;
    Reference< rendering::XIntegerBitmap > mxLeavingBitmap;
    Reference< rendering::XIntegerBitmap > mxEnteringBitmap;

    /** raw bytes of the entering bitmap
    */
    uno::Sequence<sal_Int8> maEnteringBytes;

    /** raw bytes of the leaving bitmap
    */
    uno::Sequence<sal_Int8> maLeavingBytes;

    bool mbRestoreSync;

    /** the form the raw bytes are in for the bitmaps
    */
    rendering::IntegerBitmapLayout maSlideBitmapLayout;

    /** the size of the slides
    */
    geometry::IntegerSize2D maSlideSize;

    /** Our Transition to be used.
    */
    std::shared_ptr<OGLTransitionImpl> mpTransition;

public:
    /** whether we are running on ATI fglrx with bug related to textures
     */
    bool mbBrokenTexturesATI;

    /** GL version
     */
    float mnGLVersion;

    /**
       Whether the display has GLX extension on X11, always true otherwise (?)
     */
    bool mbValidOpenGLContext;

#if OSL_DEBUG_LEVEL > 0
    std::chrono::steady_clock::time_point m_UpdateStartTime;
    std::chrono::steady_clock::time_point m_UpdateEndTime;
    std::chrono::steady_clock::time_point m_StartTime;
    std::chrono::steady_clock::time_point m_EndTime;
    std::chrono::steady_clock::duration m_TotalUpdateDuration;
    int mnFrameCount;
#endif
};

bool OGLTransitionerImpl::initialize( const Reference< presentation::XSlideShowView >& xView,
        const Reference< rendering::XBitmap >& xLeavingSlide,
        const Reference< rendering::XBitmap >& xEnteringSlide )
{
    bool const bValidContext( initWindowFromSlideShowView( xView ) );
    impl_initializeFlags( bValidContext );

    setSlides( xLeavingSlide, xEnteringSlide );

    CHECK_GL_ERROR();
    return mbValidOpenGLContext;
}

void OGLTransitionerImpl::impl_initializeFlags( bool const bValidContext )
{
    CHECK_GL_ERROR();
    mbValidOpenGLContext = bValidContext;
    if ( bValidContext ) {
        mnGLVersion = OpenGLHelper::getGLVersion();
        SAL_INFO("slideshow.opengl", "GL version: " << mnGLVersion << "" );

#if defined( UNX ) && !defined( MACOSX )
        const GLubyte* vendor = glGetString( GL_VENDOR );
        /* TODO: check for version once the bug in fglrx driver is fixed */
        mbBrokenTexturesATI = (vendor && strcmp( reinterpret_cast<const char *>(vendor), "ATI Technologies Inc." ) == 0 );
#endif
    }
    CHECK_GL_ERROR();
}

bool OGLTransitionerImpl::initWindowFromSlideShowView( const Reference< presentation::XSlideShowView >& xView )
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return false;

    mxView.set( xView, UNO_QUERY );
    if( !mxView.is() )
        return false;

#if OSL_DEBUG_LEVEL > 0
    TimerContext aTimerContext("initWindowFromSlideShowView");
#endif

    /// take the XSlideShowView and extract the parent window from it. see viewmediashape.cxx
    uno::Reference< rendering::XCanvas > xCanvas(mxView->getCanvas(), uno::UNO_QUERY_THROW);
    uno::Sequence< uno::Any > aDeviceParams;
    ::canvas::tools::getDeviceInfo( xCanvas, aDeviceParams );

    OUString aImplName;
    aDeviceParams[ 0 ] >>= aImplName;

    sal_Int64 aVal = 0;
    aDeviceParams[1] >>= aVal;

    mpContext = OpenGLContext::Create();

    if( !mpContext->init( reinterpret_cast< vcl::Window* >( aVal ) ) ) {
        mpContext->requestLegacyContext();
        if( !mpContext->init( reinterpret_cast< vcl::Window* >( aVal ) ) )
            return false;
    }
    SAL_INFO("slideshow.opengl", "created the context");

    mpContext->makeCurrent();
    CHECK_GL_ERROR();

    awt::Rectangle aCanvasArea = mxView->getCanvasArea();
    mpContext->setWinPosAndSize(Point(aCanvasArea.X, aCanvasArea.Y), Size(aCanvasArea.Width, aCanvasArea.Height));
    SAL_INFO("slideshow.opengl", "canvas area: " << aCanvasArea.X << "," << aCanvasArea.Y << " - " << aCanvasArea.Width << "x" << aCanvasArea.Height);

    CHECK_GL_ERROR();
    glEnable(GL_CULL_FACE);
    CHECK_GL_ERROR();
    glCullFace(GL_BACK);
    CHECK_GL_ERROR();
    glClearColor (0, 0, 0, 0);
    CHECK_GL_ERROR();
    glClear(GL_COLOR_BUFFER_BIT);
    CHECK_GL_ERROR();

    mpContext->swapBuffers();

    CHECK_GL_ERROR();

    return true;
}

void OGLTransitionerImpl::setSlides( const uno::Reference< rendering::XBitmap >& xLeavingSlide,
                                     const uno::Reference< rendering::XBitmap >& xEnteringSlide )
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed())
        return;

    mxLeavingBitmap.set( xLeavingSlide , UNO_QUERY_THROW );
    mxEnteringBitmap.set( xEnteringSlide , UNO_QUERY_THROW );

    maSlideSize = mxLeavingBitmap->getSize();
    SAL_INFO("slideshow.opengl", "leaving bitmap area: " << maSlideSize.Width << "x" << maSlideSize.Height);
    maSlideSize = mxEnteringBitmap->getSize();
    SAL_INFO("slideshow.opengl", "entering bitmap area: " << maSlideSize.Width << "x" << maSlideSize.Height);

    //to avoid annoying flashing under X entering and leaving slides with opengl effects set the leaving
    //bitmap as the background pixmap of the opengl child window and the entering bitmap as the background
    //pixmap of the non-opengl parent window. If any expose events occur around the start and end of
    //the transition then those windows are default filled by X with the desired start/end image so there's
    //no visible flash
    SystemChildWindow* pChildWindow = mpContext->getChildWindow();
    if (!pChildWindow)
        return;

    css::uno::Reference<css::beans::XFastPropertySet> xEnteringFastPropertySet(mxEnteringBitmap, css::uno::UNO_QUERY);
    css::uno::Reference<css::beans::XFastPropertySet> xLeavingFastPropertySet(mxLeavingBitmap, css::uno::UNO_QUERY);
    css::uno::Sequence<css::uno::Any> aEnteringBitmap;
    css::uno::Sequence<css::uno::Any> aLeavingBitmap;
    if (xEnteringFastPropertySet.get() && xLeavingFastPropertySet.get())
    {
        xEnteringFastPropertySet->getFastPropertyValue(1) >>= aEnteringBitmap;
        xLeavingFastPropertySet->getFastPropertyValue(1) >>= aLeavingBitmap;
    }
    if (aEnteringBitmap.getLength() == 3 && aLeavingBitmap.getLength() == 3)
        pChildWindow->SetLeaveEnterBackgrounds(aLeavingBitmap, aEnteringBitmap);
}


void OGLTransitionerImpl::impl_prepareSlides()
{
    geometry::IntegerRectangle2D aSlideRect;
    aSlideRect.X1 = 0;
    aSlideRect.X2 = maSlideSize.Width;
    aSlideRect.Y1 = 0;
    aSlideRect.Y2 = maSlideSize.Height;

    CHECK_GL_ERROR();
    mpContext->sync();
    CHECK_GL_ERROR();

    maLeavingBytes = mxLeavingBitmap->getData(maSlideBitmapLayout, aSlideRect);
    maEnteringBytes = mxEnteringBitmap->getData(maSlideBitmapLayout, aSlideRect);

    CHECK_GL_ERROR();
    GLInitSlides();

    SAL_WARN_IF(maSlideBitmapLayout.PlaneStride != 0, "slideshow.opengl","only handle no plane stride now");

    mpContext->sync();

    CHECK_GL_ERROR();

    // synchronized X still gives us much smoother play
    // I suspect some issues in above code in slideshow
    // synchronize whole transition for now
    const GLWindow& rGLWindow(mpContext->getOpenGLWindow());
    mbRestoreSync = rGLWindow.Synchronize(true);
}

bool OGLTransitionerImpl::impl_prepareTransition()
{
    if( mpTransition && mpTransition->getSettings().mnRequiredGLVersion <= mnGLVersion )
        return mpTransition->prepare( maLeavingSlideGL, maEnteringSlideGL, mpContext.get() );
    return false;
}

bool OGLTransitionerImpl::setTransition( const std::shared_ptr<OGLTransitionImpl>& pTransition )
{
    if ( mpTransition ) // already initialized
        return true;

    mpTransition = pTransition;

    mpContext->makeCurrent();
    CHECK_GL_ERROR();

    bool succeeded = impl_prepareTransition();
    if (!succeeded) {
        mpTransition = nullptr;
        return false;
    }

    impl_prepareSlides();
    return true;
}

void OGLTransitionerImpl::createTexture( GLuint* texID,
                     bool useMipmap,
                     uno::Sequence<sal_Int8>& data,
                     const OGLFormat* pFormat )
{
    CHECK_GL_ERROR();
    glDeleteTextures( 1, texID );
    glGenTextures( 1, texID );
    glBindTexture( GL_TEXTURE_2D, *texID );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER );
    glTexParameteri( GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER );
    CHECK_GL_ERROR();

    impl_createTexture( useMipmap, data, pFormat );

    SAL_WARN_IF(!glIsTexture(*texID), "slideshow.opengl", "Can't generate Leaving slide textures in OpenGL");
    CHECK_GL_ERROR();
}

class OGLColorSpace : public cppu::WeakImplHelper< css::rendering::XIntegerBitmapColorSpace >
{
private:
    uno::Sequence< sal_Int8 >  maComponentTags;
    uno::Sequence< sal_Int32 > maBitCounts;

    virtual sal_Int8 SAL_CALL getType(  ) override
    {
        return rendering::ColorSpaceType::RGB;
    }
    virtual uno::Sequence< sal_Int8 > SAL_CALL getComponentTags(  ) override
    {
        return maComponentTags;
    }
    virtual sal_Int8 SAL_CALL getRenderingIntent(  ) override
    {
        return rendering::RenderingIntent::PERCEPTUAL;
    }
    virtual uno::Sequence< beans::PropertyValue > SAL_CALL getProperties(  ) override
    {
        return uno::Sequence< beans::PropertyValue >();
    }
    virtual uno::Sequence< double > SAL_CALL convertColorSpace( const uno::Sequence< double >& deviceColor,
                                                                const uno::Reference< rendering::XColorSpace >& targetColorSpace ) override
    {
        // TODO(P3): if we know anything about target
        // colorspace, this can be greatly sped up
        uno::Sequence<rendering::ARGBColor> aIntermediate(
            convertToARGB(deviceColor));
        return targetColorSpace->convertFromARGB(aIntermediate);
    }
    virtual uno::Sequence< rendering::RGBColor > SAL_CALL convertToRGB( const uno::Sequence< double >& deviceColor ) override
    {
        const double*  pIn( deviceColor.getConstArray() );
        const std::size_t nLen( deviceColor.getLength() );
        ENSURE_ARG_OR_THROW2(nLen%4==0,
                                "number of channels no multiple of 4",
                                static_cast<rendering::XColorSpace*>(this), 0);

        uno::Sequence< rendering::RGBColor > aRes(nLen/4);
        rendering::RGBColor* pOut( aRes.getArray() );
        for( std::size_t i=0; i<nLen; i+=4 )
        {
            *pOut++ = rendering::RGBColor(pIn[0],pIn[1],pIn[2]);
            pIn += 4;
        }
        return aRes;
    }
    virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToARGB( const uno::Sequence< double >& deviceColor ) override
    {
        const double*  pIn( deviceColor.getConstArray() );
        const std::size_t nLen( deviceColor.getLength() );
        ENSURE_ARG_OR_THROW2(nLen%4==0,
                                "number of channels no multiple of 4",
                                static_cast<rendering::XColorSpace*>(this), 0);

        uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
        rendering::ARGBColor* pOut( aRes.getArray() );
        for( std::size_t i=0; i<nLen; i+=4 )
        {
            *pOut++ = rendering::ARGBColor(pIn[3],pIn[0],pIn[1],pIn[2]);
            pIn += 4;
        }
        return aRes;
    }
    virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertToPARGB( const uno::Sequence< double >& deviceColor ) override
    {
        const double*  pIn( deviceColor.getConstArray() );
        const std::size_t nLen( deviceColor.getLength() );
        ENSURE_ARG_OR_THROW2(nLen%4==0,
                                "number of channels no multiple of 4",
                                static_cast<rendering::XColorSpace*>(this), 0);

        uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
        rendering::ARGBColor* pOut( aRes.getArray() );
        for( std::size_t i=0; i<nLen; i+=4 )
        {
            *pOut++ = rendering::ARGBColor(pIn[3],pIn[3]*pIn[0],pIn[3]*pIn[1],pIn[3]*pIn[2]);
            pIn += 4;
        }
        return aRes;
    }
    virtual uno::Sequence< double > SAL_CALL convertFromRGB( const uno::Sequence< rendering::RGBColor >& rgbColor ) override
    {
        const rendering::RGBColor* pIn( rgbColor.getConstArray() );
        const std::size_t             nLen( rgbColor.getLength() );

        uno::Sequence< double > aRes(nLen*4);
        double* pColors=aRes.getArray();
        for( std::size_t i=0; i<nLen; ++i )
        {
            *pColors++ = pIn->Red;
            *pColors++ = pIn->Green;
            *pColors++ = pIn->Blue;
            *pColors++ = 1.0;
            ++pIn;
        }
        return aRes;
    }
    virtual uno::Sequence< double > SAL_CALL convertFromARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) override
    {
        const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
        const std::size_t              nLen( rgbColor.getLength() );

        uno::Sequence< double > aRes(nLen*4);
        double* pColors=aRes.getArray();
        for( std::size_t i=0; i<nLen; ++i )
        {
            *pColors++ = pIn->Red;
            *pColors++ = pIn->Green;
            *pColors++ = pIn->Blue;
            *pColors++ = pIn->Alpha;
            ++pIn;
        }
        return aRes;
    }
    virtual uno::Sequence< double > SAL_CALL convertFromPARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) override
    {
        const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
        const std::size_t              nLen( rgbColor.getLength() );

        uno::Sequence< double > aRes(nLen*4);
        double* pColors=aRes.getArray();
        for( std::size_t i=0; i<nLen; ++i )
        {
            *pColors++ = pIn->Red/pIn->Alpha;
            *pColors++ = pIn->Green/pIn->Alpha;
            *pColors++ = pIn->Blue/pIn->Alpha;
            *pColors++ = pIn->Alpha;
            ++pIn;
        }
        return aRes;
    }

    // XIntegerBitmapColorSpace
    virtual sal_Int32 SAL_CALL getBitsPerPixel(  ) override
    {
        return 32;
    }
    virtual uno::Sequence< sal_Int32 > SAL_CALL getComponentBitCounts(  ) override
    {
        return maBitCounts;
    }
    virtual sal_Int8 SAL_CALL getEndianness(  ) override
    {
        return util::Endianness::LITTLE;
    }
    virtual uno::Sequence<double> SAL_CALL convertFromIntegerColorSpace( const uno::Sequence< sal_Int8 >& deviceColor,
                                                                            const uno::Reference< rendering::XColorSpace >& targetColorSpace ) override
    {
        if( dynamic_cast<OGLColorSpace*>(targetColorSpace.get()) )
        {
            const sal_Int8* pIn( deviceColor.getConstArray() );
            const std::size_t  nLen( deviceColor.getLength() );
            ENSURE_ARG_OR_THROW2(nLen%4==0,
                                    "number of channels no multiple of 4",
                                    static_cast<rendering::XColorSpace*>(this), 0);

            uno::Sequence<double> aRes(nLen);
            double* pOut( aRes.getArray() );
            for( std::size_t i=0; i<nLen; i+=4 )
            {
                *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
                *pOut++ = vcl::unotools::toDoubleColor(*pIn++);
            }
            return aRes;
        }
        else
        {
            // TODO(P3): if we know anything about target
            // colorspace, this can be greatly sped up
            uno::Sequence<rendering::ARGBColor> aIntermediate(
                convertIntegerToARGB(deviceColor));
            return targetColorSpace->convertFromARGB(aIntermediate);
        }
    }
    virtual uno::Sequence< sal_Int8 > SAL_CALL convertToIntegerColorSpace( const uno::Sequence< sal_Int8 >& deviceColor,
                                                                                const uno::Reference< rendering::XIntegerBitmapColorSpace >& targetColorSpace ) override
    {
        if( dynamic_cast<OGLColorSpace*>(targetColorSpace.get()) )
        {
            // it's us, so simply pass-through the data
            return deviceColor;
        }
        else
        {
            // TODO(P3): if we know anything about target
            // colorspace, this can be greatly sped up
            uno::Sequence<rendering::ARGBColor> aIntermediate(
                convertIntegerToARGB(deviceColor));
            return targetColorSpace->convertIntegerFromARGB(aIntermediate);
        }
    }
    virtual uno::Sequence< rendering::RGBColor > SAL_CALL convertIntegerToRGB( const uno::Sequence< sal_Int8 >& deviceColor ) override
    {
        const sal_Int8* pIn( deviceColor.getConstArray() );
        const std::size_t  nLen( deviceColor.getLength() );
        ENSURE_ARG_OR_THROW2(nLen%4==0,
                                "number of channels no multiple of 4",
                                static_cast<rendering::XColorSpace*>(this), 0);

        uno::Sequence< rendering::RGBColor > aRes(nLen/4);
        rendering::RGBColor* pOut( aRes.getArray() );
        for( std::size_t i=0; i<nLen; i+=4 )
        {
            *pOut++ = rendering::RGBColor(
                vcl::unotools::toDoubleColor(pIn[0]),
                vcl::unotools::toDoubleColor(pIn[1]),
                vcl::unotools::toDoubleColor(pIn[2]));
            pIn += 4;
        }
        return aRes;
    }

    virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertIntegerToARGB( const uno::Sequence< sal_Int8 >& deviceColor ) override
    {
        const sal_Int8* pIn( deviceColor.getConstArray() );
        const std::size_t  nLen( deviceColor.getLength() );
        ENSURE_ARG_OR_THROW2(nLen%4==0,
                                "number of channels no multiple of 4",
                                static_cast<rendering::XColorSpace*>(this), 0);

        uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
        rendering::ARGBColor* pOut( aRes.getArray() );
        for( std::size_t i=0; i<nLen; i+=4 )
        {
            *pOut++ = rendering::ARGBColor(
                vcl::unotools::toDoubleColor(pIn[3]),
                vcl::unotools::toDoubleColor(pIn[0]),
                vcl::unotools::toDoubleColor(pIn[1]),
                vcl::unotools::toDoubleColor(pIn[2]));
            pIn += 4;
        }
        return aRes;
    }

    virtual uno::Sequence< rendering::ARGBColor > SAL_CALL convertIntegerToPARGB( const uno::Sequence< sal_Int8 >& deviceColor ) override
    {
        const sal_Int8* pIn( deviceColor.getConstArray() );
        const std::size_t  nLen( deviceColor.getLength() );
        ENSURE_ARG_OR_THROW2(nLen%4==0,
                                "number of channels no multiple of 4",
                                static_cast<rendering::XColorSpace*>(this), 0);

        uno::Sequence< rendering::ARGBColor > aRes(nLen/4);
        rendering::ARGBColor* pOut( aRes.getArray() );
        for( std::size_t i=0; i<nLen; i+=4 )
        {
            const sal_Int8 nAlpha( pIn[3] );
            *pOut++ = rendering::ARGBColor(
                vcl::unotools::toDoubleColor(nAlpha),
                vcl::unotools::toDoubleColor(nAlpha*pIn[0]),
                vcl::unotools::toDoubleColor(nAlpha*pIn[1]),
                vcl::unotools::toDoubleColor(nAlpha*pIn[2]));
            pIn += 4;
        }
        return aRes;
    }

    virtual uno::Sequence< sal_Int8 > SAL_CALL convertIntegerFromRGB( const uno::Sequence< rendering::RGBColor >& rgbColor ) override
    {
        const rendering::RGBColor* pIn( rgbColor.getConstArray() );
        const std::size_t             nLen( rgbColor.getLength() );

        uno::Sequence< sal_Int8 > aRes(nLen*4);
        sal_Int8* pColors=aRes.getArray();
        for( std::size_t i=0; i<nLen; ++i )
        {
            *pColors++ = vcl::unotools::toByteColor(pIn->Red);
            *pColors++ = vcl::unotools::toByteColor(pIn->Green);
            *pColors++ = vcl::unotools::toByteColor(pIn->Blue);
            *pColors++ = -1;
            ++pIn;
        }
        return aRes;
    }

    virtual uno::Sequence< sal_Int8 > SAL_CALL convertIntegerFromARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) override
    {
        const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
        const std::size_t              nLen( rgbColor.getLength() );

        uno::Sequence< sal_Int8 > aRes(nLen*4);
        sal_Int8* pColors=aRes.getArray();
        for( std::size_t i=0; i<nLen; ++i )
        {
            *pColors++ = vcl::unotools::toByteColor(pIn->Red);
            *pColors++ = vcl::unotools::toByteColor(pIn->Green);
            *pColors++ = vcl::unotools::toByteColor(pIn->Blue);
            *pColors++ = vcl::unotools::toByteColor(pIn->Alpha);
            ++pIn;
        }
        return aRes;
    }

    virtual uno::Sequence< sal_Int8 > SAL_CALL convertIntegerFromPARGB( const uno::Sequence< rendering::ARGBColor >& rgbColor ) override
    {
        const rendering::ARGBColor* pIn( rgbColor.getConstArray() );
        const std::size_t              nLen( rgbColor.getLength() );

        uno::Sequence< sal_Int8 > aRes(nLen*4);
        sal_Int8* pColors=aRes.getArray();
        for( std::size_t i=0; i<nLen; ++i )
        {
            *pColors++ = vcl::unotools::toByteColor(pIn->Red/pIn->Alpha);
            *pColors++ = vcl::unotools::toByteColor(pIn->Green/pIn->Alpha);
            *pColors++ = vcl::unotools::toByteColor(pIn->Blue/pIn->Alpha);
            *pColors++ = vcl::unotools::toByteColor(pIn->Alpha);
            ++pIn;
        }
        return aRes;
    }

public:
    OGLColorSpace() :
        maComponentTags(4),
        maBitCounts(4)
    {
        sal_Int8*  pTags = maComponentTags.getArray();
        sal_Int32* pBitCounts = maBitCounts.getArray();
        pTags[0] = rendering::ColorComponentTag::RGB_RED;
        pTags[1] = rendering::ColorComponentTag::RGB_GREEN;
        pTags[2] = rendering::ColorComponentTag::RGB_BLUE;
        pTags[3] = rendering::ColorComponentTag::ALPHA;

        pBitCounts[0] =
        pBitCounts[1] =
        pBitCounts[2] =
        pBitCounts[3] = 8;
    }
};

struct OGLColorSpaceHolder : public rtl::StaticWithInit<uno::Reference<rendering::XIntegerBitmapColorSpace>, OGLColorSpaceHolder>
{
    uno::Reference<rendering::XIntegerBitmapColorSpace> operator()()
    {
        return new OGLColorSpace();
    }
};

uno::Reference<rendering::XIntegerBitmapColorSpace> const &
getOGLColorSpace()
{
    return OGLColorSpaceHolder::get();
}

void buildMipmaps(
    GLint internalFormat, GLsizei width, GLsizei height, GLenum format,
    GLenum type, const void * data)
{
    if (epoxy_has_gl_extension("GL_ARB_framebuffer_object")) {
        glTexImage2D(
            GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type,
            data);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_TRUE);
        glTexImage2D(
            GL_TEXTURE_2D, 0, internalFormat, width, height, 0, format, type,
            data);
        glTexParameteri(GL_TEXTURE_2D, GL_GENERATE_MIPMAP, GL_FALSE);
    }
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(
        GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
}

void OGLTransitionerImpl::impl_createTexture(
                     bool useMipmap,
                     uno::Sequence<sal_Int8>& data,
                     const OGLFormat* pFormat )
{
    if( !pFormat )
    {
        CHECK_GL_ERROR();
        // force-convert color to ARGB8888 int color space
        uno::Sequence<sal_Int8> tempBytes(
            maSlideBitmapLayout.ColorSpace->convertToIntegerColorSpace(
                data,
                getOGLColorSpace()));
        buildMipmaps(     GL_RGBA,
                          maSlideSize.Width,
                          maSlideSize.Height,
                          GL_RGBA,
                          GL_UNSIGNED_BYTE,
                          &tempBytes[0]);

        //anistropic filtering (to make texturing not suck when looking at polygons from oblique angles)
        GLfloat largest_supported_anisotropy;
        glGetFloatv(GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy);
        glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy);
    } else {
        if( mpTransition && !mbBrokenTexturesATI && !useMipmap) {
            glTexImage2D( GL_TEXTURE_2D, 0, pFormat->nInternalFormat, maSlideSize.Width, maSlideSize.Height, 0, pFormat->eFormat, pFormat->eType, &data[0] );
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
            glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
        } else {
            buildMipmaps( pFormat->nInternalFormat, maSlideSize.Width, maSlideSize.Height, pFormat->eFormat, pFormat->eType, &data[0] );

            //anistropic filtering (to make texturing not suck when looking at polygons from oblique angles)
            GLfloat largest_supported_anisotropy;
            glGetFloatv( GL_MAX_TEXTURE_MAX_ANISOTROPY_EXT, &largest_supported_anisotropy );
            glTexParameterf( GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, largest_supported_anisotropy );
        }
    }
    CHECK_GL_ERROR();
}

const OGLFormat* OGLTransitionerImpl::chooseFormats()
{
    const OGLFormat* pDetectedFormat=nullptr;
    uno::Reference<rendering::XIntegerBitmapColorSpace> xIntColorSpace(
        maSlideBitmapLayout.ColorSpace);

    if( xIntColorSpace->getType() == rendering::ColorSpaceType::RGB ||
        xIntColorSpace->getType() == rendering::ColorSpaceType::SRGB )
    {
        /* table for canvas->OGL format mapping. outer index is number
           of color components (0:3, 1:4), then comes bits per pixel
           (0:16, 1:24, 2:32), then channel ordering: (0:rgba, 1:bgra,
           2:argb, 3:abgr)
         */
        static const OGLFormat lcl_RGB24[] =
        {
            // 24 bit RGB
            {3, GL_BGR, GL_UNSIGNED_BYTE},
            {3, GL_RGB, GL_UNSIGNED_BYTE},
            {3, GL_BGR, GL_UNSIGNED_BYTE},
            {3, GL_RGB, GL_UNSIGNED_BYTE}
        };

#if defined(GL_VERSION_1_2) && defined(GLU_VERSION_1_3)
        // more format constants available
        static const OGLFormat lcl_RGB16[] =
        {
            // 16 bit RGB
            {3, GL_RGB, GL_UNSIGNED_SHORT_5_6_5_REV},
            {3, GL_RGB, GL_UNSIGNED_SHORT_5_6_5},
            {3, GL_RGB, GL_UNSIGNED_SHORT_5_6_5_REV},
            {3, GL_RGB, GL_UNSIGNED_SHORT_5_6_5}
        };

        static const OGLFormat lcl_ARGB16_4[] =
        {
            // 16 bit ARGB
            {4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4_REV},
            {4, GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4_REV},
            {4, GL_BGRA, GL_UNSIGNED_SHORT_4_4_4_4},
            {4, GL_RGBA, GL_UNSIGNED_SHORT_4_4_4_4}
        };

        static const OGLFormat lcl_ARGB16_5[] =
        {
            // 16 bit ARGB
            {4, GL_RGBA, GL_UNSIGNED_SHORT_1_5_5_5_REV},
            {4, GL_BGRA, GL_UNSIGNED_SHORT_1_5_5_5_REV},
            {4, GL_BGRA, GL_UNSIGNED_SHORT_5_5_5_1},
            {4, GL_RGBA, GL_UNSIGNED_SHORT_5_5_5_1}
        };

        static const OGLFormat lcl_ARGB32[] =
        {
            // 32 bit ARGB
            {4, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8_REV},
            {4, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8_REV},
            {4, GL_BGRA, GL_UNSIGNED_INT_8_8_8_8},
            {4, GL_RGBA, GL_UNSIGNED_INT_8_8_8_8}
        };

        const uno::Sequence<sal_Int8> aComponentTags(
            xIntColorSpace->getComponentTags());
        const uno::Sequence<sal_Int32> aComponentBitcounts(
            xIntColorSpace->getComponentBitCounts());
        const sal_Int32 nNumComponents( aComponentBitcounts.getLength() );
        const sal_Int32 nBitsPerPixel( xIntColorSpace->getBitsPerPixel() );

        // supported component ordering?
        const int nComponentOrderIndex(
            calcComponentOrderIndex(aComponentTags));
        if( nComponentOrderIndex != -1 )
        {
            switch( nBitsPerPixel )
            {
                case 16:
                    if( nNumComponents == 3 )
                    {
                        pDetectedFormat = &lcl_RGB16[nComponentOrderIndex];
                    }
                    else if( nNumComponents == 4 )
                    {
                        if( aComponentBitcounts[1] == 4 )
                        {
                            pDetectedFormat = &lcl_ARGB16_4[nComponentOrderIndex];
                        }
                        else if( aComponentBitcounts[1] == 5 )
                        {
                            pDetectedFormat = &lcl_ARGB16_5[nComponentOrderIndex];
                        }
                    }
                    break;
                case 24:
                    if( nNumComponents == 3 )
                    {
                        pDetectedFormat = &lcl_RGB24[nComponentOrderIndex];
                    }
                    break;
                case 32:
                    if ( nNumComponents == 4 )
                    {
                        pDetectedFormat = &lcl_ARGB32[nComponentOrderIndex];
                    }
                    break;
            }
        }
#else
        const uno::Sequence<sal_Int8> aComponentTags(
            xIntColorSpace->getComponentTags());
        const int nComponentOrderIndex(calcComponentOrderIndex(aComponentTags));
        if( aComponentTags.getLength() == 3 &&
            nComponentOrderIndex != -1 &&
            xIntColorSpace->getBitsPerPixel() == 24 )
        {
            pDetectedFormat = &lcl_RGB24[nComponentOrderIndex];
        }
#endif
    }

    return pDetectedFormat;
}

void OGLTransitionerImpl::GLInitSlides()
{
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed() || !mpTransition || mpTransition->getSettings().mnRequiredGLVersion > mnGLVersion)
        return;

#if OSL_DEBUG_LEVEL > 0
    TimerContext aTimerContext("texture creation");
#endif

    mpContext->makeCurrent();

    const OGLFormat* pFormat = chooseFormats();

    CHECK_GL_ERROR();
    createTexture( &maLeavingSlideGL,
           mpTransition->getSettings().mbUseMipMapLeaving,
           maLeavingBytes,
           pFormat );

    createTexture( &maEnteringSlideGL,
           mpTransition->getSettings().mbUseMipMapEntering,
           maEnteringBytes,
           pFormat );

    CHECK_GL_ERROR();
    mpContext->sync();
    CHECK_GL_ERROR();
}

void SAL_CALL OGLTransitionerImpl::update( double nTime )
{
#if OSL_DEBUG_LEVEL > 0
    mnFrameCount ++;
    m_UpdateStartTime = std::chrono::steady_clock::now();
    if( mnFrameCount == 1 ) {
        m_StartTime = m_UpdateStartTime;
        m_TotalUpdateDuration = std::chrono::seconds(0);
    }
#endif
    osl::MutexGuard const guard( m_aMutex );

    if (isDisposed() || !mbValidOpenGLContext || !mpTransition || mpTransition->getSettings().mnRequiredGLVersion > mnGLVersion)
        return;

    mpContext->makeCurrent();
    CHECK_GL_ERROR();

    glEnable(GL_DEPTH_TEST);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    CHECK_GL_ERROR();

    const GLWindow& rGLWindow(mpContext->getOpenGLWindow());
    mpTransition->display(nTime, maLeavingSlideGL, maEnteringSlideGL,
                          maSlideSize.Width, maSlideSize.Height,
                          static_cast<double>(rGLWindow.Width),
                          static_cast<double>(rGLWindow.Height),
                          mpContext.get());

    mpContext->swapBuffers();

    mpContext->show();
    mpContext->sync();
    CHECK_GL_ERROR();

#if OSL_DEBUG_LEVEL > 0
    m_UpdateEndTime = std::chrono::steady_clock::now();

    SAL_INFO("slideshow.opengl", "update time: " << nTime);
    SAL_INFO("slideshow.opengl", "update took: " << std::chrono::duration_cast<std::chrono::milliseconds>(m_UpdateEndTime - m_UpdateStartTime).count());
    m_TotalUpdateDuration += m_UpdateEndTime - m_UpdateStartTime;
#endif
}

void SAL_CALL OGLTransitionerImpl::viewChanged( const Reference< presentation::XSlideShowView >& rView,
                        const Reference< rendering::XBitmap >& rLeavingBitmap,
                        const Reference< rendering::XBitmap >& rEnteringBitmap )
{
    SAL_INFO("slideshow.opengl", "transitioner: view changed");

    impl_dispose();

    initWindowFromSlideShowView( rView );
    setSlides( rLeavingBitmap, rEnteringBitmap );
    impl_prepareSlides();
    impl_prepareTransition();
}

void OGLTransitionerImpl::disposeTextures()
{
    mpContext->makeCurrent();
    CHECK_GL_ERROR();

    glDeleteTextures(1,&maLeavingSlideGL);
    maLeavingSlideGL = 0;
    glDeleteTextures(1,&maEnteringSlideGL);
    maEnteringSlideGL = 0;

    CHECK_GL_ERROR();
}

void OGLTransitionerImpl::impl_dispose()
{
    mpContext->makeCurrent();
    CHECK_GL_ERROR();

    if( mpTransition && mpTransition->getSettings().mnRequiredGLVersion <= mnGLVersion )
        mpTransition->finish();
    disposeTextures();
    if( mpContext.is() )
        mpContext->dispose();
    mpContext.clear();
}

// we are about to be disposed (someone call dispose() on us)
void OGLTransitionerImpl::disposing()
{
    osl::MutexGuard const guard( m_aMutex );

#if OSL_DEBUG_LEVEL > 0
    SAL_INFO("slideshow.opengl", "dispose " << this);
    if( mnFrameCount ) {
        m_EndTime = std::chrono::steady_clock::now();
        auto const duration = m_EndTime - m_StartTime;
        SAL_INFO("slideshow.opengl",
                "whole transition (frames: " << mnFrameCount
                << ") took: " << std::chrono::duration_cast<std::chrono::microseconds>(duration).count()
                << " fps: "
                << ((static_cast<double>(mnFrameCount)*1000000000.0)/std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count())
                << " time spent in updates: " << std::chrono::duration_cast<std::chrono::microseconds>(m_TotalUpdateDuration).count()
                << " percentage of transition time: "
                << (100*((static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(m_TotalUpdateDuration).count()))/(static_cast<double>(std::chrono::duration_cast<std::chrono::nanoseconds>(duration).count()))))
                << '%'
            );
    }
#endif

    if (mbRestoreSync && mpContext.is()) {
        // try to reestablish synchronize state
        const char* sal_synchronize = getenv("SAL_SYNCHRONIZE");
        mpContext->getOpenGLWindow().Synchronize(sal_synchronize && *sal_synchronize == '1' );
    }

    impl_dispose();

    mpTransition.reset();

    mxLeavingBitmap.clear();
    mxEnteringBitmap.clear();
    mxView.clear();
}

OGLTransitionerImpl::OGLTransitionerImpl()
    : OGLTransitionerImplBase(m_aMutex)
    , mpContext()
    , maLeavingSlideGL(0)
    , maEnteringSlideGL(0)
    , mxView()
    , maEnteringBytes()
    , maLeavingBytes()
    , mbRestoreSync(false)
    , maSlideBitmapLayout()
    , maSlideSize()
    , mbBrokenTexturesATI(false)
    , mnGLVersion(0)
    , mbValidOpenGLContext(false)
#if OSL_DEBUG_LEVEL > 0
    , mnFrameCount(0)
#endif
{
}

typedef cppu::WeakComponentImplHelper<presentation::XTransitionFactory> OGLTransitionFactoryImplBase;

class OGLTransitionFactoryImpl : private cppu::BaseMutex, public OGLTransitionFactoryImplBase
{
public:
    explicit OGLTransitionFactoryImpl( const uno::Reference< uno::XComponentContext >& ) :
        OGLTransitionFactoryImplBase(m_aMutex)
    {}

    // XTransitionFactory
    virtual sal_Bool SAL_CALL hasTransition( sal_Int16 transitionType, sal_Int16 transitionSubType ) override
    {
        // A set of css::animation::TransitionSubType that don't have any meaning (in the SMIL 2.0
        // standard) for MISCSHAPEWIPE have been chosen to refer to some of these "fancy" optional
        // transitions. (The only subtypes of 'miscShapeWipe' defined in the standard are 'heart'
        // and 'keyhole'.) The set of subtypes used seems to be a bit random; it starts from the
        // beginning of the list (in the order (numeric) in our TransitionSubType set of constants)
        // but then jumps a bit randomly. The numeric values as such have no meaning, but still.

        if( transitionType == animations::TransitionType::MISCSHAPEWIPE ) {
            switch( transitionSubType )
            {
                case animations::TransitionSubType::LEFTTORIGHT:        //   1
                case animations::TransitionSubType::TOPTOBOTTOM:        //   2
                case animations::TransitionSubType::TOPLEFT:            //   3
                case animations::TransitionSubType::TOPRIGHT:           //   4
                case animations::TransitionSubType::BOTTOMRIGHT:        //   5
                case animations::TransitionSubType::BOTTOMLEFT:         //   6
                case animations::TransitionSubType::TOPCENTER:          //   7
                case animations::TransitionSubType::RIGHTCENTER:        //   8
                case animations::TransitionSubType::BOTTOMCENTER:       //   9
                case animations::TransitionSubType::CORNERSIN:          //  11
                case animations::TransitionSubType::CORNERSOUT:         //  12
                case animations::TransitionSubType::VERTICAL:           //  13
                case animations::TransitionSubType::HORIZONTAL:         //  14
                case animations::TransitionSubType::DIAMOND:            //  26
                case animations::TransitionSubType::CIRCLE:             //  27
                case animations::TransitionSubType::HEART:              //  31
                case animations::TransitionSubType::FANOUTHORIZONTAL:   //  55
                case animations::TransitionSubType::ACROSS:             // 108
                    return true;

                default:
                    return false;
            }
        } else if( transitionType == animations::TransitionType::FADE && transitionSubType == animations::TransitionSubType::CROSSFADE ) {
            return true;
        } else if( transitionType == animations::TransitionType::FADE && transitionSubType == animations::TransitionSubType::FADEOVERCOLOR ) {
            return true;
        } else if( transitionType == animations::TransitionType::IRISWIPE && transitionSubType == animations::TransitionSubType::DIAMOND ) {
            return true;
        } else if( transitionType == animations::TransitionType::ZOOM && transitionSubType == animations::TransitionSubType::ROTATEIN ) {
            return true;
        } else
            return false;
    }

    virtual uno::Reference< presentation::XTransition > SAL_CALL createTransition(
        sal_Int16                                             transitionType,
        sal_Int16                                             transitionSubType,
        const uno::Reference< presentation::XSlideShowView >& view,
        const uno::Reference< rendering::XBitmap >&           leavingBitmap,
        const uno::Reference< rendering::XBitmap >&           enteringBitmap ) override
    {
        if( !hasTransition( transitionType, transitionSubType ) )
            return uno::Reference< presentation::XTransition >();

        rtl::Reference< OGLTransitionerImpl > xRes( new OGLTransitionerImpl() );
        if ( !xRes->initialize( view, leavingBitmap, enteringBitmap ) )
            return uno::Reference< presentation::XTransition >();

        std::shared_ptr<OGLTransitionImpl> pTransition;

        if( transitionType == animations::TransitionType::MISCSHAPEWIPE ) {
            switch( transitionSubType )
                {
                case animations::TransitionSubType::LEFTTORIGHT:
                    pTransition = makeFallLeaving();
                    break;
                case animations::TransitionSubType::TOPTOBOTTOM:
                    pTransition = makeTurnAround();
                    break;
                case animations::TransitionSubType::TOPLEFT:
                    pTransition = makeIris();
                    break;
                case animations::TransitionSubType::TOPRIGHT:
                    pTransition = makeTurnDown();
                    break;
                case animations::TransitionSubType::BOTTOMRIGHT:
                    pTransition = makeRochade();
                    break;
                case animations::TransitionSubType::BOTTOMLEFT:
                    pTransition = makeVenetianBlinds( true, 8 );
                    break;
                case animations::TransitionSubType::TOPCENTER:
                    pTransition = makeVenetianBlinds( false, 6 );
                    break;
                case animations::TransitionSubType::RIGHTCENTER:
                    pTransition = makeStatic();
                    break;
                case animations::TransitionSubType::BOTTOMCENTER:
                    pTransition = makeDissolve();
                    break;
                case animations::TransitionSubType::CORNERSIN:
                    pTransition = makeInsideCubeFaceToLeft();
                    break;
                case animations::TransitionSubType::CORNERSOUT:
                    pTransition = makeOutsideCubeFaceToLeft();
                    break;
                case animations::TransitionSubType::VERTICAL:
                    pTransition = makeVortex();
                    break;
                case animations::TransitionSubType::HORIZONTAL:
                    pTransition = makeRipple();
                    break;
                case animations::TransitionSubType::CIRCLE:
                    pTransition = makeRevolvingCircles(8,128);
                    break;
                case animations::TransitionSubType::FANOUTHORIZONTAL:
                    pTransition = makeHelix(20);
                    break;
                case animations::TransitionSubType::ACROSS:
                    pTransition = makeNByMTileFlip(8,6);
                    break;
                case animations::TransitionSubType::DIAMOND:
                    pTransition = makeGlitter();
                    break;
                case animations::TransitionSubType::HEART:
                    pTransition = makeHoneycomb();
                    break;
                }
        } else if( transitionType == animations::TransitionType::FADE && transitionSubType == animations::TransitionSubType::CROSSFADE ) {
            pTransition = makeFadeSmoothly();
        } else if( transitionType == animations::TransitionType::FADE && transitionSubType == animations::TransitionSubType::FADEOVERCOLOR ) {
            pTransition = makeFadeThroughBlack();
        } else if( transitionType == animations::TransitionType::IRISWIPE && transitionSubType == animations::TransitionSubType::DIAMOND ) {
            pTransition = makeDiamond();
        } else if( transitionType == animations::TransitionType::ZOOM && transitionSubType == animations::TransitionSubType::ROTATEIN ) {
            pTransition = makeNewsflash();
        }

        if ( !pTransition || !xRes->setTransition(pTransition) )
            return uno::Reference< presentation::XTransition >();

        return uno::Reference<presentation::XTransition>(xRes.get());
    }
};

}

namespace sdecl = comphelper::service_decl;
 const sdecl::ServiceDecl OGLTransitionFactoryDecl(
     sdecl::class_<OGLTransitionFactoryImpl>(),
    "com.sun.star.comp.presentation.OGLTransitionFactory",
    "com.sun.star.presentation.TransitionFactory" );

// The C shared lib entry points
extern "C"
SAL_DLLPUBLIC_EXPORT void* ogltrans_component_getFactory( sal_Char const* pImplName,
                                         void*, void* )
{
    return sdecl::component_getFactoryHelper( pImplName, {&OGLTransitionFactoryDecl} );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
