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

#ifndef _SBIOSYS_HXX
#define _SBIOSYS_HXX

#include <tools/stream.hxx>
#include <basic/sberrors.hxx>

class SvStream;

// Global files (channel numbers 256 to 511) are not
// implemented at the moment.

#define CHANNELS 256
#define CONSOLE  0

#define SBSTRM_INPUT    0x0001
#define SBSTRM_OUTPUT   0x0002
#define SBSTRM_RANDOM   0x0004
#define SBSTRM_APPEND   0x0008
#define SBSTRM_BINARY   0x0010

class SbiStream {
    SvStream* pStrm;
    sal_uIntPtr  nExpandOnWriteTo;  // during writing access expand the stream to this size
    ByteString aLine;
    sal_uIntPtr  nLine;
    short  nLen;                    // buffer length
    short  nMode;
    short  nChan;
    SbError nError;
    void   MapError();

public:
    SbiStream();
   ~SbiStream();
    SbError Open( short, const ByteString&, short, short, short );
    SbError Close();
    SbError Read( ByteString&, sal_uInt16 = 0, bool bForceReadingPerByte=false );
    SbError Read( char& );
    SbError Write( const ByteString&, sal_uInt16 = 0 );

    bool IsText() const     { return (nMode & SBSTRM_BINARY) == 0; }
    bool IsRandom() const   { return (nMode & SBSTRM_RANDOM) != 0; }
    bool IsBinary() const   { return (nMode & SBSTRM_BINARY) != 0; }
    bool IsSeq() const      { return (nMode & SBSTRM_RANDOM) == 0; }
    bool IsAppend() const   { return (nMode & SBSTRM_APPEND) != 0; }
    short GetBlockLen() const          { return nLen;           }
    short GetMode() const              { return nMode;          }
    sal_uIntPtr GetLine() const            { return nLine;          }
    void SetExpandOnWriteTo( sal_uIntPtr n ) { nExpandOnWriteTo = n;    }
    void ExpandFile();
    SvStream* GetStrm()                { return pStrm;          }
};

class SbiIoSystem {
    SbiStream* pChan[ CHANNELS ];
    ByteString  aPrompt;
    ByteString  aIn, aOut;
    short     nChan;
    SbError   nError;
    void      ReadCon( ByteString& );
    void      WriteCon( const ByteString& );
public:
    SbiIoSystem();
   ~SbiIoSystem();
    SbError GetError();
    void  Shutdown();
    void  SetPrompt( const ByteString& r ) { aPrompt = r; }
    void  SetChannel( short n  )       { nChan = n;   }
    short GetChannel() const           { return nChan;}
    void  ResetChannel()               { nChan = 0;   }
    void  Open( short, const ByteString&, short, short, short );
    void  Close();
    void  Read( ByteString&, short = 0 );
    char  Read();
    void  Write( const ByteString&, short = 0 );
    short NextChannel();
    // 0 == bad channel or no SvStream (nChannel=0..CHANNELS-1)
    SbiStream* GetStream( short nChannel ) const;
    void  CloseAll(); // JSM
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
