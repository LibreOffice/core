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
#ifndef _SWBLOCKS_HXX
#define _SWBLOCKS_HXX

#include <tools/string.hxx>

#include <tools/datetime.hxx>
#include <o3tl/sorted_vector.hxx>


class SwPaM;
class SwDoc;
class SvxMacroTableDtor;

// Name eines Textblocks:

class SwBlockName
{
    friend class SwImpBlocks;
    sal_uInt16 nHashS, nHashL;          // Hash-Codes zum Checken
public:
    String aShort;                      // Short name
    String aLong;                       // Long name
    OUString aPackageName;         // Package name
    sal_Bool bIsOnlyTxtFlagInit : 1;        // ist das Flag gueltig?
    sal_Bool bIsOnlyTxt : 1;                // unformatted text
    sal_Bool bInPutMuchBlocks : 1;          // put serveral block entries

    SwBlockName( const String& rShort, const String& rLong );
    SwBlockName( const String& rShort, const String& rLong, const String& rPackageName );

    // fuer das Einsortieren in das Array
    bool operator==( const SwBlockName& r ) const { return aShort == r.aShort; }
    bool operator< ( const SwBlockName& r ) const { return aShort <  r.aShort; }
};

class SwBlockNames : public o3tl::sorted_vector<SwBlockName*, o3tl::less_ptr_to<SwBlockName> > {};


class SwImpBlocks
{
    friend class SwTextBlocks;
protected:
    String aFile;                       // physikalischer Dateiname
    String aName;                       // logischer Name der Datei
    OUString aCur;                      // aktueller Text
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
    OUString GetPackageName( sal_uInt16 ) const;   // Langname fuer Index zurueck

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
    virtual sal_uLong BeginPutDoc( const String&, const String& ) = 0;
    virtual sal_uLong PutDoc() = 0;
    virtual sal_uLong GetText( sal_uInt16, OUString& ) = 0;
    virtual sal_uLong PutText( const String&, const String&, const OUString& ) = 0;
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

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
