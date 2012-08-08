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

#ifndef FRAMEWORK_XML_IMAGEDOCUMENTHANDLER_HXX_
#define FRAMEWORK_XML_IMAGEDOCUMENTHANDLER_HXX_

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
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL endDocument(void)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL startElement(
            const rtl::OUString& aName,
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList > &xAttribs)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL endElement(const rtl::OUString& aName)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL characters(const rtl::OUString& aChars)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL ignorableWhitespace(const rtl::OUString& aWhitespaces)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL processingInstruction(const rtl::OUString& aTarget,
                                                    const rtl::OUString& aData)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

        virtual void SAL_CALL setDocumentLocator(
            const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator > &xLocator)
        throw(  ::com::sun::star::xml::sax::SAXException,
                ::com::sun::star::uno::RuntimeException );

    private:
        ::rtl::OUString getErrorLineString();

        class ImageHashMap : public ::boost::unordered_map< ::rtl::OUString     ,
                                                     Image_XML_Entry        ,
                                                     OUStringHashCode       ,
                                                     ::std::equal_to< ::rtl::OUString > >
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
        ::rtl::OUString                                                                     m_aXMLXlinkNS;
        ::rtl::OUString                                                                     m_aXMLImageNS;
        ::rtl::OUString                                                                     m_aAttributeType;
        ::rtl::OUString                                                                     m_aAttributeXlinkType;
        ::rtl::OUString                                                                     m_aAttributeValueSimple;
};

} // namespace framework

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
