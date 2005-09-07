/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: viewimp.hxx,v $
 *
 *  $Revision: 1.12 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:32:30 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _VIEWIMP_HXX
#define _VIEWIMP_HXX

// include ---------------------------------------------------------------

#ifndef __SBX_SBXOBJ_HXX
#include <basic/sbxobj.hxx>
#endif
#include "viewsh.hxx"
#include "viewfrm.hxx"                  // SvBorder
#include <osl/mutex.hxx>
#ifndef _CPPUHELPER_INTERFACECONTAINER_HXX_
#include <cppuhelper/interfacecontainer.hxx>
#endif

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>

#ifndef INCLUDED_SVTOOLS_ACCELERATOREXECUTE_HXX
#include <svtools/acceleratorexecute.hxx>
#endif

// forward ---------------------------------------------------------------

class SfxMenuBarManager;
class SfxAcceleratorManager;
class SfxOfficeDispatch;
class SfxBaseController;

typedef SfxShell* SfxShellPtr_Impl;
SV_DECL_PTRARR( SfxShellArr_Impl, SfxShellPtr_Impl, 4, 4 );

// struct SfxViewShell_Impl ----------------------------------------------

struct SfxViewShell_Impl
{
    ::osl::Mutex               aMutex;
    ::cppu::OInterfaceContainerHelper aInterceptorContainer;
    BOOL                        bControllerSet;
    SfxShellArr_Impl            aArr;
    SbxObjectRef                xSelectionObj;
    SvBorder                    aBorder;
    Size                        aOptimalSize;
    Size                        aMargin;
    USHORT                      nPrinterLocks;
    BOOL                        bUseObjectSize;
    BOOL                        bCanPrint;
    BOOL                        bHasPrintOptions;
    BOOL                        bPlugInsActive;
    BOOL                        bIsShowView;
    BOOL                        bFrameSetImpl;
    BOOL                        bOwnsMenu;
    BOOL                        bGotOwnerShip;
    BOOL                        bGotFrameOwnerShip;
    SfxScrollingMode            eScroll;
    ResId*                      pMenuBarResId;
    SfxMenuBarManager*          pMenu;
    ResId*                      pAccelResId;
    SfxAcceleratorManager*      pAccel;
    USHORT                      nFamily;
    SfxBaseController*          pController;
    ::svt::AcceleratorExecute*  pAccExec;
    com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > aPrintOpts;

                                SfxViewShell_Impl()
                                  : aInterceptorContainer( aMutex )
                                  , pAccExec(0)
                                {}
};


#endif

