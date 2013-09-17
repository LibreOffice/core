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
#include "t602filter.hrc"

#include <ctype.h>
#include <stdio.h>

#include <cppuhelper/factory.hxx>
#include <cppuhelper/bootstrap.hxx>
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

using namespace ::rtl;
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

    unsigned char kam2lat[129] =
        //    0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
        "\xAC\x81\x82\xD4\x84\xD2\x9B\x9F\xD8\xB7\x91\xD6\x96\x92\x8E\xB5"  // 8
        "\x90\xA7\xA6\x93\x94\xE0\x85\xE9\xEC\x99\x9A\xE6\x95\xED\xFC\x9C"  // 9
        "\xA0\xA1\xA2\xA3\xE5\xD5\xDE\xE2\xE7\xFD\xEA\xE8\xCE\xF5\xAE\xAF"  // A
        "\x80\x80\x83\xB3\xB4\xE1\x87\x88\x89\x8A\x8B\x8C\x8F\x97\x8D\xBF"  // B
        "\xC0\xC1\xC2\xC3\xC4\xC5\xCF\x98\x9D\x9E\xA4\xA5\xA8\xA9\x86\xF3"  // C
        "\xCD\xB1\xB2\xB6\xB8\xB9\xBA\xBB\xF7\xD9\xDA\xDB\xDC\xF8\xF9\xDF"  // D
        "\xD0\xD1\xD3\xD7\xAA\xAB\xDD\xB0\xE3\xE4\xEB\xEE\xEF\xF0\xF2\xF4"  // E
        "\xBC\xBD\xBE\xC6\xC7\xC8\xF6\xC9\xCA\xFA\xFB\xCB\xF1\xCC\xFE\xFF"; // F

    unsigned char koi2lat[129] =
        //    0   1   2   3   4   5   6   7   8   9   A   B   C   D   E   F
        "\x80\x83\x86\xF5\xE1\x87\x88\x89\x8A\x8B\x8C\x8F\x97\xCF\xCF\x98"  // 8
        "\x9D\x9E\xA4\xA5\xA8\xA9\xDB\xDC\xDF\xB1\xB2\xB6\xB8\xB9\xBA\xBB"  // 9
        "\xBC\xBD\xBE\xC6\xC7\xC8\xF6\xC9\xCA\xCB\xCC\xCD\xCE\xAD\xAE\xAF"  // A
        "\xD0\xD1\xD3\xD7\xAA\xAB\xDD\xB0\xE3\xE4\xEB\xEE\xEF\xF0\xF2\xF4"  // B
        "\xDA\xA0\xC0\x9F\xD4\xD8\xEA\xC4\x81\xA1\x85\x92\x96\x94\xE5\xA2"  // C
        "\x93\x84\xFD\xE7\x9C\xA3\xC3\x82\xF7\xEC\xA7\xC2\xF3\xF8\xF9\xC5"  // D
        "\xBF\xB5\xD9\xAC\xD2\xB7\xE8\xB3\x9A\xD6\xDE\x91\x95\x99\xD5\xE0"  // E
        "\xE2\x8E\xFC\xE6\x9B\xE9\xB4\x90\xFA\xED\xA6\xC1\xF1\xFB\xFE\xFF"; // F

    unsigned char lat2UNC[257] =
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

    unsigned char rus2UNC[257] =
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

#define _AddAtt(_nam, _val) \
    mpAttrList->AddAttribute(OUString::createFromAscii( _nam ),\
    OUString::createFromAscii( _val ) );

#define _Start(_nam) \
    mxHandler->startElement(OUString::createFromAscii(_nam), mAttrList);\
    mpAttrList->Clear();

#define _End(_nam) \
    mxHandler->endElement(OUString::createFromAscii(_nam));

#define _Chars(_ch) \
    mxHandler->characters(OUString::createFromAscii((sal_Char *) _ch) );

inistruct ini;

T602ImportFilter::T602ImportFilter(const ::com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory > &r ) :
    mxMSF( r ) {}

T602ImportFilter::~T602ImportFilter()
{
}

// XExtendedTypeDetection
OUString T602ImportFilter::detect( Sequence<PropertyValue>& Descriptor)
    throw(RuntimeException)
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

    ::com::sun::star::uno::Sequence< sal_Int8 > aData;
    const size_t numBytes = 4;
    size_t numBytesRead = 0;

    numBytesRead = mxInputStream->readSomeBytes (aData, numBytes);

    if ((numBytesRead != numBytes) || (aData[0] != '@') ||
        (aData[1] != 'C') || (aData[2] != 'T') || (aData[3] != ' '))
        return OUString();

    return OUString(  "writer_T602_Document"  );
}

// XFilter
sal_Bool SAL_CALL T602ImportFilter::filter( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
    throw (RuntimeException)
{
    return importImpl ( aDescriptor );
}

// XImporter
void SAL_CALL T602ImportFilter::setTargetDocument( const Reference< ::com::sun::star::lang::XComponent >& xDoc )
    throw (::com::sun::star::lang::IllegalArgumentException, RuntimeException)
{
    mxDoc = xDoc;
}

// XInitialization
void SAL_CALL T602ImportFilter::initialize( const Sequence< Any >& aArguments )
    throw (Exception, RuntimeException)
{
    Sequence < PropertyValue > aAnySeq;
    sal_Int32 nLength = aArguments.getLength();
    if ( nLength && ( aArguments[0] >>= aAnySeq ) )
    {
        const PropertyValue * pValue = aAnySeq.getConstArray();
        nLength = aAnySeq.getLength();
        for ( sal_Int32 i = 0 ; i < nLength; i++)
        {
            if ( pValue[i].Name == "Type" )
            {
                pValue[i].Value >>= msFilterName;
                break;
            }
        }
    }
}

// Other functions

void T602ImportFilter::inschr(unsigned char ch)
{
    Reference < XAttributeList > mAttrList ( mpAttrList );

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
            _AddAtt("text:c",s);
            _Start("text:s");
            _End("text:s");
        }
    }

    pst.wasspace = 0;
    setfnt(chngul,true);
    inschrdef(ch);
}

sal_Bool SAL_CALL T602ImportFilter::importImpl( const Sequence< ::com::sun::star::beans::PropertyValue >& aDescriptor )
    throw (RuntimeException)
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
        OSL_ASSERT( 0 );
        return sal_False;
    }

    // An XML import service: what we push sax messages to..
    OUString sXMLImportService (  "com.sun.star.comp.Writer.XMLImporter"  );

    mxHandler = Reference< XDocumentHandler >( mxMSF->createInstance( sXMLImportService ), UNO_QUERY );

    // The XImporter sets up an empty target document for XDocumentHandler to write to..
    Reference < XImporter > xImporter(mxHandler, UNO_QUERY);
    xImporter->setTargetDocument(mxDoc);

    char fs[32], fs2[32];
    sprintf(fs, "%ipt", ini.fontsize);
    sprintf(fs2,"%ipt", 2*ini.fontsize);

    mpAttrList = new SvXMLAttributeList;

    Reference < XAttributeList > mAttrList ( mpAttrList );

    mxHandler->startDocument();

    _AddAtt("xmlns:office", "http://openoffice.org/2000/office");
    _AddAtt("xmlns:style", "http://openoffice.org/2000/style");
    _AddAtt("xmlns:text", "http://openoffice.org/2000/text");
    _AddAtt("xmlns:table", "http://openoffice.org/2000/table");
    _AddAtt("xmlns:draw", "http://openoffice.org/2000/draw");
    _AddAtt("xmlns:fo", "http://www.w3.org/1999/XSL/Format");
    _AddAtt("xmlns:xlink", "http://www.w3.org/1999/xlink");
    _AddAtt("xmlns:number", "http://openoffice.org/2000/datastyle");
    _AddAtt("xmlns:svg", "http://www.w3.org/2000/svg");
    _AddAtt("xmlns:chart", "http://openoffice.org/2000/chart");
    _AddAtt("xmlns:dr3d", "http://openoffice.org/2000/dr3d");
    _AddAtt("xmlns:math", "http://www.w3.org/1998/Math/MathML");
    _AddAtt("xmlns:form", "http://openoffice.org/2000/form");
    _AddAtt("xmlns:script", "http://openoffice.org/2000/script");
    _AddAtt("office:class", "text");
    _AddAtt("office:version", "1.0");
    _Start("office:document-content");

    _Start("office:font-decls");
    _AddAtt("style:name","Courier New");
    _AddAtt("fo:font-family","Courier New");
    _AddAtt("style:font-pitch","fixed");
    _Start("style:font-decl");
    _End("style:font-decl");
    _End("office:font-decls");

    _Start("office:automatic-styles");

    // Standardni text
    _AddAtt("style:name","P1");
    _AddAtt("style:family","paragraph");
    _AddAtt("style:parent-style-name","Standard");
    _Start("style:style");
    _AddAtt("style:font-name","Courier New");
    _AddAtt("fo:font-size",fs);
    _Start("style:properties");
    _End("style:properties");
    _End("style:style");

    // Standardni text - konec stranky
    _AddAtt("style:name","P2");
    _AddAtt("style:family","paragraph");
    _AddAtt("style:parent-style-name","Standard");
    _Start("style:style");
    _AddAtt("style:font-name","Courier New");
    _AddAtt("fo:font-size",fs);
    _AddAtt("fo:break-before","page");
    _Start("style:properties");
    _End("style:properties");
    _End("style:style");

    // T1 Normalni text
    _AddAtt("style:name","T1");
    _AddAtt("style:family","text");
    _Start("style:style");
    _Start("style:properties");
    _End("style:properties");
    _End("style:style");

    // T2 Tucny text
    _AddAtt("style:name","T2");
    _AddAtt("style:family","text");
    _Start("style:style");
    _AddAtt("fo:font-weight","bold");
    _Start("style:properties");
    _End("style:properties");
    _End("style:style");

    // T3 Kurziva
    _AddAtt("style:name","T3");
    _AddAtt("style:family","text");
    _Start("style:style");
    _AddAtt("fo:font-style","italic");
    _Start("style:properties");
    _End("style:properties");
    _End("style:style");

    // T4 Siroky text
    _AddAtt("style:name","T4");
    _AddAtt("style:family","text");
    _Start("style:style");
    _AddAtt("fo:font-weight","bold");
    _AddAtt("style:text-scale","200%");
    _Start("style:properties");
    _End("style:properties");
    _End("style:style");

    // T5 Vysoky text
    _AddAtt("style:name","T5");
    _AddAtt("style:family","text");
    _Start("style:style");
    _AddAtt("fo:font-size",fs2);
    _AddAtt("fo:font-weight","bold");
    _AddAtt("style:text-scale","50%");
    _Start("style:properties");
    _End("style:properties");
    _End("style:style");

    // T6 Velky text
    _AddAtt("style:name","T6");
    _AddAtt("style:family","text");
    _Start("style:style");
    _AddAtt("fo:font-size",fs2);
    _AddAtt("fo:font-weight","bold");
    _Start("style:properties");
    _End("style:properties");
    _End("style:style");

    // T7 Podtrzeny text
    _AddAtt("style:name","T7");
    _AddAtt("style:family","text");
    _Start("style:style");
    _AddAtt("style:text-underline","single");
    _Start("style:properties");
    _End("style:properties");
    _End("style:style");

    // T8 Podtrzena tucny text
    _AddAtt("style:name","T8");
    _AddAtt("style:family","text");
    _Start("style:style");
    _AddAtt("fo:font-weight","bold");
    _AddAtt("style:text-underline","single");
    _Start("style:properties");
    _End("style:properties");
    _End("style:style");

    // T9 Podtrzena kurziva
    _AddAtt("style:name","T9");
    _AddAtt("style:family","text");
    _Start("style:style");
    _AddAtt("fo:font-style","italic");
    _AddAtt("style:text-underline","single");
    _Start("style:properties");
    _End("style:properties");
    _End("style:style");

    // T10 Horni index
    _AddAtt("style:name","T10");
    _AddAtt("style:family","text");
    _Start("style:style");
    _AddAtt("style:text-position","27% 100%");
    _Start("style:properties");
    _End("style:properties");
    _End("style:style");

    // T11 Dolni index
    _AddAtt("style:name","T11");
    _AddAtt("style:family","text");
    _Start("style:style");
    _AddAtt("style:text-position","-27% 100%");
    _Start("style:properties");
    _End("style:properties");
    _End("style:style");

    _End("office:automatic-styles");

    _Start("office:styles");
    _End("office:styles");

    _Start("office:body");

    Read602();

    _End("office:body");
    _End("office:document-content");

    mxHandler->endDocument();

    return sal_True;
}

void T602ImportFilter::Reset602()
{
    node = START;

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
    ini.fontsize    = 10;

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
    mxHandler->characters(OUString(xch));
}

void T602ImportFilter::wrtfnt()
{
    Reference < XAttributeList > mAttrList ( mpAttrList );
    const sal_Char *style = "P1";

    switch(fst.nowfnt) {
        case standard : style = (char*)(fst.uline ? "T7" : "T1"); break;
        case fat      : style = (char*)(fst.uline ? "T8" : "T2"); break;
        case cursive  : style = (char*)(fst.uline ? "T9" : "T3"); break;
        case bold     : style = "T4"; break;
        case tall     : style = "T5"; break;
        case big      : style = "T6"; break;
        case lindex   : style = "T11";break;
        case hindex   : style = "T10";break;
        default       : style = "T1"; break;
    }

    _End("text:span");
    _AddAtt("text:style-name", style);
    _Start("text:span");
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
        ((unsigned char *) aData.getConstArray())[0] : 0;
}

void T602ImportFilter::par602(bool endofpage)
{
    sal_Int16 endp;
    Reference < XAttributeList > mAttrList ( mpAttrList );

    if(!endofpage || !pst.waspar) {
        if(ini.showcomm||!pst.comment) {
            if(pst.waspar||ini.reformatpars) {
                _End("text:span");
                _End("text:p");
                _AddAtt("text:style-name", "P1");
                _Start("text:p");
                _Start("text:span");
                wrtfnt();
            } else {
                _Start("text:line-break");
                _End("text:line-break");
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
                _End("text:span");
                _End("text:p");
                _AddAtt("text:style-name", "P2");
                _Start("text:p");
                _Start("text:span");
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
    return (sal_Int16) atoi((char *)buff);
}

unsigned char T602ImportFilter::Setformat602(char *cmd)
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
    pcmd[0] = (char) toupper(*ch); inschr(*ch);
    *ch = Readchar602();
    if (!*ch) return EEND;
    if (*ch=='\n') return EOL;
    if (!isalpha(*ch)) return (*ch<32) ? SETCH : WRITE;

    // warning: uChar -> char
    pcmd[1] = (char) toupper(*ch); inschr(*ch);

         if (pcmd[0]=='P' && pcmd[1]=='A') { if (pst.pars) pst.willbeeop = true; }
    else if (pcmd[0]=='C' && pcmd[1]=='P') { if (pst.pars) pst.willbeeop = true; }
    else if (pcmd[0]=='P' && pcmd[1]=='I') {
        while (*ch && (*ch != '\n') && (*ch != ','))
            { *ch = Readchar602(); inschr(*ch); }
        if (!*ch) return EEND;
        if (*ch=='\n') return EOL;
        if (*ch==',') { *ch = Readchar602(); inschr(*ch); }
        pst.pars += (readnum(ch,true)*2);
        if (!*ch) return EEND;
        if (*ch=='\n') return EOL;
    }
    // else if(pcmd[0]=='K'&&pcmd[1]=='P') {}
    // else if(pcmd[0]=='H'&&pcmd[1]=='E') {}
    // else if(pcmd[0]=='F'&&pcmd[1]=='O') {}

    return READCH;
}


void T602ImportFilter::Read602()
{
    unsigned char ch=0;
    char cmd602[3] = {0};

    Reference < XAttributeList > mAttrList ( mpAttrList );

    if (node==QUIT) return;

    _AddAtt("text:style-name", "P1");
    _Start("text:p");
    _AddAtt("text:style-name", "T1");
    _Start("text:span");

    if (node==START) { node = EOL; }

    while (node != EEND) {
        switch (node) {
        case READCH:
            ch = Readchar602();
            if (ch == 0) node = EEND;
            else if (ch == '\n') {
                if(!pst.willbeeop) par602(false);
                node = EOL;
            } else if (ch < 32) node = SETCH;
            else node = WRITE;
            break;
        case EOL:
            ch = Readchar602();
            pst.comment = false;
            if (pst.willbeeop) par602(true);
            pst.willbeeop = false;
            if(ch == 0) node = EEND;
            else if (ch == '@') node = EXPCMD;
            else if (ch == '\n') { par602(false); node = EOL; }
            else if (ch < 32) {pst.ccafterln = true; node = SETCH; break;}
            else node = WRITE;
            if (ch == '.') { pst.comment = true; node = POCMD; }
            pst.ccafterln = false;
            break;

        case POCMD: inschr('.');
            ch = Readchar602();
            if(ch == 0) node = EEND;
            else if(isalpha(ch)) node = PointCmd602(&ch);
            else if(ch <32) node=SETCH;
            else node = WRITE;
            break;

        case EXPCMD: ch = Readchar602();
            if(ch == 0) {inschr('@'); node = EEND; }
            else if(isupper(ch)) {
                cmd602[0] = ch;
                ch = Readchar602();
                cmd602[1] = ch;
                cmd602[2] = '\0';
                if(isupper(ch)) node = SETCMD;   //nedodelano
                else { inschr('@'); _Chars(cmd602); node = READCH; }
            } else {
                inschr('@');
                if(ch<32) node = SETCH;
                else node = WRITE;}
            break;

        case SETCMD:
            ch = Setformat602(cmd602);
            if(ch == 0) node = EEND;
            else if(ch == '\n') node = EOL;
            else node = READCH;
            break;

        case SETCH :
            // warning: potentially uninitialized
            switch(ch) {
            case '\t' : _Start("text:tab-stop");
                    _End("text:tab-stop");
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
            if(pst.ccafterln) node = EOL;
            else node = READCH;
            break;

        case WRITE :
            switch(ch) {
            case 0x8d:
                ch = Readchar602();
                if( ch == 0x0a) {
                    if(ini.reformatpars) inschr(' ');
                    else par602(false); //formatovaci radek
                    node = EOL;
                } else {
                    inschr(0x8d);//inschr(' ');
                    if(ch == 0) node = EEND;
                    else if(ch == '\n') {
                        if(!pst.willbeeop) par602(false);
                        node = EOL; }
                    else if(ch < 32) node = SETCH;
                    else node = WRITE;
                }
                break;
            case 0xfe:
                if (ini.showcomm||!pst.comment) inschr(' ');
                node = READCH;
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
                    node=WRITE;
                } else {
                    inschr(0xad);
                    if(ch == 0) node = EEND;
                    else if(ch == '\n') {
                        if(!pst.willbeeop) par602(false);
                        node = EOL; }
                    else if(ch < 32) node = SETCH;
                    else node = WRITE;
                }
                break;
            default:
                inschr(ch);
                node = READCH;
                break;
            }
            break;
        default: break;
        }
    }

    _End("text:span");
    _End("text:p");
    node = QUIT;
}

// XServiceInfo
OUString SAL_CALL T602ImportFilter::getImplementationName(  )
    throw (RuntimeException)
{
    return T602ImportFilter_getImplementationName();
}

sal_Bool SAL_CALL T602ImportFilter::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return T602ImportFilter_supportsService( rServiceName );
}

Sequence< OUString > SAL_CALL T602ImportFilter::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    return T602ImportFilter_getSupportedServiceNames();
}

#define IMPLEMENTATION_NAME "com.sun.star.comp.Writer.T602ImportFilter"

OUString T602ImportFilter_getImplementationName ()
    throw (RuntimeException)
{
    return OUString (  IMPLEMENTATION_NAME  );
}

#define SERVICE_NAME1 "com.sun.star.document.ImportFilter"
#define SERVICE_NAME2 "com.sun.star.document.ExtendedTypeDetection"
sal_Bool SAL_CALL T602ImportFilter_supportsService( const OUString& ServiceName )
    throw (RuntimeException)
{
    return ( ServiceName == SERVICE_NAME1 || ServiceName == SERVICE_NAME2 );
}

Sequence< OUString > SAL_CALL T602ImportFilter_getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence < OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString (  SERVICE_NAME1  );
    pArray[1] =  OUString (  SERVICE_NAME2  );
    return aRet;
}
#undef SERVICE_NAME2
#undef SERVICE_NAME1
#undef IMPLEMENTATION_NAME

Reference< XInterface > SAL_CALL T602ImportFilter_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new T602ImportFilter( rSMgr );
}

T602ImportFilterDialog::T602ImportFilterDialog(const ::com::sun::star::uno::Reference<com::sun::star::lang::XMultiServiceFactory > &r ) :
    mxMSF( r ), mpResMgr( NULL ) {}

T602ImportFilterDialog::~T602ImportFilterDialog()
{
    if (mpResMgr)
        delete mpResMgr;
}

// XLocalizable

void SAL_CALL T602ImportFilterDialog::setLocale( const Locale& eLocale )
    throw(::com::sun::star::uno::RuntimeException)
{
    meLocale = eLocale;
    initLocale();
}

Locale SAL_CALL T602ImportFilterDialog::getLocale()
    throw(::com::sun::star::uno::RuntimeException)
{
    return meLocale;
}

sal_Bool T602ImportFilterDialog::OptionsDlg()
{
    Any any;
#define _propInt(_prop,_nam,_val) \
    any <<= (sal_Int32)_val;\
    _prop->setPropertyValue(OUString::createFromAscii(_nam), any);
#define _propShort(_prop,_nam,_val) \
    any <<= (sal_Int16)_val;\
    _prop->setPropertyValue(OUString::createFromAscii(_nam), any);
#define _propBool(_prop,_nam,_val) \
    any <<= (sal_Bool)_val;\
    _prop->setPropertyValue(OUString::createFromAscii(_nam), any);
#define _propString(_prop,_nam,_val) \
    any <<= OUString::createFromAscii(_val);\
    _prop->setPropertyValue(OUString::createFromAscii(_nam), any);
#define _propStringFromResId(_prop,_nam,_val) \
    any <<= getResStr(_val);\
    _prop->setPropertyValue(OUString::createFromAscii(_nam), any);
#define _propGet(_prop,_nam) \
    _prop->getPropertyValue(OUString::createFromAscii(_nam));
#define _InstCtx(_path,_ctx)\
    rServiceManager->createInstanceWithContext(\
    OUString::createFromAscii(_path),_ctx);
#define _Inst(_path)\
    xMultiServiceFactory->createInstance(OUString::createFromAscii(_path) );
#define _Insert(_cont,_nam,_obj) \
    any <<= _obj;\
    _cont->insertByName( OUString::createFromAscii(_nam), any );

    Reference < XComponentContext > rComponentContext = defaultBootstrap_InitialComponentContext();
    Reference < XMultiComponentFactory > rServiceManager = rComponentContext->getServiceManager();
    Reference < XInterface > rInstance = _InstCtx("com.sun.star.awt.UnoControlDialogModel", rComponentContext );

    Reference <XMultiServiceFactory> xMultiServiceFactory (rInstance,UNO_QUERY);

    Reference < XPropertySet > xPSetDialog( rInstance, UNO_QUERY );
    _propInt(xPSetDialog,"PositionX",100);
    _propInt(xPSetDialog,"PositionY",100);
    _propInt(xPSetDialog,"Width",130);
    _propInt(xPSetDialog,"Height",90);
    _propStringFromResId(xPSetDialog,"Title", T602FILTER_STR_IMPORT_DIALOG_TITLE);

#define T602DLG_OK_BUTTON    "ok_button"
#define T602DLG_CANCEL_BUTTON    "cancel_button"
#define T602DLG_CODE_LB        "code_lb"
#define T602DLG_AZBUKA_CB    "azbuka_cb"
#define T602DLG_COMMENT_CB    "comment_cb"
#define T602DLG_REFORMAT_CB    "reformat_cb"
#define T602DLG_CODE_TXT    "code_txt"

    Reference < XInterface > TextModel = _Inst("com.sun.star.awt.UnoControlFixedTextModel");
    Reference < XPropertySet > xPSetText( TextModel, UNO_QUERY );
    _propInt(xPSetText,"PositionX",10);
    _propInt(xPSetText,"PositionY",8);
    _propInt(xPSetText,"Width",30);
    _propInt(xPSetText,"Height",14);
    _propString(xPSetText,"Name",T602DLG_CODE_TXT);
    _propStringFromResId(xPSetText,"Label",T602FILTER_STR_ENCODING_LABEL);

    Reference < XInterface > ListBoxModel = _Inst("com.sun.star.awt.UnoControlListBoxModel");
    Reference < XPropertySet > xPSetCodeLB( ListBoxModel, UNO_QUERY );
    _propInt(xPSetCodeLB,"PositionX",40);
    _propInt(xPSetCodeLB,"PositionY",5);
    _propInt(xPSetCodeLB,"Width",85);
    _propInt(xPSetCodeLB,"Height",14);
    _propString(xPSetCodeLB,"Name",T602DLG_CODE_LB);
    _propShort(xPSetCodeLB,"TabIndex",1);
    _propBool(xPSetCodeLB,"Dropdown",true);
    _propBool(xPSetCodeLB,"MultiSelection",false);

    Sequence< OUString > ous(4);
    ous[0] = getResStr(T602FILTER_STR_ENCODING_AUTO);
    ous[1] = getResStr(T602FILTER_STR_ENCODING_CP852);
    ous[2] = getResStr(T602FILTER_STR_ENCODING_CP895);
    ous[3] = getResStr(T602FILTER_STR_ENCODING_KOI8CS2);
    any <<= ous;
    xPSetCodeLB->setPropertyValue("StringItemList", any);

    Sequence < sal_Int16 > shr(1);
    shr[0] = ini.forcecode ? ((sal_Int16) ini.xcode) + 1 : 0;
    any <<= shr;
    xPSetCodeLB->setPropertyValue("SelectedItems", any);

    Reference < XInterface > AzbCheckBoxModel = _Inst("com.sun.star.awt.UnoControlCheckBoxModel");
    Reference < XPropertySet > xPSetAzbukaCB( AzbCheckBoxModel, UNO_QUERY );
    _propInt(xPSetAzbukaCB,"PositionX",10);
    _propInt(xPSetAzbukaCB,"PositionY",25);
    _propInt(xPSetAzbukaCB,"Width",100);
    _propInt(xPSetAzbukaCB,"Height",14);
    _propString(xPSetAzbukaCB,"Name",T602DLG_AZBUKA_CB);
    _propShort(xPSetAzbukaCB,"TabIndex",2);
    _propStringFromResId(xPSetAzbukaCB,"Label",T602FILTER_STR_CYRILLIC_MODE);
    _propShort(xPSetAzbukaCB,"State",ini.ruscode);

    Reference < XInterface > RefCheckBoxModel = _Inst("com.sun.star.awt.UnoControlCheckBoxModel");
    Reference < XPropertySet > xPSetRefCB( RefCheckBoxModel, UNO_QUERY );
    _propInt(xPSetRefCB,"PositionX",10);
    _propInt(xPSetRefCB,"PositionY",40);
    _propInt(xPSetRefCB,"Width",100);
    _propInt(xPSetRefCB,"Height",14);
    _propString(xPSetRefCB,"Name",T602DLG_REFORMAT_CB);
    _propShort(xPSetRefCB,"TabIndex",3);
    _propStringFromResId(xPSetRefCB,"Label",T602FILTER_STR_REFORMAT_TEXT);
    _propShort(xPSetRefCB,"State",ini.reformatpars);

    Reference < XInterface > CommCheckBoxModel = _Inst("com.sun.star.awt.UnoControlCheckBoxModel");
    Reference < XPropertySet > xPSetCommCB( CommCheckBoxModel, UNO_QUERY );
    _propInt(xPSetCommCB,"PositionX",10);
    _propInt(xPSetCommCB,"PositionY",55);
    _propInt(xPSetCommCB,"Width",100);
    _propInt(xPSetCommCB,"Height",14);
    _propString(xPSetCommCB,"Name",T602DLG_COMMENT_CB);
    _propShort(xPSetCommCB,"TabIndex",4);
    _propStringFromResId(xPSetCommCB,"Label",T602FILTER_STR_DOT_COMMANDS);
    _propShort(xPSetCommCB,"State",ini.showcomm);

    Reference < XInterface > CancelButtonModel = _Inst("com.sun.star.awt.UnoControlButtonModel");
    Reference < XPropertySet > xPSetCancelButton( CancelButtonModel, UNO_QUERY );
    _propInt(xPSetCancelButton,"PositionX",10);
    _propInt(xPSetCancelButton,"PositionY",70);
    _propInt(xPSetCancelButton,"Width",50);
    _propInt(xPSetCancelButton,"Height",14);
    _propString(xPSetCancelButton,"Name",T602DLG_CANCEL_BUTTON);
    _propShort(xPSetCancelButton,"TabIndex",5);
    _propShort(xPSetCancelButton,"PushButtonType",2);
    _propStringFromResId(xPSetCancelButton,"Label",T602FILTER_STR_CANCEL_BUTTON);

    Reference < XInterface > OkButtonModel = _Inst("com.sun.star.awt.UnoControlButtonModel");
    Reference < XPropertySet > xPSetOkButton( OkButtonModel, UNO_QUERY );
    _propInt(xPSetOkButton,"PositionX",70);
    _propInt(xPSetOkButton,"PositionY",70);
    _propInt(xPSetOkButton,"Width",50);
    _propInt(xPSetOkButton,"Height",14);
    _propString(xPSetOkButton,"Name",T602DLG_OK_BUTTON);
    _propShort(xPSetOkButton,"TabIndex",0);
    _propShort(xPSetOkButton,"PushButtonType",1);
    _propStringFromResId(xPSetOkButton,"Label",T602FILTER_STR_OK_BUTTON);
    _propBool(xPSetOkButton,"DefaultButton",true);

    Reference < XNameContainer > xNameCont (rInstance,UNO_QUERY);

    _Insert(xNameCont, T602DLG_OK_BUTTON, OkButtonModel);
    _Insert(xNameCont, T602DLG_CANCEL_BUTTON, CancelButtonModel);
    _Insert(xNameCont, T602DLG_AZBUKA_CB, AzbCheckBoxModel);
    _Insert(xNameCont, T602DLG_REFORMAT_CB, RefCheckBoxModel);
    _Insert(xNameCont, T602DLG_COMMENT_CB, CommCheckBoxModel);
    _Insert(xNameCont, T602DLG_CODE_LB, ListBoxModel);
    _Insert(xNameCont, T602DLG_CODE_TXT, TextModel);

    Reference< XUnoControlDialog > dialog = UnoControlDialog::create(rComponentContext);

    Reference < XControlModel > xControlModel (rInstance,UNO_QUERY);

    dialog->setModel( xControlModel );

    Reference < XToolkit > xToolkit = Toolkit::create( rComponentContext );

    dialog->setVisible( false );
    dialog->createPeer( xToolkit, NULL );

    sal_Bool ret = ( dialog->execute() != 0 );
    if ( ret ) {

        sal_Int16 tt = 0;

        any = _propGet(xPSetAzbukaCB, "State"); any >>= tt; ini.ruscode      = tt;
        any = _propGet(xPSetRefCB,    "State"); any >>= tt; ini.reformatpars = tt;
        any = _propGet(xPSetCommCB,   "State"); any >>= tt; ini.showcomm     = tt;

        any = _propGet(xPSetCodeLB,   "SelectedItems"); any >>= shr;

        if( shr[0] > 0 ) {
            ini.xcode = (tcode)(shr[0]-1);
            ini.forcecode = true;
        } else {
            ini.forcecode = false;
        }
    }

    Reference<XControl>(dialog)->dispose();

    return ret;
}

void T602ImportFilterDialog::initLocale()
{
    mpResMgr = ResMgr::CreateResMgr( "t602filter", LanguageTag( meLocale) );
}

ResMgr* T602ImportFilterDialog::getResMgr()
{
    if( !mpResMgr )
        initLocale();
    return mpResMgr;
}

void SAL_CALL T602ImportFilterDialog::setTitle( const OUString& )
            throw (::com::sun::star::uno::RuntimeException)
{
}

sal_Int16 SAL_CALL T602ImportFilterDialog::execute()
            throw (::com::sun::star::uno::RuntimeException)
{
    if (OptionsDlg())
        return com::sun::star::ui::dialogs::ExecutableDialogResults::OK;
    else
        return com::sun::star::ui::dialogs::ExecutableDialogResults::CANCEL;
}

OUString T602ImportFilterDialog::getResStr( sal_Int16 resid )
{
    OUString sStr( ResId( resid, *getResMgr() ) );
    return sStr;
}

uno::Sequence<beans::PropertyValue> SAL_CALL T602ImportFilterDialog::getPropertyValues() throw(uno::RuntimeException)
{
    return uno::Sequence<beans::PropertyValue>();
}

void SAL_CALL T602ImportFilterDialog::setPropertyValues( const uno::Sequence<beans::PropertyValue>& )
                    throw(beans::UnknownPropertyException, beans::PropertyVetoException,
                            lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
}


// XServiceInfo
OUString SAL_CALL T602ImportFilterDialog::getImplementationName(  )
    throw (RuntimeException)
{
    return T602ImportFilterDialog_getImplementationName();
}

sal_Bool SAL_CALL T602ImportFilterDialog::supportsService( const OUString& rServiceName )
    throw (RuntimeException)
{
    return T602ImportFilterDialog_supportsService( rServiceName );
}

Sequence< OUString > SAL_CALL T602ImportFilterDialog::getSupportedServiceNames(  )
    throw (RuntimeException)
{
    return T602ImportFilterDialog_getSupportedServiceNames();
}

#define IMPLEMENTATION_NAME "com.sun.star.comp.Writer.T602ImportFilterDialog"

OUString T602ImportFilterDialog_getImplementationName ()
    throw (RuntimeException)
{
    return OUString (  IMPLEMENTATION_NAME  );
}

#define SERVICE_NAME "com.sun.star.ui.dialogs.FilterOptionsDialog"
sal_Bool SAL_CALL T602ImportFilterDialog_supportsService( const OUString& ServiceName )
    throw (RuntimeException)
{
    return ServiceName == SERVICE_NAME;
}

Sequence< OUString > SAL_CALL T602ImportFilterDialog_getSupportedServiceNames(  )
    throw (RuntimeException)
{
    Sequence < OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] =  OUString (  SERVICE_NAME  );
    return aRet;
}
#undef SERVICE_NAME

Reference< XInterface > SAL_CALL T602ImportFilterDialog_createInstance( const Reference< XMultiServiceFactory > & rSMgr)
    throw( Exception )
{
    return (cppu::OWeakObject*) new T602ImportFilterDialog( rSMgr );
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
