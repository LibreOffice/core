/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlFormattedField.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 19:01:58 $
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

#ifndef RPT_XMLFORMATTEDFIELD_HXX
#include "xmlFormattedField.hxx"
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
#ifndef RPT_XMLHELPER_HXX
#include "xmlHelper.hxx"
#endif
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef RPT_XMLREPORTELEMENT_HXX
#include "xmlReportElement.hxx"
#endif
#ifndef RPT_XMLCOMPONENT_HXX
#include "xmlComponent.hxx"
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace xml::sax;
DBG_NAME( rpt_OXMLFormattedField )

OXMLFormattedField::OXMLFormattedField( ORptFilter& rImport,
                sal_uInt16 nPrfx, const ::rtl::OUString& rLName
                ,const uno::Reference< xml::sax::XAttributeList > & _xAttrList
                ,const uno::Reference< XFormattedField > & _xComponent
                ,OXMLTable* _pContainer
                ,bool _bPageCount) :
    OXMLReportElementBase( rImport, nPrfx, rLName,_xComponent.get(),_pContainer)
{
    DBG_CTOR( rpt_OXMLFormattedField,NULL);
    OSL_ENSURE(m_xComponent.is(),"Component is NULL!");
    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetControlElemTokenMap();

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
                case XML_TOK_DATA_FORMULA:
                    _xComponent->setDataField(ORptFilter::convertFormula(sValue));
                    break;
                case XML_TOK_SELECT_PAGE:
                    {
                        static const ::rtl::OUString s_sPageNumber(RTL_CONSTASCII_USTRINGPARAM("rpt:PageNumber()"));
                        _xComponent->setDataField(s_sPageNumber);
                    }
                    break;
                default:
                    break;
            }
        }
        if ( _bPageCount )
        {
            static const ::rtl::OUString s_sPageNumber(RTL_CONSTASCII_USTRINGPARAM("rpt:PageCount()"));
            _xComponent->setDataField(s_sPageNumber);
        }
    }
    catch(Exception&)
    {
        OSL_ENSURE(0,"Exception catched while filling the report definition props");
    }
}
// -----------------------------------------------------------------------------
OXMLFormattedField::~OXMLFormattedField()
{
    DBG_DTOR( rpt_OXMLFormattedField,NULL);
}
// -----------------------------------------------------------------------------
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
