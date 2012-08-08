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

#ifndef __FRAMEWORK_SERVICES_TABWINDOWSERVICE_HXX_
#define __FRAMEWORK_SERVICES_TABWINDOWSERVICE_HXX_

/** Attention: stl headers must(!) be included at first. Otherwhise it can make trouble
               with solaris headers ...
*/
#include <vector>

#include <classes/fwktabwindow.hxx>
#include <classes/propertysethelper.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <macros/generic.hxx>
#include <macros/debug.hxx>
#include <macros/xinterface.hxx>
#include <macros/xtypeprovider.hxx>
#include <macros/xserviceinfo.hxx>
#include <general.h>
#include <stdtypes.h>

#include <com/sun/star/awt/XSimpleTabController.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>

#include <cppuhelper/weak.hxx>
#include <vcl/window.hxx>

namespace framework{

struct TTabPageInfo
{
    public:

        TTabPageInfo()
            : m_nIndex      ( -1      )
            , m_bCreated    (sal_False)
            , m_pPage       ( NULL    )
            , m_lProperties (         )
        {}

        TTabPageInfo(::sal_Int32 nID)
            : m_nIndex      ( nID     )
            , m_bCreated    (sal_False)
            , m_pPage       ( NULL    )
            , m_lProperties (         )
        {}

    public:

        ::sal_Int32                                   m_nIndex;
        ::sal_Bool                                    m_bCreated;
        FwkTabPage*                                   m_pPage;
        css::uno::Sequence< css::beans::NamedValue >  m_lProperties;
};

typedef ::boost::unordered_map< ::sal_Int32                    ,
                         TTabPageInfo                   ,
                         Int32HashCode                  ,
                         ::std::equal_to< ::sal_Int32 > > TTabPageInfoHash;

/*-************************************************************************************************************//**
    @short  implements a helper service providing a dockable tab control window
*//*-*************************************************************************************************************/

class TabWindowService :  public css::lang::XTypeProvider
                         ,  public css::lang::XServiceInfo
                         ,  public css::awt::XSimpleTabController
                         ,  public css::lang::XComponent
                         ,  public ThreadHelpBase
                         ,  public TransactionBase
                         ,  public PropertySetHelper
                         ,  public ::cppu::OWeakObject
{
    //-------------------------------------------------------------------------------------------------------------
    //  public methods
    //-------------------------------------------------------------------------------------------------------------

    public:

        //---------------------------------------------------------------------------------------------------------
        //  constructor / destructor
        //---------------------------------------------------------------------------------------------------------

        TabWindowService( const css::uno::Reference< css::lang::XMultiServiceFactory >& xFactory );
        virtual ~TabWindowService();

        //---------------------------------------------------------------------------------------------------------
        //  XInterface, XTypeProvider, XServiceInfo
        //---------------------------------------------------------------------------------------------------------

        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER
        DECLARE_XSERVICEINFO

        //---------------------------------------------------------------------------------------------------------
        //  XSimpleTabController
        //---------------------------------------------------------------------------------------------------------

        virtual sal_Int32 SAL_CALL insertTab() throw ( css::uno::RuntimeException );
        virtual void SAL_CALL removeTab( sal_Int32 nID ) throw ( css::lang::IndexOutOfBoundsException, css::uno::RuntimeException );
        virtual void SAL_CALL setTabProps( sal_Int32 nID, const css::uno::Sequence< css::beans::NamedValue >& aProperties ) throw ( css::lang::IndexOutOfBoundsException, css::uno::RuntimeException );
        virtual css::uno::Sequence< css::beans::NamedValue > SAL_CALL getTabProps( sal_Int32 nID ) throw ( css::lang::IndexOutOfBoundsException, css::uno::RuntimeException );
        virtual void SAL_CALL activateTab( sal_Int32 nID ) throw ( css::lang::IndexOutOfBoundsException, css::uno::RuntimeException );
        virtual sal_Int32 SAL_CALL getActiveTabID() throw ( css::uno::RuntimeException );
        virtual void SAL_CALL addTabListener( const css::uno::Reference< css::awt::XTabListener >& Listener ) throw ( css::uno::RuntimeException );
        virtual void SAL_CALL removeTabListener( const css::uno::Reference< css::awt::XTabListener >& Listener ) throw ( css::uno::RuntimeException );

        //---------------------------------------------------------------------------------------------------------
        //  XComponent
        //---------------------------------------------------------------------------------------------------------

        virtual void SAL_CALL dispose() throw ( css::uno::RuntimeException );
        virtual void SAL_CALL addEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw ( css::uno::RuntimeException );
        virtual void SAL_CALL removeEventListener( const css::uno::Reference< css::lang::XEventListener >& xListener ) throw ( css::uno::RuntimeException );

    //-------------------------------------------------------------------------------------------------------------
    //  protected methods
    //-------------------------------------------------------------------------------------------------------------

    protected:

    //-------------------------------------------------------------------------------------------------------------
    //  private methods
    //-------------------------------------------------------------------------------------------------------------

    private:

        void impl_initializePropInfo();
        virtual void SAL_CALL impl_setPropertyValue(const ::rtl::OUString& sProperty,
                                                          sal_Int32        nHandle  ,
                                                    const css::uno::Any&   aValue   );
        virtual css::uno::Any SAL_CALL impl_getPropertyValue(const ::rtl::OUString& sProperty,
                                                                   sal_Int32        nHandle  );

        DECL_DLLPRIVATE_LINK( EventListener, VclSimpleEvent * );

        void impl_checkTabIndex (::sal_Int32 nID) throw (css::lang::IndexOutOfBoundsException);
        TTabPageInfoHash::iterator impl_getTabPageInfo(::sal_Int32 nID) throw (css::lang::IndexOutOfBoundsException);
        FwkTabWindow* mem_TabWin ();
    //-------------------------------------------------------------------------------------------------------------
    //  variables
    //  (should be private everyway!)
    //-------------------------------------------------------------------------------------------------------------

    private:

        /// reference to factory, which has created this instance
        css::uno::Reference< css::lang::XMultiServiceFactory > m_xFactory;

        /// the tab window as XWindow ( to hold window* alive !)
        css::uno::Reference< css::awt::XWindow > m_xTabWin;

        /// the VCL tab window
        FwkTabWindow* m_pTabWin;

        /// container of inserted tab pages
        TTabPageInfoHash m_lTabPageInfos;

        /// container of the added TabListener
        ::cppu::OMultiTypeInterfaceContainerHelper m_lListener;

        /// counter of the tabpage indexes
        ::sal_Int32 m_nPageIndexCounter;

        /// index of the current active page
        ::sal_Int32 m_nCurrentPageIndex;

        /// title of the tabcontrolled window
        ::rtl::OUString m_sTitle;

};      //  class TabWindowService

}       //  namespace framework

#endif  //  #ifndef __FRAMEWORK_SERVICES_TABWINDOWSERVICE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
