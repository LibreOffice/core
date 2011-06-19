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

#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <cppuhelper/implbase2.hxx>
#include <cppuhelper/interfacecontainer.h>
#include <vcl/introwin.hxx>
#include <vcl/bitmapex.hxx>
#include <com/sun/star/lang/XSingleServiceFactory.hpp>
#include <osl/mutex.hxx>
#include <vcl/virdev.hxx>


using namespace ::rtl;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::task;

namespace desktop {

class  SplashScreen
    : public ::cppu::WeakImplHelper2< XStatusIndicator, XInitialization >
    , public IntroWindow
{
private:
    struct FullScreenProgressRatioValue
    {
        double _fXRelPos;
        double _fYRelPos;
        double _fRelWidth;
        double _fRelHeight;
    };
    enum BitmapMode { BM_FULLSCREEN, BM_DEFAULTMODE };

    // don't allow anybody but ourselves to create instances of this class
    SplashScreen(const SplashScreen&);
    SplashScreen(void);
    SplashScreen operator =(const SplashScreen&);

    SplashScreen(const Reference< XMultiServiceFactory >& xFactory);

    DECL_LINK( AppEventListenerHdl, VclWindowEvent * );
    virtual ~SplashScreen();
    void loadConfig();
    void updateStatus();
    void SetScreenBitmap(BitmapEx &rBitmap);
    void determineProgressRatioValues( double& rXRelPos, double& rYRelPos, double& rRelWidth, double& rRelHeight );

    static  SplashScreen *_pINSTANCE;

    static osl::Mutex _aMutex;
    Reference< XMultiServiceFactory > _rFactory;

    VirtualDevice   _vdev;
    BitmapEx        _aIntroBmp;
    Color           _cProgressFrameColor;
    Color           _cProgressBarColor;
    bool            _bNativeProgress;
    OUString        _sAppName;
    OUString        _sProgressText;
    std::vector< FullScreenProgressRatioValue > _sFullScreenProgressRatioValues;

    sal_Int32   _iMax;
    sal_Int32   _iProgress;
    BitmapMode  _eBitmapMode;
    sal_Bool    _bPaintBitmap;
    sal_Bool    _bPaintProgress;
    sal_Bool    _bVisible;
    sal_Bool    _bShowLogo;
    sal_Bool    _bFullScreenSplash;
    sal_Bool    _bProgressEnd;
    long _height, _width, _tlx, _tly, _barwidth;
    long _barheight, _barspace;
    double _fXPos, _fYPos;
    double _fWidth, _fHeight;
    const long _xoffset, _yoffset;

public:
    static const char* interfaces[];
    static const sal_Char *serviceName;
    static const sal_Char *implementationName;
    static const sal_Char *supportedServiceNames[];

    static Reference< XInterface > getInstance(const Reference < XMultiServiceFactory >& xFactory);

    // XStatusIndicator
    virtual void SAL_CALL end() throw ( RuntimeException );
    virtual void SAL_CALL reset() throw ( RuntimeException );
    virtual void SAL_CALL setText(const OUString& aText) throw ( RuntimeException );
    virtual void SAL_CALL setValue(sal_Int32 nValue) throw ( RuntimeException );
    virtual void SAL_CALL start(const OUString& aText, sal_Int32 nRange) throw ( RuntimeException );

    // XInitialize
    virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any>& aArguments )
        throw ( RuntimeException );

    // workwindow
    virtual void Paint( const Rectangle& );

};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
