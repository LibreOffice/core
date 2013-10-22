/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "DummyXShape.hxx"
#include "CommonConverters.hxx"
#include <rtl/ustring.hxx>

#include <vcl/window.hxx>
#include <tools/gen.hxx>

#include <algorithm>

using namespace com::sun::star;

namespace chart {

namespace dummy {

DummyXShape::DummyXShape():
    mpParent(NULL)
{
}

OUString DummyXShape::getName()
    throw(uno::RuntimeException)
{
    return maName;
}

void DummyXShape::setName( const OUString& rName )
    throw(uno::RuntimeException)
{
    maName = rName;
}

awt::Point DummyXShape::getPosition()
    throw(uno::RuntimeException)
{
    return maPosition;
}

void DummyXShape::setPosition( const awt::Point& rPoint )
    throw(uno::RuntimeException)
{
    maPosition = rPoint;
}

awt::Size DummyXShape::getSize()
    throw(uno::RuntimeException)
{
    return maSize;
}

void DummyXShape::setSize( const awt::Size& rSize )
    throw(beans::PropertyVetoException, uno::RuntimeException)
{
    maSize = rSize;
}

OUString DummyXShape::getShapeType()
    throw(uno::RuntimeException)
{
    return OUString("dummy shape");
}
uno::Reference< beans::XPropertySetInfo > DummyXShape::getPropertySetInfo()
    throw(uno::RuntimeException)
{
    return uno::Reference< beans::XPropertySetInfo >();
}

void DummyXShape::setPropertyValue( const OUString& rName, const uno::Any& rValue)
    throw(beans::UnknownPropertyException, beans::PropertyVetoException,
            lang::IllegalArgumentException, lang::WrappedTargetException,
            uno::RuntimeException)
{
    SAL_DEBUG("DummyXShape::setProperty: " << rName << " " << "Any");
    maProperties[rName] = rValue;
}

uno::Any DummyXShape::getPropertyValue( const OUString& rName )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
    SAL_DEBUG("DummyXShape::getPropertyValue: " << rName);
    std::map<OUString, uno::Any>::iterator itr = maProperties.find(rName);
    if(itr != maProperties.end())
        return itr->second;

    return uno::Any();
}

void DummyXShape::addPropertyChangeListener( const OUString&, const uno::Reference< beans::XPropertyChangeListener >& )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}

void DummyXShape::removePropertyChangeListener( const OUString&, const uno::Reference< beans::XPropertyChangeListener >& )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}

void DummyXShape::addVetoableChangeListener( const OUString&, const uno::Reference< beans::XVetoableChangeListener >& )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}

void DummyXShape::removeVetoableChangeListener( const OUString&, const uno::Reference< beans::XVetoableChangeListener >& )
    throw(beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException)
{
}

void DummyXShape::setPropertyValues( const uno::Sequence< OUString >& ,
        const uno::Sequence< uno::Any >& )
throw (beans::PropertyVetoException, lang::IllegalArgumentException,
        lang::WrappedTargetException, uno::RuntimeException)
{
}

uno::Sequence< uno::Any > DummyXShape::getPropertyValues(
        const uno::Sequence< OUString >& rNames)
    throw (uno::RuntimeException)
{
    uno::Sequence< uno::Any > aValues(rNames.getLength());
    for(sal_Int32 i = 0; i < rNames.getLength(); ++i)
    {
        OUString aName = rNames[i];

        std::map<OUString, uno::Any>::iterator itr = maProperties.find(aName);
        if(itr != maProperties.end())
            aValues[i] = itr->second;
    }
    return aValues;
}

    void DummyXShape::addPropertiesChangeListener( const uno::Sequence< OUString >& , const uno::Reference< beans::XPropertiesChangeListener >& ) throw (uno::RuntimeException)
{
}

    void DummyXShape::removePropertiesChangeListener( const uno::Reference< beans::XPropertiesChangeListener >& ) throw (uno::RuntimeException)
{
}

void DummyXShape::firePropertiesChangeEvent( const uno::Sequence< OUString >& ,
        const uno::Reference< beans::XPropertiesChangeListener >& )
    throw (uno::RuntimeException)
{
}

OUString DummyXShape::getImplementationName()
    throw(uno::RuntimeException)
{
    return OUString("DummyXShape");
}

namespace {

uno::Sequence< OUString > listSupportedServices()
{
    static uno::Sequence< OUString > aSupportedServices;
    if(aSupportedServices.getLength() == 0)
    {
        aSupportedServices.realloc(3);
        aSupportedServices[0] = "com.sun.star.drawing.Shape";
        aSupportedServices[1] = "com.sun.star.container.Named";
        aSupportedServices[2] = "com.sun.star.beans.PropertySet";
    }

    return aSupportedServices;
}

}

uno::Sequence< OUString > DummyXShape::getSupportedServiceNames()
    throw(uno::RuntimeException)
{
    return listSupportedServices();
}

sal_Bool DummyXShape::supportsService( const OUString& rServiceName )
    throw(uno::RuntimeException)
{
    uno::Sequence< OUString > aSupportedServices = listSupportedServices();
    for(sal_Int32 i = 0; i < aSupportedServices.getLength(); ++i)
    {
        if(aSupportedServices[i] == rServiceName)
            return true;
    }

    return false;
}

uno::Reference< uno::XInterface > DummyXShape::getParent()
    throw(uno::RuntimeException)
{
    return mxParent;
}

void DummyXShape::setParent( const uno::Reference< uno::XInterface >& xParent )
    throw(lang::NoSupportException, uno::RuntimeException)
{
    mxParent = xParent;
}

namespace {

void setProperties( uno::Reference< beans::XPropertySet > xPropSet, const tPropertyNameMap& rPropertyNameMap,
        std::map<OUString, uno::Any>& rTargetMap )
{
    tNameSequence aNames;
    tAnySequence aValues;
    PropertyMapper::getMultiPropertyLists( aNames, aValues,
            xPropSet, rPropertyNameMap );

    sal_Int32 nSize = std::min(aNames.getLength(), aValues.getLength());
    for(sal_Int32 i = 0; i < nSize; ++i)
    {
        rTargetMap[aNames[i]] = aValues[i];
    }
}

}

DummyCube::DummyCube(const drawing::Position3D &rPos, const drawing::Direction3D& rSize,
        sal_Int32 nRotateZAngleHundredthDegree, const uno::Reference< beans::XPropertySet > xPropSet,
        const tPropertyNameMap& rPropertyNameMap, bool bRounded ):
    mnRotateZAngleHundredthDegree(nRotateZAngleHundredthDegree),
    mbRounded(bRounded)
{
    setPosition(Position3DToAWTPoint(rPos));
    setSize(Direction3DToAWTSize(rSize));
    setProperties(xPropSet, rPropertyNameMap, maProperties);
}

DummyCylinder::DummyCylinder(const drawing::Position3D& rPos, const drawing::Direction3D& rSize,
            sal_Int32 nRotateZAngleHundredthDegree ):
    mnRotateZAngleHundredthDegree(nRotateZAngleHundredthDegree)
{
    setPosition(Position3DToAWTPoint(rPos));
    setSize(Direction3DToAWTSize(rSize));
}

DummyPyramid::DummyPyramid(const drawing::Position3D& rPos, const drawing::Direction3D& rSize,
        double fTopHeight, bool bRotateZ, uno::Reference< beans::XPropertySet > xPropSet,
        const tPropertyNameMap& rPropertyNameMap):
    mfTopHeight(fTopHeight),
    mbRotateZ(bRotateZ)
{
    setPosition(Position3DToAWTPoint(rPos));
    setSize(Direction3DToAWTSize(rSize));
    setProperties(xPropSet, rPropertyNameMap, maProperties);
}

DummyCone::DummyCone(const drawing::Position3D& rPos, const drawing::Direction3D& rSize,
        double fTopHeight, sal_Int32 nRotateZAngleHundredthDegree):
    mnRotateZAngleHundredthDegree(nRotateZAngleHundredthDegree),
    mfTopHeight(fTopHeight)
{
    setPosition(Position3DToAWTPoint(rPos));
    setSize(Direction3DToAWTSize(rSize));
}

DummyPieSegment2D::DummyPieSegment2D(double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree,
        double fUnitCircleInnerRadius, double fUnitCircleOuterRadius,
        const drawing::Direction3D& rOffset, const drawing::HomogenMatrix& rUnitCircleToScene):
    mfUnitCircleStartAngleDegree(fUnitCircleStartAngleDegree),
    mfUnitCircleWidthAngleDegree(fUnitCircleWidthAngleDegree),
    mfUnitCircleInnerRadius(fUnitCircleInnerRadius),
    mfUnitCircleOuterRadius(fUnitCircleOuterRadius),
    maOffset(rOffset),
    maUnitCircleToScene(rUnitCircleToScene)
{
}

DummyPieSegment::DummyPieSegment(double fUnitCircleStartAngleDegree, double fUnitCircleWidthAngleDegree,
        double fUnitCircleInnerRadius, double fUnitCircleOuterRadius,
        const drawing::Direction3D& rOffset, const drawing::HomogenMatrix& rUnitCircleToScene,
        double fDepth ):
    mfUnitCircleStartAngleDegree(fUnitCircleStartAngleDegree),
    mfUnitCircleWidthAngleDegree(fUnitCircleWidthAngleDegree),
    mfUnitCircleInnerRadius(fUnitCircleInnerRadius),
    mfUnitCircleOuterRadius(fUnitCircleOuterRadius),
    maOffset(rOffset),
    maUnitCircleToScene(rUnitCircleToScene),
    mfDepth(fDepth)
{
}

DummyStripe::DummyStripe(const Stripe& rStripe, uno::Reference< beans::XPropertySet > xPropSet,
        const tPropertyNameMap& rPropertyNameMap, sal_Bool bDoubleSided,
        short nRotatedTexture, bool bFlatNormals ):
    maStripe(rStripe),
    mbDoubleSided(bDoubleSided),
    mnRotatedTexture(nRotatedTexture),
    mbFlatNormals(bFlatNormals)
{
    setProperties(xPropSet, rPropertyNameMap, maProperties);
}

DummyArea3D::DummyArea3D(const drawing::PolyPolygonShape3D& rShape, double fDepth):
    mfDepth(fDepth),
    maShapes(rShape)
{
}

DummyArea2D::DummyArea2D(const drawing::PolyPolygonShape3D& rShape):
    maShapes(rShape)
{
}

DummySymbol2D::DummySymbol2D(const drawing::Position3D& rPos, const drawing::Direction3D& rSize,
        sal_Int32 nStandardSymbol, sal_Int32 , sal_Int32 ):
    mnStandardSymbol(nStandardSymbol)
{
    setPosition(Position3DToAWTPoint(rPos));
    setSize(Direction3DToAWTSize(rSize));
}

DummyCircle::DummyCircle(const awt::Point& rPos, const awt::Size& rSize)
{
    setPosition(rPos);
    setSize(rSize);
}

DummyLine3D::DummyLine3D(const drawing::PolyPolygonShape3D& rPoints, const VLineProperties& ):
    maPoints(rPoints)
{
}

DummyLine2D::DummyLine2D(const drawing::PointSequenceSequence& rPoints, const VLineProperties* ):
    maPoints(rPoints)
{
}

DummyLine2D::DummyLine2D(const awt::Size& rSize, const awt::Point& rPosition)
{
    setPosition(rPosition);
    setSize(rSize);
}

DummyRectangle::DummyRectangle()
{
}

DummyRectangle::DummyRectangle(const awt::Size& rSize)
{
    setSize(rSize);
}

DummyRectangle::DummyRectangle(const awt::Size& rSize, const awt::Point& rPoint, const tNameSequence& ,
        const tAnySequence& )
{
    setSize(rSize);
    setPosition(rPoint);
}

DummyText::DummyText(const OUString& rText, const tNameSequence& ,
        const tAnySequence& , const uno::Any& rTrans ):
    maText(rText),
    maTrans(rTrans)
{
}

DummyGroup3D::DummyGroup3D(const OUString& rName)
{
    setName(rName);
}

DummyGroup2D::DummyGroup2D(const OUString& rName)
{
    setName(rName);
}

DummyGraphic2D::DummyGraphic2D(const drawing::Position3D& rPos, const drawing::Direction3D& rSize,
        const uno::Reference< graphic::XGraphic > xGraphic ):
    mxGraphic(xGraphic)
{
    setPosition(Position3DToAWTPoint(rPos));
    setSize(Direction3DToAWTSize(rSize));
}

DummyChart* DummyXShape::getRootShape()
{
    assert(mxParent.is());
    DummyXShape* pParent = dynamic_cast<DummyXShape*>(mxParent.get());
    assert(pParent);
    return pParent->getRootShape();
}

DummyChart* DummyChart::getRootShape()
{
    return this;
}

#define QUERYINT( xint ) \
    if( rType == ::getCppuType((const uno::Reference< xint >*)0) ) \
        aAny <<= uno::Reference< xint >(this)

uno::Any DummyXShapes::queryInterface( const uno::Type& rType )
    throw(uno::RuntimeException)
{
    return DummyXShape::queryInterface(rType);
}

uno::Any DummyXShapes::queryAggregation( const uno::Type & rType )
    throw(uno::RuntimeException)
{
    uno::Any aAny;

    //QUERYINT( drawing::XShapeGroup );
    QUERYINT( drawing::XShapes );
    else
        return DummyXShape::queryAggregation( rType );

    return aAny;
}

void DummyXShapes::acquire()
    throw()
{
    DummyXShape::acquire();
}

void DummyXShapes::release()
    throw()
{
    DummyXShape::release();
}

void DummyXShapes::add( const uno::Reference< drawing::XShape>& xShape )
    throw(uno::RuntimeException)
{
    DummyXShape* pChild = dynamic_cast<DummyXShape*>(xShape.get());
    assert(pChild);
    maUNOShapes.push_back(xShape);
    pChild->setParent(static_cast< ::cppu::OWeakObject* >( this ));
    maShapes.push_back(pChild);
}

void DummyXShapes::remove( const uno::Reference< drawing::XShape>& xShape )
    throw(uno::RuntimeException)
{
    std::vector< uno::Reference<drawing::XShape> >::iterator itr = std::find(maUNOShapes.begin(), maUNOShapes.end(), xShape);

    DummyXShape* pChild = dynamic_cast<DummyXShape*>((*itr).get());
    std::vector< DummyXShape* >::iterator itrShape = std::find(maShapes.begin(), maShapes.end(), pChild);
    if(itrShape != maShapes.end())
        maShapes.erase(itrShape);

    if(itr != maUNOShapes.end())
        maUNOShapes.erase(itr);
}

uno::Type DummyXShapes::getElementType()
    throw(uno::RuntimeException)
{
    return ::getCppuType(( const uno::Reference< drawing::XShape >*)0);
}

sal_Bool DummyXShapes::hasElements()
    throw(uno::RuntimeException)
{
    return !maUNOShapes.empty();
}

sal_Int32 DummyXShapes::getCount()
    throw(uno::RuntimeException)
{
    return maUNOShapes.size();
}

uno::Any DummyXShapes::getByIndex(sal_Int32 nIndex)
    throw(lang::IndexOutOfBoundsException, lang::WrappedTargetException,
            uno::RuntimeException)
{
    uno::Any aShape;
    aShape <<= maUNOShapes[nIndex];
    return aShape;
}

bool DummyChart::initWindow()
{
    const SystemEnvData* sysData(mpWindow->GetSystemData());
#if defined( WNT )
    GLWin.hWnd = sysData->hWnd;
#elif defined( UNX )
    GLWin.dpy = reinterpret_cast<unx::Display*>(sysData->pDisplay);

    if( unx::glXQueryExtension( GLWin.dpy, NULL, NULL ) == false )
        return false;

    GLWin.win = sysData->aWindow;

    OSL_TRACE("parent window: %d", GLWin.win);

    unx::XWindowAttributes xattr;
    unx::XGetWindowAttributes( GLWin.dpy, GLWin.win, &xattr );

    GLWin.screen = XScreenNumberOfScreen( xattr.screen );

    unx::XVisualInfo* vi( NULL );
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
    unx::XVisualInfo* visinfo;
    unx::XVisualInfo* firstVisual( NULL );
#endif
    static int attrList3[] =
    {
        GLX_RGBA,//only TrueColor or DirectColor
        //single buffered
        GLX_RED_SIZE,4,//use the maximum red bits, with a minimum of 4 bits
        GLX_GREEN_SIZE,4,//use the maximum green bits, with a minimum of 4 bits
        GLX_BLUE_SIZE,4,//use the maximum blue bits, with a minimum of 4 bits
        GLX_DEPTH_SIZE,0,//no depth buffer
        None
    };
    static int attrList2[] =
    {
        GLX_RGBA,//only TrueColor or DirectColor
        /// single buffered
        GLX_RED_SIZE,4,/// use the maximum red bits, with a minimum of 4 bits
        GLX_GREEN_SIZE,4,/// use the maximum green bits, with a minimum of 4 bits
        GLX_BLUE_SIZE,4,/// use the maximum blue bits, with a minimum of 4 bits
        GLX_DEPTH_SIZE,1,/// use the maximum depth bits, making sure there is a depth buffer
        None
    };
    static int attrList1[] =
    {
        GLX_RGBA,//only TrueColor or DirectColor
        GLX_DOUBLEBUFFER,/// only double buffer
        GLX_RED_SIZE,4,/// use the maximum red bits, with a minimum of 4 bits
        GLX_GREEN_SIZE,4,/// use the maximum green bits, with a minimum of 4 bits
        GLX_BLUE_SIZE,4,/// use the maximum blue bits, with a minimum of 4 bits
        GLX_DEPTH_SIZE,0,/// no depth buffer
        None
    };
    static int attrList0[] =
    {
        GLX_RGBA,//only TrueColor or DirectColor
        GLX_DOUBLEBUFFER,/// only double buffer
        GLX_RED_SIZE,4,/// use the maximum red bits, with a minimum of 4 bits
        GLX_GREEN_SIZE,4,/// use the maximum green bits, with a minimum of 4 bits
        GLX_BLUE_SIZE,4,/// use the maximum blue bits, with a minimum of 4 bits
        GLX_DEPTH_SIZE,1,/// use the maximum depth bits, making sure there is a depth buffer
        None
    };
    static int* attrTable[] =
    {
        attrList0,
        attrList1,
        attrList2,
        attrList3,
        NULL
    };
    int** pAttributeTable = attrTable;
    const SystemEnvData* pChildSysData = NULL;
    pWindow.reset();

#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
    unx::GLXFBConfig* fbconfigs = NULL;
    int nfbconfigs, value, i = 0;
#endif

    while( *pAttributeTable )
    {
        // try to find a visual for the current set of attributes
        vi = unx::glXChooseVisual( GLWin.dpy,
                GLWin.screen,
                *pAttributeTable );

#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
        if( vi ) {
            if( !firstVisual )
                firstVisual = vi;
            OSL_TRACE("trying VisualID %08X", vi->visualid);
            fbconfigs = glXGetFBConfigs (GLWin.dpy, GLWin.screen, &nfbconfigs);

            for ( ; i < nfbconfigs; i++)
            {
                visinfo = glXGetVisualFromFBConfig (GLWin.dpy, fbconfigs[i]);
                if( !visinfo || visinfo->visualid != vi->visualid )
                    continue;

                glXGetFBConfigAttrib (GLWin.dpy, fbconfigs[i], GLX_DRAWABLE_TYPE, &value);
                if (!(value & GLX_PIXMAP_BIT))
                    continue;

                glXGetFBConfigAttrib (GLWin.dpy, fbconfigs[i],
                        GLX_BIND_TO_TEXTURE_TARGETS_EXT,
                        &value);
                if (!(value & GLX_TEXTURE_2D_BIT_EXT))
                    continue;

                glXGetFBConfigAttrib (GLWin.dpy, fbconfigs[i],
                        GLX_BIND_TO_TEXTURE_RGB_EXT,
                        &value);
                if (value == sal_False)
                    continue;

                glXGetFBConfigAttrib (GLWin.dpy, fbconfigs[i],
                        GLX_BIND_TO_MIPMAP_TEXTURE_EXT,
                        &value);
                if (value == sal_False)
                    continue;

                // TODO: handle non Y inverted cases
                break;
            }

            if( i != nfbconfigs || ( firstVisual && pAttributeTable[1] == NULL ) ) {
                if( i != nfbconfigs ) {
                    vi = glXGetVisualFromFBConfig( GLWin.dpy, fbconfigs[i] );
                    // TODO:moggi
                    // mbHasTFPVisual = true;
                    OSL_TRACE("found visual suitable for texture_from_pixmap");
                } else {
                    vi = firstVisual;
                    // TODO:moggi
                    // mbHasTFPVisual = false;
                    OSL_TRACE("did not find visual suitable for texture_from_pixmap, using %08X", vi->visualid);
                }
#else
                if( vi ) {
#endif
                    SystemWindowData winData;
                    winData.nSize = sizeof(winData);
                    OSL_TRACE("using VisualID %08X", vi->visualid);
                    winData.pVisual = (void*)(vi->visual);
                    pWindow.reset(new SystemChildWindow(mpWindow.get(), 0, &winData, sal_False));
                    pChildSysData = pWindow->GetSystemData();

                    if( pChildSysData ) {
                        break;
                    } else {
                        pWindow.reset();
                    }
                }
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
            }
#endif

            ++pAttributeTable;
        }
#endif

#if defined( WNT )
        SystemWindowData winData;
        winData.nSize = sizeof(winData);
        pWindow.reset(new SystemChildWindow(mpWindow.get(), 0, &winData, sal_False));
#endif

        if( pWindow )
        {
            pWindow->SetMouseTransparent( sal_True );
            pWindow->SetParentClipMode( PARENTCLIPMODE_NOCLIP );
            pWindow->EnableEraseBackground( sal_False );
            pWindow->SetControlForeground();
            pWindow->SetControlBackground();
            pWindow->EnablePaint(sal_False);
#if defined( WNT )
            GLWin.hWnd = sysData->hWnd;
#elif defined( UNX )
            GLWin.dpy = reinterpret_cast<unx::Display*>(pChildSysData->pDisplay);
            GLWin.win = pChildSysData->aWindow;
#if defined( GLX_VERSION_1_3 ) && defined( GLX_EXT_texture_from_pixmap )
            //TODO: moggi
            /*
            if( mbHasTFPVisual )
                GLWin.fbc = fbconfigs[i];
                */
#endif
            GLWin.vi = vi;
            GLWin.GLXExtensions = unx::glXQueryExtensionsString( GLWin.dpy, GLWin.screen );
            OSL_TRACE("available GLX extensions: %s", GLWin.GLXExtensions);
#endif

        return false;
        }

    return true;
}

namespace {

static bool errorTriggered;
int oglErrorHandler( unx::Display* /*dpy*/, unx::XErrorEvent* /*evnt*/ )
{
    errorTriggered = true;

    return 0;
}

}

bool DummyChart::initOpengl()
{
    initWindow();
    mpWindow->setPosSizePixel(0,0,0,0);
    GLWin.Width = 0;
    GLWin.Height = 0;

#if defined( WNT )
    GLWin.hDC = GetDC(GLWin.hWnd);
#elif defined( UNX )
    GLWin.ctx = glXCreateContext(GLWin.dpy,
                                 GLWin.vi,
                                 0,
                                 GL_TRUE);
    if( GLWin.ctx == NULL )
    {
        OSL_TRACE("unable to create GLX context");
        return false;
    }
#endif

#if defined( WNT )
    PIXELFORMATDESCRIPTOR PixelFormatFront =                    // PixelFormat Tells Windows How We Want Things To Be
    {
        sizeof(PIXELFORMATDESCRIPTOR),
        1,                              // Version Number
        PFD_DRAW_TO_WINDOW |
        PFD_SUPPORT_OPENGL |
        PFD_DOUBLEBUFFER,
        PFD_TYPE_RGBA,                  // Request An RGBA Format
        (BYTE)32,                       // Select Our Color Depth
        0, 0, 0, 0, 0, 0,               // Color Bits Ignored
        0,                              // No Alpha Buffer
        0,                              // Shift Bit Ignored
        0,                              // No Accumulation Buffer
        0, 0, 0, 0,                     // Accumulation Bits Ignored
        64,                             // 32 bit Z-BUFFER
        0,                              // 0 bit stencil buffer
        0,                              // No Auxiliary Buffer
        0,                              // now ignored
        0,                              // Reserved
        0, 0, 0                         // Layer Masks Ignored
    };
    int WindowPix = ChoosePixelFormat(GLWin.hDC,&PixelFormatFront);
    SetPixelFormat(GLWin.hDC,WindowPix,&PixelFormatFront);
    GLWin.hRC  = wglCreateContext(GLWin.hDC);
    wglMakeCurrent(GLWin.hDC,GLWin.hRC);
#elif defined( UNX )
    if( !glXMakeCurrent( GLWin.dpy, GLWin.win, GLWin.ctx ) )
    {
        OSL_TRACE("unable to select current GLX context");
        return false;
    }

    int glxMinor, glxMajor;
    double nGLXVersion = 0;
    if( glXQueryVersion( GLWin.dpy, &glxMajor, &glxMinor ) )
      nGLXVersion = glxMajor + 0.1*glxMinor;
    OSL_TRACE("available GLX version: %f", nGLXVersion);

    GLWin.GLExtensions = glGetString( GL_EXTENSIONS );
    OSL_TRACE("available GL  extensions: %s", GLWin.GLExtensions);

    // TODO: moggi
    // mbTextureFromPixmap = GLWin.HasGLXExtension( "GLX_EXT_texture_from_pixmap" );
    // mbGenerateMipmap = GLWin.HasGLExtension( "GL_SGIS_generate_mipmap" );

    if( GLWin.HasGLXExtension("GLX_SGI_swap_control" ) )
    {
        // enable vsync
        typedef GLint (*glXSwapIntervalProc)(GLint);
        glXSwapIntervalProc glXSwapInterval = (glXSwapIntervalProc) unx::glXGetProcAddress( (const GLubyte*) "glXSwapIntervalSGI" );
        if( glXSwapInterval ) {
        int (*oldHandler)(unx::Display* /*dpy*/, unx::XErrorEvent* /*evnt*/);

        // replace error handler temporarily
        oldHandler = unx::XSetErrorHandler( oglErrorHandler );

        errorTriggered = false;

        glXSwapInterval( 1 );

        // sync so that we possibly get an XError
        unx::glXWaitGL();
        XSync(GLWin.dpy, false);

        if( errorTriggered )
            OSL_TRACE("error when trying to set swap interval, NVIDIA or Mesa bug?");
        else
            OSL_TRACE("set swap interval to 1 (enable vsync)");

        // restore the error handler
        unx::XSetErrorHandler( oldHandler );
        }
    }
#endif

    glEnable(GL_CULL_FACE);
    glCullFace(GL_BACK);
    glClearColor (0, 0, 0, 0);
    glClear(GL_COLOR_BUFFER_BIT);
#if defined( WNT )
    SwapBuffers(GLWin.hDC);
#elif defined( UNX )
    unx::glXSwapBuffers(GLWin.dpy, GLWin.win);
#endif

    glEnable(GL_LIGHTING);
    GLfloat light_direction[] = { 0.0 , 0.0 , 1.0 };
    GLfloat materialDiffuse[] = { 1.0 , 1.0 , 1.0 , 1.0};
    glLightfv(GL_LIGHT0, GL_SPOT_DIRECTION, light_direction);
    glMaterialfv(GL_FRONT,GL_DIFFUSE,materialDiffuse);
    glEnable(GL_LIGHT0);
    glEnable(GL_NORMALIZE);

    return true;
}

DummyChart::DummyChart():
    mpWindow(new Window(0, WB_NOBORDER|WB_NODIALOGCONTROL))
{
    setName("com.sun.star.chart2.shapes");
    createGLContext();
}

void DummyChart::createGLContext()
{
    initOpengl();
}

void DummyChart::setPosition( const awt::Point& aPosition )
    throw( uno::RuntimeException )
{
    DummyXShape::setPosition(aPosition);
}

void DummyChart::setSize( const awt::Size& aSize )
    throw( beans::PropertyVetoException, uno::RuntimeException )
{
    DummyXShape::setSize(aSize);
    mpWindow->SetSizePixel(Size(aSize.Width, aSize.Height));
    pWindow->SetSizePixel(Size(aSize.Width, aSize.Height));
}

}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
