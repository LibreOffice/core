/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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

