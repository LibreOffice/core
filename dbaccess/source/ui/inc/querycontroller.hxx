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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYCONTROLLER_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYCONTROLLER_HXX

#include "JoinController.hxx"
#include "JoinTableView.hxx"
#include "querycontainerwindow.hxx"
#include <svx/ParseContext.hxx>
#include "TableFieldDescription.hxx"

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/io/XObjectInputStream.hpp>
#include <com/sun/star/io/XObjectOutputStream.hpp>
#include <com/sun/star/sdb/CommandType.hpp>
#include <com/sun/star/sdb/XSQLQueryComposer.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbcx/XAlterView.hpp>

#include <comphelper/proparrhlp.hxx>
#include <comphelper/propertycontainer.hxx>
#include <comphelper/uno3.hxx>
#include <connectivity/sqliterator.hxx>
#include <connectivity/sqlnode.hxx>
#include <connectivity/sqlparse.hxx>
#include <svl/undo.hxx>

namespace comphelper
{
    class NamedValueCollection;
}

namespace dbaui
{
    class OQueryContainerWindow;

    typedef ::comphelper::OPropertyContainer                            OQueryController_PBase;
    typedef ::comphelper::OPropertyArrayUsageHelper< OQueryController > OQueryController_PABase;
    class OQueryController  :public OJoinController
                            ,public OQueryController_PBase
                            ,public OQueryController_PABase
    {
        OTableFields                            m_vTableFieldDesc;
        OTableFields                            m_vUnUsedFieldsDesc; // contains fields which aren't visible and don't have any criteria

        css::uno::Sequence< css::beans::PropertyValue >       m_aFieldInformation;

        std::unique_ptr<::svxform::OSystemParseContext>       m_pParseContext;
        ::connectivity::OSQLParser              m_aSqlParser;
        std::unique_ptr<::connectivity::OSQLParseTreeIterator> m_pSqlIterator;

        css::uno::Reference< css::sdb::XSQLQueryComposer >    m_xComposer;
        /// if we're editing an existing view, this is non-NULL
        css::uno::Reference< css::sdbcx::XAlterView >         m_xAlterView;

        OUString        m_sStatement;           // contains the current sql statement
        OUString        m_sUpdateCatalogName;   // catalog for update data
        OUString        m_sUpdateSchemaName;    // schema for update data
        mutable OUString
                        m_sName;                // name of the query

        sal_Int64       m_nLimit;           // the limit of the query result (All==-1)

        sal_Int32       m_nVisibleRows;     // which rows the selection browse should show
        sal_Int32       m_nSplitPos;        // the position of the splitter
        sal_Int32       m_nCommandType;     // the type of the object we're designing
        bool            m_bGraphicalDesign; // are we in the graphical design mode (sal_True) or in the text design (sal_False)?
        bool            m_bDistinct;        // true when you want "select distinct" otherwise false
        bool            m_bEscapeProcessing;// is true when we shouldn't parse the statement


        /** returns the container of queries, views, or command definitions, depending on what object type
            we design currently.

            Not allowed to be called if we design an independent SQL command.
        */
        css::uno::Reference< css::container::XNameAccess >
            getObjectContainer() const;

        bool    editingView() const    { return m_nCommandType == css::sdb::CommandType::TABLE; }
        bool    editingQuery() const   { return m_nCommandType == css::sdb::CommandType::QUERY; }
        bool    editingCommand() const { return m_nCommandType == css::sdb::CommandType::COMMAND; }

        bool askForNewName( const css::uno::Reference< css::container::XNameAccess>& _xElements,
                            bool _bSaveAs);
        // creates the querycomposer
        void setQueryComposer();
        void deleteIterator();
        void executeQuery();
        bool doSaveAsDoc(bool _bSaveAs);

        void saveViewSettings( ::comphelper::NamedValueCollection& o_rViewSettings, const bool i_includingCriteria ) const;
        void loadViewSettings( const ::comphelper::NamedValueCollection& o_rViewSettings );
        OUString translateStatement( bool _bFireStatementChange = true );

        void execute_QueryPropDlg();

    protected:
        // all the features which should be handled by this class
        virtual void            describeSupportedFeatures() override;
        // state of a feature. 'feature' may be the handle of a css::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const override;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId, const css::uno::Sequence< css::beans::PropertyValue>& aArgs) override;

        virtual void            reconnect( bool _bUI ) override;
        virtual OUString getPrivateTitle( ) const override;

        OQueryContainerWindow* getContainer() const { return static_cast< OQueryContainerWindow* >( getView() ); }

    public:
        OQueryController(const css::uno::Reference< css::uno::XComponentContext >& _rM);

        virtual ~OQueryController() override;
        OTableFields&   getTableFieldDesc()         { return m_vTableFieldDesc; }
        OTableFields&   getUnUsedFields()           { return m_vUnUsedFieldsDesc; }

        void            clearFields();

        virtual void impl_onModifyChanged() override;

        // should the statement be parsed by our own sql parser
        bool        isEscapeProcessing()    const { return m_bEscapeProcessing; }
        bool        isGraphicalDesign()     const { return m_bGraphicalDesign; }
        bool        isDistinct()            const { return m_bDistinct; }
        sal_Int64       getLimit()              const { return m_nLimit; }

        const OUString& getStatement()          const { return m_sStatement; }
        sal_Int32       getSplitPos()           const { return m_nSplitPos;}
        sal_Int32       getVisibleRows()        const { return m_nVisibleRows; }

        void            setDistinct(bool _bDistinct)        { m_bDistinct = _bDistinct;}
        void            setLimit(const sal_Int64 _nLimit)       { m_nLimit = _nLimit;}
        void            setSplitPos(sal_Int32 _nSplitPos)       { m_nSplitPos = _nSplitPos;}
        void            setVisibleRows(sal_Int32 _nVisibleRows) { m_nVisibleRows = _nVisibleRows;}

        sal_Int32       getColWidth(sal_uInt16 _nColPos) const;

        const css::uno::Sequence< css::beans::PropertyValue >&
                        getFieldInformation() const { return m_aFieldInformation; }

        ::connectivity::OSQLParser&             getParser()         { return m_aSqlParser;  }
        ::connectivity::OSQLParseTreeIterator&  getParseIterator()  { return *m_pSqlIterator; }

        virtual bool Construct(vcl::Window* pParent) override;

        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )
        // XPropertySet
        virtual css::uno::Reference<css::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() override;
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper() override;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // css::lang::XComponent
        virtual void        SAL_CALL disposing() override;

        virtual OUString SAL_CALL getImplementationName() override;
        virtual css::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() override;
        // need by registration
        /// @throws css::uno::RuntimeException
        static OUString getImplementationName_Static();
        /// @throws css::uno::RuntimeException
        static css::uno::Sequence< OUString > getSupportedServiceNames_Static();
        static css::uno::Reference< css::uno::XInterface >
                Create(const css::uno::Reference< css::lang::XMultiServiceFactory >&);

        // XController
        virtual css::uno::Any SAL_CALL getViewData() override;
        virtual void SAL_CALL restoreViewData(const css::uno::Any& Data) override;

    private:
        virtual void    onLoadedMenu(const css::uno::Reference< css::frame::XLayoutManager >& _xLayoutManager) override;
        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const override;

        // OPropertySetHelper
        virtual void SAL_CALL getFastPropertyValue(
                                    css::uno::Any& rValue,
                                    sal_Int32 nHandle
                                ) const override;

        virtual OJoinDesignView*  getJoinView() override;
        // ask the user if the design should be saved when it is modified
        virtual short saveModified() override;
        virtual void reset() override;
        virtual void impl_initialize() override;

        void    impl_reset( const bool i_bIgnoreQuerySettings = false );
        /// tells the user that we needed to switch to SQL view automatically
        void    impl_showAutoSQLViewError( const css::uno::Any& _rErrorDetails );

        /** switches to the graphical or SQL view mode, as determined by m_bGraphicalDesign
        */
        void    impl_setViewMode( ::dbtools::SQLExceptionInfo* _pErrorInfo );

        /// sets m_sStatement, and notifies our respective property change listeners
        void    setStatement_fireEvent( const OUString& _rNewStatement, bool _bFireStatementChange = true );
        /// sets the m_bEscapeProcessing member, and notifies our respective property change listeners
        void    setEscapeProcessing_fireEvent( const bool _bEscapeProcessing );

        // OJoinController overridables
        virtual bool allowViews() const override;
        virtual bool allowQueries() const override;

    private:
        DECL_LINK( OnExecuteAddTable, void*, void );

    private:
        using OQueryController_PBase::getFastPropertyValue;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
