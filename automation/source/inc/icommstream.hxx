/*************************************************************************
 *
 *  $RCSfile: icommstream.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: mh $ $Date: 2002-11-18 11:12:07 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2002
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2002 by Sun Microsystems, Inc.
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2002 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

/*************************************************************************
 *
 *    ATTENTION
 *    This file is intended to work inside and outside the StarOffice environment.
 *    Only adaption of file commtypes.hxx should be necessary. Else it is a bug!
 *
 ************************************************************************/

#ifndef _AUTOMATION_ICOMMSTREAM_HXX_
#define _AUTOMATION_ICOMMSTREAM_HXX_

#include "commtypes.hxx"

class ICommStream
{
public:

    ICommStream(){}
    virtual ~ICommStream(){}

    virtual ICommStream&    operator>>( comm_USHORT& rUShort )=0;
    virtual ICommStream&    operator>>( comm_ULONG& rULong )=0;
    virtual ICommStream&    operator>>( comm_BOOL& rChar )=0;

    virtual ICommStream&    operator<<( comm_USHORT nUShort )=0;
    virtual ICommStream&    operator<<( comm_ULONG nULong )=0;
    virtual ICommStream&    operator<<( comm_BOOL nChar )=0;

    virtual comm_ULONG          Read( void* pData, comm_ULONG nSize )=0;
    virtual comm_ULONG          Write( const void* pData, comm_ULONG nSize )=0;

    virtual comm_BOOL       IsEof() const=0;
    virtual comm_ULONG      SeekRel( long nPos )=0;

};

#endif
