/*************************************************************************
 *
 *  $RCSfile: xmlStyleImport.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2004-08-02 15:22:54 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef DBA_XMLSTYLEIMPORT_HXX
#include "xmlStyleImport.hxx"
#endif

#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_XMLIMPPR_HXX
#include <xmloff/xmlimppr.hxx>
#endif
#ifndef _XMLOFF_FAMILIES_HXX_
#include <xmloff/families.hxx>
#endif
#ifndef _XMLOFF_XMLNUMFI_HXX
#include <xmloff/xmlnumfi.hxx>
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _COM_SUN_STAR_STYLE_XSTYLEFAMILIESSUPPLIER_HPP_
#include <com/sun/star/style/XStyleFamiliesSupplier.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMECONTAINER_HPP_
#include <com/sun/star/container/XNameContainer.hpp>
#endif
#ifndef _COM_SUN_STAR_TABLE_BORDERLINE_HPP_
#include <com/sun/star/table/BorderLine.hpp>
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _XMLOFF_XMLPROPERTYSETCONTEXT_HXX
#include <xmloff/xmlprcon.hxx>
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef DBA_XMLFILTER_HXX
#include "xmlfilter.hxx"
#endif
#ifndef DBA_XMLHELPER_HXX
#include "xmlHelper.hxx"
#endif

#define XML_LINE_LEFT 0
#define XML_LINE_RIGHT 1
#define XML_LINE_TOP 2
#define XML_LINE_BOTTOM 3

namespace dbaxml
{

using namespace ::rtl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::style;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace xmloff::token;

// -----------------------------------------------------------------------------
TYPEINIT1( OTableStyleContext, XMLPropStyleContext );
TYPEINIT1( OTableStylesContext, SvXMLStylesContext );

OTableStyleContext::OTableStyleContext( ODBFilter& rImport,
        sal_uInt16 nPrfx, const OUString& rLName,
        const Reference< XAttributeList > & xAttrList,
        SvXMLStylesContext& rStyles, sal_uInt16 nFamily, sal_Bool bDefaultStyle ) :
    XMLPropStyleContext( rImport, nPrfx, rLName, xAttrList, rStyles, nFamily, bDefaultStyle ),
    sNumberFormat(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("NumberFormat"))),
    pStyles(&rStyles),
    m_nNumberFormat(-1),
    bParentSet(sal_False),
    bConditionalFormatCreated(sal_False)
{
}
// -----------------------------------------------------------------------------

OTableStyleContext::~OTableStyleContext()
{
}
// -----------------------------------------------------------------------------

void OTableStyleContext::FillPropertySet(
            const Reference< XPropertySet > & rPropSet )
{
    if ( !IsDefaultStyle() )
    {
        if ( GetFamily() == XML_STYLE_FAMILY_TABLE_TABLE )
        {
            if ( sPageStyle.getLength() )
            {
                uno::Any aAny;
                aAny <<= sPageStyle;
                AddProperty(CTF_DB_MASTERPAGENAME, aAny);
            }
        }
        else if ( GetFamily() == XML_STYLE_FAMILY_TABLE_COLUMN )
        {
            if ((m_nNumberFormat == -1) && m_sDataStyleName.getLength())
            {
                SvXMLNumFormatContext* pStyle = PTR_CAST(SvXMLNumFormatContext,pStyles->FindStyleChildContext(
                    XML_STYLE_FAMILY_DATA_STYLE, m_sDataStyleName, sal_True));
                if ( !pStyle )
                {
                    OTableStylesContext* pMyStyles = PTR_CAST(OTableStylesContext,GetOwnImport().GetAutoStyles());
                    if ( pMyStyles )
                        pStyle = PTR_CAST(SvXMLNumFormatContext,pMyStyles->
                            FindStyleChildContext(XML_STYLE_FAMILY_DATA_STYLE, m_sDataStyleName, sal_True));
                    else
                        DBG_ERROR("not possible to get style");
                }
                if ( pStyle )
                {
                    uno::Any aNumberFormat;
                    m_nNumberFormat = pStyle->GetKey();
                    aNumberFormat <<= m_nNumberFormat;
                    //rPropSet->setPropertyValue(rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(SC_NUMBERFORMAT)), aNumberFormat);
                    AddProperty(CTF_DB_NUMBERFORMAT, aNumberFormat);
                }
            }
        }
    }
    XMLPropStyleContext::FillPropertySet(rPropSet);
}
// -----------------------------------------------------------------------------

void OTableStyleContext::SetDefaults()
{
}
// -----------------------------------------------------------------------------

void OTableStyleContext::AddProperty(const sal_Int16 nContextID, const uno::Any& rValue)
{
    sal_Int32 nIndex(static_cast<OTableStylesContext *>(pStyles)->GetIndex(nContextID));
    DBG_ASSERT(nIndex != -1, "Property not found in Map");
    XMLPropertyState aPropState(nIndex, rValue);
    GetProperties().push_back(aPropState); // has to be insertes in a sort order later
}
// -----------------------------------------------------------------------------
void OTableStyleContext::SetAttribute( sal_uInt16 nPrefixKey,
                                        const OUString& rLocalName,
                                        const OUString& rValue )
{
    // TODO: use a map here
    if( IsXMLToken(rLocalName, XML_DATA_STYLE_NAME ) )
        m_sDataStyleName = rValue;
    else if ( IsXMLToken(rLocalName, XML_MASTER_PAGE_NAME ) )
        sPageStyle = rValue;
    else
        XMLPropStyleContext::SetAttribute( nPrefixKey, rLocalName, rValue );
}
// -----------------------------------------------------------------------------
ODBFilter& OTableStyleContext::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}
// -----------------------------------------------------------------------------

OTableStylesContext::OTableStylesContext( SvXMLImport& rImport,
        sal_uInt16 nPrfx ,
        const OUString& rLName ,
        const Reference< XAttributeList > & xAttrList,
        const sal_Bool bTempAutoStyles ) :
    SvXMLStylesContext( rImport, nPrfx, rLName, xAttrList ),
    sColumnStyleServiceName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( XML_STYLE_FAMILY_TABLE_COLUMN_STYLES_NAME ))),
    sTableStyleServiceName( rtl::OUString(RTL_CONSTASCII_USTRINGPARAM( XML_STYLE_FAMILY_TABLE_TABLE_STYLES_NAME ))),
    m_nNumberFormatIndex(-1),
    bAutoStyles(bTempAutoStyles)
{
}
// -----------------------------------------------------------------------------

OTableStylesContext::~OTableStylesContext()
{
}
// -----------------------------------------------------------------------------

void OTableStylesContext::EndElement()
{
    SvXMLStylesContext::EndElement();
    if (bAutoStyles)
        GetImport().GetTextImport()->SetAutoStyles( this );
    else
        GetImport().GetStyles()->CopyStylesToDoc(sal_True);
}
// -----------------------------------------------------------------------------

UniReference < SvXMLImportPropertyMapper >
    OTableStylesContext::GetImportPropertyMapper(
                    sal_uInt16 nFamily ) const
{
    UniReference < SvXMLImportPropertyMapper > xMapper = SvXMLStylesContext::GetImportPropertyMapper(nFamily);

    if (!xMapper.is())
    {
        switch( nFamily )
        {
            case XML_STYLE_FAMILY_TABLE_COLUMN:
            {
                if ( !m_xColumnImpPropMapper.is() )
                    m_xColumnImpPropMapper = new SvXMLImportPropertyMapper( const_cast<OTableStylesContext*>(this)->GetOwnImport().GetColumnStylesPropertySetMapper(), const_cast<SvXMLImport&>(GetImport()) );
                xMapper = m_xColumnImpPropMapper;
            }
             break;
            case XML_STYLE_FAMILY_TABLE_TABLE:
            {
                if ( !m_xTableImpPropMapper.is() )
                    m_xTableImpPropMapper = new SvXMLImportPropertyMapper( const_cast<OTableStylesContext*>(this)->GetOwnImport().GetTableStylesPropertySetMapper(), const_cast<SvXMLImport&>(GetImport()) );
                xMapper = m_xTableImpPropMapper;
            }
             break;
        }
    }

    return xMapper;
}
// ----------------------------------------------------------------------------
SvXMLStyleContext *OTableStylesContext::CreateStyleStyleChildContext(
        sal_uInt16 nFamily, sal_uInt16 nPrefix, const OUString& rLocalName,
        const Reference< xml::sax::XAttributeList > & xAttrList )
{
    SvXMLStyleContext *pStyle = SvXMLStylesContext::CreateStyleStyleChildContext( nFamily, nPrefix,
                                                            rLocalName,
                                                            xAttrList );
    if (!pStyle)
    {
        switch( nFamily )
        {
        case XML_STYLE_FAMILY_TABLE_COLUMN:
        case XML_STYLE_FAMILY_TABLE_TABLE:
            pStyle = new OTableStyleContext( GetOwnImport(), nPrefix, rLocalName,
                                               xAttrList, *this, nFamily );
            break;
        }
    }

    return pStyle;
}
// -----------------------------------------------------------------------------
Reference < XNameContainer >
        OTableStylesContext::GetStylesContainer( sal_uInt16 nFamily ) const
{
    Reference < XNameContainer > xStyles = SvXMLStylesContext::GetStylesContainer(nFamily);
    return xStyles;
}
// -----------------------------------------------------------------------------

OUString OTableStylesContext::GetServiceName( sal_uInt16 nFamily ) const
{
    rtl::OUString sServiceName = SvXMLStylesContext::GetServiceName(nFamily);
    if (!sServiceName.getLength())
    {
        switch( nFamily )
        {
        case XML_STYLE_FAMILY_TABLE_COLUMN:
            sServiceName = sColumnStyleServiceName;
            break;
        case XML_STYLE_FAMILY_TABLE_TABLE:
            sServiceName = sTableStyleServiceName;
            break;
        }
    }
    return sServiceName;
}
// -----------------------------------------------------------------------------

sal_Int32 OTableStylesContext::GetIndex(const sal_Int16 nContextID)
{
    if ( nContextID == CTF_DB_NUMBERFORMAT )
    {
        if (m_nNumberFormatIndex == -1)
            m_nNumberFormatIndex =
                GetImportPropertyMapper(XML_STYLE_FAMILY_TABLE_COLUMN)->getPropertySetMapper()->FindEntryIndex(nContextID);
        return m_nNumberFormatIndex;
    }
    else if ( nContextID == CTF_DB_MASTERPAGENAME )
    {
        if (nMasterPageNameIndex == -1)
            nMasterPageNameIndex =
                GetImportPropertyMapper(XML_STYLE_FAMILY_TABLE_TABLE)->getPropertySetMapper()->FindEntryIndex(nContextID);
        return nMasterPageNameIndex;
    }
    else if ( nContextID == CTF_DB_ISVISIBLE )
    {
        return -1;
    }
    else
        return -1;
}
// -----------------------------------------------------------------------------
ODBFilter& OTableStylesContext::GetOwnImport()
{
    return static_cast<ODBFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
} // dbaxml
// -----------------------------------------------------------------------------
