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

#ifndef INCLUDED_SVL_FILEREC_HXX
#define INCLUDED_SVL_FILEREC_HXX

#include <svl/svldllapi.h>
#include <tools/debug.hxx>
#include <tools/stream.hxx>
#include <vector>

#define SFX_REC_PRETAG_EXT              sal_uInt8(0x00) // Pre-Tag for Extended-Records
#define SFX_REC_PRETAG_EOR              sal_uInt8(0xFF) // Pre-Tag for End-Of-Records

#define SFX_REC_TYPE_NONE               sal_uInt8(0x00) // unknown Record-Typ
#define SFX_REC_TYPE_FIRST              sal_uInt8(0x01)
#define SFX_REC_TYPE_SINGLE             sal_uInt8(0x01) // Single-Content-Record
#define SFX_REC_TYPE_FIXSIZE            sal_uInt8(0x02) // Fix-Size-Multi-Content-Record
#define SFX_REC_TYPE_VARSIZE_RELOC      sal_uInt8(0x03) // variable Rec-Size
#define SFX_REC_TYPE_VARSIZE            sal_uInt8(0x04) // old (not movable)
#define SFX_REC_TYPE_MIXTAGS_RELOC      sal_uInt8(0x07) // Mixed Tag Content-Record
#define SFX_REC_TYPE_MIXTAGS            sal_uInt8(0x08) // old (not movable)
#define SFX_REC_TYPE_LAST               sal_uInt8(0x08)
#define SFX_REC_TYPE_MINI                   0x100   // Mini-Record
#define SFX_REC_TYPE_DRAWENG                0x400   // Drawing-Engine-Record
#define SFX_REC_TYPE_EOR                    0xF00   // End-Of-Records

#define SFX_REC_HEADERSIZE_MINI     4   // size of the Mini-Record-Header
#define SFX_REC_HEADERSIZE_SINGLE   4   // additional HEADERSIZE_MINI => 8
#define SFX_REC_HEADERSIZE_MULTI    6   // additional HEADERSIZE_SINGLE => 14

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

/** Writes simple records in a stream
 *
 * An instance of this class can write a simple record into a stream. It identifies itself
 * with a sal_uInt8 and stores its own size. This allows it to be skipped with old versions or
 * readers if they do not know the record type (= tag). No version number will be stored.
 *
 * One can either provide the size or the latter will be automatically calculated based on the
 * difference of Tell() before and after streaming the content.
 *
 * [File Format]
 * 1*              sal_uInt8    Content-Tag (!= 0)
 * 1*              3-sal_uInt8  OffsetToEndOfRec in Bytes
 * SizeOfContent*  sal_uInt8    Content
 *
 * @example
 * {
 *     SfxMiniRecordWriter aRecord( pStream, MY_TAG_X );
 *     *aRecord << aMember1;
 *     *aRecord << aMember2;
 * }
 * @note To ensure up- and downwards compatibility, new versions need to include
 * the data of the older ones and are only allowed to add data afterwards.
 * @see SfxMiniRecordReader
 */
class SVL_DLLPUBLIC SfxMiniRecordWriter
{
protected:
    SvStream*       _pStream;   // <SvStream> with the record
    sal_uInt32      _nStartPos; // starting position of the total record in the stream
    bool            _bHeaderOk; /* TRUE, if header already written */
    sal_uInt8       _nPreTag;   // 'pre-Tag' to write to header

public:
    inline          SfxMiniRecordWriter( SvStream *pStream, sal_uInt8 nTag );
    inline          ~SfxMiniRecordWriter();

    inline SvStream& operator*() const;

    inline void     Reset();
    sal_uInt32      Close( bool bSeekToEndOfRec = true );

private:
    /// not implementend, not allowed
    SfxMiniRecordWriter( const SfxMiniRecordWriter& );
    SfxMiniRecordWriter& operator=(const SfxMiniRecordWriter&);
};

/** Reads simple record from a stream
 *
 * An instance of this class allows to read a simple record from a stream that was written by
 * SfxMiniRecordWriter. It is also possible to skip a record, even without knowing its internal
 * format.
 *
 * @example
 * {
 *      SfxMiniRecordReader aRecord( pStream );
 *      switch ( aRecord.GetTag() )
 *      {
 *          case MY_TAG_X:
 *              *aRecord >> aMember1;
 *              *aRecord >> aMember2;
 *              break;
 *
 *          ...
 *      }
 * }
 * @see SfxMiniRecordWriter
 */
class SVL_DLLPUBLIC SfxMiniRecordReader
{
protected:
    SvStream*           _pStream;   //  <SvStream> to read from
    sal_uInt32          _nEofRec;   //  Position direkt hinter dem Record
    bool                _bSkipped;  //  TRUE: der Record wurde explizit geskippt
    sal_uInt8           _nPreTag;   //  aus dem Header gelesenes Pre-Tag

                        // Drei-Phasen-Ctor f"ur Subklassen
    SfxMiniRecordReader()
        : _pStream(NULL)
        , _nEofRec(0)
        , _bSkipped(false)
        , _nPreTag(0)
    {
    }
    void                 Construct_Impl( SvStream *pStream, sal_uInt8 nTag )
                        {
                            _pStream = pStream;
                            _bSkipped = false;
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

    inline sal_uInt8    GetTag() const;
    inline bool         IsValid() const;

    inline SvStream&    operator*() const;

    inline void         Skip();

private:
    /// not implementend, not allowed
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
    SfxSingleRecordReader()
        : _nRecordTag(0)
        , _nRecordVer(0)
        , _nRecordType(0)
    {
    }
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

/** write record with multiple content items
 *
 * Write a record into a stream that stores its own size. This allows it to be
 * skipped with old versions or readers if they do not know the record type (= tag).
 *
 * It contains multiple content items of the same tag and version, that are both
 * stored in the header of the record. The size of each content will be calculated
 * automatically and stored so that single content items can be skipped without
 * having to read them.
 *
 * [Fileformat]
 * 1*                  sal_uInt8       Pre-Tag (==0)
 * 1*                  3-sal_uInt8     OffsetToEndOfRec in Bytes
 * 1*                  sal_uInt8       Record-Type (==SFX_FILETYPE_TYPE_VARSIZE)
 * 1*                  sal_uInt8       Content-Version
 * 1*                  sal_uInt16      Content-Tag
 * 1*                  sal_uInt16      NumberOfContents
 * 1*                  sal_uInt32      OffsetToOfsTable
 * NumberOfContents*   (
 * ContentSize*        sal_uInt8       Content
 *                     )
 * NumberOfContents*   sal_uInt32      ContentOfs (je per <<8 verschoben)
 * @example
 * {
 *      SfxMultiVarRecordWriter aRecord( pStream, MY_TAG_X, MY_VERSION );
 *      for ( sal_uInt16 n = 0; n < Count(); ++n )
 *      {
 *          aRecord.NewContent();
 *          *aRecord << aMember1[n];
 *          *aRecord << aMember2[n];
 *      }
 *  }
 * @note To ensure up- and downwards compatibility, new versions need to include
 * the data of the older ones and are only allowed to add data afterwards.
 */
class SVL_DLLPUBLIC SfxMultiVarRecordWriter: public SfxMultiFixRecordWriter
{
protected:
    std::vector<sal_uInt32> _aContentOfs;
    sal_uInt16              _nContentVer;   // only for SfxMultiMixRecordWriter

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

    virtual sal_uInt32  Close( bool bSeekToEndOfRec = true );
};

/** write record with multiple content items with identical size
 *
 * Write a record into a stream that stores its own size. This allows it to be
 * skipped with old versions or readers if they do not know the record type (= tag).
 *
 * It contains multiple content items of the same tag and version, that are both
 * stored in the header of the record. All content items have a known identical
 * size.
 *
 * [Fileformat]
 * 1*                  sal_uInt8       Pre-Tag (==0)
 * 1*                  3-sal_uInt8     OffsetToEndOfRec in Bytes
 * 1*                  sal_uInt8       record type (==SFX_REC_TYPE_MIXTAGS)
 * 1*                  sal_uInt8       content version
 * 1*                  sal_uInt16      record tag
 * 1*                  sal_uInt16      NumberOfContents
 * 1*                  sal_uInt32      OffsetToOfsTable
 * NumberOfContents*   (
 * 1*                  sal_uInt16      content tag
 * ContentSize*        sal_uInt8       content
 *                     )
 * NumberOfContents*   sal_uInt32      ( ContentOfs << 8 + Version )
 * @note To ensure up- and downwards compatibility, new versions need to include
 * the data of the older ones and are only allowed to add data afterwards.
 */
class SVL_DLLPUBLIC SfxMultiMixRecordWriter: public SfxMultiVarRecordWriter
{
public:
    inline              SfxMultiMixRecordWriter( SvStream *pStream,
                                                 sal_uInt16 nRecordTag,
                                                 sal_uInt8 nRecordVer );

    void                NewContent( sal_uInt16 nTag, sal_uInt8 nVersion );
// private: not possible, since some compilers then make the previous also private
    void                NewContent()
                        { OSL_FAIL( "NewContent() only allowed with args" ); }
};

/** Read multiple content items of an existing record
 *
 * Instances of this class allow to read multiple content items of a record
 * that was written with
 * - SfxMultiFixRecordWriter
 * - SfxMultiVarRecordWriter
 * - SfxMultiMixRecordWriter
 *
 * It is possible to skip single content or the whole record without knowing
 * its internal format.
 *
 * @example
 * {
 *      SfxMultiRecordReader aRecord( pStream );
 *      for ( sal_uInt16 nRecNo = 0; aRecord.GetContent(); ++nRecNo )
 *      {
 *          switch ( aRecord.GetTag() )
 *          {
 *              case MY_TAG_X:
 *                  X *pObj = new X;
 *                  *aRecord >> pObj.>aMember1;
 *                  if ( aRecord.HasVersion(2) )
 *                      *aRecord >> pObj->aMember2;
 *                  Append( pObj );
 *                  break;
 *
 *              ...
 *          }
 *      }
 *  }
 */
class SVL_DLLPUBLIC SfxMultiRecordReader: public SfxSingleRecordReader
{
    sal_uInt32          _nStartPos;     //  start position of this record
    sal_uInt32*         _pContentOfs;   //  offsets of the start positions
    sal_uInt32          _nContentSize;  //  size of each record or table position
    sal_uInt16          _nContentCount; //  number of content items
    sal_uInt16          _nContentNo;    /*  the index of the current content
                                            contains the next content's index
                                            for GetContent() */
    sal_uInt16          _nContentTag;   //  tag of the current content
    sal_uInt8           _nContentVer;   //  version of the current content

    bool                ReadHeader_Impl();

public:
                        SfxMultiRecordReader( SvStream *pStream, sal_uInt16 nTag );
                        ~SfxMultiRecordReader();

    bool                GetContent();
    inline sal_uInt16   GetContentTag();
    inline sal_uInt8    GetContentVersion() const;
    inline bool         HasContentVersion( sal_uInt16 nVersion ) const;

    inline sal_uInt32   ContentCount() const;
};

/** create a mini record
 *
 * The content size is calculated automatically after streaming.
 *
 * @param pStream the stream that will contain the record
 * @param nTag    a record tag between 0x01 and 0xFE
 */
inline SfxMiniRecordWriter::SfxMiniRecordWriter( SvStream* pStream, sal_uInt8 nTag )
:   _pStream( pStream ),
    _nStartPos( pStream->Tell() ),
    _bHeaderOk(false),
    _nPreTag( nTag )
{
    DBG_ASSERT( _nPreTag != 0xFF, "invalid Tag" );
    SAL_INFO("svl", "SfxFileRec: writing record to " << pStream->Tell());

    pStream->SeekRel( + SFX_REC_HEADERSIZE_MINI );
}

/** The destructor closes the record automatically if not done earlier */
inline SfxMiniRecordWriter::~SfxMiniRecordWriter()
{
    // the header was not written, yet, or needs to be checked
    if ( !_bHeaderOk )
        Close();
}

/** Get the record's stream
 * @return The stream containing the record
 * @note The record must not be already closed!
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

/** The dtor moves the stream automatically to the position directly behind the record */
inline SfxMiniRecordReader::~SfxMiniRecordReader()
{
    if ( !_bSkipped )
        Skip();
}

/** position the stream directly behind the record's end */
inline void SfxMiniRecordReader::Skip()
{
    _pStream->Seek(_nEofRec);
    _bSkipped = true;
}

/** Get the pre-tag of this record
 *
 * The pre-tag might also be SFX_REC_PRETAG_EXT or SFX_REC_PRETAG_EOR.
 * The latter means that in the stream the error code ERRCODE_IO_WRONGFORMAT
 * is set. The former is valid, since extended records are just building on
 * top of SfxMiniRecord.
 *
 * @return The pre-tag
 */
inline sal_uInt8 SfxMiniRecordReader::GetTag() const
{
    return _nPreTag;
}

/** This method allows to check if the record could be recreated successfully
 *  from the stream and, hence, was correct for this record type.
 */
inline bool SfxMiniRecordReader::IsValid() const
{
    return _nPreTag != SFX_REC_PRETAG_EOR;
}

/** get the owning stream
 *
 * This method returns the stream in which the record is contained.
 * The current position of the stream must be inside the record.
 */
inline SvStream& SfxMiniRecordReader::operator*() const
{
    DBG_ASSERT( _pStream->Tell() < _nEofRec, "read behind record" );
    return *_pStream;
}

/// @see SfxMiniRecordWriter::Close()
inline sal_uInt32 SfxSingleRecordWriter::Close( bool bSeekToEndOfRec )
{
    sal_uInt32 nRet = 0;

    // was the header already written?
    if ( !_bHeaderOk )
    {
        // write base class header
        sal_uInt32 nEndPos = SfxMiniRecordWriter::Close( bSeekToEndOfRec );

        // seek the end of the own header if needed or stay behind the record
        if ( !bSeekToEndOfRec )
            _pStream->SeekRel( SFX_REC_HEADERSIZE_SINGLE );
        nRet = nEndPos;
    }
#ifdef DBG_UTIL
    else
        // check base class header
        SfxMiniRecordWriter::Close( bSeekToEndOfRec );
#endif

    return nRet;
}

inline void SfxSingleRecordWriter::Reset()
{
    _pStream->Seek( _nStartPos + SFX_REC_HEADERSIZE_MINI +
                                 SFX_REC_HEADERSIZE_SINGLE );
    _bHeaderOk = false;
}

/** @returns the tag for the overall record (stored in the record's head) */
inline sal_uInt16 SfxSingleRecordReader::GetTag() const
{
    return _nRecordTag;
}

/** @returns version of the record */
inline sal_uInt8 SfxSingleRecordReader::GetVersion() const
{
    return _nRecordVer;
}

/** determine if the read record has at least the given version */
inline bool SfxSingleRecordReader::HasVersion( sal_uInt16 nVersion ) const
{
    return _nRecordVer >= nVersion;
}

/** The destructor closes the record automatically if not done earlier */
inline SfxMultiFixRecordWriter::~SfxMultiFixRecordWriter()
{
    // the header was not written, yet, or needs to be checked
    if ( !_bHeaderOk )
        Close();
}

/** add a new content into a record
 *
 * @note each, also the first record, must be initialized by this method
 */
inline void SfxMultiFixRecordWriter::NewContent()
{
    #ifdef DBG_UTIL
    sal_uLong nOldStartPos;
    // store starting position of the current content - CAUTION: sub classes!
    nOldStartPos = _nContentStartPos;
    #endif
    _nContentStartPos = _pStream->Tell();

#ifdef DBG_UTIL
    // is there a previous content?
    if ( _nContentCount )
    {
        // check if the previous content stays in specified max. size
        DBG_ASSERT( _nContentStartPos - nOldStartPos == _nContentSize,
                    "wrong content size detected" );
    }
#endif

    // count how many
    ++_nContentCount;
}

/**
 * Creates a SfxMultiMixRecord in the given stream with a seperate tags and
 * versions of its content parts. The sizes of each part are calculated
 * automatically.
 *
 * @param pStream    target stream in which the record will be created
 * @param nRecordTag tag for the total record
 * @param nRecordVer version for the total record
 */
inline SfxMultiMixRecordWriter::SfxMultiMixRecordWriter( SvStream* pStream,
                                                         sal_uInt16 nRecordTag,
                                                         sal_uInt8 nRecordVer )
: SfxMultiVarRecordWriter( SFX_REC_TYPE_MIXTAGS, pStream, nRecordTag, nRecordVer )
{
}

inline void SfxMultiFixRecordWriter::Reset()
{
    _pStream->Seek( _nStartPos + SFX_REC_HEADERSIZE_MINI +
                                 SFX_REC_HEADERSIZE_SINGLE +
                                 SFX_REC_HEADERSIZE_MULTI );
    _bHeaderOk = false;
}

/** @returns the tag of the last opened content
 *  @see SfxMultiRecordReder::GetContent()
 */
inline sal_uInt16 SfxMultiRecordReader::GetContentTag()
{
    return _nContentTag;
}

/** @returns the version of the last opened content
 *  @see SfxMultiRecordReder::GetContent()
 */
inline sal_uInt8 SfxMultiRecordReader::GetContentVersion() const
{
    return _nContentVer;
}

/** Determines if the given version is in the last opened content
 *
 * This method checks if the version is contained in the last version of the
 * content that was opened with SfxMultiRecordReder::GetContent().
 *
 * @param nVersion The version to find
 * @return true, if found
 * @see SfxMultiRecordReder::GetContent()
 */
inline bool SfxMultiRecordReader::HasContentVersion( sal_uInt16 nVersion ) const
{
    return _nContentVer >= nVersion;
}

/** @returns number of this record's contents */
inline sal_uInt32 SfxMultiRecordReader::ContentCount() const
{
    return _nContentCount;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
