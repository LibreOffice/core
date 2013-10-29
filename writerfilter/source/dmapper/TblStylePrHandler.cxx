/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <TblStylePrHandler.hxx>
#include <PropertyMap.hxx>
#include <ooxml/resourceids.hxx>
#include <dmapperLoggers.hxx>
#include <resourcemodel/QNameToString.hxx>

#include "dmapperLoggers.hxx"

namespace writerfilter {
namespace dmapper {

TblStylePrHandler::TblStylePrHandler( DomainMapper & rDMapper ) :
LoggedProperties(dmapper_logger, "TblStylePrHandler"),
m_rDMapper( rDMapper ),
m_pTablePropsHandler( new TablePropertiesHandler( true ) ),
m_nType( TBL_STYLE_UNKNOWN ),
m_pProperties( new PropertyMap )
{
}

TblStylePrHandler::~TblStylePrHandler( )
{
    delete m_pTablePropsHandler, m_pTablePropsHandler = NULL;
}

void TblStylePrHandler::lcl_attribute(Id rName, Value & rVal)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("TblStylePrHandler.attribute");
    dmapper_logger->attribute("name", (*QNameToString::Instance())(rName));
    dmapper_logger->chars(rVal.toString());
    dmapper_logger->endElement();
#endif

    switch ( rName )
    {
        case NS_ooxml::LN_CT_TblStyleOverrideType:
            {
                // The tokenid should be the same in the model.xml than
                // in the TblStyleType enum
                m_nType = TblStyleType( rVal.getInt( ) );
            }
            break;
    }
}

void TblStylePrHandler::lcl_sprm(Sprm & rSprm)
{
#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->startElement("TblStylePrHandler.sprm");
    dmapper_logger->attribute("sprm", rSprm.toString());
#endif

    Value::Pointer_t pValue = rSprm.getValue();
    switch ( rSprm.getId( ) )
    {
        case NS_ooxml::LN_CT_PPrBase:
        case NS_ooxml::LN_EG_RPrBase:
        case NS_ooxml::LN_CT_TblPrBase:
        case NS_ooxml::LN_CT_TrPrBase:
        case NS_ooxml::LN_CT_TcPrBase:
        {
            std::vector<beans::PropertyValue> aSavedGrabBag;
            if (rSprm.getId() == NS_ooxml::LN_CT_PPrBase || rSprm.getId() == NS_ooxml::LN_EG_RPrBase)
            {
                aSavedGrabBag = m_aInteropGrabBag;
                m_aInteropGrabBag.clear();
            }
            resolveSprmProps( rSprm );
            if (rSprm.getId() == NS_ooxml::LN_CT_PPrBase || rSprm.getId() == NS_ooxml::LN_EG_RPrBase)
            {
                if (rSprm.getId() == NS_ooxml::LN_CT_PPrBase)
                    aSavedGrabBag.push_back(getInteropGrabBag("pPr"));
                else if (rSprm.getId() == NS_ooxml::LN_EG_RPrBase)
                    aSavedGrabBag.push_back(getInteropGrabBag("rPr"));
                m_aInteropGrabBag = aSavedGrabBag;
            }
        }
            break;
        default:
            // Tables specific properties have to handled here
            m_pTablePropsHandler->SetProperties( m_pProperties );
            m_pTablePropsHandler->SetInteropGrabBag(m_aInteropGrabBag);
            bool bRet = m_pTablePropsHandler->sprm( rSprm );

            if ( !bRet )
            {
                // The DomainMapper can handle some of the properties
                m_rDMapper.PushStyleSheetProperties( m_pProperties, true );
                // Just pass a non-empty string, the array will have a single element anyway.
                m_rDMapper.enableInteropGrabBag("TblStylePrHandler");
                m_rDMapper.sprm( rSprm );
                uno::Sequence<beans::PropertyValue> aGrabBag = m_rDMapper.getInteropGrabBag().Value.get< uno::Sequence<beans::PropertyValue> >();
                if (aGrabBag.hasElements())
                    m_aInteropGrabBag.push_back(aGrabBag[0]);
                m_rDMapper.PopStyleSheetProperties( true );
            }
    }

#ifdef DEBUG_DOMAINMAPPER
    dmapper_logger->endElement();
#endif
}

void TblStylePrHandler::resolveSprmProps(Sprm & rSprm)
{
    writerfilter::Reference<Properties>::Pointer_t pProperties = rSprm.getProps();
    if( pProperties.get())
        pProperties->resolve(*this);
}

beans::PropertyValue TblStylePrHandler::getInteropGrabBag(OUString aName)
{
    beans::PropertyValue aRet;
    aRet.Name = aName;

    uno::Sequence<beans::PropertyValue> aSeq(m_aInteropGrabBag.size());
    beans::PropertyValue* pSeq = aSeq.getArray();
    for (std::vector<beans::PropertyValue>::iterator i = m_aInteropGrabBag.begin(); i != m_aInteropGrabBag.end(); ++i)
        *pSeq++ = *i;

    aRet.Value = uno::makeAny(aSeq);
    return aRet;
}

}}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
