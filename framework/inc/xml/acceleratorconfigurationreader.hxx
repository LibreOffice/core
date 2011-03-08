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

#ifndef _FRAMEWORK_XML_ACCELERATORCONFIGURATIONREADER_HXX_
#define _FRAMEWORK_XML_ACCELERATORCONFIGURATIONREADER_HXX_

//_______________________________________________
// own includes

#include <accelerators/acceleratorcache.hxx>
#include <accelerators/keymapping.hxx>
#include <macros/xinterface.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <general.h>

//_______________________________________________
// interface includes

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <com/sun/star/xml/sax/XLocator.hpp>

//_______________________________________________
// other includes
#include <salhelper/singletonref.hxx>
#include <cppuhelper/weak.hxx>
#include <rtl/ustring.hxx>

namespace framework{

class AcceleratorConfigurationReader : public  css::xml::sax::XDocumentHandler
                                     , private ThreadHelpBase
                                     , public  ::cppu::OWeakObject
{
    //-------------------------------------------
    // const, types

    private:

        //---------------------------------------
        /** @short  classification of XML elements. */
        enum EXMLElement
        {
            E_ELEMENT_ACCELERATORLIST,
            E_ELEMENT_ITEM
        };

        //---------------------------------------
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

        //---------------------------------------
        /** @short  some namespace defines */
        enum EAcceleratorXMLNamespace
        {
            E_NAMESPACE_ACCEL,
            E_NAMESPACE_XLINK
        };

    //-------------------------------------------
    // member

    private:

        //---------------------------------------
        /** @short  needed to read the xml configuration. */
        css::uno::Reference< css::xml::sax::XDocumentHandler > m_xReader;

        //---------------------------------------
        /** @short  reference to the outside container, where this
                    reader/writer must work on. */
        AcceleratorCache& m_rContainer;

        //---------------------------------------
        /** @short  used to detect if an accelerator list
                    occures recursive inside xml. */
        sal_Bool m_bInsideAcceleratorList;

        //---------------------------------------
        /** @short  used to detect if an accelerator item
                    occures recursive inside xml. */
        sal_Bool m_bInsideAcceleratorItem;

        //---------------------------------------
        /** @short  is used to map key codes to its
                    string representation.

            @descr  To perform this operatio is
                    created only one times and holded
                    alive forever ...*/
        ::salhelper::SingletonRef< KeyMapping > m_rKeyMapping;

        //---------------------------------------
        /** @short  provide informations abou the parsing state.

            @descr  We use it to find out the line and column, where
                    an error occure.
          */
        css::uno::Reference< css::xml::sax::XLocator > m_xLocator;

/*        SfxAcceleratorItemList& m_aReadAcceleratorList;
*/

    //-------------------------------------------
    // interface

    public:

        //---------------------------------------
        /** @short  connect this new reader/writer instance
                    to an outside container, which should be used
                    flushed to the underlying XML configuration or
                    filled from there.

            @param  rContainer
                    a reference to the outside container.
          */
        AcceleratorConfigurationReader(AcceleratorCache& rContainer);

        //---------------------------------------
        /** @short  does nothing real ... */
        virtual ~AcceleratorConfigurationReader();

        //---------------------------------------
        // XInterface
        FWK_DECLARE_XINTERFACE

        //---------------------------------------
        // XDocumentHandler
        virtual void SAL_CALL startDocument()
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );

        virtual void SAL_CALL endDocument()
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );

        virtual void SAL_CALL startElement(const ::rtl::OUString&                                      sElement      ,
                                           const css::uno::Reference< css::xml::sax::XAttributeList >& xAttributeList)
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );

        virtual void SAL_CALL endElement(const ::rtl::OUString& sElement)
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );

        virtual void SAL_CALL characters(const ::rtl::OUString& sChars)
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );

        virtual void SAL_CALL ignorableWhitespace(const ::rtl::OUString& sWhitespaces)
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );

        virtual void SAL_CALL processingInstruction(const ::rtl::OUString& sTarget,
                                                    const ::rtl::OUString& sData  )
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );

        virtual void SAL_CALL setDocumentLocator(const css::uno::Reference< css::xml::sax::XLocator >& xLocator)
            throw(css::xml::sax::SAXException,
                  css::uno::RuntimeException );

    //-------------------------------------------
    // helper

    private:

        //---------------------------------------
        /** TODO document me */
        static EXMLElement implst_classifyElement(const ::rtl::OUString& sElement);

        //---------------------------------------
        /** TODO document me */
        static EXMLAttribute implst_classifyAttribute(const ::rtl::OUString& sAttribute);

        //---------------------------------------
        /** TODO document me */
        ::rtl::OUString implts_getErrorLineString();
};

} // namespace framework

#endif // _FRAMEWORK_XML_ACCELERATORCONFIGURATIONREADER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
