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

#include <xml/acceleratorconfigurationwriter.hxx>

#include <acceleratorconst.h>

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>

#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>

#include <comphelper/attributelist.hxx>

namespace framework{

AcceleratorConfigurationWriter::AcceleratorConfigurationWriter(const AcceleratorCache&                                       rContainer,
                                                               const css::uno::Reference< css::xml::sax::XDocumentHandler >& xConfig   )
    : m_xConfig     (xConfig                      )
    , m_rContainer  (rContainer                   )
{
}

AcceleratorConfigurationWriter::~AcceleratorConfigurationWriter()
{
}

void AcceleratorConfigurationWriter::flush()
{
    css::uno::Reference< css::xml::sax::XExtendedDocumentHandler > xExtendedCFG(m_xConfig, css::uno::UNO_QUERY_THROW);

    // prepare attribute list
    ::comphelper::AttributeList* pAttribs = new ::comphelper::AttributeList;
    css::uno::Reference< css::xml::sax::XAttributeList > xAttribs(static_cast< css::xml::sax::XAttributeList* >(pAttribs), css::uno::UNO_QUERY);

    pAttribs->AddAttribute(AL_XMLNS_ACCEL, ATTRIBUTE_TYPE_CDATA, NS_XMLNS_ACCEL);
    pAttribs->AddAttribute(AL_XMLNS_XLINK, ATTRIBUTE_TYPE_CDATA, NS_XMLNS_XLINK);

    // generate xml
    xExtendedCFG->startDocument();

    xExtendedCFG->unknown(DOCTYPE_ACCELERATORS);
    xExtendedCFG->ignorableWhitespace(OUString());

    xExtendedCFG->startElement(AL_ELEMENT_ACCELERATORLIST, xAttribs);
    xExtendedCFG->ignorableWhitespace(OUString());

    // TODO think about threadsafe using of cache
    AcceleratorCache::TKeyList                 lKeys = m_rContainer.getAllKeys();
    AcceleratorCache::TKeyList::const_iterator pKey;
    for (  pKey  = lKeys.begin();
           pKey != lKeys.end();
         ++pKey                 )
    {
        const css::awt::KeyEvent& rKey     = *pKey;
        const OUString&    rCommand = m_rContainer.getCommandByKey(rKey);
        impl_ts_writeKeyCommandPair(rKey, rCommand, xExtendedCFG);
    }

    /* TODO write key-command list
    std::vector< SfxAcceleratorConfigItem>::const_iterator p;
    for ( p = m_aWriteAcceleratorList.begin(); p != m_aWriteAcceleratorList.end(); p++ )
        WriteAcceleratorItem( *p );
    */

    xExtendedCFG->ignorableWhitespace(OUString());
    xExtendedCFG->endElement(AL_ELEMENT_ACCELERATORLIST);
    xExtendedCFG->ignorableWhitespace(OUString());
    xExtendedCFG->endDocument();
}

void AcceleratorConfigurationWriter::impl_ts_writeKeyCommandPair(const css::awt::KeyEvent&                                     aKey    ,
                                                                 const OUString&                                        sCommand,
                                                                 const css::uno::Reference< css::xml::sax::XDocumentHandler >& xConfig )
{
    ::comphelper::AttributeList* pAttribs = new ::comphelper::AttributeList;
    css::uno::Reference< css::xml::sax::XAttributeList > xAttribs (static_cast< css::xml::sax::XAttributeList* >(pAttribs) , css::uno::UNO_QUERY_THROW);

    OUString sKey = m_rKeyMapping->mapCodeToIdentifier(aKey.KeyCode);
    // TODO check if key is empty!

    pAttribs->AddAttribute(AL_ATTRIBUTE_KEYCODE, ATTRIBUTE_TYPE_CDATA, sKey    );
    pAttribs->AddAttribute(AL_ATTRIBUTE_URL    , ATTRIBUTE_TYPE_CDATA, sCommand);

    if ((aKey.Modifiers & css::awt::KeyModifier::SHIFT) == css::awt::KeyModifier::SHIFT)
        pAttribs->AddAttribute(AL_ATTRIBUTE_MOD_SHIFT, ATTRIBUTE_TYPE_CDATA, "true");

    if ((aKey.Modifiers & css::awt::KeyModifier::MOD1) == css::awt::KeyModifier::MOD1)
        pAttribs->AddAttribute(AL_ATTRIBUTE_MOD_MOD1, ATTRIBUTE_TYPE_CDATA, "true");

    if ((aKey.Modifiers & css::awt::KeyModifier::MOD2) == css::awt::KeyModifier::MOD2)
        pAttribs->AddAttribute(AL_ATTRIBUTE_MOD_MOD2, ATTRIBUTE_TYPE_CDATA, "true");

    if ((aKey.Modifiers & css::awt::KeyModifier::MOD3) == css::awt::KeyModifier::MOD3)
        pAttribs->AddAttribute(AL_ATTRIBUTE_MOD_MOD3, ATTRIBUTE_TYPE_CDATA, "true");

    xConfig->ignorableWhitespace(OUString());
    xConfig->startElement(AL_ELEMENT_ITEM, xAttribs);
    xConfig->ignorableWhitespace(OUString());
    xConfig->endElement(AL_ELEMENT_ITEM);
    xConfig->ignorableWhitespace(OUString());
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
