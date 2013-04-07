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

#ifndef _SBIOSYS_HXX
#define _SBIOSYS_HXX

#include <tools/stream.hxx>
#include <basic/sberrors.hxx>

class SvStream;

// Global files (channel numbers 256 to 511) are not
// implemented at the moment.

#define CHANNELS 256

#define SBSTRM_INPUT    0x0001
#define SBSTRM_OUTPUT   0x0002
#define SBSTRM_RANDOM   0x0004
#define SBSTRM_APPEND   0x0008
#define SBSTRM_BINARY   0x0010

class SbiStream
{
    SvStream* pStrm;
    sal_uIntPtr  nExpandOnWriteTo;  // during writing access expand the stream to this size
    OString aLine;
    sal_uIntPtr  nLine;
    short  nLen;                    // buffer length
    short  nMode;
    short  nChan;
    SbError nError;
    void   MapError();

public:
    SbiStream();
   ~SbiStream();
    SbError Open( short, const OString&, short, short, short );
    SbError Close();
    SbError Read(OString&, sal_uInt16 = 0, bool bForceReadingPerByte=false);
    SbError Read( char& );
    SbError Write( const OString&, sal_uInt16 = 0 );

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

class SbiIoSystem
{
    SbiStream* pChan[ CHANNELS ];
    OString aPrompt;
    OString aIn;
    OString aOut;
    short     nChan;
    SbError   nError;
    void      ReadCon(OString&);
    void      WriteCon(const OString&);
public:
    SbiIoSystem();
   ~SbiIoSystem();
    SbError GetError();
    void  Shutdown();
    void  SetPrompt(const OString& r) { aPrompt = r; }
    void  SetChannel( short n  )       { nChan = n;   }
    short GetChannel() const           { return nChan;}
    void  ResetChannel()               { nChan = 0;   }
    void  Open( short, const OString&, short, short, short );
    void  Close();
    void  Read(OString&, short = 0);
    char  Read();
    void  Write(const OString&, short = 0);
    // 0 == bad channel or no SvStream (nChannel=0..CHANNELS-1)
    SbiStream* GetStream( short nChannel ) const;
    void  CloseAll(); // JSM
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
