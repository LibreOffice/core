/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef SW_SHELLIO_HXX
#define SW_SHELLIO_HXX

#include <memory>
#include <boost/utility.hpp>

#include <com/sun/star/uno/Reference.h>
#include <com/sun/star/embed/XStorage.hpp>
#include <sfx2/docfile.hxx>
#include <sfx2/fcontnr.hxx>
#include <sot/formats.hxx>
#include <sot/storage.hxx>
#include <svtools/parhtml.hxx>
#include <tools/string.hxx>
#include <tools/date.hxx>
#include <tools/time.hxx>
#include <tools/datetime.hxx>
#include <tools/ref.hxx>
#include <tools/urlobj.hxx>
#include <swdllapi.h>
#include <swtypes.hxx>
#include <docfac.hxx>   // SwDocFac
#include <iodetect.hxx>

// einige Forward - Deklarationen
class SfxFilterContainer;
class SfxFilter;
class SfxItemPool;
class SfxItemSet;
class SfxMedium;
class SvPtrarr;
class SvStream;
class SvStrings;
class SvxFontItem;
class SvxMacroTableDtor;
class SwCntntNode;
class SwCrsrShell;
class SwDoc;
class SwPaM;
class SwTextBlocks;
struct SwPosition;
struct Writer_Impl;

// ab so vielen chars wird ein mit einem ASCII/W4W-Reader eingelesener
// Absatz zwangsweise umgebrochen. Muss immer groesser als 200 sein !!!
#define MAX_ASCII_PARA 10000


class SW_DLLPUBLIC SwAsciiOptions
{
    String sFont;
    rtl_TextEncoding eCharSet;
    USHORT nLanguage;
    LineEnd eCRLF_Flag;

public:

    const String& GetFontName() const { return sFont; }
    void SetFontName( const String& rFont ) { sFont = rFont; }

    rtl_TextEncoding GetCharSet() const { return eCharSet; }
    void SetCharSet( rtl_TextEncoding nVal ) { eCharSet = nVal; }

    USHORT GetLanguage() const { return nLanguage; }
    void SetLanguage( USHORT nVal ) { nLanguage = nVal; }

    LineEnd GetParaFlags() const { return eCRLF_Flag; }
    void SetParaFlags( LineEnd eVal ) { eCRLF_Flag = eVal; }

    void Reset()
    {
        sFont.Erase();
        eCRLF_Flag = GetSystemLineEnd();
        eCharSet = ::gsl_getSystemTextEncoding();
        nLanguage = 0;
    }
    // for the automatic conversion (mail/news/...)
    void ReadUserData( const String& );
    void WriteUserData( String& );

    SwAsciiOptions() { Reset(); }
};

/**************** SwReader/Reader ************************/
// Basisklasse der moeglichen Optionen fuer einen speziellen Reader
class Reader;
// Ruft den Reader mit seinen Optionen, Dokument, Cursor etc.
class SwReader;
// SwRead ist der Pointer auf die Read-Optionen-Basisklasse
typedef Reader *SwRead;

class SwgReaderOption
{
    SwAsciiOptions aASCIIOpts;
    union
    {
        BOOL bFmtsOnly;
        struct
        {
            BOOL bFrmFmts: 1;
            BOOL bPageDescs: 1;
            BOOL bTxtFmts: 1;
            BOOL bNumRules: 1;
            BOOL bMerge:1;
        }  Fmts;
    } What;

public:
    void ResetAllFmtsOnly() { What.bFmtsOnly = 0; }
    BOOL IsFmtsOnly() const { return What.bFmtsOnly; }

    BOOL IsFrmFmts() const { return What.Fmts.bFrmFmts; }
    void SetFrmFmts( const BOOL bNew) { What.Fmts.bFrmFmts = bNew; }

    BOOL IsPageDescs() const { return What.Fmts.bPageDescs; }
    void SetPageDescs( const BOOL bNew) { What.Fmts.bPageDescs = bNew; }

    BOOL IsTxtFmts() const { return What.Fmts.bTxtFmts; }
    void SetTxtFmts( const BOOL bNew) { What.Fmts.bTxtFmts = bNew; }

    BOOL IsNumRules() const { return What.Fmts.bNumRules; }
    void SetNumRules( const BOOL bNew) { What.Fmts.bNumRules = bNew; }

    BOOL IsMerge() const { return What.Fmts.bMerge; }
    void SetMerge( const BOOL bNew ) { What.Fmts.bMerge = bNew; }

    const SwAsciiOptions& GetASCIIOpts() const { return aASCIIOpts; }
    void SetASCIIOpts( const SwAsciiOptions& rOpts ) { aASCIIOpts = rOpts; }
    void ResetASCIIOpts() { aASCIIOpts.Reset(); }

    SwgReaderOption()
    {   ResetAllFmtsOnly(); aASCIIOpts.Reset(); }
};

class SwReader: public SwDocFac
{
    SvStream* pStrm;
    SotStorageRef pStg;
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > xStg;
    SfxMedium* pMedium;     // wer ein Medium haben will (W4W)

    SwPaM* pCrsr;
    String aFileName;
    String sBaseURL;

public:
    /*
     * Initiales Einlesen. Dokument wird erst beim Read(..) angelegt.
     *  oder falls es mitgegeben wird, in dieses.
     *              Sonderfall fuer Load mit Sw3Reader
     */
    SwReader( SfxMedium&, const String& rFilename, SwDoc *pDoc = 0 );
    /*
     * In ein existierendes Dokument einlesen, Dokument und
     * Position im Dokument werden aus dem SwPaM uebernommen.
     */
    SwReader( SvStream&, const String& rFilename, const String& rBaseURL, SwPaM& );
    SwReader( SfxMedium&, const String& rFilename, SwPaM& );
    SwReader( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&, const String& rFilename, SwPaM& );

    /*
     * Nur SwReader::Read(...) ist die Export-Schnittstelle!!!
     */
    BOOL NeedsPasswd( const Reader& );
    BOOL CheckPasswd( const String&, const Reader& );
    ULONG Read( const Reader& );

    // ask for glossaries
    BOOL HasGlossaries( const Reader& );
    BOOL ReadGlossaries( const Reader&, SwTextBlocks&, BOOL bSaveRelFiles );

    const String&       GetBaseURL() const { return sBaseURL;}

protected:
    void                SetBaseURL( const String& rURL ) { sBaseURL = rURL; }
};



/****************  SPEZIELLE Reader ************************/

// spezielle - Reader koennen beides sein !! (Excel, W4W, .. )
#define SW_STREAM_READER    1
#define SW_STORAGE_READER   2

class SW_DLLPUBLIC Reader
{
    friend class SwReader;
    SwDoc* pTemplate;
    String aTemplateNm;

    Date aDStamp;
    Time aTStamp;
    DateTime aChkDateTime;

protected:
    SvStream* pStrm;
    SotStorageRef pStg;
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > xStg;
    SfxMedium* pMedium;     // wer ein Medium haben will (W4W)

    SwgReaderOption aOpt;
    BOOL bInsertMode : 1;
    BOOL bTmplBrowseMode : 1;
    BOOL bReadUTF8: 1;      // Stream als UTF-8 interpretieren
    BOOL bBlockMode: 1;
    BOOL bOrganizerMode : 1;
    BOOL bHasAskTemplateName : 1;
    BOOL bIgnoreHTMLComments : 1;

    virtual String GetTemplateName() const;

public:
    Reader();
    virtual ~Reader();

    virtual int GetReaderType();
    SwgReaderOption& GetReaderOpt() { return aOpt; }

    virtual void SetFltName( const String& rFltNm );
    static void SetNoOutlineNum( SwDoc& rDoc );

    // den Item-Set eines Frm-Formats an das alte Format anpassen
    static void ResetFrmFmtAttrs( SfxItemSet &rFrmSet );

    // die Rahmen-/Grafik-/OLE-Vorlagen an das alte Format (ohne
    // Umrandung etc.) anpassen
    static void ResetFrmFmts( SwDoc& rDoc );

    // Die Filter-Vorlage laden, setzen und wieder freigeben
    SwDoc* GetTemplateDoc();
    BOOL SetTemplate( SwDoc& rDoc );
    void ClearTemplate();
    void SetTemplateName( const String& rDir );
    void MakeHTMLDummyTemplateDoc();

    BOOL IsReadUTF8() const { return bReadUTF8; }
    void SetReadUTF8( BOOL bSet ) { bReadUTF8 = bSet; }

    BOOL IsBlockMode() const { return bBlockMode; }
    void SetBlockMode( BOOL bSet ) { bBlockMode = bSet; }

    BOOL IsOrganizerMode() const { return bOrganizerMode; }
    void SetOrganizerMode( BOOL bSet ) { bOrganizerMode = bSet; }

    void SetIgnoreHTMLComments( BOOL bSet ) { bIgnoreHTMLComments = bSet; }

    virtual BOOL HasGlossaries() const;
    virtual BOOL ReadGlossaries( SwTextBlocks&, BOOL bSaveRelFiles ) const;

    // read the sections of the document, which is equal to the medium.
    // returns the count of it
    virtual USHORT GetSectionList( SfxMedium& rMedium,
                                    SvStrings& rStrings ) const;

    SotStorageRef getSotStorageRef() { return pStg; };
    void setSotStorageRef(SotStorageRef pStgRef) { pStg = pStgRef; };

private:
    virtual ULONG Read(SwDoc &, const String& rBaseURL, SwPaM &,const String &)=0;

    // alle die die Streams / Storages nicht geoeffnet brauchen,
    // muessen die Methode ueberladen (W4W!!)
    virtual int SetStrmStgPtr();
};

class AsciiReader: public Reader
{
    friend class SwReader;
    virtual ULONG Read( SwDoc &, const String& rBaseURL, SwPaM &,const String &);
public:
    AsciiReader(): Reader() {}
};

class SW_DLLPUBLIC StgReader : public Reader
{
    String aFltName;
public:
    virtual int GetReaderType();
    const String& GetFltName() { return aFltName; }
    virtual void SetFltName( const String& r );
};


////////////////////////////////////////////////////////////////////////////

// Der uebergebene Stream muss dynamisch angelegt werden und
// vor dem Loeschen der Instanz per Stream() angefordert
// und geloescht werden!

class SwImpBlocks;

class SW_DLLPUBLIC SwTextBlocks
{
    SwImpBlocks* pImp;
    ULONG        nErr;

public:
    SwTextBlocks( const String& );
    ~SwTextBlocks();

    void Flush(){}

    SwDoc* GetDoc();
    void   ClearDoc();                  // Doc-Inhalt loeschen
    const  String& GetName();
    void   SetName( const String& );
    ULONG GetError() const { return nErr; }

    String GetBaseURL() const;
    void   SetBaseURL( const String& rURL );

    BOOL   IsOld() const;
    ULONG  ConvertToNew();              // Textbausteine konvertieren

    USHORT GetCount() const;                        // Anzahl Textbausteine ermitteln
    USHORT GetIndex( const String& ) const;         // Index fuer Kurznamen ermitteln
    USHORT GetLongIndex( const String& ) const;     //Index fuer Langnamen ermitteln
    const  String& GetShortName( USHORT ) const;    // Kurzname fuer Index zurueck
    const  String& GetLongName( USHORT ) const;     // Langname fuer Index zurueck

    BOOL   Delete( USHORT );            // Loeschen
    USHORT Rename( USHORT, const String*, const String* ); // Umbenennen
    ULONG  CopyBlock( SwTextBlocks& rSource, String& rSrcShort,
                                    const String& rLong ); // Block kopieren

    BOOL   BeginGetDoc( USHORT );           // Textbaustein einlesen
    void   EndGetDoc();                     // Textbaustein wieder loslassen

    BOOL   BeginPutDoc( const String&, const String& ); // Speichern Beginn
    USHORT PutDoc();                                // Speichern Ende

    USHORT PutText( const String&, const String&, const String& ); // Speichern( Kurzn., Text)

    BOOL IsOnlyTextBlock( USHORT ) const;
    BOOL IsOnlyTextBlock( const String& rShort ) const;

    const String& GetFileName() const;      // Dateiname von pImp
    BOOL IsReadOnly() const;                // ReadOnly-Flag von pImp

    BOOL GetMacroTable( USHORT nIdx, SvxMacroTableDtor& rMacroTbl );
    BOOL SetMacroTable( USHORT nIdx, const SvxMacroTableDtor& rMacroTbl );

    BOOL StartPutMuchBlockEntries();
    void EndPutMuchBlockEntries();
};

// BEGIN source/filter/basflt/fltini.cxx

extern void _InitFilter();
extern void _FinitFilter();

extern SwRead ReadAscii, /*ReadSwg, ReadSw3, */ReadHTML, ReadXML;

SW_DLLPUBLIC SwRead SwGetReaderXML();

// END source/filter/basflt/fltini.cxx


extern BOOL SetHTMLTemplate( SwDoc &rDoc ); //Fuer Vorlagen aus HTML.vor laden shellio.cxx


/////////////////////////////////////////////////////////////////////////////

/*
 * Schreiben, Writer
 */


/* Basis-Klasse aller Writer */

class IDocumentSettingAccess;
class IDocumentStylePoolAccess;

class SW_DLLPUBLIC Writer
    : public SvRefBase
    , private ::boost::noncopyable
{
    SwAsciiOptions aAscOpts;
    String          sBaseURL;

    void _AddFontItem( SfxItemPool& rPool, const SvxFontItem& rFont );
    void _AddFontItems( SfxItemPool& rPool, USHORT nWhichId );

    ::std::auto_ptr<Writer_Impl> m_pImpl;

protected:

    SwPaM* pOrigPam;            // der letze zu bearbeitende Pam
    const String* pOrigFileName;

    void ResetWriter();
    BOOL CopyNextPam( SwPaM ** );

    void PutNumFmtFontsInAttrPool();
    void PutEditEngFontsInAttrPool( BOOL bIncl_CJK_CTL = TRUE );
    void PutCJKandCTLFontsInAttrPool();

    virtual ULONG WriteStream() = 0;
    void                SetBaseURL( const String& rURL ) { sBaseURL = rURL; }

    IDocumentSettingAccess* getIDocumentSettingAccess();
    const IDocumentSettingAccess* getIDocumentSettingAccess() const;

    IDocumentStylePoolAccess* getIDocumentStylePoolAccess();
    const IDocumentStylePoolAccess* getIDocumentStylePoolAccess() const;

public:
    SwDoc* pDoc;
    SwPaM* pCurPam;
    BOOL bWriteAll : 1;
    BOOL bShowProgress : 1;
    BOOL bWriteClipboardDoc : 1;
    BOOL bWriteOnlyFirstTable : 1;
    BOOL bASCII_ParaAsCR : 1;
    BOOL bASCII_ParaAsBlanc : 1;
    BOOL bASCII_NoLastLineEnd : 1;
    BOOL bUCS2_WithStartChar : 1;
    BOOL bExportPargraphNumbering : 1;

    BOOL bBlock : 1;
    BOOL bOrganizerMode : 1;

    Writer();
    virtual ~Writer();

    virtual ULONG Write( SwPaM&, SfxMedium&, const String* = 0 );
            ULONG Write( SwPaM&, SvStream&,  const String* = 0 );
    virtual ULONG Write( SwPaM&, const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&, const String* = 0, SfxMedium* = 0 );
    virtual ULONG Write( SwPaM&, SotStorage&, const String* = 0 );

    virtual void SetPasswd( const String& );
    virtual void SetVersion( const String&, long );
    virtual BOOL IsStgWriter() const;

    void SetShowProgress( BOOL bFlag = FALSE )  { bShowProgress = bFlag; }

    const String* GetOrigFileName() const       { return pOrigFileName; }

    const SwAsciiOptions& GetAsciiOptions() const { return aAscOpts; }
    void SetAsciiOptions( const SwAsciiOptions& rOpt ) { aAscOpts = rOpt; }

    const String&       GetBaseURL() const { return sBaseURL;}

    // suche die naechste Bookmark-Position aus der Bookmark-Tabelle
    sal_Int32 FindPos_Bkmk( const SwPosition& rPos ) const;
    // build a bookmark table, which is sort by the node position. The
    // OtherPos of the bookmarks also inserted.
    void CreateBookmarkTbl();
    // search alle Bookmarks in the range and return it in the Array
    USHORT GetBookmarks( const SwCntntNode& rNd,
                        xub_StrLen nStt, xub_StrLen nEnd,
                        SvPtrarr& rArr );

    // lege einen neuen PaM an der Position an
    static SwPaM* NewSwPaM( SwDoc & rDoc, ULONG nStartIdx, ULONG nEndIdx,
                                    BOOL bNodesArray = TRUE );

    // kopiere ggfs. eine lokale Datei ins Internet
    BOOL CopyLocalFileToINet( String& rFileNm );

    // Stream-spezifische Routinen, im Storage-Writer NICHT VERWENDEN!

    // Optimierung der Ausgabe auf den Stream.
    SvStream& OutLong( SvStream& rStrm, long nVal );
    SvStream& OutULong( SvStream& rStrm, ULONG nVal );

    // Hex-Zahl ausgeben, default ist 2.stellige Zahl
    SvStream& OutHex( SvStream& rStrm, ULONG nHex, BYTE nLen = 2 );
    // 4-st. Hex-Zahl ausgeben
    inline SvStream& OutHex4( SvStream& rStrm, USHORT nHex )
        {   return OutHex( rStrm, nHex, 4 ); }

    inline SvStream& OutHex( USHORT nHex, BYTE nLen = 2 )      { return OutHex( Strm(), nHex, nLen ); }
    inline SvStream& OutHex4( USHORT nHex )     { return OutHex( Strm(), nHex, 4 ); }
    inline SvStream& OutLong( long nVal )       { return OutLong( Strm(), nVal ); }
    inline SvStream& OutULong( ULONG nVal )     { return OutULong( Strm(), nVal ); }

    void SetStream(SvStream *const pStream);
    SvStream& Strm();

    void SetOrganizerMode( BOOL bSet ) { bOrganizerMode = bSet; }
};

#ifndef SW_DECL_WRITER_DEFINED
#define SW_DECL_WRITER_DEFINED
SV_DECL_REF(Writer)
#endif
SV_IMPL_REF(Writer)

// Basisklasse fuer alle Storage-Writer
class SW_DLLPUBLIC StgWriter : public Writer
{
protected:
    String aFltName;
    SotStorageRef pStg;
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > xStg;

    // Fehler beim Aufruf erzeugen
    virtual ULONG WriteStream();
    virtual ULONG WriteStorage() = 0;
    virtual ULONG WriteMedium( SfxMedium& ) = 0;

    using Writer::Write;

public:
    StgWriter() : Writer() {}

    virtual BOOL IsStgWriter() const;

    virtual ULONG Write( SwPaM&, const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&, const String* = 0, SfxMedium* = 0 );
    virtual ULONG Write( SwPaM&, SotStorage&, const String* = 0 );

    SotStorage& GetStorage() const       { return *pStg; }
};

// Schnittstellenklasse fuer den allgemeinen Zugriff auf die
// speziellen Writer

class SwWriter
{
    SvStream* pStrm;
    SotStorageRef pStg;
    com::sun::star::uno::Reference < com::sun::star::embed::XStorage > xStg;
    SfxMedium* pMedium;

    SwPaM* pOutPam;
    SwCrsrShell *pShell;
    SwDoc &rDoc;

    BOOL bWriteAll;

public:
    ULONG Write( WriterRef& rxWriter, const String* = 0);

    SwWriter( SvStream&, SwCrsrShell &,BOOL bWriteAll = FALSE );
    SwWriter( SvStream&, SwDoc & );
    SwWriter( SvStream&, SwPaM &, BOOL bWriteAll = FALSE );

    SwWriter( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&, SwDoc& );

    SwWriter( SfxMedium&, SwCrsrShell &,BOOL bWriteAll = FALSE );
    SwWriter( SfxMedium&, SwDoc & );
};


/////////////////////////////////////////////////////////////////////////////

typedef Reader* (*FnGetReader)();
typedef void (*FnGetWriter)(const String&, const String& rBaseURL, WriterRef&);
ULONG SaveOrDelMSVBAStorage( SfxObjectShell&, SotStorage&, BOOL, const String& );
ULONG GetSaveWarningOfMSVBAStorage( SfxObjectShell &rDocS );

struct SwReaderWriterEntry
{
    Reader* pReader;
    FnGetReader fnGetReader;
    FnGetWriter fnGetWriter;
    BOOL bDelReader;

    SwReaderWriterEntry( const FnGetReader fnReader, const FnGetWriter fnWriter, BOOL bDel )
        : pReader( NULL ), fnGetReader( fnReader ), fnGetWriter( fnWriter ), bDelReader( bDel )
    {}

    /// Get access to the reader
    Reader* GetReader();

    /// Get access to the writer
    void GetWriter( const String& rNm, const String& rBaseURL, WriterRef& xWrt ) const;
};

namespace SwReaderWriter
{
    /// Return reader based on ReaderWriterEnum
    Reader* GetReader( ReaderWriterEnum eReader );

    /// Return reader based on the name
    Reader* GetReader( const String& rFltName );

    /// Return writer based on the name
    void GetWriter( const String& rFltName, const String& rBaseURL, WriterRef& xWrt );
}

void GetRTFWriter( const String&, const String&, WriterRef& );
void GetASCWriter( const String&, const String&, WriterRef& );
void GetHTMLWriter( const String&, const String&, WriterRef& );
void GetXMLWriter( const String&, const String&, WriterRef& );
void GetWW8Writer( const String&, const String&, WriterRef& );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
