/*************************************************************************
 *
 *  $RCSfile: dbmgr.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: os $ $Date: 2000-11-13 08:25:32 $
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
#ifndef _SWDBMGR_HXX
#define _SWDBMGR_HXX


#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _SBAITEMS_HXX
#include "offmgr/sbaitems.hxx"
#endif
#ifndef _COM_SUN_STAR_UTIL_DATE_HPP_
#include <com/sun/star/util/Date.hpp>
#endif

#include "swtypes.hxx"  // fuer aEmptyStr

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_LANG_LOCALE_HPP_
#include <com/sun/star/lang/Locale.hpp>
#endif
namespace com{namespace sun{namespace star{
    namespace sdbc{
        class XConnection;
        class XStatement;
        class XDataSource;
        class XResultSet;
    }
    namespace beans{

        class XPropertySet;
        struct PropertyValue;
    }
    namespace sdbcx{
        class XColumnsSupplier;
    }
    namespace util{
        class XNumberFormatter;
    }
}}}

struct SwDBFormatData
{
    com::sun::star::util::Date aNullDate;
    com::sun::star::uno::Reference< com::sun::star::util::XNumberFormatter> xFormatter;
    com::sun::star::lang::Locale aLocale;
};

class SwView;
class SwWrtShell;
class SwPrtOptions;
class SfxProgress;
class ListBox;
class Button;
class SvNumberFormatter;

// -----------------------------------------------------------------------

enum DBMgrOptions
{
    DBMGR_MERGE,            // Datensaetze in Felder
    DBMGR_INSERT,           // Datensaetze in Text
    DBMGR_MERGE_MAILMERGE,  // Serienbriefe drucken
    DBMGR_MERGE_MAILING,    // Serienbriefe als email versenden
    DBMGR_MERGE_MAILFILES   // Serienbriefe als Datei(en) speichern
};

// -----------------------------------------------------------------------

/*--------------------------------------------------------------------
     Beschreibung: (neue) Logische Datenbanken verwalten
 --------------------------------------------------------------------*/
#define SW_DB_SELECT_UNKNOWN    0
#define SW_DB_SELECT_TABLE      1
#define SW_DB_SELECT_QUERY      2

struct SwDSParam
{
    String                      sDataSource;
    String                      sTableOrQuery;
    BYTE                        nTableOrQuery;
    String                      sStatement;
    com::sun::star::util::Date  aNullDate;

    ::com::sun::star::uno::Reference<com::sun::star::util::XNumberFormatter>    xFormatter;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>      xConnection;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement>       xStatement;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>       xResultSet;
    ::com::sun::star::uno::Sequence< sal_Int32 >                                aSelection;
    SbaSelectionListRef         xSelectionList;
    BOOL bScrollable;
    BOOL bSelectionList;
    BOOL bEndOfDB;
    long nSelectionIndex;

    SwDSParam(const String& rSource, const String& rTable, BYTE nType, const String& rStatement) :
        sDataSource(rSource),
        sTableOrQuery(rTable),
        nTableOrQuery(nType),
        sStatement(rStatement),
        bScrollable(FALSE),
        bSelectionList(FALSE),
        bEndOfDB(FALSE),
        nSelectionIndex(0)
        {}

    SwDSParam(const String& rSource, const String& rTable, BYTE nType,
        ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>       xResSet,
        ::com::sun::star::uno::Sequence< sal_Int32 >    rSelection) :
        sDataSource(rSource),
        sTableOrQuery(rTable),
        nTableOrQuery(nType),
        bScrollable(TRUE),
        bSelectionList(FALSE),
        bEndOfDB(FALSE),
        nSelectionIndex(0),
        xResultSet(xResSet),
        aSelection(rSelection)
        {}
};
typedef SwDSParam* SwDSParamPtr;
SV_DECL_PTRARR_DEL(SwDSParamArr, SwDSParamPtr, 0, 5)


class SwNewDBMgr
{
    SbaSelectionListRef pMergeList;     // Liste der fÅr Serienbrief selektierten EintrÑge
    String              sEMailAddrFld;  // Mailing: Spaltenname der E-Mail Adresse
    String              sSubject;       // Mailing: Subject
    String              sAttached;      // Mailing: Attachte Files
    USHORT              nMergeType;
    BOOL                bInitDBFields : 1;  // TRUE: Datenbank an Feldern beim Mergen
    BOOL                bSingleJobs : 1;    // Einzelne Druckjobs bei Aufruf aus Basic
    BOOL                bCancel : 1;        // Serienbrief-Save abgebrochen

    BOOL                bInMerge    : 1;    //merge process active
    SwDSParamArr        aDataSourceParams;

    SwDSParam*          pMergeData;

    BOOL                OpenMergeSource(const String& rDataSource,
                            const String& rDataTableOrQuery,
                            const String& rStatement,
                            const SbaSelectionListRef xSelectionList);
    SwDSParam*          FindDSData(const String& rDBName, BOOL bCreate);


    DECL_LINK( PrtCancelHdl, Button * );

    // Datensaetze als Text ins Dokument einfuegen
    void ImportFromConnection( SwWrtShell* pSh);

    // Einzelnen Datensatz als Text ins Dokument einfuegen
    void ImportDBEntry(SwWrtShell* pSh);

    // Mischen von Datensaetzen in Felder, dann per email versenden
    BOOL            MergeMailing(SwWrtShell* pSh);
    // Mischen von Datensaetzen in Felder, dann als Datei abspeichern
    BOOL            MergeMailFiles(SwWrtShell* pSh);
public:
    SwNewDBMgr();
    ~SwNewDBMgr();
    // Am Dokument Datenbank- Tabellenname und SQL-Select-Statement setzen
    void            ChgDBName( SwWrtShell* pSh,
                        const String& rDataSource,
                        const String& rTableOrQuery,
                                const String& rStatement );

    // Art des aktellen Mergens. Siehe DBMgrOptions-enum
    inline USHORT   GetMergeType() const            { return nMergeType; }
    inline void     SetMergeType( USHORT nTyp )     { nMergeType = nTyp; }

    // Mischen von Datensaetzen in Felder
    BOOL            MergeNew(USHORT nOpt, SwWrtShell& rSh,
                        const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>& rProperties,
                        const String *pPrinter = NULL);
    BOOL            Merge(USHORT nOpt, SwWrtShell* pSh, const String& rStatement,
                        const SbaSelectionListRef pSelectionList,
                        const String& rDataSource,
                        const String& rTableOrQuery,
                        const String *pPrinter = NULL);
    BOOL            Merge(SwWrtShell* pSh);
    // Mischen von Datensaetzen in Felder, dann drucken
    BOOL            MergePrint( SwView& rView,
                                SwPrtOptions& rOpt, SfxProgress& rProgress );
    inline SbaSelectionListRef& GetMergeList()  { return pMergeList; }

    // Datenbankfelder mit fehlendem Datenbankname initialisieren
    inline BOOL     IsInitDBFields() const  { return bInitDBFields; }
    inline void     SetInitDBFields(BOOL b) { bInitDBFields = b;    }

    // Serienbriefe einzelnd oder alle zusammen drucken/speichern
    inline BOOL     IsSingleJobs() const    { return bSingleJobs;   }
    inline void     SetSingleJobs(BOOL b)   { bSingleJobs = b;  }

    // Mailing
    // email-Daten setzen
    inline void     SetEMailColumn(const String& sColName) { sEMailAddrFld = sColName; }
    inline void     SetSubject(const String& sSbj) { sSubject = sSbj; }
    inline void     SetAttachment(const String& sAtt) { sAttached = sAtt; }


    // Listbox mit allen Tabellennamen einer Datenbank fuellen
    BOOL            GetTableNames(ListBox* pListBox, const String& rDBName );
    // Listbox mit allen Spaltennamen einer Datenbanktabelle fuellen
    BOOL            GetColumnNames(ListBox* pListBox,
                        const String& rDBName, const String& rTableName, BOOL bAppend = FALSE);
    ULONG GetColumnFmt( const String& rDBName,
                        const String& rTableName,
                        const String& rColNm,
                        SvNumberFormatter* pNFmtr,
                        long nLanguage );
    sal_Int32 GetColumnType( const String& rDBName,
                          const String& rTableName,
                          const String& rColNm );

    inline BOOL     IsInMerge() const   { return bInMerge; }
    void            EndMerge();

    void            ExecuteFormLetter(SwWrtShell& rSh,
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rProperties);

    void            InsertText(SwWrtShell& rSh,
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rProperties);

    // check if a data source is open as merge source
    BOOL            IsDataSourceOpen(const String& rDataSource, const String& rTableOrQuery)const;

    // add data source information to the data source array - was PreInitDBData
    void            AddDSData(const String& rDBName, long nSelStart, long nSelEnd);
    void            GetDSSelection(const String& sDBDesc, long& rSelStart, long& rSelEnd);

    // open the source while fields are updated - for the calculator only!
    BOOL            OpenDataSource(const String& rDataSource, const String& rTableOrQuery);
    sal_uInt32      GetSelectedRecordId(const String& rDataSource, const String& rTableOrQuery);
    BOOL            GetColumnCnt(const String& rSourceName, const String& rTableName,
                            const String& rColumnName, sal_uInt32 nAbsRecordId, long nLanguage,
                            String& rResult, double* pNumber);

    const SwDSParam* CreateDSData(const String& rDBName)
                        {return FindDSData(rDBName, TRUE);}
    const SwDSParamArr& GetDSParamArray() const {return aDataSourceParams;}


    //close all data sources - after fields were updated
    void            CloseAll(BOOL bIncludingMerge = TRUE);

    BOOL            GetMergeColumnCnt(const String& rColumnName, USHORT nLanguage,
                                String &rResult, double *pNumber, sal_uInt32 *pFormat);
    BOOL            ToNextMergeRecord();
    BOOL            ExistsNextRecord()const;
    sal_uInt32      GetSelectedRecordId();
    sal_Bool        ToRecordId(sal_Int32 nSet);

    BOOL            ShowInBeamer(const String& rDBName, const String& rTableName,
                                                BYTE nType, const String& rStatement);

    const String&   GetAddressDBName();

    static String GetDBField(
                    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xColumnProp,
                    const SwDBFormatData& rDBFormatData,
                    double *pNumber = NULL);

    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>
            GetConnection(const String& rDataSource,
                ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>& rxSource);

    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>
            GetColumnSupplier(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>,
                                    const String& rTableOrQuery,
                                    BYTE    eTableOrQuery = SW_DB_SELECT_UNKNOWN);

    static ::com::sun::star::uno::Sequence<rtl::OUString> GetExistingDatabaseNames();

};

#endif

