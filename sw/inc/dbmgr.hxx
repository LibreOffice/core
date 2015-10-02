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

#include <memory>
#include <vector>

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
    com::sun::star::util::Date  aNullDate;

    ::com::sun::star::uno::Reference<com::sun::star::util::XNumberFormatter>    xFormatter;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>      xConnection;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XStatement>       xStatement;
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>       xResultSet;
    ::com::sun::star::uno::Sequence<  ::com::sun::star::uno::Any >              aSelection;
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
        const ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XResultSet>&    xResSet,
        const ::com::sun::star::uno::Sequence<  ::com::sun::star::uno::Any >&   rSelection) :
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

    ::com::sun::star::uno::Sequence<  ::com::sun::star::beans::PropertyValue >  aPrintOptions;

    SwMergeDescriptor( DBManagerOptions nType, SwWrtShell& rShell, svx::ODataAccessDescriptor& rDesc ) :
        nMergeType(nType),
        rSh(rShell),
        rDescriptor(rDesc),
        bSendAsHTML( true ),
        bSendAsAttachment( false ),
        bPrintAsync( false ),
        bCreateSingleFile( false ),
        bSubjectIsFilename( false ),
        pMailMergeConfigItem(0)
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
    SwDBManager_Impl*    pImpl;
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
    SAL_DLLPRIVATE bool          ToNextRecord(SwDSParam* pParam);

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
    bool            MergeNew( const SwMergeDescriptor& rMergeDesc, vcl::Window* pParent = NULL );
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

    static sal_uLong GetColumnFormat( ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource> xSource,
                            ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection> xConnection,
                            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet> xColumn,
                            SvNumberFormatter* pNFormatr,
                            long nLanguage );

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
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rProperties,
                        bool bWithDataSourceBrowser = false);

    static void     InsertText(SwWrtShell& rSh,
                        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue>& rProperties);

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
    ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>
                    RegisterConnection(OUString& rSource);

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
                    ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > xColumnProp,
                    const SwDBFormatData& rDBFormatData,
                    double *pNumber = NULL);

    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>
            GetConnection(const OUString& rDataSource,
                ::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XDataSource>& rxSource);

    static ::com::sun::star::uno::Reference< ::com::sun::star::sdbcx::XColumnsSupplier>
            GetColumnSupplier(::com::sun::star::uno::Reference< ::com::sun::star::sdbc::XConnection>,
                                    const OUString& rTableOrQuery,
                                    SwDBSelect eTableOrQuery = SwDBSelect::UNKNOWN);

    static ::com::sun::star::uno::Sequence<OUString> GetExistingDatabaseNames();

    static DBConnURITypes GetDBunoURI(const OUString &rURI, ::com::sun::star::uno::Any &aURLAny);

    /**
     Loads a data source from file and registers it.

     This function requires GUI interaction, as it loads the data source from
     the filename returned by a file picker and additional settings dialog.
     In case of success it returns the registered name, otherwise an empty string.
     */
    static OUString            LoadAndRegisterDataSource(SwDocShell* pDocShell = 0);

    /**
     Loads a data source from file and registers it.

     In case of success it returns the registered name, otherwise an empty string.
     Optionally add a prefix to the registered DB name.
     */
    static OUString            LoadAndRegisterDataSource(const DBConnURITypes type, const ::com::sun::star::uno::Any &rUnoURI,
                                                         const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > *pSettings,
                                                         const OUString &rURI, const OUString *pPrefix = 0, const OUString *pDestDir = 0,
                                                         SwDocShell* pDocShell = 0);
    /**
     Loads a data source from file and registers it.

     Convenience function, which calls GetDBunoURI and has just one mandatory parameter.
     In case of success it returns the registered name, otherwise an empty string.
     */
    static OUString            LoadAndRegisterDataSource(const OUString& rURI, const OUString *pPrefix = 0, const OUString *pDestDir = 0,
                                                         const ::com::sun::star::uno::Reference < ::com::sun::star::beans::XPropertySet > *pSettings = 0);

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
