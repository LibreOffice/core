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
#include "queryview.hxx"
#include "svx/ParseContext.hxx"
#include "TableFieldDescription.hxx"

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

        ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >       m_aFieldInformation;

        ::svxform::OSystemParseContext*         m_pParseContext;
        ::connectivity::OSQLParser              m_aSqlParser;
        ::connectivity::OSQLParseTreeIterator*  m_pSqlIterator;

        ::com::sun::star::uno::Reference< ::com::sun::star::sdb::XSQLQueryComposer >    m_xComposer;
        /// if we're editing an existing view, this is non-NULL
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XAlterView >         m_xAlterView;

        OUString m_sStatement;           // contains the current sql statement
        OUString m_sUpdateCatalogName;   // catalog for update data
        OUString m_sUpdateSchemaName;    // schema for update data
        OUString m_sUpdateTableName;     // table for update data
        mutable OUString
                        m_sName;                // name of the query

        sal_Int64       m_nLimit;           // the limit of the query result (All==-1)

        sal_Int32       m_nVisibleRows;     // which rows the selection browse should show
        sal_Int32       m_nSplitPos;        // the position of the splitter
        sal_Int32       m_nCommandType;     // the type of the object we're designing
        sal_Bool        m_bGraphicalDesign; // are we in the graphical design mode (sal_True) or in the text design (sal_False)?
        sal_Bool        m_bDistinct;        // true when you want "select distinct" otherwise false
        sal_Bool        m_bViewAlias;       // show the alias row in the design view
        sal_Bool        m_bViewTable;       // show the table row in the design view
        sal_Bool        m_bViewFunction;    // show the function row in the design view
        sal_Bool        m_bEscapeProcessing;// is true when we shouldn't parse the statement


        /** returns the container of queries, views, or command definitions, depending on what object type
            we design currently.

            Not allowed to be called if we design an independent SQL command.
        */
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >
            getObjectContainer() const;

        inline  bool    editingView() const    { return m_nCommandType == ::com::sun::star::sdb::CommandType::TABLE; }
        inline  bool    editingQuery() const   { return m_nCommandType == ::com::sun::star::sdb::CommandType::QUERY; }
        inline  bool    editingCommand() const { return m_nCommandType == ::com::sun::star::sdb::CommandType::COMMAND; }

        sal_Bool askForNewName( const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>& _xElements,
                            sal_Bool _bSaveAs);
        // creates the querycomposer
        void setQueryComposer();
        void deleteIterator();
        void executeQuery();
        bool doSaveAsDoc(sal_Bool _bSaveAs);

        void saveViewSettings( ::comphelper::NamedValueCollection& o_rViewSettings, const bool i_includingCriteria ) const;
        void loadViewSettings( const ::comphelper::NamedValueCollection& o_rViewSettings );
        OUString translateStatement( bool _bFireStatementChange = true );

        OUString getDefaultName() const;

        void execute_QueryPropDlg();

    protected:
        // all the features which should be handled by this class
        virtual void            describeSupportedFeatures();
        // state of a feature. 'feature' may be the handle of a ::com::sun::star::util::URL somebody requested a dispatch interface for OR a toolbar slot.
        virtual FeatureState    GetState(sal_uInt16 nId) const;
        // execute a feature
        virtual void            Execute(sal_uInt16 nId, const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& aArgs);

        virtual void            reconnect( sal_Bool _bUI );
        virtual OUString getPrivateTitle( ) const;

        OQueryContainerWindow* getContainer() const { return static_cast< OQueryContainerWindow* >( getView() ); }

    public:
        OQueryController(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext >& _rM);

        virtual ~OQueryController();
        OTableFields&   getTableFieldDesc()         { return m_vTableFieldDesc; }
        OTableFields&   getUnUsedFields()           { return m_vUnUsedFieldsDesc; }

        void            clearFields();

        virtual void impl_onModifyChanged();

        // should the statement be parsed by our own sql parser
        sal_Bool        isEsacpeProcessing()    const { return m_bEscapeProcessing; }
        sal_Bool        isGraphicalDesign()     const { return m_bGraphicalDesign; }
        sal_Bool        isDistinct()            const { return m_bDistinct; }
        sal_Int64       getLimit()              const { return m_nLimit; }

        OUString getStatement()          const { return m_sStatement; }
        sal_Int32       getSplitPos()           const { return m_nSplitPos;}
        sal_Int32       getVisibleRows()        const { return m_nVisibleRows; }

        void            setDistinct(sal_Bool _bDistinct)        { m_bDistinct = _bDistinct;}
        void            setLimit(const sal_Int64 _nLimit)       { m_nLimit = _nLimit;}
        void            setSplitPos(sal_Int32 _nSplitPos)       { m_nSplitPos = _nSplitPos;}
        void            setVisibleRows(sal_Int32 _nVisibleRows) { m_nVisibleRows = _nVisibleRows;}

        sal_Int32       getColWidth(sal_uInt16 _nColPos) const;

        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >&
                        getFieldInformation() const { return m_aFieldInformation; }

        ::connectivity::OSQLParser&             getParser()         { return m_aSqlParser;  }
        ::connectivity::OSQLParseTreeIterator&  getParseIterator()  { return *m_pSqlIterator; }

        virtual sal_Bool Construct(Window* pParent);

        DECLARE_XINTERFACE( )
        DECLARE_XTYPEPROVIDER( )
        // XPropertySet
        virtual com::sun::star::uno::Reference<com::sun::star::beans::XPropertySetInfo>  SAL_CALL getPropertySetInfo() throw(com::sun::star::uno::RuntimeException, std::exception);
        virtual ::cppu::IPropertyArrayHelper& SAL_CALL getInfoHelper();

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException, std::exception);

        // ::com::sun::star::lang::XComponent
        virtual void        SAL_CALL disposing();

        virtual OUString SAL_CALL getImplementationName() throw(::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Sequence< OUString> SAL_CALL getSupportedServiceNames() throw(::com::sun::star::uno::RuntimeException, std::exception);
        // need by registration
        static OUString getImplementationName_Static() throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Sequence< OUString > getSupportedServiceNames_Static(void) throw( ::com::sun::star::uno::RuntimeException );
        static ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >
                SAL_CALL Create(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >&);

        // XController
        virtual ::com::sun::star::uno::Any SAL_CALL getViewData(void) throw( ::com::sun::star::uno::RuntimeException, std::exception );
        virtual void SAL_CALL restoreViewData(const ::com::sun::star::uno::Any& Data) throw( ::com::sun::star::uno::RuntimeException, std::exception );

    private:
        virtual void    onLoadedMenu(const ::com::sun::star::uno::Reference< ::com::sun::star::frame::XLayoutManager >& _xLayoutManager);
        // OPropertyArrayUsageHelper
        virtual ::cppu::IPropertyArrayHelper* createArrayHelper( ) const;

        // OPropertySetHelper
        virtual sal_Bool SAL_CALL convertFastPropertyValue(
                                    ::com::sun::star::uno::Any& rConvertedValue,
                                    ::com::sun::star::uno::Any& rOldValue,
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                                ) throw (::com::sun::star::lang::IllegalArgumentException);
        virtual void SAL_CALL setFastPropertyValue_NoBroadcast(
                                    sal_Int32 nHandle,
                                    const ::com::sun::star::uno::Any& rValue
                                ) throw (::com::sun::star::uno::Exception, std::exception );
        virtual void SAL_CALL getFastPropertyValue(
                                    ::com::sun::star::uno::Any& rValue,
                                    sal_Int32 nHandle
                                ) const;

        virtual OJoinDesignView*  getJoinView();
        // ask the user if the design should be saved when it is modified
        virtual short saveModified();
        virtual void reset();
        virtual void impl_initialize();

        void    impl_reset( const bool i_bIgnoreQuerySettings = false );
        /// tells the user that we needed to switch to SQL view automatically
        void    impl_showAutoSQLViewError( const ::com::sun::star::uno::Any& _rErrorDetails );

        /** switches to the graphical or SQL view mode, as determined by m_bGraphicalDesign
        */
        bool    impl_setViewMode( ::dbtools::SQLExceptionInfo* _pErrorInfo );

        /// sets m_sStatement, and notifies our respective property change listeners
        void    setStatement_fireEvent( const OUString& _rNewStatement, bool _bFireStatementChange = true );
        /// sets the m_bEscapeProcessing member, and notifies our respective property change listeners
        void    setEscapeProcessing_fireEvent( const sal_Bool _bEscapeProcessing );

        // OJoinController overridables
        virtual bool allowViews() const;
        virtual bool allowQueries() const;

    private:
        DECL_LINK( OnExecuteAddTable, void* );

    private:
        using OQueryController_PBase::getFastPropertyValue;
    };
}
#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_QUERYCONTROLLER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
