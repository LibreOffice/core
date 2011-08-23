/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _XMLOFF_XMLCHANGEIMPORTCONTEXT_HXX
#define _XMLOFF_XMLCHANGEIMPORTCONTEXT_HXX


#include "xmlictxt.hxx"

#include <com/sun/star/uno/Reference.h>


namespace com { namespace sun { namespace star {
    namespace xml { namespace sax {
        class XAttributeList;
    } }
} } }
namespace rtl {
    class OUString;
}

namespace binfilter {


/**
 * import change tracking/redlining markers
 * <text:change>, <text:change-start>, <text:change-end>
 */
class XMLChangeImportContext : public SvXMLImportContext
{
    sal_Bool bIsStart;
    sal_Bool bIsEnd;
    sal_Bool bIsOutsideOfParagraph;

public:

    TYPEINFO();

    /**
     * import a change mark 
     * (<text:change>, <text:change-start>, <text:change-end>)
     * Note: a <text:change> mark denotes start and end of a change 
     * simultaniously, so both bIsStart and bIsEnd parameters would 
     * be set true.
     */
    XMLChangeImportContext(
        SvXMLImport& rImport,
        sal_Int16 nPrefix,
        const ::rtl::OUString& rLocalName,
        sal_Bool bIsStart,	/// mark start of a change
        sal_Bool bIsEnd,	/// mark end of a change
        /// true if change mark is encountered outside of a paragraph
        /// (usually before a section or table)
        sal_Bool bIsOutsideOfParagraph = sal_False);

    ~XMLChangeImportContext();

    virtual void StartElement(
        const ::com::sun::star::uno::Reference<
                ::com::sun::star::xml::sax::XAttributeList> & xAttrList);
};

}//end of namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
