/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include "xmlmappingi.hxx"
#include "xmltransformationi.hxx"

#include <xmloff/xmltkmap.hxx>
#include <xmloff/nmspmap.hxx>
#include <xmloff/xmltoken.hxx>
#include <xmloff/xmlnmspe.hxx>
#include <xmloff/xmlerror.hxx>

#include <datamapper.hxx>
#include <document.hxx>
#include <dbdata.hxx>

#include <sax/tools/converter.hxx>

using namespace com::sun::star;
using namespace xmloff::token;

ScXMLMappingsContext::ScXMLMappingsContext( ScXMLImport& rImport ) :
    ScXMLImportContext( rImport )
{
    // has no attributes
    rImport.LockSolarMutex();
}

ScXMLMappingsContext::~ScXMLMappingsContext()
{
    GetScImport().UnlockSolarMutex();
}

uno::Reference< xml::sax::XFastContextHandler > SAL_CALL ScXMLMappingsContext::createFastChildContext(
                                      sal_Int32 nElement,
                                      const uno::Reference< xml::sax::XFastAttributeList >& xAttrList )
{
    SvXMLImportContext *pContext = nullptr;
    sax_fastparser::FastAttributeList *pAttribList =
        sax_fastparser::FastAttributeList::castToFastAttributeList( xAttrList );

    switch( nElement )
    {
        case XML_ELEMENT( CALC_EXT, XML_DATA_MAPPING ):
        {
            pContext = new ScXMLMappingContext( GetScImport(), pAttribList );
        }
        break;
        case XML_ELEMENT( CALC_EXT, XML_DATA_TRANSFORMATIONS):
        {
             pContext = new ScXMLTransformationsContext( GetScImport() );
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}

ScXMLMappingContext::ScXMLMappingContext( ScXMLImport& rImport,
                                      const rtl::Reference<sax_fastparser::FastAttributeList>& rAttrList ) :
    ScXMLImportContext( rImport )
{
    OUString aProvider;
    OUString aID;
    OUString aURL;
    // OUString aFrequency;
    OUString aDBName;
    if ( rAttrList.is() )
    {
        for (auto &aIter : *rAttrList)
        {
            switch (aIter.getToken())
            {
                case XML_ELEMENT( XLINK, XML_HREF ):
                {
                    aURL = aIter.toString();
                }
                break;
                case XML_ELEMENT( CALC_EXT, XML_PROVIDER ):
                {
                    aProvider = aIter.toString();
                }
                break;
                case XML_ELEMENT( CALC_EXT, XML_ID ):
                {
                    aID = aIter.toString();
                }
                break;
                case XML_ELEMENT( CALC_EXT, XML_DATABASE_NAME ):
                {
                    aDBName = aIter.toString();
                }
                break;
                case XML_ELEMENT( CALC_EXT, XML_DATA_FREQUENCY ):
                {
                }
                break;
            }
        }
    }

    if (!aProvider.isEmpty())
    {
        ScDocument* pDoc = GetScImport().GetDocument();
        auto& rDataMapper = pDoc->GetExternalDataMapper();
        sc::ExternalDataSource aSource(aURL, aProvider, pDoc);
        aSource.setID(aID);
        aSource.setDBData(aDBName);
        rDataMapper.insertDataSource(aSource);
    }
}

ScXMLMappingContext::~ScXMLMappingContext()
{
    ScDocument* pDoc = GetScImport().GetDocument();
    auto& rDataMapper = pDoc->GetExternalDataMapper();
    auto& rDataSources = rDataMapper.getDataSources();
    if(!rDataSources.empty())
        rDataSources[0].refresh(pDoc, true);
}

uno::Reference<xml::sax::XFastContextHandler>
    SAL_CALL ScXMLMappingContext::createFastChildContext(
        sal_Int32 nElement, const uno::Reference<xml::sax::XFastAttributeList>& /*xAttrList*/)
{
    SvXMLImportContext *pContext = nullptr;

    switch( nElement )
    {
        case XML_ELEMENT( CALC_EXT, XML_DATA_TRANSFORMATIONS):
        {
             pContext = new ScXMLTransformationsContext( GetScImport() );
        }
        break;
    }

    if( !pContext )
        pContext = new SvXMLImportContext( GetImport() );

    return pContext;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
