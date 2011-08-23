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

#ifndef _XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX
#define _XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX


#ifndef _COM_SUN_STAR_UCB_XANYCOMPARE_HPP_ 
#include <com/sun/star/ucb/XAnyCompare.hpp>
#endif

namespace com { namespace sun { namespace star { namespace container {
    class XIndexReplace; } } } }
namespace rtl { class OUString; }
namespace binfilter {

class XMLTextListAutoStylePool_Impl;
class XMLTextListAutoStylePoolNames_Impl;
class XMLTextListAutoStylePoolEntry_Impl;
class SvXMLExport;

class XMLTextListAutoStylePool
{
    SvXMLExport& rExport;

    const ::rtl::OUString sPrefix;

    XMLTextListAutoStylePool_Impl *pPool;
    XMLTextListAutoStylePoolNames_Impl *pNames;
    sal_uInt32 nName;

    /** this is an optional NumRule compare component for applications where
        the NumRules don't have names */
    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XAnyCompare > mxNumRuleCompare;

    sal_uInt32 Find( XMLTextListAutoStylePoolEntry_Impl* pEntry ) const;
public:

    XMLTextListAutoStylePool( SvXMLExport& rExport );
    ~XMLTextListAutoStylePool();

    void RegisterName( const ::rtl::OUString& rName );

    ::rtl::OUString Add(
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::container::XIndexReplace > & rNumRules );

    ::rtl::OUString Find(
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::container::XIndexReplace > & rNumRules ) const;
    ::rtl::OUString Find( const ::rtl::OUString& rInternalName ) const;

    void exportXML() const; 
};

}//end of namespace binfilter
#endif	//  _XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX
