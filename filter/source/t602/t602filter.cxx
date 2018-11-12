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

#include <stdio.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/bootstrap.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <com/sun/star/awt/Toolkit.hpp>
#include <com/sun/star/bridge/XUnoUrlResolver.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/ucb/XSimpleFileAccess.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/container/XNameContainer.hpp>
#include <com/sun/star/awt/UnoControlDialog.hpp>
#include <com/sun/star/awt/XControl.hpp>
#include <com/sun/star/awt/XDialog.hpp>
#include <com/sun/star/ui/dialogs/ExecutableDialogResults.hpp>
#include <osl/diagnose.h>
#include <rtl/ref.hxx>
#include <rtl/character.hxx>
#include <unotools/resmgr.hxx>
#include <unotools/streamwrap.hxx>

using namespace ::cppu;
using namespace ::osl;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::registry;
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

T602ImportFilter::T602ImportFilter(const css::uno::Reference<css::lang::XMultiServiceFactory > &r )
    : mxMSF(r)
    , mpAttrList(nullptr)
    , node(tnode::START)
{
}

T602ImportFilter::T602ImportFilter(css::uno::Reference<css::io::XInputStream> const & xInputStream)
    : mxInputStream(xInputStream)
    , mpAttrList(nullptr)
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
    size_t numBytesRead = 0;

    numBytesRead = mxInputStream->readSomeBytes (aData, numBytes);

    if ((numBytesRead != numBytes) || (aData[0] != '@') ||
        (aData[1] != 'C') || (aData[2] != 'T') || (aData[3] != ' '))
        return OUString();

    return OUString(  "writer_T602_Document"  );
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
            char s[20];
            sprintf(s,"%i",pst.wasspace);
            if (mpAttrList)
                mpAttrList->AddAttribute("text:c",OUString::createFromAscii(s));
            Start_("text:s");
            End_("text:s");
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

    // An XML import service: what we push sax messages to..
    mxHandler.set( mxMSF->createInstance( "com.sun.star.comp.Writer.XMLImporter" ), UNO_QUERY );

    // The XImporter sets up an empty target document for XDocumentHandler to write to..
    Reference < XImporter > xImporter(mxHandler, UNO_QUERY);
    xImporter->setTargetDocument(mxDoc);

    char fs[32], fs2[32];
    sprintf(fs, "%ipt", inistruct::fontsize);
    sprintf(fs2,"%ipt", 2*inistruct::fontsize);

    mpAttrList = new SvXMLAttributeList;

    Reference < XAttributeList > xAttrList ( mpAttrList );

    mxHandler->startDocument();

    mpAttrList->AddAttribute("xmlns:office", "http://openoffice.org/2000/office");
    mpAttrList->AddAttribute("xmlns:style", "http://openoffice.org/2000/style");
    mpAttrList->AddAttribute("xmlns:text", "http://openoffice.org/2000/text");
    mpAttrList->AddAttribute("xmlns:table", "http://openoffice.org/2000/table");
    mpAttrList->AddAttribute("xmlns:draw", "http://openoffice.org/2000/draw");
    mpAttrList->AddAttribute("xmlns:fo", "http://www.w3.org/1999/XSL/Format");
    mpAttrList->AddAttribute("xmlns:xlink", "http://www.w3.org/1999/xlink");
    mpAttrList->AddAttribute("xmlns:number", "http://openoffice.org/2000/datastyle");
    mpAttrList->AddAttribute("xmlns:svg", "http://www.w3.org/2000/svg");
    mpAttrList->AddAttribute("xmlns:chart", "http://openoffice.org/2000/chart");
    mpAttrList->AddAttribute("xmlns:dr3d", "http://openoffice.org/2000/dr3d");
    mpAttrList->AddAttribute("xmlns:math", "http://www.w3.org/1998/Math/MathML");
    mpAttrList->AddAttribute("xmlns:form", "http://openoffice.org/2000/form");
    mpAttrList->AddAttribute("xmlns:script", "http://openoffice.org/2000/script");
    mpAttrList->AddAttribute("office:class", "text");
    mpAttrList->AddAttribute("office:version", "1.0");
    Start_("office:document-content");

    Start_("office:font-decls");
    mpAttrList->AddAttribute("style:name","Courier New");
    mpAttrList->AddAttribute("fo:font-family","Courier New");
    mpAttrList->AddAttribute("style:font-pitch","fixed");
    Start_("style:font-decl");
    End_("style:font-decl");
    End_("office:font-decls");

    Start_("office:automatic-styles");

    // Standardni text
    mpAttrList->AddAttribute("style:name","P1");
    mpAttrList->AddAttribute("style:family","paragraph");
    mpAttrList->AddAttribute("style:parent-style-name","Standard");
    Start_("style:style");
    mpAttrList->AddAttribute("style:font-name","Courier New");
    mpAttrList->AddAttribute("fo:font-size",OUString::createFromAscii(fs));
    Start_("style:properties");
    End_("style:properties");
    End_("style:style");

    // Standardni text - konec stranky
    mpAttrList->AddAttribute("style:name","P2");
    mpAttrList->AddAttribute("style:family","paragraph");
    mpAttrList->AddAttribute("style:parent-style-name","Standard");
    Start_("style:style");
    mpAttrList->AddAttribute("style:font-name","Courier New");
    mpAttrList->AddAttribute("fo:font-size",OUString::createFromAscii(fs));
    mpAttrList->AddAttribute("fo:break-before","page");
    Start_("style:properties");
    End_("style:properties");
    End_("style:style");

    // T1 Normalni text
    mpAttrList->AddAttribute("style:name","T1");
    mpAttrList->AddAttribute("style:family","text");
    Start_("style:style");
    Start_("style:properties");
    End_("style:properties");
    End_("style:style");

    // T2 Tucny text
    mpAttrList->AddAttribute("style:name","T2");
    mpAttrList->AddAttribute("style:family","text");
    Start_("style:style");
    mpAttrList->AddAttribute("fo:font-weight","bold");
    Start_("style:properties");
    End_("style:properties");
    End_("style:style");

    // T3 Kurziva
    mpAttrList->AddAttribute("style:name","T3");
    mpAttrList->AddAttribute("style:family","text");
    Start_("style:style");
    mpAttrList->AddAttribute("fo:font-style","italic");
    Start_("style:properties");
    End_("style:properties");
    End_("style:style");

    // T4 Siroky text
    mpAttrList->AddAttribute("style:name","T4");
    mpAttrList->AddAttribute("style:family","text");
    Start_("style:style");
    mpAttrList->AddAttribute("fo:font-weight","bold");
    mpAttrList->AddAttribute("style:text-scale","200%");
    Start_("style:properties");
    End_("style:properties");
    End_("style:style");

    // T5 Vysoky text
    mpAttrList->AddAttribute("style:name","T5");
    mpAttrList->AddAttribute("style:family","text");
    Start_("style:style");
    mpAttrList->AddAttribute("fo:font-size",OUString::createFromAscii(fs2));
    mpAttrList->AddAttribute("fo:font-weight","bold");
    mpAttrList->AddAttribute("style:text-scale","50%");
    Start_("style:properties");
    End_("style:properties");
    End_("style:style");

    // T6 Velky text
    mpAttrList->AddAttribute("style:name","T6");
    mpAttrList->AddAttribute("style:family","text");
    Start_("style:style");
    mpAttrList->AddAttribute("fo:font-size",OUString::createFromAscii(fs2));
    mpAttrList->AddAttribute("fo:font-weight","bold");
    Start_("style:properties");
    End_("style:properties");
    End_("style:style");

    // T7 Podtrzeny text
    mpAttrList->AddAttribute("style:name","T7");
    mpAttrList->AddAttribute("style:family","text");
    Start_("style:style");
    mpAttrList->AddAttribute("style:text-underline","single");
    Start_("style:properties");
    End_("style:properties");
    End_("style:style");

    // T8 Podtrzena tucny text
    mpAttrList->AddAttribute("style:name","T8");
    mpAttrList->AddAttribute("style:family","text");
    Start_("style:style");
    mpAttrList->AddAttribute("fo:font-weight","bold");
    mpAttrList->AddAttribute("style:text-underline","single");
    Start_("style:properties");
    End_("style:properties");
    End_("style:style");

    // T9 Podtrzena kurziva
    mpAttrList->AddAttribute("style:name","T9");
    mpAttrList->AddAttribute("style:family","text");
    Start_("style:style");
    mpAttrList->AddAttribute("fo:font-style","italic");
    mpAttrList->AddAttribute("style:text-underline","single");
    Start_("style:properties");
    End_("style:properties");
    End_("style:style");

    // T10 Horni index
    mpAttrList->AddAttribute("style:name","T10");
    mpAttrList->AddAttribute("style:family","text");
    Start_("style:style");
    mpAttrList->AddAttribute("style:text-position","27% 100%");
    Start_("style:properties");
    End_("style:properties");
    End_("style:style");

    // T11 Dolni index
    mpAttrList->AddAttribute("style:name","T11");
    mpAttrList->AddAttribute("style:family","text");
    Start_("style:style");
    mpAttrList->AddAttribute("style:text-position","-27% 100%");
    Start_("style:properties");
    End_("style:properties");
    End_("style:style");

    End_("office:automatic-styles");

    Start_("office:styles");
    End_("office:styles");

    Start_("office:body");

    Read602();

    End_("office:body");
    End_("office:document-content");

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
    static sal_Unicode xch[2];

    xch[1] = 0;

    if(ch > 127) {

        switch(ini.xcode) {
            case L2 : break;
            case KOI: ch = koi2lat[ch-128]; break;
            case KAM: ch = kam2lat[ch-128]; break;
            default : ch = kam2lat[ch-128];
        }

        if(ini.ruscode)
            xch[0] = (rus2UNC[(ch-128)*2] << 8) + rus2UNC[(ch-128)*2+1];
        else
            xch[0] = (lat2UNC[(ch-128)*2] << 8) + lat2UNC[(ch-128)*2+1];
    } else
        xch[0] = ch;

    pst.waspar = false;
    if (mxHandler.is())
        mxHandler->characters(xch);
}

void T602ImportFilter::wrtfnt()
{
    Reference < XAttributeList > xAttrList ( mpAttrList );
    const sal_Char *style = "P1";

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

    End_("text:span");
    if (mpAttrList)
        mpAttrList->AddAttribute(
            "text:style-name", OUString::createFromAscii(style));
    Start_("text:span");
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
                End_("text:span");
                End_("text:p");
                if (mpAttrList)
                    mpAttrList->AddAttribute("text:style-name", "P1");
                Start_("text:p");
                Start_("text:span");
                wrtfnt();
            } else {
                Start_("text:line-break");
                End_("text:line-break");
            }
        }

        pst.waspar = true;
    }

    if(!pst.comment) {
        switch(format602.lh) {
            case 3: pst.pars += 4; break;
            case 4: pst.pars += 3; break;
            case 6: pst.pars += 2; break;
            default: pst.pars +=2; break;
        }
        endp = format602.pl-format602.mt-format602.mb;
        if(((pst.pars+1)/2) >= endp || endofpage) {
            pst.pars = 0;
            if(!ini.reformatpars) {
                End_("text:span");
                End_("text:p");
                if (mpAttrList)
                    mpAttrList->AddAttribute("text:style-name", "P2");
                Start_("text:p");
                Start_("text:span");
                wrtfnt();
            }
        }
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
    sal_Int16 i;

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
        i = readnum(&ch,false);
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
        mpAttrList->AddAttribute("text:style-name", "P1");
    Start_("text:p");
    if (mpAttrList)
        mpAttrList->AddAttribute("text:style-name", "T1");
    Start_("text:span");

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
            case '\t' : Start_("text:tab-stop");
                    End_("text:tab-stop");
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

    End_("text:span");
    End_("text:p");
    node = tnode::QUIT;
}

// XServiceInfo
OUString SAL_CALL T602ImportFilter::getImplementationName(  )
{
    return T602ImportFilter_getImplementationName();
}

sal_Bool SAL_CALL T602ImportFilter::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}

Sequence< OUString > SAL_CALL T602ImportFilter::getSupportedServiceNames(  )
{
    return T602ImportFilter_getSupportedServiceNames();
}

OUString T602ImportFilter_getImplementationName ()
{
    return OUString ( "com.sun.star.comp.Writer.T602ImportFilter" );
}

Sequence< OUString > T602ImportFilter_getSupportedServiceNames(  )
{
    Sequence < OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] =  "com.sun.star.document.ImportFilter";
    pArray[1] =  "com.sun.star.document.ExtendedTypeDetection";
    return aRet;
}

Reference< XInterface > T602ImportFilter_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
{
    return static_cast<cppu::OWeakObject*>(new T602ImportFilter( rSMgr ));
}

T602ImportFilterDialog::T602ImportFilterDialog()
    : maLocale(SvtSysLocale().GetUILanguageTag())
    , maResLocale(Translate::Create("flt"))
{
}

T602ImportFilterDialog::~T602ImportFilterDialog()
{
}

// XLocalizable

void SAL_CALL T602ImportFilterDialog::setLocale(const Locale& rLocale)
{
    LanguageTag aLocale(rLocale);
    if (maLocale != aLocale)
    {
        maLocale = aLocale;
        maResLocale = Translate::Create("flt", maLocale);
    }
}

Locale SAL_CALL T602ImportFilterDialog::getLocale()
{
    return maLocale.getLocale(false);
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
    any <<= OUString(_val);\
    _prop->setPropertyValue(_nam, any);
#define propStringFromResId_(_prop,_nam,_val) \
    any <<= getResStr(_val);\
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
    Reference < XInterface > rInstance = rServiceManager->createInstanceWithContext("com.sun.star.awt.UnoControlDialogModel", rComponentContext );

    Reference <XMultiServiceFactory> xMultiServiceFactory (rInstance,UNO_QUERY);

    Reference < XPropertySet > xPSetDialog( rInstance, UNO_QUERY );
    propInt_(xPSetDialog,"PositionX",100);
    propInt_(xPSetDialog,"PositionY",100);
    propInt_(xPSetDialog,"Width",130);
    propInt_(xPSetDialog,"Height",90);
    propStringFromResId_(xPSetDialog,"Title", T602FILTER_STR_IMPORT_DIALOG_TITLE);

#define T602DLG_OK_BUTTON    "ok_button"
#define T602DLG_CANCEL_BUTTON    "cancel_button"
#define T602DLG_CODE_LB        "code_lb"
#define T602DLG_AZBUKA_CB    "azbuka_cb"
#define T602DLG_COMMENT_CB    "comment_cb"
#define T602DLG_REFORMAT_CB    "reformat_cb"
#define T602DLG_CODE_TXT    "code_txt"

    Reference < XInterface > TextModel = Inst_("com.sun.star.awt.UnoControlFixedTextModel");
    Reference < XPropertySet > xPSetText( TextModel, UNO_QUERY );
    propInt_(xPSetText,"PositionX",10);
    propInt_(xPSetText,"PositionY",8);
    propInt_(xPSetText,"Width",30);
    propInt_(xPSetText,"Height",14);
    propString_(xPSetText,"Name",T602DLG_CODE_TXT);
    propStringFromResId_(xPSetText,"Label",T602FILTER_STR_ENCODING_LABEL);

    Reference < XInterface > ListBoxModel = Inst_("com.sun.star.awt.UnoControlListBoxModel");
    Reference < XPropertySet > xPSetCodeLB( ListBoxModel, UNO_QUERY );
    propInt_(xPSetCodeLB,"PositionX",40);
    propInt_(xPSetCodeLB,"PositionY",5);
    propInt_(xPSetCodeLB,"Width",85);
    propInt_(xPSetCodeLB,"Height",14);
    propString_(xPSetCodeLB,"Name",T602DLG_CODE_LB);
    propShort_(xPSetCodeLB,"TabIndex",1);
    propBool_(xPSetCodeLB,"Dropdown",true);
    propBool_(xPSetCodeLB,"MultiSelection",false);

    Sequence< OUString > ous(4);
    ous[0] = getResStr(T602FILTER_STR_ENCODING_AUTO);
    ous[1] = getResStr(T602FILTER_STR_ENCODING_CP852);
    ous[2] = getResStr(T602FILTER_STR_ENCODING_CP895);
    ous[3] = getResStr(T602FILTER_STR_ENCODING_KOI8CS2);
    any <<= ous;
    xPSetCodeLB->setPropertyValue("StringItemList", any);

    Sequence < sal_Int16 > shr(1);
    shr[0] = ini.forcecode ? static_cast<sal_Int16>(ini.xcode) + 1 : 0;
    any <<= shr;
    xPSetCodeLB->setPropertyValue("SelectedItems", any);

    Reference < XInterface > AzbCheckBoxModel = Inst_("com.sun.star.awt.UnoControlCheckBoxModel");
    Reference < XPropertySet > xPSetAzbukaCB( AzbCheckBoxModel, UNO_QUERY );
    propInt_(xPSetAzbukaCB,"PositionX",10);
    propInt_(xPSetAzbukaCB,"PositionY",25);
    propInt_(xPSetAzbukaCB,"Width",100);
    propInt_(xPSetAzbukaCB,"Height",14);
    propString_(xPSetAzbukaCB,"Name",T602DLG_AZBUKA_CB);
    propShort_(xPSetAzbukaCB,"TabIndex",2);
    propStringFromResId_(xPSetAzbukaCB,"Label",T602FILTER_STR_CYRILLIC_MODE);
    propShort_(xPSetAzbukaCB,"State",ini.ruscode);

    Reference < XInterface > RefCheckBoxModel = Inst_("com.sun.star.awt.UnoControlCheckBoxModel");
    Reference < XPropertySet > xPSetRefCB( RefCheckBoxModel, UNO_QUERY );
    propInt_(xPSetRefCB,"PositionX",10);
    propInt_(xPSetRefCB,"PositionY",40);
    propInt_(xPSetRefCB,"Width",100);
    propInt_(xPSetRefCB,"Height",14);
    propString_(xPSetRefCB,"Name",T602DLG_REFORMAT_CB);
    propShort_(xPSetRefCB,"TabIndex",3);
    propStringFromResId_(xPSetRefCB,"Label",T602FILTER_STR_REFORMAT_TEXT);
    propShort_(xPSetRefCB,"State",ini.reformatpars);

    Reference < XInterface > CommCheckBoxModel = Inst_("com.sun.star.awt.UnoControlCheckBoxModel");
    Reference < XPropertySet > xPSetCommCB( CommCheckBoxModel, UNO_QUERY );
    propInt_(xPSetCommCB,"PositionX",10);
    propInt_(xPSetCommCB,"PositionY",55);
    propInt_(xPSetCommCB,"Width",100);
    propInt_(xPSetCommCB,"Height",14);
    propString_(xPSetCommCB,"Name",T602DLG_COMMENT_CB);
    propShort_(xPSetCommCB,"TabIndex",4);
    propStringFromResId_(xPSetCommCB,"Label",T602FILTER_STR_DOT_COMMANDS);
    propShort_(xPSetCommCB,"State",ini.showcomm);

    Reference < XInterface > CancelButtonModel = Inst_("com.sun.star.awt.UnoControlButtonModel");
    Reference < XPropertySet > xPSetCancelButton( CancelButtonModel, UNO_QUERY );
    propInt_(xPSetCancelButton,"PositionX",10);
    propInt_(xPSetCancelButton,"PositionY",70);
    propInt_(xPSetCancelButton,"Width",50);
    propInt_(xPSetCancelButton,"Height",14);
    propString_(xPSetCancelButton,"Name",T602DLG_CANCEL_BUTTON);
    propShort_(xPSetCancelButton,"TabIndex",5);
    propShort_(xPSetCancelButton,"PushButtonType",2);
    propStringFromResId_(xPSetCancelButton,"Label",T602FILTER_STR_CANCEL_BUTTON);

    Reference < XInterface > OkButtonModel = Inst_("com.sun.star.awt.UnoControlButtonModel");
    Reference < XPropertySet > xPSetOkButton( OkButtonModel, UNO_QUERY );
    propInt_(xPSetOkButton,"PositionX",70);
    propInt_(xPSetOkButton,"PositionY",70);
    propInt_(xPSetOkButton,"Width",50);
    propInt_(xPSetOkButton,"Height",14);
    propString_(xPSetOkButton,"Name",T602DLG_OK_BUTTON);
    propShort_(xPSetOkButton,"TabIndex",0);
    propShort_(xPSetOkButton,"PushButtonType",1);
    propStringFromResId_(xPSetOkButton,"Label",T602FILTER_STR_OK_BUTTON);
    propBool_(xPSetOkButton,"DefaultButton",true);

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

        any = propGet_(xPSetAzbukaCB, "State"); any >>= tt; ini.ruscode      = tt;
        any = propGet_(xPSetRefCB,    "State"); any >>= tt; ini.reformatpars = tt;
        any = propGet_(xPSetCommCB,   "State"); any >>= tt; ini.showcomm     = tt;

        any = propGet_(xPSetCodeLB,   "SelectedItems"); any >>= shr;

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

OUString T602ImportFilterDialog::getResStr(const char* resid)
{
    return Translate::get(resid, maResLocale);
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
    return T602ImportFilterDialog_getImplementationName();
}

sal_Bool SAL_CALL T602ImportFilterDialog::supportsService( const OUString& rServiceName )
{
    return cppu::supportsService( this, rServiceName );
}

Sequence< OUString > SAL_CALL T602ImportFilterDialog::getSupportedServiceNames(  )
{
    return T602ImportFilterDialog_getSupportedServiceNames();
}

OUString T602ImportFilterDialog_getImplementationName ()
{
    return OUString ( "com.sun.star.comp.Writer.T602ImportFilterDialog" );
}

Sequence< OUString > T602ImportFilterDialog_getSupportedServiceNames(  )
{
    Sequence<OUString> aRet { "com.sun.star.ui.dialogs.FilterOptionsDialog" };
    return aRet;
}

Reference< XInterface > T602ImportFilterDialog_createInstance( const Reference< XMultiServiceFactory > & )
{
    return static_cast<cppu::OWeakObject*>(new T602ImportFilterDialog);
}

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
