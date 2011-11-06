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
#include "WTypeSelect.hxx"

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

class Window;
class SvNumberFormatter;
namespace dbaui
{
    class OFieldDescription;
    class OTypeInfo;
    class OWizTypeSelect;
    class ODatabaseExport
    {
    public:
        DECLARE_STL_MAP(::rtl::OUString,OFieldDescription*,::comphelper::UStringMixLess,TColumns);
        typedef ::std::vector<TColumns::const_iterator>             TColumnVector;
        typedef ::std::vector< ::std::pair<sal_Int32,sal_Int32> >   TPositions;

    protected:
        TPositions                      m_vColumns;     // Welche Spalten "ubernommen werden sollen
        ::std::vector<sal_Int32>        m_vColumnTypes; // FeldTypen f"ur schnelleren Zugriff
        ::std::vector<sal_Int32>        m_vColumnSize;
        ::std::vector<sal_Int16>        m_vNumberFormat;
        ::com::sun::star::lang::Locale  m_aLocale;

        TColumns                        m_aDestColumns; // container for new created columns
        TColumnVector                   m_vDestVector;

        ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet >       m_xTable;       // dest table
        ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess>     m_xTables;      // container
        SharedConnection                                                                m_xConnection;  // dest conn

        ::boost::shared_ptr<IUpdateHelper>                                              m_pUpdateHelper;
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet >          m_xResultSet;   //
        ::com::sun::star::uno::Reference< ::com::sun::star::util::XNumberFormatter >    m_xFormatter;   // a number formatter working with the connection's NumberFormatsSupplier
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory> m_xFactory;
        ::com::sun::star::util::Date                                                    m_aNullDate;

        SvNumberFormatter*  m_pFormatter;
        SvStream&           m_rInputStream;
        //dyf add 2006/06/01
        //for save the selected tablename
        ::rtl::OUString     m_sDefaultTableName;
        //dyf add end
        String              m_sTextToken;       // Zellen Inhalt
        String              m_sNumToken;        /// SDNUM value
        String              m_sValToken;        /// SDVAL value
        TOTypeInfoSP        m_pTypeInfo;    // contains the default type
        const TColumnVector* m_pColumnList;
        const OTypeInfoMap* m_pInfoMap;
        sal_Int32           m_nColumnPos;       // aktuelle Spaltenposition
        sal_Int32           m_nRows;        // Anzahl der Zeilen die durchsucht werden sollen
        sal_Int32           m_nRowCount;    // current count of rows
        rtl_TextEncoding    m_nDefToken;        // Sprache
        sal_Bool            m_bError;           // Fehler und Abbruchstatus
        sal_Bool            m_bInTbl;           // Ist gesetzt, wenn der Parser sich in der RTF Tabelle befindet
        sal_Bool            m_bHead;        // ist true, wenn die Kopfzeile noch nicht gelesen wurde
        sal_Bool            m_bDontAskAgain;// Falls beim Einf"ugen ein Fehler auftritt, soll die Fehlermeldung nicht
        sal_Bool            m_bIsAutoIncrement; // if PKey is set by user
        sal_Bool            m_bFoundTable;      // set to true when a table was found
        sal_Bool            m_bCheckOnly;
        bool                m_bAppendFirstLine;


        virtual sal_Bool        CreateTable(int nToken)         = 0;
        virtual TypeSelectionPageFactory
                                getTypeSelectionPageFactory()   = 0;

        void                    CreateDefaultColumn(const ::rtl::OUString& _rColumnName);
        sal_Int16               CheckString(const String& aToken, sal_Int16 _nOldNumberFormat);
        void                    adjustFormat();
        void                    eraseTokens();
        void                    insertValueIntoColumn();
        sal_Bool                createRowSet();
        void                    showErrorDialog(const ::com::sun::star::sdbc::SQLException& e);
        void                    ensureFormatter();

        /** executeWizard calls a wizard to create/append data
            @param  _sTableName the tablename
            @param  _aTextColor the text color of the new created table
            @param  _rFont      the font of the new table

            @return true when an error occurs
        */
        sal_Bool                executeWizard(  const ::rtl::OUString& _sTableName,
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

        // wird f"ur auto. Typ-Erkennung gebraucht
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

        //dyf add 20070601
        inline void                    SetTableName(const ::rtl::OUString &_sTableName){ m_sDefaultTableName = _sTableName ; }
        //dyf add end

        virtual void release() = 0;

        void enableCheckOnly() { m_bCheckOnly = sal_True; }
        sal_Bool isCheckEnabled() const { return m_bCheckOnly; }

        static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XPreparedStatement > createPreparedStatment( const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDatabaseMetaData>& _xMetaData
                                                       ,const ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet>& _xDestTable
                                                       ,const TPositions& _rvColumns);
    };
}

#endif // DBAUI_DATABASEEXPORT_HXX





