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

#ifndef _SD_XMLVIEWSETTINGSCONTEXT_HXX
#define _SD_XMLVIEWSETTINGSCONTEXT_HXX


#ifndef _COM_SUN_STAR_AWT_RECTANGLE_HPP_
#include <com/sun/star/awt/Rectangle.hpp>
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include "xmlictxt.hxx"
#endif
namespace binfilter {

class SdXMLImport;

class SdXMLViewSettingsContext : public SvXMLImportContext
{
    ::com::sun::star::awt::Rectangle maVisArea;

public:
    SdXMLViewSettingsContext( SdXMLImport& rImport, USHORT nPrfx, const ::rtl::OUString& rLName,
                                      const ::com::sun::star::uno::Reference<
                                      ::com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual ~SdXMLViewSettingsContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const ::rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

}//end of namespace binfilter
#endif
