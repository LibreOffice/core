/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: progressbar.hxx,v $
 * $Revision: 1.5 $
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

#define UNO3_REFERENCE                  ::com::sun::star::uno::Reference
#define UNO3_XMULTISERVICEFACTORY       ::com::sun::star::lang::XMultiServiceFactory
#define UNO3_RUNTIMEEXCEPTION           ::com::sun::star::uno::RuntimeException
#define UNO3_XCONTROLMODEL              ::com::sun::star::awt::XControlModel
#define UNO3_XPROGRESSBAR               ::com::sun::star::awt::XProgressBar
#define UNO3_ANY                        ::com::sun::star::uno::Any
#define UNO3_TYPE                       ::com::sun::star::uno::Type
#define UNO3_SEQUENCE                   ::com::sun::star::uno::Sequence
#define UNO3_XGRAPHICS                  ::com::sun::star::awt::XGraphics
#define UNO3_OUSTRING                   ::rtl::OUString
#define UNO3_SIZE                       ::com::sun::star::awt::Size

//____________________________________________________________________________________________________________
//  defines
//____________________________________________________________________________________________________________

#define SERVICENAME_PROGRESSBAR             "com.sun.star.awt.XProgressBar"
#define IMPLEMENTATIONNAME_PROGRESSBAR      "stardiv.UnoControls.ProgressBar"
#define FREESPACE                           4
#define DEFAULT_HORIZONTAL                  sal_True
#define DEFAULT_BLOCKDIMENSION              Size(1,1)
#define DEFAULT_BACKGROUNDCOLOR             TRGB_COLORDATA( 0x00, 0xC0, 0xC0, 0xC0 )    // lightgray
#define DEFAULT_FOREGROUNDCOLOR             TRGB_COLORDATA( 0x00, 0x00, 0x00, 0x80 )    // blue
#define DEFAULT_MINRANGE                    INT_MIN
#define DEFAULT_MAXRANGE                    INT_MAX
#define DEFAULT_BLOCKVALUE                  1
#define DEFAULT_VALUE                       DEFAULT_MINRANGE
#define LINECOLOR_BRIGHT                    TRGB_COLORDATA( 0x00, 0xFF, 0xFF, 0xFF )    // white
#define LINECOLOR_SHADOW                    TRGB_COLORDATA( 0x00, 0x00, 0x00, 0x00 )    // black

//____________________________________________________________________________________________________________
//  classes
//____________________________________________________________________________________________________________

class ProgressBar   : public UNO3_XCONTROLMODEL
                    , public UNO3_XPROGRESSBAR
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

    ProgressBar( const UNO3_REFERENCE< UNO3_XMULTISERVICEFACTORY >& xFactory );

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

    virtual UNO3_ANY SAL_CALL queryInterface( const UNO3_TYPE& aType ) throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual UNO3_SEQUENCE< UNO3_TYPE > SAL_CALL getTypes() throw( UNO3_RUNTIMEEXCEPTION );

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

    UNO3_ANY SAL_CALL queryAggregation( const UNO3_TYPE& aType ) throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual void SAL_CALL setForegroundColor( sal_Int32 nColor ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL setBackgroundColor( sal_Int32 nColor ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL setValue( sal_Int32 nValue ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual void SAL_CALL setRange( sal_Int32   nMin    ,
                                    sal_Int32   nMax    ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual sal_Int32 SAL_CALL getValue() throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual void SAL_CALL setPosSize(   sal_Int32   nX      ,
                                        sal_Int32   nY      ,
                                        sal_Int32   nWidth  ,
                                        sal_Int32   nHeight ,
                                        sal_Int16   nFlags  ) throw( UNO3_RUNTIMEEXCEPTION );

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

    virtual sal_Bool SAL_CALL setModel( const UNO3_REFERENCE< UNO3_XCONTROLMODEL >& xModel ) throw( UNO3_RUNTIMEEXCEPTION );

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    virtual UNO3_REFERENCE< UNO3_XCONTROLMODEL > SAL_CALL getModel() throw( UNO3_RUNTIMEEXCEPTION );

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

    static const UNO3_SEQUENCE< UNO3_OUSTRING > impl_getStaticSupportedServiceNames();

    /**_________________________________________________________________________________________________________
        @short
        @descr

        @seealso

        @param

        @return

        @onerror
    */

    static const UNO3_OUSTRING impl_getStaticImplementationName();

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

    virtual void impl_paint(        sal_Int32                           nX          ,
                                    sal_Int32                           nY          ,
                            const   UNO3_REFERENCE< UNO3_XGRAPHICS >&   xGraphics   );

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

    sal_Bool    m_bHorizontal       ;   // orientation for steps            [true=horizontal/false=vertikal]
    UNO3_SIZE   m_aBlockSize        ;   // width and height of a block      [>=0,0]
    sal_Int32   m_nForegroundColor  ;   //                                  (alpha,r,g,b)
    sal_Int32   m_nBackgroundColor  ;   //                                  (alpha,r,g,b)
    sal_Int32   m_nMinRange         ;   // lowest value  =   0%             [long, <_nMaxRange]
    sal_Int32   m_nMaxRange         ;   // highest value = 100%             [long, >_nMinRange]
    double      m_nBlockValue       ;   // value for one block              [long, >0]
    sal_Int32   m_nValue            ;   // value for progress               [long]

};  // class ProgressBar

}   // namespace unocontrols

#endif  // #ifndef _UNOCONTROLS_PROGRESSBAR_CTRL_HXX
