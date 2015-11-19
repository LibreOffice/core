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

#ifndef INCLUDED_FRAMEWORK_INC_XML_ACCELERATORCONFIGURATIONREADER_HXX
#define INCLUDED_FRAMEWORK_INC_XML_ACCELERATORCONFIGURATIONREADER_HXX

#include <accelerators/acceleratorcache.hxx>
#include <accelerators/keymapping.hxx>
#include <macros/xinterface.hxx>
#include <general.h>

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <com/sun/star/xml/sax/XLocator.hpp>

#include <salhelper/singletonref.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>

namespace framework{

class AcceleratorConfigurationReader : public ::cppu::WeakImplHelper< css::xml::sax::XDocumentHandler >
{

    // const, types

    private:

        /** @short  classification of XML elements. */
        enum EXMLElement
        {
            E_ELEMENT_ACCELERATORLIST,
            E_ELEMENT_ITEM
        };

        /** @short  classification of XML attributes. */
        enum EXMLAttribute
        {
            E_ATTRIBUTE_KEYCODE,
            E_ATTRIBUTE_MOD_SHIFT,
            E_ATTRIBUTE_MOD_MOD1,
            E_ATTRIBUTE_MOD_MOD2,
                        E_ATTRIBUTE_MOD_MOD3,
            E_ATTRIBUTE_URL
        };

        /** @short  some namespace defines */
        enum EAcceleratorXMLNamespace
        {
            E_NAMESPACE_ACCEL,
            E_NAMESPACE_XLINK
        };

    // member

    private:

        /** @short  reference to the outside container, where this
                    reader/writer must work on. */
        AcceleratorCache& m_rContainer;

        /** @short  used to detect if an accelerator list
                    occurs recursive inside xml. */
        bool m_bInsideAcceleratorList;

        /** @short  used to detect if an accelerator item
                    occurs recursive inside xml. */
        bool m_bInsideAcceleratorItem;

        /** @short  is used to map key codes to its
                    string representation.

            @descr  To perform this operation is
                    created only one time and kept
                    alive forever ...*/
        ::salhelper::SingletonRef< KeyMapping > m_rKeyMapping;

        /** @short  provide information about the parsing state.

            @descr  We use it to find out the line and column, where
                    an error occurred.
          */
        css::uno::Reference< css::xml::sax::XLocator > m_xLocator;

    // interface

    public:

        /** @short  connect this new reader/writer instance
                    to an outside container, which should be used
                    flushed to the underlying XML configuration or
                    filled from there.

            @param  rContainer
                    a reference to the outside container.
          */
        AcceleratorConfigurationReader(AcceleratorCache& rContainer);

        /** @short  does nothing real ... */
        virtual ~AcceleratorConfigurationReader();

        // XDocumentHandler
        virtual void SAL_CALL startDocument()
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL endDocument()
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL startElement(const OUString&                                      sElement      ,
                                           const css::uno::Reference< css::xml::sax::XAttributeList >& xAttributeList)
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL endElement(const OUString& sElement)
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL characters(const OUString& sChars)
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL ignorableWhitespace(const OUString& sWhitespaces)
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL processingInstruction(const OUString& sTarget,
                                                    const OUString& sData  )
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException, std::exception ) override;

        virtual void SAL_CALL setDocumentLocator(const css::uno::Reference< css::xml::sax::XLocator >& xLocator)
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException, std::exception ) override;

    // helper

    private:

        /** TODO document me */
        static EXMLElement implst_classifyElement(const OUString& sElement);

        /** TODO document me */
        static EXMLAttribute implst_classifyAttribute(const OUString& sAttribute);

        /** TODO document me */
        OUString implts_getErrorLineString();
};

} // namespace framework

#endif // INCLUDED_FRAMEWORK_INC_XML_ACCELERATORCONFIGURATIONREADER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
