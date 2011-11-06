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


#ifndef _CHART_ELEMENTSELECTOR_HXX
#define _CHART_ELEMENTSELECTOR_HXX

#include "ServiceMacros.hxx"
#include "ObjectHierarchy.hxx"
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <cppuhelper/implbase1.hxx>
#include <svtools/toolboxcontroller.hxx>

#include <vcl/lstbox.hxx>
#include <cppuhelper/weakref.hxx>

#include <memory>

//.............................................................................
namespace chart
{
//.............................................................................

struct ListBoxEntryData
{
    rtl::OUString UIName;
    ObjectHierarchy::tOID OID;
    sal_Int32 nHierarchyDepth;

    ListBoxEntryData() : nHierarchyDepth(0)
    {
    }
};

class SelectorListBox : public ListBox
{
    public:
        SelectorListBox( Window* pParent, WinBits nStyle );
        virtual ~SelectorListBox();

        virtual void Select();
        virtual long Notify( NotifyEvent& rNEvt );
        virtual ::com::sun::star::uno::Reference< ::com::sun::star::accessibility::XAccessible > CreateAccessible();

        void ReleaseFocus_Impl();

        void SetChartController( const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >& xChartController );
        void UpdateChartElementsListAndSelection();

    private:
        ::com::sun::star::uno::WeakReference<
            ::com::sun::star::frame::XController >   m_xChartController;

        ::std::vector< ListBoxEntryData > m_aEntries;

        bool m_bReleaseFocus;
};

// ------------------------------------------------------------------
// ------------------------------------------------------------------

typedef ::cppu::ImplHelper1 < ::com::sun::star::lang::XServiceInfo> ElementSelectorToolbarController_BASE;

class ElementSelectorToolbarController : public ::svt::ToolboxController
                                                , ElementSelectorToolbarController_BASE
{
public:
    ElementSelectorToolbarController( ::com::sun::star::uno::Reference<
               ::com::sun::star::uno::XComponentContext > const & xContext );
    virtual ~ElementSelectorToolbarController();

    // XServiceInfo
    APPHELPER_XSERVICEINFO_DECL()
    APPHELPER_SERVICE_FACTORY_HELPER(ElementSelectorToolbarController)

    // XInterface
     virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType ) throw (::com::sun::star::uno::RuntimeException);
     virtual void SAL_CALL acquire() throw ();
     virtual void SAL_CALL release() throw ();

     // XInitialization
     virtual void SAL_CALL initialize( const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Any >& aArguments ) throw (::com::sun::star::uno::Exception, ::com::sun::star::uno::RuntimeException);
     // XStatusListener
     virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw ( ::com::sun::star::uno::RuntimeException );
     // XToolbarController
     virtual ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow > SAL_CALL createItemWindow( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XWindow >& Parent ) throw (::com::sun::star::uno::RuntimeException);

private:
    //no default constructor
    ElementSelectorToolbarController(){}

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>        m_xCC;
    ::std::auto_ptr< SelectorListBox > m_apSelectorListBox;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif
