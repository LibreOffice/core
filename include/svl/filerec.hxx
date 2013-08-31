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

#ifndef _SFXFILEREC_HXX
#define _SFXFILEREC_HXX

#include "svl/svldllapi.h"
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <vector>

#define SFX_REC_PRETAG_EXT              sal_uInt8(0x00) // Pre-Tag f"ur Extended-Records
#define SFX_REC_PRETAG_EOR              sal_uInt8(0xFF) // Pre-Tag f"ur End-Of-Records

#define SFX_REC_TYPE_NONE               sal_uInt8(0x00) // unbekannter Record-Typ
#define SFX_REC_TYPE_FIRST              sal_uInt8(0x01)
#define SFX_REC_TYPE_SINGLE             sal_uInt8(0x01) // Single-Content-Record
#define SFX_REC_TYPE_FIXSIZE            sal_uInt8(0x02) // Fix-Size-Multi-Content-Record
#define SFX_REC_TYPE_VARSIZE_RELOC      sal_uInt8(0x03) // variable Rec-Size
#define SFX_REC_TYPE_VARSIZE            sal_uInt8(0x04) // alt (nicht verschiebbar)
#define SFX_REC_TYPE_MIXTAGS_RELOC      sal_uInt8(0x07) // Mixed Tag Content-Record
#define SFX_REC_TYPE_MIXTAGS            sal_uInt8(0x08) // alt (nicht verschiebbar)
#define SFX_REC_TYPE_LAST               sal_uInt8(0x08)
#define SFX_REC_TYPE_MINI                   0x100   // Mini-Record
#define SFX_REC_TYPE_DRAWENG                0x400   // Drawing-Engine-Record
#define SFX_REC_TYPE_EOR                    0xF00   // End-Of-Records

#define SFX_REC_HEADERSIZE_MINI     4   // Gr"o\se des Mini-Record-Headers
#define SFX_REC_HEADERSIZE_SINGLE   4   // zzgl. HEADERSIZE_MINI => 8
#define SFX_REC_HEADERSIZE_MULTI    6   // zzgl. HEADERSIZE_SINGLE => 14

#ifndef DBG
#ifdef DBG_UTIL
#define DBG(x) x
#else
#define DBG(x)
#endif
#endif

/*  [Fileformat]

    Jeder Record beginnt mit einem Byte, dem sogenannten 'Pre-Tag'.

    Ist dieses 'Pre-Tag' == 0x00, dann handelt es sich um einen Extended-
    Record, dessen Typ durch ein weiteres Byte an Position 5 n?her
    beschrieben wird:

    0x01:       SfxSingleRecord
    0x02:       SfxMultiFixRecord
    0x03+0x04:  SfxMultiVarRecord
    0x07+0x08:  SfxMultiMixRecord
    (Alle weiteren Record-Typ-Kennungen sind reserviert.)

    I.d.R. werden File-Formate schon aus Performance-Gr"unden so aufgebaut,
    da\s beim Lesen jeweils vorher schon feststeht, welcher Record-Typ
    vorliegt. Diese Kennung dient daher hautps"achlich der "Uberpr"ufung
    und File-Viewern, die das genaue File-Format (unterhalb der Records)
    nicht kennen.

    Der 'SfxMiniRecordReader' verf"ugt dazu auch "uber eine statische
    Methode 'ScanRecordType()', mit der festgestellt werden kann, welcher
    Record-Typ in dem "ubergebenen Stream zu finden ist.

    Ein 'Pre-Tag' mit dem Wert 0xFF ist als Terminator reserviert.
    Terminatoren werden verwendet, um das Suchen nach einem speziellen
    Record zu terminieren, d.h. ist er bis dorthin nicht gefunden, wird
    auch nicht weitergesucht.

    Bei allen anderen Werten des 'Pre-Tags' (also von 0x01 bis 0xFE)
    handelt es sich um einen zum SW3 kompatbilen Record, der hier
    'SfxMiniRecord' genannt wird, er kann daher mit einem <SfxMiniRecordReader>
    gelesen werden.

    Beginnt ein Record mit 0x44 k"onnte es sich um einen Drawing-Engine-
    Record handeln. Dies ist dann der Fall, wenn die folgenden drei Bytes
    die Zeichenkette 'RMD' bzw. 'RVW' ergeben (zusammen mit 'D'==0x44
    ergibt dies die K"urzel f"ur 'DRaw-MoDel' bzw. 'DRaw-VieW'). Records
    dieser Art k"onnen von den hier dargestellten Klassen weder gelesen,
    noch in irgendeiner Weise interpretiert werden. Einzig die Methode
    'ScanRecordType()' kann sie erkennen - weitere Behandlung obliegt
    jedoch der Anwendungsprogrammierung.

    Diese drei Bytes an den Positionen 2 bis 4 enthalten normalerweise
    die Gr"o\se des Records ohne Pre-Tag und Gr"o\sen-Bytes selbst,
    also die Restgr"o\se nach diesem 4-Byte-Header.

        Struktur des Mini-Records:

                            1 sal_uInt8         Pre-Tag
                            3 sal_uInt8         OffsetToEndOfRec
        OffsetToEndOfRec*   1 sal_uInt8         Content

    Bei den Extended-Reords folgt auf diesen 4-Byte-Header ein erweiterter
    Header, der zun"achst den o.g. Record-Typ, dann eine Versions-Kennung
    sowie ein Tag enth"alt, welches den Inhalt kennzeichnet.

        Struktur des Extended-Records:

                            1 sal_uInt8         Pre-Tag (==0x00)
                            3 sal_uInt8         OffsetToEndOfRec
        OffsetToEndOfRec*   1 sal_uInt8         Content
                            1 sal_uInt8         Record-Type
                            1 sal_uInt8         Version
                            2 sal_uInt8         Tag
        ContentSize*        1 sal_uInt8         Content

        (ContentSize = OffsetToEndOfRec - 8)

    [Anmerkung]

    Der Aufbau der Records wird wie folgt begr"undet:

    Der SW-Record-Typ war zuerst vorhanden, mu\ste also 1:1 "ubernommen
    werden. Zum Gl"uck wurden einige Record-Tags nicht verwendet, (Z.B.
    0x00 und 0xFF).
    =>  1. Byte 0x00 kann als Kennung f"ur erweiterten Record verwendet werden
    =>  1. Byte 0xFF kann f"ur besondere Zwecke verwendet werden

    Egal welcher Record-Typ vorliegt, sollte eine Erkennung des Typs, ein
    Auslesen des Headers und ein "uberpspringen des Records m"oglich sein,
    ohne zu"uck-seeken zu m"ussen und ohne "uberfl"ussige Daten lesen zu
    m"ussen.
    =>  die Bytes 2-4 werden bei allen Records als Offset zum Ende des
        Records interpretiert, so da\s die Gesamt-Recors-Size sich wie
        folgt berechnet: sizeof(sal_uInt32) + OffsetToEndOfRec

    Die Records sollten einfach zu parsen un einheitlich aufgebaut sein.
    =>  Sie bauen aufeinander auf, so ist z.B. der SfxMiniRecord in jedem
        anderen enthalten.

    Die Records sollten auch von denen der Drawing Enginge unterscheidbar
    sein. Diese beginnen mit 'DRMD' und 'DRVW'.
    =>  Mini-Records mit dem Pre-Tag 'D' d"urfen maximal 4MB gro\s sein,
        um nicht in diesen Kennungs-Bereich zu reichen.

    [Erweiterungen]

    Es ist geplant das File-Format so zu erweitern, da\s das High-Nibble
    des Record-Typs der erweiterten Records besondere Aufgaben "ubernehmen
    soll. Zum Beispiel ist geplant, Record-Contents als 'nur aus Records
    bestehend' zu kennzeichnen. Ein File-Viewer k"onnte sich dann automatisch
    durch solche Strukturen 'hangeln', ohne Gefahr zu laufen, auf Daten
    zu sto\sen, die sich zwar als Records interpretieren lassen, aber
    tats"achlis als 'flache' Daten geschrieben wurden. Die m"ogliche
    Erweiterung wird schon jetzt insofern vorbereitet, als da\s das
    High-Nibble des Typs bei Vergleichen nicht ber"ucksichtigt wird.
*/

/*  [Beschreibung]

    Mit Instanzen dieser Klasse kann ein einfacher Record in einen Stream
    geschrieben werden, der sich durch ein sal_uInt8-Tag identifiziert, sowie
    seine eigene L"ange speichert und somit auch von "alteren Versionen
    bzw. Readern, die diesen Record-Type (Tag) nicht kennen, "ubersprungen
    werden kann. Es wird keine Version-Nummer gespeichert.

    Alternativ kann die Gr"o\se fest angegeben werden oder sie wird
    automatisch aus der Differenz der Tell()-Angaben vor und nach dem
    Streamen des Inhalts ermittelt.

    Um Auf- und Abw"artskompatiblit"at gew"ahrleisten zu k"onnen, m"ussen
    neue Versionen die Daten der "alteren immer komplett enthalten,
    es d"urfen allenfalls neue Daten hintenan geh"angt werden!

    [Fileformat]

    1*              sal_uInt8       Content-Tag (!= 0)
    1*              3-sal_uInt8     OffsetToEndOfRec in Bytes
    SizeOfContent*  sal_uInt8       Content

    [Beispiel]

    {
        SfxMiniRecordWriter aRecord( pStream, MY_TAG_X );
        *aRecord << aMember1;
        *aRecord << aMember2;
    }
*/
class SVL_DLLPUBLIC SfxMiniRecordWriter
{
protected:
    SvStream*       _pStream;   //  <SvStream>, in dem der Record liegt
    sal_uInt32          _nStartPos; //  Start-Position des Gesamt-Records im Stream
    bool             _bHeaderOk; /* TRUE, wenn der Header schon geschrieben ist; */
    sal_uInt8           _nPreTag;   //  in den Header zu schreibendes 'Pre-Tag'

public:
    inline          SfxMiniRecordWriter( SvStream *pStream,
                                         sal_uInt8 nTag );
    inline          SfxMiniRecordWriter( SvStream *pStream, sal_uInt8 nTag,
                                         sal_uInt32 nSize );

    inline          ~SfxMiniRecordWriter();

    inline SvStream& operator*() const;

    inline void     Reset();

    sal_uInt32          Close( bool bSeekToEndOfRec = true );

private:
                    // not implementend, not allowed
                    SfxMiniRecordWriter( const SfxMiniRecordWriter& );
    SfxMiniRecordWriter& operator=(const SfxMiniRecordWriter&);
};

/*  [Beschreibung]

    Mit Instanzen dieser Klasse kann ein einfacher Record aus einem Stream
    gelesen werden, der mit der Klasse <SfxRecordWriter> geschrieben wurde.

    Es ist auch m"oglich, den Record zu "uberspringen, ohne sein internes
    Format zu kennen.

    [Beispiel]

    {
        SfxMiniRecordReader aRecord( pStream );
        switch ( aRecord.GetTag() )
        {
            case MY_TAG_X:
                *aRecord >> aMember1;
                *aRecord >> aMember2;
                break;

            ...
        }
    }
*/
class SVL_DLLPUBLIC SfxMiniRecordReader
{
protected:
    SvStream*           _pStream;   //  <SvStream>, aus dem gelesen wird
    sal_uInt32              _nEofRec;   //  Position direkt hinter dem Record
    bool                _bSkipped;  //  TRUE: der Record wurde explizit geskippt
    sal_uInt8               _nPreTag;   //  aus dem Header gelesenes Pre-Tag

                        // Drei-Phasen-Ctor f"ur Subklassen
                        SfxMiniRecordReader() {}
   void                 Construct_Impl( SvStream *pStream, sal_uInt8 nTag )
                        {
                            _pStream = pStream;
                            _bSkipped = sal_False;
                            _nPreTag = nTag;
                        }
    inline bool         SetHeader_Impl( sal_uInt32 nHeader );

                        // als ung"ultig markieren und zur"uck-seeken
    void                SetInvalid_Impl( sal_uInt32 nRecordStartPos )
                        {
                            _nPreTag = SFX_REC_PRETAG_EOR;
                            _pStream->Seek( nRecordStartPos );
                        }

public:
    SfxMiniRecordReader( SvStream *pStream, sal_uInt8 nTag );
    inline              ~SfxMiniRecordReader();

    inline sal_uInt8        GetTag() const;
    inline bool         IsValid() const;

    inline SvStream&    operator*() const;

    inline void         Skip();

private:
                        // not implementend, not allowed
                        SfxMiniRecordReader( const SfxMiniRecordReader& );
    SfxMiniRecordReader& operator=(const SfxMiniRecordReader&);
};

/*  [Beschreibung]

    Mit Instanzen dieser Klasse kann ein Record in einen Stream geschrieben
    werden, dessen einziger Inhalt sich durch ein sal_uInt16-Tag und eine
    sal_uInt8-Versions-Nummer identifiziert, sowie seine eigene L"ange speichert
    und somit auch von "alteren Versionen bzw. Readern, die diesen
    Record-Type (Tag) nicht kennen, "ubersprungen werden kann.

    Alternativ kann die Gr"o\se fest angegeben werden oder sie wird
    automatisch aus der Differenz der Tell()-Angaben vor und nach dem
    Streamen des Inhalts ermittelt.

    Um Auf- und Abw"artskompatiblit"at gew"ahrleisten zu k"onnen, m"ussen
    neue Versionen die Daten der "alteren immer komplett enthalten,
    es d"urfen allenfalls neue Daten hintenan geh"angt werden!

    [Fileformat]

    1*              sal_uInt8       Pre-Tag (!= 0)
    1*              3-sal_uInt8     OffsetToEndOfRec in Bytes
    1*              sal_uInt8       Record-Type (==SFX_REC_TYPE_SINGLE)
    1*              sal_uInt8       Content-Version
    1*              sal_uInt16      Content-Tag
    SizeOfContent*  sal_uInt8       Content
*/
class SVL_DLLPUBLIC SfxSingleRecordWriter: public SfxMiniRecordWriter
{
protected:
                    SfxSingleRecordWriter( sal_uInt8 nRecordType,
                                           SvStream *pStream,
                                           sal_uInt16 nTag, sal_uInt8 nCurVer );

public:
    inline void     Reset();

    sal_uInt32          Close( bool bSeekToEndOfRec = true );
};

/*  [Beschreibung]

    Mit Instanzen dieser Klasse kann ein einfacher Record aus einem Stream
    gelesen werden, der mit der Klasse <SfxSingleRecordWriter> geschrieben
    wurde.

    Es ist auch m"oglich, den Record zu "uberspringen, ohne sein internes
    Format zu kennen.
*/
class SVL_DLLPUBLIC SfxSingleRecordReader: public SfxMiniRecordReader
{
protected:
    sal_uInt16              _nRecordTag;    // Art des Gesamt-Inhalts
    sal_uInt8               _nRecordVer;    // Version des Gesamt-Inhalts
    sal_uInt8               _nRecordType;   // Record Type aus dem Header

                        // Drei-Phasen-Ctor f"ur Subklassen
                        SfxSingleRecordReader() {}
    void                Construct_Impl( SvStream *pStream )
                        {
                            SfxMiniRecordReader::Construct_Impl(
                                    pStream, SFX_REC_PRETAG_EXT );
                        }
    bool                FindHeader_Impl( sal_uInt16 nTypes, sal_uInt16 nTag );
    bool                ReadHeader_Impl( sal_uInt16 nTypes );

public:

    inline sal_uInt16       GetTag() const;

    inline sal_uInt8        GetVersion() const;
    inline bool         HasVersion( sal_uInt16 nVersion ) const;
};

/*  [Beschreibung]

    Mit Instanzen dieser Klasse kann ein Record in einen Stream geschrieben
    werden, der seine eigene L"ange speichert und somit auch von "alteren
    Versionen bzw. Readern, die diesen Record-Type (Tag) nicht kennen,
    "ubersprungen werden kann.

    Er enth"alt mehrere Inhalte von demselben Typ (Tag) und derselben
    Version, die einmalig (stellvertretend f"ur alle) im Header des Records
    identifiziert werden. Alle Inhalte haben eine vorher bekannte und
    identische L"ange.

    Um Auf- und Abw"artskompatiblit"at gew"ahrleisten zu k"onnen, m"ussen
    neue Versionen die Daten der "alteren immer komplett enthalten,
    es d"urfen allenfalls neue Daten hinten angeh"angt werden! Hier sind
    damit selbstverst"andlich nur die Daten der einzelnen Inhalte gemeint,
    die Anzahl der Inhalte ist selbstverst"andlich variabel und sollte
    von lesenden Applikationen auch so behandelt werden.

    [Fileformat]

    1*                  sal_uInt8       Pre-Tag (==0)
    1*                  3-sal_uInt8     OffsetToEndOfRec in Bytes
    1*                  sal_uInt8       Record-Type (==SFX_REC_TYPE_FIXSIZE)
    1*                  sal_uInt8       Content-Version
    1*                  sal_uInt16      Content-Tag
    1*                  sal_uInt16      NumberOfContents
    1*                  sal_uInt32      SizeOfEachContent
    NumberOfContents*   (
    SizeOfEachContent   sal_uInt8       Content
                        )

    [Beispiel]

    {
        SfxMultiFixRecordWriter aRecord( pStream, MY_TAG_X, MY_VERSION );
        for ( sal_uInt16 n = 0; n < Count(); ++n )
        {
            aRecord.NewContent();
            *aRecord << aMember1[n];
            *aRecord << aMember2[n];
        }
    }
*/
class SVL_DLLPUBLIC SfxMultiFixRecordWriter: public SfxSingleRecordWriter
{
protected:
    sal_uInt32          _nContentStartPos;  /*  Startposition des jeweiligen
                                            Contents - nur bei DBG_UTIL
                                            und f"ur Subklassen */
    sal_uInt32          _nContentSize;      //  Gr"o\se jedes Contents
    sal_uInt16          _nContentCount;     //  jeweilige Anzahl der Contents

                    SfxMultiFixRecordWriter( sal_uInt8 nRecordType,
                                             SvStream *pStream,
                                             sal_uInt16 nTag,
                                             sal_uInt8 nCurVer );

public:
    inline          ~SfxMultiFixRecordWriter();

    inline void     NewContent();

    inline void     Reset();

    sal_uInt32          Close( bool bSeekToEndOfRec = true );
};

/*  [Beschreibung]

    Mit Instanzen dieser Klasse kann ein Record in einen Stream geschrieben
    werden, der seine eigene L"ange speichert und somit auch von "alteren
    Versionen bzw. Readern, die diesen Record-Type (Tag) nicht kennen,
    "ubersprungen werden kann.

    Er enth"alt mehrere Inhalte von demselben Typ (Tag) und derselben
    Version, die einmalig (stellvertretend f"ur alle) im Header des Records
    identifiziert werden. Die L"ange f"ur jeden einzelnen Inhalt wird
    automatisch berechnet und gespeichert, so da\s auch einzelne Inhalte
    "ubersprungen werden k"onnen, ohne sie interpretieren zu m"ussen.

    Um Auf- und Abw"artskompatiblit"at gew"ahrleisten zu k"onnen, m"ussen
    neue Versionen die Daten der "alteren immer komplett enthalten,
    es d"urfen allenfalls neue Daten hinten angeh"angt werden!

    [Fileformat]

    1*                  sal_uInt8       Pre-Tag (==0)
    1*                  3-sal_uInt8     OffsetToEndOfRec in Bytes
    1*                  sal_uInt8       Record-Type (==SFX_FILETYPE_TYPE_VARSIZE)
    1*                  sal_uInt8       Content-Version
    1*                  sal_uInt16      Content-Tag
    1*                  sal_uInt16      NumberOfContents
    1*                  sal_uInt32      OffsetToOfsTable
    NumberOfContents*   (
    ContentSize*        sal_uInt8       Content
                        )
    NumberOfContents*   sal_uInt32      ContentOfs (je per <<8 verschoben)

    [Beispiel]

    {
        SfxMultiVarRecordWriter aRecord( pStream, MY_TAG_X, MY_VERSION );
        for ( sal_uInt16 n = 0; n < Count(); ++n )
        {
            aRecord.NewContent();
            *aRecord << aMember1[n];
            *aRecord << aMember2[n];
        }
    }
*/
class SVL_DLLPUBLIC SfxMultiVarRecordWriter: public SfxMultiFixRecordWriter
{
protected:
    std::vector<sal_uInt32> _aContentOfs;
    sal_uInt16              _nContentVer;   // nur f"ur SfxMultiMixRecordWriter

                        SfxMultiVarRecordWriter( sal_uInt8 nRecordType,
                                                 SvStream *pStream,
                                                 sal_uInt16 nRecordTag,
                                                 sal_uInt8 nRecordVer );

    void                FlushContent_Impl();

public:
                        SfxMultiVarRecordWriter( SvStream *pStream,
                                                 sal_uInt16 nRecordTag,
                                                 sal_uInt8 nRecordVer );
    virtual             ~SfxMultiVarRecordWriter();

    void                NewContent();

    virtual sal_uInt32      Close( bool bSeekToEndOfRec = true );
};

/*  [Beschreibung]

    Mit Instanzen dieser Klasse kann ein Record in einen Stream geschrieben
    werden, der seine eigene L"ange speichert und somit auch von "alteren
    Versionen bzw. Readern, die diesen Record-Type (Tag) nicht kennen,
    "ubersprungen werden kann.

    Er enth"alt mehrere Inhalte von demselben Typ (Tag) und derselben
    Version, die einmalig (stellvertretend f"ur alle) im Header des Records
    identifiziert werden. Alle Inhalte haben eine vorher bekannte und
    identische L"ange.

    Um Auf- und Abw"artskompatiblit"at gew"ahrleisten zu k"onnen, m"ussen
    neue Versionen die Daten der "alteren immer komplett enthalten,
    es d"urfen allenfalls neue Daten hinten angeh"angt werden!

    [Fileformat]

    1*                  sal_uInt8       Pre-Tag (==0)
    1*                  3-sal_uInt8     OffsetToEndOfRec in Bytes
    1*                  sal_uInt8       Record-Type (==SFX_REC_TYPE_MIXTAGS)
    1*                  sal_uInt8       Content-Version
    1*                  sal_uInt16      Record-Tag
    1*                  sal_uInt16      NumberOfContents
    1*                  sal_uInt32      OffsetToOfsTable
    NumberOfContents*   (
    1*                  sal_uInt16      Content-Tag
    ContentSize*        sal_uInt8        Content
                        )
    NumberOfContents*   sal_uInt32      ( ContentOfs << 8 + Version )
*/
class SVL_DLLPUBLIC SfxMultiMixRecordWriter: public SfxMultiVarRecordWriter
{
public:
    inline              SfxMultiMixRecordWriter( SvStream *pStream,
                                                 sal_uInt16 nRecordTag,
                                                 sal_uInt8 nRecordVer );

    void                NewContent( sal_uInt16 nTag, sal_uInt8 nVersion );

// private: geht nicht, da einige Compiler dann auch vorherige privat machen
    void                NewContent()
                        { OSL_FAIL( "NewContent() only allowed with args" ); }
};

/*  [Beschreibung]

    Mit Instanzen dieser Klasse kann ein aus mehreren Contents bestehender
    Record aus einem Stream gelesen werden, der mit einer der Klassen
    <SfxMultiFixRecordWriter>, <SfxMultiVarRecordWriter> oder
    <SfxMultiMixRecordWriter> geschrieben wurde.

    Es ist auch m"oglich, den Record oder einzelne Contents zu "uberspringen,
    ohne das jeweilis interne Format zu kennen.

    [Beispiel]

    {
        SfxMultiRecordReader aRecord( pStream );
        for ( sal_uInt16 nRecNo = 0; aRecord.GetContent(); ++nRecNo )
        {
            switch ( aRecord.GetTag() )
            {
                case MY_TAG_X:
                    X *pObj = new X;
                    *aRecord >> pObj.>aMember1;
                    if ( aRecord.HasVersion(2) )
                        *aRecord >> pObj->aMember2;
                    Append( pObj );
                    break;

                ...
            }
        }
    }
*/
class SVL_DLLPUBLIC SfxMultiRecordReader: public SfxSingleRecordReader
{
    sal_uInt32              _nStartPos;     //  Start-Position des Records
    sal_uInt32*             _pContentOfs;   //  Offsets der Startpositionen
    sal_uInt32              _nContentSize;  //  Size jedes einzelnen / Tabellen-Pos
    sal_uInt16              _nContentCount; //  Anzahl der Contents im Record
    sal_uInt16              _nContentNo;    /*  der Index des aktuellen Contents
                                            enth"alt jeweils den Index des
                                            Contents, der beim n"achsten
                                            GetContent() geholt wird */
    sal_uInt16              _nContentTag;   //  Art-Kennung des aktuellen Contents
    sal_uInt8               _nContentVer;   //  Versions-Kennung des akt. Contents

    bool                ReadHeader_Impl();

public:
                        SfxMultiRecordReader( SvStream *pStream, sal_uInt16 nTag );
                        ~SfxMultiRecordReader();

    bool                GetContent();
    inline sal_uInt16       GetContentTag();
    inline sal_uInt8        GetContentVersion() const;
    inline bool         HasContentVersion( sal_uInt16 nVersion ) const;

    inline sal_uInt32       ContentCount() const;
};

/*  [Beschreibung]

    Legt in 'pStream' einen 'SfxMiniRecord' an, dessen Content-Gr"o\se
    nicht bekannt ist, sondern nach dam Streamen des Contents errechnet
    werden soll.
*/
inline SfxMiniRecordWriter::SfxMiniRecordWriter
(
    SvStream*       pStream,        // Stream, in dem der Record angelegt wird
    sal_uInt8            nTag            // Record-Tag zwischen 0x01 und 0xFE
)
:   _pStream( pStream ),
    _nStartPos( pStream->Tell() ),
    _bHeaderOk(false),
    _nPreTag( nTag )
{
    DBG_ASSERT( _nPreTag != 0xFF, "invalid Tag" );
    DBG( DbgOutf( "SfxFileRec: writing record to %ul", pStream->Tell() ) );

    pStream->SeekRel( + SFX_REC_HEADERSIZE_MINI );
}

/*  [Beschreibung]

    Legt in 'pStream' einen 'SfxMiniRecord' an, dessen Content-Gr"o\se
    von vornherein bekannt ist.
*/
inline SfxMiniRecordWriter::SfxMiniRecordWriter
(
    SvStream*       pStream,        // Stream, in dem der Record angelegt wird
    sal_uInt8           nTag,           // Record-Tag zwischen 0x01 und 0xFE
    sal_uInt32          nSize           // Gr"o\se der Daten in Bytes
)
:   _pStream( pStream ),
    // _nTag( uninitialized ),
    // _nStarPos( uninitialized ),
    _bHeaderOk(true)
{
    DBG_ASSERT( nTag != 0 && nTag != 0xFF, "invalid Tag" );
    DBG(_nStartPos = pStream->Tell());
    DBG( DbgOutf( "SfxFileRec: writing record to %ul", _nStartPos ) );

    *pStream << ( ( nTag << 24 ) | nSize );
}

/*  [Beschreibung]

    Der Dtor der Klasse <SfxMiniRecordWriter> schlie\st den Record
    automatisch, falls <SfxMiniRecordWriter::Close()> nicht bereits
    explizit gerufen wurde.
*/
inline SfxMiniRecordWriter::~SfxMiniRecordWriter()
{
    // wurde der Header noch nicht geschrieben oder mu\s er gepr"uft werden
    if ( !_bHeaderOk )
        Close();
}

/*  [Beschreibung]

    Dieser Operator liefert den Stream, in dem der Record liegt.
    Der Record darf noch nicht geschlossen worden sein.
*/
inline SvStream& SfxMiniRecordWriter::operator*() const
{
    DBG_ASSERT( !_bHeaderOk, "getting Stream of closed record" );
    return *_pStream;
}

inline void SfxMiniRecordWriter::Reset()
{
    _pStream->Seek( _nStartPos + SFX_REC_HEADERSIZE_MINI );
    _bHeaderOk = false;
}

/*  [Beschreibung]

    Der Dtor der Klasse <SfxMiniRecordReader> positioniert den Stream
    automatisch auf die Position direkt hinter dem Record, falls nicht
    <SfxMiniRecordReader::Skip()> bereits explizit gerufen wurde.
*/
inline SfxMiniRecordReader::~SfxMiniRecordReader()
{
    // noch nicht explizit ans Ende gesprungen?
    if ( !_bSkipped )
        Skip();
}

/*  [Beschreibung]

    Mit dieser Methode wird der Stream direkt hinter das Ende des Records
    positioniert.
*/
inline void SfxMiniRecordReader::Skip()
{
    _pStream->Seek(_nEofRec);
    _bSkipped = sal_True;
}

/*  [Beschreibung]

    Liefert des aus dem Header gelesene Pre-Tag des Records. Dieses kann
    auch SFX_REC_PRETAG_EXT oder SFX_REC_PRETAG_EOR sein, im
    letzteren Fall ist am Stream der Fehlercode ERRCODE_IO_WRONGFORMAT
    gesetzt. SFX_REC_PRETAG_EXT ist g"ultig, da diese extended-Records
    nur eine Erweiterung des SfxMiniRecord darstellen.
*/
inline sal_uInt8 SfxMiniRecordReader::GetTag() const
{
    return _nPreTag;
}

/*  [Beschreibung]

    Hiermit kann abgefragt werden, ob der Record erfolgreich aus dem
    Stream konstruiert werden konnte, der Header also f"ur diesen Record-Typ
    passend war.
*/
inline bool SfxMiniRecordReader::IsValid() const
{
    return _nPreTag != SFX_REC_PRETAG_EOR;
}

/*  [Beschreibung]

    Dieser Operator liefert den Stream in dem der Record liegt.
    Die aktuelle Position des Streams mu\s innerhalb des Records liegen.
*/
inline SvStream& SfxMiniRecordReader::operator*() const
{
    DBG_ASSERT( _pStream->Tell() < _nEofRec, "read behind record" );
    return *_pStream;
}

//  siehe <SfxMiniRecordWriter::Close(bool)>
inline sal_uInt32 SfxSingleRecordWriter::Close( bool bSeekToEndOfRec )
{
    sal_uInt32 nRet = 0;

    // wurde der Header noch nicht geschrieben?
    if ( !_bHeaderOk )
    {
        // Basisklassen-Header schreiben
        sal_uInt32 nEndPos = SfxMiniRecordWriter::Close( bSeekToEndOfRec );

        // ggf. ans Ende des eigenen Headers seeken oder hinter Rec bleiben
        if ( !bSeekToEndOfRec )
            _pStream->SeekRel( SFX_REC_HEADERSIZE_SINGLE );
        nRet = nEndPos;
    }
#ifdef DBG_UTIL
    else
        // Basisklassen-Header pr"ufen
        SfxMiniRecordWriter::Close( bSeekToEndOfRec );
#endif

    // Record war bereits geschlossen
//  nRet = 0;
    return nRet;
}

inline void SfxSingleRecordWriter::Reset()
{
    _pStream->Seek( _nStartPos + SFX_REC_HEADERSIZE_MINI +
                                 SFX_REC_HEADERSIZE_SINGLE );
    _bHeaderOk = false;
}

/*  [Beschreibung]

    Liefert des aus dem Header gelesene Tag f"ur den Gesamt-Record.
*/
inline sal_uInt16 SfxSingleRecordReader::GetTag() const
{
    return _nRecordTag;
}

/*  [Beschreibung]

    Liefert die Version des aus dem Stream gelesenen Records.
*/
inline sal_uInt8 SfxSingleRecordReader::GetVersion() const
{
    return _nRecordVer;
}

/*  [Beschreibung]

    Stellt fest, ob der aus dem Stream gelese Record in der Version
    'nVersion' oder h"oher vorliegt.
*/
inline bool SfxSingleRecordReader::HasVersion( sal_uInt16 nVersion ) const
{
    return _nRecordVer >= nVersion;
}

/*  [Beschreibung]

    Der Dtor der Klasse <SfxMultiFixRecordWriter> schlie\st den Record
    automatisch, falls <SfxMutiFixRecordWriter::Close()> nicht bereits
    explizit gerufen wurde.
*/
inline SfxMultiFixRecordWriter::~SfxMultiFixRecordWriter()
{
    // wurde der Header noch nicht geschrieben oder mu\s er gepr"uft werden
    if ( !_bHeaderOk )
        Close();
}

/*  [Beschreibung]

    Mit dieser Methode wird in den Record ein neuer Content eingef"ugt.
    Jeder, auch der 1. Record mu\s durch Aufruf dieser Methode eingeleitet
    werden.
*/
inline void SfxMultiFixRecordWriter::NewContent()
{
    #ifdef DBG_UTIL
    sal_uLong nOldStartPos;
    // Startposition des aktuellen Contents merken - Achtung Subklassen!
    nOldStartPos = _nContentStartPos;
    #endif
    _nContentStartPos = _pStream->Tell();

#ifdef DBG_UTIL
    // ist ein vorhergehender Content vorhanden?
    if ( _nContentCount )
    {
        // pr"ufen, ob der vorhergehende die Soll-Gr"o\se eingehalten hat
        DBG_ASSERT( _nContentStartPos - nOldStartPos == _nContentSize,
                    "wrong content size detected" );
    }
#endif

    // Anzahl mitz"ahlen
    ++_nContentCount;
}

/*  [Beschreibung]

    Legt in 'pStream' einen 'SfxMultiMixRecord' an, f"ur dessen Contents
    je eine separate Kennung f"ur Art (Tag) und Version gespeichert wird.
    Die Gr"o\sen der einzelnen Contents werden automatisch ermittelt.
*/
inline SfxMultiMixRecordWriter::SfxMultiMixRecordWriter
(
    SvStream*       pStream,    // Stream, in dem der Record angelegt wird
    sal_uInt16          nRecordTag, // Gesamt-Record-Art-Kennung
    sal_uInt8           nRecordVer  // Gesamt-Record-Versions-Kennung
)
:   SfxMultiVarRecordWriter( SFX_REC_TYPE_MIXTAGS,
                             pStream, nRecordTag, nRecordVer )
{
}

inline void SfxMultiFixRecordWriter::Reset()
{
    _pStream->Seek( _nStartPos + SFX_REC_HEADERSIZE_MINI +
                                 SFX_REC_HEADERSIZE_SINGLE +
                                 SFX_REC_HEADERSIZE_MULTI );
    _bHeaderOk = false;
}

/*  [Beschreibung]

    Diese Methode liefert die Art-Kennung des zuletzt mit der Methode
    <SfxMultiRecordReder::GetContent()> ge"offneten Contents.
*/
inline sal_uInt16 SfxMultiRecordReader::GetContentTag()
{
    return _nContentTag;
}

/*  [Beschreibung]

    Diese Methode liefert die Version-Kennung des zuletzt mit der Methode
    <SfxMultiRecordReder::GetContent()> ge"offneten Contents.
*/
inline sal_uInt8 SfxMultiRecordReader::GetContentVersion() const
{
    return _nContentVer;
}

/*  [Beschreibung]

    Diese Methode stellt fest, ob die Version 'nVersion' in der Version des
    zuletzt mit der Methode <SfxMultiRecordReder::GetContent()> ge"offneten
    Contents enthalten ist.
*/
inline bool SfxMultiRecordReader::HasContentVersion( sal_uInt16 nVersion ) const
{
    return _nContentVer >= nVersion;
}

/*  [Beschreibung]

    Diese Methode liefert die Anzahl im Record befindlichen Contents.
*/
inline sal_uInt32 SfxMultiRecordReader::ContentCount() const
{
    return _nContentCount;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
