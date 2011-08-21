/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#include "precompiled_rptxml.hxx"
#include "xmlMasterFields.hxx"
#include "xmlfilter.hxx"
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/nmspmap.hxx>
#include "xmlEnums.hxx"
#include "xmlReport.hxx"
#include <tools/debug.hxx>


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
    if ( sMasterField.getLength() )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
