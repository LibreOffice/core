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

#ifndef _SFXFILEREC_HXX
#define _SFXFILEREC_HXX

#include <tools/debug.hxx>

#include <tools/stream.hxx>

#include <bf_svtools/svarray.hxx>

namespace binfilter
{

SV_DECL_VARARR( SfxUINT32s, UINT32, 8, 8 )

//------------------------------------------------------------------------

#define SFX_BOOL_DONTCARE				BOOL(2) 	// Don't-Care-Wert f"ur BOOLs

#define SFX_REC_PRETAG_EXT				BYTE(0x00)	// Pre-Tag f"ur Extended-Records
#define SFX_REC_PRETAG_EOR				BYTE(0xFF)	// Pre-Tag f"ur End-Of-Records

#define SFX_REC_TYPE_NONE				BYTE(0x00)	// unbekannter Record-Typ
#define SFX_REC_TYPE_FIRST				BYTE(0x01)
#define SFX_REC_TYPE_SINGLE 			BYTE(0x01)	// Single-Content-Record
#define SFX_REC_TYPE_FIXSIZE			BYTE(0x02)	// Fix-Size-Multi-Content-Record
#define SFX_REC_TYPE_VARSIZE_RELOC      BYTE(0x03)	// variable Rec-Size
#define SFX_REC_TYPE_VARSIZE            BYTE(0x04)	// alt (nicht verschiebbar)
#define SFX_REC_TYPE_MIXTAGS_RELOC  	BYTE(0x07)	// Mixed Tag Content-Record
#define SFX_REC_TYPE_MIXTAGS			BYTE(0x08)	// alt (nicht verschiebbar)
#define SFX_REC_TYPE_LAST				BYTE(0x08)
#define SFX_REC_TYPE_MINI					0x100	// Mini-Record
#define SFX_REC_TYPE_DRAWENG				0x400	// Drawing-Engine-Record
#define SFX_REC_TYPE_EOR					0xF00	// End-Of-Records

//------------------------------------------------------------------------

#define SFX_REC_HEADERSIZE_MINI 	4	// Gr"o\se des Mini-Record-Headers
#define SFX_REC_HEADERSIZE_SINGLE	4	// zzgl. HEADERSIZE_MINI => 8
#define SFX_REC_HEADERSIZE_MULTI	6	// zzgl. HEADERSIZE_SINGLE => 14

//------------------------------------------------------------------------

#ifndef DBG
#ifdef DBG_UTIL
#define DBG(x) x
#else
#define DBG(x)
#endif
#endif

//------------------------------------------------------------------------

/*	[Fileformat]

    Jeder Record beginnt mit einem Byte, dem sogenannten 'Pre-Tag'.

    Ist dieses 'Pre-Tag' == 0x00, dann handelt es sich um einen Extended-
    Record, dessen Typ durch ein weiteres Byte an Position 5 n�her
    beschrieben wird:

    0x01:		SfxSingleRecord
    0x02:		SfxMultiFixRecord
    0x03+0x04:	SfxMultiVarRecord
    0x07+0x08:	SfxMultiMixRecord
    (Alle weiteren Record-Typ-Kennungen sind reserviert.)

    I.d.R. werden File-Formate schon aus Performance-Gr"unden so aufgebaut,
    da\s beim Lesen	jeweils vorher schon feststeht, welcher Record-Typ
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

                            1 BYTE			Pre-Tag
                            3 BYTE			OffsetToEndOfRec
        OffsetToEndOfRec*	1 BYTE			Content

    Bei den Extended-Reords folgt auf diesen 4-Byte-Header ein erweiterter
    Header, der zun"achst den o.g. Record-Typ, dann eine Versions-Kennung
    sowie ein Tag enth"alt, welches den Inhalt kennzeichnet.

        Struktur des Extended-Records:

                            1 BYTE			Pre-Tag (==0x00)
                            3 BYTE			OffsetToEndOfRec
        OffsetToEndOfRec*	1 BYTE			Content
                            1 BYTE			Record-Type
                            1 BYTE			Version
                            2 BYTE			Tag
        ContentSize*		1 BYTE			Content

        (ContentSize = OffsetToEndOfRec - 8)

    [Anmerkung]

    Der Aufbau der Records wird wie folgt begr"undet:

    Der SW-Record-Typ war zuerst vorhanden, mu\ste also 1:1 "ubernommen
    werden. Zum Gl"uck wurden einige Record-Tags nicht verwendet, (Z.B.
    0x00 und 0xFF).
    => 	1. Byte 0x00 kann als Kennung f"ur erweiterten Record verwendet werden
    => 	1. Byte 0xFF kann f"ur besondere Zwecke verwendet werden

    Egal welcher Record-Typ	vorliegt, sollte eine Erkennung des Typs, ein
    Auslesen des Headers und ein "uberpspringen des Records m"oglich sein,
    ohne zu"uck-seeken zu m"ussen und ohne "uberfl"ussige Daten lesen zu
    m"ussen.
    => 	die Bytes 2-4 werden bei allen Records als Offset zum Ende des
        Records interpretiert, so da\s die Gesamt-Recors-Size sich wie
        folgt berechnet: sizeof(UINT32) + OffsetToEndOfRec

    Die Records sollten einfach zu parsen un einheitlich aufgebaut sein.
    => 	Sie bauen aufeinander auf, so ist z.B. der SfxMiniRecord in jedem
        anderen enthalten.

    Die Records sollten auch von denen der Drawing Enginge unterscheidbar
    sein. Diese beginnen mit 'DRMD' und 'DRVW'.
    =>	Mini-Records mit dem Pre-Tag 'D' d"urfen maximal 4MB gro\s sein,
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

//------------------------------------------------------------------------

class  SfxMiniRecordWriter

/*	[Beschreibung]

    Mit Instanzen dieser Klasse kann ein einfacher Record in einen Stream
    geschrieben werden, der sich durch ein BYTE-Tag identifiziert, sowie
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

    1*				BYTE  		Content-Tag (!= 0)
    1*				3-BYTE		OffsetToEndOfRec in Bytes
    SizeOfContent*	BYTE		Content

    [Beispiel]

    {
        SfxMiniRecordWriter aRecord( pStream, MY_TAG_X );
        *aRecord << aMember1;
        *aRecord << aMember2;
    }
*/

{
protected:
    SvStream*		_pStream;	//	<SvStream>, in dem der Record liegt
    UINT32			_nStartPos; //	Start-Position des Gesamt-Records im Stream
    FASTBOOL		_bHeaderOk; /*	TRUE, wenn der Header schon geschrieben ist;
                                    bei DBG_UTIL wird SFX_BOOL_DONTCARE ver-
                                    wendet, um die Gr"o\se von Fix-Sized-Records
                                    zu pr"ufen. */
    BYTE			_nPreTag;	//	in den Header zu schreibendes 'Pre-Tag'

public:
    inline			SfxMiniRecordWriter( SvStream *pStream,
                                         BYTE nTag );
    inline			SfxMiniRecordWriter( SvStream *pStream, BYTE nTag,
                                         UINT32 nSize );

    inline			~SfxMiniRecordWriter();

    inline SvStream& operator*() const;

    inline void		Reset();

    UINT32			Close( FASTBOOL bSeekToEndOfRec = TRUE );

private:
                    // not implementend, not allowed
                    SfxMiniRecordWriter( const SfxMiniRecordWriter& );
    SfxMiniRecordWriter& operator=(const SfxMiniRecordWriter&);
};

//------------------------------------------------------------------------

class  SfxMiniRecordReader

/*	[Beschreibung]

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

{
protected:
    SvStream*			_pStream;	//	<SvStream>, aus dem gelesen wird
    UINT32				_nEofRec;	//	Position direkt hinter dem Record
    FASTBOOL			_bSkipped;	//	TRUE: der Record wurde explizit geskippt
    BYTE				_nPreTag;	//	aus dem Header gelesenes Pre-Tag

                        // Drei-Phasen-Ctor f"ur Subklassen
                        SfxMiniRecordReader() {}
   void 				Construct_Impl( SvStream *pStream, BYTE nTag )
                        {
                            _pStream = pStream;
                            _bSkipped = FALSE;
                            _nPreTag = nTag;
                        }
    inline FASTBOOL 	SetHeader_Impl( UINT32 nHeader );

                        // als ung"ultig markieren und zur"uck-seeken
    void				SetInvalid_Impl( UINT32 nRecordStartPos )
                        {
                            _nPreTag = SFX_REC_PRETAG_EOR;
                            _pStream->Seek( nRecordStartPos );
                        }

public:
                        SfxMiniRecordReader( SvStream *pStream, BYTE nTag );
    inline				~SfxMiniRecordReader();

    inline BYTE 		GetTag() const;
    inline FASTBOOL 	IsValid() const;

    inline SvStream&	operator*() const;

    inline void 		Skip();

private:
                        // not implementend, not allowed
                        SfxMiniRecordReader( const SfxMiniRecordReader& );
    SfxMiniRecordReader& operator=(const SfxMiniRecordReader&);
};

//------------------------------------------------------------------------

class  SfxSingleRecordWriter: public SfxMiniRecordWriter

/*	[Beschreibung]

    Mit Instanzen dieser Klasse kann ein Record in einen Stream geschrieben
    werden, dessen einziger Inhalt sich durch ein UINT16-Tag und eine
    BYTE-Versions-Nummer identifiziert, sowie seine eigene L"ange speichert
    und somit auch von "alteren Versionen bzw. Readern, die diesen
    Record-Type (Tag) nicht kennen, "ubersprungen werden kann.

    Alternativ kann die Gr"o\se fest angegeben werden oder sie wird
    automatisch aus der Differenz der Tell()-Angaben vor und nach dem
    Streamen des Inhalts ermittelt.

    Um Auf- und Abw"artskompatiblit"at gew"ahrleisten zu k"onnen, m"ussen
    neue Versionen die Daten der "alteren immer komplett enthalten,
    es d"urfen allenfalls neue Daten hintenan geh"angt werden!

    [Fileformat]

    1*				BYTE  		Pre-Tag (!= 0)
    1*				3-BYTE		OffsetToEndOfRec in Bytes
    1*				BYTE		Record-Type (==SFX_REC_TYPE_SINGLE)
    1*				BYTE		Content-Version
    1*				USHORT		Content-Tag
    SizeOfContent*	BYTE		Content

    [Beispiel]

    {
        SfxSingleRecordWriter aRecord( pStream, MY_TAG_X, MY_VERSION );
        *aRecord << aMember1;
        *aRecord << aMember2;
    }
*/

{
protected:
                    SfxSingleRecordWriter( BYTE nRecordType,
                                           SvStream *pStream,
                                           UINT16 nTag, BYTE nCurVer );

public:
                    SfxSingleRecordWriter( SvStream *pStream,
                                           UINT16 nTag, BYTE nCurVer );

    inline void		Reset();

    UINT32			Close( FASTBOOL bSeekToEndOfRec = TRUE );
};

//------------------------------------------------------------------------

class  SfxSingleRecordReader: public SfxMiniRecordReader

/*	[Beschreibung]

    Mit Instanzen dieser Klasse kann ein einfacher Record aus einem Stream
    gelesen werden, der mit der Klasse <SfxSingleRecordWriter> geschrieben
    wurde.

    Es ist auch m"oglich, den Record zu "uberspringen, ohne sein internes
    Format zu kennen.

    [Beispiel]

    {
        SfxSingleRecordReader aRecord( pStream );
        switch ( aRecord.GetTag() )
        {
            case MY_TAG_X:
                aRecord >> aMember1;
                if ( aRecord.HasVersion(2) )
                    *aRecord >> aMember2;
                break;

            ...
        }
    }
*/

{
protected:
    UINT16				_nRecordTag;	// Art des Gesamt-Inhalts
    BYTE				_nRecordVer;	// Version des Gesamt-Inhalts
    BYTE				_nRecordType;	// Record Type aus dem Header

                        // Drei-Phasen-Ctor f"ur Subklassen
                        SfxSingleRecordReader() {}
    void				Construct_Impl( SvStream *pStream )
                        {
                            SfxMiniRecordReader::Construct_Impl(
                                    pStream, SFX_REC_PRETAG_EXT );
                        }
    FASTBOOL			FindHeader_Impl( UINT16 nTypes, UINT16 nTag );
    FASTBOOL            ReadHeader_Impl( USHORT nTypes );

public:
                        SfxSingleRecordReader( SvStream *pStream, USHORT nTag );

    inline UINT16		GetTag() const;

    inline BYTE 		GetVersion() const;
    inline FASTBOOL 	HasVersion( USHORT nVersion ) const;
};

//------------------------------------------------------------------------

class  SfxMultiFixRecordWriter: public SfxSingleRecordWriter

/*	[Beschreibung]

    Mit Instanzen dieser Klasse kann ein Record in einen Stream geschrieben
    werden, der seine eigene L"ange speichert und somit	auch von "alteren
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

    1*					BYTE		Pre-Tag (==0)
    1*					3-BYTE		OffsetToEndOfRec in Bytes
    1*					BYTE		Record-Type (==SFX_REC_TYPE_FIXSIZE)
    1*					BYTE		Content-Version
    1*					UINT16		Content-Tag
    1*					UINT16		NumberOfContents
    1*					UINT32		SizeOfEachContent
    NumberOfContents*	(
    SizeOfEachContent	BYTE		Content
                        )

    [Beispiel]

    {
        SfxMultiFixRecordWriter aRecord( pStream, MY_TAG_X, MY_VERSION );
        for ( USHORT n = 0; n < Count(); ++n )
        {
            aRecord.NewContent();
            *aRecord << aMember1[n];
            *aRecord << aMember2[n];
        }
    }
*/

{
protected:
    UINT32			_nContentStartPos;	/*	Startposition des jeweiligen
                                            Contents - nur bei DBG_UTIL
                                            und f"ur Subklassen */
    UINT32			_nContentSize;		//	Gr"o\se jedes Contents
    UINT16			_nContentCount; 	//	jeweilige Anzahl der Contents

                    SfxMultiFixRecordWriter( BYTE nRecordType,
                                             SvStream *pStream,
                                             UINT16 nTag, BYTE nCurVer,
                                             UINT32 nContentSize );

public:
    inline			~SfxMultiFixRecordWriter();

    inline void 	NewContent();

    inline void		Reset();

    UINT32			Close( FASTBOOL bSeekToEndOfRec = TRUE );
};

//------------------------------------------------------------------------

class  SfxMultiVarRecordWriter: public SfxMultiFixRecordWriter

/*	[Beschreibung]

    Mit Instanzen dieser Klasse kann ein Record in einen Stream geschrieben
    werden, der seine eigene L"ange speichert und somit	auch von "alteren
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

    1*					BYTE		Pre-Tag (==0)
    1*					3-BYTE		OffsetToEndOfRec in Bytes
    1*					BYTE		Record-Type (==SFX_FILETYPE_TYPE_VARSIZE)
    1*					BYTE		Content-Version
    1*					USHORT		Content-Tag
    1*					UINT16		NumberOfContents
    1*					UINT32		OffsetToOfsTable
    NumberOfContents*	(
    ContentSize*		BYTE		Content
                        )
    NumberOfContents*	UINT32		ContentOfs (je per <<8 verschoben)

    [Beispiel]

    {
        SfxMultiVarRecordWriter aRecord( pStream, MY_TAG_X, MY_VERSION );
        for ( USHORT n = 0; n < Count(); ++n )
        {
            aRecord.NewContent();
            *aRecord << aMember1[n];
            *aRecord << aMember2[n];
        }
    }
*/

{
protected:
    SfxUINT32s  		_aContentOfs;
    USHORT              _nContentVer;   // nur f"ur SfxMultiMixRecordWriter

                        SfxMultiVarRecordWriter( BYTE nRecordType,
                                                 SvStream *pStream,
                                                 USHORT nRecordTag,
                                                 BYTE nRecordVer );

    void                FlushContent_Impl();

public:
                        SfxMultiVarRecordWriter( SvStream *pStream,
                                                 USHORT nRecordTag,
                                                 BYTE nRecordVer );
    virtual				~SfxMultiVarRecordWriter();

    void				NewContent();

    virtual UINT32		Close( FASTBOOL bSeekToEndOfRec = TRUE );
};

//------------------------------------------------------------------------

class  SfxMultiMixRecordWriter: public SfxMultiVarRecordWriter

/*	[Beschreibung]

    Mit Instanzen dieser Klasse kann ein Record in einen Stream geschrieben
    werden, der seine eigene L"ange speichert und somit	auch von "alteren
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

    1*					BYTE		Pre-Tag (==0)
    1*					3-BYTE		OffsetToEndOfRec in Bytes
    1*					BYTE		Record-Type (==SFX_REC_TYPE_MIXTAGS)
    1*					BYTE		Content-Version
    1*					USHORT		Record-Tag
    1*					UINT16		NumberOfContents
    1*					UINT32		OffsetToOfsTable
    NumberOfContents*   (
    1*                  USHORT      Content-Tag
    ContentSize*        BYTE        Content
                        )
    NumberOfContents*	UINT32		( ContentOfs << 8 + Version )
*/

{
public:
    inline				SfxMultiMixRecordWriter( SvStream *pStream,
                                                 USHORT nRecordTag,
                                                 BYTE nRecordVer );

    void				NewContent( USHORT nTag, BYTE nVersion );

// private: geht nicht, da einige Compiler dann auch vorherige privat machen
    void                NewContent()
                        { DBG_ERROR( "NewContent() only allowed with args" ); }
};

//------------------------------------------------------------------------

class  SfxMultiRecordReader: public SfxSingleRecordReader

/*	[Beschreibung]

    Mit Instanzen dieser Klasse kann ein aus mehreren Contents bestehender
    Record aus einem Stream gelesen werden, der mit einer der Klassen
    <SfxMultiFixRecordWriter>, <SfxMultiVarRecordWriter> oder
    <SfxMultiMixRecordWriter> geschrieben wurde.

    Es ist auch m"oglich, den Record oder einzelne Contents zu "uberspringen,
    ohne das jeweilis interne Format zu kennen.

    [Beispiel]

    {
        SfxMultiRecordReader aRecord( pStream );
        for ( USHORT nRecNo = 0; aRecord.GetContent(); ++nRecNo )
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

{
    UINT32				_nStartPos;		//	Start-Position des Records
    UINT32* 			_pContentOfs;	//	Offsets der Startpositionen
    UINT32				_nContentSize;	//	Size jedes einzelnen / Tabellen-Pos
    UINT16				_nContentCount; //	Anzahl der Contents im Record
    UINT16				_nContentNo;	/*	der Index des aktuellen Contents
                                            enth"alt jeweils den Index des
                                            Contents, der beim n"achsten
                                            GetContent() geholt wird */
    UINT16				_nContentTag;	//	Art-Kennung des aktuellen Contents
    BYTE				_nContentVer;	//	Versions-Kennung des akt. Contents

    FASTBOOL			ReadHeader_Impl();

public:
                        SfxMultiRecordReader( SvStream *pStream, UINT16 nTag );
                        ~SfxMultiRecordReader();

    FASTBOOL			GetContent();
    inline UINT16		GetContentTag();
    inline BYTE 		GetContentVersion() const;
    inline FASTBOOL 	HasContentVersion( USHORT nVersion ) const;

    inline UINT32		ContentCount() const;
};

//=========================================================================

inline SfxMiniRecordWriter::SfxMiniRecordWriter
(
    SvStream*       pStream,        // Stream, in dem der Record angelegt wird
    BYTE            nTag            // Record-Tag zwischen 0x01 und 0xFE
)

/*  [Beschreibung]

    Legt in 'pStream' einen 'SfxMiniRecord' an, dessen Content-Gr"o\se
    nicht bekannt ist, sondern nach dam Streamen des Contents errechnet
    werden soll.
*/

:   _pStream( pStream ),
    _nStartPos( pStream->Tell() ),
    _bHeaderOk(FALSE),
    _nPreTag( nTag )
{
    DBG_ASSERT( _nPreTag != 0xFF, "invalid Tag" );
    DBG( DbgOutf( "SfxFileRec: writing record to %ul", pStream->Tell() ) );

    pStream->SeekRel( + SFX_REC_HEADERSIZE_MINI );
}

//-------------------------------------------------------------------------

inline SfxMiniRecordWriter::SfxMiniRecordWriter
(
    SvStream*		pStream,		// Stream, in dem der Record angelegt wird
    BYTE			nTag,			// Record-Tag zwischen 0x01 und 0xFE
    UINT32			nSize			// Gr"o\se der Daten in Bytes
)

/*	[Beschreibung]

    Legt in 'pStream' einen 'SfxMiniRecord' an, dessen Content-Gr"o\se
    von vornherein bekannt ist.
*/

:   _pStream( pStream ),
    // _nTag( uninitialized ),
    // _nStarPos( uninitialized ),
    _bHeaderOk(SFX_BOOL_DONTCARE)
{
    DBG_ASSERT( nTag != 0 && nTag != 0xFF, "invalid Tag" );
    DBG(_nStartPos = pStream->Tell());
    DBG( DbgOutf( "SfxFileRec: writing record to %ul", _nStartPos ) );

    *pStream << ( ( nTag << 24 ) | nSize );
}

//-------------------------------------------------------------------------

inline SfxMiniRecordWriter::~SfxMiniRecordWriter()

/*	[Beschreibung]

    Der Dtor der Klasse <SfxMiniRecordWriter> schlie\st den Record
    automatisch, falls <SfxMiniRecordWriter::Close()> nicht bereits
    explizit gerufen wurde.
*/

{
    // wurde der Header noch nicht geschrieben oder mu\s er gepr"uft werden
    if ( !_bHeaderOk DBG(||TRUE) )
        Close();
}

//-------------------------------------------------------------------------

inline SvStream& SfxMiniRecordWriter::operator*() const

/*	[Beschreibung]

    Dieser Operator liefert den Stream, in dem der Record liegt.
    Der Record darf noch nicht geschlossen worden sein.
*/

{
    DBG_ASSERT( !_bHeaderOk, "getting Stream of closed record" );
    return *_pStream;
}

//-------------------------------------------------------------------------

inline void	SfxMiniRecordWriter::Reset()
{
    _pStream->Seek( _nStartPos + SFX_REC_HEADERSIZE_MINI );
    _bHeaderOk = FALSE;
}

//=========================================================================

inline SfxMiniRecordReader::~SfxMiniRecordReader()

/*  [Beschreibung]

    Der Dtor der Klasse <SfxMiniRecordReader> positioniert den Stream
    automatisch auf die Position direkt hinter dem Record, falls nicht
    <SfxMiniRecordReader::Skip()> bereits explizit gerufen wurde.
*/

{
    // noch nicht explizit ans Ende gesprungen?
    if ( !_bSkipped )
        Skip();
}

//-------------------------------------------------------------------------

inline void SfxMiniRecordReader::Skip()

/*  [Beschreibung]

    Mit dieser Methode wird der Stream direkt hinter das Ende des Records
    positioniert.
*/

{
    _pStream->Seek(_nEofRec);
    _bSkipped = TRUE;
}

//-------------------------------------------------------------------------

inline BYTE SfxMiniRecordReader::GetTag() const

/*	[Beschreibung]

    Liefert des aus dem Header gelesene Pre-Tag des Records. Dieses kann
    auch SFX_REC_PRETAG_EXT oder SFX_REC_PRETAG_EOR sein, im
    letzteren Fall ist am Stream der Fehlercode ERRCODE_IO_WRONGFORMAT
    gesetzt. SFX_REC_PRETAG_EXT ist g"ultig, da diese extended-Records
    nur eine Erweiterung des SfxMiniRecord darstellen.
*/

{
    return _nPreTag;
}

//-------------------------------------------------------------------------

inline FASTBOOL SfxMiniRecordReader::IsValid() const

/*	[Beschreibung]

    Hiermit kann abgefragt werden, ob der Record erfolgreich aus dem
    Stream konstruiert werden konnte, der Header also f"ur diesen Record-Typ
    passend war.
*/

{
    return _nPreTag != SFX_REC_PRETAG_EOR;
}

//-------------------------------------------------------------------------

inline SvStream& SfxMiniRecordReader::operator*() const

/*	[Beschreibung]

    Dieser Operator liefert den Stream in dem der Record liegt.
    Die aktuelle Position des Streams mu\s innerhalb des Records liegen.
*/

{
    DBG_ASSERT( _pStream->Tell() < _nEofRec, "read behind record" );
    return *_pStream;
}

//=========================================================================

inline UINT32 SfxSingleRecordWriter::Close( FASTBOOL bSeekToEndOfRec )

//	siehe <SfxMiniRecordWriter::Close(FASTBOOL)>

{
    UINT32 nRet = 0;

    // wurde der Header noch nicht geschrieben?
    if ( !_bHeaderOk )
    {
        // Basisklassen-Header schreiben
        UINT32 nEndPos = SfxMiniRecordWriter::Close( bSeekToEndOfRec );

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
//	nRet = 0;
    return nRet;
}

//-------------------------------------------------------------------------

inline void	SfxSingleRecordWriter::Reset()
{
    _pStream->Seek( _nStartPos + SFX_REC_HEADERSIZE_MINI +
                                 SFX_REC_HEADERSIZE_SINGLE );
    _bHeaderOk = FALSE;
}

//=========================================================================

inline UINT16 SfxSingleRecordReader::GetTag() const

/*  [Beschreibung]

    Liefert des aus dem Header gelesene Tag f"ur den Gesamt-Record.
*/

{
    return _nRecordTag;
}

//-------------------------------------------------------------------------

inline BYTE SfxSingleRecordReader::GetVersion() const

/*	[Beschreibung]

    Liefert die Version des aus dem Stream gelesenen Records.
*/

{
    return _nRecordVer;
}

//-------------------------------------------------------------------------

inline FASTBOOL SfxSingleRecordReader::HasVersion( USHORT nVersion ) const

/*	[Beschreibung]

    Stellt fest, ob der aus dem Stream gelese Record in der Version
    'nVersion' oder h"oher vorliegt.
*/

{
    return _nRecordVer >= nVersion;
}

//=========================================================================

inline SfxMultiFixRecordWriter::~SfxMultiFixRecordWriter()

/*	[Beschreibung]

    Der Dtor der Klasse <SfxMultiFixRecordWriter> schlie\st den Record
    automatisch, falls <SfxMutiFixRecordWriter::Close()> nicht bereits
    explizit gerufen wurde.
*/

{
    // wurde der Header noch nicht geschrieben oder mu\s er gepr"uft werden
    if ( !_bHeaderOk )
        Close();
}

//-------------------------------------------------------------------------

inline void SfxMultiFixRecordWriter::NewContent()

/*	[Beschreibung]

    Mit dieser Methode wird in den Record ein neuer Content eingef"ugt.
    Jeder, auch der 1. Record mu\s durch Aufruf dieser Methode eingeleitet
    werden.
*/

{
    #ifdef DBG_UTIL
    ULONG nOldStartPos;
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

//=========================================================================

inline SfxMultiMixRecordWriter::SfxMultiMixRecordWriter
(
    SvStream*		pStream,	// Stream, in dem der Record angelegt wird
    USHORT			nRecordTag, // Gesamt-Record-Art-Kennung
    BYTE			nRecordVer	// Gesamt-Record-Versions-Kennung
)

/*  [Beschreibung]

    Legt in 'pStream' einen 'SfxMultiMixRecord' an, f"ur dessen Contents
    je eine separate Kennung f"ur Art (Tag) und Version gespeichert wird.
    Die Gr"o\sen der einzelnen Contents werden automatisch ermittelt.
*/

:	SfxMultiVarRecordWriter( SFX_REC_TYPE_MIXTAGS,
                             pStream, nRecordTag, nRecordVer )
{
}

//=========================================================================

inline void	SfxMultiFixRecordWriter::Reset()
{
    _pStream->Seek( _nStartPos + SFX_REC_HEADERSIZE_MINI +
                                 SFX_REC_HEADERSIZE_SINGLE +
                                 SFX_REC_HEADERSIZE_MULTI );
    _bHeaderOk = FALSE;
}

//=========================================================================

inline UINT16 SfxMultiRecordReader::GetContentTag()

/*	[Beschreibung]

    Diese Methode liefert die Art-Kennung des zuletzt mit der Methode
    <SfxMultiRecordReder::GetContent()> ge"offneten Contents.
*/

{
    return _nContentTag;
}

//-------------------------------------------------------------------------

inline BYTE SfxMultiRecordReader::GetContentVersion() const

/*  [Beschreibung]

    Diese Methode liefert die Version-Kennung des zuletzt mit der Methode
    <SfxMultiRecordReder::GetContent()> ge"offneten Contents.
*/

{
    return _nContentVer;
}

//-------------------------------------------------------------------------

inline FASTBOOL SfxMultiRecordReader::HasContentVersion( USHORT nVersion ) const

/*  [Beschreibung]

    Diese Methode stellt fest, ob die Version 'nVersion' in der Version des
    zuletzt mit der Methode <SfxMultiRecordReder::GetContent()> ge"offneten
    Contents enthalten ist.
*/

{
    return _nContentVer >= nVersion;
}

//-------------------------------------------------------------------------

inline UINT32 SfxMultiRecordReader::ContentCount() const

/*	[Beschreibung]

    Diese Methode liefert die Anzahl im Record befindlichen Contents.
*/

{
    return _nContentCount;
}

}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
