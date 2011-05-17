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
    sal_uInt16 nLanguage;
    LineEnd eCRLF_Flag;

public:

    const String& GetFontName() const { return sFont; }
    void SetFontName( const String& rFont ) { sFont = rFont; }

    rtl_TextEncoding GetCharSet() const { return eCharSet; }
    void SetCharSet( rtl_TextEncoding nVal ) { eCharSet = nVal; }

    sal_uInt16 GetLanguage() const { return nLanguage; }
    void SetLanguage( sal_uInt16 nVal ) { nLanguage = nVal; }

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
    sal_Bool NeedsPasswd( const Reader& );
    sal_Bool CheckPasswd( const String&, const Reader& );
    sal_uLong Read( const Reader& );

    // ask for glossaries
    sal_Bool HasGlossaries( const Reader& );
    sal_Bool ReadGlossaries( const Reader&, SwTextBlocks&, sal_Bool bSaveRelFiles );

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
    sal_Bool bInsertMode : 1;
    sal_Bool bTmplBrowseMode : 1;
    sal_Bool bReadUTF8: 1;      // Stream als UTF-8 interpretieren
    sal_Bool bBlockMode: 1;
    sal_Bool bOrganizerMode : 1;
    sal_Bool bHasAskTemplateName : 1;
    sal_Bool bIgnoreHTMLComments : 1;

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
    sal_Bool SetTemplate( SwDoc& rDoc );
    void ClearTemplate();
    void SetTemplateName( const String& rDir );
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

    // read the sections of the document, which is equal to the medium.
    // returns the count of it
    virtual sal_uInt16 GetSectionList( SfxMedium& rMedium,
                                    SvStrings& rStrings ) const;

    SotStorageRef getSotStorageRef() { return pStg; };
    void setSotStorageRef(SotStorageRef pStgRef) { pStg = pStgRef; };

private:
    virtual sal_uLong Read(SwDoc &, const String& rBaseURL, SwPaM &,const String &)=0;

    // alle die die Streams / Storages nicht geoeffnet brauchen,
    // muessen die Methode ueberladen (W4W!!)
    virtual int SetStrmStgPtr();
};

class AsciiReader: public Reader
{
    friend class SwReader;
    virtual sal_uLong Read( SwDoc &, const String& rBaseURL, SwPaM &,const String &);
public:
    AsciiReader(): Reader() {}
};

class SW_DLLPUBLIC StgReader : public Reader
{
    String aFltName;

protected:
    sal_uLong OpenMainStream( SotStorageStreamRef& rRef, sal_uInt16& rBuffSize );
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
    sal_uLong        nErr;

public:
    SwTextBlocks( const String& );
    ~SwTextBlocks();

    void Flush(){}

    SwDoc* GetDoc();
    void   ClearDoc();                  // Doc-Inhalt loeschen
    const  String& GetName();
    void   SetName( const String& );
    sal_uLong GetError() const { return nErr; }

    String GetBaseURL() const;
    void   SetBaseURL( const String& rURL );

    sal_Bool   IsOld() const;
    sal_uLong  ConvertToNew();              // Textbausteine konvertieren

    sal_uInt16 GetCount() const;                        // Anzahl Textbausteine ermitteln
    sal_uInt16 GetIndex( const String& ) const;         // Index fuer Kurznamen ermitteln
    sal_uInt16 GetLongIndex( const String& ) const;     //Index fuer Langnamen ermitteln
    const  String& GetShortName( sal_uInt16 ) const;    // Kurzname fuer Index zurueck
    const  String& GetLongName( sal_uInt16 ) const;     // Langname fuer Index zurueck

    sal_Bool   Delete( sal_uInt16 );            // Loeschen
    sal_uInt16 Rename( sal_uInt16, const String*, const String* ); // Umbenennen
    sal_uLong  CopyBlock( SwTextBlocks& rSource, String& rSrcShort,
                                    const String& rLong ); // Block kopieren

    sal_Bool   BeginGetDoc( sal_uInt16 );           // Textbaustein einlesen
    void   EndGetDoc();                     // Textbaustein wieder loslassen

    sal_Bool   BeginPutDoc( const String&, const String& ); // Speichern Beginn
    sal_uInt16 PutDoc();                                // Speichern Ende

    sal_uInt16 PutText( const String&, const String&, const String& ); // Speichern( Kurzn., Text)

    sal_Bool IsOnlyTextBlock( sal_uInt16 ) const;
    sal_Bool IsOnlyTextBlock( const String& rShort ) const;

    const String& GetFileName() const;      // Dateiname von pImp
    sal_Bool IsReadOnly() const;                // ReadOnly-Flag von pImp

    sal_Bool GetMacroTable( sal_uInt16 nIdx, SvxMacroTableDtor& rMacroTbl );
    sal_Bool SetMacroTable( sal_uInt16 nIdx, const SvxMacroTableDtor& rMacroTbl );

    sal_Bool StartPutMuchBlockEntries();
    void EndPutMuchBlockEntries();
};

// BEGIN source/filter/basflt/fltini.cxx

extern void _InitFilter();
extern void _FinitFilter();

extern SwRead ReadAscii, /*ReadSwg, ReadSw3, */ReadHTML, ReadXML;

SW_DLLPUBLIC SwRead SwGetReaderXML();

// END source/filter/basflt/fltini.cxx


extern sal_Bool SetHTMLTemplate( SwDoc &rDoc ); //Fuer Vorlagen aus HTML.vor laden shellio.cxx


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
    void _AddFontItems( SfxItemPool& rPool, sal_uInt16 nWhichId );

    ::std::auto_ptr<Writer_Impl> m_pImpl;

protected:

    SwPaM* pOrigPam;            // der letze zu bearbeitende Pam
    const String* pOrigFileName;

    void ResetWriter();
    sal_Bool CopyNextPam( SwPaM ** );

    void PutNumFmtFontsInAttrPool();
    void PutEditEngFontsInAttrPool( sal_Bool bIncl_CJK_CTL = sal_True );
    void PutCJKandCTLFontsInAttrPool();

    virtual sal_uLong WriteStream() = 0;
    void                SetBaseURL( const String& rURL ) { sBaseURL = rURL; }

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

    virtual sal_uLong Write( SwPaM&, SfxMedium&, const String* = 0 );
            sal_uLong Write( SwPaM&, SvStream&,  const String* = 0 );
    virtual sal_uLong Write( SwPaM&, const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&, const String* = 0, SfxMedium* = 0 );
    virtual sal_uLong Write( SwPaM&, SotStorage&, const String* = 0 );

    virtual void SetPasswd( const String& );
    virtual void SetVersion( const String&, long );
    virtual sal_Bool IsStgWriter() const;

    void SetShowProgress( sal_Bool bFlag = sal_False )  { bShowProgress = bFlag; }

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
    sal_uInt16 GetBookmarks( const SwCntntNode& rNd,
                        xub_StrLen nStt, xub_StrLen nEnd,
                        SvPtrarr& rArr );

    // lege einen neuen PaM an der Position an
    static SwPaM * NewSwPaM(SwDoc & rDoc,
                            sal_uLong const nStartIdx, sal_uLong const nEndIdx);

    // kopiere ggfs. eine lokale Datei ins Internet
    sal_Bool CopyLocalFileToINet( String& rFileNm );

    // Stream-spezifische Routinen, im Storage-Writer NICHT VERWENDEN!

    // Optimierung der Ausgabe auf den Stream.
    SvStream& OutLong( SvStream& rStrm, long nVal );
    SvStream& OutULong( SvStream& rStrm, sal_uLong nVal );

    // Hex-Zahl ausgeben, default ist 2.stellige Zahl
    SvStream& OutHex( SvStream& rStrm, sal_uLong nHex, sal_uInt8 nLen = 2 );
    // 4-st. Hex-Zahl ausgeben
    inline SvStream& OutHex4( SvStream& rStrm, sal_uInt16 nHex )
        {   return OutHex( rStrm, nHex, 4 ); }

    inline SvStream& OutHex( sal_uInt16 nHex, sal_uInt8 nLen = 2 )      { return OutHex( Strm(), nHex, nLen ); }
    inline SvStream& OutHex4( sal_uInt16 nHex )     { return OutHex( Strm(), nHex, 4 ); }
    inline SvStream& OutLong( long nVal )       { return OutLong( Strm(), nVal ); }
    inline SvStream& OutULong( sal_uLong nVal )     { return OutULong( Strm(), nVal ); }

    void SetStream(SvStream *const pStream);
    SvStream& Strm();

    void SetOrganizerMode( sal_Bool bSet ) { bOrganizerMode = bSet; }
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
    virtual sal_uLong WriteStream();
    virtual sal_uLong WriteStorage() = 0;
    virtual sal_uLong WriteMedium( SfxMedium& ) = 0;

    using Writer::Write;

public:
    StgWriter() : Writer() {}

    virtual sal_Bool IsStgWriter() const;

    virtual sal_uLong Write( SwPaM&, const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&, const String* = 0, SfxMedium* = 0 );
    virtual sal_uLong Write( SwPaM&, SotStorage&, const String* = 0 );

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

    sal_Bool bWriteAll;

public:
    sal_uLong Write( WriterRef& rxWriter, const String* = 0);

    SwWriter( SvStream&, SwCrsrShell &,sal_Bool bWriteAll = sal_False );
    SwWriter( SvStream&, SwDoc & );
    SwWriter( SvStream&, SwPaM &, sal_Bool bWriteAll = sal_False );

    SwWriter( const com::sun::star::uno::Reference < com::sun::star::embed::XStorage >&, SwDoc& );

    SwWriter( SfxMedium&, SwCrsrShell &,sal_Bool bWriteAll = sal_False );
    SwWriter( SfxMedium&, SwDoc & );
};


/////////////////////////////////////////////////////////////////////////////

typedef Reader* (*FnGetReader)();
typedef void (*FnGetWriter)(const String&, const String& rBaseURL, WriterRef&);
sal_uLong SaveOrDelMSVBAStorage( SfxObjectShell&, SotStorage&, sal_Bool, const String& );
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
