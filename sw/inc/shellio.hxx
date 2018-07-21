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
#ifndef INCLUDED_SW_INC_SHELLIO_HXX
#define INCLUDED_SW_INC_SHELLIO_HXX

#include <memory>

#include <com/sun/star/uno/Reference.h>
#include <sot/storage.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/datetime.hxx>
#include <tools/ref.hxx>
#include <rtl/ref.hxx>
#include <osl/thread.h>
#include <o3tl/typed_flags_set.hxx>
#include "swdllapi.h"
#include "docfac.hxx"

class SfxItemPool;
class SfxItemSet;
class SfxMedium;
class SvStream;
class SvxFontItem;
class SvxMacroTableDtor;
class SwContentNode;
class SwCursorShell;
class SwDoc;
class SwPaM;
class SwTextBlocks;
struct SwPosition;
struct Writer_Impl;
namespace sw
{
namespace mark
{
class IMark;
}
}
namespace com { namespace sun { namespace star { namespace embed { class XStorage; } } } }

// Defines the count of chars at which a paragraph read via ASCII/W4W-Reader
// is forced to wrap. It has to be always greater than 200!!!
#define MAX_ASCII_PARA 10000

class SW_DLLPUBLIC SwAsciiOptions
{
    OUString sFont;
    rtl_TextEncoding eCharSet;
    LanguageType nLanguage;
    LineEnd eCRLF_Flag;

public:

    const OUString& GetFontName() const { return sFont; }
    void SetFontName( const OUString& rFont ) { sFont = rFont; }

    rtl_TextEncoding GetCharSet() const { return eCharSet; }
    void SetCharSet( rtl_TextEncoding nVal ) { eCharSet = nVal; }

    LanguageType GetLanguage() const { return nLanguage; }
    void SetLanguage( LanguageType nVal ) { nLanguage = nVal; }

    LineEnd GetParaFlags() const { return eCRLF_Flag; }
    void SetParaFlags( LineEnd eVal ) { eCRLF_Flag = eVal; }

    void Reset()
    {
        sFont.clear();
        eCRLF_Flag = GetSystemLineEnd();
        eCharSet = ::osl_getThreadTextEncoding();
        nLanguage = LANGUAGE_SYSTEM;
    }
    // for the automatic conversion (mail/news/...)
    void ReadUserData( const OUString& );
    void WriteUserData( OUString& );

    SwAsciiOptions() { Reset(); }
};

// Base class of possible options for a special reader.
class Reader;
// SwRead is pointer to the read-options base class.
typedef Reader *SwRead;

class SwgReaderOption
{
    SwAsciiOptions aASCIIOpts;
    bool m_bFrameFormats;
    bool m_bPageDescs;
    bool m_bTextFormats;
    bool m_bNumRules;
    bool m_bMerge;
public:
    void ResetAllFormatsOnly() { m_bFrameFormats = m_bPageDescs = m_bTextFormats = m_bNumRules = m_bMerge = false; }
    bool IsFormatsOnly() const { return m_bFrameFormats || m_bPageDescs || m_bTextFormats || m_bNumRules || m_bMerge; }

    bool IsFrameFormats() const { return m_bFrameFormats; }
    void SetFrameFormats( const bool bNew) { m_bFrameFormats = bNew; }

    bool IsPageDescs() const { return m_bPageDescs; }
    void SetPageDescs( const bool bNew) { m_bPageDescs = bNew; }

    bool IsTextFormats() const { return m_bTextFormats; }
    void SetTextFormats( const bool bNew) { m_bTextFormats = bNew; }

    bool IsNumRules() const { return m_bNumRules; }
    void SetNumRules( const bool bNew) { m_bNumRules = bNew; }

    bool IsMerge() const { return m_bMerge; }
    void SetMerge( const bool bNew ) { m_bMerge = bNew; }

    const SwAsciiOptions& GetASCIIOpts() const { return aASCIIOpts; }
    void SetASCIIOpts( const SwAsciiOptions& rOpts ) { aASCIIOpts = rOpts; }
    void ResetASCIIOpts() { aASCIIOpts.Reset(); }

    SwgReaderOption()
        { ResetAllFormatsOnly(); aASCIIOpts.Reset(); }
};

// Calls reader with its options, document, cursor etc.
class SW_DLLPUBLIC SwReader: public SwDocFac
{
    SvStream* pStrm;
    tools::SvRef<SotStorage> pStg;
    css::uno::Reference < css::embed::XStorage > xStg;
    SfxMedium* pMedium;     // Who wants to obtain a Medium (W4W).

    SwPaM* pCursor;
    OUString aFileName;
    OUString sBaseURL;
    bool mbSkipImages;

public:

    // Initial reading. Document is created only at Read(...)
    // or in case it is given, into that.
    // Special case for Load with Sw3Reader.
    SwReader( SfxMedium&, const OUString& rFilename, SwDoc *pDoc = nullptr );

    // Read into existing document.
    // Document and position in document are taken from SwPaM.
    SwReader( SvStream&, const OUString& rFilename, const OUString& rBaseURL, SwPaM& );
    SwReader( SfxMedium&, const OUString& rFilename, SwPaM& );
    SwReader( const css::uno::Reference < css::embed::XStorage >&, const OUString& rFilename, SwPaM& );

    // The only export interface is SwReader::Read(...)!!!
    ErrCode Read( const Reader& );

    // Ask for glossaries.
    bool HasGlossaries( const Reader& );
    bool ReadGlossaries( const Reader&, SwTextBlocks&, bool bSaveRelFiles );

protected:
    void                SetBaseURL( const OUString& rURL ) { sBaseURL = rURL; }
    void                SetSkipImages( bool bSkipImages ) { mbSkipImages = bSkipImages; }
};

// Special Readers can be both!! (Excel, W4W, .. ).
enum class SwReaderType {
    NONE = 0x00,
    Stream = 0x01,
    Storage = 0x02
};
namespace o3tl {
    template<> struct typed_flags<SwReaderType> : is_typed_flags<SwReaderType, 0x03> {};
};

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportDOC(SvStream &rStream, const OUString &rFltName);
extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportRTF(SvStream &rStream);
extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportHTML(SvStream &rStream);
SAL_DLLPUBLIC_EXPORT void FlushFontCache();

class SW_DLLPUBLIC Reader
{
    friend class SwReader;
    friend bool TestImportDOC(SvStream &rStream, const OUString &rFltName);
    friend bool TestImportRTF(SvStream &rStream);
    friend bool TestImportHTML(SvStream &rStream);
    rtl::Reference<SwDoc> mxTemplate;
    OUString m_aTemplateName;

    Date m_aDateStamp;
    tools::Time m_aTimeStamp;
    DateTime m_aCheckDateTime;

protected:
    SvStream* m_pStream;
    tools::SvRef<SotStorage> m_pStorage;
    css::uno::Reference < css::embed::XStorage > m_xStorage;
    SfxMedium* m_pMedium;     // Who wants to obtain a Medium (W4W).

    SwgReaderOption m_aOption;
    bool m_bInsertMode : 1;
    bool m_bTemplateBrowseMode : 1;
    bool m_bReadUTF8: 1;      // Interpret stream as UTF-8.
    bool m_bBlockMode: 1;
    bool m_bOrganizerMode : 1;
    bool m_bHasAskTemplateName : 1;
    bool m_bIgnoreHTMLComments : 1;
    bool m_bSkipImages : 1;

    virtual OUString GetTemplateName(SwDoc& rDoc) const;

public:
    Reader();
    virtual ~Reader();

    virtual SwReaderType GetReaderType();
    SwgReaderOption& GetReaderOpt() { return m_aOption; }

    virtual void SetFltName( const OUString& rFltNm );

    // Adapt item-set of a Frame-Format to the old format.
    static void ResetFrameFormatAttrs( SfxItemSet &rFrameSet );

    // Adapt Frame-/Graphics-/OLE- styles to the old format
    // (without borders etc.).
    static void ResetFrameFormats( SwDoc& rDoc );

    // Load filter template, set it and release it again.
    SwDoc* GetTemplateDoc(SwDoc& rDoc);
    bool SetTemplate( SwDoc& rDoc );
    void ClearTemplate();
    void SetTemplateName( const OUString& rDir );
    void MakeHTMLDummyTemplateDoc();

    bool IsReadUTF8() const { return m_bReadUTF8; }
    void SetReadUTF8( bool bSet ) { m_bReadUTF8 = bSet; }

    bool IsBlockMode() const { return m_bBlockMode; }
    void SetBlockMode( bool bSet ) { m_bBlockMode = bSet; }

    bool IsOrganizerMode() const { return m_bOrganizerMode; }
    void SetOrganizerMode( bool bSet ) { m_bOrganizerMode = bSet; }

    void SetIgnoreHTMLComments( bool bSet ) { m_bIgnoreHTMLComments = bSet; }

    virtual bool HasGlossaries() const;
    virtual bool ReadGlossaries( SwTextBlocks&, bool bSaveRelFiles ) const;

    // Read the sections of the document, which is equal to the medium.
    // Returns the count of it
    virtual size_t GetSectionList( SfxMedium& rMedium,
                                   std::vector<OUString>& rStrings) const;

    const tools::SvRef<SotStorage>& getSotStorageRef() { return m_pStorage; };
    void setSotStorageRef(const tools::SvRef<SotStorage>& pStgRef) { m_pStorage = pStgRef; };

private:
    virtual ErrCode Read(SwDoc &, const OUString& rBaseURL, SwPaM &, const OUString &)=0;

    // Everyone who does not need the streams / storages open
    // has to override the method (W4W!!).
    virtual bool SetStrmStgPtr();
};

class AsciiReader: public Reader
{
    friend class SwReader;
    virtual ErrCode Read( SwDoc &, const OUString& rBaseURL, SwPaM &, const OUString &) override;
public:
    AsciiReader(): Reader() {}
};

class SW_DLLPUBLIC StgReader : public Reader
{
    OUString aFltName;

public:
    virtual SwReaderType GetReaderType() override;
    const OUString& GetFltName() { return aFltName; }
    virtual void SetFltName( const OUString& r ) override;
};

// The given stream has to be created dynamically and must
// be requested via Stream() before the instance is deleted!

class SwImpBlocks;

class SW_DLLPUBLIC SwTextBlocks
{
    std::unique_ptr<SwImpBlocks> pImp;
    ErrCode        nErr;

public:
    SwTextBlocks( const OUString& );
    ~SwTextBlocks();

    SwDoc* GetDoc();
    void   ClearDoc();                  // Delete Doc-contents.
    OUString GetName();
    void   SetName( const OUString& );
    ErrCode const & GetError() const { return nErr; }

    OUString GetBaseURL() const;
    void   SetBaseURL( const OUString& rURL );

    bool   IsOld() const;

    sal_uInt16 GetCount() const;                        // Get count text modules.
    sal_uInt16 GetIndex( const OUString& ) const;       // Get index of short names.
    sal_uInt16 GetLongIndex( const OUString& ) const;   // Get index of long names.
    OUString GetShortName( sal_uInt16 ) const;          // Get short name for index.
    OUString GetLongName( sal_uInt16 ) const;           // Get long name for index.

    bool   Delete( sal_uInt16 );
    void   Rename( sal_uInt16, const OUString*, const OUString* );
    ErrCode const & CopyBlock( SwTextBlocks const & rSource, OUString& rSrcShort,
                                    const OUString& rLong );

    bool   BeginGetDoc( sal_uInt16 );   // Read text modules.
    void   EndGetDoc();                     // Release text modules.

    bool   BeginPutDoc( const OUString&, const OUString& ); // Begin save.
    sal_uInt16 PutDoc();                                    // End save.

    sal_uInt16 PutText( const OUString&, const OUString&, const OUString& ); // Save (short name, text).

    bool IsOnlyTextBlock( sal_uInt16 ) const;
    bool IsOnlyTextBlock( const OUString& rShort ) const;

    OUString const & GetFileName() const;           // Filename of pImp.
    bool IsReadOnly() const;            // ReadOnly-flag of pImp.

    bool GetMacroTable( sal_uInt16 nIdx, SvxMacroTableDtor& rMacroTable );
    bool SetMacroTable( sal_uInt16 nIdx, const SvxMacroTableDtor& rMacroTable );

    bool StartPutMuchBlockEntries();
    void EndPutMuchBlockEntries();
};

// BEGIN source/filter/basflt/fltini.cxx

extern SwRead ReadAscii, /*ReadSwg, ReadSw3, */ReadHTML, ReadXML;

SW_DLLPUBLIC SwRead SwGetReaderXML();

// END source/filter/basflt/fltini.cxx

extern bool SetHTMLTemplate( SwDoc &rDoc ); //For templates from HTML before loading shellio.cxx.

// Base-class of all writers.

class IDocumentSettingAccess;
class IDocumentStylePoolAccess;

class SW_DLLPUBLIC Writer
    : public SvRefBase
{
    SwAsciiOptions m_aAsciiOptions;
    OUString       m_sBaseURL;

    void AddFontItem( SfxItemPool& rPool, const SvxFontItem& rFont );
    void AddFontItems_( SfxItemPool& rPool, sal_uInt16 nWhichId );

    std::unique_ptr<Writer_Impl> m_pImpl;

    Writer(Writer const&) = delete;
    Writer& operator=(Writer const&) = delete;

protected:

    const OUString* m_pOrigFileName;

    void ResetWriter();
    bool CopyNextPam( SwPaM ** );

    void PutNumFormatFontsInAttrPool();
    void PutEditEngFontsInAttrPool();

    virtual ErrCode WriteStream() = 0;
    void                SetBaseURL( const OUString& rURL ) { m_sBaseURL = rURL; }

    IDocumentSettingAccess& getIDocumentSettingAccess();
    const IDocumentSettingAccess& getIDocumentSettingAccess() const;

    IDocumentStylePoolAccess& getIDocumentStylePoolAccess();
    const IDocumentStylePoolAccess& getIDocumentStylePoolAccess() const;

public:
    SwDoc* m_pDoc;
    SwPaM* m_pOrigPam;            // Last Pam that has to be processed.
    SwPaM* m_pCurrentPam;
    bool m_bWriteAll : 1;
    bool m_bShowProgress : 1;
    bool m_bWriteClipboardDoc : 1;
    bool m_bWriteOnlyFirstTable : 1;
    bool m_bASCII_ParaAsCR : 1;
    bool m_bASCII_ParaAsBlank : 1;
    bool m_bASCII_NoLastLineEnd : 1;
    bool m_bUCS2_WithStartChar : 1;
    bool m_bExportPargraphNumbering : 1;

    bool m_bBlock : 1;
    bool m_bOrganizerMode : 1;

    Writer();
    virtual ~Writer() override;

    virtual ErrCode Write( SwPaM&, SfxMedium&, const OUString* );
            ErrCode Write( SwPaM&, SvStream&,  const OUString* );
    virtual ErrCode Write( SwPaM&, const css::uno::Reference < css::embed::XStorage >&, const OUString*, SfxMedium* = nullptr );
    virtual ErrCode Write( SwPaM&, SotStorage&, const OUString* );

    virtual void SetupFilterOptions(SfxMedium& rMedium);

    virtual bool IsStgWriter() const;

    void SetShowProgress( bool bFlag )  { m_bShowProgress = bFlag; }

    const OUString* GetOrigFileName() const       { return m_pOrigFileName; }

    const SwAsciiOptions& GetAsciiOptions() const { return m_aAsciiOptions; }
    void SetAsciiOptions( const SwAsciiOptions& rOpt ) { m_aAsciiOptions = rOpt; }

    const OUString& GetBaseURL() const { return m_sBaseURL;}

    // Look up next bookmark position from bookmark-table.
    sal_Int32 FindPos_Bkmk( const SwPosition& rPos ) const;
    // Build a bookmark table, which is sort by the node position. The
    // OtherPos of the bookmarks also inserted.
    void CreateBookmarkTable();
    // Search all Bookmarks in the range and return it in the Array.
    bool GetBookmarks( const SwContentNode& rNd,
                        sal_Int32 nStt, sal_Int32 nEnd,
                        std::vector< const ::sw::mark::IMark* >& rArr );

    // Create new PaM at position.
    static SwPaM * NewSwPaM(SwDoc & rDoc,
                            sal_uLong const nStartIdx, sal_uLong const nEndIdx);

    // If applicable copy a local file into internet.
    bool CopyLocalFileToINet( OUString& rFileNm );

    // Stream-specific routines. Do not use in storage-writer!

    // Optimizing output on stream.
    static SvStream& OutLong( SvStream& rStrm, long nVal );
    static SvStream& OutULong( SvStream& rStrm, sal_uLong nVal );

    void SetStream(SvStream *const pStream);
    SvStream& Strm();

    void SetOrganizerMode( bool bSet ) { m_bOrganizerMode = bSet; }
};

typedef tools::SvRef<Writer> WriterRef;

// Base class for all storage writers.
class SW_DLLPUBLIC StgWriter : public Writer
{
protected:
    tools::SvRef<SotStorage> pStg;
    css::uno::Reference < css::embed::XStorage > xStg;

    // Create error at call.
    virtual ErrCode WriteStream() override;
    virtual ErrCode WriteStorage() = 0;
    virtual ErrCode WriteMedium( SfxMedium& ) = 0;

    using Writer::Write;

public:
    StgWriter() : Writer() {}

    virtual bool IsStgWriter() const override;

    virtual ErrCode Write( SwPaM&, const css::uno::Reference < css::embed::XStorage >&, const OUString*, SfxMedium* = nullptr ) override;
    virtual ErrCode Write( SwPaM&, SotStorage&, const OUString* ) override;

    SotStorage& GetStorage() const       { return *pStg; }
};

// Interface class for general access on special writers.

class SW_DLLPUBLIC SwWriter
{
    SvStream* pStrm;
    css::uno::Reference < css::embed::XStorage > xStg;
    SfxMedium* pMedium;

    SwPaM* pOutPam;
    SwCursorShell *pShell;
    SwDoc &rDoc;

    bool bWriteAll;

public:
    ErrCode Write( WriterRef const & rxWriter, const OUString* = nullptr);

    SwWriter( SvStream&, SwCursorShell &, bool bWriteAll = false );
    SwWriter( SvStream&, SwDoc & );
    SwWriter( SvStream&, SwPaM &, bool bWriteAll = false );

    SwWriter( const css::uno::Reference < css::embed::XStorage >&, SwDoc& );

    SwWriter( SfxMedium&, SwCursorShell &, bool bWriteAll );
    SwWriter( SfxMedium&, SwDoc & );
};

typedef Reader* (*FnGetReader)();
typedef void (*FnGetWriter)(const OUString&, const OUString& rBaseURL, WriterRef&);
ErrCode SaveOrDelMSVBAStorage( SfxObjectShell&, SotStorage&, bool, const OUString& );
ErrCode GetSaveWarningOfMSVBAStorage( SfxObjectShell &rDocS );

struct SwReaderWriterEntry
{
    Reader* pReader;
    FnGetReader fnGetReader;
    FnGetWriter fnGetWriter;
    bool bDelReader;

    SwReaderWriterEntry( const FnGetReader fnReader, const FnGetWriter fnWriter, bool bDel )
        : pReader( nullptr ), fnGetReader( fnReader ), fnGetWriter( fnWriter ), bDelReader( bDel )
    {}

    /// Get access to the reader.
    Reader* GetReader();

    /// Get access to the writer.
    void GetWriter( const OUString& rNm, const OUString& rBaseURL, WriterRef& xWrt ) const;
};

namespace SwReaderWriter
{
    SW_DLLPUBLIC Reader* GetRtfReader();
    SW_DLLPUBLIC Reader* GetDOCXReader();

    /// Return reader based on the name.
    Reader* GetReader( const OUString& rFltName );

    /// Return writer based on the name.
    SW_DLLPUBLIC void GetWriter( const OUString& rFltName, const OUString& rBaseURL, WriterRef& xWrt );
}

void GetRTFWriter( const OUString&, const OUString&, WriterRef& );
void GetASCWriter(const OUString&, const OUString&, WriterRef&);
void GetHTMLWriter( const OUString&, const OUString&, WriterRef& );
void GetXMLWriter( const OUString&, const OUString&, WriterRef& );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
