/*************************************************************************
 *
 *  $RCSfile: adrparse.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:49 $
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

#ifndef _ADRPARSE_HXX
#define _ADRPARSE_HXX

#ifndef _LIST_HXX
#include <tools/list.hxx>
#endif
#ifndef _STRING_HXX
#include <tools/string.hxx>
#endif

//============================================================================
struct SvAddressEntry_Impl
{
    UniString m_aAddrSpec;
    UniString m_aRealName;

    SvAddressEntry_Impl() {};
    SvAddressEntry_Impl(UniString const & rTheAddrSpec,
                        UniString const & rTheRealName):
        m_aAddrSpec(rTheAddrSpec), m_aRealName(rTheRealName) {}
};

//============================================================================
DECLARE_LIST(SvAddressList_Impl, SvAddressEntry_Impl *);

//============================================================================
class SvAddressParser
{
    friend class SvAddressParser_Impl;

    SvAddressEntry_Impl m_aFirst;
    SvAddressList_Impl m_aRest;
    bool m_bHasFirst;

public:
    SvAddressParser(UniString const & rInput);

    ~SvAddressParser();

    sal_Int32 Count() const { return m_bHasFirst ? m_aRest.Count() + 1 : 0; }

    inline UniString const & GetEmailAddress(sal_Int32 nIndex) const;

    inline UniString const &GetRealName(sal_Int32 nIndex) const;

    /** Create an RFC 822 <mailbox> (i.e., 'e-mail address').

        @param rPhrase  Either an empty string (the <mailbox> will have no
        <phrase> an will be of the form <addr-spec>), or some text that will
        become the <phrase> part of a <phrase route-addr> form <mailbox>.  Non
        US-ASCII characters within the text are put into a <qouted-string>
        verbatim, so the result may actually not be a valid RFC 822 <mailbox>,
        but a more human-readable representation.

        @param rAddrSpec  A valid RFC 822 <addr-spec>.  (An RFC 822 <mailbox>
        including a <route> cannot be created by this method.)

        @param rMailbox  If this method returns true, this parameter returns
        the created RFC 822 <mailbox> (rather, a more human-readable
        representation thereof).  Otherwise, this parameter is not modified.

        @return  True, if rAddrSpec is a valid RFC 822 <addr-spec>.
     */
    static bool createRFC822Mailbox(String const & rPhrase,
                                    String const & rAddrSpec,
                                    String & rMailbox);
};

inline UniString const & SvAddressParser::GetEmailAddress(sal_Int32 nIndex)
    const
{
    return nIndex == 0 ? m_aFirst.m_aAddrSpec :
                         m_aRest.GetObject(nIndex - 1)->m_aAddrSpec;
}

inline UniString const & SvAddressParser::GetRealName(sal_Int32 nIndex) const
{
    return nIndex == 0 ? m_aFirst.m_aRealName :
                         m_aRest.GetObject(nIndex - 1)->m_aRealName;
}

#endif // _ADRPARSE_HXX

