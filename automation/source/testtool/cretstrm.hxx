/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: cretstrm.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 19:31:21 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CRETSTRM_HXX
#define _CRETSTRM_HXX

#ifndef _STREAM_HXX //autogen
#include <tools/stream.hxx>
#endif
#ifndef _SBXVAR_HXX //autogen
#include <basic/sbxvar.hxx>
#endif

#include "cmdbasestream.hxx"

class CRetStream : public CmdBaseStream
{
    SvStream *pSammel;
    USHORT nId;

public:
    CRetStream( SvStream *pIn );
    ~CRetStream();

//  CmdBaseStream::Read;
    void Read ( comm_USHORT &nNr ){CmdBaseStream::Read ( nNr );}
    void Read ( comm_ULONG &nNr ){CmdBaseStream::Read ( nNr );}
//  void Read ( comm_UniChar* &aString, comm_USHORT &nLenInChars ){CmdBaseStream::Read ( aString, nLenInChars );}
    virtual void Read ( SmartId* &pId ){CmdBaseStream::Read ( pId );}
    void Read ( comm_BOOL &bBool ){CmdBaseStream::Read ( bBool );}
//  new
    void Read( String &aString );
    void Read( SbxValue &aValue );
};

#endif
