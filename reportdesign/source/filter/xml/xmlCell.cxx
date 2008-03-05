/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlCell.cxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:00:56 $
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
#include "precompiled_reportdesign.hxx"

#ifndef RPT_XMLCELL_HXX
#include "xmlCell.hxx"
#endif
#ifndef RPT_XMLHELPER_HXX
#include "xmlHelper.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef RPT_XMLFILTER_HXX
#include "xmlfilter.hxx"
#endif
#ifndef _XMLOFF_XMLTOKEN_HXX
#include <xmloff/xmltoken.hxx>
#endif
#ifndef _XMLOFF_XMLNMSPE_HXX
#include <xmloff/xmlnmspe.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef RPT_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef RPT_XMLSTYLEIMPORT_HXX
#include "xmlStyleImport.hxx"
#endif
#ifndef _COMPHELPER_NAMECONTAINER_HXX_
#include <comphelper/namecontainer.hxx>
#endif
#ifndef _COMPHELPER_GENERICPROPERTYSET_HXX_
#include <comphelper/genericpropertyset.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HXX_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#include <com/sun/star/report/XShape.hpp>
#include <com/sun/star/report/XFixedLine.hpp>
#include <com/sun/star/table/BorderLine.hpp>
#ifndef RPT_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#ifndef RPT_XMLTABLE_HXX
#include "xmlTable.hxx"
#endif
#ifndef RPT_XMLFIXEDTEXT_HXX
#include "xmlFixedText.hxx"
#endif
#ifndef RPT_XMLFORMATTEDFIELD_HXX
#include "xmlFormattedField.hxx"
#endif
#ifndef RPT_XMLIMAGE_HXX
#include "xmlImage.hxx"
#endif
#ifndef RPT_XMLFIXEDCONTENT_HXX
#include "xmlFixedContent.hxx"
#endif
#include "xmlSubDocument.hxx"

namespace rptxml
{
    using namespace ::comphelper;
    using namespace ::com::sun::star;
    using namespace uno;
    using namespace beans;
    using namespace xml::sax;

DBG_NAME( rpt_OXMLCell )

OXMLCell::OXMLCell( ORptFilter& rImport
                ,sal_uInt16 nPrfx
                ,const ::rtl::OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,OXMLTable* _pContainer
                ,OXMLCell* _pCell) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_pContainer(_pContainer)
    ,m_pCell(_pCell)
    ,m_nCurrentCount(0)
    ,m_bContainsShape(false)
{
    DBG_CTOR( rpt_OXMLCell,NULL);
    if ( !m_pCell )
        m_pCell = this;

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetColumnTokenMap();

    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        ::rtl::OUString sLocalName;
        const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        const rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_COLUMN_STYLE_NAME:
                m_sStyleName = sValue;
                break;
            case XML_TOK_NUMBER_COLUMNS_SPANNED:
                m_pContainer->setColumnSpanned(sValue.toInt32());
                break;
            case XML_TOK_NUMBER_ROWS_SPANNED:
                m_pContainer->setRowSpanned(sValue.toInt32());
                break;
            default:
                break;
        }
    }
}
// -----------------------------------------------------------------------------
OXMLCell::~OXMLCell()
{
    DBG_DTOR( rpt_OXMLCell,NULL);
}
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLCell::CreateChildContext(
        sal_uInt16 _nPrefix,
        const ::rtl::OUString& _rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    ORptFilter& rImport = GetOwnImport();
    const SvXMLTokenMap&    rTokenMap   = rImport.GetCellElemTokenMap();
    Reference<XMultiServiceFactory> xFactor(rImport.GetModel(),uno::UNO_QUERY);
    static const ::rtl::OUString s_sStringConcat(RTL_CONSTASCII_USTRINGPARAM(" & "));

    const sal_uInt16 nToken = rTokenMap.Get( _nPrefix, _rLocalName );
    switch( nToken )
    {
        case XML_TOK_FIXED_CONTENT:
            {
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                pContext = new OXMLFixedContent( rImport, _nPrefix, _rLocalName,*m_pCell,m_pContainer);
            }
            break;
        case XML_TOK_PAGE_NUMBER:
            m_sText += s_sStringConcat + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" PageNumber()"));
            break;
        case XML_TOK_PAGE_COUNT:
            m_sText += s_sStringConcat + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" PageCount()"));
            break;
        case XML_TOK_FORMATTED_TEXT:
            {
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                uno::Reference< uno::XInterface> xInt = xFactor->createInstance(SERVICE_FORMATTEDFIELD);
                Reference< report::XFormattedField > xControl(xInt,uno::UNO_QUERY);

                OSL_ENSURE(xControl.is(),"Could not create FormattedField!");
                setComponent(xControl.get());
                if ( xControl.is() )
                    pContext = new OXMLFormattedField( rImport, _nPrefix, _rLocalName,xAttrList,xControl.get(),m_pContainer,XML_TOK_PAGE_COUNT == nToken);
            }
            break;
        case XML_TOK_IMAGE:
            {
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                Reference< XImageControl > xControl(xFactor->createInstance(SERVICE_IMAGECONTROL),uno::UNO_QUERY);

                OSL_ENSURE(xControl.is(),"Could not create ImageControl!");
                setComponent(xControl.get());
                if ( xControl.is() )
                    pContext = new OXMLImage( rImport, _nPrefix, _rLocalName,xAttrList,xControl.get(),m_pContainer);
            }
            break;
        case XML_TOK_SUB_DOCUMENT:
            {
                rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                if ( !m_bContainsShape )
                    m_nCurrentCount = m_pContainer->getSection()->getCount();
                uno::Reference< uno::XInterface> xInt = xFactor->createInstance(SERVICE_FORMATTEDFIELD);
                Reference< report::XFormattedField > xControl(xInt,uno::UNO_QUERY);
                pContext = new OXMLSubDocument( rImport, _nPrefix, _rLocalName,xControl.get(),m_pContainer);
            }
            break;

        case XML_TOK_P:
            pContext = new OXMLCell( rImport, _nPrefix, _rLocalName,xAttrList ,m_pContainer,this);
            break;
        case XML_TOK_CUSTOM_SHAPE:
        case XML_TOK_FRAME:
            {
                if ( !m_bContainsShape )
                    m_nCurrentCount = m_pContainer->getSection()->getCount();
                UniReference< XMLShapeImportHelper > xShapeImportHelper = rImport.GetShapeImport();
                uno::Reference< drawing::XShapes > xShapes = m_pContainer->getSection().get();
                pContext = xShapeImportHelper->CreateGroupChildContext(rImport,_nPrefix,_rLocalName,xAttrList,xShapes);
                m_bContainsShape = true;
            }
            break;
        default:
            break;
    }

    if ( m_xComponent.is() )
        m_pContainer->addCell(m_xComponent);

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), _nPrefix, _rLocalName );

    return pContext;
}
// -----------------------------------------------------------------------------
void OXMLCell::EndElement()
{
    if ( m_bContainsShape )
    {
        const sal_Int32 nCount = m_pContainer->getSection()->getCount();
        for (sal_Int32 i = m_nCurrentCount; i < nCount; ++i)
        {
            uno::Reference<report::XShape> xShape(m_pContainer->getSection()->getByIndex(i),uno::UNO_QUERY);
            if ( xShape.is() )
                m_pContainer->addCell(xShape.get());
        }
    }
    if ( m_pCell != this && m_sText.getLength() )
    {
        ORptFilter& rImport = GetOwnImport();
        Reference<XMultiServiceFactory> xFactor(rImport.GetModel(),uno::UNO_QUERY);
        uno::Reference< uno::XInterface> xInt = xFactor->createInstance(SERVICE_FORMATTEDFIELD);
        Reference< report::XFormattedField > xControl(xInt,uno::UNO_QUERY);
        xControl->setDataField(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("rpt:")) + m_sText);

        OSL_ENSURE(xControl.is(),"Could not create FormattedField!");
        setComponent(xControl.get());
        m_xComponent = xControl.get();
        m_pContainer->getSection()->add(m_xComponent.get());
        m_pContainer->addCell(m_xComponent);
    }
    // check if we have a FixedLine
    else if ( m_sStyleName.getLength() && !m_xComponent.is() && m_pCell == this )
    {
        ORptFilter& rImport = GetOwnImport();
        Reference<XMultiServiceFactory> xFactor(rImport.GetModel(),uno::UNO_QUERY);
        Reference<XFixedLine> xFixedLine(xFactor->createInstance(SERVICE_FIXEDLINE),uno::UNO_QUERY);
        m_xComponent = xFixedLine.get();
        m_pContainer->getSection()->add(m_xComponent.get());
        m_pContainer->addCell(m_xComponent);
        XMLPropStyleContext* pAutoStyle = PTR_CAST(XMLPropStyleContext,GetImport().GetAutoStyles()->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_CELL,m_sStyleName));
        if ( pAutoStyle )
        {
            uno::Reference<beans::XPropertySet> xBorderProp = OXMLHelper::createBorderPropertySet();
            try
            {
                pAutoStyle->FillPropertySet(xBorderProp);
                table::BorderLine aRight,aLeft;
                xBorderProp->getPropertyValue(PROPERTY_BORDERRIGHT) >>= aRight;
                xBorderProp->getPropertyValue(PROPERTY_BORDERLEFT) >>= aLeft;
                xFixedLine->setOrientation( (aRight.OuterLineWidth != 0 || aLeft.OuterLineWidth != 0) ? 1 : 0);
               }
            catch(uno::Exception&)
            {
                OSL_ENSURE(0,"OXMLCell::EndElement -> exception catched");
            }
        }
    }
    else
        OXMLHelper::copyStyleElements(m_sStyleName,GetImport().GetAutoStyles(),m_xComponent.get());
}
// -----------------------------------------------------------------------------
ORptFilter& OXMLCell::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
void OXMLCell::setComponent(const uno::Reference< report::XReportComponent >& _xComponent)
{
    m_pCell->m_xComponent = _xComponent;
    m_xComponent = _xComponent;
}
// -----------------------------------------------------------------------------
void OXMLCell::Characters( const ::rtl::OUString& rChars )
{
    if ( rChars.getLength() )
    {
        static const ::rtl::OUString s_Quote(RTL_CONSTASCII_USTRINGPARAM("\""));
        if ( m_sText.getLength() )
        {
            static const ::rtl::OUString s_sStringConcat(RTL_CONSTASCII_USTRINGPARAM(" & "));
            m_sText += s_sStringConcat;
        }

        m_sText += s_Quote + rChars + s_Quote;
    }
}

//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------

