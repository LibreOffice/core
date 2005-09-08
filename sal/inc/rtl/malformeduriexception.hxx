/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: malformeduriexception.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 14:39:15 $
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

#ifndef INCLUDED_RTL_MALFORMEDURIEXCEPTION_HXX
#define INCLUDED_RTL_MALFORMEDURIEXCEPTION_HXX

#ifndef _RTL_USTRING_HXX_
#include "rtl/ustring.hxx"
#endif

namespace rtl {

/** An exception indicating a malformed URI.

    <P>Used when parsing (part of) a URI fails for syntactical reasons.</P>
 */
class MalformedUriException
{
public:
    /** Create a MalformedUriException.

        @param rMessage
        A message containing any details about the exception.
     */
    inline MalformedUriException(rtl::OUString const & rMessage):
        m_aMessage(rMessage) {}

    /** Get the message.

        @return
        A reference to the message.  The reference is valid for the lifetime of
        this MalformedUriException.
     */
    inline rtl::OUString const & getMessage() const { return m_aMessage; }

private:
    rtl::OUString m_aMessage;
};

}

#endif // INCLUDED_RTL_MALFORMEDURIEXCEPTION_HXX
