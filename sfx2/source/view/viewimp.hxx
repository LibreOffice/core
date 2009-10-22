/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: viewimp.hxx,v $
 * $Revision: 1.16 $
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

#ifndef _VIEWIMP_HXX
#define _VIEWIMP_HXX

// include ---------------------------------------------------------------

#ifndef __SBX_SBXOBJ_HXX
#include <basic/sbxobj.hxx>
#endif
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>                  // SvBorder
#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.hxx>

#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <svtools/acceleratorexecute.hxx>

#include <rtl/ref.hxx>

#include <queue>

// forward ---------------------------------------------------------------

class SfxOfficeDispatch;
class SfxBaseController;

typedef SfxShell* SfxShellPtr_Impl;
SV_DECL_PTRARR( SfxShellArr_Impl, SfxShellPtr_Impl, 4, 4 )

// struct SfxViewShell_Impl ----------------------------------------------
class SfxAsyncPrintExec_Impl : public SfxListener
{
    SfxViewShell*                   pView;
    ::std::queue < SfxRequest*>     aReqs;

    virtual void                    Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

public:
                                    SfxAsyncPrintExec_Impl( SfxViewShell* pShell)
                                        : pView( pShell )
                                    {}

    void                            AddRequest( SfxRequest& rReq );
};

class SfxClipboardChangeListener;

struct SfxViewShell_Impl
{
    ::osl::Mutex                aMutex;
    ::cppu::OInterfaceContainerHelper aInterceptorContainer;
    BOOL                        bControllerSet;
    SfxShellArr_Impl            aArr;
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
    USHORT                      nFamily;
    SfxBaseController*          pController;
    ::svt::AcceleratorExecute*  pAccExec;
    SfxAsyncPrintExec_Impl*     pPrinterCommandQueue;
    com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > aPrintOpts;
    ::rtl::Reference< SfxClipboardChangeListener > xClipboardListener;

    SfxViewShell_Impl();
};

#endif

