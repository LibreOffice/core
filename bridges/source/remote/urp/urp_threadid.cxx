/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: urp_threadid.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-07 22:49:26 $
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
#include "urp_threadid.hxx"

#include <rtl/strbuf.hxx>

using namespace rtl;

namespace bridges_urp
{
    rtl::OString byteSequence2HumanReadableString( const rtl::ByteSequence &a )
    {
        const sal_uInt8 *p = (const sal_uInt8 * ) a.getConstArray();
        sal_Int32 nLength = a.getLength();
        OStringBuffer buf( a.getLength() * 2 + 2 );
        buf.append( RTL_CONSTASCII_STRINGPARAM( "0x" ) );
        for( sal_Int32 i = 0 ; i < nLength ; i ++ )
            buf.append( (sal_Int32)  p[i] , 16 );
        return buf.makeStringAndClear();
    }
}
