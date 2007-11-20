/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlColumn.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 18:59:54 $
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

#ifndef RPT_XMLCOLUMN_HXX
#include "xmlColumn.hxx"
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
#ifndef RPT_XMLCELL_HXX
#include "xmlCell.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef RPT_XMLSTYLEIMPORT_HXX
#include "xmlStyleImport.hxx"
#endif
#ifndef RPT_XMLTABLE_HXX
#include "xmlTable.hxx"
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
#ifndef RPT_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif

#define PROPERTY_ID_WIDTH    1
#define PROPERTY_ID_HEIGHT   2

namespace rptxml
{
    using namespace ::comphelper;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xml::sax;

DBG_NAME( rpt_OXMLRowColumn )

OXMLRowColumn::OXMLRowColumn( ORptFilter& rImport
                ,sal_uInt16 nPrfx
                ,const ::rtl::OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,OXMLTable* _pContainer
                ) :
    SvXMLImportContext( rImport, nPrfx, _sLocalName )
    ,m_pContainer(_pContainer)
{
    DBG_CTOR( rpt_OXMLRowColumn,NULL);

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
                fillStyle(sValue);
                break;
            default:
                break;
        }
    }
}
// -----------------------------------------------------------------------------

OXMLRowColumn::~OXMLRowColumn()
{
    DBG_DTOR( rpt_OXMLRowColumn,NULL);
}
// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLRowColumn::CreateChildContext(
        sal_uInt16 nPrefix,
        const ::rtl::OUString& rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    ORptFilter& rImport = GetOwnImport();
    const SvXMLTokenMap&    rTokenMap   = rImport.GetColumnTokenMap();

    switch( rTokenMap.Get( nPrefix, rLocalName ) )
    {
        case XML_TOK_COLUMN:
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLRowColumn( rImport, nPrefix, rLocalName,xAttrList,m_pContainer);
            break;
        case XML_TOK_ROW:
            m_pContainer->incrementRowIndex();
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLRowColumn( rImport, nPrefix, rLocalName,xAttrList,m_pContainer);
            break;
        case XML_TOK_CELL:
            m_pContainer->incrementColumnIndex();
            rImport.GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
            pContext = new OXMLCell( rImport, nPrefix, rLocalName,xAttrList,m_pContainer);
            break;
        case XML_TOK_COV_CELL:
            m_pContainer->incrementColumnIndex();
            m_pContainer->addCell(NULL);
            break;
        default:
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), nPrefix, rLocalName );

    return pContext;
}
// -----------------------------------------------------------------------------
void OXMLRowColumn::fillStyle(const ::rtl::OUString& _sStyleName)
{
    if ( _sStyleName.getLength() )
    {
        const SvXMLStylesContext* pAutoStyles = GetOwnImport().GetAutoStyles();
        if ( pAutoStyles )
        {
            PropertySetInfo* pInfo = new PropertySetInfo();
            static PropertyMapEntry pMap[] =
            {
                {PROPERTY_WIDTH,        static_cast<sal_uInt16>(PROPERTY_WIDTH.length),     PROPERTY_ID_WIDTH,          &::getCppuType(static_cast< sal_Int32* >( NULL ))       ,PropertyAttribute::BOUND,0},
                {PROPERTY_HEIGHT,       static_cast<sal_uInt16>(PROPERTY_HEIGHT.length),    PROPERTY_ID_HEIGHT,         &::getCppuType(static_cast< sal_Int32* >( NULL ))       ,PropertyAttribute::BOUND,0},
                { NULL, 0, 0, NULL, 0, 0 }
            };
            pInfo->add(pMap);
            Reference<XPropertySet> xProp = GenericPropertySet_CreateInstance(pInfo);
            XMLPropStyleContext* pAutoStyle = PTR_CAST(XMLPropStyleContext,pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_COLUMN,_sStyleName));
            if ( pAutoStyle )
            {
                pAutoStyle->FillPropertySet(xProp);
                sal_Int32 nWidth = 0;
                xProp->getPropertyValue(PROPERTY_WIDTH) >>= nWidth;
                m_pContainer->addWidth(nWidth);
            }
            else
            {
                pAutoStyle = PTR_CAST(XMLPropStyleContext,pAutoStyles->FindStyleChildContext(XML_STYLE_FAMILY_TABLE_ROW,_sStyleName));
                if ( pAutoStyle )
                {
                    pAutoStyle->FillPropertySet(xProp);
                    sal_Int32 nHeight = 0;
                    xProp->getPropertyValue(PROPERTY_HEIGHT) >>= nHeight;
                    m_pContainer->addHeight(nHeight);
                }
            }
        }
    }
}
// -----------------------------------------------------------------------------
ORptFilter& OXMLRowColumn::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
void OXMLRowColumn::EndElement()
{
}
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
