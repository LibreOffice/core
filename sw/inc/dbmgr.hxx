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
#include <tools/solar.h>
#include <i18nlangtag/lang.h>
#include <com/sun/star/util/Date.hpp>
#include "swdllapi.h"
#include "swdbdata.hxx"
#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/Locale.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

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
    namespace embed { class XStorage; }
    namespace frame { class XStorable; }
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

namespace weld {
    class ComboBox;
    class Window;
}

class SwView;
class SwWrtShell;
class ListBox;
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
    css::util::Date  aNullDate;

    css::uno::Reference<css::util::XNumberFormatter>    xFormatter;
    css::uno::Reference< css::sdbc::XConnection>       xConnection;
    css::uno::Reference< css::sdbc::XStatement>        xStatement;
    css::uno::Reference< css::sdbc::XResultSet>        xResultSet;
    css::uno::Sequence<  css::uno::Any >               aSelection;
    bool bScrollable;
    bool bEndOfDB;
    long nSelectionIndex;

    SwDSParam(const SwDBData& rData) :
        SwDBData(rData),
        bScrollable(false),
        bEndOfDB(false),
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
        nSelectionIndex(0)
        {}

    bool HasValidRecord() const
        { return( !bEndOfDB && xResultSet.is() ); }
};

typedef std::vector<std::unique_ptr<SwDSParam>> SwDSParams_t;

struct SwMergeDescriptor
{
    const DBManagerOptions                              nMergeType;
    SwWrtShell&                                         rSh;
    const svx::ODataAccessDescriptor&                   rDescriptor;

    /**
     * Create a single or multiple results
     *
     * This currently just affects FILE and PRINTER, as EMAIL is always
     * multiple and SHELL is always single.
     */
    bool                                                bCreateSingleFile;

    /**
     * @defgroup save Export filter settings
     * @addtogroup save
     * @{ */
    OUString                                            sSaveToFilter;
    OUString                                            sSaveToFilterOptions;
    css::uno::Sequence< css::beans::PropertyValue >     aSaveToFilterData;
    /** @} */

    /**
     * @defgroup file Mail merge as File settings
     * @addtogroup file
     * @{ */

    /**
     * Basename incl. the path for the generated files.
     *
     * The final filename will be created by concatenating a number to prevent
     * overwriting an existing file and the extension based on the filter
     * settings.
     */
    OUString                                            sPrefix;
    /**
     * Use the sPrefix as the target filename also overwriting an existing
     * target file.
     *
     * Just used for the internal mail merge dialogs as mail merge never
     * overwrites existing files (see SwDBManager::ExecuteFormLetter).
     */
    bool                                                bPrefixIsFilename;
    /** @} */

    /**
     * @defgroup email Mail merge as eMail settings
     * @addtogroup email
     * @{ */
    OUString                                            sSubject;
    OUString                                            sMailBody;
    OUString                                            sAttachmentName;
    css::uno::Sequence< OUString >                      aCopiesTo;
    css::uno::Sequence< OUString >                      aBlindCopiesTo;
    css::uno::Reference< css::mail::XSmtpService >      xSmtpServer;
    bool                                                bSendAsHTML;
    bool                                                bSendAsAttachment;
    /** @} */

    /**
     * @addtogroup file email
     * @{ */

    /** DB column to fetch EMail of Filename from
     */
    OUString                                            sDBcolumn;

    /** @} */

    /**
     * @defgroup print Mail merge to Printer
     * @addtogroup print
     * @{ */
    css::uno::Sequence<  css::beans::PropertyValue >    aPrintOptions;
    /** @} */

    SwMailMergeConfigItem*                              pMailMergeConfigItem;

    SwMergeDescriptor( const DBManagerOptions nType,
                       SwWrtShell& rShell,
                       const svx::ODataAccessDescriptor& rDesc ) :
        nMergeType(nType),
        rSh(rShell),
        rDescriptor(rDesc),
        bCreateSingleFile( false ),
        bPrefixIsFilename( false ),
        bSendAsHTML( true ),
        bSendAsAttachment( false ),
        pMailMergeConfigItem( nullptr )
    {
        if( nType == DBMGR_MERGE_SHELL || nType == DBMGR_MERGE_PRINTER )
            bCreateSingleFile = true;
    }
};

class SwDoc;

class SW_DLLPUBLIC SwDBManager
{
    struct SwDBManager_Impl;
    class ConnectionDisposedListener_Impl;
    class MailDispatcherListener_Impl;

    enum class MergeStatus
    {
        Ok = 0, Cancel, Error
    };

    MergeStatus     m_aMergeStatus;     ///< current / last merge status
    bool            m_bInitDBFields : 1;
    bool            m_bInMerge    : 1;    ///< merge process active
    bool            m_bMergeSilent : 1;   ///< suppress display of dialogs/boxes (used when called over API)
    SwDSParams_t    m_DataSourceParams;
    std::unique_ptr<SwDBManager_Impl>  m_pImpl;
    const SwXMailMerge* m_pMergeEvtSrc;   ///< != 0 if mail merge events are to be send
    /// Name of the embedded database that's included in the current document.
    OUString     m_sEmbeddedName;

    /// Store last registrations to revoke or commit
    static std::vector<std::pair<SwDocShell*, OUString>> m_aUncommittedRegistrations;

    /// Not used connections.
    std::vector<OUString> m_aNotUsedConnections;

    /// Set connection as used.
    void SetAsUsed(const OUString& rName);

    /// The document that owns this manager.
    SwDoc* m_pDoc;

    SAL_DLLPRIVATE SwDSParam*          FindDSData(const SwDBData& rData, bool bCreate);
    SAL_DLLPRIVATE SwDSParam*          FindDSConnection(const OUString& rSource, bool bCreate);

    /// Insert data record as text into document.
    SAL_DLLPRIVATE void ImportFromConnection( SwWrtShell* pSh);

    /// Insert a single data record as text into document.
    SAL_DLLPRIVATE void ImportDBEntry(SwWrtShell* pSh);

    /// Run the mail merge for defined modes, except DBMGR_MERGE
    SAL_DLLPRIVATE bool MergeMailFiles( SwWrtShell* pSh,
                                        const SwMergeDescriptor& rMergeDescriptor );

    SAL_DLLPRIVATE bool ToNextMergeRecord();
    SAL_DLLPRIVATE bool IsValidMergeRecord() const;

    SwDBManager(SwDBManager const&) = delete;
    SwDBManager& operator=(SwDBManager const&) = delete;

public:
    SwDBManager(SwDoc* pDoc);
    ~SwDBManager() COVERITY_NOEXCEPT_FALSE;

    /// MailMergeEvent source
    const SwXMailMerge *    GetMailMergeEvtSrc() const  { return m_pMergeEvtSrc; }
    void SetMailMergeEvtSrc( const SwXMailMerge *pSrc ) { m_pMergeEvtSrc = pSrc; }

    bool     IsMergeSilent() const           { return m_bMergeSilent; }
    void     SetMergeSilent( bool bVal )     { m_bMergeSilent = bVal; }

    /// Merging of data records into fields.
    bool            Merge( const SwMergeDescriptor& rMergeDesc );
    void            MergeCancel();

    bool     IsMergeOk()     { return MergeStatus::Ok     == m_aMergeStatus; };
    bool     IsMergeError()  { return MergeStatus::Error  <= m_aMergeStatus; };

    static std::shared_ptr<SwMailMergeConfigItem> PerformMailMerge(SwView const * pView);

    /// Initialize data fields that lack name of database.
    bool     IsInitDBFields() const  { return m_bInitDBFields; }
    void     SetInitDBFields(bool b) { m_bInitDBFields = b;    }

    /// Fill listbox with all table names of a database.
    bool            GetTableNames(weld::ComboBox& rBox, const OUString& rDBName);

    /// Fill listbox with all column names of a database table.
    void            GetColumnNames(ListBox* pListBox,
                            const OUString& rDBName, const OUString& rTableName);
    void            GetColumnNames(weld::ComboBox& rBox,
                            const OUString& rDBName, const OUString& rTableName);
    static void GetColumnNames(ListBox* pListBox,
                            css::uno::Reference< css::sdbc::XConnection> const & xConnection,
                            const OUString& rTableName);
    static void GetColumnNames(weld::ComboBox& rBox,
                            css::uno::Reference< css::sdbc::XConnection> const & xConnection,
                            const OUString& rTableName);

    static sal_uLong GetColumnFormat( css::uno::Reference< css::sdbc::XDataSource> const & xSource,
                            css::uno::Reference< css::sdbc::XConnection> const & xConnection,
                            css::uno::Reference< css::beans::XPropertySet> const & xColumn,
                            SvNumberFormatter* pNFormatr,
                            LanguageType nLanguage );

    sal_uLong GetColumnFormat( const OUString& rDBName,
                            const OUString& rTableName,
                            const OUString& rColNm,
                            SvNumberFormatter* pNFormatr,
                            LanguageType nLanguage );
    sal_Int32 GetColumnType( const OUString& rDBName,
                          const OUString& rTableName,
                          const OUString& rColNm );

    bool     IsInMerge() const   { return m_bInMerge; }

    void            ExecuteFormLetter(SwWrtShell& rSh,
                        const css::uno::Sequence< css::beans::PropertyValue>& rProperties);

    static void     InsertText(SwWrtShell& rSh,
                        const css::uno::Sequence< css::beans::PropertyValue>& rProperties);

    /// check if a data source is open
    bool            IsDataSourceOpen(const OUString& rDataSource,
                                    const OUString& rTableOrQuery, bool bMergeShell);

    /// open the source while fields are updated - for the calculator only!
    bool            OpenDataSource(const OUString& rDataSource, const OUString& rTableOrQuery);
    sal_uInt32      GetSelectedRecordId(const OUString& rDataSource, const OUString& rTableOrQuery, sal_Int32 nCommandType = -1);
    bool            GetColumnCnt(const OUString& rSourceName, const OUString& rTableName,
                            const OUString& rColumnName, sal_uInt32 nAbsRecordId, LanguageType nLanguage,
                            OUString& rResult, double* pNumber);
    /** create and store or find an already stored connection to a data source for use
    in SwFieldMgr and SwDBTreeList */
    css::uno::Reference< css::sdbc::XConnection> const &
                    RegisterConnection(OUString const& rSource);

    void            CreateDSData(const SwDBData& rData)
                        { FindDSData(rData, true); }
    const SwDSParams_t& GetDSParamArray() const { return m_DataSourceParams; }

    /// close all data sources - after fields were updated
    void            CloseAll(bool bIncludingMerge = true);

    bool            GetMergeColumnCnt(const OUString& rColumnName, LanguageType nLanguage,
                                      OUString &rResult, double *pNumber);
    bool            FillCalcWithMergeData(SvNumberFormatter *pDocFormatter,
                                          LanguageType nLanguage, SwCalc &aCalc);
    void            ToNextRecord(const OUString& rDataSource, const OUString& rTableOrQuery);

    sal_uInt32      GetSelectedRecordId();
    bool            ToRecordId(sal_Int32 nSet);

    static const SwDBData& GetAddressDBName();

    static OUString GetDBField(
                    css::uno::Reference< css::beans::XPropertySet > const & xColumnProp,
                    const SwDBFormatData& rDBFormatData,
                    double *pNumber = nullptr);

    static css::uno::Reference< css::sdbc::XConnection>
            GetConnection(const OUString& rDataSource,
                css::uno::Reference< css::sdbc::XDataSource>& rxSource,
                const SwView* pView);

    static css::uno::Reference< css::sdbcx::XColumnsSupplier>
            GetColumnSupplier(css::uno::Reference< css::sdbc::XConnection> const & xConnection,
                                    const OUString& rTableOrQuery,
                                    SwDBSelect eTableOrQuery = SwDBSelect::UNKNOWN);

    static css::uno::Sequence<OUString> GetExistingDatabaseNames();

    /**
     Loads a data source from file and registers it.

     This function requires GUI interaction, as it loads the data source from
     the filename returned by a file picker and additional settings dialog.
     In case of success it returns the registered name, otherwise an empty string.
     */
    static OUString            LoadAndRegisterDataSource(weld::Window* pParent, SwDocShell* pDocShell = nullptr);

    /**
     Loads a data source from file and registers it.

     Convenience function, which calls GetDBunoURI and has just one mandatory parameter.
     In case of success it returns the registered name, otherwise an empty string.
     */
    static OUString            LoadAndRegisterDataSource(const OUString& rURI, const OUString *pDestDir);

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
            createCursor(const OUString& _sDataSourceName,
                         const OUString& _sCommand,
                         sal_Int32 _nCommandType,
                         const css::uno::Reference< css::sdbc::XConnection>& _xConnection,
                         const SwView* pView);

    void setEmbeddedName(const OUString& rEmbeddedName, SwDocShell& rDocShell);
    const OUString& getEmbeddedName() const;

    // rOwnURL should be taken using INetURLObject::GetMainURL(INetURLObject::DecodeMechanism::NONE)
    static void StoreEmbeddedDataSource(const css::uno::Reference<css::frame::XStorable>& xStorable,
                                        const css::uno::Reference<css::embed::XStorage>& xStorage,
                                        const OUString& rStreamRelPath,
                                        const OUString& rOwnURL, bool bCopyTo = false);

    SwDoc* getDoc() const;
    /// Stop reacting to removed database registrations.
    void releaseRevokeListener();

    /// Revoke not committed registrations in case of mail merge cancel
    void RevokeLastRegistrations();

    /// Accept not committed registrations
    void CommitLastRegistrations();

    /// Remove not used connections.
    void RevokeNotUsedConnections();
};

namespace sw
{
enum class DBConnURIType
{
    UNKNOWN = 0,
    ODB,
    CALC,
    DBASE,
    FLAT,
    MSJET,
    MSACE,
    WRITER
};

DBConnURIType SW_DLLPUBLIC GetDBunoType(const INetURLObject &rURL);
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
