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

#ifndef _SVX_XMLXTIMP_HXX
#define _SVX_XMLXTIMP_HXX

#ifndef _XMLOFF_XMLIMP_HXX 
#include <bf_xmloff/xmlimp.hxx>
#endif

namespace rtl {	class OUString; }
namespace com { namespace sun { namespace star {
    namespace uno { template<class X> class Reference; }
    namespace uno { class XInterface; }
    namespace document { class XGraphicObjectResolver; }
    namespace container { class XNameContainer; }

} } }
namespace binfilter {

class SvxXMLXTableImport : public SvXMLImport
{
public:
    // #110680#
    SvxXMLXTableImport(
        const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > xServiceFactory,
        const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & rTable,
        ::com::sun::star::uno::Reference<	::com::sun::star::document::XGraphicObjectResolver >& xGrfResolver);
    virtual ~SvxXMLXTableImport() throw ();

    static sal_Bool load( const ::rtl::OUString& rUrl, const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer >& xTable ) throw();
protected:
    virtual SvXMLImportContext *CreateContext( sal_uInt16 nPrefix,
                                      const ::rtl::OUString& rLocalName,
                                      const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttrList );

private:
    const ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameContainer > & mrTable;
};

}//end of namespace binfilter
#endif

