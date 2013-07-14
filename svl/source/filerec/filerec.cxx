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

#include <svl/filerec.hxx>
#include <osl/endian.h>


/*  Die folgenden Makros extrahieren Teilbereiche aus einem sal_uInt32 Wert.
    Diese sal_uInt32-Werte werden anstelle der einzelnen Werte gestreamt,
    um Calls zu sparen.
*/

#define SFX_REC_PRE(n) ( ((n) & 0x000000FF) )
#define SFX_REC_OFS(n) ( ((n) & 0xFFFFFF00) >> 8 )
#define SFX_REC_TYP(n) ( ((n) & 0x000000FF) )
#define SFX_REC_VER(n) ( ((n) & 0x0000FF00) >> 8 )
#define SFX_REC_TAG(n) ( ((n) & 0xFFFF0000) >> 16 )

#define SFX_REC_CONTENT_VER(n) ( ((n) & 0x000000FF) )
#define SFX_REC_CONTENT_OFS(n) ( ((n) & 0xFFFFFF00) >> 8 )


/*  Die folgenden Makros setzen Teilbereiche zu einem sal_uInt32 Wert zusammen.
    Diese sal_uInt32-Werte werden anstelle der einzelnen Werte gestreamt,
    um Calls zu sparen.
*/

#define SFX_REC_MINI_HEADER(nPreTag,nStartPos,nEndPos) \
                    ( sal_uInt32(nPreTag) | \
                      sal_uInt32(nEndPos-nStartPos-SFX_REC_HEADERSIZE_MINI) << 8 )

#define SFX_REC_HEADER(nRecType,nContentTag,nContentVer) \
                    ( sal_uInt32(nRecType) | \
                      ( sal_uInt32(nContentVer) << 8 ) | \
                      ( sal_uInt32(nContentTag) << 16 ) )

#define SFX_REC_CONTENT_HEADER(nContentVer,n1StStartPos,nCurStartPos) \
                    ( sal_uInt32(nContentVer) | \
                      sal_uInt32( nCurStartPos - n1StStartPos ) << 8 )


sal_uInt32 SfxMiniRecordWriter::Close
(
    bool         bSeekToEndOfRec    /*  true (default)
                                        Der Stream wird an das Ende des Records
                                        positioniert.

                                        false
                                        Der Stream wird an den Anfang des
                                        Contents (also hinter den Header)
                                        positioniert.
                                    */
)

/*  [Beschreibung]

    Diese Methode schlie\st den Record. Dabei wird haupts"achlich der
    Header geschrieben.

    Wurde der Header bereits geschrieben, hat der Aufruf keine Wirkung.


    [R"uckgabewert]

    sal_uInt32      != 0
                Position im Stream, die direkt hinter dem Record liegt.
                'bSeekToEndOfRecord==sal_True'
                => R"uckgabewert == aktuelle Stream-Position nach Aufruf

                == 0
                Der Header war bereits geschrieben worden.
*/

{
    // wurde der Header noch nicht geschrieben?
    if ( !_bHeaderOk )
    {
        // Header an den Anfang des Records schreiben
        sal_uInt32 nEndPos = _pStream->Tell();
        _pStream->Seek( _nStartPos );
        *_pStream << SFX_REC_MINI_HEADER( _nPreTag, _nStartPos, nEndPos );

        // je nachdem ans Ende des Records seeken oder hinter Header bleiben
        if ( bSeekToEndOfRec )
            _pStream->Seek( nEndPos );

        // Header wurde JETZT geschrieben
        _bHeaderOk = true;
        return nEndPos;
    }

    // Record war bereits geschlossen
    return 0;
}

bool SfxMiniRecordReader::SetHeader_Impl( sal_uInt32 nHeader )

/*  [Beschreibung]

    Interne Methode zum nachtr"aglichen Verarbeiten eines extern gelesenen
    Headers. Falls der Header eine End-Of-Records-Kennung darstellt,
    wird am Stream ein Errorcode gesetzt und sal_False zur"uckgeliefert. Im
    Fehlerfall wird der Stream jedoch nicht auf den Record-Anfang zur"uck-
    gesetzt.
*/

{
    bool bRet = true;

    // Record-Ende und Pre-Tag aus dem Header ermitteln
    _nEofRec = _pStream->Tell() + SFX_REC_OFS(nHeader);
    _nPreTag = sal::static_int_cast< sal_uInt8 >(SFX_REC_PRE(nHeader));

    // wenn End-Of-Record-Kennung, dann Fehler
    if ( _nPreTag == SFX_REC_PRETAG_EOR )
    {
        _pStream->SetError( ERRCODE_IO_WRONGFORMAT );
        bRet = true;
    }
    return bRet;
}

SfxMiniRecordReader::SfxMiniRecordReader
(
    SvStream*       pStream,        /*  <SvStream>, an dessen aktueller
                                        Position sich ein <SfxMiniRecord>
                                        befindet.
                                    */
    sal_uInt8           nTag            //  Pre-Tag des gew"unschten Records
)

/*  [Beschreibung]

    Dieser Ctor interpretiert 'pStream' ab der aktuellen Position als
    eine l"uckenlose Folge von, von dieser Klassen-Gruppe interpretierbaren,
    Records. Der in dieser Folge erste als <SfxMiniRecord> interpretierbare
    (also ggf. auch ein extended-Record) mit dem PreTag 'nTag' wird ge"offnet
    und durch diese Instanz repr"asentiert.

    Wird das Ende des Streams oder die Kennung SFX_REC_PRETAG_EOR
    erreicht, bevor ein Record mit dem ge"unschten Pre-Tag gefunden wird,
    ist die erzeugte Instanz ung"ultig ('IsValid() == sal_False'). Ein ent-
    sprechender Error-Code (ERRCODE_IO_EOF bzw. ERRCODE_IO_WRONGFORMAT)
    ist dann am Stream gesetzt, dessen Position ist dann au\serdem unver-
    "andert.

    Bei 'nTag==SFX_FILEREC_PRETAG_EOR' wird nicht versucht, einen Record
    zu lesen, es wird sofort 'IsValid()' auf sal_False gesetzt und kein Error-Code
    am Stream gesetzt. Dies ist dauzu gedacht, ohne 'new' und 'delete'
    abw"rtskompatibel SfxMiniRecords einbauen zu k"onnen. Siehe dazu
    <SfxItemSet::Load()>.


    [Anwendungsvorschlag]

    Wird dieser Ctor in einer bereits ausgelieferten Programmversion
    verwendet, k"onnen in das File-Format jeweils davor kompatibel neue
    Records mit einer anderen Kennung eingef"ugt werden. Diese werden
    schlie\slich automatisch "uberlesen. Erkauft wird diese M"oglichkeit
    allerdings mit etwas schlechterem Laufzeitverhalten im Vergleich mit
    direktem 'drauf-los-lesen', der sich jedoch auf einen Vergleich zweier
    Bytes reduziert, falls der gesuchte Record der erste in der Folge ist.
*/

:   _pStream( pStream ),
    _bSkipped( nTag == SFX_REC_PRETAG_EOR )
{
    // ggf. ignorieren (s.o.)
    if ( _bSkipped )
    {
        _nPreTag = nTag;
        return;
    }

    // StartPos merken, um im Fehlerfall zur"uck-seeken zu k"onnen
    sal_uInt32 nStartPos = pStream->Tell();

    // passenden Record suchen
    while(true)
    {
        // Header lesen
        DBG( DbgOutf( "SfxFileRec: searching record at %ul", pStream->Tell() ) );
        sal_uInt32 nHeader;
        *pStream >> nHeader;

        // Headerdaten von Basisklasse extrahieren lassen
        SetHeader_Impl( nHeader );

        // ggf. Fehler behandeln
        if ( pStream->IsEof() )
            _nPreTag = SFX_REC_PRETAG_EOR;
        else if ( _nPreTag == SFX_REC_PRETAG_EOR )
            pStream->SetError( ERRCODE_IO_WRONGFORMAT );
        else
        {
            // wenn gefunden, dann Schleife abbrechen
            if ( _nPreTag == nTag )
                break;

            // sonst skippen und weitersuchen
            pStream->Seek( _nEofRec );
            continue;
        }

        // Fehler => zur"uck-seeken
        pStream->Seek( nStartPos );
        break;
    }
}


SfxSingleRecordWriter::SfxSingleRecordWriter
(
    sal_uInt8           nRecordType,    // f"ur Subklassen
    SvStream*       pStream,        // Stream, in dem der Record angelegt wird
    sal_uInt16          nContentTag,    // Inhalts-Art-Kennung
    sal_uInt8           nContentVer     // Inhalts-Versions-Kennung
)

/*  [Beschreibung]

    Interner Ctor f"ur Subklassen.
*/

:   SfxMiniRecordWriter( pStream, SFX_REC_PRETAG_EXT )
{
    // Erweiterten Header hiner den des SfxMiniRec schreiben
    *pStream << SFX_REC_HEADER(nRecordType, nContentTag, nContentVer);
}


inline bool SfxSingleRecordReader::ReadHeader_Impl( sal_uInt16 nTypes )

/*  [Beschreibung]

    Interne Methode zum Einlesen eines SfxMultiRecord-Headers, nachdem
    die Basisklasse bereits initialisiert und deren Header gelesen ist.
    Ggf. ist ein Error-Code am Stream gesetzt, im Fehlerfall wird jedoch
    nicht zur"uckge-seekt.
*/

{
    bool bRet;

    // Basisklassen-Header einlesen
    sal_uInt32 nHeader=0;
    *_pStream >> nHeader;
    if ( !SetHeader_Impl( nHeader ) )
        bRet = false;
    else
    {
        // eigenen Header einlesen
        *_pStream >> nHeader;
        _nRecordVer = sal::static_int_cast< sal_uInt8 >(SFX_REC_VER(nHeader));
        _nRecordTag = sal::static_int_cast< sal_uInt16 >(SFX_REC_TAG(nHeader));

        // falscher Record-Typ?
        _nRecordType = sal::static_int_cast< sal_uInt8 >(SFX_REC_TYP(nHeader));
        bRet = 0 != ( nTypes & _nRecordType);
    }
    return bRet;
}


bool SfxSingleRecordReader::FindHeader_Impl
(
    sal_uInt16      nTypes,     // arithm. Veroderung erlaubter Record-Typen
    sal_uInt16      nTag        // zu findende Record-Art-Kennung
)

/*  [Beschreibung]

    Interne Methode zum lesen des Headers des ersten Record, der einem
    der Typen in 'nTypes' entspricht und mit der Art-Kennung 'nTag'
    gekennzeichnet ist.

    Kann ein solcher Record nicht gefunden werden, wird am Stream ein
    Errorcode gesetzt, zur"uck-geseekt und sal_False zur"uckgeliefert.
*/

{
    // StartPos merken, um im Fehlerfall zur"uck-seeken zu k"onnen
    sal_uInt32 nStartPos = _pStream->Tell();

    // richtigen Record suchen
    while ( !_pStream->IsEof() )
    {
        // Header lesen
        sal_uInt32 nHeader;
        DBG( DbgOutf( "SfxFileRec: searching record at %ul", _pStream->Tell() ) );
        *_pStream >> nHeader;
        if ( !SetHeader_Impl( nHeader ) )
            // EOR => Such-Schleife abbreichen
            break;

        // Extended Record gefunden?
        if ( _nPreTag == SFX_REC_PRETAG_EXT )
        {
            // Extended Header lesen
            *_pStream >> nHeader;
            _nRecordTag = sal::static_int_cast< sal_uInt16 >(SFX_REC_TAG(nHeader));

            // richtigen Record gefunden?
            if ( _nRecordTag == nTag )
            {
                // gefundener Record-Typ passend?
                _nRecordType = sal::static_int_cast< sal_uInt8 >(
                    SFX_REC_TYP(nHeader));
                if ( nTypes & _nRecordType )
                    // ==> gefunden
                    return sal_True;

                // error => Such-Schleife abbrechen
                break;
            }
        }

        // sonst skippen
        if ( !_pStream->IsEof() )
            _pStream->Seek( _nEofRec );
    }

    // Fehler setzen und zur"uck-seeken
    _pStream->SetError( ERRCODE_IO_WRONGFORMAT );
    _pStream->Seek( nStartPos );
    return sal_False;
}


SfxMultiFixRecordWriter::SfxMultiFixRecordWriter
(
    sal_uInt8           nRecordType,    // Subklassen Record-Kennung
    SvStream*       pStream,        // Stream, in dem der Record angelegt wird
    sal_uInt16          nContentTag,    // Content-Art-Kennung
    sal_uInt8           nContentVer     // Content-Versions-Kennung
)

/*  [Beschreibung]

    Interne Methode f"ur Subklassen.
*/

:   SfxSingleRecordWriter( nRecordType, pStream, nContentTag, nContentVer ),
    _nContentCount( 0 )
{
    // Platz f"ur eigenen Header
    pStream->SeekRel( + SFX_REC_HEADERSIZE_MULTI );
}


sal_uInt32 SfxMultiFixRecordWriter::Close( bool bSeekToEndOfRec )

//  siehe <SfxMiniRecordWriter>

{
    // Header noch nicht geschrieben?
    if ( !_bHeaderOk )
    {
        // Position hinter Record merken, um sie restaurieren zu k"onnen
        sal_uInt32 nEndPos = SfxSingleRecordWriter::Close( sal_False );

        // gegen"uber SfxSingleRecord erweiterten Header schreiben
        *_pStream << _nContentCount;
        *_pStream << _nContentSize;

        // je nachdem ans Ende des Records seeken oder hinter Header bleiben
        if ( bSeekToEndOfRec )
            _pStream->Seek(nEndPos);
        return nEndPos;
    }

    // Record war bereits geschlossen
    return 0;
}


SfxMultiVarRecordWriter::SfxMultiVarRecordWriter
(
    sal_uInt8           nRecordType,    // Record-Kennung der Subklasse
    SvStream*       pStream,        // Stream, in dem der Record angelegt wird
    sal_uInt16          nRecordTag,     // Gesamt-Art-Kennung
    sal_uInt8           nRecordVer      // Gesamt-Versions-Kennung
)

/*  [Beschreibung]

    Interner Ctor f"ur Subklassen.
*/

:   SfxMultiFixRecordWriter( nRecordType, pStream, nRecordTag, nRecordVer ),
    _nContentVer( 0 )
{
}


SfxMultiVarRecordWriter::SfxMultiVarRecordWriter
(
    SvStream*       pStream,        // Stream, in dem der Record angelegt wird
    sal_uInt16          nRecordTag,     // Gesamt-Art-Kennung
    sal_uInt8           nRecordVer      // Gesamt-Versions-Kennung
)

/*  [Beschreibung]

    Legt in 'pStream' einen 'SfxMultiVarRecord' an, dessen Content-Gr"o\sen
    weder bekannt sind noch identisch sein m"ussen, sondern jeweils nach dem
    Streamen jedes einzelnen Contents errechnet werden sollen.


    [Anmerkung]

    Diese Methode ist nicht inline, da f"ur die Initialisierung eines
    <SvULongs>-Members zu viel Code generiert werden w"urde.
*/

:   SfxMultiFixRecordWriter( SFX_REC_TYPE_VARSIZE,
                             pStream, nRecordTag, nRecordVer ),
    _nContentVer( 0 )
{
}


SfxMultiVarRecordWriter::~SfxMultiVarRecordWriter()

/*  [Beschreibung]

    Der Dtor der Klasse <SfxMultiVarRecordWriter> schlie\st den Record
    automatisch, falls <SfxMultiVarRecordWriter::Close()> nicht bereits
    explizit gerufen wurde.
*/

{
    // wurde der Header noch nicht geschrieben oder mu\s er gepr"uft werden
    if ( !_bHeaderOk )
        Close();
}


void SfxMultiVarRecordWriter::FlushContent_Impl()

/*  [Beschreibung]

    Interne Methode zum Abschlie\sen eines einzelnen Contents.
*/

{
    // Versions-Kennung und Positions-Offset des aktuellen Contents merken;
    // das Positions-Offset ist relativ zur Startposition des ersten Contents
    assert(_aContentOfs.size() == static_cast<size_t>(_nContentCount)-1);
    _aContentOfs.resize(_nContentCount-1);
    _aContentOfs.push_back(
            SFX_REC_CONTENT_HEADER(_nContentVer,_nStartPos,_nContentStartPos));
}


void SfxMultiVarRecordWriter::NewContent()

// siehe <SfxMultiFixRecordWriter>

{
    // schon ein Content geschrieben?
    if ( _nContentCount )
        FlushContent_Impl();

    // neuen Content beginnen
    _nContentStartPos = _pStream->Tell();
    ++_nContentCount;
}


sal_uInt32 SfxMultiVarRecordWriter::Close( bool bSeekToEndOfRec )

// siehe <SfxMiniRecordWriter>

{
    // Header noch nicht geschrieben?
    if ( !_bHeaderOk )
    {
        // ggf. letzten Content abschlie\sen
        if ( _nContentCount )
            FlushContent_Impl();

        // Content-Offset-Tabelle schreiben
        sal_uInt32 nContentOfsPos = _pStream->Tell();
        //! darf man das so einr"ucken?
        for ( sal_uInt16 n = 0; n < _nContentCount; ++n )
            *_pStream << _aContentOfs[n];

        // SfxMultiFixRecordWriter::Close() "uberspringen!
        sal_uInt32 nEndPos = SfxSingleRecordWriter::Close( sal_False );

        // eigenen Header schreiben
        *_pStream << _nContentCount;
        if ( SFX_REC_TYPE_VARSIZE_RELOC == _nPreTag ||
             SFX_REC_TYPE_MIXTAGS_RELOC == _nPreTag )
            *_pStream << static_cast<sal_uInt32>(nContentOfsPos - ( _pStream->Tell() + sizeof(sal_uInt32) ));
        else
            *_pStream << nContentOfsPos;

        // ans Ende des Records seeken bzw. am Ende des Headers bleiben
        if ( bSeekToEndOfRec )
             _pStream->Seek(nEndPos);
        return nEndPos;
    }

    // Record war bereits vorher geschlossen
    return 0;
}


void SfxMultiMixRecordWriter::NewContent
(
    sal_uInt16      nContentTag,    // Kennung f"ur die Art des Contents
    sal_uInt8       nContentVer     // Kennung f"ur die Version des Contents
)

/*  [Beschreibung]

    Mit dieser Methode wird in den Record ein neuer Content eingef"ugt
    und dessen Content-Tag sowie dessen Content-Version angegeben. Jeder,
    auch der 1. Record mu\s durch Aufruf dieser Methode eingeleitet werden.
*/

{
    // ggf. vorherigen Record abschlie\sen
    if ( _nContentCount )
        FlushContent_Impl();

    // Tag vor den Content schreiben, Version und Startposition merken
    _nContentStartPos = _pStream->Tell();
    ++_nContentCount;
    *_pStream << nContentTag;
    _nContentVer = nContentVer;
}


bool SfxMultiRecordReader::ReadHeader_Impl()

/*  [Beschreibung]

    Interne Methode zum Einlesen eines SfxMultiRecord-Headers, nachdem
    die Basisklasse bereits initialisiert und deren Header gelesen ist.
    Ggf. ist ein Error-Code am Stream gesetzt, im Fehlerfall wird jedoch
    nicht zur"uckge-seekt.
*/

{
    // eigenen Header lesen
    *_pStream >> _nContentCount;
    *_pStream >> _nContentSize; // Fix: jedes einzelnen, Var|Mix: Tabellen-Pos.

    // mu\s noch eine Tabelle mit Content-Offsets geladen werden?
    if ( _nRecordType != SFX_REC_TYPE_FIXSIZE )
    {
        // Tabelle aus dem Stream einlesen
        sal_uInt32 nContentPos = _pStream->Tell();
        if ( _nRecordType == SFX_REC_TYPE_VARSIZE_RELOC ||
             _nRecordType == SFX_REC_TYPE_MIXTAGS_RELOC )
            _pStream->SeekRel( + _nContentSize );
        else
            _pStream->Seek( _nContentSize );
        _pContentOfs = new sal_uInt32[_nContentCount];
        memset(_pContentOfs, 0, _nContentCount*sizeof(sal_uInt32));
        //! darf man jetzt so einr"ucken
        #if defined(OSL_LITENDIAN)
        _pStream->Read( _pContentOfs, sizeof(sal_uInt32)*_nContentCount );
        #else
        for ( sal_uInt16 n = 0; n < _nContentCount; ++n )
            *_pStream >> _pContentOfs[n];
        #endif
        _pStream->Seek( nContentPos );
    }

    // Header konnte gelesen werden, wenn am Stream kein Error gesetzt ist
    return !_pStream->GetError();
}


SfxMultiRecordReader::SfxMultiRecordReader( SvStream *pStream, sal_uInt16 nTag )
    : _pContentOfs(0)
    , _nContentSize(0)
    , _nContentCount(0)
    , _nContentNo(0)
{
    // Position im Stream merken, um im Fehlerfall zur"uck-seeken zu k"onnen
    _nStartPos = pStream->Tell();

    // passenden Record suchen und Basisklasse initialisieren
    SfxSingleRecordReader::Construct_Impl( pStream );
    if ( SfxSingleRecordReader::FindHeader_Impl( SFX_REC_TYPE_FIXSIZE |
            SFX_REC_TYPE_VARSIZE | SFX_REC_TYPE_VARSIZE_RELOC |
            SFX_REC_TYPE_MIXTAGS | SFX_REC_TYPE_MIXTAGS_RELOC,
            nTag ) )
    {
        // eigenen Header dazu-lesen
        if ( !ReadHeader_Impl() )
            // nicht lesbar => als ung"ultig markieren und zur"uck-seeken
            SetInvalid_Impl( _nStartPos);
    }
}


SfxMultiRecordReader::~SfxMultiRecordReader()
{
    delete[] _pContentOfs;
}


bool SfxMultiRecordReader::GetContent()

/*  [Beschreibung]

    Positioniert den Stream an den Anfang des n"chsten bzw. beim 1. Aufruf
    auf den Anfang des ersten Contents im Record und liest ggf. dessen
    Header ein.

    Liegt laut Record-Header kein Content mehr vor, wird sal_False zur"uck-
    gegeben. Trotz einem sal_True-Returnwert kann am Stream ein Fehlercode
    gesetzt sein, z.B. falls er unvorhergesehenerweise (kaputtes File)
    zuende ist.
*/

{
    // noch ein Content vorhanden?
    if ( _nContentNo < _nContentCount )
    {
        // den Stream an den Anfang des Contents positionieren
        sal_uInt32 nOffset = _nRecordType == SFX_REC_TYPE_FIXSIZE
                    ? _nContentNo * _nContentSize
                    : SFX_REC_CONTENT_OFS(_pContentOfs[_nContentNo]);
        sal_uInt32 nNewPos = _nStartPos + nOffset;
        DBG_ASSERT( nNewPos >= _pStream->Tell(), "SfxMultiRecordReader::GetContent() - New position before current, to much data red!" );

        // #99366#: correct stream pos in every case;
        // the if clause was added by MT  a long time ago,
        // maybe to 'repair' other corrupt documents; but this
        // gives errors when writing with 5.1 and reading with current
        // versions, so we decided to remove the if clause (KA-05/17/2002)
        // if ( nNewPos > _pStream->Tell() )
        _pStream->Seek( nNewPos );

        // ggf. Content-Header lesen
        if ( _nRecordType == SFX_REC_TYPE_MIXTAGS ||
             _nRecordType == SFX_REC_TYPE_MIXTAGS_RELOC )
        {
            _nContentVer = sal::static_int_cast< sal_uInt8 >(
                SFX_REC_CONTENT_VER(_pContentOfs[_nContentNo]));
            *_pStream >> _nContentTag;
        }

        // ContentNo weiterz"ahlen
        ++_nContentNo;
        return sal_True;
    }

    return sal_False;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
