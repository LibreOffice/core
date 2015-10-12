/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_JOINCONTROLLER_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_JOINCONTROLLER_HXX

#include "singledoccontroller.hxx"
#include <com/sun/star/beans/XPropertySet.hpp>
#include "moduledbu.hxx"
#include "JoinTableView.hxx"
#include "JoinDesignView.hxx"
#include "TableConnectionData.hxx"
#include "TableWindowData.hxx"
#include <tools/fract.hxx>
#include <memory>

namespace comphelper
{
    class NamedValueCollection;
}

namespace dbaui
{
    class OAddTableDlg;
    class AddTableDialogContext;
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

        VclPtr<OAddTableDlg>                          m_pAddTableDialog;
        ::std::unique_ptr< AddTableDialogContext >    m_pDialogContext;
        Point                                   m_aMinimumTableViewSize;

        // state of a feature. 'feature' may be the handle of a css::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const override;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) override;

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
        OJoinController(const css::uno::Reference< css::uno::XComponentContext >& _rM);

        // attribute access
        inline TTableWindowData&        getTableWindowData()     { return m_vTableData; }
        inline TTableConnectionData&    getTableConnectionData() { return m_vTableConnectionData;}
        inline OAddTableDlg*            getAddTableDialog()const { return m_pAddTableDialog; }

        // OSingleDocumentController overridables
        virtual void        reconnect( bool _bUI ) override;
        virtual void        impl_onModifyChanged() override;

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

        /** erase the data in the data vector
            @param  _pData
                    the data which should be erased
        */
        void    removeConnectionData(const TTableConnectionData::value_type& _pData);

        void    SaveTabWinsPosSize( OJoinTableView::OTableWindowMap* pTabWinList, long nOffsetX, long nOffsetY );

        static void SaveTabWinPosSize(OTableWindow* pTabWin, long nOffsetX, long nOffsetY);

        // UNO interface overridables
        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) throw(css::uno::RuntimeException, std::exception) override;

        // css::lang::XComponent
        virtual void    SAL_CALL disposing() override;
        // css::frame::XController
        virtual sal_Bool SAL_CALL suspend(sal_Bool bSuspend) throw( css::uno::RuntimeException, std::exception ) override;

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
        TTableWindowData::value_type createTableWindowData(const OUString& _sComposedName,const OUString& _sTableName,const OUString& _sWindowName);
        // ask the user if the design should be saved when it is modified
        virtual short saveModified() = 0;
        // called when the orignal state should be reseted (first time load)
        virtual void reset()         = 0;
        virtual void describeSupportedFeatures() override;

        AddTableDialogContext&  impl_getDialogContext() const;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_JOINCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
