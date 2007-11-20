/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlFunction.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 19:02:13 $
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

#ifndef RPT_XMLFUNCTION_HXX
#include "xmlFunction.hxx"
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
#ifndef RPT_XMLHELPER_HXX
#include "xmlHelper.hxx"
#endif
#ifndef RPT_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#include "xmlstrings.hrc"
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::xml::sax;

DBG_NAME( rpt_OXMLFunction )

OXMLFunction::OXMLFunction( ORptFilter& _rImport
                ,sal_uInt16 nPrfx
                ,const ::rtl::OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const Reference< XFunctionsSupplier >& _xFunctions
                ,bool _bAddToReport
                ) :
    SvXMLImportContext( _rImport, nPrfx, _sLocalName )
    ,m_xFunctions(_xFunctions->getFunctions())
    ,m_bAddToReport(_bAddToReport)
{
    DBG_CTOR( rpt_OXMLFunction,NULL);

    OSL_ENSURE(m_xFunctions.is(),"Functions is NULL!");
    m_xFunction = m_xFunctions->createFunction();

    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");

    const SvXMLNamespaceMap& rMap = _rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = _rImport.GetFunctionElemTokenMap();

    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    static const ::rtl::OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
     ::rtl::OUString sLocalName;
        const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        const rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        try
        {
            switch( rTokenMap.Get( nPrefix, sLocalName ) )
            {
                case XML_TOK_FUNCTION_NAME:
                    m_xFunction->setName(sValue);
                    break;
                case XML_TOK_FUNCTION_FORMULA:
                    m_xFunction->setFormula(ORptFilter::convertFormula(sValue));
                    break;
                case XML_TOK_PRE_EVALUATED:
                    m_xFunction->setPreEvaluated(sValue == s_sTRUE);
                    break;
                case XML_TOK_INITIAL_FORMULA:
                    if ( sValue.getLength() )
                        m_xFunction->setInitialFormula(beans::Optional< ::rtl::OUString>(sal_True,ORptFilter::convertFormula(sValue)));
                    break;
                case XML_TOK_DEEP_TRAVERSING:
                    m_xFunction->setDeepTraversing(sValue == s_sTRUE);
                    break;
                default:
                    break;
            }
        }
        catch(const Exception&)
        {
            OSL_ENSURE(0,"Exception catched while putting Function props!");
        }
    }
}
// -----------------------------------------------------------------------------

OXMLFunction::~OXMLFunction()
{
    DBG_DTOR( rpt_OXMLFunction,NULL);
}
// -----------------------------------------------------------------------------
ORptFilter& OXMLFunction::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
void OXMLFunction::EndElement()
{
    if ( m_bAddToReport )
    {
        GetOwnImport().insertFunction(m_xFunction);
        m_xFunction.clear();
    }
    else
    {
        try
        {
            m_xFunctions->insertByIndex(m_xFunctions->getCount(),uno::makeAny(m_xFunction));
            m_xFunction.clear();
        }catch(uno::Exception&)
        {
            OSL_ENSURE(0,"Exception catched!");
        }
    }
}
// -----------------------------------------------------------------------------
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
