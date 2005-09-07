/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ddectrl.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 21:38:20 $
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

#ifndef _DDECTRL_HXX
#define _DDECTRL_HXX

#ifndef _LINK_HXX //autogen
#include <tools/link.hxx>
#endif
#ifndef _SBERRORS_HXX
#include "sberrors.hxx"
#endif
#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

class DdeConnection;
class DdeConnections;
class DdeData;

class SbiDdeControl
{
private:
    DECL_LINK( Data, DdeData* );
    SbError GetLastErr( DdeConnection* );
    INT16 GetFreeChannel();
    DdeConnections* pConvList;
    String aData;

public:

    SbiDdeControl();
    ~SbiDdeControl();

    SbError Initiate( const String& rService, const String& rTopic,
                     INT16& rnHandle );
    SbError Terminate( INT16 nChannel );
    SbError TerminateAll();
    SbError Request( INT16 nChannel, const String& rItem, String& rResult );
    SbError Execute( INT16 nChannel, const String& rCommand );
    SbError Poke( INT16 nChannel, const String& rItem, const String& rData );
};

#endif
