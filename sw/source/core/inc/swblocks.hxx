/*************************************************************************
 *
 *  $RCSfile: swblocks.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:08:21 $
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
#ifndef _SWBLOCKS_HXX
#define _SWBLOCKS_HXX

#ifndef _DATE_HXX //autogen
#include <tools/date.hxx>
#endif
#ifndef _PERSIST_HXX //autogen
#include <so3/persist.hxx>
#endif
#ifndef _SVARRAY_HXX //autogen
#include <svtools/svarray.hxx>
#endif

class SwPaM;
class SwDoc;
class SvxMacroTableDtor;

// Name eines Textblocks:

class SwBlockName
{
    friend class SwImpBlocks;
    friend class Sw2TextBlocks;
    USHORT nHashS, nHashL;              // Hash-Codes zum Checken
    long   nPos;                        // Dateiposition (SW2-Format)
public:
    String aShort;                      // Kurztext
    String aLong;                       // Langtext
    BOOL bIsOnlyTxtFlagInit : 1;        // ist das Flag gueltig?
    BOOL bIsOnlyTxt : 1;                // Text ohne Formatierung
    BOOL bInPutMuchBlocks : 1;          // put serveral block entries

    SwBlockName( const String& rShort, const String& rLong, long n );

    // fuer das Einsortieren in das Array
    int operator==( const SwBlockName& r ) { return aShort == r.aShort; }
    int operator< ( const SwBlockName& r ) { return aShort <  r.aShort; }
};

SV_DECL_PTRARR_SORT( SwBlockNames, SwBlockName*, 10, 10 );


class SwImpBlocks
{
    friend class SwTextBlocks;
protected:
    String aFile;                       // physikalischer Dateiname
    String aName;                       // logischer Name der Datei
    String aCur;                        // aktueller Text
    String aShort, aLong;               // Kurz- und Langname (PutDoc)
    SwBlockNames aNames;                // Liste aller Bausteine
    Date aDateModified;                 // fuers abgleichen bei den Aktionen
    Time aTimeModified;
    SwDoc* pDoc;                        // Austauschdokument
    USHORT nCur;                        // aktueller Index
    BOOL bReadOnly : 1;
    BOOL bInPutMuchBlocks : 1;          // put serveral block entries
    BOOL bInfoChanged : 1;              // any Info of TextBlock is changed

    SwImpBlocks( const String&, BOOL = FALSE );
    virtual ~SwImpBlocks();

    static short GetFileType( const String& );
#define SWBLK_NO_FILE   0               // nicht da
#define SWBLK_NONE      1               // keine TB-Datei
#define SWBLK_SW2       2               // SW2-Datei
#define SWBLK_SW3       3               // SW3-Datei

    void   ClearDoc();                  // Doc-Inhalt loeschen
    SwPaM* MakePaM();                   // PaM ueber Doc aufspannen
    void   AddName( const String&, const String&, BOOL bOnlyTxt = FALSE );
    BOOL   IsFileChanged() const;
    void   Touch();

public:
    static USHORT Hash( const String& );        // Hashcode fuer Blocknamen
    USHORT GetCount() const;                    // Anzahl Textbausteine ermitteln
    USHORT GetIndex( const String& ) const;     // Index fuer Kurznamen ermitteln
    USHORT GetLongIndex( const String& ) const; //Index fuer Langnamen ermitteln
    const String& GetShortName( USHORT ) const; // Kurzname fuer Index zurueck
    const String& GetLongName( USHORT ) const;  // Langname fuer Index zurueck

    const String& GetFileName() const {return aFile;}   // phys. Dateinamen liefern
    void SetName( const String& rName )                 // logic name
        { aName = rName; bInfoChanged = TRUE; }

    virtual BOOL IsOld() const = 0;

    virtual ULONG Delete( USHORT ) = 0;
    virtual ULONG Rename( USHORT, const String&, const String& ) = 0;
    virtual ULONG CopyBlock( SwImpBlocks& rImp, String& rShort, const String& rLong) = 0;
    virtual ULONG GetDoc( USHORT ) = 0;
    virtual ULONG BeginPutDoc( const String&, const String& ) = 0;
    virtual ULONG PutDoc() = 0;
    virtual ULONG GetText( USHORT, String& ) = 0;
    virtual ULONG PutText( const String&, const String&, const String& ) = 0;
    virtual ULONG MakeBlockList() = 0;

    virtual ULONG OpenFile( BOOL bReadOnly = TRUE ) = 0;
    virtual void  CloseFile() = 0;

    virtual BOOL IsOnlyTextBlock( const String& rShort ) const;

    virtual ULONG GetMacroTable( USHORT nIdx, SvxMacroTableDtor& rMacroTbl );
    virtual BOOL PutMuchEntries( BOOL bOn );
};

class Sw3Persist : public SvPersist
{
    virtual void FillClass( SvGlobalName * pClassName,
                            ULONG * pClipFormat,
                            String * pAppName,
                            String * pLongUserName,
                            String * pUserName,
                            long nFileFormat=SOFFICE_FILEFORMAT_NOW ) const;
    virtual BOOL Save();
    virtual BOOL SaveCompleted( SvStorage * );
public:
    Sw3Persist();
};

class SwSwgReader;

class Sw2TextBlocks : public SwImpBlocks
{
    SvPersistRef refPersist;            // Fuer OLE-Objekte
    SwSwgReader* pRdr;                  // Lese-Routinen
    SvStream* pFstrm;                   // der logische Input-Stream
    String* pText;                      // String fuer GetText()
    long   nDocStart;                   // Beginn des Doc-Records
    long   nDocSize;                    // Laenge des Doc-Records
    long   nStart;                      // Beginn des CONTENTS-Records
    long   nSize;                       // Laenge des CONTENTS-Records
    USHORT nNamedFmts;                  // benannte Formate
    USHORT nColls;                      // Text-Collections
    USHORT nBlks;                       // Anzahl Elemente im CONTENTS-Record
public:
    Sw2TextBlocks( const String& );
    virtual ~Sw2TextBlocks();
    virtual BOOL   IsOld() const;
    virtual ULONG Delete( USHORT );
    virtual ULONG Rename( USHORT, const String&, const String& );
    virtual ULONG CopyBlock( SwImpBlocks& rImp, String& rShort, const String& rLong);
    virtual ULONG GetDoc( USHORT );
    virtual ULONG BeginPutDoc( const String&, const String& );
    virtual ULONG PutDoc();
    virtual ULONG GetText( USHORT, String& );
    virtual ULONG PutText( const String&, const String&, const String& );
    virtual ULONG MakeBlockList();
    ULONG LoadDoc();

    virtual ULONG OpenFile( BOOL bReadOnly = TRUE );
    virtual void  CloseFile();

    void StatLineStartPercent();                // zum Anzeigen des Prozessbars
};

class Sw3Io;
class Sw3IoImp;

class Sw3TextBlocks : public SwImpBlocks
{
    Sw3Io*       pIo3;
    Sw3IoImp*    pImp;
    BOOL         bAutocorrBlock;

public:
    Sw3TextBlocks( const String& );
    Sw3TextBlocks( SvStorage& );
    virtual ~Sw3TextBlocks();
    virtual BOOL   IsOld() const;
    virtual ULONG Delete( USHORT );
    virtual ULONG Rename( USHORT, const String&, const String& );
    virtual ULONG CopyBlock( SwImpBlocks& rImp, String& rShort, const String& rLong);
    virtual ULONG GetDoc( USHORT );
    virtual ULONG BeginPutDoc( const String&, const String& );
    virtual ULONG PutDoc();
    virtual ULONG GetText( USHORT, String& );
    virtual ULONG PutText( const String&, const String&, const String& );
    virtual ULONG MakeBlockList();

    virtual ULONG OpenFile( BOOL bReadOnly = TRUE );
    virtual void  CloseFile();

    ULONG SetConvertMode( BOOL );

    // Methoden fuer die neue Autokorrektur
    ULONG GetText( const String& rShort, String& );
    SwDoc* GetDoc() const { return pDoc; }

    virtual BOOL IsOnlyTextBlock( const String& rShort ) const;

    virtual ULONG GetMacroTable( USHORT, SvxMacroTableDtor& rMacroTbl );
    virtual BOOL PutMuchEntries( BOOL bOn );

    void ReadInfo();
    void WriteInfo();
};

#endif
