/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: imagesdocumenthandler.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 11:08:52 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef __FRAMEWORK_XML_IMAGEDOCUMENTHANDLER_HXX_
#define __FRAMEWORK_XML_IMAGEDOCUMENTHANDLER_HXX_

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef __COM_SUN_STAR_XML_SAX_XDOCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_XML_IMAGESCONFIGURATION_HXX_
#include <xml/imagesconfiguration.hxx>
#endif

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef _RTL_USTRING_
#include <rtl/ustring.hxx>
#endif

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

#include <hash_map>

#ifndef __FRAMEWORK_STDTYPES_H_
#include <stdtypes.h>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//*****************************************************************************************************************
// Hash code function for using in all hash maps of follow implementation.

class OReadImagesDocumentHandler : public ::com::sun::star::xml::sax::XDocumentHandler,
                                   private ThreadHelpBase,  // Struct for right initalization of lock member! Must be first of baseclasses.
                                   public ::cppu::OWeakObject
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

        // XInterface
        virtual void SAL_CALL acquire() throw()
            { OWeakObject::acquire(); }
        virtual void SAL_CALL release() throw()
            { OWeakObject::release(); }
        virtual ::com::sun::star::uno::Any SAL_CALL queryInterface(
            const ::com::sun::star::uno::Type & rType ) throw( ::com::sun::star::uno::RuntimeException );

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

        class ImageHashMap : public ::std::hash_map< ::rtl::OUString        ,
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
