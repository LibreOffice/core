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
#ifndef INCLUDED_SW_INC_DBMGR_HXX
#define INCLUDED_SW_INC_DBMGR_HXX

#include <rtl/ustring.hxx>
#include <tools/link.hxx>
#include <com/sun/star/util/Date.hpp>
#include "swdllapi.h"
#include <swdbdata.hxx>
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>
#include <boost/ptr_container/ptr_vector.hpp>
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
    namespace mail{
        class XSmtpService;
    }
}}}
namespace svx {
    class ODataAccessDescriptor;
}

struct SwDBFormatData
{
    com::sun::star::util::Date aNullDate;
    com::sun::star::uno::Reference< com::sun::star::util::XNumberFormatter> xFormatter;
    com::sun::star::lang::Locale aLocale;
};

class SwView;
class SwWrtShell;
class SfxProgress;
class ListBox;
class Button;
class SvNumberFormatter;
class SwDbtoolsClient;
class SwXMailMerge;
class SwMailMergeConfigItem;

enum DBMgrOptions
{
    DBMGR_MERGE,             ///< Data records in fields.
    DBMGR_INSERT,            ///< Data records in text.
    DBMGR_MERGE_MAILMERGE,   ///< Print mail merge.
    DBMGR_MERGE_MAILING,     ///< Send mail merge as email.
    DBMGR_MERGE_MAILFILES,   ///< Save mail merg as files.
    DBMGR_MERGE_DOCUMENTS,   ///< Print merged documents.
    DBMGR_MERGE_SINGLE_FILE  ///< Save merge as single file.
};

/*--------------------------------------------------------------------
     Administration of (new) logical databases.
 --------------------------------------------------------------------*/
#define SW_DB_SELECT_UNKNOWN    0
#define SW_DB_SELECT_TABLE      1
#define SW_DB_SELECT_QUERY      2

struct SwDSParam : public SwDBData
{
    com::sun::star::util::Date  aNullDate;

    ::com::sun::star::uno::Reference<com::sun::star::util::XNumberFormatter>    xFormatter;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>      xConnection;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement>       xStatement;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>       xResultSet;
    ::com::sun::star::uno::Sequence<  ::com::sun::star::uno::Any >              aSelection;
    sal_Bool bScrollable;
    sal_Bool bEndOfDB;
    sal_Bool bAfterSelection;
    long nSelectionIndex;

    SwDSParam(const SwDBData& rData) :
        SwDBData(rData),
        bScrollable(sal_False),
        bEndOfDB(sal_False),
        bAfterSelection(sal_False),
        nSelectionIndex(0)
        {}

    SwDSParam(const SwDBData& rData,
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>&    xResSet,
        const ::com::sun::star::uno::Sequence<  ::com::sun::star::uno::Any >&   rSelection) :
        SwDBData(rData),
        xResultSet(xResSet),
        aSelection(rSelection),
        bScrollable(sal_True),
        bEndOfDB(sal_False),
        bAfterSelection(sal_False),
        nSelectionIndex(0)
        {}

        void CheckEndOfDB()
        {
            if(bEndOfDB)
                bAfterSelection = sal_True;
        }
};
typedef boost::ptr_vector<SwDSParam> SwDSParamArr;

struct SwMergeDescriptor
{
    sal_uInt16                                              nMergeType;
    SwWrtShell&                                         rSh;
    const ::svx::ODataAccessDescriptor&                 rDescriptor;
    OUString                                            sSaveToFilter; ///< export filter to save resulting files
    OUString                                            sSaveToFilterOptions;
    ::css::uno::Sequence< ::css::beans::PropertyValue > aSaveToFilterData;

    OUString                                            sSubject;
    OUString                                            sAddressFromColumn;
    OUString                                            sMailBody;
    OUString                                            sAttachmentName;
    ::css::uno::Sequence< OUString >                    aCopiesTo;
    ::css::uno::Sequence< OUString >                    aBlindCopiesTo;

    ::css::uno::Reference< ::css::mail::XSmtpService >  xSmtpServer;

    sal_Bool                                            bSendAsHTML;
    sal_Bool                                            bSendAsAttachment;

    sal_Bool                                            bPrintAsync;
    sal_Bool                                            bCreateSingleFile;

    SwMailMergeConfigItem*                              pMailMergeConfigItem;

    ::com::sun::star::uno::Sequence<  ::com::sun::star::beans::PropertyValue >  aPrintOptions;

    SwMergeDescriptor( sal_uInt16 nType, SwWrtShell& rShell, ::svx::ODataAccessDescriptor& rDesc ) :
        nMergeType(nType),
        rSh(rShell),
        rDescriptor(rDesc),
        bSendAsHTML( sal_True ),
        bSendAsAttachment( sal_False ),
        bPrintAsync( sal_False ),
        bCreateSingleFile( sal_False ),
        pMailMergeConfigItem(0)
        {}

};

struct SwNewDBMgr_Impl;
class SwConnectionDisposedListener_Impl;
class AbstractMailMergeDlg;

class SW_DLLPUBLIC SwNewDBMgr
{
friend class SwConnectionDisposedListener_Impl;

    static SwDbtoolsClient* pDbtoolsClient;

    OUString            sEMailAddrFld;      ///< Mailing: Column name of email address.
    OUString            sSubject;           ///< Mailing: Subject
    OUString            sAttached;          ///< Mailing: Attached Files.
    sal_Bool            bCancel;            ///< Mail merge canceled.
    sal_Bool            bInitDBFields : 1;
    sal_Bool            bSingleJobs : 1;    ///< Printing job when called from Basic.
    sal_Bool            bInMerge    : 1;    ///< merge process active
    sal_Bool            bMergeSilent : 1;   ///< suppress display of dialogs/boxes (used when called over API)
    sal_Bool            bMergeLock : 1;     /**< prevent update of database fields while document is
                                             actually printed at the SwViewShell */
    SwDSParamArr        aDataSourceParams;
    SwNewDBMgr_Impl*    pImpl;
    const SwXMailMerge* pMergeEvtSrc;   ///< != 0 if mail merge events are to be send

    SAL_DLLPRIVATE SwDSParam*          FindDSData(const SwDBData& rData, sal_Bool bCreate);
    SAL_DLLPRIVATE SwDSParam*          FindDSConnection(const OUString& rSource, sal_Bool bCreate);

    SAL_DLLPRIVATE DECL_LINK( PrtCancelHdl, Button * );

    /// Insert data record as text into document.
    SAL_DLLPRIVATE void ImportFromConnection( SwWrtShell* pSh);

    /// Insert a single data record as text into document.
    SAL_DLLPRIVATE void ImportDBEntry(SwWrtShell* pSh);

    /// merge to file _and_ merge to e-Mail
    SAL_DLLPRIVATE sal_Bool          MergeMailFiles(SwWrtShell* pSh,
                                        const SwMergeDescriptor& rMergeDescriptor );
    SAL_DLLPRIVATE sal_Bool          ToNextRecord(SwDSParam* pParam);

public:
    SwNewDBMgr();
    ~SwNewDBMgr();

    /// MailMergeEvent source
    const SwXMailMerge *    GetMailMergeEvtSrc() const  { return pMergeEvtSrc; }
    void SetMailMergeEvtSrc( const SwXMailMerge *pSrc ) { pMergeEvtSrc = pSrc; }

    inline sal_Bool     IsMergeSilent() const           { return bMergeSilent != 0; }
    inline void     SetMergeSilent( sal_Bool bVal )     { bMergeSilent = bVal; }

    /// Merging of data records into fields.
    sal_Bool            MergeNew( const SwMergeDescriptor& rMergeDesc );
    sal_Bool            Merge(SwWrtShell* pSh);
    void                MergeCancel();

    /// Initialize data fields that lack name of database.
    inline sal_Bool     IsInitDBFields() const  { return bInitDBFields; }
    inline void     SetInitDBFields(sal_Bool b) { bInitDBFields = b;    }

    /// Print / Save mail merge one by one or all together.
    inline sal_Bool     IsSingleJobs() const    { return bSingleJobs;   }
    inline void     SetSingleJobs(sal_Bool b)   { bSingleJobs = b;  }

    /// Mailing: Set email data.
    inline void     SetEMailColumn(const OUString& sColName) { sEMailAddrFld = sColName; }
    inline void     SetSubject(const OUString& sSbj) { sSubject = sSbj; }
    inline void     SetAttachment(const OUString& sAtt) { sAttached = sAtt; }

    /// Fill listbox with all table names of a database.
    sal_Bool            GetTableNames(ListBox* pListBox, const OUString& rDBName );

    /// Fill listbox with all column names of a database table.
    sal_Bool            GetColumnNames(ListBox* pListBox,
                            const OUString& rDBName, const OUString& rTableName, sal_Bool bAppend = sal_False);
    sal_Bool            GetColumnNames(ListBox* pListBox,
                            ::css::uno::Reference< ::css::sdbc::XConnection> xConnection,
                            const OUString& rTableName, sal_Bool bAppend = sal_False);

    sal_uLong GetColumnFmt( ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> xSource,
                            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> xConnection,
                            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xColumn,
                            SvNumberFormatter* pNFmtr,
                            long nLanguage );

    sal_uLong GetColumnFmt( const OUString& rDBName,
                            const OUString& rTableName,
                            const OUString& rColNm,
                            SvNumberFormatter* pNFmtr,
                            long nLanguage );
    sal_Int32 GetColumnType( const OUString& rDBName,
                          const OUString& rTableName,
                          const OUString& rColNm );

    inline sal_Bool     IsInMerge() const   { return bInMerge; }
    void            EndMerge();

    void            ExecuteFormLetter(SwWrtShell& rSh,
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rProperties,
                        sal_Bool bWithDataSourceBrowser = sal_False);

    void            InsertText(SwWrtShell& rSh,
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rProperties);

    /// check if a data source is open
    sal_Bool            IsDataSourceOpen(const OUString& rDataSource,
                                    const OUString& rTableOrQuery, sal_Bool bMergeOnly);

    /// open the source while fields are updated - for the calculator only!
    sal_Bool            OpenDataSource(const OUString& rDataSource, const OUString& rTableOrQuery,
                        sal_Int32 nCommandType = -1, bool bCreate = false);
    sal_uInt32      GetSelectedRecordId(const OUString& rDataSource, const OUString& rTableOrQuery, sal_Int32 nCommandType = -1);
    sal_Bool            GetColumnCnt(const OUString& rSourceName, const OUString& rTableName,
                            const OUString& rColumnName, sal_uInt32 nAbsRecordId, long nLanguage,
                            OUString& rResult, double* pNumber);
    /** create and store or find an already stored connection to a data source for use
    in SwFldMgr and SwDBTreeList */
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>
                    RegisterConnection(OUString& rSource);

    const SwDSParam* CreateDSData(const SwDBData& rData)
                        {return FindDSData(rData, sal_True);}
    const SwDSParamArr& GetDSParamArray() const {return aDataSourceParams;}

    /// close all data sources - after fields were updated
    void            CloseAll(sal_Bool bIncludingMerge = sal_True);

    sal_Bool            GetMergeColumnCnt(const OUString& rColumnName, sal_uInt16 nLanguage,
                                OUString &rResult, double *pNumber, sal_uInt32 *pFormat);
    sal_Bool            ToNextMergeRecord();
    sal_Bool            ToNextRecord(const OUString& rDataSource, const OUString& rTableOrQuery, sal_Int32 nCommandType = -1);

    sal_Bool            ExistsNextRecord()const;
    sal_uInt32      GetSelectedRecordId();
    sal_Bool        ToRecordId(sal_Int32 nSet);

    const SwDBData& GetAddressDBName();

    static OUString GetDBField(
                    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xColumnProp,
                    const SwDBFormatData& rDBFormatData,
                    double *pNumber = NULL);

    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>
            GetConnection(const OUString& rDataSource,
                ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>& rxSource);

    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>
            GetColumnSupplier(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>,
                                    const OUString& rTableOrQuery,
                                    sal_uInt8   eTableOrQuery = SW_DB_SELECT_UNKNOWN);

    static ::com::sun::star::uno::Sequence<OUString> GetExistingDatabaseNames();

    /**
     Loads a data source from file and registers it. Returns the registered name.
     */
    static OUString            LoadAndRegisterDataSource();

    static SwDbtoolsClient&    GetDbtoolsClient();
    /// has to be called from _FinitUI()
    static void                RemoveDbtoolsClient();

    /** try to get the data source from the given connection through the XChild interface.
        If this is not possible, the data source will be created through its name.
        @param _xConnection
            The connection which should support the XChild interface. (not a must)
        @param _sDataSourceName
            The data source name will be used to create the data source when the connection can not be used for it.
        @return
            The data source.
    */
    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>
            getDataSourceAsParent(const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection,const OUString& _sDataSourceName);

    /** creates a RowSet, which must be disposed after use.
        @param  _sDataSourceName
            The data source name
        @param  _sCommand
            The command.
        @param  _nCommandType
            The type of the command.
        @param  _xConnection
            The active connection which may be <NULL/>.
        @return
            The new created RowSet.

    */
    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>
            createCursor(   const OUString& _sDataSourceName,
                            const OUString& _sCommand,
                            sal_Int32 _nCommandType,
                            const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>& _xConnection
                            );
    //merge into one document - returns the number of merged docs
    sal_Int32 MergeDocuments( SwMailMergeConfigItem& rMMConfig, SwView& rSourceView );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
