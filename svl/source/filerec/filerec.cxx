/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
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

static void lclWriteMiniHeader(SvStream *p, sal_uInt32 nPreTag, sal_uInt32 nStartPos, sal_uInt32 nEndPos)
{
   (*p).WriteUInt32(  sal_uInt32(nPreTag) |
             sal_uInt32(nEndPos-nStartPos-SFX_REC_HEADERSIZE_MINI) << 8  );
}

static void lclWriteHeader(SvStream *p, sal_uInt32 nRecType, sal_uInt32 nContentTag, sal_uInt32 nContentVer)
{
    (*p).WriteUInt32(  sal_uInt32(nRecType) |
             ( sal_uInt32(nContentVer) << 8 ) |
             ( sal_uInt32(nContentTag) << 16 )  );
}

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
    
    if ( !_bHeaderOk )
    {
        
        sal_uInt32 nEndPos = _pStream->Tell();
        _pStream->Seek( _nStartPos );
        lclWriteMiniHeader(_pStream, _nPreTag, _nStartPos, nEndPos );

        
        if ( bSeekToEndOfRec )
            _pStream->Seek( nEndPos );

        
        _bHeaderOk = true;
        return nEndPos;
    }

    
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

    
    _nEofRec = _pStream->Tell() + SFX_REC_OFS(nHeader);
    _nPreTag = sal::static_int_cast< sal_uInt8 >(SFX_REC_PRE(nHeader));

    
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
    sal_uInt8           nTag            
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
    
    if ( _bSkipped )
    {
        _nPreTag = nTag;
        return;
    }

    
    sal_uInt32 nStartPos = pStream->Tell();

    
    while(true)
    {
        
        DBG( DbgOutf( "SfxFileRec: searching record at %ul", pStream->Tell() ) );
        sal_uInt32 nHeader;
        pStream->ReadUInt32( nHeader );

        
        SetHeader_Impl( nHeader );

        
        if ( pStream->IsEof() )
            _nPreTag = SFX_REC_PRETAG_EOR;
        else if ( _nPreTag == SFX_REC_PRETAG_EOR )
            pStream->SetError( ERRCODE_IO_WRONGFORMAT );
        else
        {
            
            if ( _nPreTag == nTag )
                break;

            
            pStream->Seek( _nEofRec );
            continue;
        }

        
        pStream->Seek( nStartPos );
        break;
    }
}


SfxSingleRecordWriter::SfxSingleRecordWriter
(
    sal_uInt8           nRecordType,    
    SvStream*       pStream,        
    sal_uInt16          nContentTag,    
    sal_uInt8           nContentVer     
)

/*  [Beschreibung]

    Interner Ctor f"ur Subklassen.
*/

:   SfxMiniRecordWriter( pStream, SFX_REC_PRETAG_EXT )
{
    
    lclWriteHeader(pStream, nRecordType, nContentTag, nContentVer);
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

    
    sal_uInt32 nHeader=0;
    _pStream->ReadUInt32( nHeader );
    if ( !SetHeader_Impl( nHeader ) )
        bRet = false;
    else
    {
        
        _pStream->ReadUInt32( nHeader );
        _nRecordVer = sal::static_int_cast< sal_uInt8 >(SFX_REC_VER(nHeader));
        _nRecordTag = sal::static_int_cast< sal_uInt16 >(SFX_REC_TAG(nHeader));

        
        _nRecordType = sal::static_int_cast< sal_uInt8 >(SFX_REC_TYP(nHeader));
        bRet = 0 != ( nTypes & _nRecordType);
    }
    return bRet;
}


bool SfxSingleRecordReader::FindHeader_Impl
(
    sal_uInt16      nTypes,     
    sal_uInt16      nTag        
)

/*  [Beschreibung]

    Interne Methode zum lesen des Headers des ersten Record, der einem
    der Typen in 'nTypes' entspricht und mit der Art-Kennung 'nTag'
    gekennzeichnet ist.

    Kann ein solcher Record nicht gefunden werden, wird am Stream ein
    Errorcode gesetzt, zur"uck-geseekt und sal_False zur"uckgeliefert.
*/

{
    
    sal_uInt32 nStartPos = _pStream->Tell();

    
    while ( !_pStream->IsEof() )
    {
        
        sal_uInt32 nHeader;
        DBG( DbgOutf( "SfxFileRec: searching record at %ul", _pStream->Tell() ) );
        _pStream->ReadUInt32( nHeader );
        if ( !SetHeader_Impl( nHeader ) )
            
            break;

        
        if ( _nPreTag == SFX_REC_PRETAG_EXT )
        {
            
            _pStream->ReadUInt32( nHeader );
            _nRecordTag = sal::static_int_cast< sal_uInt16 >(SFX_REC_TAG(nHeader));

            
            if ( _nRecordTag == nTag )
            {
                
                _nRecordType = sal::static_int_cast< sal_uInt8 >(
                    SFX_REC_TYP(nHeader));
                if ( nTypes & _nRecordType )
                    
                    return true;

                
                break;
            }
        }

        
        if ( !_pStream->IsEof() )
            _pStream->Seek( _nEofRec );
    }

    
    _pStream->SetError( ERRCODE_IO_WRONGFORMAT );
    _pStream->Seek( nStartPos );
    return false;
}


SfxMultiFixRecordWriter::SfxMultiFixRecordWriter
(
    sal_uInt8           nRecordType,    
    SvStream*       pStream,        
    sal_uInt16          nContentTag,    
    sal_uInt8           nContentVer     
)

/*  [Beschreibung]

    Interne Methode f"ur Subklassen.
*/

    :  SfxSingleRecordWriter( nRecordType, pStream, nContentTag, nContentVer )
    , _nContentStartPos(0)
    , _nContentSize(0)
    , _nContentCount(0)
{
    
    pStream->SeekRel( + SFX_REC_HEADERSIZE_MULTI );
}


sal_uInt32 SfxMultiFixRecordWriter::Close( bool bSeekToEndOfRec )



{
    
    if ( !_bHeaderOk )
    {
        
        sal_uInt32 nEndPos = SfxSingleRecordWriter::Close( false );

        
        _pStream->WriteUInt16( _nContentCount );
        _pStream->WriteUInt32( _nContentSize );

        
        if ( bSeekToEndOfRec )
            _pStream->Seek(nEndPos);
        return nEndPos;
    }

    
    return 0;
}


SfxMultiVarRecordWriter::SfxMultiVarRecordWriter
(
    sal_uInt8           nRecordType,    
    SvStream*       pStream,        
    sal_uInt16          nRecordTag,     
    sal_uInt8           nRecordVer      
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
    SvStream*       pStream,        
    sal_uInt16          nRecordTag,     
    sal_uInt8           nRecordVer      
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
    
    if ( !_bHeaderOk )
        Close();
}


void SfxMultiVarRecordWriter::FlushContent_Impl()

/*  [Beschreibung]

    Interne Methode zum Abschlie\sen eines einzelnen Contents.
*/

{
    
    
    assert(_aContentOfs.size() == static_cast<size_t>(_nContentCount)-1);
    _aContentOfs.resize(_nContentCount-1);
    _aContentOfs.push_back(
            SFX_REC_CONTENT_HEADER(_nContentVer,_nStartPos,_nContentStartPos));
}


void SfxMultiVarRecordWriter::NewContent()



{
    
    if ( _nContentCount )
        FlushContent_Impl();

    
    _nContentStartPos = _pStream->Tell();
    ++_nContentCount;
}


sal_uInt32 SfxMultiVarRecordWriter::Close( bool bSeekToEndOfRec )



{
    
    if ( !_bHeaderOk )
    {
        
        if ( _nContentCount )
            FlushContent_Impl();

        
        sal_uInt32 nContentOfsPos = _pStream->Tell();
        
        for ( sal_uInt16 n = 0; n < _nContentCount; ++n )
            _pStream->WriteUInt32( _aContentOfs[n] );

        
        sal_uInt32 nEndPos = SfxSingleRecordWriter::Close( false );

        
        _pStream->WriteUInt16( _nContentCount );
        if ( SFX_REC_TYPE_VARSIZE_RELOC == _nPreTag ||
             SFX_REC_TYPE_MIXTAGS_RELOC == _nPreTag )
            _pStream->WriteUInt32( static_cast<sal_uInt32>(nContentOfsPos - ( _pStream->Tell() + sizeof(sal_uInt32) )) );
        else
            _pStream->WriteUInt32( nContentOfsPos );

        
        if ( bSeekToEndOfRec )
             _pStream->Seek(nEndPos);
        return nEndPos;
    }

    
    return 0;
}


void SfxMultiMixRecordWriter::NewContent
(
    sal_uInt16      nContentTag,    
    sal_uInt8       nContentVer     
)

/*  [Beschreibung]

    Mit dieser Methode wird in den Record ein neuer Content eingef"ugt
    und dessen Content-Tag sowie dessen Content-Version angegeben. Jeder,
    auch der 1. Record mu\s durch Aufruf dieser Methode eingeleitet werden.
*/

{
    
    if ( _nContentCount )
        FlushContent_Impl();

    
    _nContentStartPos = _pStream->Tell();
    ++_nContentCount;
    _pStream->WriteUInt16( nContentTag );
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
    
    _pStream->ReadUInt16( _nContentCount );
    _pStream->ReadUInt32( _nContentSize ); 

    
    if ( _nRecordType != SFX_REC_TYPE_FIXSIZE )
    {
        
        sal_uInt32 nContentPos = _pStream->Tell();
        if ( _nRecordType == SFX_REC_TYPE_VARSIZE_RELOC ||
             _nRecordType == SFX_REC_TYPE_MIXTAGS_RELOC )
            _pStream->SeekRel( + _nContentSize );
        else
            _pStream->Seek( _nContentSize );
        _pContentOfs = new sal_uInt32[_nContentCount];
        memset(_pContentOfs, 0, _nContentCount*sizeof(sal_uInt32));
        
        #if defined(OSL_LITENDIAN)
        _pStream->Read( _pContentOfs, sizeof(sal_uInt32)*_nContentCount );
        #else
        for ( sal_uInt16 n = 0; n < _nContentCount; ++n )
            *_pStream >> _pContentOfs[n];
        #endif
        _pStream->Seek( nContentPos );
    }

    
    return !_pStream->GetError();
}


SfxMultiRecordReader::SfxMultiRecordReader( SvStream *pStream, sal_uInt16 nTag )
    : _pContentOfs(0)
    , _nContentSize(0)
    , _nContentCount(0)
    , _nContentNo(0)
{
    
    _nStartPos = pStream->Tell();

    
    SfxSingleRecordReader::Construct_Impl( pStream );
    if ( SfxSingleRecordReader::FindHeader_Impl( SFX_REC_TYPE_FIXSIZE |
            SFX_REC_TYPE_VARSIZE | SFX_REC_TYPE_VARSIZE_RELOC |
            SFX_REC_TYPE_MIXTAGS | SFX_REC_TYPE_MIXTAGS_RELOC,
            nTag ) )
    {
        
        if ( !ReadHeader_Impl() )
            
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
    
    if ( _nContentNo < _nContentCount )
    {
        
        sal_uInt32 nOffset = _nRecordType == SFX_REC_TYPE_FIXSIZE
                    ? _nContentNo * _nContentSize
                    : SFX_REC_CONTENT_OFS(_pContentOfs[_nContentNo]);
        sal_uInt32 nNewPos = _nStartPos + nOffset;
        DBG_ASSERT( nNewPos >= _pStream->Tell(), "SfxMultiRecordReader::GetContent() - New position before current, to much data red!" );

        
        
        
        
        
        
        _pStream->Seek( nNewPos );

        
        if ( _nRecordType == SFX_REC_TYPE_MIXTAGS ||
             _nRecordType == SFX_REC_TYPE_MIXTAGS_RELOC )
        {
            _nContentVer = sal::static_int_cast< sal_uInt8 >(
                SFX_REC_CONTENT_VER(_pContentOfs[_nContentNo]));
            _pStream->ReadUInt16( _nContentTag );
        }

        
        ++_nContentNo;
        return true;
    }

    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
