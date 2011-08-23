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

#ifndef _XMLOFF_XMLFOOTNOTESEPARATORIMPORT_HXX
#define _XMLOFF_XMLFOOTNOTESEPARATORIMPORT_HXX



#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif

#ifndef _UNIVERSALL_REFERENCE_HXX
#include "uniref.hxx"
#endif

#include <vector>
namespace rtl { class OUString; }
namespace com { namespace sun { namespace star {
    namespace uno { template<class X> class Reference; }
    namespace xml { namespace sax { class XAttributeList; } }
} } }
namespace binfilter {


class SvXMLImport;
struct XMLPropertyState;
class XMLPropertySetMapper;



/**
 * Import the footnote-separator element in page styles.
 */
class XMLFootnoteSeparatorImport : public SvXMLImportContext
{
    ::std::vector<XMLPropertyState> & rProperties;
    UniReference<XMLPropertySetMapper> rMapper;
    sal_Int32 nPropIndex;

public:
    
    TYPEINFO();

    XMLFootnoteSeparatorImport(
        SvXMLImport& rImport, 
        sal_uInt16 nPrefix, 
        const ::rtl::OUString& rLocalName, 
        ::std::vector<XMLPropertyState> & rProperties,
        const UniReference<XMLPropertySetMapper> & rMapperRef,
        sal_Int32 nIndex);

    ~XMLFootnoteSeparatorImport();

    virtual void StartElement( 
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList> & xAttrList );
};

}//end of namespace binfilter
#endif
