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

#ifndef _UNOCONTROLS_PROGRESSBAR_CTRL_HXX
#define _UNOCONTROLS_PROGRESSBAR_CTRL_HXX

#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

#include "basecontrol.hxx"

//____________________________________________________________________________________________________________
//  namespaces
//____________________________________________________________________________________________________________

namespace unocontrols{

//____________________________________________________________________________________________________________
//  defines
//____________________________________________________________________________________________________________

#define SERVICENAME_PROGRESSBAR             "com.sun.star.awt.XProgressBar"
#define IMPLEMENTATIONNAME_PROGRESSBAR      "stardiv.UnoControls.ProgressBar"
#define PROGRESSBAR_FREESPACE               4
#define PROGRESSBAR_DEFAULT_HORIZONTAL      sal_True
#define PROGRESSBAR_DEFAULT_BLOCKDIMENSION  Size(1,1)
#define PROGRESSBAR_DEFAULT_BACKGROUNDCOLOR TRGB_COLORDATA( 0x00, 0xC0, 0xC0, 0xC0 )    // lightgray
#define PROGRESSBAR_DEFAULT_FOREGROUNDCOLOR TRGB_COLORDATA( 0x00, 0x00, 0x00, 0x80 )    // blue
#define PROGRESSBAR_DEFAULT_MINRANGE        INT_MIN
#define PROGRESSBAR_DEFAULT_MAXRANGE        INT_MAX
#define PROGRESSBAR_DEFAULT_BLOCKVALUE      1
#define PROGRESSBAR_DEFAULT_VALUE           PROGRESSBAR_DEFAULT_MINRANGE
#define PROGRESSBAR_LINECOLOR_BRIGHT        TRGB_COLORDATA( 0x00, 0xFF, 0xFF, 0xFF )    // white
#define PROGRESSBAR_LINECOLOR_SHADOW        TRGB_COLORDATA( 0x00, 0x00, 0x00, 0x00 )    // black

//____________________________________________________________________________________________________________
//  classes
//____________________________________________________________________________________________________________

class ProgressBar   : public ::com::sun::star::awt::XControlModel
                    , public ::com::sun::star::awt::XProgressBar
                    , public BaseControl
{

//____________________________________________________________________________________________________________
//  public methods
//____________________________________________________________________________________________________________

public:

    //________________________________________________________________________________________________________
    //  construct/destruct
    //________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    ProgressBar( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& xFactory );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ~ProgressBar();

    //__________________________________________________________________________________________________________
    //  XInterface
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
        throw( ::com::sun::star::uno::RuntimeException );

    /**_______________________________________________________________________________________________________
        @short      increment refcount
        @descr      -

        @seealso    XInterface
        @seealso    release()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL acquire() throw();

    /**_______________________________________________________________________________________________________
        @short      decrement refcount
        @descr      -

        @seealso    XInterface
        @seealso    acquire()

        @param      -

        @return     -

        @onerror    A RuntimeException is thrown.
    */

    virtual void SAL_CALL release() throw();

    //__________________________________________________________________________________________________________
    //  XTypeProvider
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Type > SAL_CALL getTypes()
        throw( ::com::sun::star::uno::RuntimeException );

    //__________________________________________________________________________________________________________
    //  XAggregation
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    ::com::sun::star::uno::Any SAL_CALL queryAggregation( const ::com::sun::star::uno::Type& aType )
        throw( ::com::sun::star::uno::RuntimeException );

    //________________________________________________________________________________________________________
    //  XProgressBar
    //________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL setForegroundColor( sal_Int32 nColor )
        throw( ::com::sun::star::uno::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL setBackgroundColor( sal_Int32 nColor )
        throw( ::com::sun::star::uno::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL setValue( sal_Int32 nValue ) throw( ::com::sun::star::uno::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL setRange(
        sal_Int32   nMin    ,
        sal_Int32   nMax
    ) throw( ::com::sun::star::uno::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual sal_Int32 SAL_CALL getValue() throw( ::com::sun::star::uno::RuntimeException );

    //__________________________________________________________________________________________________________
    //  XWindow
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL setPosSize(
        sal_Int32   nX      ,
        sal_Int32   nY      ,
        sal_Int32   nWidth  ,
        sal_Int32   nHeight ,
        sal_Int16   nFlags
    ) throw( ::com::sun::star::uno::RuntimeException );

    //__________________________________________________________________________________________________________
    //  XControl
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual sal_Bool SAL_CALL setModel(
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel >& xModel
    ) throw( ::com::sun::star::uno::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > SAL_CALL getModel()
        throw( ::com::sun::star::uno::RuntimeException );

    //__________________________________________________________________________________________________________
    //  BaseControl
    //__________________________________________________________________________________________________________

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    static const ::com::sun::star::uno::Sequence< ::rtl::OUString > impl_getStaticSupportedServiceNames();

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    static const ::rtl::OUString impl_getStaticImplementationName();

//____________________________________________________________________________________________________________
//  protected methods
//____________________________________________________________________________________________________________

protected:

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void impl_paint(
        sal_Int32 nX ,
        sal_Int32 nY ,
        const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XGraphics >& xGraphics
    );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    void impl_recalcRange();

//____________________________________________________________________________________________________________
//  private variables
//____________________________________________________________________________________________________________

private:

    sal_Bool        m_bHorizontal       ;   // orientation for steps            [true=horizontal/false=vertikal]
    ::com::sun::star::awt::Size   m_aBlockSize        ;   // width and height of a block      [>=0,0]
    sal_Int32       m_nForegroundColor  ;   //                                  (alpha,r,g,b)
    sal_Int32       m_nBackgroundColor  ;   //                                  (alpha,r,g,b)
    sal_Int32       m_nMinRange         ;   // lowest value  =   0%             [long, <_nMaxRange]
    sal_Int32       m_nMaxRange         ;   // highest value = 100%             [long, >_nMinRange]
    double          m_nBlockValue       ;   // value for one block              [long, >0]
    sal_Int32       m_nValue            ;   // value for progress               [long]

};  // class ProgressBar

}   // namespace unocontrols

#endif  // #ifndef _UNOCONTROLS_PROGRESSBAR_CTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
