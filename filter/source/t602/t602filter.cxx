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

#include "t602filter.hxx"
#include <strings.hrc>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/awt/UnoControlDialog.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <osl/diagnose.h>
#include <rtl/ref.hxx>
#include <rtl/character.hxx>
#include <unotools/streamwrap.hxx>
#include <utility>

using namespace ::cppu;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::xml::sax;
using namespace ::com::sun::star::document;
using com::sun::star::io::XInputStream;

namespace T602ImportFilter {

    unsigned char const kam2lat[129] =
        //    0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
        "\xAC\x81\x82\xD4\x84\xD2\x9B\x9F\xD8\xB7\x91\xD6\x96\x92\x8E\xB5"  // 8
        "\x90\xA7\xA6\x93\x94\xE0\x85\xE9\xEC\x99\x9A\xE6\x95\xED\xFC\x9C"  // 9
        "\xA0\xA1\xA2\xA3\xE5\xD5\xDE\xE2\xE7\xFD\xEA\xE8\xCE\xF5\xAE\xAF"  // A
        "\x80\x80\x83\xB3\xB4\xE1\x87\x88\x89\x8A\x8B\x8C\x8F\x97\x8D\xBF"  // B
        "\xC0\xC1\xC2\xC3\xC4\xC5\xCF\x98\x9D\x9E\xA4\xA5\xA8\xA9\x86\xF3"  // C
        "\xCD\xB1\xB2\xB6\xB8\xB9\xBA\xBB\xF7\xD9\xDA\xDB\xDC\xF8\xF9\xDF"  // D
        "\xD0\xD1\xD3\xD7\xAA\xAB\xDD\xB0\xE3\xE4\xEB\xEE\xEF\xF0\xF2\xF4"  // E
        "\xBC\xBD\xBE\xC6\xC7\xC8\xF6\xC9\xCA\xFA\xFB\xCB\xF1\xCC\xFE\xFF"; // F

    unsigned char const koi2lat[129] =
        //    0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
        "\x80\x83\x86\xF5\xE1\x87\x88\x89\x8A\x8B\x8C\x8F\x97\xCF\xCF\x98"  // 8
        "\x9D\x9E\xA4\xA5\xA8\xA9\xDB\xDC\xDF\xB1\xB2\xB6\xB8\xB9\xBA\xBB"  // 9
        "\xBC\xBD\xBE\xC6\xC7\xC8\xF6\xC9\xCA\xCB\xCC\xCD\xCE\xAD\xAE\xAF"  // A
        "\xD0\xD1\xD3\xD7\xAA\xAB\xDD\xB0\xE3\xE4\xEB\xEE\xEF\xF0\xF2\xF4"  // B
        "\xDA\xA0\xC0\x9F\xD4\xD8\xEA\xC4\x81\xA1\x85\x92\x96\x94\xE5\xA2"  // C
        "\x93\x84\xFD\xE7\x9C\xA3\xC3\x82\xF7\xEC\xA7\xC2\xF3\xF8\xF9\xC5"  // D
        "\xBF\xB5\xD9\xAC\xD2\xB7\xE8\xB3\x9A\xD6\xDE\x91\x95\x99\xD5\xE0"  // E
        "\xE2\x8E\xFC\xE6\x9B\xE9\xB4\x90\xFA\xED\xA6\xC1\xF1\xFB\xFE\xFF"; // F

    unsigned char const lat2UNC[257] =
        //    0       1       2       3       4       5       6       7
        //    8       9       A       B       C       D       E       F
        "\x00\xe7\x00\xfc\x00\xe9\x00\xf9\x00\xe4\x01\x6f\x00\xe8\x00\xa3"  // 8
        "\x00\xc6\x00\xd8\x00\xc5\x00\xe6\x00\xf8\x00\xc4\x00\xc4\x00\xe5"
        "\x00\xc9\x01\x39\x00\xf4\x00\xf4\x00\xf6\x01\x3d\x01\x3e\x00\x20"  // 9
        "\x00\xf2\x00\xd6\x00\xdc\x01\x64\x01\x65\x00\xec\x20\xa7\x01\x0d"
        "\x00\xe1\x00\xed\x00\xf3\x00\xfa\x00\xa1\x00\xd1\x01\x7d\x01\x7e"  // A
        "\x00\xbf\x00\xf1\x22\x11\x03\xc3\x01\x0c\x00\xa7\x00\xbb\x00\xab"
        "\x25\x91\x25\x92\x25\x93\x25\x02\x25\x24\x00\xc1\x00\xc2\x01\x1a"  // B
        "\x00\x20\x00\x20\x00\x20\x00\x20\x22\x61\x00\xb1\x22\x65\x25\x10"
        "\x25\x14\x25\x34\x25\x2c\x25\x1c\x25\x00\x25\x3c\x22\x64\x23\x20"  // C
        "\x23\x21\x22\x48\x00\xba\x22\x1a\x00\xb2\x00\xbd\x00\xbc\x00\xa4"
        "\x03\xb1\x03\xb2\x01\x0e\x03\x93\x01\x0f\x01\x47\x00\xcd\x03\xc0"  // D
        "\x01\x1b\x25\x18\x25\x0c\x25\x88\x25\x84\x00\xb5\x01\x6e\x25\x80"
        "\x00\xd3\x00\xdf\x00\xd4\x03\xa6\x03\x98\x01\x48\x01\x60\x01\x61"  // E
        "\x01\x54\x00\xda\x01\x55\x03\xa9\x00\xfd\x00\xdd\x03\xb4\x22\x1e"
        "\x00\xf8\x02\xdd\x03\xb5\x02\xc7\x22\x29\x00\xa7\x00\xf7\x00\xe0"  // F
        "\x00\xb4\x00\xb0\x00\xc0\x02\xc6\x01\x58\x01\x59\x00\x20\x00\x20";

    unsigned char const rus2UNC[257] =
        //    0       1       2       3       4       5       6       7
        //    8       9       A       B       C       D       E       F
        "\x04\x11\x00\xfc\x00\xe9\x04\x12\x00\xe4\x01\x6f\x04\x13\x04\x14"  // 8
        "\x04\x01\x04\x16\x04\x17\x04\x18\x04\x19\x00\xc4\x00\xc4\x04\x1a"
        "\x00\xc9\x01\x39\x01\x3a\x00\xf4\x00\xf6\x01\x3d\x01\x3e\x00\x20"  // 9
        "\x04\x1c\x00\xd6\x00\xdc\x01\x64\x01\x65\x04\x1d\x04\x1f\x01\x0a"
        "\x00\xe1\x00\xed\x00\xf3\x00\xfa\x04\x20\x04\x22\x01\x7d\x01\x7e"  // A
        "\x04\x23\x04\x24\x04\x43\x04\x44\x01\x0c\x00\xa7\x04\x3b\x04\x3c"
        "\x04\x47\x04\x26\x04\x27\x25\x02\x25\x24\x00\xc1\x04\x28\x01\x1a"  // B
        "\x04\x29\x04\x2a\x04\x2b\x04\x2c\x04\x2d\x04\x2e\x04\x2f\x25\x10"
        "\x25\x14\x25\x34\x25\x2c\x25\x1c\x25\x00\x25\x3c\x04\x31\x04\x32"  // C
        "\x04\x33\x04\x51\x04\x36\x04\x37\x04\x38\x04\x39\x04\x3a\x04\x1b"
        "\x04\x3d\x04\x3f\x01\x0e\x04\x40\x01\x0f\x01\x47\x00\xcd\x04\x42"  // D
        "\x01\x1b\x25\x18\x25\x0c\x25\x88\x25\x84\x04\x46\x01\x6e\x25\x80"
        "\x00\xd3\x00\xdf\x00\xd4\x04\x48\x04\x49\x01\x48\x01\x60\x01\x61"  // E
        "\x01\x54\x00\xda\x01\x55\x04\x4a\x00\xfd\x00\xdd\x04\x4b\x04\x4c"
        "\x04\x4d\x02\xdd\x04\x4e\x02\xc7\x04\x4f\x00\xa7\x04\x34\x00\xe0"  // F
        "\x00\xb4\x00\xb0\x00\xc0\x02\xc6\x01\x58\x01\x59\x00\x20\x00\x20";

#define Start_(_nam) \
    if (mxHandler.is()) \
    { \
        mxHandler->startElement(_nam, xAttrList); \
        if (mpAttrList) \
            mpAttrList->Clear(); \
    }

#define End_(_nam) \
    if (mxHandler.is()) \
    { \
        mxHandler->endElement(_nam); \
    }

static inistruct ini;

T602ImportFilter::T602ImportFilter(css::uno::Reference<css::uno::XComponentContext > x )
    : mxContext(std::move(x))
    , node(tnode::START)
{
}

T602ImportFilter::T602ImportFilter(css::uno::Reference<css::io::XInputStream> xInputStream)
    : mxInputStream(std::move(xInputStream))
    , node(tnode::START)
{
}


T602ImportFilter::~T602ImportFilter()
{
}

// XExtendedTypeDetection
OUString T602ImportFilter::detect( Sequence<PropertyValue>& Descriptor)
{
    sal_Int32 nLength = Descriptor.getLength();
    const PropertyValue * pValue = Descriptor.getConstArray();
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name == "InputStream" )
            pValue[i].Value >>= mxInputStream;
    }

    if (!mxInputStream.is())
        return OUString();

    css::uno::Sequence< sal_Int8 > aData;
    const size_t numBytes = 4;
    size_t numBytesRead = mxInputStream->readSomeBytes (aData, numBytes);

    if ((numBytesRead != numBytes) || (aData[0] != '@') ||
        (aData[1] != 'C') || (aData[2] != 'T') || (aData[3] != ' '))
        return OUString();

    return u"writer_T602_Document"_ustr;
}

// XFilter
sal_Bool SAL_CALL T602ImportFilter::filter( const Sequence< css::beans::PropertyValue >& aDescriptor )
{
    return importImpl ( aDescriptor );
}

// XImporter
void SAL_CALL T602ImportFilter::setTargetDocument( const Reference< css::lang::XComponent >& xDoc )
{
    mxDoc = xDoc;
}

// XInitialization
void SAL_CALL T602ImportFilter::initialize( const Sequence< Any >& /*aArguments*/ )
{
}

// Other functions

void T602ImportFilter::inschr(unsigned char ch)
{
    Reference < XAttributeList > xAttrList ( mpAttrList );

    if(!ini.showcomm&&pst.comment) return;

    if(ch==' ') {
        setfnt(chngul,true);
        pst.wasspace++;
        return;
    }

    if(pst.wasspace > 0) {
        if(ini.reformatpars) {
            if(!pst.wasfdash)
                inschrdef(' ');
            pst.wasfdash = false;
        } else {
            if (mpAttrList)
                mpAttrList->AddAttribute(u"text:c"_ustr,OUString::number(pst.wasspace));
            Start_(u"text:s"_ustr);
            End_(u"text:s"_ustr);
        }
    }

    pst.wasspace = 0;
    setfnt(chngul,true);
    inschrdef(ch);
}

bool T602ImportFilter::importImpl( const Sequence< css::beans::PropertyValue >& aDescriptor )
{
    Reset602();

    sal_Int32 nLength = aDescriptor.getLength();
    const PropertyValue * pValue = aDescriptor.getConstArray();
    for ( sal_Int32 i = 0 ; i < nLength; i++)
    {
        if ( pValue[i].Name == "InputStream" )
            pValue[i].Value >>= mxInputStream;
    }

    if ( !mxInputStream.is() )
    {
        OSL_ASSERT( false );
        return false;
    }

    // An XML import service: what we push sax messages to...
    mxHandler.set( mxContext->getServiceManager()->createInstanceWithContext( u"com.sun.star.comp.Writer.XMLImporter"_ustr, mxContext ), UNO_QUERY );

    // The XImporter sets up an empty target document for XDocumentHandler to write to...
    Reference < XImporter > xImporter(mxHandler, UNO_QUERY);
    xImporter->setTargetDocument(mxDoc);

    auto const fs = OUString(OUString::number(inistruct::fontsize) + "pt");
    auto const fs2 = OUString(OUString::number(2*inistruct::fontsize) + "pt");

    mpAttrList = new comphelper::AttributeList;

    Reference < XAttributeList > xAttrList ( mpAttrList );

    mxHandler->startDocument();

    mpAttrList->AddAttribute(u"xmlns:office"_ustr, u"http://openoffice.org/2000/office"_ustr);
    mpAttrList->AddAttribute(u"xmlns:style"_ustr, u"http://openoffice.org/2000/style"_ustr);
    mpAttrList->AddAttribute(u"xmlns:text"_ustr, u"http://openoffice.org/2000/text"_ustr);
    mpAttrList->AddAttribute(u"xmlns:table"_ustr, u"http://openoffice.org/2000/table"_ustr);
    mpAttrList->AddAttribute(u"xmlns:draw"_ustr, u"http://openoffice.org/2000/draw"_ustr);
    mpAttrList->AddAttribute(u"xmlns:fo"_ustr, u"http://www.w3.org/1999/XSL/Format"_ustr);
    mpAttrList->AddAttribute(u"xmlns:xlink"_ustr, u"http://www.w3.org/1999/xlink"_ustr);
    mpAttrList->AddAttribute(u"xmlns:number"_ustr, u"http://openoffice.org/2000/datastyle"_ustr);
    mpAttrList->AddAttribute(u"xmlns:svg"_ustr, u"http://www.w3.org/2000/svg"_ustr);
    mpAttrList->AddAttribute(u"xmlns:chart"_ustr, u"http://openoffice.org/2000/chart"_ustr);
    mpAttrList->AddAttribute(u"xmlns:dr3d"_ustr, u"http://openoffice.org/2000/dr3d"_ustr);
    mpAttrList->AddAttribute(u"xmlns:math"_ustr, u"http://www.w3.org/1998/Math/MathML"_ustr);
    mpAttrList->AddAttribute(u"xmlns:form"_ustr, u"http://openoffice.org/2000/form"_ustr);
    mpAttrList->AddAttribute(u"xmlns:script"_ustr, u"http://openoffice.org/2000/script"_ustr);
    mpAttrList->AddAttribute(u"office:class"_ustr, u"text"_ustr);
    mpAttrList->AddAttribute(u"office:version"_ustr, u"1.0"_ustr);
    Start_(u"office:document-content"_ustr);

    Start_(u"office:font-decls"_ustr);
    mpAttrList->AddAttribute(u"style:name"_ustr,u"Courier New"_ustr);
    mpAttrList->AddAttribute(u"fo:font-family"_ustr,u"Courier New"_ustr);
    mpAttrList->AddAttribute(u"style:font-pitch"_ustr,u"fixed"_ustr);
    Start_(u"style:font-decl"_ustr);
    End_(u"style:font-decl"_ustr);
    End_(u"office:font-decls"_ustr);

    Start_(u"office:automatic-styles"_ustr);

    // Standardni text
    mpAttrList->AddAttribute(u"style:name"_ustr,u"P1"_ustr);
    mpAttrList->AddAttribute(u"style:family"_ustr,u"paragraph"_ustr);
    mpAttrList->AddAttribute(u"style:parent-style-name"_ustr,u"Standard"_ustr);
    Start_(u"style:style"_ustr);
    mpAttrList->AddAttribute(u"style:font-name"_ustr,u"Courier New"_ustr);
    mpAttrList->AddAttribute(u"fo:font-size"_ustr,fs);
    Start_(u"style:properties"_ustr);
    End_(u"style:properties"_ustr);
    End_(u"style:style"_ustr);

    // Standardni text - konec stranky
    mpAttrList->AddAttribute(u"style:name"_ustr,u"P2"_ustr);
    mpAttrList->AddAttribute(u"style:family"_ustr,u"paragraph"_ustr);
    mpAttrList->AddAttribute(u"style:parent-style-name"_ustr,u"Standard"_ustr);
    Start_(u"style:style"_ustr);
    mpAttrList->AddAttribute(u"style:font-name"_ustr,u"Courier New"_ustr);
    mpAttrList->AddAttribute(u"fo:font-size"_ustr,fs);
    mpAttrList->AddAttribute(u"fo:break-before"_ustr,u"page"_ustr);
    Start_(u"style:properties"_ustr);
    End_(u"style:properties"_ustr);
    End_(u"style:style"_ustr);

    // T1 Normalni text
    mpAttrList->AddAttribute(u"style:name"_ustr,u"T1"_ustr);
    mpAttrList->AddAttribute(u"style:family"_ustr,u"text"_ustr);
    Start_(u"style:style"_ustr);
    Start_(u"style:properties"_ustr);
    End_(u"style:properties"_ustr);
    End_(u"style:style"_ustr);

    // T2 Tucny text
    mpAttrList->AddAttribute(u"style:name"_ustr,u"T2"_ustr);
    mpAttrList->AddAttribute(u"style:family"_ustr,u"text"_ustr);
    Start_(u"style:style"_ustr);
    mpAttrList->AddAttribute(u"fo:font-weight"_ustr,u"bold"_ustr);
    Start_(u"style:properties"_ustr);
    End_(u"style:properties"_ustr);
    End_(u"style:style"_ustr);

    // T3 Kurziva
    mpAttrList->AddAttribute(u"style:name"_ustr,u"T3"_ustr);
    mpAttrList->AddAttribute(u"style:family"_ustr,u"text"_ustr);
    Start_(u"style:style"_ustr);
    mpAttrList->AddAttribute(u"fo:font-style"_ustr,u"italic"_ustr);
    Start_(u"style:properties"_ustr);
    End_(u"style:properties"_ustr);
    End_(u"style:style"_ustr);

    // T4 Siroky text
    mpAttrList->AddAttribute(u"style:name"_ustr,u"T4"_ustr);
    mpAttrList->AddAttribute(u"style:family"_ustr,u"text"_ustr);
    Start_(u"style:style"_ustr);
    mpAttrList->AddAttribute(u"fo:font-weight"_ustr,u"bold"_ustr);
    mpAttrList->AddAttribute(u"style:text-scale"_ustr,u"200%"_ustr);
    Start_(u"style:properties"_ustr);
    End_(u"style:properties"_ustr);
    End_(u"style:style"_ustr);

    // T5 Vysoky text
    mpAttrList->AddAttribute(u"style:name"_ustr,u"T5"_ustr);
    mpAttrList->AddAttribute(u"style:family"_ustr,u"text"_ustr);
    Start_(u"style:style"_ustr);
    mpAttrList->AddAttribute(u"fo:font-size"_ustr,fs2);
    mpAttrList->AddAttribute(u"fo:font-weight"_ustr,u"bold"_ustr);
    mpAttrList->AddAttribute(u"style:text-scale"_ustr,u"50%"_ustr);
    Start_(u"style:properties"_ustr);
    End_(u"style:properties"_ustr);
    End_(u"style:style"_ustr);

    // T6 Velky text
    mpAttrList->AddAttribute(u"style:name"_ustr,u"T6"_ustr);
    mpAttrList->AddAttribute(u"style:family"_ustr,u"text"_ustr);
    Start_(u"style:style"_ustr);
    mpAttrList->AddAttribute(u"fo:font-size"_ustr,fs2);
    mpAttrList->AddAttribute(u"fo:font-weight"_ustr,u"bold"_ustr);
    Start_(u"style:properties"_ustr);
    End_(u"style:properties"_ustr);
    End_(u"style:style"_ustr);

    // T7 Podtrzeny text
    mpAttrList->AddAttribute(u"style:name"_ustr,u"T7"_ustr);
    mpAttrList->AddAttribute(u"style:family"_ustr,u"text"_ustr);
    Start_(u"style:style"_ustr);
    mpAttrList->AddAttribute(u"style:text-underline"_ustr,u"single"_ustr);
    Start_(u"style:properties"_ustr);
    End_(u"style:properties"_ustr);
    End_(u"style:style"_ustr);

    // T8 Podtrzena tucny text
    mpAttrList->AddAttribute(u"style:name"_ustr,u"T8"_ustr);
    mpAttrList->AddAttribute(u"style:family"_ustr,u"text"_ustr);
    Start_(u"style:style"_ustr);
    mpAttrList->AddAttribute(u"fo:font-weight"_ustr,u"bold"_ustr);
    mpAttrList->AddAttribute(u"style:text-underline"_ustr,u"single"_ustr);
    Start_(u"style:properties"_ustr);
    End_(u"style:properties"_ustr);
    End_(u"style:style"_ustr);

    // T9 Podtrzena kurziva
    mpAttrList->AddAttribute(u"style:name"_ustr,u"T9"_ustr);
    mpAttrList->AddAttribute(u"style:family"_ustr,u"text"_ustr);
    Start_(u"style:style"_ustr);
    mpAttrList->AddAttribute(u"fo:font-style"_ustr,u"italic"_ustr);
    mpAttrList->AddAttribute(u"style:text-underline"_ustr,u"single"_ustr);
    Start_(u"style:properties"_ustr);
    End_(u"style:properties"_ustr);
    End_(u"style:style"_ustr);

    // T10 Horni index
    mpAttrList->AddAttribute(u"style:name"_ustr,u"T10"_ustr);
    mpAttrList->AddAttribute(u"style:family"_ustr,u"text"_ustr);
    Start_(u"style:style"_ustr);
    mpAttrList->AddAttribute(u"style:text-position"_ustr,u"27% 100%"_ustr);
    Start_(u"style:properties"_ustr);
    End_(u"style:properties"_ustr);
    End_(u"style:style"_ustr);

    // T11 Dolni index
    mpAttrList->AddAttribute(u"style:name"_ustr,u"T11"_ustr);
    mpAttrList->AddAttribute(u"style:family"_ustr,u"text"_ustr);
    Start_(u"style:style"_ustr);
    mpAttrList->AddAttribute(u"style:text-position"_ustr,u"-27% 100%"_ustr);
    Start_(u"style:properties"_ustr);
    End_(u"style:properties"_ustr);
    End_(u"style:style"_ustr);

    End_(u"office:automatic-styles"_ustr);

    Start_(u"office:styles"_ustr);
    End_(u"office:styles"_ustr);

    Start_(u"office:body"_ustr);

    Read602();

    End_(u"office:body"_ustr);
    End_(u"office:document-content"_ustr);

    mxHandler->endDocument();

    return true;
}

void T602ImportFilter::test()
{
    Reset602();
    Read602();
}

void T602ImportFilter::Reset602()
{
    node = tnode::START;

    format602.mt = 0;
    format602.mb = 0;
    format602.tb = 0;
    format602.ct = 0;
    format602.pn = 1;
    format602.lh = 1;
    format602.lm = 6;
    format602.rm = 60;
    format602.pl = 60;

    ini.showcomm    = false;
    ini.forcecode   = false;
    ini.xcode    = KAM;
    ini.ruscode    = false;
    ini.reformatpars= false;

    fst.nowfnt    = fst.oldfnt    = standard;
    fst.uline    = fst.olduline    = false;

    pst.pars    = 0;
    pst.comment    = false;
    pst.wasspace    = 0;
    pst.ccafterln    = false;
    pst.willbeeop    = false;
    pst.waspar    = false;
    pst.wasfdash    = false;
}

void T602ImportFilter::inschrdef(unsigned char ch)
{
    sal_Unicode xch;

    if(ch > 127) {

        switch(ini.xcode) {
            case L2 : break;
            case KOI: ch = koi2lat[ch-128]; break;
            case KAM: ch = kam2lat[ch-128]; break;
            default : ch = kam2lat[ch-128];
        }

        if(ini.ruscode)
            xch = (rus2UNC[(ch-128)*2] << 8) + rus2UNC[(ch-128)*2+1];
        else
            xch = (lat2UNC[(ch-128)*2] << 8) + lat2UNC[(ch-128)*2+1];
    } else
        xch = ch;

    pst.waspar = false;
    if (mxHandler.is())
        mxHandler->characters(OUString(xch));
}

void T602ImportFilter::wrtfnt()
{
    Reference < XAttributeList > xAttrList ( mpAttrList );
    const char *style;

    switch(fst.nowfnt) {
        case standard : style = fst.uline ? "T7" : "T1"; break;
        case fat      : style = fst.uline ? "T8" : "T2"; break;
        case cursive  : style = fst.uline ? "T9" : "T3"; break;
        case bold     : style = "T4"; break;
        case tall     : style = "T5"; break;
        case big      : style = "T6"; break;
        case lindex   : style = "T11";break;
        case hindex   : style = "T10";break;
        default       : style = "T1"; break;
    }

    End_(u"text:span"_ustr);
    if (mpAttrList)
        mpAttrList->AddAttribute(
            u"text:style-name"_ustr, OUString::createFromAscii(style));
    Start_(u"text:span"_ustr);
}

void T602ImportFilter::setfnt(fonts fnt,bool mustwrite)
{
    if (fnt == fst.oldfnt && fnt == fst.nowfnt && !mustwrite)
        fst.nowfnt = standard;
    else if (fnt != chngul)
        fst.nowfnt = fnt;

    if (mustwrite && (fst.oldfnt != fst.nowfnt || fst.olduline != fst.uline))
    {
        wrtfnt();
        fst.oldfnt = fst.nowfnt;
        fst.olduline = fst.uline;
    }
}

unsigned char T602ImportFilter::Readchar602()
{
    static Sequence< sal_Int8 > aData;

    return (mxInputStream->readBytes(aData, 1) > 0) ?
        reinterpret_cast<unsigned char const *>(aData.getConstArray())[0] : 0;
}

void T602ImportFilter::par602(bool endofpage)
{
    sal_Int16 endp;
    Reference < XAttributeList > xAttrList ( mpAttrList );

    if(!endofpage || !pst.waspar) {
        if(ini.showcomm||!pst.comment) {
            if(pst.waspar||ini.reformatpars) {
                End_(u"text:span"_ustr);
                End_(u"text:p"_ustr);
                if (mpAttrList)
                    mpAttrList->AddAttribute(u"text:style-name"_ustr, u"P1"_ustr);
                Start_(u"text:p"_ustr);
                Start_(u"text:span"_ustr);
                wrtfnt();
            } else {
                Start_(u"text:line-break"_ustr);
                End_(u"text:line-break"_ustr);
            }
        }

        pst.waspar = true;
    }

    if(pst.comment)
        return;

    switch(format602.lh) {
        case 3: pst.pars += 4; break;
        case 4: pst.pars += 3; break;
        case 6: pst.pars += 2; break;
        default: pst.pars +=2; break;
    }
    endp = format602.pl-format602.mt-format602.mb;
    if(!(((pst.pars+1)/2) >= endp || endofpage))        return;

    pst.pars = 0;
    if(!ini.reformatpars) {
        End_(u"text:span"_ustr);
        End_(u"text:p"_ustr);
        if (mpAttrList)
            mpAttrList->AddAttribute(u"text:style-name"_ustr, u"P2"_ustr);
        Start_(u"text:p"_ustr);
        Start_(u"text:span"_ustr);
        wrtfnt();
    }
}

sal_Int16 T602ImportFilter::readnum(unsigned char *ch, bool show)
{
    unsigned char buff[21];
    sal_Int16 i=0;

    while((i<20) && *ch && (*ch!='\n') && (*ch!=','))
    {
        *ch = Readchar602();
        if(show) inschr(*ch);
        buff[i++] = *ch;
    }
    buff[i]='\0';

    // mba: cast from unsigned char via int to sal_Int16 seems to be safe
    return static_cast<sal_Int16>(atoi(reinterpret_cast<char const *>(buff)));
}

unsigned char T602ImportFilter::Setformat602(const char *cmd)
{
    unsigned char ch;

    ch = Readchar602();

    if (cmd[0]=='M' && cmd[1]=='T') format602.mt = readnum(&ch,false);
    else if (cmd[0]=='M' && cmd[1]=='B') format602.mb = readnum(&ch,false);
    // else if (cmd[0]=='T' && cmd[1]=='B') {}
    // else if (cmd[0]=='H' && cmd[1]=='E') {}
    // else if (cmd[0]=='F' && cmd[1]=='O') {}
    else if (cmd[0]=='C' && cmd[1]=='T') {
        format602.ct = readnum(&ch,false);
        if (!ini.forcecode)
            switch(format602.ct) {
                case 0:  ini.xcode = KAM; break;
                case 1:  ini.xcode = L2;  break;
                case 2:  ini.xcode = KOI; break;
                default: ini.xcode = KAM; break;
            }
        }
    else if (cmd[0]=='P' && cmd[1]=='N')  format602.pn = readnum(&ch,false);
    else if (cmd[0]=='L' && cmd[1]=='H')  format602.lh = readnum(&ch,false);
    // else if(cmd[0]=='S' && cmd[1]=='T') {} //styl
    else if (cmd[0]=='L' && cmd[1]=='M')  format602.lm = readnum(&ch,false);
    else if (cmd[0]=='R' && cmd[1]=='M')  format602.rm = readnum(&ch,false);
    else if (cmd[0]=='P' && cmd[1]=='L')  format602.pl = readnum(&ch,false);
    else if (cmd[0]=='P' && cmd[1]=='A') { if(pst.pars) par602(true); }
    else if (cmd[0]=='C' && cmd[1]=='P') {
        sal_Int16 i = readnum(&ch,false);
        if ((i+pst.pars/2) > (format602.pl-format602.mt-format602.mb))
            if(pst.pars) par602(true);
    }
    while (ch && (ch != '\n')) ch = Readchar602();
    return ch;
}

tnode T602ImportFilter::PointCmd602(unsigned char *ch)
{
    char pcmd[2];

    // warning: uChar -> char
    pcmd[0] = static_cast<char>(rtl::toAsciiUpperCase(*ch)); inschr(*ch);
    *ch = Readchar602();
    if (!*ch) return tnode::EEND;
    if (*ch=='\n') return tnode::EOL;
    if (!rtl::isAsciiAlpha(*ch)) return (*ch<32) ? tnode::SETCH : tnode::WRITE;

    // warning: uChar -> char
    pcmd[1] = static_cast<char>(rtl::toAsciiUpperCase(*ch)); inschr(*ch);

    if (pcmd[0]=='P' && pcmd[1]=='A') { if (pst.pars) pst.willbeeop = true; }
    else if (pcmd[0]=='C' && pcmd[1]=='P') { if (pst.pars) pst.willbeeop = true; }
    else if (pcmd[0]=='P' && pcmd[1]=='I') {
        while (*ch && (*ch != '\n') && (*ch != ','))
            { *ch = Readchar602(); inschr(*ch); }
        if (!*ch) return tnode::EEND;
        if (*ch=='\n') return tnode::EOL;
        if (*ch==',') { *ch = Readchar602(); inschr(*ch); }
        pst.pars += (readnum(ch,true)*2);
        if (!*ch) return tnode::EEND;
        if (*ch=='\n') return tnode::EOL;
    }
    // else if(pcmd[0]=='K'&&pcmd[1]=='P') {}
    // else if(pcmd[0]=='H'&&pcmd[1]=='E') {}
    // else if(pcmd[0]=='F'&&pcmd[1]=='O') {}

    return tnode::READCH;
}


void T602ImportFilter::Read602()
{
    unsigned char ch=0;
    char cmd602[3] = {0};

    Reference < XAttributeList > xAttrList ( mpAttrList );

    if (node==tnode::QUIT) return;

    if (mpAttrList)
        mpAttrList->AddAttribute(u"text:style-name"_ustr, u"P1"_ustr);
    Start_(u"text:p"_ustr);
    if (mpAttrList)
        mpAttrList->AddAttribute(u"text:style-name"_ustr, u"T1"_ustr);
    Start_(u"text:span"_ustr);

    if (node==tnode::START) { node = tnode::EOL; }

    while (node != tnode::EEND) {
        switch (node) {
        case tnode::READCH:
            ch = Readchar602();
            if (ch == 0) node = tnode::EEND;
            else if (ch == '\n') {
                if(!pst.willbeeop) par602(false);
                node = tnode::EOL;
            } else if (ch < 32) node = tnode::SETCH;
            else node = tnode::WRITE;
            break;
        case tnode::EOL:
            ch = Readchar602();
            pst.comment = false;
            if (pst.willbeeop) par602(true);
            pst.willbeeop = false;
            if(ch == 0) node = tnode::EEND;
            else if (ch == '@') node = tnode::EXPCMD;
            else if (ch == '\n') { par602(false); node = tnode::EOL; }
            else if (ch < 32) {pst.ccafterln = true; node = tnode::SETCH; break;}
            else node = tnode::WRITE;
            if (ch == '.') { pst.comment = true; node = tnode::POCMD; }
            pst.ccafterln = false;
            break;

        case tnode::POCMD: inschr('.');
            ch = Readchar602();
            if(ch == 0) node = tnode::EEND;
            else if(rtl::isAsciiAlpha(ch)) node = PointCmd602(&ch);
            else if(ch <32) node=tnode::SETCH;
            else node = tnode::WRITE;
            break;

        case tnode::EXPCMD: ch = Readchar602();
            if(ch == 0) {inschr('@'); node = tnode::EEND; }
            else if(rtl::isAsciiUpperCase(ch)) {
                cmd602[0] = ch;
                ch = Readchar602();
                cmd602[1] = ch;
                cmd602[2] = '\0';
                if(rtl::isAsciiUpperCase(ch))
                    node = tnode::SETCMD;   //nedodelano
                else {
                    inschr('@');
                    if (mxHandler.is())
                        mxHandler->characters(OUString::createFromAscii(cmd602));
                    node = tnode::READCH;
                }
            } else {
                inschr('@');
                if(ch<32) node = tnode::SETCH;
                else node = tnode::WRITE;}
            break;

        case tnode::SETCMD:
            ch = Setformat602(cmd602);
            if(ch == 0) node = tnode::EEND;
            else if(ch == '\n') node = tnode::EOL;
            else node = tnode::READCH;
            break;

        case tnode::SETCH :
            // warning: potentially uninitialized
            switch(ch) {
            case '\t' : Start_(u"text:tab-stop"_ustr);
                    End_(u"text:tab-stop"_ustr);
                    break;
            case 0x03 : break; //condensed
            case 0x01 : break; //elite
            case 0x11 : break; //uzv1
            case 0x12 : break; //uzv2
            case 0x15 : break; //uzv3
            case 0x17 : break; //uzv4
            case 0x18 : break; //uzv5
            case 0x19 : break; //uzv6
            case 0x13 : fst.uline = !fst.uline;break;
            case 0x02 : setfnt(fat,false);     break;
            case 0x04 : setfnt(cursive,false); break;
            case 0x0f : setfnt(bold,false);    break;
            case 0x10 : setfnt(tall,false);    break;
            case 0x1d : setfnt(big,false);     break;
            case 0x14 : setfnt(hindex,false);  break;
            case 0x16 : setfnt(lindex,false);  break;
            default   : break;
            }
            if(pst.ccafterln) node = tnode::EOL;
            else node = tnode::READCH;
            break;

        case tnode::WRITE :
            switch(ch) {
            case 0x8d:
                ch = Readchar602();
                if( ch == 0x0a) {
                    if(ini.reformatpars) inschr(' ');
                    else par602(false); //formatovaci radek
                    node = tnode::EOL;
                } else {
                    inschr(0x8d);//inschr(' ');
                    if(ch == 0) node = tnode::EEND;
                    else if(ch < 32) node = tnode::SETCH;
                    else node = tnode::WRITE;
                }
                break;
            case 0xfe:
                if (ini.showcomm||!pst.comment) inschr(' ');
                node = tnode::READCH;
                break;
            case 0xad:
                ch = Readchar602();
                if( ch==0x8d ) {
                    if(ini.showcomm||!pst.comment) { //formatovaci pomlcka
                        if(!ini.reformatpars)
                            inschr('-');
                        else {
                            pst.wasspace--;
                            pst.wasfdash = true;
                        }
                    }
                    node=tnode::WRITE;
                } else {
                    inschr(0xad);
                    if(ch == 0) node = tnode::EEND;
                    else if(ch == '\n') {
                        if(!pst.willbeeop) par602(false);
                        node = tnode::EOL; }
                    else if(ch < 32) node = tnode::SETCH;
                    else node = tnode::WRITE;
                }
                break;
            default:
                inschr(ch);
                node = tnode::READCH;
                break;
            }
            break;
        default: break;
        }
    }

    End_(u"text:span"_ustr);
    End_(u"text:p"_ustr);
    node = tnode::QUIT;
}

// XServiceInfo
OUString SAL_CALL T602ImportFilter::getImplementationName(  )
{
    return u"com.sun.star.comp.Writer.T602ImportFilter"_ustr;
}

sal_Bool SAL_CALL T602ImportFilter::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}

Sequence< OUString > SAL_CALL T602ImportFilter::getSupportedServiceNames(  )
{
    return { u"com.sun.star.document.ImportFilter"_ustr, u"com.sun.star.document.ExtendedTypeDetection"_ustr };
}

T602ImportFilterDialog::T602ImportFilterDialog()
{
}

T602ImportFilterDialog::~T602ImportFilterDialog()
{
}

bool T602ImportFilterDialog::OptionsDlg()
{
    Any any;
#define propInt_(_prop,_nam,_val) \
    any <<= static_cast<sal_Int32>(_val);\
    _prop->setPropertyValue(_nam, any);
#define propShort_(_prop,_nam,_val) \
    any <<= static_cast<sal_Int16>(_val);\
    _prop->setPropertyValue(_nam, any);
#define propBool_(_prop,_nam,_val) \
    any <<= _val;\
    _prop->setPropertyValue(_nam, any);
#define propString_(_prop,_nam,_val) \
    any <<= _val;\
    _prop->setPropertyValue(_nam, any);
#define propStringFromResId_(_prop,_nam,_val) \
    any <<= FilterResId(_val);\
    _prop->setPropertyValue(_nam, any);
#define propGet_(_prop,_nam) \
    _prop->getPropertyValue(_nam);
#define Inst_(_path)\
    xMultiServiceFactory->createInstance(_path);
#define Insert_(_cont,_nam,_obj) \
    any <<= _obj;\
    _cont->insertByName( _nam, any );

    Reference < XComponentContext > rComponentContext = defaultBootstrap_InitialComponentContext();
    Reference < XMultiComponentFactory > rServiceManager = rComponentContext->getServiceManager();
    Reference < XInterface > rInstance = rServiceManager->createInstanceWithContext(u"com.sun.star.awt.UnoControlDialogModel"_ustr, rComponentContext );

    Reference <XMultiServiceFactory> xMultiServiceFactory (rInstance,UNO_QUERY);

    Reference < XPropertySet > xPSetDialog( rInstance, UNO_QUERY );
    propInt_(xPSetDialog,u"PositionX"_ustr,100);
    propInt_(xPSetDialog,u"PositionY"_ustr,100);
    propInt_(xPSetDialog,u"Width"_ustr,130);
    propInt_(xPSetDialog,u"Height"_ustr,90);
    propStringFromResId_(xPSetDialog,u"Title"_ustr, T602FILTER_STR_IMPORT_DIALOG_TITLE);

    static constexpr OUString T602DLG_OK_BUTTON = u"ok_button"_ustr;
    static constexpr OUString T602DLG_CANCEL_BUTTON = u"cancel_button"_ustr;
    static constexpr OUString T602DLG_CODE_LB = u"code_lb"_ustr;
    static constexpr OUString T602DLG_AZBUKA_CB = u"azbuka_cb"_ustr;
    static constexpr OUString T602DLG_COMMENT_CB = u"comment_cb"_ustr;
    static constexpr OUString T602DLG_REFORMAT_CB = u"reformat_cb"_ustr;
    static constexpr OUString T602DLG_CODE_TXT = u"code_txt"_ustr;

    Reference < XInterface > TextModel = Inst_(u"com.sun.star.awt.UnoControlFixedTextModel"_ustr);
    Reference < XPropertySet > xPSetText( TextModel, UNO_QUERY );
    propInt_(xPSetText,u"PositionX"_ustr,10);
    propInt_(xPSetText,u"PositionY"_ustr,8);
    propInt_(xPSetText,u"Width"_ustr,30);
    propInt_(xPSetText,u"Height"_ustr,14);
    propString_(xPSetText,u"Name"_ustr,T602DLG_CODE_TXT);
    propStringFromResId_(xPSetText,u"Label"_ustr,T602FILTER_STR_ENCODING_LABEL);

    Reference < XInterface > ListBoxModel = Inst_(u"com.sun.star.awt.UnoControlListBoxModel"_ustr);
    Reference < XPropertySet > xPSetCodeLB( ListBoxModel, UNO_QUERY );
    propInt_(xPSetCodeLB,u"PositionX"_ustr,40);
    propInt_(xPSetCodeLB,u"PositionY"_ustr,5);
    propInt_(xPSetCodeLB,u"Width"_ustr,85);
    propInt_(xPSetCodeLB,u"Height"_ustr,14);
    propString_(xPSetCodeLB,u"Name"_ustr,T602DLG_CODE_LB);
    propShort_(xPSetCodeLB,u"TabIndex"_ustr,1);
    propBool_(xPSetCodeLB,u"Dropdown"_ustr,true);
    propBool_(xPSetCodeLB,u"MultiSelection"_ustr,false);

    Sequence< OUString > ous
    {
        FilterResId(T602FILTER_STR_ENCODING_AUTO),
        FilterResId(T602FILTER_STR_ENCODING_CP852),
        FilterResId(T602FILTER_STR_ENCODING_CP895),
        FilterResId(T602FILTER_STR_ENCODING_KOI8CS2)
    };
    any <<= ous;
    xPSetCodeLB->setPropertyValue(u"StringItemList"_ustr, any);

    Sequence < sal_Int16 > shr{ static_cast<sal_Int16>(ini.forcecode ? ini.xcode + 1 : 0) };
    any <<= shr;
    xPSetCodeLB->setPropertyValue(u"SelectedItems"_ustr, any);

    Reference < XInterface > AzbCheckBoxModel = Inst_(u"com.sun.star.awt.UnoControlCheckBoxModel"_ustr);
    Reference < XPropertySet > xPSetAzbukaCB( AzbCheckBoxModel, UNO_QUERY );
    propInt_(xPSetAzbukaCB,u"PositionX"_ustr,10);
    propInt_(xPSetAzbukaCB,u"PositionY"_ustr,25);
    propInt_(xPSetAzbukaCB,u"Width"_ustr,100);
    propInt_(xPSetAzbukaCB,u"Height"_ustr,14);
    propString_(xPSetAzbukaCB,u"Name"_ustr,T602DLG_AZBUKA_CB);
    propShort_(xPSetAzbukaCB,u"TabIndex"_ustr,2);
    propStringFromResId_(xPSetAzbukaCB,u"Label"_ustr,T602FILTER_STR_CYRILLIC_MODE);
    propShort_(xPSetAzbukaCB,u"State"_ustr,ini.ruscode);

    Reference < XInterface > RefCheckBoxModel = Inst_(u"com.sun.star.awt.UnoControlCheckBoxModel"_ustr);
    Reference < XPropertySet > xPSetRefCB( RefCheckBoxModel, UNO_QUERY );
    propInt_(xPSetRefCB,u"PositionX"_ustr,10);
    propInt_(xPSetRefCB,u"PositionY"_ustr,40);
    propInt_(xPSetRefCB,u"Width"_ustr,100);
    propInt_(xPSetRefCB,u"Height"_ustr,14);
    propString_(xPSetRefCB,u"Name"_ustr,T602DLG_REFORMAT_CB);
    propShort_(xPSetRefCB,u"TabIndex"_ustr,3);
    propStringFromResId_(xPSetRefCB,u"Label"_ustr,T602FILTER_STR_REFORMAT_TEXT);
    propShort_(xPSetRefCB,u"State"_ustr,ini.reformatpars);

    Reference < XInterface > CommCheckBoxModel = Inst_(u"com.sun.star.awt.UnoControlCheckBoxModel"_ustr);
    Reference < XPropertySet > xPSetCommCB( CommCheckBoxModel, UNO_QUERY );
    propInt_(xPSetCommCB,u"PositionX"_ustr,10);
    propInt_(xPSetCommCB,u"PositionY"_ustr,55);
    propInt_(xPSetCommCB,u"Width"_ustr,100);
    propInt_(xPSetCommCB,u"Height"_ustr,14);
    propString_(xPSetCommCB,u"Name"_ustr,T602DLG_COMMENT_CB);
    propShort_(xPSetCommCB,u"TabIndex"_ustr,4);
    propStringFromResId_(xPSetCommCB,u"Label"_ustr,T602FILTER_STR_DOT_COMMANDS);
    propShort_(xPSetCommCB,u"State"_ustr,ini.showcomm);

    Reference < XInterface > CancelButtonModel = Inst_(u"com.sun.star.awt.UnoControlButtonModel"_ustr);
    Reference < XPropertySet > xPSetCancelButton( CancelButtonModel, UNO_QUERY );
    propInt_(xPSetCancelButton,u"PositionX"_ustr,10);
    propInt_(xPSetCancelButton,u"PositionY"_ustr,70);
    propInt_(xPSetCancelButton,u"Width"_ustr,50);
    propInt_(xPSetCancelButton,u"Height"_ustr,14);
    propString_(xPSetCancelButton,u"Name"_ustr,T602DLG_CANCEL_BUTTON);
    propShort_(xPSetCancelButton,u"TabIndex"_ustr,5);
    propShort_(xPSetCancelButton,u"PushButtonType"_ustr,2);
    propStringFromResId_(xPSetCancelButton,u"Label"_ustr,T602FILTER_STR_CANCEL_BUTTON);

    Reference < XInterface > OkButtonModel = Inst_(u"com.sun.star.awt.UnoControlButtonModel"_ustr);
    Reference < XPropertySet > xPSetOkButton( OkButtonModel, UNO_QUERY );
    propInt_(xPSetOkButton,u"PositionX"_ustr,70);
    propInt_(xPSetOkButton,u"PositionY"_ustr,70);
    propInt_(xPSetOkButton,u"Width"_ustr,50);
    propInt_(xPSetOkButton,u"Height"_ustr,14);
    propString_(xPSetOkButton,u"Name"_ustr,T602DLG_OK_BUTTON);
    propShort_(xPSetOkButton,u"TabIndex"_ustr,0);
    propShort_(xPSetOkButton,u"PushButtonType"_ustr,1);
    propStringFromResId_(xPSetOkButton,u"Label"_ustr,T602FILTER_STR_OK_BUTTON);
    propBool_(xPSetOkButton,u"DefaultButton"_ustr,true);

    Reference < XNameContainer > xNameCont (rInstance,UNO_QUERY);

    Insert_(xNameCont, T602DLG_OK_BUTTON, OkButtonModel);
    Insert_(xNameCont, T602DLG_CANCEL_BUTTON, CancelButtonModel);
    Insert_(xNameCont, T602DLG_AZBUKA_CB, AzbCheckBoxModel);
    Insert_(xNameCont, T602DLG_REFORMAT_CB, RefCheckBoxModel);
    Insert_(xNameCont, T602DLG_COMMENT_CB, CommCheckBoxModel);
    Insert_(xNameCont, T602DLG_CODE_LB, ListBoxModel);
    Insert_(xNameCont, T602DLG_CODE_TXT, TextModel);

    Reference< XUnoControlDialog > dialog = UnoControlDialog::create(rComponentContext);

    Reference < XControlModel > xControlModel (rInstance,UNO_QUERY);

    dialog->setModel( xControlModel );

    Reference < XToolkit > xToolkit = Toolkit::create( rComponentContext );

    dialog->setVisible( false );
    dialog->createPeer( xToolkit, nullptr );

    bool ret = ( dialog->execute() != 0 );
    if ( ret ) {

        sal_Int16 tt = 0;

        any = propGet_(xPSetAzbukaCB, u"State"_ustr); any >>= tt; ini.ruscode      = tt;
        any = propGet_(xPSetRefCB,    u"State"_ustr); any >>= tt; ini.reformatpars = tt;
        any = propGet_(xPSetCommCB,   u"State"_ustr); any >>= tt; ini.showcomm     = tt;

        any = propGet_(xPSetCodeLB,   u"SelectedItems"_ustr); any >>= shr;

        if( shr[0] > 0 ) {
            ini.xcode = static_cast<tcode>(shr[0]-1);
            ini.forcecode = true;
        } else {
            ini.forcecode = false;
        }
    }

    Reference<XControl>(dialog)->dispose();

    return ret;
}

void SAL_CALL T602ImportFilterDialog::setTitle( const OUString& )
{
}

sal_Int16 SAL_CALL T602ImportFilterDialog::execute()
{
    if (OptionsDlg())
        return css::ui::dialogs::ExecutableDialogResults::OK;
    else
        return css::ui::dialogs::ExecutableDialogResults::CANCEL;
}

uno::Sequence<beans::PropertyValue> SAL_CALL T602ImportFilterDialog::getPropertyValues()
{
    return uno::Sequence<beans::PropertyValue>();
}

void SAL_CALL T602ImportFilterDialog::setPropertyValues( const uno::Sequence<beans::PropertyValue>& )
{
}


// XServiceInfo
OUString SAL_CALL T602ImportFilterDialog::getImplementationName(  )
{
    return u"com.sun.star.comp.Writer.T602ImportFilterDialog"_ustr;
}

sal_Bool SAL_CALL T602ImportFilterDialog::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}

Sequence< OUString > SAL_CALL T602ImportFilterDialog::getSupportedServiceNames(  )
{
    return { u"com.sun.star.ui.dialogs.FilterOptionsDialog"_ustr };
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
filter_T602ImportFilterDialog_get_implementation(
    css::uno::XComponentContext* , css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new T602ImportFilter::T602ImportFilterDialog());
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
filter_T602ImportFilter_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new T602ImportFilter::T602ImportFilter(context));
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImport602(SvStream &rStream)
{
    css::uno::Reference<io::XInputStream> xStream(new utl::OSeekableInputStreamWrapper(rStream));
    rtl::Reference<T602ImportFilter::T602ImportFilter> aImport(
        new T602ImportFilter::T602ImportFilter(xStream));
    aImport->test();
    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
