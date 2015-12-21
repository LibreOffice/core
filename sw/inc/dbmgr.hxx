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
#include <com/sun/star/frame/XStorable.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/sdbc/XRowSet.hpp>
#include <com/sun/star/sdbcx/XColumnsSupplier.hpp>
#include <unotools/tempfile.hxx>
#include <sfx2/objsh.hxx>
#include <dbui.hxx>

#include <memory>
#include <vector>

namespace com{namespace sun{namespace star{
    namespace sdbc{
        class XConnection;
        class XStatement;
        class XDataSource;
        class XResultSet;
        class xRowSet;
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
    css::util::Date aNullDate;
    css::uno::Reference< css::util::XNumberFormatter> xFormatter;
    css::lang::Locale aLocale;
};

namespace vcl {
    class Window;
}

class SwView;
class SwWrtShell;
class SfxProgress;
class ListBox;
class Button;
class SvNumberFormatter;
class SwXMailMerge;
class SwMailMergeConfigItem;
class SwCalc;
class INetURLObject;
class SwDocShell;
class SwDoc;
class SfxFilter;

enum DBManagerOptions
{
    DBMGR_MERGE,             ///< Data records in fields.
    DBMGR_MERGE_PRINTER,     ///< Print mail merge.
    DBMGR_MERGE_EMAIL,       ///< Send mail merge as email.
    DBMGR_MERGE_FILE,        ///< Save mail merge as files.
    DBMGR_MERGE_SHELL        ///< Create merge doc and keep the doc shell.
};

// Administration of (new) logical databases.
enum class SwDBSelect
{
    UNKNOWN, TABLE, QUERY
};

struct SwDSParam : public SwDBData
{
    css::util::Date  aNullDate;

    css::uno::Reference<css::util::XNumberFormatter>    xFormatter;
    css::uno::Reference< css::sdbc::XConnection>       xConnection;
    css::uno::Reference< css::sdbc::XStatement>        xStatement;
    css::uno::Reference< css::sdbc::XResultSet>        xResultSet;
    css::uno::Sequence<  css::uno::Any >               aSelection;
    bool bScrollable;
    bool bEndOfDB;
    bool bAfterSelection;
    long nSelectionIndex;

    SwDSParam(const SwDBData& rData) :
        SwDBData(rData),
        bScrollable(false),
        bEndOfDB(false),
        bAfterSelection(false),
        nSelectionIndex(0)
        {}

    SwDSParam(const SwDBData& rData,
        const css::uno::Reference< css::sdbc::XResultSet>&    xResSet,
        const css::uno::Sequence<  css::uno::Any >&   rSelection) :
        SwDBData(rData),
        xResultSet(xResSet),
        aSelection(rSelection),
        bScrollable(true),
        bEndOfDB(false),
        bAfterSelection(false),
        nSelectionIndex(0)
        {}

        void CheckEndOfDB()
        {
            if(bEndOfDB)
                bAfterSelection = true;
        }
};
typedef std::vector<std::unique_ptr<SwDSParam>> SwDSParams_t;

struct SwMergeDescriptor
{
    DBManagerOptions                                    nMergeType;
    SwWrtShell&                                         rSh;
    const svx::ODataAccessDescriptor&                 rDescriptor;
    OUString                                            sSaveToFilter; ///< export filter to save resulting files
    OUString                                            sSaveToFilterOptions;
    css::uno::Sequence< css::beans::PropertyValue >     aSaveToFilterData;

    OUString                                            sSubject;
    OUString                                            sAddressFromColumn;
    OUString                                            sMailBody;
    OUString                                            sAttachmentName;
    css::uno::Sequence< OUString >                      aCopiesTo;
    css::uno::Sequence< OUString >                      aBlindCopiesTo;

    css::uno::Reference< css::mail::XSmtpService >      xSmtpServer;

    bool                                            bSendAsHTML;
    bool                                            bSendAsAttachment;

    bool                                            bPrintAsync;
    bool                                            bCreateSingleFile;
    bool                                            bSubjectIsFilename;

    SwMailMergeConfigItem*                              pMailMergeConfigItem;

    css::uno::Sequence<  css::beans::PropertyValue >  aPrintOptions;

    SwMergeDescriptor( DBManagerOptions nType, SwWrtShell& rShell, svx::ODataAccessDescriptor& rDesc ) :
        nMergeType(nType),
        rSh(rShell),
        rDescriptor(rDesc),
        bSendAsHTML( true ),
        bSendAsAttachment( false ),
        bPrintAsync( false ),
        bCreateSingleFile( false ),
        bSubjectIsFilename( false ),
        pMailMergeConfigItem(nullptr)
        {}

};

struct SwDBManager_Impl;
class SwConnectionDisposedListener_Impl;
class AbstractMailMergeDlg;
class SwDoc;

class SW_DLLPUBLIC SwDBManager
{
friend class SwConnectionDisposedListener_Impl;

    OUString            sEMailAddrField;      ///< Mailing: Column name of email address.
    OUString            sSubject;           ///< Mailing: Subject
    bool            bCancel;            ///< Mail merge canceled.
    bool            bInitDBFields : 1;
    bool            bInMerge    : 1;    ///< merge process active
    bool            bMergeSilent : 1;   ///< suppress display of dialogs/boxes (used when called over API)
    bool            bMergeLock : 1;     /**< prevent update of database fields while document is
                                             actually printed at the SwViewShell */
    SwDSParams_t    m_DataSourceParams;
    std::unique_ptr<SwDBManager_Impl>  pImpl;
    const SwXMailMerge* pMergeEvtSrc;   ///< != 0 if mail merge events are to be send
    /// Name of the embedded database that's included in the current document.
    OUString     m_sEmbeddedName;

    /// The document that owns this manager.
    SwDoc* m_pDoc;

    SAL_DLLPRIVATE SwDSParam*          FindDSData(const SwDBData& rData, bool bCreate);
    SAL_DLLPRIVATE SwDSParam*          FindDSConnection(const OUString& rSource, bool bCreate);

    DECL_DLLPRIVATE_LINK_TYPED( PrtCancelHdl, Button *, void );

    /// Insert data record as text into document.
    SAL_DLLPRIVATE void ImportFromConnection( SwWrtShell* pSh);

    /// Insert a single data record as text into document.
    SAL_DLLPRIVATE void ImportDBEntry(SwWrtShell* pSh);

    /// merge to file _and_ merge to e-Mail
    SAL_DLLPRIVATE bool          MergeMailFiles(SwWrtShell* pSh,
                                        const SwMergeDescriptor& rMergeDescriptor, vcl::Window* pParent );
    SAL_DLLPRIVATE bool          ToNextRecord(SwDSParam* pParam, bool bReset);

    static css::uno::Reference< css::sdbc::XRowSet>
        GetRowSet(css::uno::Reference< css::sdbc::XConnection>,
        const OUString& rTableOrQuery, SwDBSelect   eTableOrQuery);

    SAL_DLLPRIVATE static css::uno::Reference< css::beans::XPropertySet>
        GetRowSupplier(css::uno::Reference< css::sdbc::XConnection> xConnection,
        const OUString& rTableOrQuery,  SwDBSelect   eTableOrQuery)
    {
        css::uno::Reference<css::sdbc::XRowSet> xRowSet = GetRowSet(xConnection, rTableOrQuery, eTableOrQuery);

        return css::uno::Reference<css::beans::XPropertySet>( xRowSet, css::uno::UNO_QUERY );
    }

    SAL_DLLPRIVATE void CreateDumpDocs(sal_Int32 &nMaxDumpDocs);

    SAL_DLLPRIVATE void SetSourceProp(SwDocShell* pSourceDocSh);

    SAL_DLLPRIVATE void GetPathAddress(OUString &sPath, OUString &sAddress,
                                      css::uno::Reference< css::beans::XPropertySet > xColumnProp);

    SAL_DLLPRIVATE bool CreateNewTemp(OUString &sPath, const OUString &sAddress,
                                      std::unique_ptr< utl::TempFile > &aTempFile,
                                      const SwMergeDescriptor& rMergeDescriptor,  const SfxFilter* pStoreToFilter);

    SAL_DLLPRIVATE void UpdateProgressDlg(bool bMergeShell,  VclPtr<CancelableDialog> pProgressDlg, bool createTempFile,
                                          std::unique_ptr< INetURLObject > &aTempFileURL,
                                          SwDocShell *pSourceDocSh, sal_Int32 nDocNo);

    SAL_DLLPRIVATE bool CreateTargetDocShell(sal_Int32 nMaxDumpDocs, bool bMergeShell, vcl::Window *pSourceWindow,
                                             SwWrtShell *pSourceShell, SwDocShell *pSourceDocSh,
                                             SfxObjectShellRef &xTargetDocShell, SwDoc *&pTargetDoc,
                                             SwWrtShell *&pTargetShell, SwView  *&pTargetView,
                                             sal_uInt16 &nStartingPageNo, OUString &sStartingPageDesc);

    SAL_DLLPRIVATE void LockUnlockDisp(bool bLock, SwDocShell *pSourceDocSh);

    SAL_DLLPRIVATE void CreateProgessDlg(vcl::Window *&pSourceWindow, VclPtr<CancelableDialog> &pProgressDlg,
                                         bool bMergeShell, SwWrtShell *pSourceShell, vcl::Window *pParent);

    SAL_DLLPRIVATE void CreateWorkDoc(SfxObjectShellLock &xWorkDocSh, SwView *&pWorkView, SwDoc *&pWorkDoc, SwDBManager *&pOldDBManager,
                                      SwDocShell *pSourceDocSh, sal_Int32 nMaxDumpDocs,  sal_Int32 nDocNo);

    SAL_DLLPRIVATE void UpdateExpFields(SwWrtShell& rWorkShell, SfxObjectShellLock xWorkDocSh);

    SAL_DLLPRIVATE void CreateStoreToFilter(const SfxFilter *&pStoreToFilter, const OUString *&pStoreToFilterOptions,
                                            SwDocShell *pSourceDocSh, bool bEMail, const SwMergeDescriptor &rMergeDescriptor);

    SAL_DLLPRIVATE void MergeSingleFiles(SwDoc *pWorkDoc, SwWrtShell &rWorkShell, SwWrtShell *pTargetShell, SwDoc *pTargetDoc,
                                         SfxObjectShellLock &xWorkDocSh, SfxObjectShellRef xTargetDocShell,
                                         bool bPageStylesWithHeaderFooter, bool bSynchronizedDoc,
                                         OUString &sModifiedStartingPageDesc, OUString &sStartingPageDesc, sal_Int32 nDocNo,
                                         long nStartRow, sal_uInt16 nStartingPageNo, int &targetDocPageCount, const bool bMergeShell,
                                         const SwMergeDescriptor& rMergeDescriptor, sal_Int32 nMaxDumpDocs);

    SAL_DLLPRIVATE void ResetWorkDoc(SwDoc *pWorkDoc, SfxObjectShellLock &xWorkDocSh, SwDBManager *pOldDBManager);

    SAL_DLLPRIVATE void FreezeLayouts(SwWrtShell *pTargetShell, bool freeze);

    SAL_DLLPRIVATE void FinishMailMergeFile(SfxObjectShellLock &xWorkDocSh, SwView *pWorkView, SwDoc *pTargetDoc,
                                             SwWrtShell *pTargetShell, bool bCreateSingleFile, bool bPrinter,
                                             SwDoc *pWorkDoc, SwDBManager *pOldDBManager);

    SAL_DLLPRIVATE bool SavePrintDoc(SfxObjectShellRef xTargetDocShell, SwView *pTargetView,
                                     const SwMergeDescriptor &rMergeDescriptor,
                                     std::unique_ptr< utl::TempFile > &aTempFile,
                                     const SfxFilter *&pStoreToFilter, const OUString *&pStoreToFilterOptions,
                                     const bool bMergeShell, bool bCreateSingleFile, const bool bPrinter);

    SAL_DLLPRIVATE void SetPrinterOptions(const SwMergeDescriptor &rMergeDescriptor,
                                          css::uno::Sequence< css::beans::PropertyValue > &aOptions);

    SAL_DLLPRIVATE void RemoveTmpFiles(::std::vector< OUString> &aFilesToRemove);

    SwDBManager(SwDBManager const&) = delete;
    SwDBManager& operator=(SwDBManager const&) = delete;

public:
    SwDBManager(SwDoc* pDoc);
    ~SwDBManager();

    enum DBConnURITypes {
        DBCONN_UNKNOWN = 0,
        DBCONN_ODB,
        DBCONN_CALC,
        DBCONN_DBASE,
        DBCONN_FLAT,
        DBCONN_MSJET,
        DBCONN_MSACE
    };

    /// MailMergeEvent source
    const SwXMailMerge *    GetMailMergeEvtSrc() const  { return pMergeEvtSrc; }
    void SetMailMergeEvtSrc( const SwXMailMerge *pSrc ) { pMergeEvtSrc = pSrc; }

    inline bool     IsMergeSilent() const           { return bMergeSilent; }
    inline void     SetMergeSilent( bool bVal )     { bMergeSilent = bVal; }

    /// Merging of data records into fields.
    bool            MergeNew( const SwMergeDescriptor& rMergeDesc, vcl::Window* pParent = nullptr );
    static bool     Merge(SwWrtShell* pSh);
    void            MergeCancel();

    /// Initialize data fields that lack name of database.
    inline bool     IsInitDBFields() const  { return bInitDBFields; }
    inline void     SetInitDBFields(bool b) { bInitDBFields = b;    }

    /// Mailing: Set email data.
    inline void     SetEMailColumn(const OUString& sColName) { sEMailAddrField = sColName; }
    inline void     SetSubject(const OUString& sSbj) { sSubject = sSbj; }

    /// Fill listbox with all table names of a database.
    bool            GetTableNames(ListBox* pListBox, const OUString& rDBName );

    /// Fill listbox with all column names of a database table.
    void            GetColumnNames(ListBox* pListBox,
                            const OUString& rDBName, const OUString& rTableName, bool bAppend = false);
    static void GetColumnNames(ListBox* pListBox,
                            css::uno::Reference< css::sdbc::XConnection> xConnection,
                            const OUString& rTableName, bool bAppend = false);

    static sal_uLong GetColumnFormat( css::uno::Reference< css::sdbc::XDataSource> xSource,
                            css::uno::Reference< css::sdbc::XConnection> xConnection,
                            css::uno::Reference< css::beans::XPropertySet> xColumn,
                            SvNumberFormatter* pNFormatr,
                            long nLanguage );

    sal_Int32   GetRowCount(const OUString& rDBName, const OUString& rTableName);

    static sal_Int32 GetRowCount(css::uno::Reference< css::sdbc::XConnection> xConnection,
                                 const OUString& rTableName);

    sal_Int32 GetRowCount() const;

    sal_uLong GetColumnFormat( const OUString& rDBName,
                            const OUString& rTableName,
                            const OUString& rColNm,
                            SvNumberFormatter* pNFormatr,
                            long nLanguage );
    sal_Int32 GetColumnType( const OUString& rDBName,
                          const OUString& rTableName,
                          const OUString& rColNm );

    inline bool     IsInMerge() const   { return bInMerge; }

    void            ExecuteFormLetter(SwWrtShell& rSh,
                        const css::uno::Sequence< css::beans::PropertyValue>& rProperties,
                        bool bWithDataSourceBrowser = false);

    static void     InsertText(SwWrtShell& rSh,
                        const css::uno::Sequence< css::beans::PropertyValue>& rProperties);

    /// check if a data source is open
    bool            IsDataSourceOpen(const OUString& rDataSource,
                                    const OUString& rTableOrQuery, bool bMergeShell);

    /// open the source while fields are updated - for the calculator only!
    bool            OpenDataSource(const OUString& rDataSource, const OUString& rTableOrQuery,
                        sal_Int32 nCommandType = -1, bool bCreate = false);
    sal_uInt32      GetSelectedRecordId(const OUString& rDataSource, const OUString& rTableOrQuery, sal_Int32 nCommandType = -1);
    bool            GetColumnCnt(const OUString& rSourceName, const OUString& rTableName,
                            const OUString& rColumnName, sal_uInt32 nAbsRecordId, long nLanguage,
                            OUString& rResult, double* pNumber);
    /** create and store or find an already stored connection to a data source for use
    in SwFieldMgr and SwDBTreeList */
    css::uno::Reference< css::sdbc::XConnection>
                    RegisterConnection(OUString const& rSource);

    const SwDSParam* CreateDSData(const SwDBData& rData)
                        {return FindDSData(rData, true);}
    const SwDSParams_t& GetDSParamArray() const { return m_DataSourceParams; }

    /// close all data sources - after fields were updated
    void            CloseAll(bool bIncludingMerge = true);

    bool            GetMergeColumnCnt(const OUString& rColumnName, sal_uInt16 nLanguage,
                                      OUString &rResult, double *pNumber);
    bool            FillCalcWithMergeData(SvNumberFormatter *pDocFormatter,
                                          sal_uInt16 nLanguage, bool asString, SwCalc &aCalc);
    bool            ToNextMergeRecord();
    bool            ToNextRecord(const OUString& rDataSource, const OUString& rTableOrQuery, sal_Int32 nCommandType = -1);

    bool            ExistsNextRecord()const;
    sal_uInt32      GetSelectedRecordId();
    bool            ToRecordId(sal_Int32 nSet);

    static const SwDBData& GetAddressDBName();

    static OUString GetDBField(
                    css::uno::Reference< css::beans::XPropertySet > xColumnProp,
                    const SwDBFormatData& rDBFormatData,
                    double *pNumber = nullptr);

    static css::uno::Reference< css::sdbc::XConnection>
            GetConnection(const OUString& rDataSource,
                css::uno::Reference< css::sdbc::XDataSource>& rxSource);

    static css::uno::Reference< css::sdbcx::XColumnsSupplier>
            GetColumnSupplier(css::uno::Reference< css::sdbc::XConnection> xConnection,
                                    const OUString& rTableOrQuery,
                                    SwDBSelect eTableOrQuery = SwDBSelect::UNKNOWN)
    {
        css::uno::Reference<css::sdbc::XRowSet> xRowSet = GetRowSet(xConnection, rTableOrQuery, eTableOrQuery);

        return css::uno::Reference<css::sdbcx::XColumnsSupplier>( xRowSet, css::uno::UNO_QUERY );
    }

    static css::uno::Sequence<OUString> GetExistingDatabaseNames();

    static DBConnURITypes GetDBunoURI(const OUString &rURI, css::uno::Any &aURLAny);

    /**
     Loads a data source from file and registers it.

     This function requires GUI interaction, as it loads the data source from
     the filename returned by a file picker and additional settings dialog.
     In case of success it returns the registered name, otherwise an empty string.
     */
    static OUString            LoadAndRegisterDataSource(SwDocShell* pDocShell = nullptr);

    /**
     Loads a data source from file and registers it.

     In case of success it returns the registered name, otherwise an empty string.
     Optionally add a prefix to the registered DB name.
     */
    static OUString            LoadAndRegisterDataSource(const DBConnURITypes type, const css::uno::Any &rUnoURI,
                                                         const css::uno::Reference < css::beans::XPropertySet > *pSettings,
                                                         const OUString &rURI, const OUString *pPrefix = nullptr, const OUString *pDestDir = nullptr,
                                                         SwDocShell* pDocShell = nullptr);
    /**
     Loads a data source from file and registers it.

     Convenience function, which calls GetDBunoURI and has just one mandatory parameter.
     In case of success it returns the registered name, otherwise an empty string.
     */
    static OUString            LoadAndRegisterDataSource(const OUString& rURI, const OUString *pPrefix = nullptr, const OUString *pDestDir = nullptr,
                                                         const css::uno::Reference < css::beans::XPropertySet > *pSettings = nullptr);

    /// Load the embedded data source of the document and also register it.
    void LoadAndRegisterEmbeddedDataSource(const SwDBData& rData, const SwDocShell& rDocShell);

    /// Unregister a data source.
    static void RevokeDataSource(const OUString& rName);

    /** try to get the data source from the given connection through the XChild interface.
        If this is not possible, the data source will be created through its name.
        @param _xConnection
            The connection which should support the XChild interface. (not a must)
        @param _sDataSourceName
            The data source name will be used to create the data source when the connection can not be used for it.
        @return
            The data source.
    */
    static css::uno::Reference< css::sdbc::XDataSource>
            getDataSourceAsParent(const css::uno::Reference< css::sdbc::XConnection>& _xConnection,const OUString& _sDataSourceName);

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
    static css::uno::Reference< css::sdbc::XResultSet>
            createCursor(   const OUString& _sDataSourceName,
                            const OUString& _sCommand,
                            sal_Int32 _nCommandType,
                            const css::uno::Reference< css::sdbc::XConnection>& _xConnection
                            );

    void setEmbeddedName(const OUString& rEmbeddedName, SwDocShell& rDocShell);
    OUString getEmbeddedName() const;

    static void StoreEmbeddedDataSource(const css::uno::Reference<css::frame::XStorable>& xStorable,
                                        const css::uno::Reference<css::embed::XStorage>& xStorage,
                                        const OUString& rStreamRelPath,
                                        const OUString& rOwnURL);

    SwDoc* getDoc() const;
    /// Stop reacting to removed database registrations.
    void releaseRevokeListener();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
