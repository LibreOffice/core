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
#ifndef _SWBLOCKS_HXX
#define _SWBLOCKS_HXX

#include <tools/string.hxx>

#include <tools/datetime.hxx>
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
    USHORT nHashS, nHashL;              // Hash-Codes zum Checken
    long   nPos;                        // Dateiposition (SW2-Format)
public:
    String aShort;                      // Short name
    String aLong;                       // Long name
    String aPackageName;                // Package name
    BOOL bIsOnlyTxtFlagInit : 1;        // ist das Flag gueltig?
    BOOL bIsOnlyTxt : 1;                // unformatted text
    BOOL bInPutMuchBlocks : 1;          // put serveral block entries

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
    USHORT nCur;                        // aktueller Index
    BOOL bReadOnly : 1;
    BOOL bInPutMuchBlocks : 1;          // put serveral block entries
    BOOL bInfoChanged : 1;              // any Info of TextBlock is changed

    SwImpBlocks( const String&, BOOL = FALSE );
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
    virtual void   AddName( const String&, const String&, BOOL bOnlyTxt = FALSE );
    BOOL   IsFileChanged() const;
    void   Touch();

public:
    static USHORT Hash( const String& );        // Hashcode fuer Blocknamen
    USHORT GetCount() const;                    // Anzahl Textbausteine ermitteln
    USHORT GetIndex( const String& ) const;     // Index fuer Kurznamen ermitteln
    USHORT GetLongIndex( const String& ) const; //Index fuer Langnamen ermitteln
    const String& GetShortName( USHORT ) const; // Kurzname fuer Index zurueck
    const String& GetLongName( USHORT ) const;  // Langname fuer Index zurueck
    const String& GetPackageName( USHORT ) const;   // Langname fuer Index zurueck

    const String& GetFileName() const {return aFile;}   // phys. Dateinamen liefern
    void SetName( const String& rName )                 // logic name
        { aName = rName; bInfoChanged = TRUE; }
    const String & GetName( void )
        { return aName; }

    const String&       GetBaseURL() const { return sBaseURL;}
    void                SetBaseURL( const String& rURL ) { sBaseURL = rURL; }

    virtual ULONG Delete( USHORT ) = 0;
    virtual ULONG Rename( USHORT, const String&, const String& ) = 0;
    virtual ULONG CopyBlock( SwImpBlocks& rImp, String& rShort, const String& rLong) = 0;
    virtual ULONG GetDoc( USHORT ) = 0;
    virtual ULONG GetDocForConversion( USHORT );
    virtual ULONG BeginPutDoc( const String&, const String& ) = 0;
    virtual ULONG PutDoc() = 0;
    virtual ULONG GetText( USHORT, String& ) = 0;
    virtual ULONG PutText( const String&, const String&, const String& ) = 0;
    virtual ULONG MakeBlockList() = 0;

    virtual ULONG OpenFile( BOOL bReadOnly = TRUE ) = 0;
    virtual void  CloseFile() = 0;

    virtual BOOL IsOnlyTextBlock( const String& rShort ) const;

    virtual ULONG GetMacroTable( USHORT nIdx, SvxMacroTableDtor& rMacroTbl,
                                 sal_Bool bFileAlreadyOpen = sal_False );
    virtual ULONG SetMacroTable( USHORT nIdx,
                                 const SvxMacroTableDtor& rMacroTbl,
                                 sal_Bool bFileAlreadyOpen = sal_False );
    virtual BOOL PutMuchEntries( BOOL bOn );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
