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

#ifndef _XMLOFF_TRANSGRADIENTSTYLE_HXX
#define _XMLOFF_TRANSGRADIENTSTYLE_HXX


#ifndef _SAL_TYPES_H_
#include <sal/types.h>
#endif
namespace com { namespace sun { namespace star {
    namespace uno { template<class A> class Reference; }
    namespace xml { namespace sax { class XAttributeList; } }
    namespace uno { class Any; }
} } }
namespace rtl { class OUString; }
namespace binfilter {

class SvXMLImport;
class SvXMLExport;



class XMLTransGradientStyleImport
{
    SvXMLImport& rImport;

public:
    XMLTransGradientStyleImport( SvXMLImport& rImport );
    ~XMLTransGradientStyleImport();

    sal_Bool importXML( 
        const ::com::sun::star::uno::Reference< 
            ::com::sun::star::xml::sax::XAttributeList >& xAttrList,
        ::com::sun::star::uno::Any& rValue,
        ::rtl::OUString& rStrName );
};

class XMLTransGradientStyleExport
{
    SvXMLExport& rExport;

public:
    XMLTransGradientStyleExport( SvXMLExport& rExport );
    ~XMLTransGradientStyleExport();

    sal_Bool exportXML( const ::rtl::OUString& rStrName, const ::com::sun::star::uno::Any& rValue );
};

}//end of namespace binfilter
#endif // _XMLOFF_TRANSGRADIENTSTYLE_HXX
