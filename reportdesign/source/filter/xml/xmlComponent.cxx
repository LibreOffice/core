/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlComponent.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-20 19:00:20 $
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

#ifndef RPT_XMLCOMPONENT_HXX
#include "xmlComponent.hxx"
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
#ifndef _XMLOFF_XMLUCONV_HXX
#include <xmloff/xmluconv.hxx>
#endif
#ifndef _XMLOFF_NMSPMAP_HXX
#include <xmloff/nmspmap.hxx>
#endif
#ifndef RPT_XMLENUMS_HXX
#include "xmlEnums.hxx"
#endif
#ifndef RPT_XMLHELPER_HXX
#include "xmlHelper.hxx"
#endif
#ifndef RPT_SHARED_XMLSTRINGS_HRC
#include "xmlstrings.hrc"
#endif
#ifndef RPT_XMLSTYLEIMPORT_HXX
#include "xmlStyleImport.hxx"
#endif
#ifndef _UCBHELPER_CONTENT_HXX
#include <ucbhelper/content.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _COMPHELPER_NAMECONTAINER_HXX_
#include <comphelper/namecontainer.hxx>
#endif
#ifndef _COMPHELPER_GENERICPROPERTYSET_HXX_
#include <comphelper/genericpropertyset.hxx>
#endif
#ifndef _COM_SUN_STAR_AWT_FONTDESCRIPTOR_HPP_
#include <com/sun/star/awt/FontDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_REPORT_XREPORTCONTROLMODEL_HPP_
#include <com/sun/star/report/XReportControlModel.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HXX_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

namespace rptxml
{
    using namespace ::comphelper;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::xml::sax;
    DBG_NAME( rpt_OXMLComponent )
OXMLComponent::OXMLComponent( ORptFilter& _rImport
                ,sal_uInt16 nPrfx
                ,const ::rtl::OUString& _sLocalName
                ,const Reference< XAttributeList > & _xAttrList
                ,const Reference< XReportComponent > & _xComponent
                ) :
    SvXMLImportContext( _rImport, nPrfx, _sLocalName )
    ,m_xComponent(_xComponent)
{
    DBG_CTOR( rpt_OXMLComponent,NULL);
    OSL_ENSURE(_xAttrList.is(),"Attribute list is NULL!");
    OSL_ENSURE(m_xComponent.is(),"Component is NULL!");

    const SvXMLNamespaceMap& rMap = _rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = _rImport.GetComponentElemTokenMap();

    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    static const ::rtl::OUString s_sTRUE = ::xmloff::token::GetXMLToken(XML_TRUE);

    for(sal_Int16 i = 0; i < nLength; ++i)
    {
        try
        {
            ::rtl::OUString sLocalName;
            const ::rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
            const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
            const ::rtl::OUString sValue = _xAttrList->getValueByIndex( i );

            switch( rTokenMap.Get( nPrefix, sLocalName ) )
            {
                case XML_TOK_NAME:
                    m_xComponent->setName(sValue);
                    break;
                case XML_TOK_TEXT_STYLE_NAME:
                    m_sTextStyleName = sValue;
                    break;
                case XML_TOK_TRANSFORM:
                    break;
                default:
                    break;
            }
        }
        catch(const Exception&)
        {
            OSL_ENSURE(0,"Exception catched while putting props into report component!");
        }
    }
}
// -----------------------------------------------------------------------------

OXMLComponent::~OXMLComponent()
{
    DBG_DTOR( rpt_OXMLComponent,NULL);
}
// -----------------------------------------------------------------------------
// -----------------------------------------------------------------------------
ORptFilter& OXMLComponent::GetOwnImport()
{
    return static_cast<ORptFilter&>(GetImport());
}
// -----------------------------------------------------------------------------
//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
