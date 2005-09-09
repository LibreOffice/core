/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PageMasterExportPropMapper.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 14:26:07 $
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

//______________________________________________________________________________

class XMLPageMasterExportPropMapper : public SvXMLExportPropertyMapper
{
protected:
    XMLBackgroundImageExport aBackgroundImageExport;
    XMLTextColumnsExport aTextColumnsExport;
    XMLFootnoteSeparatorExport aFootnoteSeparatorExport;

    virtual void        ContextFilter(
                            ::std::vector< XMLPropertyState >& rProperties,
                            ::com::sun::star::uno::Reference< ::com::sun::star::beans::XPropertySet > rPropSet
                            ) const;

public:
                        XMLPageMasterExportPropMapper(
                             const UniReference< XMLPropertySetMapper >& rMapper,
                            SvXMLExport& rExport
                            );
    virtual             ~XMLPageMasterExportPropMapper();

    virtual void        handleElementItem(
                            SvXMLExport& rExport,
                            const XMLPropertyState& rProperty,
                            sal_uInt16 nFlags,
                            const ::std::vector< XMLPropertyState >* pProperties = 0,
                            sal_uInt32 nIdx = 0
                            ) const;
    virtual void        handleSpecialItem(
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

#endif

