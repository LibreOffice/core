/*************************************************************************
 *
 *  $RCSfile: ManifestImport.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mtg $ $Date: 2001-09-05 19:26:06 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): Martin Gallwey (gallwey@sun.com)
 *
 *
 ************************************************************************/

#ifndef _MANIFEST_IMPORT_HXX
#define _MANIFEST_IMPORT_HXX

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx> // helper for implementations
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XDUCUMENTHANDLER_HPP_
#include <com/sun/star/xml/sax/XDocumentHandler.hpp>
#endif
#ifndef _COM_SUN_STAR_XML_SAX_XATTRIBUTELIST_HPP_
#include <com/sun/star/xml/sax/XAttributeList.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#include <vector>
#include <stack>

enum ElementNames
{
    e_Manifest,
    e_FileEntry,
    e_EncryptionData,
    e_Algorithm,
    e_KeyDerivation
};
class ManifestImport : public cppu::WeakImplHelper1 < com::sun::star::xml::sax::XDocumentHandler >
{
protected:
    com::sun::star::uno::Sequence < com::sun::star::beans::PropertyValue > aSequence;
    sal_Int16       nNumProperty;
    ::std::stack < ElementNames > aStack;
    sal_Bool bIgnoreEncryptData;
    ::std::vector < ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue > > & rManVector;

    const ::rtl::OUString sFileEntryElement;
    const ::rtl::OUString sManifestElement;
    const ::rtl::OUString sEncryptionDataElement;
    const ::rtl::OUString sAlgorithmElement;
    const ::rtl::OUString sKeyDerivationElement;

    const ::rtl::OUString sCdataAttribute;
    const ::rtl::OUString sMediaTypeAttribute;
    const ::rtl::OUString sFullPathAttribute;
    const ::rtl::OUString sSizeAttribute;
    const ::rtl::OUString sSaltAttribute;
    const ::rtl::OUString sInitialisationVectorAttribute;
    const ::rtl::OUString sIterationCountAttribute;
    const ::rtl::OUString sAlgorithmNameAttribute;
    const ::rtl::OUString sKeyDerivationNameAttribute;
    const ::rtl::OUString sChecksumAttribute;
    const ::rtl::OUString sChecksumTypeAttribute;

    const ::rtl::OUString sFullPathProperty;
    const ::rtl::OUString sMediaTypeProperty;
    const ::rtl::OUString sIterationCountProperty;
    const ::rtl::OUString sSaltProperty;
    const ::rtl::OUString sInitialisationVectorProperty;
    const ::rtl::OUString sSizeProperty;
    const ::rtl::OUString sDigestProperty;

    const ::rtl::OUString sWhiteSpace;
    const ::rtl::OUString sBlowfish;
    const ::rtl::OUString sPBKDF2;
    const ::rtl::OUString sMD5;

public:
    ManifestImport( std::vector < ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue > > & rNewVector );
    ~ManifestImport( void );
    virtual void SAL_CALL startDocument(  )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endDocument(  )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL startElement( const ::rtl::OUString& aName, const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XAttributeList >& xAttribs )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL endElement( const ::rtl::OUString& aName )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL characters( const ::rtl::OUString& aChars )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL ignorableWhitespace( const ::rtl::OUString& aWhitespaces )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL processingInstruction( const ::rtl::OUString& aTarget, const ::rtl::OUString& aData )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL setDocumentLocator( const ::com::sun::star::uno::Reference< ::com::sun::star::xml::sax::XLocator >& xLocator )
        throw(::com::sun::star::xml::sax::SAXException, ::com::sun::star::uno::RuntimeException);
};
#endif
