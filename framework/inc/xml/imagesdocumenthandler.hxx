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

#include <xml/imagesconfiguration.hxx>
#include <threadhelp/threadhelpbase.hxx>
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase1.hxx>

#include <boost/unordered_map.hpp>
#include <stdtypes.h>

namespace framework{

//*****************************************************************************************************************
// Hash code function for using in all hash maps of follow implementation.

class OReadImagesDocumentHandler : private ThreadHelpBase,  // Struct for right initalization of lock member! Must be first of baseclasses.
                                   public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XDocumentHandler >
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
            IMG_NS_XLINK,
            TBL_XML_NAMESPACES_COUNT
        };

        OReadImagesDocumentHandler( ImageListsDescriptor& aItems );
        virtual ~OReadImagesDocumentHandler();

        // XDocumentHandler
        virtual void SAL_CALL startDocument(void)
        throw ( ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException, std::exception );

        virtual void SAL_CALL endDocument(void)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException, std::exception );

        virtual void SAL_CALL startElement(
            const OUString& aName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > &xAttribs)
        throw (::com::sun::star::xml::sax::SAXException,
               ::com::sun::star::uno::RuntimeException,
               std::exception);

        virtual void SAL_CALL endElement(const OUString& aName)
        throw (::com::sun::star::xml::sax::SAXException,
               ::com::sun::star::uno::RuntimeException,
               std::exception);

        virtual void SAL_CALL characters(const OUString& aChars)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException, std::exception );

        virtual void SAL_CALL ignorableWhitespace(const OUString& aWhitespaces)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException, std::exception );

        virtual void SAL_CALL processingInstruction(const OUString& aTarget,
                                                    const OUString& aData)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException, std::exception );

        virtual void SAL_CALL setDocumentLocator(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > &xLocator)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException, std::exception );

    private:
        OUString getErrorLineString();

        class ImageHashMap : public ::boost::unordered_map< OUString     ,
                                                     Image_XML_Entry        ,
                                                     OUStringHash,
                                                     ::std::equal_to< OUString > >
        {
            public:
                inline void free()
                {
                    ImageHashMap().swap( *this );
                }
        };

        sal_Bool                                                                    m_bImageContainerStartFound;
        sal_Bool                                                                    m_bImageContainerEndFound;
        sal_Bool                                                                    m_bImagesStartFound;
        sal_Bool                                                                    m_bImagesEndFound;
        sal_Bool                                                                    m_bImageStartFound;
        sal_Bool                                                                    m_bExternalImagesStartFound;
        sal_Bool                                                                    m_bExternalImagesEndFound;
        sal_Bool                                                                    m_bExternalImageStartFound;
        sal_Int32                                                                   m_nHashMaskModeBitmap;
        sal_Int32                                                                   m_nHashMaskModeColor;
        ImageHashMap                                                                m_aImageMap;
        ImageListsDescriptor&                                                       m_aImageList;
        ImageListItemDescriptor*                                                    m_pImages;
        ExternalImageItemListDescriptor*                                            m_pExternalImages;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >    m_xLocator;
};

class OWriteImagesDocumentHandler : private ThreadHelpBase // Struct for right initalization of lock member! Must be first of baseclasses.
{
    public:
        OWriteImagesDocumentHandler(
            const ImageListsDescriptor& aItems,
            ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler > );
        virtual ~OWriteImagesDocumentHandler();

        void WriteImagesDocument() throw
            ( ::com::sun::star::xml::sax::SAXException,
              ::com::sun::star::uno::RuntimeException );

    protected:
        virtual void WriteImageList( const ImageListItemDescriptor* ) throw
            ( ::com::sun::star::xml::sax::SAXException,
              ::com::sun::star::uno::RuntimeException );

        virtual void WriteExternalImageList( const ExternalImageItemListDescriptor* ) throw
            ( ::com::sun::star::xml::sax::SAXException,
              ::com::sun::star::uno::RuntimeException );

        virtual void WriteImage( const ImageItemDescriptor* ) throw
            ( ::com::sun::star::xml::sax::SAXException,
              ::com::sun::star::uno::RuntimeException );

        virtual void WriteExternalImage( const ExternalImageItemDescriptor* ) throw
            ( ::com::sun::star::xml::sax::SAXException,
              ::com::sun::star::uno::RuntimeException );

        const ImageListsDescriptor&                                                         m_aImageListsItems;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XDocumentHandler >    m_xWriteDocumentHandler;
        ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >      m_xEmptyList;
        OUString                                                                     m_aXMLXlinkNS;
        OUString                                                                     m_aXMLImageNS;
        OUString                                                                     m_aAttributeType;
        OUString                                                                     m_aAttributeXlinkType;
        OUString                                                                     m_aAttributeValueSimple;
};

} // namespace framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
