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

#ifndef INCLUDED_FRAMEWORK_INC_XML_IMAGESDOCUMENTHANDLER_HXX
#define INCLUDED_FRAMEWORK_INC_XML_IMAGESDOCUMENTHANDLER_HXX

#include <framework/fwedllapi.h>

#include <com/sun/star/xml/sax/XDocumentHandler.hpp>

#include <xml/saxnamespacefilter.hxx> // HACK: needed for MSVC 2013 ENABLE_LTO build: WeakImplHelper<XDocumentHandler>
#include <xml/imagesconfiguration.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase.hxx>

#include <unordered_map>
#include <stdtypes.h>

namespace framework{

// Hash code function for using in all hash maps of follow implementation.

class OReadImagesDocumentHandler : public ::cppu::WeakImplHelper< css::xml::sax::XDocumentHandler >
{
    public:
        enum Image_XML_Entry
        {
            IMG_ELEMENT_IMAGECONTAINER,
            IMG_ELEMENT_IMAGES,
            IMG_ELEMENT_ENTRY,
            IMG_ELEMENT_EXTERNALIMAGES,
            IMG_ELEMENT_EXTERNALENTRY,
            IMG_ATTRIBUTE_HREF,
            IMG_ATTRIBUTE_MASKCOLOR,
            IMG_ATTRIBUTE_COMMAND,
            IMG_ATTRIBUTE_BITMAPINDEX,
            IMG_ATTRIBUTE_MASKURL,
            IMG_ATTRIBUTE_MASKMODE,
            IMG_ATTRIBUTE_HIGHCONTRASTURL,
            IMG_ATTRIBUTE_HIGHCONTRASTMASKURL,
            IMG_XML_ENTRY_COUNT
        };

        enum Image_XML_Namespace
        {
            IMG_NS_IMAGE,
            IMG_NS_XLINK
        };

        OReadImagesDocumentHandler( ImageListsDescriptor& aItems );
        virtual ~OReadImagesDocumentHandler() override;

        // XDocumentHandler
        virtual void SAL_CALL startDocument() override;

        virtual void SAL_CALL endDocument() override;

        virtual void SAL_CALL startElement(
            const OUString& aName,
            const css::uno::Reference< css::xml::sax::XAttributeList > &xAttribs) override;

        virtual void SAL_CALL endElement(const OUString& aName) override;

        virtual void SAL_CALL characters(const OUString& aChars) override;

        virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces) override;

        virtual void SAL_CALL processingInstruction(const OUString& aTarget,
                                                    const OUString& aData) override;

        virtual void SAL_CALL setDocumentLocator(
            const css::uno::Reference< css::xml::sax::XLocator > &xLocator) override;

    private:
        OUString getErrorLineString();

        class ImageHashMap : public std::unordered_map< OUString     ,
                                                        Image_XML_Entry        ,
                                                        OUStringHash >
        {
        };

        bool                                                m_bImageContainerStartFound;
        bool                                                m_bImageContainerEndFound;
        bool                                                m_bImagesStartFound;
        bool                                                m_bImageStartFound;
        bool                                                m_bExternalImagesStartFound;
        bool                                                m_bExternalImageStartFound;
        sal_Int32                                           m_nHashMaskModeBitmap;
        sal_Int32                                           m_nHashMaskModeColor;
        ImageHashMap                                        m_aImageMap;
        ImageListsDescriptor&                               m_aImageList;
        ImageListItemDescriptor*                            m_pImages;
        ExternalImageItemListDescriptor*                    m_pExternalImages;
        css::uno::Reference< css::xml::sax::XLocator >      m_xLocator;
};

class OWriteImagesDocumentHandler final
{
    public:
        OWriteImagesDocumentHandler(
            const ImageListsDescriptor& aItems,
            css::uno::Reference< css::xml::sax::XDocumentHandler > const &
                rWriteDocumentHandler);
        ~OWriteImagesDocumentHandler();

        /// @throws css::xml::sax::SAXException
        /// @throws css::uno::RuntimeException
        void WriteImagesDocument();

    private:
        /// @throws css::xml::sax::SAXException
        /// @throws css::uno::RuntimeException
        void WriteImageList( const ImageListItemDescriptor* );

        /// @throws css::xml::sax::SAXException
        /// @throws css::uno::RuntimeException
        void WriteExternalImageList( const ExternalImageItemListDescriptor* );

        /// @throws css::xml::sax::SAXException
        /// @throws css::uno::RuntimeException
        void WriteImage( const ImageItemDescriptor* );

        /// @throws css::xml::sax::SAXException
        /// @throws css::uno::RuntimeException
        void WriteExternalImage( const ExternalImageItemDescriptor* );

        const ImageListsDescriptor&                               m_aImageListsItems;
        css::uno::Reference< css::xml::sax::XDocumentHandler >    m_xWriteDocumentHandler;
        css::uno::Reference< css::xml::sax::XAttributeList >      m_xEmptyList;
        OUString                                                  m_aXMLXlinkNS;
        OUString                                                  m_aXMLImageNS;
        OUString                                                  m_aAttributeType;
        OUString                                                  m_aAttributeXlinkType;
        OUString                                                  m_aAttributeValueSimple;
};

} // namespace framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
