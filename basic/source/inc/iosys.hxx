/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



#ifndef _SBIOSYS_HXX
#define _SBIOSYS_HXX

#include <tools/stream.hxx>
#ifndef _SBERRORS_HXX
#include <basic/sberrors.hxx>
#endif

class SvStream;

// Zur Zeit sind globale Dateien (Kanalnummern 256 bis 511)
// nicht implementiert.

#define CHANNELS 256
#define CONSOLE  0

#define SBSTRM_INPUT    0x0001      // Input
#define SBSTRM_OUTPUT   0x0002      // Output
#define SBSTRM_RANDOM   0x0004      // Random
#define SBSTRM_APPEND   0x0008      // Append
#define SBSTRM_BINARY   0x0010      // Binary

class SbiStream {
    SvStream* pStrm;                // der Stream
    sal_uIntPtr  nExpandOnWriteTo;      // bei Schreibzugriff, den Stream
                                    // bis zu dieser Groesse aufblasen
    ByteString aLine;               // aktuelle Zeile
    sal_uIntPtr  nLine;                 // aktuelle Zeilennummer
    short  nLen;                    // Pufferlaenge
    short  nMode;                   // Bits:
    short  nChan;                   // aktueller Kanal
    SbError nError;                 // letzter Fehlercode
    void   MapError();              // Fehlercode mappen

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
    ByteString  aPrompt;            // Input-Prompt
    ByteString  aIn, aOut;          // Console-Buffer
    short     nChan;                // aktueller Kanal
    SbError   nError;               // letzter Fehlercode
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

