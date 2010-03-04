/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: filterdetect.hxx,v $
 * $Revision: 1.1 $
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

#ifndef OOX_CORE_FILTERDETECT_HXX
#define OOX_CORE_FILTERDETECT_HXX

#include <vector>
#include <com/sun/star/document/XExtendedFilterDetection.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/xml/sax/XFastDocumentHandler.hpp>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase2.hxx>
#include "oox/dllapi.h"

namespace com { namespace sun { namespace star {
    namespace io { class XInputStream; }
} } }

namespace comphelper { class MediaDescriptor; }

namespace oox { class AttributeList; }

namespace oox {
namespace core {

// ============================================================================

/** Document handler specifically designed for detecting OOXML file formats.

    It takes a reference to the filter string object via its constructor, and
    puts the name of the detected filter to it, if it successfully finds one.
 */
class FilterDetectDocHandler : public ::cppu::WeakImplHelper1< ::com::sun::star::xml::sax::XFastDocumentHandler >
{
public:
    explicit            FilterDetectDocHandler( ::rtl::OUString& rFilter );
    virtual             ~FilterDetectDocHandler();

    // XFastDocumentHandler
    virtual void SAL_CALL startDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endDocument() throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDocumentLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& xLocator ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

    // XFastContextHandler
    virtual void SAL_CALL startFastElement( sal_Int32 nElement, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startUnknownElement( const ::rtl::OUString& Namespace, const ::rtl::OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endFastElement( sal_Int32 Element ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endUnknownElement( const ::rtl::OUString& Namespace, const ::rtl::OUString& Name ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< XFastContextHandler > SAL_CALL createFastChildContext( sal_Int32 Element, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Reference< XFastContextHandler > SAL_CALL createUnknownChildContext( const ::rtl::OUString& Namespace, const ::rtl::OUString& Name, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XFastAttributeList >& Attribs ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL characters( const ::rtl::OUString& aChars ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString& aWhitespaces ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData ) throw (::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);

private:
    void                parseRelationship( const AttributeList& rAttribs );

    ::rtl::OUString     getFilterNameFromContentType( const ::rtl::OUString& rContentType ) const;
    void                parseContentTypesDefault( const AttributeList& rAttribs );
    void                parseContentTypesOverride( const AttributeList& rAttribs );

private:
    typedef ::std::vector< sal_Int32 > ContextVector;

    ::rtl::OUString&    mrFilterName;
    ContextVector       maContextStack;
    ::rtl::OUString     maTargetPath;
};

// ============================================================================

class OOX_DLLPUBLIC FilterDetect : public ::cppu::WeakImplHelper2< ::com::sun::star::document::XExtendedFilterDetection, ::com::sun::star::lang::XServiceInfo >
{
public:
    explicit            FilterDetect( const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >& rxFactory );
    virtual             ~FilterDetect();

    /** Tries to extract an unencrypted ZIP package from the passed media
        descriptor.

        First, this function checks if the input stream provided by the media
        descriptor property 'InputStream' contains a ZIP package. If yes, this
        stream is returned.

        Second, this function checks if the 'ComponentData' property exists and
        contains a sequence of com.sun.star.beans.NamedValue. If yes, a named
        value is searched with the name 'DecryptedPackage' and a value of type
        com.sun.star.io.XStream. If the input stream provided by this XStream
        contains a ZIP package, this input stream is returned.

        Third, this function checks if the input stream of the media descriptor
        contains an OLE package. If yes, it checks the existence of the streams
        'EncryptionInfo' and 'EncyptedPackage' and tries to decrypt the package
        into a temporary file. This may include requesting a password from the
        media descriptor property 'Password' or from the user, using the
        interaction handler provided by the descriptor. On success, and if the
        decrypted package is a ZIP package, the XStream of the temporary file
        is stored in the property 'ComponentData' of the media descriptor and
        its input stream is returned.
     */
    ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream >
                        extractUnencryptedPackage( ::comphelper::MediaDescriptor& rMediaDesc ) const;

    // com.sun.star.lang.XServiceInfo interface -------------------------------

    virtual ::rtl::OUString SAL_CALL getImplementationName() throw( ::com::sun::star::uno::RuntimeException );
    virtual sal_Bool SAL_CALL supportsService( const ::rtl::OUString& rServiceName ) throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::rtl::OUString > SAL_CALL getSupportedServiceNames() throw( ::com::sun::star::uno::RuntimeException );

    // com.sun.star.document.XExtendedFilterDetection interface ---------------

    /** Detects MS Office 2007 file types and supports package decryption.

        The following file types are detected:
        - MS Word 2007 XML Document (*.docx, *.docm)
        - MS Word 2007 XML Template (*.dotx, *.dotm)
        - MS Excel 2007 XML Document (*.xlsx, *.xlsm)
        - MS Excel 2007 BIFF12 Document (*.xlsb)
        - MS Excel 2007 XML Template (*.xltx, *.xltm)
        - MS Powerpoint 2007 XML Document (*.pptx, *.pptm)
        - MS Powerpoint 2007 XML Template (*.potx, *.potm)

        If the package is encrypted, the detection tries to decrypt it into a
        temporary file. The user may be asked for a password. The XStream
        interface of the temporary file will be stored in the 'ComponentData'
        property of the passed media descriptor.
     */
    virtual ::rtl::OUString SAL_CALL detect(
                            ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& rMediaDescSeq )
                            throw( ::com::sun::star::uno::RuntimeException );

private:
    ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory > mxFactory;
};

// ============================================================================

} // namespace core
} // namespace oox

#endif

