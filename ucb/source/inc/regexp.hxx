/*************************************************************************
 *
 *  $RCSfile: regexp.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kso $ $Date: 2000-10-16 14:53:07 $
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

#ifndef _UCB_REGEXP_HXX_
#define _UCB_REGEXP_HXX_

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif
#ifndef _VOS_DIAGNOSE_H_
#include <vos/diagnose.hxx>
#endif

//============================================================================
namespace ucb {

class Regexp
{
public:
    enum Kind
    {
        KIND_PREFIX,
        KIND_AUTHORITY,
        KIND_DOMAIN
    };

    inline bool operator ==(Regexp const & rOther) const;

    inline bool isDefault() const
    { return m_eKind == KIND_PREFIX && m_aPrefix.getLength() == 0; }

    inline Kind getKind() const { return m_eKind; }

    bool matches(rtl::OUString const & rString, rtl::OUString * pTranslation,
                 bool * pTranslated) const;

    rtl::OUString getRegexp(bool bReverse) const;

    static Regexp parse(rtl::OUString const & rRegexp);

private:
    Kind m_eKind;
    rtl::OUString m_aPrefix;
    rtl::OUString m_aInfix;
    rtl::OUString m_aReversePrefix;
    bool m_bEmptyDomain;
    bool m_bTranslation;

    inline Regexp(Kind eTheKind, rtl::OUString const & rThePrefix,
                  bool bTheEmptyDomain, rtl::OUString const & rTheInfix,
                  bool bTheTranslation,
                  rtl::OUString const & rTheReversePrefix);
};

inline bool Regexp::operator ==(Regexp const & rOther) const
{
    return m_eKind == rOther.m_eKind
           && m_aPrefix == rOther.m_aPrefix
           && m_aInfix == rOther.m_aInfix;
}

}

#endif // _UCPRMT_RMTREGX_HXX_

