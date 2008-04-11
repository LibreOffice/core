/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: scmdstrm.hxx,v $
 * $Revision: 1.5 $
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
#ifndef _SCMDSTRM_HXX
#define _SCMDSTRM_HXX

#include <tools/solar.h>
#include <com/sun/star/beans/PropertyValue.hpp>

#include "cmdbasestream.hxx"

class SvStream;
class SfxPoolItem;
class String;
class ICommStream;

class SCmdStream: public CmdBaseStream
{
    SvStream *pSammel;

public:
    SCmdStream( SvStream *pIn );
    ~SCmdStream();

    using CmdBaseStream::Read;
    void Read ( comm_USHORT &nNr ){CmdBaseStream::Read ( nNr );}
    void Read ( comm_ULONG &nNr ){CmdBaseStream::Read ( nNr );}
//  void Read ( comm_UniChar* &aString, comm_USHORT &nLenInChars ){CmdBaseStream::Read ( aString, nLenInChars );}
    void Read ( comm_BOOL &bBool ){CmdBaseStream::Read ( bBool );}
//  new
    void Read ( String &aString );
    void Read ( SfxPoolItem *&pItem );
    void Read ( ::com::sun::star::beans::PropertyValue &rItem );

    virtual void Read (String* &pString);
};

#endif
