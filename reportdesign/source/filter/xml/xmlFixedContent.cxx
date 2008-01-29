/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlFixedContent.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2008-01-29 13:45:26 $
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

#ifndef RPT_XMLFIXEDCONTENT_HXX
#include "xmlFixedContent.hxx"
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
#ifndef RPT_XMLREPORTELEMENT_HXX
#include "xmlReportElement.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COM_SUN_STAR_REPORT_XSHAPE_HPP_
#include <com/sun/star/report/XShape.hpp>
#endif
#ifndef RPT_XMLFIXEDTEXT_HXX
#include "xmlFixedText.hxx"
#endif
#ifndef RPT_XMLCELL_HXX
#include "xmlCell.hxx"
#endif
#ifndef RPT_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#ifndef _COM_SUN_STAR_REPORT_XSHAPE_HPP_
#include <com/sun/star/report/XShape.hpp>
#endif
#include <com/sun/star/report/XFormattedField.hpp>
#include <com/sun/star/text/ControlCharacter.hpp>
#ifndef RPT_XMLTABLE_HXX
#include "xmlTable.hxx"
#endif
#include <xmloff/XMLCharContext.hxx>

namespace rptxml
{
    using namespace ::com::sun::star;

class OXMLCharContent : public XMLCharContext
{
    OXMLFixedContent* m_pFixedContent;
    OXMLCharContent(const OXMLCharContent&);
    void operator =(const OXMLCharContent&);
public:
    OXMLCharContent(
            SvXMLImport& rImport,
            OXMLFixedContent* _pFixedContent,
            sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList,
            sal_Unicode c,
            sal_Bool bCount );
    OXMLCharContent(
            SvXMLImport& rImport,
            OXMLFixedContent* _pFixedContent,
            sal_uInt16 nPrfx,
            const ::rtl::OUString& rLName,
            const uno::Reference< xml::sax::XAttributeList > & xAttrList,
            sal_Int16 nControl );

    virtual void InsertControlCharacter(sal_Int16   _nControl);
    virtual void InsertString(const ::rtl::OUString& _sString);
};
OXMLCharContent::OXMLCharContent(
        SvXMLImport& rImport,
        OXMLFixedContent* _pFixedContent,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Unicode c,
        sal_Bool bCount )
    : XMLCharContext(rImport,nPrfx,rLName,xAttrList,c,bCount)
    ,m_pFixedContent(_pFixedContent)
{
}
// -----------------------------------------------------------------------------
OXMLCharContent::OXMLCharContent(
        SvXMLImport& rImport,
        OXMLFixedContent* _pFixedContent,
        sal_uInt16 nPrfx,
        const ::rtl::OUString& rLName,
        const uno::Reference< xml::sax::XAttributeList > & xAttrList,
        sal_Int16 nControl )
    : XMLCharContext(rImport,nPrfx,rLName,xAttrList,nControl)
    ,m_pFixedContent(_pFixedContent)
{
}
// -----------------------------------------------------------------------------
void OXMLCharContent::InsertControlCharacter(sal_Int16   _nControl)
{
    switch( _nControl )
    {
        case ControlCharacter::LINE_BREAK:
            m_pFixedContent->Characters(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("\n")));
            break;
        default:
            OSL_ENSURE(0,"Not supported control character");
            break;
    }
}
// -----------------------------------------------------------------------------
void OXMLCharContent::InsertString(const ::rtl::OUString& _sString)
{
    m_pFixedContent->Characters(_sString);
}
// -----------------------------------------------------------------------------

DBG_NAME( rpt_OXMLFixedContent )

OXMLFixedContent::OXMLFixedContent( ORptFilter& rImport,
                sal_uInt16 nPrfx, const ::rtl::OUString& rLName
                ,OXMLCell& _rCell
                ,OXMLTable* _pContainer
                ,OXMLFixedContent* _pInP) :
    OXMLReportElementBase( rImport, nPrfx, rLName,NULL,_pContainer)
,m_rCell(_rCell)
,m_pInP(_pInP)
,m_bFormattedField(false)
{
    DBG_CTOR( rpt_OXMLFixedContent,NULL);
}
// -----------------------------------------------------------------------------

OXMLFixedContent::~OXMLFixedContent()
{

    DBG_DTOR( rpt_OXMLFixedContent,NULL);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLFixedContent::_CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = OXMLReportElementBase::_CreateChildContext(nPrefix,rLocalName,xAttrList);
    if ( pContext )
        return pContext;

    static const ::rtl::OUString s_sStringConcat(RTL_CONSTASCII_USTRINGPARAM(" & "));
    const SvXMLTokenMap&    rTokenMap   = m_rImport.GetCellElemTokenMap();
    Reference<XMultiServiceFactory> xFactor = m_rImport.getServiceFactory();

    m_rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
    const sal_uInt16 nToken = rTokenMap.Get( nPrefix, rLocalName );
    switch( nToken )
    {
        case XML_TOK_P:
            pContext = new OXMLFixedContent(m_rImport,nPrefix, rLocalName,m_rCell,m_pContainer,this);
            break;
        case XML_TOK_TEXT_TAB_STOP:
            pContext = new OXMLCharContent( m_rImport, this,nPrefix,
                                                rLocalName, xAttrList,
                                                0x0009, sal_False );
            break;

        case XML_TOK_TEXT_LINE_BREAK:
            pContext = new OXMLCharContent( m_rImport, this,nPrefix,
                                                rLocalName, xAttrList,
                                                ControlCharacter::LINE_BREAK );
            break;

        case XML_TOK_TEXT_S:
            pContext = new OXMLCharContent( m_rImport, this,nPrefix,
                                                rLocalName, xAttrList,
                                                0x0020, sal_True );
            break;
        case XML_TOK_PAGE_NUMBER:
            m_sPageText += s_sStringConcat + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" PageNumber()"));
            m_bFormattedField = true;
            break;
        case XML_TOK_PAGE_COUNT:
            m_sPageText += s_sStringConcat + ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM(" PageCount()"));
            m_bFormattedField = true;
            break;
        default:
            ;
    }
    return pContext;
}
// -----------------------------------------------------------------------------
void OXMLFixedContent::EndElement()
{
    if ( m_pInP )
    {
        const Reference<XMultiServiceFactory> xFactor(m_rImport.GetModel(),uno::UNO_QUERY);
        if ( m_bFormattedField )
        {
            uno::Reference< uno::XInterface> xInt = xFactor->createInstance(SERVICE_FORMATTEDFIELD);
            Reference< report::XFormattedField > xControl(xInt,uno::UNO_QUERY);
            xControl->setDataField(::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("rpt:")) + m_sPageText);
             OSL_ENSURE(xControl.is(),"Could not create FormattedField!");
            m_pInP->m_xComponent = xControl.get();
            m_xComponent = xControl.get();
        }
        else
        {
            Reference< XFixedText > xControl(xFactor->createInstance(SERVICE_FIXEDTEXT),uno::UNO_QUERY);
             OSL_ENSURE(xControl.is(),"Could not create FixedContent!");
            m_pInP->m_xComponent = xControl.get();
            m_xComponent = xControl.get();
            xControl->setLabel(m_sLabel);
        }

        m_pContainer->addCell(m_xComponent);
        m_rCell.setComponent(m_xComponent);

        OXMLReportElementBase::EndElement();
    }
}
// -----------------------------------------------------------------------------
void OXMLFixedContent::Characters( const ::rtl::OUString& rChars )
{
    m_sLabel += rChars;
    if ( rChars.getLength() )
    {
        static const ::rtl::OUString s_Quote(RTL_CONSTASCII_USTRINGPARAM("\""));
        if ( m_sPageText.getLength() )
        {
            static const ::rtl::OUString s_sStringConcat(RTL_CONSTASCII_USTRINGPARAM(" & "));
            m_sPageText += s_sStringConcat;
        }

        m_sPageText += s_Quote + rChars + s_Quote;
    }
}

//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
