/*************************************************************************
 *
 *  $RCSfile: unodatbr.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2000-11-09 14:44:18 $
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

#ifndef _SBA_UNODATBR_HXX_
#define _SBA_UNODATBR_HXX_

#ifndef _SBA_BWRCTRLR_HXX
#include "brwctrlr.hxx"
#endif

#ifndef _COMPHELPER_STLTYPES_HXX_
#include <comphelper/stl_types.hxx>
#endif
#ifndef _COM_SUN_STAR_FRAME_XSTATUSLISTENER_HPP_
#include <com/sun/star/frame/XStatusListener.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XDISPATCH_HPP_
#include <com/sun/star/frame/XDispatch.hpp>
#endif
#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

// =========================================================================
class SvLBoxEntry;
class Splitter;

namespace dbaui
{
    class DBTreeView;
    class DBTreeListModel;
    // =========================================================================
    typedef ::cppu::ImplHelper1< ::com::sun::star::frame::XStatusListener >
                            SbaTableQueryBrowser_Base;
    class SbaTableQueryBrowser
                :public SbaXDataBrowserController
                ,public SbaTableQueryBrowser_Base
    {
    protected:
        DBTreeView*             m_pTreeView;
        Splitter*               m_pSplitter;
        DBTreeListModel*        m_pTreeModel;           // contains the datasources of the registry

        DECLARE_STL_STDKEY_MAP( sal_Int32, ::com::sun::star::uno::Reference< ::com::sun::star::frame::XDispatch >, SpecialSlotDispatchers);
        DECLARE_STL_STDKEY_MAP( sal_Int32, sal_Bool, SpecialSlotStates);
        SpecialSlotDispatchers  m_aDispatchers;         // external dispatchers for slots we do not execute ourself
        SpecialSlotStates       m_aDispatchStates;      // states of the slots handled by external dispatchers

        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
                                m_xDatabaseContext;

    // attribute access
    public:
        SbaTableQueryBrowser(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM);
        ~SbaTableQueryBrowser();

//      static void * SAL_CALL operator new( size_t nSize ) throw()
//          { return ::rtl_allocateMemory( nSize ); }
//      static void SAL_CALL operator delete( void * pMem ) throw()
//          { ::rtl_freeMemory( pMem ); }
//
        // need by registration
        static ::rtl::OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Sequence< ::rtl::OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

        DECLARE_UNO3_DEFAULTS(SbaTableQueryBrowser,SbaXDataBrowserController_Base);
        // late construction
        virtual sal_Bool Construct(Window* pParent);

        // ::com::sun::star::beans::XPropertyChangeListener
        virtual void SAL_CALL propertyChange(const ::com::sun::star::beans::PropertyChangeEvent& evt);

        // ::com::sun::star::frame::XController
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL attachFrame(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > & xFrame) throw( ::com::sun::star::uno::RuntimeException );

        // ::com::sun::star::lang::XComponent
        virtual void        SAL_CALL dispose();

        // XStatusListener
        virtual void SAL_CALL statusChanged( const ::com::sun::star::frame::FeatureStateEvent& Event ) throw(::com::sun::star::uno::RuntimeException);

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

        // XServiceInfo
        virtual ::rtl::OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException);
        virtual ::comphelper::StringSequence SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException);

    protected:
        // SbaXDataBrowserController overridables
        virtual sal_Bool InitializeForm(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XRowSet > & xForm);
        virtual sal_Bool InitializeGridModel(const ::com::sun::star::uno::Reference< ::com::sun::star::form::XFormComponent > & xGrid);

        virtual ToolBox* CreateToolBox(Window* pParent);

        virtual void InvalidateFeature(sal_uInt16 nId, const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > & xListener = ::com::sun::star::uno::Reference< ::com::sun::star::frame::XStatusListener > ());

        virtual void addModelListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel);
        virtual void removeModelListeners(const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XControlModel > & _xGridControlModel);

        virtual void AddColumnListener(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xCol);
        virtual void RemoveColumnListener(const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > & xCol);

        virtual void            AddSupportedFeatures();
        virtual FeatureState    GetState(sal_uInt16 nId);
        virtual void            Execute(sal_uInt16 nId);

        String getURL() const;
    private:
        // check the state of the external slot given, update any UI elements if necessary
        void implCheckExternalSlot(sal_Int32 _nId);

        sal_Bool    populateTree(const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xNameAccess, SvLBoxEntry* _pParent, const Image& _rImage);
        void        initializeTreeModel();

        // is called when a table or a query was selected
        DECL_LINK( OnSelectEntry, SvLBoxEntry* );
        DECL_LINK( OnExpandEntry, SvLBoxEntry* );
    };
}
#endif // _SBA_UNODATBR_HXX_

