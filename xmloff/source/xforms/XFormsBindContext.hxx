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

#ifndef _XMLOFF_XFORMSBINDCONTEXT_HXX
#define _XMLOFF_XFORMSBINDCONTEXT_HXX


//
// include for parent class and members
//

#include "TokenContext.hxx"
#include <com/sun/star/uno/Reference.hxx>


//
// forward declarations
//

namespace com { namespace sun { namespace star {
    namespace xml { namespace sax { class XAttributeList; } }
    namespace beans { class XPropertySet; }
    namespace xforms { class XModel; }
} } }
namespace rtl { class OUString; }
class SvXMLImport;
class SvXMLImportContext;


/** import the xforms:binding element */
class XFormsBindContext : public TokenContext
{
    const com::sun::star::uno::Reference<com::sun::star::xforms::XModel> mxModel;
    com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> mxBinding;

public:

    XFormsBindContext( SvXMLImport& rImport,
                       sal_uInt16 nPrefix,
                       const rtl::OUString& rLocalName,
                       const com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet>& xModel );
    virtual ~XFormsBindContext();


    //
    // implement SvXMLImportContext & TokenContext methods:
    //

    virtual void StartElement(
        const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList );

protected:
    virtual void HandleAttribute( sal_uInt16 nToken,
                                  const rtl::OUString& rValue );

    virtual SvXMLImportContext* HandleChild(
        sal_uInt16 nToken,
        sal_uInt16 nNamespace,
        const rtl::OUString& rLocalName,
        const com::sun::star::uno::Reference<com::sun::star::xml::sax::XAttributeList>& xAttrList );

};

#endif
