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

#include <sal/config.h>
#include <sal/log.hxx>

#include <accelerators/keymapping.hxx>
#include <xml/acceleratorconfigurationreader.hxx>

#include <acceleratorconst.h>

#include <com/sun/star/xml/sax/XExtendedDocumentHandler.hpp>
#include <com/sun/star/xml/sax/SAXException.hpp>
#include <com/sun/star/awt/KeyModifier.hpp>
#include <com/sun/star/awt/KeyEvent.hpp>
#include <com/sun/star/awt/Key.hpp>
#include <com/sun/star/container/ElementExistException.hpp>

#include <vcl/svapp.hxx>
#include <rtl/ustrbuf.hxx>

namespace framework{

/* Throws a SaxException in case a wrong formatted XML
   structure was detected.

   This macro combined the given comment with a generic
   way to find out the XML line (where the error occurred)
   to format a suitable message.

   @param   COMMENT
            an ascii string, which describe the problem.
 */
#define THROW_PARSEEXCEPTION(COMMENT)                                   \
    {                                                                   \
        throw css::xml::sax::SAXException(                              \
                implts_getErrorLineString() + COMMENT,                  \
                static_cast< css::xml::sax::XDocumentHandler* >(this),  \
                css::uno::Any());                                       \
    }

AcceleratorConfigurationReader::AcceleratorConfigurationReader(AcceleratorCache& rContainer)
    : m_rContainer            (rContainer                   )
    , m_bInsideAcceleratorList(false                    )
    , m_bInsideAcceleratorItem(false                    )
{
}

AcceleratorConfigurationReader::~AcceleratorConfigurationReader()
{
}

void SAL_CALL AcceleratorConfigurationReader::startDocument()
{
}

void SAL_CALL AcceleratorConfigurationReader::endDocument()
{
    // The xml file seems to be corrupted.
    // Because we found no end-tags ... at least for
    // one list or item.
    if (m_bInsideAcceleratorList || m_bInsideAcceleratorItem)
    {
        THROW_PARSEEXCEPTION("No matching start or end element 'acceleratorlist' found!")
    }
}

void SAL_CALL AcceleratorConfigurationReader::startElement(const OUString&                                      sElement      ,
                                                           const css::uno::Reference< css::xml::sax::XAttributeList >& xAttributeList)
{
    EXMLElement eElement = AcceleratorConfigurationReader::implst_classifyElement(sElement);

    // Note: We handle "accel:item" before "accel:acceleratorlist" to perform this operation.
    // Because an item occurs very often ... a list should occur one times only!
    if (eElement == E_ELEMENT_ITEM)
    {
        if (!m_bInsideAcceleratorList)
            THROW_PARSEEXCEPTION("An element \"accel:item\" must be embedded into 'accel:acceleratorlist'.")
        if (m_bInsideAcceleratorItem)
            THROW_PARSEEXCEPTION("An element \"accel:item\" is not a container.")
        m_bInsideAcceleratorItem = true;

        OUString    sCommand;
        css::awt::KeyEvent aEvent;

        sal_Int16 c = xAttributeList->getLength();
        sal_Int16 i = 0;
        for (i=0; i<c; ++i)
        {
            OUString sAttribute = xAttributeList->getNameByIndex(i);
            OUString sValue     = xAttributeList->getValueByIndex(i);
            EXMLAttribute   eAttribute = AcceleratorConfigurationReader::implst_classifyAttribute(sAttribute);
            switch(eAttribute)
            {
                case E_ATTRIBUTE_URL :
                    sCommand = sValue.intern();
                    break;

                case E_ATTRIBUTE_KEYCODE :
                    aEvent.KeyCode = KeyMapping::get().mapIdentifierToCode(sValue);
                    break;

                case E_ATTRIBUTE_MOD_SHIFT :
                    aEvent.Modifiers |= css::awt::KeyModifier::SHIFT;
                    break;

                case E_ATTRIBUTE_MOD_MOD1  :
                    aEvent.Modifiers |= css::awt::KeyModifier::MOD1;
                    break;

                case E_ATTRIBUTE_MOD_MOD2  :
                    aEvent.Modifiers |= css::awt::KeyModifier::MOD2;
                    break;

                case E_ATTRIBUTE_MOD_MOD3  :
                    aEvent.Modifiers |= css::awt::KeyModifier::MOD3;
            }
        }

        // validate command and key event.
        if (
            sCommand.isEmpty() ||
            (aEvent.KeyCode == 0  )
           )
        {
            THROW_PARSEEXCEPTION("XML element does not describe a valid accelerator nor a valid command.")
        }

        // register key event + command inside cache ...
        // Check for already existing items there.
        if (!m_rContainer.hasKey(aEvent))
            m_rContainer.setKeyCommandPair(aEvent, sCommand);
        else
        {
            // Attention: It's not really a reason to throw an exception and kill the office, if the configuration contains
            // multiple registrations for the same key :-) Show a warning ... and ignore the second item.
            // THROW_PARSEEXCEPTION("Command is registered for the same key more than once.")
            SAL_INFO("fwk",
                     "AcceleratorConfigurationReader::startElement(): Double registration detected. Command=\"" <<
                     sCommand <<
                     "\" KeyCode=" <<
                     aEvent.KeyCode <<
                     "Modifiers=" <<
                     aEvent.Modifiers);
        }
    }

    if (eElement == E_ELEMENT_ACCELERATORLIST)
    {
        if (m_bInsideAcceleratorList)
            THROW_PARSEEXCEPTION("An element \"accel:acceleratorlist\" cannot be used recursive.")
        m_bInsideAcceleratorList = true;
        return;
    }
}

void SAL_CALL AcceleratorConfigurationReader::endElement(const OUString& sElement)
{
    EXMLElement eElement = AcceleratorConfigurationReader::implst_classifyElement(sElement);

    // Note: We handle "accel:item" before "accel:acceleratorlist" to perform this operation.
    // Because an item occurs very often ... a list should occur one times only!
    if (eElement == E_ELEMENT_ITEM)
    {
        if (!m_bInsideAcceleratorItem)
            THROW_PARSEEXCEPTION("Found end element 'accel:item', but no start element.")
        m_bInsideAcceleratorItem = false;
    }

    if (eElement == E_ELEMENT_ACCELERATORLIST)
    {
        if (!m_bInsideAcceleratorList)
            THROW_PARSEEXCEPTION("Found end element 'accel:acceleratorlist', but no start element.")
        m_bInsideAcceleratorList = false;
    }
}

void SAL_CALL AcceleratorConfigurationReader::characters(const OUString&)
{
}

void SAL_CALL AcceleratorConfigurationReader::ignorableWhitespace(const OUString&)
{
}

void SAL_CALL AcceleratorConfigurationReader::processingInstruction(const OUString& /*sTarget*/,
                                                                    const OUString& /*sData*/  )
{
}

void SAL_CALL AcceleratorConfigurationReader::setDocumentLocator(const css::uno::Reference< css::xml::sax::XLocator >& xLocator)
{
    m_xLocator = xLocator;
}

AcceleratorConfigurationReader::EXMLElement AcceleratorConfigurationReader::implst_classifyElement(const OUString& sElement)
{
    AcceleratorConfigurationReader::EXMLElement eElement;

    if (sElement == "http://openoffice.org/2001/accel^acceleratorlist")
        eElement = E_ELEMENT_ACCELERATORLIST;
    else if (sElement == "http://openoffice.org/2001/accel^item")
        eElement = E_ELEMENT_ITEM;
    else
        throw css::uno::RuntimeException(
                "Unknown XML element detected!",
                css::uno::Reference< css::xml::sax::XDocumentHandler >());

    return eElement;
}

AcceleratorConfigurationReader::EXMLAttribute AcceleratorConfigurationReader::implst_classifyAttribute(const OUString& sAttribute)
{
    AcceleratorConfigurationReader::EXMLAttribute eAttribute;

    if (sAttribute == "http://openoffice.org/2001/accel^code")
        eAttribute = E_ATTRIBUTE_KEYCODE;
    else if (sAttribute == "http://openoffice.org/2001/accel^shift")
        eAttribute = E_ATTRIBUTE_MOD_SHIFT;
    else if (sAttribute == "http://openoffice.org/2001/accel^mod1")
        eAttribute = E_ATTRIBUTE_MOD_MOD1;
    else if (sAttribute == "http://openoffice.org/2001/accel^mod2")
        eAttribute = E_ATTRIBUTE_MOD_MOD2;
    else if (sAttribute == "http://openoffice.org/2001/accel^mod3")
        eAttribute = E_ATTRIBUTE_MOD_MOD3;
    else if (sAttribute == "http://www.w3.org/1999/xlink^href")
        eAttribute = E_ATTRIBUTE_URL;
    else
        throw css::uno::RuntimeException(
                "Unknown XML attribute detected!",
                css::uno::Reference< css::xml::sax::XDocumentHandler >());

    return eAttribute;
}

OUString AcceleratorConfigurationReader::implts_getErrorLineString()
{
    if (!m_xLocator.is())
        return OUString("Error during parsing XML. (No further info available ...)");

    OUStringBuffer sMsg(256);
    sMsg.append("Error during parsing XML in\nline = ");
    sMsg.append     (m_xLocator->getLineNumber()           );
    sMsg.append("\ncolumn = "                         );
    sMsg.append     (m_xLocator->getColumnNumber()         );
    sMsg.append("."                                   );
    return sMsg.makeStringAndClear();
}

} // namespace framework

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
