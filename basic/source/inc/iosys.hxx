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

#pragma once

#include <memory>
#include <string_view>

#include <tools/stream.hxx>
#include <o3tl/typed_flags_set.hxx>

class SvStream;

// Global files (channel numbers 256 to 511) are not
// implemented at the moment.

#define CHANNELS 256

enum class SbiStreamFlags
{
    NONE     = 0x0000,
    Input    = 0x0001,
    Output   = 0x0002,
    Random   = 0x0004,
    Append   = 0x0008,
    Binary   = 0x0010,
};
namespace o3tl
{
    template<> struct typed_flags<SbiStreamFlags> : is_typed_flags<SbiStreamFlags, 0x1f> {};
}

class SbiStream
{
    std::unique_ptr<SvStream> pStrm;
    sal_uInt64  nExpandOnWriteTo;  // during writing access expand the stream to this size
    OString aLine;
    sal_uInt64  nLine;
    short  nLen;                    // buffer length
    SbiStreamFlags  nMode;
    ErrCode nError;
    void   MapError();

public:
    SbiStream();
   ~SbiStream();
    ErrCode const & Open( std::string_view, StreamMode, SbiStreamFlags, short );
    ErrCode const & Close();
    ErrCode Read(OString&, sal_uInt16 = 0, bool bForceReadingPerByte=false);
    ErrCode const & Read( char& );
    ErrCode Write( const OString& );

    bool IsText() const     { return !bool(nMode & SbiStreamFlags::Binary); }
    bool IsRandom() const   { return bool(nMode & SbiStreamFlags::Random); }
    bool IsBinary() const   { return bool(nMode & SbiStreamFlags::Binary); }
    bool IsSeq() const      { return !bool(nMode & SbiStreamFlags::Random); }
    bool IsAppend() const   { return bool(nMode & SbiStreamFlags::Append); }
    short GetBlockLen() const          { return nLen;           }
    SbiStreamFlags GetMode() const              { return nMode;          }
    sal_uInt64 GetLine() const            { return nLine;          }
    void SetExpandOnWriteTo( sal_uInt64 n ) { nExpandOnWriteTo = n;    }
    void ExpandFile();
    SvStream* GetStrm()                { return pStrm.get();        }
};

class SbiIoSystem
{
    SbiStream* pChan[ CHANNELS ];
    OString aPrompt;
    OString aIn;
    OUString aOut;
    short     nChan;
    ErrCode   nError;
    void      ReadCon(OString&);
    void      WriteCon(std::u16string_view);
public:
    SbiIoSystem();
   ~SbiIoSystem() COVERITY_NOEXCEPT_FALSE;
    ErrCode GetError();
    void  Shutdown();
    void  SetPrompt(const OString& r) { aPrompt = r; }
    void  SetChannel( short n  )       { nChan = n;   }
    short GetChannel() const           { return nChan;}
    void  ResetChannel()               { nChan = 0;   }
    void  Open( short, std::string_view, StreamMode, SbiStreamFlags, short );
    void  Close();
    void  Read(OString&);
    char  Read();
    void  Write(std::u16string_view);
    // 0 == bad channel or no SvStream (nChannel=0..CHANNELS-1)
    SbiStream* GetStream( short nChannel ) const;
    void  CloseAll(); // JSM
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
