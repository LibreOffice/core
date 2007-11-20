/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlTable.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 19:04:17 $
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

#ifndef RPT_XMLTABLE_HXX
#include "xmlTable.hxx"
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
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _REPORT_RPTUIDEF_HXX
#include "RptDef.hxx"
#endif
#ifndef RPT_XMLHELPER_HXX
#include "xmlHelper.hxx"
#endif
#ifndef RPT_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef RPT_XMLCOLUMN_HXX
#include "xmlColumn.hxx"
#endif
#ifndef _COM_SUN_STAR_REPORT_FORCENEWPAGE_HPP_
#include <com/sun/star/report/ForceNewPage.hpp>
#endif
#ifndef RPT_XMLCONDPRTEXPR_HXX
#include "xmlCondPrtExpr.hxx"
#endif
#ifndef RPT_XMLSTYLEIMPORT_HXX
#include "xmlStyleImport.hxx"
#endif
#include "xmlstrings.hrc"
#ifndef _CONNECTIVITY_DBTOOLS_HXX_
#include <connectivity/dbtools.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef REPORTDESIGN_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#include <com/sun/star/report/XShape.hpp>
#include <com/sun/star/report/XFixedLine.hpp>

#define MIN_WIDTH   80
#define MIN_HEIGHT  20

namespace rptxml
{
    using namespace ::xmloff;
    using namespace ::com::sun::star;
    using ::com::sun::star::uno::Reference;
    using namespace ::com::sun::star::xml::sax;
    using ::com::sun::star::xml::sax::XAttributeList;

    sal_uInt16 lcl_getForceNewPageOption(const ::rtl::OUString& _sValue)
    {
        sal_uInt16 nRet = report::ForceNewPage::NONE;
        const SvXMLEnumMapEntry* aXML_EnumMap = OXMLHelper::GetForceNewPageOptions();
        SvXMLUnitConverter::convertEnum( nRet,_sValue,aXML_EnumMap );
        return nRet;
    }
DBG_NAME( rpt_OXMLTable )

OXMLTable::OXMLTable( ORptFilter& rImport
                ,sal_uInt16 nPrfx
                ,const ::rtl::OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const uno::Reference< report::XSection >& _xSection
                )
:SvXMLImportContext( rImport, nPrfx, _sLocalName )
,m_xSection(_xSection)
,m_nColSpan(1)
,m_nRowSpan(0)
,m_nRowIndex(0)
,m_nColumnIndex(0)
{
    DBG_CTOR( rpt_OXMLTable,NULL);
    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetSectionElemTokenMap();

    const sal_Int16 nLength = (m_xSection.is() && _xAttrList.is()) ? _xAttrList->getLength() : 0;
    static const ::rtl::OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    try
    {
        for(sal_Int16 i = 0; i < nLength; ++i)
        {
            rtl::OUString sLocalName;
            const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
            const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
            const rtl::OUString sValue = _xAttrList->getValueByIndex( i );

            switch( rTokenMap.Get( nPrefix, sLocalName ) )
            {
                case XML_TOK_VISIBLE:
                    m_xSection->setVisible(sValue == s_sTRUE);
                    break;
                case XML_TOK_FORCE_NEW_PAGE:
                    m_xSection->setForceNewPage(lcl_getForceNewPageOption(sValue));
                    break;
                case XML_TOK_FORCE_NEW_COLUMN:
                    m_xSection->setNewRowOrCol(lcl_getForceNewPageOption(sValue));
                    break;
                case XML_TOK_KEEP_TOGETHER:
                    m_xSection->setKeepTogether(sValue == s_sTRUE);
                    break;
                case XML_TOK_SECTION_NAME:
                    m_xSection->setName(sValue);
                    break;
                case XML_TOK_SECT_STYLE_NAME:
                    m_sStyleName = sValue;
                    break;
                default:
                    break;
            }
        }
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"Exception catched while filling the section props");
    }
}
// -----------------------------------------------------------------------------
OXMLTable::~OXMLTable()
{
    DBG_DTOR( rpt_OXMLTable,NULL);
}
// -----------------------------------------------------------------------------

SvXMLImportContext* OXMLTable::CreateChildContext(
        sal_uInt16 _nPrefix,
        const ::rtl::OUString& _rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    ORptFilter& rImport = GetOwnImport();
    const SvXMLTokenMap&    rTokenMap   = rImport.GetColumnTokenMap();
    Reference<XMultiServiceFactory> xFactor = rImport.getServiceFactory();

    switch( rTokenMap.Get( _nPrefix, _rLocalName ) )
    {
        case XML_TOK_TABLE_COLUMNS:
        case XML_TOK_TABLE_ROWS:
            pContext = new OXMLRowColumn( rImport, _nPrefix, _rLocalName,xAttrList ,this);
            break;
        case XML_TOK_ROW:
            incrementRowIndex();
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLRowColumn( rImport, _nPrefix, _rLocalName,xAttrList,this);
            break;
        case XML_TOK_COLUMN:
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLRowColumn( rImport, _nPrefix, _rLocalName,xAttrList,this);
            break;
        case XML_TOK_CONDITIONAL_PRINT_EXPRESSION:
            pContext = new OXMLCondPrtExpr( rImport, _nPrefix, _rLocalName,xAttrList,m_xSection.get());
            break;
        default:
                break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( rImport, _nPrefix, _rLocalName );

    return pContext;
}
// -----------------------------------------------------------------------------
ORptFilter& OXMLTable::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
void OXMLTable::EndElement()
{
    try
    {
        if ( m_xSection.is() )
        {
            if ( m_sStyleName.getLength() )
            {
                const SvXMLStylesContext* pAutoStyles = GetImport().GetAutoStyles();
                if ( pAutoStyles )
                {
                    XMLPropStyleContext* pAutoStyle = PTR_CAST(XMLPropStyleContext,pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_TABLE,m_sStyleName));
                    if ( pAutoStyle )
                    {
                        pAutoStyle->FillPropertySet(m_xSection.get());
                    }
                }
            } // if ( m_sStyleName.getLength() )
            // set height
            ::std::vector<sal_Int32>::iterator aIter = m_aHeight.begin();
            ::std::vector<sal_Int32>::iterator aEnd = m_aHeight.end();
            sal_Int32 nHeight = 0;
            for (; aIter != aEnd; ++aIter)
                nHeight += *aIter;
            m_xSection->setHeight( nHeight );
            // set positions, widths, and heights
            sal_Int32 nLeftMargin = rptui::getStyleProperty<sal_Int32>(m_xSection->getReportDefinition(),PROPERTY_LEFTMARGIN);
            sal_Int32 nPosY = 0;
            ::std::vector< ::std::vector<TCell> >::iterator aRowIter = m_aGrid.begin();
            ::std::vector< ::std::vector<TCell> >::iterator aRowEnd = m_aGrid.end();
            for (sal_Int32 i = 0; aRowIter != aRowEnd; ++aRowIter,++i)
            {
                sal_Int32 nPosX = nLeftMargin;
                ::std::vector<TCell>::iterator aColIter = (*aRowIter).begin();
                ::std::vector<TCell>::iterator aColEnd = (*aRowIter).end();
                for (sal_Int32 j = 0; aColIter != aColEnd; ++aColIter,++j)
                {
                    TCell& rCell = *aColIter;
                    if ( !rCell.xElements.empty())
                    {
                        ::std::vector< uno::Reference< report::XReportComponent> >::iterator aCellIter = rCell.xElements.begin();
                        const ::std::vector< uno::Reference< report::XReportComponent> >::iterator aCellEnd = rCell.xElements.end();
                        for (;aCellIter != aCellEnd ; ++aCellIter)
                        {
                            uno::Reference<report::XShape> xShape(*aCellIter,uno::UNO_QUERY);
                            if ( xShape.is() )
                            {
                                xShape->setPositionX(xShape->getPositionX() + nLeftMargin);
                            }
                            else
                            {
                                sal_Int32 nWidth = rCell.nWidth;
                                sal_Int32 nColSpan = rCell.nColSpan;
                                if ( nColSpan > 1 )
                                {
                                    ::std::vector<TCell>::iterator aWidthIter = aColIter + 1;
                                    while ( nColSpan > 1 )
                                    {
                                        nWidth += (aWidthIter++)->nWidth;
                                        --nColSpan;
                                    }
                                }
                                nHeight = rCell.nHeight;
                                sal_Int32 nRowSpan = rCell.nRowSpan;
                                if ( nRowSpan > 1 )
                                {
                                    ::std::vector< ::std::vector<TCell> >::iterator aHeightIter = aRowIter + 1;
                                    while( nRowSpan > 1)
                                    {
                                        nHeight += (*aHeightIter)[j].nHeight;
                                        ++aHeightIter;
                                        --nRowSpan;
                                    }
                                }
                                Reference<XFixedLine> xFixedLine(*aCellIter,uno::UNO_QUERY);
                                if ( xFixedLine.is() )
                                {
                                    if ( xFixedLine->getOrientation() == 1 ) // vertical
                                    {
                                        OSL_ENSURE(static_cast<sal_uInt32>(j+1) < m_aWidth.size(),"Illegal pos of col iter. There should be an empty cell for the next line part.");
                                        nWidth += m_aWidth[j+1];
                                        if ( nWidth < MIN_WIDTH )
                                            nWidth = MIN_WIDTH;
                                    }
                                    else if ( nHeight < MIN_HEIGHT )
                                        nHeight = MIN_HEIGHT;
                                }
                                try
                                {
                                    (*aCellIter)->setSize(awt::Size(nWidth,nHeight));
                                    (*aCellIter)->setPosition(awt::Point(nPosX,nPosY));
                                }
                                catch(beans::PropertyVetoException)
                                {
                                    OSL_ENSURE(0,"Could not set the correct position or size!");
                                }
                            }
                        }
                    }
                    nPosX += m_aWidth[j];
                }
                nPosY += m_aHeight[i];
            }
        } // if ( m_xComponent.is() )
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"OXMLTable::EndElement -> exception catched");
    }
}
// -----------------------------------------------------------------------------
void OXMLTable::addCell(const Reference<XReportComponent>& _xElement)
{
    uno::Reference<report::XShape> xShape(_xElement,uno::UNO_QUERY);
    OSL_ENSURE(static_cast<sal_uInt32>(m_nRowIndex-1 ) < m_aGrid.size() && static_cast<sal_uInt32>( m_nColumnIndex-1 ) < m_aGrid[m_nRowIndex-1].size(),
        "OXMLTable::addCell: Invalid column and row index");
    if ( static_cast<sal_uInt32>(m_nRowIndex-1 ) < m_aGrid.size() && static_cast<sal_uInt32>( m_nColumnIndex-1 ) < m_aGrid[m_nRowIndex-1].size() )
    {
        TCell& rCell = m_aGrid[m_nRowIndex-1][m_nColumnIndex-1];
        if ( _xElement.is() )
            rCell.xElements.push_back( _xElement );
        if ( !xShape.is() )
        {
            rCell.nWidth   = m_aWidth[m_nColumnIndex-1];
            rCell.nHeight  = m_aHeight[m_nRowIndex-1];
            rCell.nColSpan = m_nColSpan;
            rCell.nRowSpan = m_nRowSpan;
        }
    }

    if ( !xShape.is() )
        m_nColSpan = m_nRowSpan = 1;
}
// -----------------------------------------------------------------------------
void OXMLTable::incrementRowIndex()
{
    ++m_nRowIndex;
    m_nColumnIndex = 0;
    m_aGrid.push_back(::std::vector<TCell>(m_aWidth.size()));
}
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
