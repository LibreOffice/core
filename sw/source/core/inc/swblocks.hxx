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
#ifndef _SWBLOCKS_HXX
#define _SWBLOCKS_HXX

#include <tools/string.hxx>

#ifndef _DATE_HXX //autogen
#include <tools/datetime.hxx>
#endif
#include <svl/svarray.hxx>


class SfxMedium;
class SwPaM;
class SwDoc;
class SvxMacroTableDtor;

// Name eines Textblocks:

class SwBlockName
{
    friend class SwImpBlocks;
    friend class Sw2TextBlocks;
    sal_uInt16 nHashS, nHashL;              // Hash-Codes zum Checken
    long   nPos;                        // Dateiposition (SW2-Format)
public:
    String aShort;                      // Short name
    String aLong;                       // Long name
    String aPackageName;                // Package name
    sal_Bool bIsOnlyTxtFlagInit : 1;        // ist das Flag gueltig?
    sal_Bool bIsOnlyTxt : 1;                // unformatted text
    sal_Bool bInPutMuchBlocks : 1;          // put serveral block entries

    SwBlockName( const String& rShort, const String& rLong, long n );
    SwBlockName( const String& rShort, const String& rLong, const String& rPackageName );

    // fuer das Einsortieren in das Array
    int operator==( const SwBlockName& r ) { return aShort == r.aShort; }
    int operator< ( const SwBlockName& r ) { return aShort <  r.aShort; }
};

SV_DECL_PTRARR_SORT( SwBlockNames, SwBlockName*, 10, 10 )


class SwImpBlocks
{
    friend class SwTextBlocks;
protected:
    String aFile;                       // physikalischer Dateiname
    String aName;                       // logischer Name der Datei
    String aCur;                        // aktueller Text
    String aShort, aLong;               // Kurz- und Langname (PutDoc)
    String sBaseURL;                    // base URL - has to be set at the Readers and Writers
    SwBlockNames aNames;                // Liste aller Bausteine
    Date aDateModified;                 // fuers abgleichen bei den Aktionen
    Time aTimeModified;
    SwDoc* pDoc;                        // Austauschdokument
    sal_uInt16 nCur;                        // aktueller Index
    sal_Bool bReadOnly : 1;
    sal_Bool bInPutMuchBlocks : 1;          // put serveral block entries
    sal_Bool bInfoChanged : 1;              // any Info of TextBlock is changed

    SwImpBlocks( const String&, sal_Bool = sal_False );
    virtual ~SwImpBlocks();

    static short GetFileType( const String& );
    virtual short GetFileType() const = 0;
#define SWBLK_NO_FILE   0               // nicht da
#define SWBLK_NONE      1               // keine TB-Datei
#define SWBLK_SW2       2               // SW2-Datei
#define SWBLK_SW3       3               // SW3-Datei
#define SWBLK_XML       4               // XML Block List

    virtual void   ClearDoc();                  // Doc-Inhalt loeschen
    SwPaM* MakePaM();                   // PaM ueber Doc aufspannen
    virtual void   AddName( const String&, const String&, sal_Bool bOnlyTxt = sal_False );
    sal_Bool   IsFileChanged() const;
    void   Touch();

public:
    static sal_uInt16 Hash( const String& );        // Hashcode fuer Blocknamen
    sal_uInt16 GetCount() const;                    // Anzahl Textbausteine ermitteln
    sal_uInt16 GetIndex( const String& ) const;     // Index fuer Kurznamen ermitteln
    sal_uInt16 GetLongIndex( const String& ) const; //Index fuer Langnamen ermitteln
    const String& GetShortName( sal_uInt16 ) const; // Kurzname fuer Index zurueck
    const String& GetLongName( sal_uInt16 ) const;  // Langname fuer Index zurueck
    const String& GetPackageName( sal_uInt16 ) const;   // Langname fuer Index zurueck

    const String& GetFileName() const {return aFile;}   // phys. Dateinamen liefern
    void SetName( const String& rName )                 // logic name
        { aName = rName; bInfoChanged = sal_True; }
    const String & GetName( void )
        { return aName; }

    const String&       GetBaseURL() const { return sBaseURL;}
    void                SetBaseURL( const String& rURL ) { sBaseURL = rURL; }

    virtual sal_uLong Delete( sal_uInt16 ) = 0;
    virtual sal_uLong Rename( sal_uInt16, const String&, const String& ) = 0;
    virtual sal_uLong CopyBlock( SwImpBlocks& rImp, String& rShort, const String& rLong) = 0;
    virtual sal_uLong GetDoc( sal_uInt16 ) = 0;
    virtual sal_uLong GetDocForConversion( sal_uInt16 );
    virtual sal_uLong BeginPutDoc( const String&, const String& ) = 0;
    virtual sal_uLong PutDoc() = 0;
    virtual sal_uLong GetText( sal_uInt16, String& ) = 0;
    virtual sal_uLong PutText( const String&, const String&, const String& ) = 0;
    virtual sal_uLong MakeBlockList() = 0;

    virtual sal_uLong OpenFile( sal_Bool bReadOnly = sal_True ) = 0;
    virtual void  CloseFile() = 0;

    virtual sal_Bool IsOnlyTextBlock( const String& rShort ) const;

    virtual sal_uLong GetMacroTable( sal_uInt16 nIdx, SvxMacroTableDtor& rMacroTbl,
                                 sal_Bool bFileAlreadyOpen = sal_False );
    virtual sal_uLong SetMacroTable( sal_uInt16 nIdx,
                                 const SvxMacroTableDtor& rMacroTbl,
                                 sal_Bool bFileAlreadyOpen = sal_False );
    virtual sal_Bool PutMuchEntries( sal_Bool bOn );
};

/*
class Sw3Persist : public SvPersist
{
    virtual void FillClass( SvGlobalName * pClassName,
                            sal_uLong * pClipFormat,
                            String * pAppName,
                            String * pLongUserName,
                            String * pUserName,
                            sal_Int32 nFileFormat=SOFFICE_FILEFORMAT_CURRENT ) const;
    virtual sal_Bool Save();
    virtual sal_Bool SaveCompleted( SvStorage * );
public:
    Sw3Persist();
};

class SwSwgReader;

class Sw2TextBlocks : public SwImpBlocks
{
    SvPersistRef refPersist;            // Fuer OLE-Objekte
    SwSwgReader* pRdr;                  // Lese-Routinen
    SfxMedium* pMed;                    // der logische Input-Stream
    String* pText;                      // String fuer GetText()
    long   nDocStart;                   // Beginn des Doc-Records
    long   nDocSize;                    // Laenge des Doc-Records
    long   nStart;                      // Beginn des CONTENTS-Records
    long   nSize;                       // Laenge des CONTENTS-Records
    sal_uInt16 nNamedFmts;                  // benannte Formate
    sal_uInt16 nColls;                      // Text-Collections
    sal_uInt16 nBlks;                       // Anzahl Elemente im CONTENTS-Record
public:
    Sw2TextBlocks( const String& );
    virtual ~Sw2TextBlocks();
    virtual sal_uLong Delete( sal_uInt16 );
    virtual sal_uLong Rename( sal_uInt16, const String&, const String& );
    virtual sal_uLong CopyBlock( SwImpBlocks& rImp, String& rShort, const String& rLong);
    virtual sal_uLong GetDoc( sal_uInt16 );
    virtual sal_uLong BeginPutDoc( const String&, const String& );
    virtual sal_uLong PutDoc();
    virtual sal_uLong GetText( sal_uInt16, String& );
    virtual sal_uLong PutText( const String&, const String&, const String& );
    virtual sal_uLong MakeBlockList();
    virtual short GetFileType( ) const;
    sal_uLong LoadDoc();

    virtual sal_uLong OpenFile( sal_Bool bReadOnly = sal_True );
    virtual void  CloseFile();

    void StatLineStartPercent();                // zum Anzeigen des Prozessbars
};

class Sw3Io;
class Sw3IoImp;

class Sw3TextBlocks : public SwImpBlocks
{
    Sw3Io*       pIo3;
    Sw3IoImp*    pImp;
    sal_Bool         bAutocorrBlock;

public:
    Sw3TextBlocks( const String& );
    Sw3TextBlocks( SvStorage& );
    virtual ~Sw3TextBlocks();
    virtual sal_uLong Delete( sal_uInt16 );
    virtual sal_uLong Rename( sal_uInt16, const String&, const String& );
    virtual sal_uLong CopyBlock( SwImpBlocks& rImp, String& rShort, const String& rLong);
    virtual sal_uLong GetDoc( sal_uInt16 );
    virtual sal_uLong GetDocForConversion( sal_uInt16 );
    virtual sal_uLong BeginPutDoc( const String&, const String& );
    virtual sal_uLong PutDoc();
    virtual void SetDoc( SwDoc * pNewDoc);
    virtual sal_uLong GetText( sal_uInt16, String& );
    virtual sal_uLong PutText( const String&, const String&, const String& );
    virtual sal_uLong MakeBlockList();
    virtual short GetFileType( ) const;

    virtual sal_uLong OpenFile( sal_Bool bReadOnly = sal_True );
    virtual void  CloseFile();

    // Methoden fuer die neue Autokorrektur
    sal_uLong GetText( const String& rShort, String& );
    SwDoc* GetDoc() const { return pDoc; }

    virtual sal_Bool IsOnlyTextBlock( const String& rShort ) const;

    virtual sal_uLong GetMacroTable( sal_uInt16, SvxMacroTableDtor& rMacroTbl,
                                 sal_Bool bFileAlreadyOpen = sal_False );
    virtual sal_uLong SetMacroTable( sal_uInt16 nIdx,
                                 const SvxMacroTableDtor& rMacroTbl,
                                 sal_Bool bFileAlreadyOpen = sal_False );

    void ReadInfo();
};
*/
#endif
