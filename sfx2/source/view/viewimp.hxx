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

#ifndef SFX_VIEWIMP_HXX
#define SFX_VIEWIMP_HXX

// include ---------------------------------------------------------------

#include <basic/sbxobj.hxx>
#include <sfx2/viewsh.hxx>
#include <sfx2/viewfrm.hxx>                  // SvBorder
#include <osl/mutex.hxx>
#include <cppuhelper/interfacecontainer.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <svtools/acceleratorexecute.hxx>
#include <rtl/ref.hxx>
#include <vcl/print.hxx>
#include <queue>

// forward ---------------------------------------------------------------

class SfxOfficeDispatch;
class SfxBaseController;

typedef SfxShell* SfxShellPtr_Impl;
SV_DECL_PTRARR( SfxShellArr_Impl, SfxShellPtr_Impl, 4, 4 )

class SfxClipboardChangeListener;

struct SfxViewShell_Impl
{
    ::osl::Mutex                aMutex;
    ::cppu::OInterfaceContainerHelper aInterceptorContainer;
    bool                        m_bControllerSet;
    SfxShellArr_Impl            aArr;
    SvBorder                    aBorder;
    Size                        aOptimalSize;
    Size                        aMargin;
    sal_uInt16                      m_nPrinterLocks;
    bool                        m_bCanPrint;
    bool                        m_bHasPrintOptions;
    bool                        m_bPlugInsActive;
    bool                        m_bIsShowView;
    // FIXME UNUSED ???
    //bool                        m_bOwnsMenu;
    bool                        m_bGotOwnership;
    bool                        m_bGotFrameOwnership;
    SfxScrollingMode            m_eScroll;
    sal_uInt16                      m_nFamily;
    ::rtl::Reference<SfxBaseController> m_pController;
    ::std::auto_ptr< ::svt::AcceleratorExecute >    m_pAccExec;
    com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > aPrintOpts;
    ::rtl::Reference< SfxClipboardChangeListener > xClipboardListener;
    ::boost::shared_ptr< vcl::PrinterController > m_pPrinterController;

    SfxViewShell_Impl(sal_uInt16 const nFlags);
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
