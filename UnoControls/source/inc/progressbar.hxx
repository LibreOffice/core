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

#ifndef _UNOCONTROLS_PROGRESSBAR_CTRL_HXX
#define _UNOCONTROLS_PROGRESSBAR_CTRL_HXX

//____________________________________________________________________________________________________________
//  includes of other projects
//____________________________________________________________________________________________________________

#include <com/sun/star/lang/XServiceName.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>

//____________________________________________________________________________________________________________
//  includes of my own project
//____________________________________________________________________________________________________________
#include "basecontrol.hxx"

//____________________________________________________________________________________________________________
//  namespaces
//____________________________________________________________________________________________________________

namespace unocontrols{

#define CSS_UNO     ::com::sun::star::uno
#define CSS_LANG    ::com::sun::star::lang
#define CSS_AWT     ::com::sun::star::awt

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

class ProgressBar   : public CSS_AWT::XControlModel
                    , public CSS_AWT::XProgressBar
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

    ProgressBar( const CSS_UNO::Reference< CSS_LANG::XMultiServiceFactory >& xFactory );

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

    virtual CSS_UNO::Any SAL_CALL queryInterface( const CSS_UNO::Type& aType )
        throw( CSS_UNO::RuntimeException );

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

    virtual CSS_UNO::Sequence< CSS_UNO::Type > SAL_CALL getTypes()
        throw( CSS_UNO::RuntimeException );

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

    CSS_UNO::Any SAL_CALL queryAggregation( const CSS_UNO::Type& aType )
        throw( CSS_UNO::RuntimeException );

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
        throw( CSS_UNO::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL setBackgroundColor( sal_Int32 nColor )
        throw( CSS_UNO::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL setValue( sal_Int32 nValue ) throw( CSS_UNO::RuntimeException );

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
    ) throw( CSS_UNO::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual sal_Int32 SAL_CALL getValue() throw( CSS_UNO::RuntimeException );

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
    ) throw( CSS_UNO::RuntimeException );

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
        const CSS_UNO::Reference< CSS_AWT::XControlModel >& xModel
    ) throw( CSS_UNO::RuntimeException );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual CSS_UNO::Reference< CSS_AWT::XControlModel > SAL_CALL getModel()
        throw( CSS_UNO::RuntimeException );

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

    static const CSS_UNO::Sequence< ::rtl::OUString > impl_getStaticSupportedServiceNames();

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
        const CSS_UNO::Reference< CSS_AWT::XGraphics >& xGraphics
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
    CSS_AWT::Size   m_aBlockSize        ;   // width and height of a block      [>=0,0]
    sal_Int32       m_nForegroundColor  ;   //                                  (alpha,r,g,b)
    sal_Int32       m_nBackgroundColor  ;   //                                  (alpha,r,g,b)
    sal_Int32       m_nMinRange         ;   // lowest value  =   0%             [long, <_nMaxRange]
    sal_Int32       m_nMaxRange         ;   // highest value = 100%             [long, >_nMinRange]
    double          m_nBlockValue       ;   // value for one block              [long, >0]
    sal_Int32       m_nValue            ;   // value for progress               [long]

};  // class ProgressBar

// The namespace aliases are only used in the header
#undef CSS_UNO
#undef CSS_LANG
#undef CSS_AWT

}   // namespace unocontrols

#endif  // #ifndef _UNOCONTROLS_PROGRESSBAR_CTRL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
