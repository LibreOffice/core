/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlCondPrtExpr.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2007-09-26 14:21:04 $
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

#ifndef RPT_XMLCONDPRTEXPR_HXX
#include "xmlCondPrtExpr.hxx"
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
#ifndef RPT_XMLCONTROLPROPERTY_HXX
#include "xmlControlProperty.hxx"
#endif
#ifndef RPT_XMLCOMPONENT_HXX
#include "xmlComponent.hxx"
#endif
#ifndef REPORTDESIGN_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace report;
    using namespace uno;
    using namespace xml::sax;
DBG_NAME( rpt_OXMLCondPrtExpr )

OXMLCondPrtExpr::OXMLCondPrtExpr( ORptFilter& _rImport,
                sal_uInt16 nPrfx
                ,const ::rtl::OUString& rLName
                ,const uno::Reference< xml::sax::XAttributeList > & _xAttrList
                ,const Reference< XPropertySet > & _xComponent ) :
    SvXMLImportContext( _rImport, nPrfx, rLName )
,m_xComponent(_xComponent)
{
    DBG_CTOR( rpt_OXMLCondPrtExpr,NULL);

    OSL_ENSURE(m_xComponent.is(),"Component is NULL!");
    const SvXMLNamespaceMap& rMap = _rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = _rImport.GetFunctionElemTokenMap();
    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    try
    {
        for(sal_Int16 i = 0; i < nLength; ++i)
        {
            ::rtl::OUString sLocalName;
            const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
            const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
            const rtl::OUString sValue = _xAttrList->getValueByIndex( i );

            switch( rTokenMap.Get( nPrefix, sLocalName ) )
            {
                case XML_TOK_FUNCTION_FORMULA:
                    m_xComponent->setPropertyValue(PROPERTY_CONDITIONALPRINTEXPRESSION,uno::makeAny(ORptFilter::convertFormula(sValue)));
                    break;
                default:
                    break;
            }

        }
    }
    catch(const Exception&)
    {
        OSL_ENSURE(0,"Exception catched while putting Function props!");
    }
}
// -----------------------------------------------------------------------------

OXMLCondPrtExpr::~OXMLCondPrtExpr()
{

    DBG_DTOR( rpt_OXMLCondPrtExpr,NULL);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
void OXMLCondPrtExpr::Characters( const ::rtl::OUString& rChars )
{
    m_xComponent->setPropertyValue(PROPERTY_CONDITIONALPRINTEXPRESSION,makeAny(rChars));
}
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
