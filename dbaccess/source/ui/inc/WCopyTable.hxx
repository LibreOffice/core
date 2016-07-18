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

#ifndef INCLUDED_DBACCESS_SOURCE_UI_INC_WCOPYTABLE_HXX
#define INCLUDED_DBACCESS_SOURCE_UI_INC_WCOPYTABLE_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/sdbc/XConnection.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/sdbc/XDatabaseMetaData.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <comphelper/stl_types.hxx>
#include "TypeInfo.hxx"
#include <vcl/button.hxx>
#include <svtools/wizdlg.hxx>
#include "DExport.hxx"
#include "WTabPage.hxx"
#include "FieldDescriptions.hxx"
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <com/sun/star/sdbcx/XKeysSupplier.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <vcl/lstbox.hxx>
#include <functional>
#include <map>
#include <algorithm>

namespace dbaui
{

    typedef ::std::unary_function< OUString,bool> TColumnFindFunctorType;
    class TColumnFindFunctor : public TColumnFindFunctorType
    {
    public:
        virtual bool operator()(const OUString& _sColumnName) const = 0;

    protected:
        ~TColumnFindFunctor() {}
    };

    class TExportColumnFindFunctor : public TColumnFindFunctor
    {
        ODatabaseExport::TColumns* m_pColumns;
    public:
        TExportColumnFindFunctor(ODatabaseExport::TColumns* _pColumns)
        {
            m_pColumns = _pColumns;
        }

        virtual ~TExportColumnFindFunctor() {}

        bool operator()(const OUString& _sColumnName) const override
        {
            return m_pColumns->find(_sColumnName) != m_pColumns->end();
        }
    };

    class TMultiListBoxEntryFindFunctor : public TColumnFindFunctor
    {
        ::comphelper::UStringMixEqual m_aCase;
        ::std::vector< OUString>* m_pVector;
    public:
        TMultiListBoxEntryFindFunctor(::std::vector< OUString>* _pVector,
                                    const ::comphelper::UStringMixEqual& _aCase)
            :m_aCase(_aCase)
            ,m_pVector(_pVector)
        {
        }

        virtual ~TMultiListBoxEntryFindFunctor() {}

        bool operator()(const OUString& _sColumnName) const override
        {
            return ::std::any_of(m_pVector->begin(),m_pVector->end(),
                ::std::bind2nd(m_aCase, _sColumnName));
        }
    };

    // ICopyTableSourceObject
    /** interface to an object to copy to another DB, using the OCopyTableWizard

        when the wizard is used to copy an object to another DB, it usually requires
        a sdbcx-level or sdb-level object (a css.sdbcx.Table or css.sdb.Query, that is).

        However, to also support copying tables from sdbc-level connections, we allow to
        work with the object name only. This implies some less features (like copying the
        UI settings of a table is not done), but still allows to copy definition and data.
    */
    class ICopyTableSourceObject
    {
    public:
        /// retrieves the fully qualified name of the object to copy
        virtual OUString     getQualifiedObjectName() const = 0;
        /// determines whether the object is a view
        virtual bool                isView() const = 0;
        /** copies the UI settings of the object to the given target object. Might be
            ignored by implementations which do not have Ui settings.
        */
        virtual void                copyUISettingsTo( const css::uno::Reference< css::beans::XPropertySet >& _rxObject ) const = 0;
        /// retrieves the column names of the to-be-copied object
        virtual css::uno::Sequence< OUString >
                                    getColumnNames() const = 0;
        /// retrieves the names of the primary keys of the to-be-copied object
        virtual css::uno::Sequence< OUString >
                                    getPrimaryKeyColumnNames() const = 0;
        /// creates a OFieldDescription for the given column of the to-be-copied object
        virtual OFieldDescription*  createFieldDescription( const OUString& _rColumnName ) const = 0;
        /// returns the SELECT statement which can be used to retrieve the data of the to-be-copied object
        virtual OUString            getSelectStatement() const = 0;

        /** copies the filter and sorting
        *
        * \return
        */
        virtual void                copyFilterAndSortingTo(const css::uno::Reference< css::sdbc::XConnection >& _xConnection,const css::uno::Reference< css::beans::XPropertySet >& _rxObject ) const = 0;

        /** returns the prepared statement which can be used to retrieve the data of the to-be-copied object

            The default implementation of this method will simply prepare a statement with the return value
            of ->getSelectStatement.
        */
        virtual ::utl::SharedUNOComponent< css::sdbc::XPreparedStatement >
                                    getPreparedSelectStatement() const = 0;

        virtual ~ICopyTableSourceObject();
    };

    // ObjectCopySource
    class ObjectCopySource : public ICopyTableSourceObject
    {
    private:
        css::uno::Reference< css::sdbc::XConnection >         m_xConnection;
        css::uno::Reference< css::sdbc::XDatabaseMetaData >   m_xMetaData;
        css::uno::Reference< css::beans::XPropertySet >       m_xObject;
        css::uno::Reference< css::beans::XPropertySetInfo >   m_xObjectPSI;
        css::uno::Reference< css::container::XNameAccess >    m_xObjectColumns;

    public:
        ObjectCopySource(
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
            const css::uno::Reference< css::beans::XPropertySet >& _rxObject
        );

        // ICopyTableSourceObject overridables
        virtual OUString            getQualifiedObjectName() const override;
        virtual bool                isView() const override;
        virtual void                copyUISettingsTo( const css::uno::Reference< css::beans::XPropertySet >& _rxObject ) const override;
        virtual void                copyFilterAndSortingTo(const css::uno::Reference< css::sdbc::XConnection >& _xConnection, const css::uno::Reference< css::beans::XPropertySet >& _rxObject ) const override;
        virtual css::uno::Sequence< OUString >
                                    getColumnNames() const override;
        virtual css::uno::Sequence< OUString >
                                    getPrimaryKeyColumnNames() const override;
        virtual OFieldDescription*  createFieldDescription( const OUString& _rColumnName ) const override;
        virtual OUString     getSelectStatement() const override;
        virtual ::utl::SharedUNOComponent< css::sdbc::XPreparedStatement >
                                    getPreparedSelectStatement() const override;
    };

    // NamedTableCopySource
    class NamedTableCopySource : public ICopyTableSourceObject
    {
    private:
        css::uno::Reference< css::sdbc::XConnection >                      m_xConnection;
        css::uno::Reference< css::sdbc::XDatabaseMetaData >                m_xMetaData;
        OUString                                                           m_sTableName;
        OUString                                                           m_sTableCatalog;
        OUString                                                           m_sTableSchema;
        OUString                                                           m_sTableBareName;
        ::std::vector< OFieldDescription >                                 m_aColumnInfo;
        ::utl::SharedUNOComponent< css::sdbc::XPreparedStatement >         m_xStatement;

    public:
        NamedTableCopySource(
            const css::uno::Reference< css::sdbc::XConnection >& _rxConnection,
            const OUString& _rTableName
        );

        // ICopyTableSourceObject overridables
        virtual OUString     getQualifiedObjectName() const override;
        virtual bool                isView() const override;
        virtual void                copyUISettingsTo( const css::uno::Reference< css::beans::XPropertySet >& _rxObject ) const override;
        virtual void                copyFilterAndSortingTo(const css::uno::Reference< css::sdbc::XConnection >& _xConnection,const css::uno::Reference< css::beans::XPropertySet >& _rxObject ) const override;
        virtual css::uno::Sequence< OUString >
                                    getColumnNames() const override;
        virtual css::uno::Sequence< OUString >
                                    getPrimaryKeyColumnNames() const override;
        virtual OFieldDescription*  createFieldDescription( const OUString& _rColumnName ) const override;
        virtual OUString     getSelectStatement() const override;
        virtual ::utl::SharedUNOComponent< css::sdbc::XPreparedStatement >
                                    getPreparedSelectStatement() const override;

    private:
        void    impl_ensureColumnInfo_throw();
        ::utl::SharedUNOComponent< css::sdbc::XPreparedStatement > const &
                impl_ensureStatement_throw();
    };

    // Wizard Dialog
    class OCopyTableWizard : public WizardDialog
    {
        friend class        OWizColumnSelect;
        friend class        OWizTypeSelect;
        friend class        OWizTypeSelectControl;
        friend class        OCopyTable;
        friend class        OWizNameMatching;

    public:
        typedef std::map<OUString, OUString, ::comphelper::UStringMixLess> TNameMapping;

        enum Wizard_Button_Style
        {
            WIZARD_NEXT,
            WIZARD_PREV,
            WIZARD_FINISH,

            WIZARD_NONE
        };

    private:
        ODatabaseExport::TColumns       m_vDestColumns; // contains the columns
        ODatabaseExport::TColumnVector  m_aDestVec;     // the order to insert the columns
        ODatabaseExport::TColumns       m_vSourceColumns;
        ODatabaseExport::TColumnVector  m_vSourceVec;

        VclPtr<HelpButton>             m_pbHelp;
        VclPtr<CancelButton>           m_pbCancel;
        VclPtr<PushButton>             m_pbPrev;
        VclPtr<PushButton>             m_pbNext;
        VclPtr<PushButton>             m_pbFinish;

        OTypeInfoMap                            m_aTypeInfo;
        ::std::vector<OTypeInfoMap::iterator>   m_aTypeInfoIndex;
        OTypeInfoMap                            m_aDestTypeInfo;
        ::std::vector<OTypeInfoMap::iterator>   m_aDestTypeInfoIndex;
        TNameMapping                            m_mNameMapping;

        ODatabaseExport::TPositions             m_vColumnPos;
        ::std::vector<sal_Int32>                m_vColumnTypes;

        css::uno::Reference< css::sdbc::XConnection >         m_xDestConnection;

        const ICopyTableSourceObject&                         m_rSourceObject;

        css::uno::Reference< css::util::XNumberFormatter >    m_xFormatter;
        css::uno::Reference< css::uno::XComponentContext>     m_xContext;
        css::uno::Reference< css::task::XInteractionHandler>  m_xInteractionHandler;

        OUString                m_sTypeNames;       // these type names are the ones out of the resource file
        sal_uInt32              m_nPageCount;
        bool                    m_bDeleteSourceColumns;
        bool                    m_bInterConnectionCopy;    // are we copying between different connections?

        css::lang::Locale        m_aLocale;
        OUString                 m_sName;    // for a table the name is composed
        OUString                 m_sSourceName;
        OUString                 m_aKeyName;
        TOTypeInfoSP             m_pTypeInfo; // default type
        bool                     m_bAddPKFirstTime;
        sal_Int16                m_nOperation;
        Wizard_Button_Style      m_ePressed;
        bool                     m_bCreatePrimaryKeyColumn;
        bool                     m_bUseHeaderLine;

    private:
        DECL_LINK_TYPED( ImplPrevHdl, Button*, void );
        DECL_LINK_TYPED( ImplNextHdl, Button*, void);
        DECL_LINK_TYPED( ImplOKHdl, Button*, void );
        DECL_LINK_TYPED( ImplActivateHdl, WizardDialog*, void );
        bool CheckColumns(sal_Int32& _rnBreakPos);
        void loadData( const ICopyTableSourceObject& _rSourceObject,
                       ODatabaseExport::TColumns& _rColumns,
                       ODatabaseExport::TColumnVector& _rColVector );
        void construct();
        // need for table creation
        static void appendColumns( css::uno::Reference< css::sdbcx::XColumnsSupplier>& _rxColSup, const ODatabaseExport::TColumnVector* _pVec, bool _bKeyColumns = false );
        static void appendKey(css::uno::Reference< css::sdbcx::XKeysSupplier>& _rxSup,const ODatabaseExport::TColumnVector* _pVec);
        // checks if the type is supported in the destination database
        bool supportsType(sal_Int32 _nDataType,sal_Int32& _rNewDataType);

        void    impl_loadSourceData();

    public:
        // used for copy tables or queries
        OCopyTableWizard(
            vcl::Window * pParent,
            const OUString& _rDefaultName,
            sal_Int16 _nOperation,
            const ICopyTableSourceObject&                                                           _rSourceObject,
            const css::uno::Reference< css::sdbc::XConnection >&          _xSourceConnection,
            const css::uno::Reference< css::sdbc::XConnection >&          _xConnection,
            const css::uno::Reference< css::uno::XComponentContext >&     _rxContext,
            const css::uno::Reference< css::task::XInteractionHandler>&   _xInteractionHandler
        );

        // used for importing rtf/html sources
        OCopyTableWizard(
            vcl::Window* pParent,
            const OUString& _rDefaultName,
            sal_Int16 _nOperation,
            const ODatabaseExport::TColumns& _rDestColumns,
            const ODatabaseExport::TColumnVector& _rSourceColVec,
            const css::uno::Reference< css::sdbc::XConnection >& _xConnection,
            const css::uno::Reference< css::util::XNumberFormatter >& _xFormatter,
            TypeSelectionPageFactory _pTypeSelectionPageFactory,
            SvStream& _rTypeSelectionPageArg,
            const css::uno::Reference< css::uno::XComponentContext >& _rxContext
        );

        virtual ~OCopyTableWizard();
        virtual void        dispose() override;

        virtual bool        DeactivatePage() override;
        OKButton&           GetOKButton() { return static_cast<OKButton&>(*m_pbFinish); }
        Wizard_Button_Style GetPressedButton() const { return m_ePressed; }
        void                EnableNextButton(bool bEnable);
        void                AddWizardPage(OWizardPage* pPage); // delete page from OCopyTableWizard
        void                CheckButtons(); // checks which button can be disabled, enabled

        // returns a vector where the position of a column and if the column is in the selection
        // when not the value is COLUMN_POSITION_NOT_FOUND == (sal_uInt32)-1
        const ODatabaseExport::TPositions& GetColumnPositions()    const { return m_vColumnPos; }
        const ::std::vector<sal_Int32>&    GetColumnTypes()        const { return m_vColumnTypes; }
        bool                        UseHeaderLine()         const { return m_bUseHeaderLine; }
        void                        setUseHeaderLine(bool _bUseHeaderLine) { m_bUseHeaderLine = _bUseHeaderLine; }

        void insertColumn(sal_Int32 _nPos,OFieldDescription* _pField);

        /** replaces a field description with another one. The name must not be known so far.
            @param  _nPos
                The pos inside the vector, 0 based.
            @param  _pField
                The field to set.
            @param  _sOldName
                The name of column to be replaced.
        */
        void replaceColumn(sal_Int32 _nPos,OFieldDescription* _pField,const OUString& _sOldName);

        /** returns whether a primary key should be created in the target database
        */
        bool            shouldCreatePrimaryKey() const { return m_bCreatePrimaryKeyColumn;}
        void            setCreatePrimaryKey( bool _bDoCreate, const OUString& _rSuggestedName );

        static bool     supportsPrimaryKey( const css::uno::Reference< css::sdbc::XConnection >& _rxConnection );
        bool            supportsPrimaryKey() const { return supportsPrimaryKey( m_xDestConnection ); }

        static bool     supportsViews( const css::uno::Reference< css::sdbc::XConnection >& _rxConnection );
        bool            supportsViews() const { return supportsViews( m_xDestConnection ); }

        /** returns the name of the primary key
            @return
                The name of the primary key.
        */
        const OUString& getPrimaryKeyName() const { return m_aKeyName; }

        const OTypeInfoMap& getTypeInfo()                       const { return m_aTypeInfo; }

        TOTypeInfoSP        getDestTypeInfo(sal_Int32 _nPos)    const { return m_aDestTypeInfoIndex[_nPos]->second; }
        const OTypeInfoMap& getDestTypeInfo()                   const { return m_aDestTypeInfo; }

        const css::lang::Locale&  GetLocale() const { return m_aLocale; }
        const css::uno::Reference< css::util::XNumberFormatter >& GetFormatter() const { return m_xFormatter; }
        const css::uno::Reference< css::uno::XComponentContext>& GetComponentContext() const { return m_xContext; }

        const ODatabaseExport::TColumns&        getSourceColumns() const{ return m_vSourceColumns; }
        const ODatabaseExport::TColumnVector&   getSrcVector() const    { return m_vSourceVec; }
        ODatabaseExport::TColumns&              getDestColumns()        { return m_vDestColumns; }
        const ODatabaseExport::TColumnVector&   getDestVector() const   { return m_aDestVec; }
        const OUString& getName() const { return m_sName; }

        /** clears the dest vectors
        */
        void clearDestColumns();

        css::uno::Reference< css::beans::XPropertySet > createTable();
        css::uno::Reference< css::beans::XPropertySet > createView() const;
        sal_Int32 getMaxColumnNameLength() const;

        void setOperation( const sal_Int16 _nOperation );
        sal_Int16 getOperation() const { return m_nOperation;}

        OUString convertColumnName(  const TColumnFindFunctor&   _rCmpFunctor,
                                            const OUString&  _sColumnName,
                                            const OUString&  _sExtraChars,
                                            sal_Int32               _nMaxNameLen);
        TOTypeInfoSP convertType(const TOTypeInfoSP&_pType, bool& _bNotConvert);

        OUString createUniqueName(const OUString& _sName);

        // displays a error message that a column type is not supported
        void showColumnTypeNotSupported(const OUString& _rColumnName);

        void removeColumnNameFromNameMap(const OUString& _sName);
        void showError(const OUString& _sErrorMessage);
        void showError(const css::uno::Any& _aError);
    };
}

#endif // INCLUDED_DBACCESS_SOURCE_UI_INC_WCOPYTABLE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
