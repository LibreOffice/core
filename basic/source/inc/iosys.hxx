/*************************************************************************
 *
 *  $RCSfile: iosys.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:12:10 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SBIOSYS_HXX
#define _SBIOSYS_HXX

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SBERRORS_HXX
#include "sberrors.hxx"
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
    ULONG  nExpandOnWriteTo;        // bei Schreibzugriff, den Stream
                                    // bis zu dieser Groesse aufblasen
    ByteString aLine;               // aktuelle Zeile
    ULONG  nLine;                   // aktuelle Zeilennummer
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
    SbError Read( ByteString&, USHORT = 0 );
    SbError Read( char& );
    SbError Write( const ByteString&, USHORT = 0 );

    BOOL IsText() const     { return !(nMode & SBSTRM_BINARY);  }
    BOOL IsRandom() const   { return  (nMode & SBSTRM_RANDOM);  }
    BOOL IsBinary() const   { return  (nMode & SBSTRM_BINARY);  }
    BOOL IsSeq() const      { return !(nMode & SBSTRM_RANDOM);  }
    BOOL IsAppend() const   { return  (nMode & SBSTRM_APPEND);  }
    short GetBlockLen() const          { return nLen;           }
    short GetMode() const              { return nMode;          }
    ULONG GetLine() const              { return nLine;          }
    void SetExpandOnWriteTo( ULONG n ) { nExpandOnWriteTo = n;  }
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

