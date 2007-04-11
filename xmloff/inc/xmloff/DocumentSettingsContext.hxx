/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: DocumentSettingsContext.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 13:19:42 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef _XMLOFF_DOCUMENTSETTINGSCONTEXT_HXX
#define _XMLOFF_DOCUMENTSETTINGSCONTEXT_HXX

#ifndef _SAL_CONFIG_H_
#include "sal/config.h"
#endif

#ifndef INCLUDED_XMLOFF_DLLAPI_H
#include "xmloff/dllapi.h"
#endif

#ifndef _XMLOFF_XMLICTXT_HXX
#include <xmloff/xmlictxt.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_H_
#include <com/sun/star/uno/Sequence.h>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif

class XMLOFF_DLLPUBLIC XMLDocumentSettingsContext : public SvXMLImportContext
{
    com::sun::star::uno::Any    aViewProps;
    com::sun::star::uno::Any    aConfigProps;

public:
    XMLDocumentSettingsContext(SvXMLImport& rImport, USHORT nPrfx, const rtl::OUString& rLName,
                                    const ::com::sun::star::uno::Reference<
                                    ::com::sun::star::xml::sax::XAttributeList>& xAttrList);
    virtual ~XMLDocumentSettingsContext();

    virtual SvXMLImportContext *CreateChildContext( USHORT nPrefix,
                                                    const rtl::OUString& rLocalName,
                                                    const ::com::sun::star::uno::Reference<
                                          ::com::sun::star::xml::sax::XAttributeList>& xAttrList );
    virtual void EndElement();
};

#endif
