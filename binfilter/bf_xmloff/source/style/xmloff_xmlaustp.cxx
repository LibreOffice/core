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


#ifndef _COM_SUN_STAR_CONTAINER_XINDEXREPLACE_HPP_ 
#include <com/sun/star/container/XIndexReplace.hpp>
#endif

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _XMLOFF_XMLASTPL_IMPL_HXX
#include "impastpl.hxx"
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include "xmlnmspe.hxx"
#endif
#ifndef _XMLOFF_XMLEXP_HXX
#include "xmlexp.hxx"
#endif

#ifndef _XMLOFF_XMLTEXTLISTAUTOSTYLEPOOL_HXX
#include "XMLTextListAutoStylePool.hxx"
#endif

#ifndef _XMLOFF_PAGEMASTERSTYLEMAP_HXX
#include "PageMasterStyleMap.hxx"
#endif
namespace binfilter {


using namespace ::std;
using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::binfilter::xmloff::token;


namespace
{
    static void lcl_exportDataStyle( SvXMLExport& _rExport, const UniReference< XMLPropertySetMapper >& _rxMapper,
        const XMLPropertyState& _rProperty )
    {
        DBG_ASSERT( _rxMapper.is(), "xmloff::lcl_exportDataStyle: invalid property mapper!" );
        // obtain the data style name
        ::rtl::OUString sDataStyleName;
        _rProperty.maValue >>= sDataStyleName;
        DBG_ASSERT( sDataStyleName.getLength(), "xmloff::lcl_exportDataStyle: invalid property value for the data style name!" );

        // add the attribute
        _rExport.AddAttribute( 
            _rxMapper->GetEntryNameSpace( _rProperty.mnIndex ),
            _rxMapper->GetEntryXMLName( _rProperty.mnIndex ),
            sDataStyleName );
    }
}

void SvXMLAutoStylePoolP::exportStyleAttributes(
        SvXMLAttributeList& rAttrList,
        sal_Int32 nFamily,
        const vector< XMLPropertyState >& rProperties,
        const SvXMLExportPropertyMapper& rPropExp, 
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap
        ) const
{
    if ( XML_STYLE_FAMILY_CONTROL_ID == nFamily )
    {	// it's a control-related style
        UniReference< XMLPropertySetMapper > aPropertyMapper = rPropExp.getPropertySetMapper();

        for	(	vector< XMLPropertyState >::const_iterator pProp = rProperties.begin();
                pProp != rProperties.end();
                ++pProp
            )
        {
            if	(	( pProp->mnIndex > -1 )
                &&	( CTF_FORMS_DATA_STYLE == aPropertyMapper->GetEntryContextId( pProp->mnIndex ) )
                )
            {	// it's the data-style for a grid column
                lcl_exportDataStyle( GetExport(), aPropertyMapper, *pProp );
            }
        }
    }

    if( (XML_STYLE_FAMILY_SD_GRAPHICS_ID == nFamily) || (XML_STYLE_FAMILY_SD_PRESENTATION_ID == nFamily) )
    {	// it's a graphics style
        UniReference< XMLPropertySetMapper > aPropertyMapper = rPropExp.getPropertySetMapper();
        DBG_ASSERT(aPropertyMapper.is(), "SvXMLAutoStylePoolP::exportStyleAttributes: invalid property set mapper!");

        sal_Bool bFoundControlShapeDataStyle = sal_False;
        sal_Bool bFoundNumberingRulesName = sal_False;

        for	(	vector< XMLPropertyState >::const_iterator pProp = rProperties.begin();
                pProp != rProperties.end();
                ++pProp
            )
        {
            if (pProp->mnIndex > -1)
            {	// it's a valid property
                switch( aPropertyMapper->GetEntryContextId(pProp->mnIndex) )
                {
                case CTF_SD_CONTROL_SHAPE_DATA_STYLE:
                    {	// it's the control shape data style property

                        if (bFoundControlShapeDataStyle)
                        {
                            DBG_ERROR("SvXMLAutoStylePoolP::exportStyleAttributes: found two properties with the ControlShapeDataStyle context id!");
                            // already added the attribute for the first occurence
                            break;
                        }

                        lcl_exportDataStyle( GetExport(), aPropertyMapper, *pProp );

                        // check if there is another property with the special context id we're handling here
                        bFoundControlShapeDataStyle = sal_True;
                        break;
                    }
                case CTF_SD_NUMBERINGRULES_NAME:
                    {
                        if (bFoundNumberingRulesName)
                        {
                            DBG_ERROR("SvXMLAutoStylePoolP::exportStyleAttributes: found two properties with the numbering rules name context id!");
                            // already added the attribute for the first occurence
                            break;
                        }

                        uno::Reference< container::XIndexReplace > xNumRule;
                        pProp->maValue >>= xNumRule;
                        if( xNumRule.is() && (xNumRule->getCount() > 0 ) )
                        {
                            const OUString sName(((XMLTextListAutoStylePool*)&GetExport().GetTextParagraphExport()->GetListAutoStylePool())->Add( xNumRule ));

                            GetExport().AddAttribute( XML_NAMESPACE_STYLE, XML_LIST_STYLE_NAME, sName );
                        }

                        bFoundNumberingRulesName = sal_True;
                        break;
                    }	
                }
            }
        }
    }

    if( nFamily == XML_STYLE_FAMILY_PAGE_MASTER )
    {
        for( vector< XMLPropertyState >::const_iterator pProp = rProperties.begin(); pProp != rProperties.end(); pProp++ )
        {
            if (pProp->mnIndex > -1)
            {
                UniReference< XMLPropertySetMapper > aPropMapper = rPropExp.getPropertySetMapper();
                sal_Int32 nIndex = pProp->mnIndex;
                sal_Int16 nContextID = aPropMapper->GetEntryContextId( nIndex );
                switch( nContextID )
                {
                    case CTF_PM_PAGEUSAGE:
                    {
                        OUString sValue;
                        const XMLPropertyHandler* pPropHdl = aPropMapper->GetPropertyHandler( nIndex );
                        if( pPropHdl && 
                            pPropHdl->exportXML( sValue, pProp->maValue, 
                                                 GetExport().GetMM100UnitConverter() ) &&
                            ( ! IsXMLToken( sValue, XML_ALL ) ) )
                        {
                            GetExport().AddAttribute( aPropMapper->GetEntryNameSpace( nIndex ), aPropMapper->GetEntryXMLName( nIndex ), sValue );
                        }
                    }
                    break;
                }
            }
        }
    }
}

void SvXMLAutoStylePoolP::exportStyleContent(
        const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
        sal_Int32 nFamily,
        const vector< XMLPropertyState >& rProperties,
        const SvXMLExportPropertyMapper& rPropExp,
        const SvXMLUnitConverter& rUnitConverter,
        const SvXMLNamespaceMap& rNamespaceMap
        ) const
{
    if( nFamily == XML_STYLE_FAMILY_PAGE_MASTER )
    {
        OUString sWS( GetXMLToken(XML_WS) );

        sal_Int32		nHeaderStartIndex(-1);
        sal_Int32		nHeaderEndIndex(-1);
        sal_Int32		nFooterStartIndex(-1);
        sal_Int32		nFooterEndIndex(-1);
        sal_Bool		bHeaderStartIndex(sal_False);
        sal_Bool		bHeaderEndIndex(sal_False);
        sal_Bool		bFooterStartIndex(sal_False);
        sal_Bool		bFooterEndIndex(sal_False);

        UniReference< XMLPropertySetMapper > aPropMapper = rPropExp.getPropertySetMapper();

        sal_Int32 nIndex(0);
        while(nIndex < aPropMapper->GetEntryCount())
        {
            switch( aPropMapper->GetEntryContextId( nIndex ) & CTF_PM_FLAGMASK )
            {
                case CTF_PM_HEADERFLAG:
                {
                    if (!bHeaderStartIndex)
                    {
                        nHeaderStartIndex = nIndex;
                        bHeaderStartIndex = sal_True;
                    }
                    if (bFooterStartIndex && !bFooterEndIndex)
                    {
                        nFooterEndIndex = nIndex;
                        bFooterEndIndex = sal_True;
                    }
                }
                break;
                case CTF_PM_FOOTERFLAG:
                {
                    if (!bFooterStartIndex)
                    {
                        nFooterStartIndex = nIndex;
                        bFooterStartIndex = sal_True;
                    }
                    if (bHeaderStartIndex && !bHeaderEndIndex)
                    {
                        nHeaderEndIndex = nIndex;
                        bHeaderEndIndex = sal_True;
                    }
                }
                break;
            }
            nIndex++;
        }
        if (!bHeaderEndIndex)
            nHeaderEndIndex = nIndex;
        if (!bFooterEndIndex)
            nFooterEndIndex = nIndex;

        // export header style element
        {
            SvXMLElementExport aElem( 
                GetExport(), XML_NAMESPACE_STYLE, XML_HEADER_STYLE,
                sal_True, sal_True );

            rPropExp.exportXML( 
                GetExport(), rProperties, 
                nHeaderStartIndex, nHeaderEndIndex, XML_EXPORT_FLAG_IGN_WS);
        }

        // export footer style
        {
            SvXMLElementExport aElem(
                GetExport(), XML_NAMESPACE_STYLE, XML_FOOTER_STYLE,
                sal_True, sal_True );

            rPropExp.exportXML(
                GetExport(), rProperties, 
                nFooterStartIndex, nFooterEndIndex, XML_EXPORT_FLAG_IGN_WS);
        }
    }
}

SvXMLAutoStylePoolP::SvXMLAutoStylePoolP( SvXMLExport& rExport )
{
    pImpl = new SvXMLAutoStylePoolP_Impl( rExport );
}

SvXMLAutoStylePoolP::~SvXMLAutoStylePoolP()
{
    delete pImpl;
}

SvXMLExport& SvXMLAutoStylePoolP::GetExport() const
{
    return pImpl->GetExport();
}

// TODO: romove this
void SvXMLAutoStylePoolP::AddFamily(
        sal_Int32 nFamily,
        const OUString& rStrName,
        SvXMLExportPropertyMapper* pMapper,
        OUString aStrPrefix,
        sal_Bool bAsFamily )
{
    UniReference <SvXMLExportPropertyMapper> xTmp = pMapper;
    AddFamily( nFamily, rStrName, xTmp, aStrPrefix, bAsFamily );
}

void SvXMLAutoStylePoolP::AddFamily(
        sal_Int32 nFamily,
        const OUString& rStrName,
        const UniReference < SvXMLExportPropertyMapper > & rMapper,
        const OUString& rStrPrefix,
        sal_Bool bAsFamily )
{
    pImpl->AddFamily( nFamily, rStrName, rMapper, rStrPrefix, bAsFamily );
}

void SvXMLAutoStylePoolP::RegisterName( sal_Int32 nFamily,
                                         const OUString& rName )
{
    pImpl->RegisterName( nFamily, rName );
}

OUString SvXMLAutoStylePoolP::Add( sal_Int32 nFamily,
                                  const vector< XMLPropertyState >& rProperties )
{
    OUString sEmpty;
    OUString sName;
    pImpl->Add(sName, nFamily, sEmpty, rProperties );
    return sName;
}

OUString SvXMLAutoStylePoolP::Add( sal_Int32 nFamily,
                                  const OUString& rParent,
                                  const vector< XMLPropertyState >& rProperties )
{
    OUString sName;
    pImpl->Add(sName, nFamily, rParent, rProperties );
    return sName;
}

sal_Bool SvXMLAutoStylePoolP::Add(OUString& rName, sal_Int32 nFamily, const OUString& rParent, const ::std::vector< XMLPropertyState >& rProperties )
{
    return pImpl->Add(rName, nFamily, rParent, rProperties);
}

OUString SvXMLAutoStylePoolP::Find( sal_Int32 nFamily,
                                   const OUString& rParent,
                                   const vector< XMLPropertyState >& rProperties ) const
{
    return pImpl->Find( nFamily, rParent, rProperties );
}

void SvXMLAutoStylePoolP::exportXML( sal_Int32 nFamily, 
    const uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > & rHandler,
    const SvXMLUnitConverter& rUnitConverter,
    const SvXMLNamespaceMap& rNamespaceMap
    ) const
{
    pImpl->exportXML( nFamily, 
                      GetExport().GetDocHandler(),
                      GetExport().GetMM100UnitConverter(),
                      GetExport().GetNamespaceMap(),
                      this);
}

void SvXMLAutoStylePoolP::ClearEntries()
{
    pImpl->ClearEntries();
}
}//end of namespace binfilter
