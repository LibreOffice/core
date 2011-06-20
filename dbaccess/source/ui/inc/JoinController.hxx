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
#ifndef DBAUI_JOINCONTROLLER_HXX
#define DBAUI_JOINCONTROLLER_HXX

#include "singledoccontroller.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include "moduledbu.hxx"
#include "JoinTableView.hxx"
#include "JoinDesignView.hxx"
#include "TableConnectionData.hxx"
#include "TableWindowData.hxx"
#include <memory>
#include <boost/shared_ptr.hpp>

namespace comphelper
{
    class NamedValueCollection;
}

class VCLXWindow;
namespace dbaui
{
    class OAddTableDlg;
    class AddTableDialogContext;
    class OTableConnectionData;
    class OTableWindowData;
    class OTableWindow;
    typedef OSingleDocumentController OJoinController_BASE;

    class OJoinController : public OJoinController_BASE
    {
        OModuleClient                    m_aModuleClient;
    protected:
        TTableConnectionData m_vTableConnectionData;
        TTableWindowData     m_vTableData;

        Fraction                                m_aZoom;
        ::dbtools::SQLExceptionInfo             m_aExceptionInfo;

        OAddTableDlg*                               m_pAddTableDialog;
        ::std::auto_ptr< AddTableDialogContext >    m_pDialogContext;
        Point                                   m_aMinimumTableViewSize;

        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);

        /** loads the information for the windows.
            @param  i_rViewSettings
                The properties which comes from the layout information.
        */
        void loadTableWindows( const ::comphelper::NamedValueCollection& i_rViewSettings );

        /** loads the information for one window.
            @param  _rTable
                The properties which comes from the layout information.
        */
        void loadTableWindow( const ::comphelper::NamedValueCollection& i_rTableWindowSettings );

        /** saves the TableWindows structure in a sequence of property values
            @param  _rViewProps
                Contains the new sequence.
        */
        void saveTableWindows( ::comphelper::NamedValueCollection& o_rViewSettings ) const;

        virtual ~OJoinController();
    public:
        OJoinController(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM);

        // ---------------------------------------------------------------
        // attribute access
        inline TTableWindowData*        getTableWindowData()     { return &m_vTableData; }
        inline TTableConnectionData*    getTableConnectionData() { return &m_vTableConnectionData;}
        inline OAddTableDlg*            getAddTableDialog()const { return m_pAddTableDialog; }

        // ---------------------------------------------------------------
        // OSingleDocumentController overridables
        virtual void        reconnect( sal_Bool _bUI );
        virtual void        impl_onModifyChanged();

        // ---------------------------------------------------------------
        // own overridables
        /** determines whether or not it's allowed for database views to participate in the game
        */
        virtual bool allowViews() const = 0;

        /** determines whether or not it's allowed for queries to participate in the game
        */
        virtual bool allowQueries() const = 0;

        /** provides access to the OJoinDesignView belonging to the controller, which might
            or might not be the direct view (getView)
        */
        virtual OJoinDesignView*    getJoinView();


        // ---------------------------------------------------------------
        /** erase the data in the data vector
            @param  _pData
                    the data whioch should be erased
        */
        void    removeConnectionData(const TTableConnectionData::value_type& _pData);

        void    SaveTabWinsPosSize( OJoinTableView::OTableWindowMap* pTabWinList, long nOffsetX, long nOffsetY );

        void    SaveTabWinPosSize(OTableWindow* pTabWin, long nOffsetX, long nOffsetY);

        // ---------------------------------------------------------------
        // UNO interface overridables
        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

        // ::com::sun::star::lang::XComponent
        virtual void    SAL_CALL disposing();
        // ::com::sun::star::frame::XController
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( ::com::sun::star::uno::RuntimeException );


        // ---------------------------------------------------------------
        // misc
        /** only defines a method to save a SQLException in d&d methods to show the error at a later state
            set the internal member m_aExceptionInfo to _rInfo
        */
        void setErrorOccurred(const ::dbtools::SQLExceptionInfo& _rInfo)
        {
            m_aExceptionInfo = _rInfo;
        }
        /**
            just returns the internal member and clears it
        */
        ::dbtools::SQLExceptionInfo clearOccurredError()
        {
            ::dbtools::SQLExceptionInfo aInfo = m_aExceptionInfo;
            m_aExceptionInfo = ::dbtools::SQLExceptionInfo();
            return aInfo;
        }

    protected:
        TTableWindowData::value_type createTableWindowData(const ::rtl::OUString& _sComposedName,const ::rtl::OUString& _sTableName,const ::rtl::OUString& _sWindowName);
        // ask the user if the design should be saved when it is modified
        virtual short saveModified() = 0;
        // called when the orignal state should be reseted (first time load)
        virtual void reset()         = 0;
        virtual void describeSupportedFeatures();

        AddTableDialogContext&  impl_getDialogContext() const;
    };
}
#endif // DBAUI_JOINCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
