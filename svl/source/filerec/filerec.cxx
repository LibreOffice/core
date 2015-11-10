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

#include <sal/config.h>

#include <sal/log.hxx>
#include <svl/filerec.hxx>
#include <osl/endian.h>


/*  The following macros extract parts from a sal_uInt32 value.
    These sal_uInt32 values are written out instead of the individual
    values to reduce the number of calls.
*/

#define SFX_REC_PRE(n) ( ((n) & 0x000000FF) )
#define SFX_REC_OFS(n) ( ((n) & 0xFFFFFF00) >> 8 )
#define SFX_REC_TYP(n) ( ((n) & 0x000000FF) )
#define SFX_REC_TAG(n) ( ((n) & 0xFFFF0000) >> 16 )

#define SFX_REC_CONTENT_VER(n) ( ((n) & 0x000000FF) )
#define SFX_REC_CONTENT_OFS(n) ( ((n) & 0xFFFFFF00) >> 8 )


/*  The following macros combine parts to a sal_uInt32 value.
    This sal_uInt32 value is written instead of the individual values
    to reduce the number of calls.
*/

static void lclWriteMiniHeader(SvStream *p, sal_uInt32 nPreTag, sal_uInt32 nStartPos, sal_uInt32 nEndPos)
{
   (*p).WriteUInt32( sal_uInt32(nPreTag) |
                     sal_uInt32(nEndPos-nStartPos-SFX_REC_HEADERSIZE_MINI) << 8  );
}

static void lclWriteHeader(SvStream *p, sal_uInt32 nRecType, sal_uInt32 nContentTag, sal_uInt32 nContentVer)
{
    (*p).WriteUInt32( sal_uInt32(nRecType) |
                     ( sal_uInt32(nContentVer) << 8 ) |
                     ( sal_uInt32(nContentTag) << 16 )  );
}

#define SFX_REC_CONTENT_HEADER(nContentVer,n1StStartPos,nCurStartPos) \
                    ( sal_uInt32(nContentVer) | \
                      sal_uInt32( nCurStartPos - n1StStartPos ) << 8 )

/** Close the record; write the header
 *
 * @param bSeekToEndOfRec
 * if true (default) the stream is positioned at the end of the record;
 * if false the stream at the start of the content (so after the header).
 *
 * This method closes the record. The main function is to write the header.
 * If the header was written already this method is a no-op.
 *
 * @return sal_uInt32 != 0:  Position im the stream immediately after the record.
 * If 'bSeekToEndOfRecord==sal_True' this will be equal to the current stream position.
 * == 0: The header was already written.
 */
sal_uInt32 SfxMiniRecordWriter::Close(bool bSeekToEndOfRec)
{
    // The header wasn't written yet?
    if ( !_bHeaderOk )
    {
        // Write header at the start of the record
        sal_uInt32 nEndPos = _pStream->Tell();
        _pStream->Seek( _nStartPos );
        lclWriteMiniHeader(_pStream, _nPreTag, _nStartPos, nEndPos );

        // seek to the end of the record or stay where we are
        if ( bSeekToEndOfRec )
            _pStream->Seek( nEndPos );

        // the header has been written NOW
        _bHeaderOk = true;
        return nEndPos;
    }

    // Record was closed already
    return 0;
}

/**
    Internal method for belatedly processsing a header read externally.
    If the header corresponds to an End-Of-Record tag, an error
    code is set on the stream and sal_False is returned.
    But the stream will not be reset to the record start in case of an error.
*/
bool SfxMiniRecordReader::SetHeader_Impl( sal_uInt32 nHeader )
{
    bool bRet = true;

    // determine record end and PreTag from the header
    _nEofRec = _pStream->Tell() + SFX_REC_OFS(nHeader);
    _nPreTag = sal::static_int_cast< sal_uInt8 >(SFX_REC_PRE(nHeader));

    // Error in case of End of Record tag
    if ( _nPreTag == SFX_REC_PRETAG_EOR )
    {
        _pStream->SetError( ERRCODE_IO_WRONGFORMAT );
        bRet = true;
    }
    return bRet;
}

/**
 *
 * @param pstream
 *   an \a SvStream, which has an \a SfxMiniRecord at the current position
 * @param nTag
 *   Pre-Tag of the wanted record
 *
 * This constructor interprets a 'pStream' from the current position
 * as a continuous sequence of records that should be parsable by
 * this group of classes. The first record that is an <SfxMiniRecord>
 * (possibly an extended-Record> that has the PreTag 'nTag' will be opened
 * and represented by this instance.
 *
 * If the end of stream is reached or a record with tag
 * SFX_REC_PRETAG_EOR is seen before a record with the wanted 'nTag'
 * tag is found, the created instance is invalid ('IsValid() ==
 * sal_False').  The ERRCODE_IO_WRONGFORMAT error code will be set on
 * the stream,and the current position will be unchanged.
 *
 * If (the wanted tag) 'nTag==SFX_FILEREC_PRETAG_EOR' no attempt is
 * made to read a record, but 'IsValid()' is set to sal_False immediately.
 * This gives the possibility to include backward compatible SfxMiniRecords
 * without 'new' or 'delete'. See <SfxItemSet::Load()>.
 *
 * Suggested usage:
 *
 * This constructor allows for adding new record types in a backward
 * compatible way by writing out a record with a new tag followed
 * by one with an old tag. In that case previous versions of the program
 * that do not recognise the new tag will skip the new record
 * automatically. This does cause a slight run time inefficiency,
 * compared just starting reading, but if the first record
 * is the wanted one the difference is just a comparison of 2 bytes.
 */

SfxMiniRecordReader::SfxMiniRecordReader(SvStream* pStream, sal_uInt8 nTag)
    : _pStream(pStream)
    , _nEofRec(0)
    , _bSkipped(nTag == SFX_REC_PRETAG_EOR)
{
    // ignore if we are looking for SFX_REC_PRETAG_EOR
    if ( _bSkipped )
    {
        _nPreTag = nTag;
        return;
    }

    // remember StartPos to be able to seek back in case of error
    sal_uInt32 nStartPos = pStream->Tell();

    // look for the matching record
    while(true)
    {
        // read header
        SAL_INFO("svl", "SfxFileRec: searching record at " << pStream->Tell());
        sal_uInt32 nHeader;
        pStream->ReadUInt32( nHeader );

        // let the base class extract the header data
        SetHeader_Impl( nHeader );

        // handle error, if any
        if ( pStream->IsEof() )
            _nPreTag = SFX_REC_PRETAG_EOR;
        else if ( _nPreTag == SFX_REC_PRETAG_EOR )
            pStream->SetError( ERRCODE_IO_WRONGFORMAT );
        else
        {
            // stop the loop if the right tag is found
            if ( _nPreTag == nTag )
                break;

            // or else skip the record and continue
            pStream->Seek( _nEofRec );
            continue;
        }

        // seek back in case of error
        pStream->Seek( nStartPos );
        break;
    }
}

/**
 *
 * @param nRecordType  for sub classes
 * @param pStream      stream to write the record to
 * @param nContentTag  record type
 * @param nContentVer  record version
 *
 * internal constructor for sub classes
 */
SfxSingleRecordWriter::SfxSingleRecordWriter(sal_uInt8  nRecordType,
                                             SvStream*  pStream,
                                             sal_uInt16 nContentTag,
                                             sal_uInt8  nContentVer)
:   SfxMiniRecordWriter( pStream, SFX_REC_PRETAG_EXT )
{
    // write extend header after the SfxMiniRec
    lclWriteHeader(pStream, nRecordType, nContentTag, nContentVer);
}

/**
 *
 * @param nTypes arithmetic OR of allowed record types
 * @param nTag   record tag to find
 *
 * Internal method for reading the header of the first record
 * that has the tag 'nTag', for which then the type should be
 * one of the types in 'nTypes'.
 *
 * If such a record is not found an error code is set, the stream
 * position is seek-ed back and sal_False is returned.
 */
bool SfxSingleRecordReader::FindHeader_Impl(sal_uInt16 nTypes, sal_uInt16 nTag)
{
    // remember StartPos to be able to seek back in case of error
    sal_uInt32 nStartPos = _pStream->Tell();

    // look for the right record
    while ( !_pStream->IsEof() )
    {
        // read header
        sal_uInt32 nHeader;
        SAL_INFO("svl", "SfxFileRec: searching record at " << _pStream->Tell());
        _pStream->ReadUInt32( nHeader );
        if ( !SetHeader_Impl( nHeader ) )
            // EOR => abort loop
            break;

        // found extended record?
        if ( _nPreTag == SFX_REC_PRETAG_EXT )
        {
            // read extended header
            _pStream->ReadUInt32( nHeader );
            _nRecordTag = sal::static_int_cast< sal_uInt16 >(SFX_REC_TAG(nHeader));

            // found right record?
            if ( _nRecordTag == nTag )
            {
                // record type matches as well?
                _nRecordType = sal::static_int_cast< sal_uInt8 >(
                    SFX_REC_TYP(nHeader));
                if ( nTypes & _nRecordType )
                    // ==> found it
                    return true;

                // error => abort loop
                break;
            }
        }

        // else skip
        if ( !_pStream->IsEof() )
            _pStream->Seek( _nEofRec );
    }

    // set error and seek back
    _pStream->SetError( ERRCODE_IO_WRONGFORMAT );
    _pStream->Seek( nStartPos );
    return false;
}

/**
 *
 * @param nRecordType  sub class record type
 * @param pStream      Stream to write the record to
 * @param nContentTag  Content type
 * @param nContentVer  Content version
 *
 * Internal method for sub classes
 */
SfxMultiFixRecordWriter::SfxMultiFixRecordWriter(sal_uInt8  nRecordType,
                                                 SvStream*  pStream,
                                                 sal_uInt16 nContentTag,
                                                 sal_uInt8  nContentVer)
    :  SfxSingleRecordWriter( nRecordType, pStream, nContentTag, nContentVer )
    , _nContentStartPos(0)
    , _nContentSize(0)
    , _nContentCount(0)
{
    // space for own header
    pStream->SeekRel( + SFX_REC_HEADERSIZE_MULTI );
}

/**
 * @see SfxMiniRecordWriter
 */
sal_uInt32 SfxMultiFixRecordWriter::Close( bool bSeekToEndOfRec )
{
    // Header not written yet?
    if ( !_bHeaderOk )
    {
        // remember position after header, to be able to seek back to it
        sal_uInt32 nEndPos = SfxSingleRecordWriter::Close( false );

        // write extended header after SfxSingleRecord
        _pStream->WriteUInt16( _nContentCount );
        _pStream->WriteUInt32( _nContentSize );

        // seek to end of record or stay after the header
        if ( bSeekToEndOfRec )
            _pStream->Seek(nEndPos);
        return nEndPos;
    }

    // Record was closed already
    return 0;
}

/**
 *
 * @param nRecordType  Record type of the sub class
 * @param pStream      stream to write the record to
 * @param nRecordTag   record base type
 * @param nRecordVer   record base version
 *
 * Internal constructor for sub classes
 */
SfxMultiVarRecordWriter::SfxMultiVarRecordWriter(sal_uInt8  nRecordType,
                                                 SvStream*  pStream,
                                                 sal_uInt16 nRecordTag,
                                                 sal_uInt8  nRecordVer)
:   SfxMultiFixRecordWriter( nRecordType, pStream, nRecordTag, nRecordVer ),
    _nContentVer( 0 )
{
}

/**
 *
 * @param pStream,    stream to write the record to
 * @param nRecordTag  record base type
 * @param nRecordVer  record base version
 *
 * Starts an SfxMultiVarRecord in \a pStream, for which the size
 * of the content does not have to be known or identical;
 * after streaming a record its size will be calculated.
 *
 * Note:
 *
 * This method is not inline since too much code was generated
 * for initializing the <SvULong> members.
 */
SfxMultiVarRecordWriter::SfxMultiVarRecordWriter(SvStream*  pStream,
                                                 sal_uInt16 nRecordTag,
                                                 sal_uInt8  nRecordVer)
:   SfxMultiFixRecordWriter( SFX_REC_TYPE_VARSIZE,
                             pStream, nRecordTag, nRecordVer ),
    _nContentVer( 0 )
{
}


/**
 *
 *  The destructor of class <SfxMultiVarRecordWriter> closes the
 *  record automatically, in case <SfxMultiVarRecordWriter::Close()>
 *  has not been called explicitly yet.
 */
SfxMultiVarRecordWriter::~SfxMultiVarRecordWriter()
{
    // close if the header has not been written yet
    if ( !_bHeaderOk )
        Close();
}

/**
 *
 * Internal method for finishing individual content
 */
void SfxMultiVarRecordWriter::FlushContent_Impl()
{
    // record the version and position offset of the current content;
    // the position offset is relative ot the start position of the
    // first content.
    assert(_aContentOfs.size() == static_cast<size_t>(_nContentCount)-1);
    _aContentOfs.resize(_nContentCount-1);
    _aContentOfs.push_back(
            SFX_REC_CONTENT_HEADER(_nContentVer,_nStartPos,_nContentStartPos));
}

/**
 * @see SfxMultiFixRecordWriter
 */
void SfxMultiVarRecordWriter::NewContent()
{
    // written Content already?
    if ( _nContentCount )
        FlushContent_Impl();

    // start new Content
    _nContentStartPos = _pStream->Tell();
    ++_nContentCount;
}

/**
 * @see SfxMiniRecordWriter
 */
sal_uInt32 SfxMultiVarRecordWriter::Close( bool bSeekToEndOfRec )
{
    // Header not written yet?
    if ( !_bHeaderOk )
    {
        // finish content if needed
        if ( _nContentCount )
            FlushContent_Impl();

        // write out content offset table
        sal_uInt32 nContentOfsPos = _pStream->Tell();
        //! (loop without braces)
        for ( sal_uInt16 n = 0; n < _nContentCount; ++n )
            _pStream->WriteUInt32( _aContentOfs[n] );

        // skip SfxMultiFixRecordWriter::Close()!
        sal_uInt32 nEndPos = SfxSingleRecordWriter::Close( false );

        // write own header
        _pStream->WriteUInt16( _nContentCount );
        if ( SFX_REC_TYPE_VARSIZE_RELOC == _nPreTag ||
             SFX_REC_TYPE_MIXTAGS_RELOC == _nPreTag )
            _pStream->WriteUInt32( static_cast<sal_uInt32>(nContentOfsPos - ( _pStream->Tell() + sizeof(sal_uInt32) )) );
        else
            _pStream->WriteUInt32( nContentOfsPos );

        // seek to the end of the record or stay where we are
        if ( bSeekToEndOfRec )
             _pStream->Seek(nEndPos);
        return nEndPos;
    }

    // Record was closed already
    return 0;
}

/**
 *
 * @param nContentTag  tag for this content type
 * @param nContentVer  content version
 *
 * With this method new Content is added to a record and
 * its tag and version are recorded. This method must be called
 * to start each content, including the first record.
 */
void SfxMultiMixRecordWriter::NewContent(sal_uInt16 nContentTag, sal_uInt8 nContentVer)
{
    // Finish the previous record if necessary
    if ( _nContentCount )
        FlushContent_Impl();

    // Write the content tag, and record the version and starting position
    _nContentStartPos = _pStream->Tell();
    ++_nContentCount;
    _pStream->WriteUInt16( nContentTag );
    _nContentVer = nContentVer;
}

/**
 *
 * Internal method for reading an SfxMultiRecord-Headers, after
 * the base class has been initialized and its header has been read.
 * If an error occurs an error code is set on the stream, but
 * the stream position will not be seek-ed back in that case.
 */
bool SfxMultiRecordReader::ReadHeader_Impl()
{
    // read own header
    _pStream->ReadUInt16( _nContentCount );
    _pStream->ReadUInt32( _nContentSize ); // Fix: each on its own, Var|Mix: table position

    // do we still need to read a table with Content offsets?
    if ( _nRecordType != SFX_REC_TYPE_FIXSIZE )
    {
        // read table from the stream
        sal_uInt32 nContentPos = _pStream->Tell();
        if ( _nRecordType == SFX_REC_TYPE_VARSIZE_RELOC ||
             _nRecordType == SFX_REC_TYPE_MIXTAGS_RELOC )
            _pStream->SeekRel( + _nContentSize );
        else
            _pStream->Seek( _nContentSize );
        const size_t nMaxRecords = _pStream->remainingSize() / sizeof(sal_uInt32);
        if (_nContentCount > nMaxRecords)
        {
            SAL_WARN("svl", "Parsing error: " << nMaxRecords << " max possible entries, but " <<
                     _nContentCount << " claimed, truncating");
            _nContentCount = nMaxRecords;
        }
        _pContentOfs = new sal_uInt32[_nContentCount];
        memset(_pContentOfs, 0, _nContentCount*sizeof(sal_uInt32));
        #if defined(OSL_LITENDIAN)
        _pStream->Read( _pContentOfs, sizeof(sal_uInt32)*_nContentCount );
        #else
        // (loop without braces)
        for ( sal_uInt16 n = 0; n < _nContentCount; ++n )
            _pStream->ReadUInt32( _pContentOfs[n] );
        #endif
        _pStream->Seek( nContentPos );
    }

    // It was possible to read the error if no error is set on the stream
    return !_pStream->GetError();
}


SfxMultiRecordReader::SfxMultiRecordReader( SvStream *pStream, sal_uInt16 nTag )
    : _pContentOfs(nullptr)
    , _nContentSize(0)
    , _nContentCount(0)
    , _nContentNo(0)
    , _nContentTag( 0 )
    , _nContentVer( 0 )
{
    // remember position in the stream to be able seek back in case of error
    _nStartPos = pStream->Tell();

    // look for matching record and initialize base class
    SfxSingleRecordReader::Construct_Impl( pStream );
    if ( SfxSingleRecordReader::FindHeader_Impl( SFX_REC_TYPE_FIXSIZE |
            SFX_REC_TYPE_VARSIZE | SFX_REC_TYPE_VARSIZE_RELOC |
            SFX_REC_TYPE_MIXTAGS | SFX_REC_TYPE_MIXTAGS_RELOC,
            nTag ) )
    {
        // also read own header
        if ( !ReadHeader_Impl() )
            // not readable => mark as invalid and reset stream position
            SetInvalid_Impl( _nStartPos);
    }
}


SfxMultiRecordReader::~SfxMultiRecordReader()
{
    delete[] _pContentOfs;
}

/**
 *
 * Positions the stream at the start of the next Content, or
 * for the first call at the start of the first Content in the record,
 * and reads its header if necessary.
 *
 * @return sal_False if there is no further Content according to
 * the record header. Even if sal_True is returned an error can
 * be set on the stream, for instance if the record finished prematurely
 * in a broken file.
 */
bool SfxMultiRecordReader::GetContent()
{
    // more Content available?
    if ( _nContentNo < _nContentCount )
    {
        // position the stream at the start of the Content
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

        // Read Content Header if available
        if ( _nRecordType == SFX_REC_TYPE_MIXTAGS ||
             _nRecordType == SFX_REC_TYPE_MIXTAGS_RELOC )
        {
            _nContentVer = sal::static_int_cast< sal_uInt8 >(
                SFX_REC_CONTENT_VER(_pContentOfs[_nContentNo]));
            _pStream->ReadUInt16( _nContentTag );
        }

        // Increment ContentNo
        ++_nContentNo;
        return true;
    }

    return false;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
