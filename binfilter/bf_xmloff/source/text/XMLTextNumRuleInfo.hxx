/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef _XMLOFF_XMLTEXTNUMRULEINFO_HXX
#define _XMLOFF_XMLTEXTNUMRULEINFO_HXX

#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_ 
#include <com/sun/star/container/XIndexReplace.hpp>
#endif

namespace com { namespace sun { namespace star {
    namespace text { class XTextContent; }
} } }

#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
namespace binfilter {

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

    ::rtl::OUString		sName;
    sal_Int16			nStartValue;
    sal_Int16			nLevel;
    
    sal_Bool			bIsNumbered : 1;
    sal_Bool			bIsOrdered : 1;
    sal_Bool			bIsRestart : 1;
    sal_Bool			bIsNamed : 1;
    
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

}//end of namespace binfilter
#endif	//  _XMLOFF_XMLTEXTNUMRULEINFO_HXX
