/*************************************************************************
 *
 *  $RCSfile: XMLTextListAutoStylePool.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-01-11 14:14:15 $
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

#ifndef _XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX
#define _XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _SAL_TYPES_H_
#include "sal/types.h"
#endif

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _COM_SUN_STAR_UCB_XANYCOMPARE_HPP_
#include <com/sun/star/ucb/XAnyCompare.hpp>
#endif

namespace com { namespace sun { namespace star { namespace container {
    class XIndexReplace; } } } }
namespace rtl { class OUString; }


class XMLTextListAutoStylePool_Impl;
class XMLTextListAutoStylePoolNames_Impl;
class XMLTextListAutoStylePoolEntry_Impl;
class SvXMLExport;

class XMLOFF_DLLPUBLIC XMLTextListAutoStylePool
{
    SvXMLExport& rExport;

    const ::rtl::OUString sPrefix;

    XMLTextListAutoStylePool_Impl *pPool;
    XMLTextListAutoStylePoolNames_Impl *pNames;
    sal_uInt32 nName;

    /** this is an optional NumRule compare component for applications where
        the NumRules don't have names */
    ::com::sun::star::uno::Reference< ::com::sun::star::ucb::XAnyCompare > mxNumRuleCompare;

    SAL_DLLPRIVATE sal_uInt32 Find( XMLTextListAutoStylePoolEntry_Impl* pEntry )
        const;
public:

    XMLTextListAutoStylePool( SvXMLExport& rExport );
    ~XMLTextListAutoStylePool();

    void RegisterName( const ::rtl::OUString& rName );
    sal_Bool HasName( const ::rtl::OUString& rName ) const;

    ::rtl::OUString Add(
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::container::XIndexReplace > & rNumRules );

    ::rtl::OUString Find(
            const ::com::sun::star::uno::Reference <
                ::com::sun::star::container::XIndexReplace > & rNumRules ) const;
    ::rtl::OUString Find( const ::rtl::OUString& rInternalName ) const;

    void exportXML() const;
};


#endif  //  _XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX
