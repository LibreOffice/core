/*************************************************************************
 *
 *  $RCSfile: XMLTextNumRuleInfo.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: mib $ $Date: 2000-10-31 09:00:40 $
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

#ifndef _XMLOFF_XMLTEXTNUMRULEINFO_HXX
#define _XMLOFF_XMLTEXTNUMRULEINFO_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_
#include <com/sun/star/container/XIndexReplace.hpp>
#endif

namespace com { namespace sun { namespace star {
    namespace text { class XTextContent; }
} } }

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif

class XMLTextNumRuleInfo
{
    const ::rtl::OUString sNumberingRules;
    const ::rtl::OUString sNumberingLevel;
    const ::rtl::OUString sNumberingStartValue;
    const ::rtl::OUString sParaIsNumberingRestart;
    const ::rtl::OUString sNumberingType;
    const ::rtl::OUString sIsNumbering;
    const ::rtl::OUString sNumberingIsNumber;

    ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexReplace > xNumRules;

    ::rtl::OUString     sName;
    sal_Int16           nStartValue;
    sal_Int16           nLevel;

    sal_Bool            bIsNumbered : 1;
    sal_Bool            bIsOrdered : 1;
    sal_Bool            bIsRestart : 1;
    sal_Bool            bIsNamed : 1;

public:

    XMLTextNumRuleInfo();

    inline XMLTextNumRuleInfo& operator=( const XMLTextNumRuleInfo& rInfo );

    void Set(
        const ::com::sun::star::uno::Reference <
            ::com::sun::star::text::XTextContent > & rTextContnt );
    inline void Reset();

    const ::rtl::OUString& GetName() const { return sName; }
    const ::com::sun::star::uno::Reference <
        ::com::sun::star::container::XIndexReplace >& GetNumRules() const
        { return xNumRules; }
    sal_Int16 GetLevel() const { return nLevel; }

    sal_Bool HasStartValue() const { return nStartValue != -1; }
    sal_uInt32 GetStartValue() const { return nStartValue; }

    sal_Bool IsNumbered() const { return bIsNumbered; }
    sal_Bool IsOrdered() const { return bIsOrdered; }
    sal_Bool IsRestart() const { return bIsRestart; }
    sal_Bool IsNamed() const { return bIsNamed; }

    inline sal_Bool HasSameNumRules( const XMLTextNumRuleInfo& rCmp ) const;
};

inline XMLTextNumRuleInfo& XMLTextNumRuleInfo::operator=(
        const XMLTextNumRuleInfo& rInfo )
{
    sName = rInfo.sName;
    xNumRules = rInfo.xNumRules;
    nStartValue = rInfo.nStartValue;
    nLevel = rInfo.nLevel;
    bIsNumbered = rInfo.bIsNumbered;
    bIsOrdered = rInfo.bIsOrdered;
    bIsRestart = rInfo.bIsRestart;
    bIsNamed = rInfo.bIsNamed;

    return *this;
}

inline void XMLTextNumRuleInfo::Reset()
{
    sName = ::rtl::OUString();
    xNumRules = 0;
    nStartValue = -1;
    nLevel = 0;
    bIsNumbered = bIsOrdered = bIsRestart = bIsNamed = sal_False;
}

inline sal_Bool XMLTextNumRuleInfo::HasSameNumRules(
                        const XMLTextNumRuleInfo& rCmp ) const
{
    return (bIsNamed && rCmp.bIsNamed) ? (rCmp.sName == sName)
                                   : (rCmp.xNumRules == xNumRules);
}

#endif  //  _XMLOFF_XMLTEXTNUMRULEINFO_HXX
