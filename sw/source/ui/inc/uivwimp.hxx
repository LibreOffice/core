/*************************************************************************
 *
 *  $RCSfile: uivwimp.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: os $ $Date: 2000-11-07 14:39:02 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _UIVWIMP_HXX
#define _UIVWIMP_HXX

#ifndef _SWVIEW_HXX //autogen
#include <view.hxx>
#endif

#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // helper for implementations
#endif

class SwXTextView;
namespace com{namespace sun{namespace star{namespace frame { class XDispatchProviderInterceptor;}}}}
/* -----------------------------29.05.00 08:22--------------------------------

 ---------------------------------------------------------------------------*/
class SwScannerEventListener : public ::cppu::WeakImplHelper1< ::com::sun::star::lang::XEventListener >
{
private:

    SwView*                 m_pParent;

public:

                            SwScannerEventListener( SwView* pParent ) : m_pParent( pParent )  {};
    virtual                 ~SwScannerEventListener();

    // XEventListener
    virtual void SAL_CALL   disposing( const ::com::sun::star::lang::EventObject& rEventObject );

    void                    ParentDestroyed() { m_pParent = NULL; }
};
/* ---------------------------------------------------------------------------

 ---------------------------------------------------------------------------*/
class SwView_Impl
{
    SwView*                     pView;
    ::com::sun::star::uno::Reference< ::com::sun::star::view::XSelectionSupplier > *pxXTextView;        // UNO object
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XEventListener >      xScanEvtLstnr;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatchProviderInterceptor >   xDisProvInterceptor;
    SwScannerEventListener*     pScanEvtLstnr;
    ShellModes                  eShellMode;

    public:
        SwView_Impl(SwView* pShell);
        ~SwView_Impl();

        void                            SetShellMode(ShellModes eSet);

         ::com::sun::star::view::XSelectionSupplier* GetUNOObject();
        SwXTextView*                    GetUNOObject_Impl();

        ShellModes                      GetShellMode() {return eShellMode;}

        void                            ExcuteScan(USHORT nSlot);
        SwScannerEventListener&         GetScannerEventListener();
};
#endif

