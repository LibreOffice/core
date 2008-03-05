/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xmlMasterFields.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2008-03-05 18:04:14 $
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

#ifndef RPT_XMLMASTERFIELDS_HXX
#include "xmlMasterFields.hxx"
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
#include "xmlReport.hxx"
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif


namespace rptxml
{
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::report;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::xml::sax;
DBG_NAME( rpt_OXMLMasterFields )

OXMLMasterFields::OXMLMasterFields( ORptFilter& rImport,
                sal_uInt16 nPrfx, const ::rtl::OUString& rLName,
                const Reference< XAttributeList > & _xAttrList
                ,IMasterDetailFieds* _pReport
                ) :
    SvXMLImportContext( rImport, nPrfx, rLName)
,m_pReport(_pReport)
{
    DBG_CTOR( rpt_OXMLMasterFields,NULL);

    const SvXMLNamespaceMap& rMap = rImport.GetNamespaceMap();
    const SvXMLTokenMap& rTokenMap = rImport.GetSubDocumentElemTokenMap();

    ::rtl::OUString sMasterField,sDetailField;
    const sal_Int16 nLength = (_xAttrList.is()) ? _xAttrList->getLength() : 0;
    for(sal_Int16 i = 0; i < nLength; ++i)
    {
     ::rtl::OUString sLocalName;
        const rtl::OUString sAttrName = _xAttrList->getNameByIndex( i );
        const sal_uInt16 nPrefix = rMap.GetKeyByAttrName( sAttrName,&sLocalName );
        const rtl::OUString sValue = _xAttrList->getValueByIndex( i );

        switch( rTokenMap.Get( nPrefix, sLocalName ) )
        {
            case XML_TOK_MASTER:
                sMasterField = sValue;
                break;
            case XML_TOK_SUB_DETAIL:
                sDetailField = sValue;
                break;
            default:
                break;
        }
    }
    if ( !sDetailField.getLength() )
        sDetailField = sMasterField;
    m_pReport->addMasterDetailPair(::std::pair< ::rtl::OUString,::rtl::OUString >(sMasterField,sDetailField));
}
// -----------------------------------------------------------------------------

OXMLMasterFields::~OXMLMasterFields()
{
    DBG_DTOR( rpt_OXMLMasterFields,NULL);
}

// -----------------------------------------------------------------------------
SvXMLImportContext* OXMLMasterFields::CreateChildContext(
        sal_uInt16 _nPrefix,
        const ::rtl::OUString& _rLocalName,
        const Reference< XAttributeList > & xAttrList )
{
    SvXMLImportContext *pContext = 0;
    const SvXMLTokenMap&    rTokenMap   = static_cast<ORptFilter&>(GetImport()).GetSubDocumentElemTokenMap();

    switch( rTokenMap.Get( _nPrefix, _rLocalName ) )
    {
        case XML_TOK_MASTER_DETAIL_FIELD:
            {
                GetImport().GetProgressBarHelper()->Increment( PROGRESS_BAR_STEP );
                pContext = new OXMLMasterFields(static_cast<ORptFilter&>(GetImport()), _nPrefix, _rLocalName,xAttrList ,m_pReport);
            }
            break;
        default:
            break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport(), _nPrefix, _rLocalName );

    return pContext;
}

//----------------------------------------------------------------------------
} // namespace rptxml
// -----------------------------------------------------------------------------
