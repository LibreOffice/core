/*************************************************************************
 *
 *  $RCSfile: ChartModel.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: bm $ $Date: 2003-10-17 14:48:14 $
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
 *  Copyright: 2003 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef _CHART_MODEL_HXX
#define _CHART_MODEL_HXX

#include "LifeTime.hxx"
#include "ServiceMacros.hxx"

#ifndef _COM_SUN_STAR_FRAME_XMODEL_HPP_
#include <com/sun/star/frame/XModel.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTORABLE_HPP_
#include <com/sun/star/frame/XStorable.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
/*
#ifndef _COM_SUN_STAR_SDBC_XCLOSEABLE_HPP_
#include <com/sun/star/util/XCloseable.hpp>
#endif
*/
#ifndef _COM_SUN_STAR_VIEW_XPRINTABLE_HPP_
#include <com/sun/star/view/XPrintable.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTBROADCASTER_HPP_
#include <com/sun/star/document/XEventBroadcaster.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XEVENTSSUPPLIER_HPP_
#include <com/sun/star/document/XEventsSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XDOCUMENTINFOSUPPLIER_HPP_
#include <com/sun/star/document/XDocumentInfoSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_DOCUMENT_XVIEWDATASUPPLIER_HPP_
#include <com/sun/star/document/XViewDataSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif

// #ifndef _DRAFTS_COM_SUN_STAR_CHART2_XMODELDATAPROVIDER_HPP_
// #include <drafts/com/sun/star/chart2/XModelDataProvider.hpp>
// #endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XCHARTDOCUMENT_HPP_
#include <drafts/com/sun/star/chart2/XChartDocument.hpp>
#endif
#ifndef _DRAFTS_COM_SUN_STAR_CHART2_XTITLED_HPP_
#include <drafts/com/sun/star/chart2/XTitled.hpp>
#endif

/*
#ifndef _COM_SUN_STAR_FRAME_XCONTROLLER_HPP_
#include <com/sun/star/frame/XController.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCHPROVIDER_HPP_
#include <com/sun/star/frame/XDispatchProvider.hpp>
#endif
#ifndef _COM_SUN_STAR_VIEW_XSELECTIONSUPPLIER_HPP_
#include <com/sun/star/view/XSelectionSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_UI_XCONTEXTMENUINTERCEPTION_HPP_
#include <com/sun/star/ui/XContextMenuInterception.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XWEAK_HPP_
#include <com/sun/star/uno/XWeak.hpp>
#endif
#ifndef _COM_SUN_STAR_SDBC_XCLOSELISTENER_HPP_
#include <com/sun/star/util/XCloseListener.hpp>
#endif

*/
/*
#ifndef _COM_SUN_STAR_BEANS_XPROPERTYCHANGELISTENER_HPP_
#include <com/sun/star/beans/XPropertyChangeListener.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_XLOADABLE_HPP_
#include <com/sun/star/form/XLoadable.hpp>
#endif
*/
#ifndef _CPPUHELPER_IMPLBASE7_HXX_
#include <cppuhelper/implbase7.hxx>
#endif

// for auto_ptr
#include <memory>

//=============================================================================
/** this is an example implementation for the service ::com::sun::star::document::OfficeDocument
*/

namespace chart
{

namespace impl
{
    class ImplChartModel;

// Note: needed for queryInterface (if it calls the base-class implementation)
typedef ::cppu::WeakImplHelper7 <
//       ::com::sun::star::frame::XModel        //comprehends XComponent (required interface), base of XChartDocument
         ::com::sun::star::util::XCloseable     //comprehends XCloseBroadcaster
        ,::com::sun::star::frame::XStorable     //(required interface)
        ,::com::sun::star::util::XModifiable    //comprehends XModifyBroadcaster (required interface)
//      ,::com::sun::star::view::XPrintable     //(optional interface)
//      ,::com::sun::star::document::XEventBroadcaster      //(optional interface)
//      ,::com::sun::star::document::XEventsSupplier        //(optional interface)
//      ,::com::sun::star::document::XDocumentInfoSupplier  //(optional interface)
//      ,::com::sun::star::document::XViewDataSupplier      //(optional interface)

    //  ,::com::sun::star::uno::XWeak           // implemented by WeakImplHelper(optional interface)
    //  ,::com::sun::star::uno::XInterface      // implemented by WeakImplHelper(optional interface)
    //  ,::com::sun::star::lang::XTypeProvider  // implemented by WeakImplHelper
        ,::com::sun::star::lang::XServiceInfo
//         ,::drafts::com::sun::star::chart2::XModelDataProvider
        ,::drafts::com::sun::star::chart2::XChartDocument  // derived from XModel
        ,::com::sun::star::style::XStyleFamiliesSupplier
        ,::drafts::com::sun::star::chart2::XTitled
        >
    ChartModel_Base;
}

class ChartModel : public impl::ChartModel_Base
{

private:
    mutable ::apphelper::CloseableLifeTimeManager   m_aLifeTimeManager;

    mutable ::osl::Mutex    m_aModelMutex;
    sal_Bool volatile       m_bReadOnly;
    sal_Bool volatile       m_bModified;

    ::rtl::OUString                                                             m_aResource;
    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   m_aMediaDescriptor;

    ::cppu::OInterfaceContainerHelper*  volatile                                m_pControllers;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController >    m_xCurrentController;
    sal_uInt16                                                                  m_nControllerLockCount;

//  ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >   m_aPrinterOptions;

    ::std::auto_ptr< impl::ImplChartModel >                                     m_pImplChartModel;

private:
    //private methods

    ::rtl::OUString impl_g_getLocation();

    ::cppu::OInterfaceContainerHelper*
        impl_getControllerContainer()
                            throw (::com::sun::star::uno::RuntimeException);

    sal_Bool
        impl_isControllerConnected( const com::sun::star::uno::Reference<
                            com::sun::star::frame::XController >& xController );

    com::sun::star::uno::Reference< com::sun::star::frame::XController >
        impl_getCurrentController()
                            throw( com::sun::star::uno::RuntimeException);

    void SAL_CALL
        impl_notifyModifiedListeners()
                            throw( com::sun::star::uno::RuntimeException);

public:
    //no default constructor
    ChartModel(::com::sun::star::uno::Reference<
               ::com::sun::star::uno::XComponentContext > const & xContext);
    virtual ~ChartModel();

    //-----------------------------------------------------------------
    // ::com::sun::star::lang::XServiceInfo
    //-----------------------------------------------------------------

    APPHELPER_XSERVICEINFO_DECL()
    APPHELPER_SERVICE_FACTORY_HELPER(ChartModel)

    //-----------------------------------------------------------------
    // ::com::sun::star::frame::XModel (required interface)
    //-----------------------------------------------------------------

    virtual sal_Bool SAL_CALL
        attachResource( const ::rtl::OUString& rURL
                            , const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::beans::PropertyValue >& rMediaDescriptor )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL
        getURL()            throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
        getArgs()           throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        connectController( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XController >& xController )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        disconnectController( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XController >& xController )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        lockControllers()   throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        unlockControllers() throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL
        hasControllersLocked()
                            throw (::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::frame::XController > SAL_CALL
        getCurrentController()
                            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        setCurrentController( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::frame::XController >& xController )
                            throw (::com::sun::star::container::NoSuchElementException
                            , ::com::sun::star::uno::RuntimeException);

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface > SAL_CALL
        getCurrentSelection()
                            throw (::com::sun::star::uno::RuntimeException);


    //-----------------------------------------------------------------
    // ::com::sun::star::lang::XComponent (base of XModel)
    //-----------------------------------------------------------------
    virtual void SAL_CALL
        dispose()           throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        addEventListener( const ::com::sun::star::uno::Reference<
                          ::com::sun::star::lang::XEventListener > & xListener )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        removeEventListener( const ::com::sun::star::uno::Reference<
                          ::com::sun::star::lang::XEventListener > & xListener )
                            throw (::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::com::sun::star::util::XCloseable
    //-----------------------------------------------------------------
    virtual void SAL_CALL
        close( sal_Bool bDeliverOwnership )
                            throw( ::com::sun::star::util::CloseVetoException );

    //-----------------------------------------------------------------
    // ::com::sun::star::util::XCloseBroadcaster (base of XCloseable)
    //-----------------------------------------------------------------
    virtual void SAL_CALL
        addCloseListener( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::util::XCloseListener > & xListener )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        removeCloseListener( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::util::XCloseListener > & xListener )
                            throw (::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::com::sun::star::frame::XStorable (required interface)
    //-----------------------------------------------------------------
    virtual sal_Bool SAL_CALL
        hasLocation()       throw (::com::sun::star::uno::RuntimeException);

    virtual ::rtl::OUString SAL_CALL
        getLocation()       throw (::com::sun::star::uno::RuntimeException);

    virtual sal_Bool SAL_CALL
        isReadonly()        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        store()             throw (::com::sun::star::io::IOException
                            , ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        storeAsURL( const ::rtl::OUString& rURL
                            , const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::beans::PropertyValue >& rMediaDescriptor )
                            throw (::com::sun::star::io::IOException
                            , ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        storeToURL( const ::rtl::OUString& rURL
                            , const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::beans::PropertyValue >& rMediaDescriptor )
                            throw (::com::sun::star::io::IOException
                            , ::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::com::sun::star::util::XModifiable (required interface)
    //-----------------------------------------------------------------
    virtual sal_Bool SAL_CALL
        isModified()        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        setModified( sal_Bool bModified )
                            throw (::com::sun::star::beans::PropertyVetoException
                            , ::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::com::sun::star::util::XModifyBroadcaster (base of XModifiable)
    //-----------------------------------------------------------------
    virtual void SAL_CALL
        addModifyListener( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::util::XModifyListener >& xListener )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        removeModifyListener( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::util::XModifyListener >& xListener )
                            throw (::com::sun::star::uno::RuntimeException);
/*
    //-----------------------------------------------------------------
    // ::com::sun::star::view::XPrintable (optional interface)
    //-----------------------------------------------------------------
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > SAL_CALL
        getPrinter()        throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        setPrinter( const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::beans::PropertyValue >& rPrinter )
                            throw (::com::sun::star::lang::IllegalArgumentException
                            , ::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        print( const ::com::sun::star::uno::Sequence<
                            ::com::sun::star::beans::PropertyValue >& rOptions )
                            throw (::com::sun::star::lang::IllegalArgumentException
                            , ::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::com::sun::star::document::XEventBroadcaster (optional interface)
    //-----------------------------------------------------------------

    virtual void SAL_CALL
        addEventListener( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::document::XEventListener >& xListener )
                            throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        removeEventListener( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::document::XEventListener >& xListener )
                            throw (::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::com::sun::star::document::XEventsSupplier (optional interface)
    //-----------------------------------------------------------------

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameReplace > SAL_CALL
        getEvents()         throw (::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::com::sun::star::document::XDocumentInfoSupplier (optional interface)
    //-----------------------------------------------------------------

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::document::XDocumentInfo > SAL_CALL
        getDocumentInfo()   throw (::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::com::sun::star::document::XViewDataSupplier (optional interface)
    //-----------------------------------------------------------------

    virtual ::com::sun::star::uno::Reference< ::com::sun::star::container::XIndexAccess > SAL_CALL
        getViewData()       throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        setViewData( const ::com::sun::star::uno::Reference<
                            ::com::sun::star::container::XIndexAccess >& xData )
                            throw (::com::sun::star::uno::RuntimeException);
    */

    //-----------------------------------------------------------------
    // ::com::sun::star::style::XStyleFamiliesSupplier
    //-----------------------------------------------------------------
    virtual ::com::sun::star::uno::Reference<
            ::com::sun::star::container::XNameAccess > SAL_CALL
        getStyleFamilies() throw (::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::drafts::com::sun::star::chart2::XModelDataProvider
    //-----------------------------------------------------------------
//     virtual ::com::sun::star::uno::Sequence<
//                 ::com::sun::star::uno::Reference<
//                     ::drafts::com::sun::star::chart2::XDataSeries > > SAL_CALL
//         getDataSeries()    throw (::com::sun::star::uno::RuntimeException);

    //-----------------------------------------------------------------
    // ::drafts::com::sun::star::chart2::XChartDocument
    //-----------------------------------------------------------------
    virtual ::com::sun::star::uno::Reference<
                ::drafts::com::sun::star::chart2::XDiagram > SAL_CALL
        getDiagram()       throw (::com::sun::star::uno::RuntimeException);

    virtual void SAL_CALL
        setDiagram( const ::com::sun::star::uno::Reference<
                        ::drafts::com::sun::star::chart2::XDiagram >& xDiagram )
            throw (::com::sun::star::lang::IllegalArgumentException,
                   ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        attachDataProvider( const ::com::sun::star::uno::Reference<
                            ::drafts::com::sun::star::chart2::XDataProvider >& xProvider )
            throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        setRangeRepresentation( const ::rtl::OUString& aRangeRepresentation )
            throw (::com::sun::star::lang::IllegalArgumentException,
                   ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        setChartTypeManager( const ::com::sun::star::uno::Reference<
                             ::drafts::com::sun::star::chart2::XChartTypeManager >& xNewManager )
            throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XChartTypeManager > SAL_CALL
        getChartTypeManager()
            throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL
        setChartTypeTemplate( const ::com::sun::star::uno::Reference<
                                  ::drafts::com::sun::star::chart2::XChartTypeTemplate >& xNewTemplate )
            throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::drafts::com::sun::star::chart2::XChartTypeTemplate > SAL_CALL
        getChartTypeTemplate()
            throw (::com::sun::star::uno::RuntimeException);
//     virtual void SAL_CALL setSplitLayoutContainer(
//         const ::com::sun::star::uno::Reference<
//             ::drafts::com::sun::star::layout::XSplitLayoutContainer >& xLayoutCnt )
//         throw (::com::sun::star::uno::RuntimeException);
//     virtual ::com::sun::star::uno::Reference<
//             ::drafts::com::sun::star::layout::XSplitLayoutContainer > SAL_CALL getSplitLayoutContainer()
//         throw (::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > SAL_CALL
        getPageBackground()
            throw (::com::sun::star::uno::RuntimeException);

// ____ XTitled ____
    virtual ::com::sun::star::uno::Reference<
        ::drafts::com::sun::star::chart2::XTitle > SAL_CALL getTitle()
        throw (::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setTitle( const ::com::sun::star::uno::Reference<
                                    ::drafts::com::sun::star::chart2::XTitle >& Title )
        throw (::com::sun::star::uno::RuntimeException);

    // ____ XInterface (for old API wrapper) ____
    virtual ::com::sun::star::uno::Any SAL_CALL queryInterface( const ::com::sun::star::uno::Type& aType )
        throw (::com::sun::star::uno::RuntimeException);
};

}  // namespace chart

#endif

