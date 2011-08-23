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

#ifndef _XMLOFF_PAGEMASTEREXPORTPROPMAPPER_HXX
#define _XMLOFF_PAGEMASTEREXPORTPROPMAPPER_HXX

#ifndef _XMLOFF_XMLEXPPR_HXX
#include "xmlexppr.hxx"
#endif
#ifndef _XMLBACKGROUNDIMAGEEXPORT_HXX
#include "XMLBackgroundImageExport.hxx"
#endif
#ifndef _XMLOFF_XMLTEXTCOLUMNSEXPORT_HXX
#include "XMLTextColumnsExport.hxx"
#endif
#ifndef _XMLOFF_XMLFOOTNOTESEPARATOREXPORT_HXX
#include "XMLFootnoteSeparatorExport.hxx"
#endif
namespace binfilter {

//______________________________________________________________________________

class XMLPageMasterExportPropMapper : public SvXMLExportPropertyMapper
{
protected:
    XMLBackgroundImageExport aBackgroundImageExport;
    XMLTextColumnsExport aTextColumnsExport;
    XMLFootnoteSeparatorExport aFootnoteSeparatorExport;

    virtual void		ContextFilter(
                            ::std::vector< XMLPropertyState >& rProperties,
                            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rPropSet
                            ) const;

public:
                        XMLPageMasterExportPropMapper(
                             const UniReference< XMLPropertySetMapper >& rMapper,
                            SvXMLExport& rExport
                            );
    virtual				~XMLPageMasterExportPropMapper();

    virtual void		handleElementItem(
                            SvXMLExport& rExport,
                            const XMLPropertyState& rProperty,
                            sal_uInt16 nFlags,
                            const ::std::vector< XMLPropertyState >* pProperties = 0,
                            sal_uInt32 nIdx = 0
                            ) const;
    virtual void		handleSpecialItem(
                            SvXMLAttributeList& rAttrList,
                            const XMLPropertyState& rProperty,
                            const SvXMLUnitConverter& rUnitConverter,
                            const SvXMLNamespaceMap& rNamespaceMap,
                            const ::std::vector< XMLPropertyState >* pProperties = 0,
                            sal_uInt32 nIdx = 0
                            ) const;

    inline XMLBackgroundImageExport& GetBackgroundImageExport()
                                { return aBackgroundImageExport; }

};

}//end of namespace binfilter
#endif

