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
#include <boost/utility.hpp>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/embed/XStorage.hpp>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <sot/formats.hxx>
#include <sot/storage.hxx>
#include <svtools/parhtml.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/datetime.hxx>
#include <tools/ref.hxx>
#include <swdllapi.h>
#include <swtypes.hxx>
#include <docfac.hxx>
#include <iodetect.hxx>
#include <IMark.hxx>

class SfxFilterContainer;
class SfxFilter;
class SfxItemPool;
class SfxItemSet;
class SfxMedium;
class SvStream;
class SvxFontItem;
class SvxMacroTableDtor;
class SwCntntNode;
class SwCrsrShell;
class SwDoc;
class SwPaM;
class SwTextBlocks;
struct SwPosition;
struct Writer_Impl;

// Defines the count of chars at which a paragraph read via ASCII/W4W-Reader
// is forced to wrap. It has to be always greater than 200!!!
#define MAX_ASCII_PARA 10000

class SW_DLLPUBLIC SwAsciiOptions
{
    OUString sFont;
    rtl_TextEncoding eCharSet;
    sal_uInt16 nLanguage;
    LineEnd eCRLF_Flag;

public:

    OUString GetFontName() const { return sFont; }
    void SetFontName( const OUString& rFont ) { sFont = rFont; }

    rtl_TextEncoding GetCharSet() const { return eCharSet; }
    void SetCharSet( rtl_TextEncoding nVal ) { eCharSet = nVal; }

    sal_uInt16 GetLanguage() const { return nLanguage; }
    void SetLanguage( sal_uInt16 nVal ) { nLanguage = nVal; }

    LineEnd GetParaFlags() const { return eCRLF_Flag; }
    void SetParaFlags( LineEnd eVal ) { eCRLF_Flag = eVal; }

    void Reset()
    {
        sFont = OUString();
        eCRLF_Flag = GetSystemLineEnd();
        eCharSet = ::osl_getThreadTextEncoding();
        nLanguage = 0;
    }
    // for the automatic conversion (mail/news/...)
    void ReadUserData( const OUString& );
    void WriteUserData( OUString& );

    SwAsciiOptions() { Reset(); }
};

// Base class of possible options for a special reader.
class Reader;
// Calls reader with its options, document, cursor etc.
class SwReader;
// SwRead is pointer to the read-options base class.
typedef Reader *SwRead;

class SwgReaderOption
{
    SwAsciiOptions aASCIIOpts;
    union
    {
        sal_Bool bFmtsOnly;
        struct
        {
            sal_Bool bFrmFmts: 1;
            sal_Bool bPageDescs: 1;
            sal_Bool bTxtFmts: 1;
            sal_Bool bNumRules: 1;
            sal_Bool bMerge:1;
        }  Fmts;
    } What;

public:
    void ResetAllFmtsOnly() { What.bFmtsOnly = 0; }
    sal_Bool IsFmtsOnly() const { return What.bFmtsOnly; }

    sal_Bool IsFrmFmts() const { return What.Fmts.bFrmFmts; }
    void SetFrmFmts( const sal_Bool bNew) { What.Fmts.bFrmFmts = bNew; }

    sal_Bool IsPageDescs() const { return What.Fmts.bPageDescs; }
    void SetPageDescs( const sal_Bool bNew) { What.Fmts.bPageDescs = bNew; }

    sal_Bool IsTxtFmts() const { return What.Fmts.bTxtFmts; }
    void SetTxtFmts( const sal_Bool bNew) { What.Fmts.bTxtFmts = bNew; }

    sal_Bool IsNumRules() const { return What.Fmts.bNumRules; }
    void SetNumRules( const sal_Bool bNew) { What.Fmts.bNumRules = bNew; }

    sal_Bool IsMerge() const { return What.Fmts.bMerge; }
    void SetMerge( const sal_Bool bNew ) { What.Fmts.bMerge = bNew; }

    const SwAsciiOptions& GetASCIIOpts() const { return aASCIIOpts; }
    void SetASCIIOpts( const SwAsciiOptions& rOpts ) { aASCIIOpts = rOpts; }
    void ResetASCIIOpts() { aASCIIOpts.Reset(); }

    SwgReaderOption()
    {   ResetAllFmtsOnly(); aASCIIOpts.Reset(); }
};

class SW_DLLPUBLIC SwReader: public SwDocFac
{
    SvStream* pStrm;
    SotStorageRef pStg;
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > xStg;
    SfxMedium* pMedium;     // Who wants to obtain a Medium (W4W).

    SwPaM* pCrsr;
    OUString aFileName;
    OUString sBaseURL;

public:

    // Initial reading. Document is created only at Read(...)
    // or in case it is given, into that.
    // Special case for Load with Sw3Reader.
    SwReader( SfxMedium&, const OUString& rFilename, SwDoc *pDoc = 0 );

    // Read into existing document.
    // Document and position in document are taken from SwPaM.
    SwReader( SvStream&, const OUString& rFilename, const OUString& rBaseURL, SwPaM& );
    SwReader( SfxMedium&, const OUString& rFilename, SwPaM& );
    SwReader( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&, const OUString& rFilename, SwPaM& );

    // The only export interface is SwReader::Read(...)!!!
    sal_Bool NeedsPasswd( const Reader& );
    sal_Bool CheckPasswd( const OUString&, const Reader& );
    sal_uLong Read( const Reader& );

    // Ask for glossaries.
    sal_Bool HasGlossaries( const Reader& );
    sal_Bool ReadGlossaries( const Reader&, SwTextBlocks&, sal_Bool bSaveRelFiles );

    OUString            GetBaseURL() const { return sBaseURL;}

protected:
    void                SetBaseURL( const OUString& rURL ) { sBaseURL = rURL; }
};

// Special Readers can be both!! (Excel, W4W, .. ).
#define SW_STREAM_READER    1
#define SW_STORAGE_READER   2

class SW_DLLPUBLIC Reader
{
    friend class SwReader;
    SwDoc* pTemplate;
    OUString aTemplateNm;

    Date aDStamp;
    Time aTStamp;
    DateTime aChkDateTime;

protected:
    SvStream* pStrm;
    SotStorageRef pStg;
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > xStg;
    SfxMedium* pMedium;     // Who wants to obtain a Medium (W4W).

    SwgReaderOption aOpt;
    sal_Bool bInsertMode : 1;
    sal_Bool bTmplBrowseMode : 1;
    sal_Bool bReadUTF8: 1;      // Interprete stream as UTF-8.
    sal_Bool bBlockMode: 1;
    sal_Bool bOrganizerMode : 1;
    sal_Bool bHasAskTemplateName : 1;
    sal_Bool bIgnoreHTMLComments : 1;

    virtual OUString GetTemplateName() const;

public:
    Reader();
    virtual ~Reader();

    virtual int GetReaderType();
    SwgReaderOption& GetReaderOpt() { return aOpt; }

    virtual void SetFltName( const OUString& rFltNm );

    // Adapt item-set of a Frm-Format to the old format.
    static void ResetFrmFmtAttrs( SfxItemSet &rFrmSet );

    // Adapt Frame-/Graphics-/OLE- styles to the old format
    // (without borders etc.).
    static void ResetFrmFmts( SwDoc& rDoc );

    // Load filter template, set it and release it again.
    SwDoc* GetTemplateDoc();
    sal_Bool SetTemplate( SwDoc& rDoc );
    void ClearTemplate();
    void SetTemplateName( const OUString& rDir );
    void MakeHTMLDummyTemplateDoc();

    sal_Bool IsReadUTF8() const { return bReadUTF8; }
    void SetReadUTF8( sal_Bool bSet ) { bReadUTF8 = bSet; }

    sal_Bool IsBlockMode() const { return bBlockMode; }
    void SetBlockMode( sal_Bool bSet ) { bBlockMode = bSet; }

    sal_Bool IsOrganizerMode() const { return bOrganizerMode; }
    void SetOrganizerMode( sal_Bool bSet ) { bOrganizerMode = bSet; }

    void SetIgnoreHTMLComments( sal_Bool bSet ) { bIgnoreHTMLComments = bSet; }

    virtual sal_Bool HasGlossaries() const;
    virtual sal_Bool ReadGlossaries( SwTextBlocks&, sal_Bool bSaveRelFiles ) const;

    // Read the sections of the document, which is equal to the medium.
    // Returns the count of it
    virtual size_t GetSectionList( SfxMedium& rMedium,
                                   std::vector<OUString*>& rStrings ) const;

    SotStorageRef getSotStorageRef() { return pStg; };
    void setSotStorageRef(SotStorageRef pStgRef) { pStg = pStgRef; };

private:
    virtual sal_uLong Read(SwDoc &, const OUString& rBaseURL, SwPaM &, const OUString &)=0;

    // Everyone who does not need the streams / storages open
    // has to overload the method (W4W!!).
    virtual int SetStrmStgPtr();
};

class AsciiReader: public Reader
{
    friend class SwReader;
    virtual sal_uLong Read( SwDoc &, const OUString& rBaseURL, SwPaM &, const OUString &) SAL_OVERRIDE;
public:
    AsciiReader(): Reader() {}
};

class SW_DLLPUBLIC StgReader : public Reader
{
    OUString aFltName;

protected:
    sal_uLong OpenMainStream( SotStorageStreamRef& rRef, sal_uInt16& rBuffSize );
public:
    virtual int GetReaderType() SAL_OVERRIDE;
    OUString GetFltName() { return aFltName; }
    virtual void SetFltName( const OUString& r ) SAL_OVERRIDE;
};

// The given stream has to be created dynamically and must
// be requested via Stream() before the instance is deleted!

class SwImpBlocks;

class SW_DLLPUBLIC SwTextBlocks
{
    SwImpBlocks* pImp;
    sal_uLong        nErr;

public:
    SwTextBlocks( const OUString& );
    ~SwTextBlocks();

    void Flush(){}

    SwDoc* GetDoc();
    void   ClearDoc();                  // Delete Doc-contents.
    OUString GetName();
    void   SetName( const OUString& );
    sal_uLong GetError() const { return nErr; }

    OUString GetBaseURL() const;
    void   SetBaseURL( const OUString& rURL );

    sal_Bool   IsOld() const;
    sal_uLong  ConvertToNew();              // Convert text modules.

    sal_uInt16 GetCount() const;                        // Get count text modules.
    sal_uInt16 GetIndex( const OUString& ) const;       // Get index of short names.
    sal_uInt16 GetLongIndex( const OUString& ) const;   // Get index of long names.
    OUString GetShortName( sal_uInt16 ) const;          // Get short name for index.
    OUString GetLongName( sal_uInt16 ) const;           // Get long name for index.

    sal_Bool   Delete( sal_uInt16 );
    sal_uInt16 Rename( sal_uInt16, const OUString*, const OUString* );
    sal_uLong  CopyBlock( SwTextBlocks& rSource, OUString& rSrcShort,
                                    const OUString& rLong );

    sal_Bool   BeginGetDoc( sal_uInt16 );   // Read text modules.
    void   EndGetDoc();                     // Release text modules.

    sal_Bool   BeginPutDoc( const OUString&, const OUString& ); // Begin save.
    sal_uInt16 PutDoc();                                    // End save.

    sal_uInt16 PutText( const OUString&, const OUString&, const OUString& ); // Save (short name, text).

    sal_Bool IsOnlyTextBlock( sal_uInt16 ) const;
    sal_Bool IsOnlyTextBlock( const OUString& rShort ) const;

    OUString GetFileName() const;           // Filename of pImp.
    sal_Bool IsReadOnly() const;            // ReadOnly-flag of pImp.

    sal_Bool GetMacroTable( sal_uInt16 nIdx, SvxMacroTableDtor& rMacroTbl );
    sal_Bool SetMacroTable( sal_uInt16 nIdx, const SvxMacroTableDtor& rMacroTbl );

    sal_Bool StartPutMuchBlockEntries();
    void EndPutMuchBlockEntries();
};

// BEGIN source/filter/basflt/fltini.cxx

extern SwRead ReadAscii, /*ReadSwg, ReadSw3, */ReadHTML, ReadXML;

SW_DLLPUBLIC SwRead SwGetReaderXML();

// END source/filter/basflt/fltini.cxx

extern sal_Bool SetHTMLTemplate( SwDoc &rDoc ); //For templates from HTML before loading shellio.cxx.

// Base-class of all writers.

class IDocumentSettingAccess;
class IDocumentStylePoolAccess;

class SW_DLLPUBLIC Writer
    : public SvRefBase
    , private ::boost::noncopyable
{
    SwAsciiOptions aAscOpts;
    OUString       sBaseURL;

    void _AddFontItem( SfxItemPool& rPool, const SvxFontItem& rFont );
    void _AddFontItems( SfxItemPool& rPool, sal_uInt16 nWhichId );

    ::std::auto_ptr<Writer_Impl> m_pImpl;

protected:

    SwPaM* pOrigPam;            // Last Pam that has to be processed.
    const OUString* pOrigFileName;

    void ResetWriter();
    sal_Bool CopyNextPam( SwPaM ** );

    void PutNumFmtFontsInAttrPool();
    void PutEditEngFontsInAttrPool( bool bIncl_CJK_CTL = true );

    virtual sal_uLong WriteStream() = 0;
    void                SetBaseURL( const OUString& rURL ) { sBaseURL = rURL; }

    IDocumentSettingAccess* getIDocumentSettingAccess();
    const IDocumentSettingAccess* getIDocumentSettingAccess() const;

    IDocumentStylePoolAccess* getIDocumentStylePoolAccess();
    const IDocumentStylePoolAccess* getIDocumentStylePoolAccess() const;

public:
    SwDoc* pDoc;
    SwPaM* pCurPam;
    sal_Bool bWriteAll : 1;
    sal_Bool bShowProgress : 1;
    sal_Bool bWriteClipboardDoc : 1;
    sal_Bool bWriteOnlyFirstTable : 1;
    sal_Bool bASCII_ParaAsCR : 1;
    sal_Bool bASCII_ParaAsBlanc : 1;
    sal_Bool bASCII_NoLastLineEnd : 1;
    sal_Bool bUCS2_WithStartChar : 1;
    sal_Bool bExportPargraphNumbering : 1;

    sal_Bool bBlock : 1;
    sal_Bool bOrganizerMode : 1;

    Writer();
    virtual ~Writer();

    virtual sal_uLong Write( SwPaM&, SfxMedium&, const OUString* = 0 );
            sal_uLong Write( SwPaM&, SvStream&,  const OUString* = 0 );
    virtual sal_uLong Write( SwPaM&, const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&, const OUString* = 0, SfxMedium* = 0 );
    virtual sal_uLong Write( SwPaM&, SotStorage&, const OUString* = 0 );

    virtual void SetupFilterOptions(SfxMedium& rMedium);

    virtual void SetVersion( const OUString&, long );
    virtual sal_Bool IsStgWriter() const;

    void SetShowProgress( sal_Bool bFlag = sal_False )  { bShowProgress = bFlag; }

    const OUString* GetOrigFileName() const       { return pOrigFileName; }

    const SwAsciiOptions& GetAsciiOptions() const { return aAscOpts; }
    void SetAsciiOptions( const SwAsciiOptions& rOpt ) { aAscOpts = rOpt; }

    OUString GetBaseURL() const { return sBaseURL;}

    // Look up next bookmark position from bookmark-table.
    sal_Int32 FindPos_Bkmk( const SwPosition& rPos ) const;
    // Build a bookmark table, which is sort by the node position. The
    // OtherPos of the bookmarks also inserted.
    void CreateBookmarkTbl();
    // Search alle Bookmarks in the range and return it in the Array.
    sal_uInt16 GetBookmarks( const SwCntntNode& rNd,
                        sal_Int32 nStt, sal_Int32 nEnd,
                        std::vector< const ::sw::mark::IMark* >& rArr );

    // Create new PaM at position.
    static SwPaM * NewSwPaM(SwDoc & rDoc,
                            sal_uLong const nStartIdx, sal_uLong const nEndIdx);

    // Stream-specific routines. Do not use in storage-writer!

    // Optimizing output on stream.
    SvStream& OutLong( SvStream& rStrm, long nVal );
    SvStream& OutULong( SvStream& rStrm, sal_uLong nVal );

    inline SvStream& OutLong( long nVal )       { return OutLong( Strm(), nVal ); }
    inline SvStream& OutULong( sal_uLong nVal )     { return OutULong( Strm(), nVal ); }

    void SetStream(SvStream *const pStream);
    SvStream& Strm();

    void SetOrganizerMode( sal_Bool bSet ) { bOrganizerMode = bSet; }
};

#ifndef SW_DECL_WRITER_DEFINED
#define SW_DECL_WRITER_DEFINED
typedef tools::SvRef<Writer> WriterRef;
#endif

// Base class for all storage writers.
class SW_DLLPUBLIC StgWriter : public Writer
{
protected:
    OUString aFltName;
    SotStorageRef pStg;
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > xStg;

    // Create error at call.
    virtual sal_uLong WriteStream() SAL_OVERRIDE;
    virtual sal_uLong WriteStorage() = 0;
    virtual sal_uLong WriteMedium( SfxMedium& ) = 0;

    using Writer::Write;

public:
    StgWriter() : Writer() {}

    virtual sal_Bool IsStgWriter() const SAL_OVERRIDE;

    virtual sal_uLong Write( SwPaM&, const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&, const OUString* = 0, SfxMedium* = 0 ) SAL_OVERRIDE;
    virtual sal_uLong Write( SwPaM&, SotStorage&, const OUString* = 0 ) SAL_OVERRIDE;

    SotStorage& GetStorage() const       { return *pStg; }
};

// Interface class for general access on special writers.

class SW_DLLPUBLIC SwWriter
{
    SvStream* pStrm;
    SotStorageRef pStg;
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > xStg;
    SfxMedium* pMedium;

    SwPaM* pOutPam;
    SwCrsrShell *pShell;
    SwDoc &rDoc;

    sal_Bool bWriteAll;

public:
    sal_uLong Write( WriterRef& rxWriter, const OUString* = 0);

    SwWriter( SvStream&, SwCrsrShell &,sal_Bool bWriteAll = sal_False );
    SwWriter( SvStream&, SwDoc & );
    SwWriter( SvStream&, SwPaM &, sal_Bool bWriteAll = sal_False );

    SwWriter( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&, SwDoc& );

    SwWriter( SfxMedium&, SwCrsrShell &,sal_Bool bWriteAll = sal_False );
    SwWriter( SfxMedium&, SwDoc & );
};

typedef Reader* (*FnGetReader)();
typedef void (*FnGetWriter)(const OUString&, const OUString& rBaseURL, WriterRef&);
sal_uLong SaveOrDelMSVBAStorage( SfxObjectShell&, SotStorage&, sal_Bool, const OUString& );
sal_uLong GetSaveWarningOfMSVBAStorage( SfxObjectShell &rDocS );

struct SwReaderWriterEntry
{
    Reader* pReader;
    FnGetReader fnGetReader;
    FnGetWriter fnGetWriter;
    sal_Bool bDelReader;

    SwReaderWriterEntry( const FnGetReader fnReader, const FnGetWriter fnWriter, sal_Bool bDel )
        : pReader( NULL ), fnGetReader( fnReader ), fnGetWriter( fnWriter ), bDelReader( bDel )
    {}

    /// Get access to the reader.
    Reader* GetReader();

    /// Get access to the writer.
    void GetWriter( const OUString& rNm, const OUString& rBaseURL, WriterRef& xWrt ) const;
};

namespace SwReaderWriter
{
    /// Return reader based on ReaderWriterEnum.
    SW_DLLPUBLIC Reader* GetReader( ReaderWriterEnum eReader );

    /// Return reader based on the name.
    Reader* GetReader( const OUString& rFltName );

    /// Return writer based on the name.
    SW_DLLPUBLIC void GetWriter( const OUString& rFltName, const OUString& rBaseURL, WriterRef& xWrt );
}

void GetRTFWriter( const OUString&, const OUString&, WriterRef& );
void GetASCWriter( const OUString&, const OUString&, WriterRef& );
void GetHTMLWriter( const OUString&, const OUString&, WriterRef& );
void GetXMLWriter( const OUString&, const OUString&, WriterRef& );
void GetWW8Writer( const OUString&, const OUString&, WriterRef& );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
