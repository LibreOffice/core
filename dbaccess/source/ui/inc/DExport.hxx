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
#ifndef DBAUI_DATABASEEXPORT_HXX
#define DBAUI_DATABASEEXPORT_HXX

#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XResultSetMetaData.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/util/XNumberFormatter.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/util/Date.hpp>
#include <vector>
#include <tools/string.hxx>
#include <comphelper/stl_types.hxx>
#include "TypeInfo.hxx"
#include "WTypeSelect.hxx"
#include "commontypes.hxx"
#include "IUpdateHelper.hxx"

namespace com { namespace sun { namespace star {
    namespace awt{
        struct FontDescriptor;
    }
    namespace sdbc{
        class XPreparedStatement;
        class XDatabaseMetaData;
    }
}}}

#define COLUMN_POSITION_NOT_FOUND   ((sal_Int32)-1)

class SvNumberFormatter;
namespace dbaui
{
    class OFieldDescription;
    class ODatabaseExport
    {
    public:
        DECLARE_STL_MAP(::rtl::OUString,OFieldDescription*,::comphelper::UStringMixLess,TColumns);
        typedef ::std::vector<TColumns::const_iterator>             TColumnVector;
        typedef ::std::vector< ::std::pair<sal_Int32,sal_Int32> >   TPositions;

    protected:
        TPositions                      m_vColumns;     ///< columns to be used
        ::std::vector<sal_Int32>        m_vColumnTypes; ///< ColumnTypes for faster access
        ::std::vector<sal_Int32>        m_vColumnSize;
        ::std::vector<sal_Int16>        m_vNumberFormat;
        ::com::sun::star::lang::Locale  m_aLocale;

        TColumns                        m_aDestColumns; ///< container for new created columns
        TColumnVector                   m_vDestVector;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xTable;       ///< dest table
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>     m_xTables;      ///< container
        SharedConnection                                                                m_xConnection;  ///< dest conn

        ::boost::shared_ptr<IUpdateHelper>                                              m_pUpdateHelper;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >          m_xResultSet;
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    m_xFormatter;   ///< a number formatter working with the connection's NumberFormatsSupplier
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> m_xFactory;
        ::com::sun::star::util::Date                                                    m_aNullDate;

        SvNumberFormatter*  m_pFormatter;
        SvStream&           m_rInputStream;
        /// for saving the selected tablename
        ::rtl::OUString     m_sDefaultTableName;

        String              m_sTextToken;   ///< cell content
        String              m_sNumToken;    ///< SDNUM value
        String              m_sValToken;    ///< SDVAL value
        TOTypeInfoSP        m_pTypeInfo;    ///< contains the default type
        const TColumnVector* m_pColumnList;
        const OTypeInfoMap* m_pInfoMap;
        sal_Int32           m_nColumnPos;   ///< current column position
        sal_Int32           m_nRows;        ///< number of rows to be searched
        sal_Int32           m_nRowCount;    ///< current count of rows
        rtl_TextEncoding    m_nDefToken;    ///< language
        sal_Bool            m_bError;       ///< error and termination code
        sal_Bool            m_bInTbl;       ///< true, if parser is in RTF table
        sal_Bool            m_bHead;        ///< true, if the header hasn't been read yet
        sal_Bool            m_bDontAskAgain;///< if there is an error when pasting, don't show it again
        sal_Bool            m_bIsAutoIncrement; ///< if PKey is set by user
        sal_Bool            m_bFoundTable;  ///< set to true when a table was found
        sal_Bool            m_bCheckOnly;
        bool                m_bAppendFirstLine;


        virtual sal_Bool    CreateTable(int nToken)         = 0;
        virtual TypeSelectionPageFactory
                            getTypeSelectionPageFactory()   = 0;

        void                CreateDefaultColumn(const ::rtl::OUString& _rColumnName);
        sal_Int16           CheckString(const String& aToken, sal_Int16 _nOldNumberFormat);
        void                adjustFormat();
        void                eraseTokens();
        void                insertValueIntoColumn();
        sal_Bool            createRowSet();
        void                showErrorDialog(const ::com::sun::star::sdbc::SQLException& e);
        void                ensureFormatter();

        /** executeWizard calls a wizard to create/append data

            @param  _sTableName the tablename
            @param  _aTextColor the text color of the new created table
            @param  _rFont      the font of the new table

            @return true when an error occurs
        */
        sal_Bool            executeWizard( const ::rtl::OUString& _sTableName,
                                           const ::com::sun::star::uno::Any& _aTextColor,
                                           const ::com::sun::star::awt::FontDescriptor& _rFont);

        virtual ~ODatabaseExport();

    public:
        ODatabaseExport(
            const SharedConnection& _rxConnection,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
            const TColumnVector* rList,
            const OTypeInfoMap* _pInfoMap,
            SvStream& _rInputStream
        );

        // required for automatic type recognition
        ODatabaseExport(
            sal_Int32 nRows,
            const TPositions& _rColumnPositions,
            const ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >& _rxNumberF,
            const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& _rM,
            const TColumnVector* rList,
            const OTypeInfoMap* _pInfoMap,
            sal_Bool _bAutoIncrementEnabled,
            SvStream& _rInputStream
        );

        void    SetColumnTypes(const TColumnVector* rList,const OTypeInfoMap* _pInfoMap);

        inline void                    SetTableName(const ::rtl::OUString &_sTableName){ m_sDefaultTableName = _sTableName ; }

        virtual void release() = 0;

        void enableCheckOnly() { m_bCheckOnly = sal_True; }
        sal_Bool isCheckEnabled() const { return m_bCheckOnly; }

        static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > createPreparedStatment( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _xMetaData
                                                       ,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xDestTable
                                                       ,const TPositions& _rvColumns);
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
