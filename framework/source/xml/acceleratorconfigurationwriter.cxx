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

#include <xml/acceleratorconfigurationwriter.hxx>

#include <acceleratorconst.h>
#include <threadhelp/readguard.hxx>

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>

#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>

#include <comphelper/attributelist.hxx>


//_______________________________________________
// namespace

namespace framework{


//-----------------------------------------------
AcceleratorConfigurationWriter::AcceleratorConfigurationWriter(const AcceleratorCache&                                       rContainer,
                                                               const css::uno::Reference< css::xml::sax::XDocumentHandler >& xConfig   )
    : ThreadHelpBase(&Application::GetSolarMutex())
    , m_xConfig     (xConfig                      )
    , m_rContainer  (rContainer                   )
{
}

//-----------------------------------------------
AcceleratorConfigurationWriter::~AcceleratorConfigurationWriter()
{
}

//-----------------------------------------------
void AcceleratorConfigurationWriter::flush()
{
    // SAFE -> ----------------------------------
    ReadGuard aReadLock(m_aLock);

    css::uno::Reference< css::xml::sax::XDocumentHandler >         xCFG        = m_xConfig;
    css::uno::Reference< css::xml::sax::XExtendedDocumentHandler > xExtendedCFG(m_xConfig, css::uno::UNO_QUERY_THROW);

    aReadLock.unlock();
    // <- SAFE ----------------------------------

    // prepare attribute list
    ::comphelper::AttributeList* pAttribs = new ::comphelper::AttributeList;
    css::uno::Reference< css::xml::sax::XAttributeList > xAttribs(static_cast< css::xml::sax::XAttributeList* >(pAttribs), css::uno::UNO_QUERY);

    pAttribs->AddAttribute(AL_XMLNS_ACCEL, ATTRIBUTE_TYPE_CDATA, NS_XMLNS_ACCEL);
    pAttribs->AddAttribute(AL_XMLNS_XLINK, ATTRIBUTE_TYPE_CDATA, NS_XMLNS_XLINK);

    // generate xml
    xCFG->startDocument();

    xExtendedCFG->unknown(DOCTYPE_ACCELERATORS);
    xCFG->ignorableWhitespace(::rtl::OUString());

    xCFG->startElement(AL_ELEMENT_ACCELERATORLIST, xAttribs);
    xCFG->ignorableWhitespace(::rtl::OUString());

    // TODO think about threadsafe using of cache
    AcceleratorCache::TKeyList                 lKeys = m_rContainer.getAllKeys();
    AcceleratorCache::TKeyList::const_iterator pKey  ;
    for (  pKey  = lKeys.begin();
           pKey != lKeys.end()  ;
         ++pKey                 )
    {
        const css::awt::KeyEvent& rKey     = *pKey;
        const ::rtl::OUString&    rCommand = m_rContainer.getCommandByKey(rKey);
        impl_ts_writeKeyCommandPair(rKey, rCommand, xCFG);
    }

    /* TODO write key-command list
    std::vector< SfxAcceleratorConfigItem>::const_iterator p;
    for ( p = m_aWriteAcceleratorList.begin(); p != m_aWriteAcceleratorList.end(); p++ )
        WriteAcceleratorItem( *p );
    */

    xCFG->ignorableWhitespace(::rtl::OUString());
    xCFG->endElement(AL_ELEMENT_ACCELERATORLIST);
    xCFG->ignorableWhitespace(::rtl::OUString());
    xCFG->endDocument();
}

//-----------------------------------------------
void AcceleratorConfigurationWriter::impl_ts_writeKeyCommandPair(const css::awt::KeyEvent&                                     aKey    ,
                                                                 const ::rtl::OUString&                                        sCommand,
                                                                 const css::uno::Reference< css::xml::sax::XDocumentHandler >& xConfig )
{
    ::comphelper::AttributeList* pAttribs = new ::comphelper::AttributeList;
    css::uno::Reference< css::xml::sax::XAttributeList > xAttribs (static_cast< css::xml::sax::XAttributeList* >(pAttribs) , css::uno::UNO_QUERY_THROW);

    ::rtl::OUString sKey = m_rKeyMapping->mapCodeToIdentifier(aKey.KeyCode);
    // TODO check if key is empty!

    pAttribs->AddAttribute(AL_ATTRIBUTE_KEYCODE, ATTRIBUTE_TYPE_CDATA, sKey    );
    pAttribs->AddAttribute(AL_ATTRIBUTE_URL    , ATTRIBUTE_TYPE_CDATA, sCommand);

    if ((aKey.Modifiers & css::awt::KeyModifier::SHIFT) == css::awt::KeyModifier::SHIFT)
        pAttribs->AddAttribute(AL_ATTRIBUTE_MOD_SHIFT, ATTRIBUTE_TYPE_CDATA, ::rtl::OUString("true"));

    if ((aKey.Modifiers & css::awt::KeyModifier::MOD1) == css::awt::KeyModifier::MOD1)
        pAttribs->AddAttribute(AL_ATTRIBUTE_MOD_MOD1, ATTRIBUTE_TYPE_CDATA, ::rtl::OUString("true"));

    if ((aKey.Modifiers & css::awt::KeyModifier::MOD2) == css::awt::KeyModifier::MOD2)
        pAttribs->AddAttribute(AL_ATTRIBUTE_MOD_MOD2, ATTRIBUTE_TYPE_CDATA, ::rtl::OUString("true"));

    if ((aKey.Modifiers & css::awt::KeyModifier::MOD3) == css::awt::KeyModifier::MOD3)
        pAttribs->AddAttribute(AL_ATTRIBUTE_MOD_MOD3, ATTRIBUTE_TYPE_CDATA, ::rtl::OUString("true"));

    xConfig->ignorableWhitespace(::rtl::OUString());
    xConfig->startElement(AL_ELEMENT_ITEM, xAttribs);
    xConfig->ignorableWhitespace(::rtl::OUString());
    xConfig->endElement(AL_ELEMENT_ITEM);
    xConfig->ignorableWhitespace(::rtl::OUString());
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
