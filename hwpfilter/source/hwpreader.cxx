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

#include <deque>
#include <memory>
#include <string_view>

#include "hwpreader.hxx"
#include <cmath>

#include <o3tl/safeint.hxx>
#include <o3tl/sprintf.hxx>
#include <osl/diagnose.h>
#include <tools/stream.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <comphelper/configuration.hxx>

#include "fontmap.hxx"
#include "formula.h"
#include "cspline.h"
#include "datecode.h"

#include <iostream>
#include <locale.h>
#include <sal/types.h>
#include <rtl/character.hxx>
#include <rtl/ustrbuf.hxx>
#include <sal/log.hxx>

// xmloff/xmlkyd.hxx
constexpr OUString sXML_CDATA = u"CDATA"_ustr;

namespace
{
double WTI(double x) { return x / 1800.; } // unit => inch
double WTMM(double x) { return x / 1800. * 25.4; } // unit => mm
int WTSM(double x) { return x / 1800. * 2540; } // unit ==> 1/100 mm

constexpr OUString sBeginOfDoc(u"[\uBB38\uC11C\uC758 \uCC98\uC74C]"_ustr);
    // U+BB38 HANGUL SYLLABLE MUN, U+C11C HANGUL SYLLABLE SEO,
    // U+C758 HANGUL SYLLABLE YI, U+CC98 HANGUL SYLLABLE CEO,
    // U+C74C HANGUL SYLLABLE EUM: "Begin of Document"
}

struct HwpReaderPrivate
{
    HwpReaderPrivate()
    {
        bFirstPara = true;
        bInBody = false;
        bInHeader = false;
        nPnPos = 0;
        pPn = nullptr;
        pField = nullptr;
    }
    bool bFirstPara;
    bool bInBody;
    bool bInHeader;
    ShowPageNum *pPn;
    hchar *pField;
    int nPnPos;
    char buf[1024];
};

HwpReader::HwpReader() : mxList(new AttributeListImpl), d(new HwpReaderPrivate)
{
}


HwpReader::~HwpReader()
{
}

extern "C" SAL_DLLPUBLIC_EXPORT bool TestImportHWP(SvStream &rStream)
{
    try
    {
        std::unique_ptr<HStream> stream(new HStream);
        byte aData[32768];

        while (true)
        {
            std::size_t nRead = rStream.ReadBytes(aData, 32768);
            if (nRead == 0)
                break;
            stream->addData(aData, static_cast<int>(nRead));
        }

        rtl::Reference<HwpReader> hwpreader(new HwpReader);
        return hwpreader->importHStream(std::move(stream));
    }
    catch (...)
    {
    }
    return false;
}

bool HwpReader::importHStream(std::unique_ptr<HStream> stream)
{
    if (hwpfile.ReadHwpFile(std::move(stream)))
          return false;

    if (m_rxDocumentHandler.is())
        m_rxDocumentHandler->startDocument();

    mxList->addAttribute(u"office:class"_ustr, sXML_CDATA, u"text"_ustr);
    mxList->addAttribute(u"office:version"_ustr, sXML_CDATA, u"0.9"_ustr);

    mxList->addAttribute(u"xmlns:office"_ustr, u"CDATA"_ustr, u"http://openoffice.org/2000/office"_ustr);
    mxList->addAttribute(u"xmlns:style"_ustr, u"CDATA"_ustr, u"http://openoffice.org/2000/style"_ustr);
    mxList->addAttribute(u"xmlns:text"_ustr, u"CDATA"_ustr, u"http://openoffice.org/2000/text"_ustr);
    mxList->addAttribute(u"xmlns:table"_ustr, u"CDATA"_ustr, u"http://openoffice.org/2000/table"_ustr);
    mxList->addAttribute(u"xmlns:draw"_ustr, u"CDATA"_ustr, u"http://openoffice.org/2000/drawing"_ustr);
    mxList->addAttribute(u"xmlns:fo"_ustr, u"CDATA"_ustr, u"http://www.w3.org/1999/XSL/Format"_ustr);
    mxList->addAttribute(u"xmlns:xlink"_ustr, u"CDATA"_ustr, u"http://www.w3.org/1999/xlink"_ustr);
    mxList->addAttribute(u"xmlns:dc"_ustr, u"CDATA"_ustr, u"http://purl.org/dc/elements/1.1/"_ustr);
    mxList->addAttribute(u"xmlns:meta"_ustr, u"CDATA"_ustr, u"http://openoffice.org/2000/meta"_ustr);
    mxList->addAttribute(u"xmlns:number"_ustr, u"CDATA"_ustr, u"http://openoffice.org/2000/datastyle"_ustr);
    mxList->addAttribute(u"xmlns:svg"_ustr, u"CDATA"_ustr, u"http://www.w3.org/2000/svg"_ustr);
    mxList->addAttribute(u"xmlns:chart"_ustr, u"CDATA"_ustr, u"http://openoffice.org/2000/chart"_ustr);
    mxList->addAttribute(u"xmlns:dr3d"_ustr, u"CDATA"_ustr, u"http://openoffice.org/2000/dr3d"_ustr);
    mxList->addAttribute(u"xmlns:math"_ustr, u"CDATA"_ustr, u"http://www.w3.org/1998/Math/MathML"_ustr);
    mxList->addAttribute(u"xmlns:form"_ustr, u"CDATA"_ustr, u"http://openoffice.org/2000/form"_ustr);
    mxList->addAttribute(u"xmlns:script"_ustr, u"CDATA"_ustr, u"http://openoffice.org/2000/script"_ustr);

    startEl(u"office:document"_ustr);
    mxList->clear();

    makeMeta();
    makeStyles();
    makeAutoStyles();
    makeMasterStyles();
    makeBody();

    endEl(u"office:document"_ustr);

    if (m_rxDocumentHandler.is())
        m_rxDocumentHandler->endDocument();
    return true;
}

sal_Bool HwpReader::filter(const Sequence< PropertyValue >& rDescriptor)
{
    utl::MediaDescriptor aDescriptor(rDescriptor);
    aDescriptor.addInputStream();

    Reference< XInputStream > xInputStream(
        aDescriptor[utl::MediaDescriptor::PROP_INPUTSTREAM], UNO_QUERY_THROW);

    std::unique_ptr<HStream> stream(new HStream);
    Sequence < sal_Int8 > aBuffer;
    sal_Int32 nRead, nTotal = 0;
    while( true )
    {
        nRead = xInputStream->readBytes(aBuffer, 32768);
        if( nRead == 0 )
            break;
        stream->addData( reinterpret_cast<const byte *>(aBuffer.getConstArray()), nRead );
        nTotal += nRead;
    }

    if( nTotal == 0 ) return false;

    return importHStream(std::move(stream));
}

/**
 * make office:body
 */
void HwpReader::makeBody()
{
    startEl(u"office:body"_ustr);
    makeTextDecls();
    HWPPara *hwppara = hwpfile.GetFirstPara();
    d->bInBody = true;
    parsePara(hwppara);
    endEl(u"office:body"_ustr);
    d->bInBody = false;
}


/**
 * make text decls
 */
void HwpReader::makeTextDecls()
{
    startEl(u"text:sequence-decls"_ustr);
    mxList->addAttribute(u"text:display-outline-level"_ustr, sXML_CDATA, u"0"_ustr);
    mxList->addAttribute(u"text:name"_ustr, sXML_CDATA, u"Illustration"_ustr);
    startEl(u"text:sequence-decl"_ustr);
    mxList->clear();
    endEl(u"text:sequence-decl"_ustr);
    mxList->addAttribute(u"text:display-outline-level"_ustr, sXML_CDATA, u"0"_ustr);
    mxList->addAttribute(u"text:name"_ustr, sXML_CDATA, u"Table"_ustr);
    startEl(u"text:sequence-decl"_ustr);
    mxList->clear();
    endEl(u"text:sequence-decl"_ustr);
    mxList->addAttribute(u"text:display-outline-level"_ustr, sXML_CDATA, u"0"_ustr);
    mxList->addAttribute(u"text:name"_ustr, sXML_CDATA, u"Text"_ustr);
    startEl(u"text:sequence-decl"_ustr);
    mxList->clear();
    endEl(u"text:sequence-decl"_ustr);
    mxList->addAttribute(u"text:display-outline-level"_ustr, sXML_CDATA, u"0"_ustr);
    mxList->addAttribute(u"text:name"_ustr, sXML_CDATA, u"Drawing"_ustr);
    startEl(u"text:sequence-decl"_ustr);
    mxList->clear();
    endEl(u"text:sequence-decl"_ustr);
    endEl(u"text:sequence-decls"_ustr);
}


/**
 * make office:meta
 * Completed
 */
void HwpReader::makeMeta()
{
    HWPInfo& hwpinfo = hwpfile.GetHWPInfo();

    startEl(u"office:meta"_ustr);

    if (hwpinfo.summary.title[0])
    {
        startEl(u"dc:title"_ustr);
        chars(hstr2OUString(reinterpret_cast<hchar *>(hwpinfo.summary.title)));
        endEl(u"dc:title"_ustr);
    }

    if (hwpinfo.summary.subject[0])
    {
        startEl(u"dc:subject"_ustr);
        chars(hstr2OUString(reinterpret_cast<hchar *>(hwpinfo.summary.subject)));
        endEl(u"dc:subject"_ustr);
    }

    if (hwpinfo.summary.author[0])
    {
        startEl(u"meta:initial-creator"_ustr);
        chars(hstr2OUString(reinterpret_cast<hchar *>(hwpinfo.summary.author)));
        endEl(u"meta:initial-creator"_ustr);
    }

    if (hwpinfo.summary.date[0])
    {
        unsigned short *pDate = hwpinfo.summary.date;
        int year,month,day,hour,minute;
        int gab = 0;
        if( rtl::isAsciiDigit( pDate[0] ) && rtl::isAsciiDigit( pDate[1] ) &&
            rtl::isAsciiDigit( pDate[2] ) && rtl::isAsciiDigit( pDate[3] ))
        {
            year = (pDate[0]-0x30) * 1000 + (pDate[1]-0x30) * 100 +
                (pDate[2]-0x30) * 10 + (pDate[3]-0x30);
        }
        else {
            year = 0;
        }
        if( rtl::isAsciiDigit( pDate[6] ))
        {
            if( rtl::isAsciiDigit( pDate[7] ) )
                month = (pDate[6] - 0x30) * 10 + (pDate[6+ ++gab]-0x30);
            else
                month = (pDate[6] - 0x30);
        }
        else {
            month = 0;
        }
        if( rtl::isAsciiDigit( pDate[9 + gab] ) )
        {
            if( rtl::isAsciiDigit( pDate[10 + gab])) {
                day = ( pDate[9 + gab] - 0x30 ) * 10 + (pDate[9+ gab + 1]-0x30);
                ++gab;
            } else
                day = (pDate[9+gab]-0x30);
        }
        else {
            day = 0;
        }
        if( rtl::isAsciiDigit( pDate[17 + gab] ) )
        {
            if( rtl::isAsciiDigit( pDate[18 + gab])) {
                hour = ( pDate[17 + gab] - 0x30 ) * 10 + (pDate[17+ gab + 1]-0x30);
                ++gab;
            } else
                hour = (pDate[17+gab]-0x30);
        }
        else {
            hour = 0;
        }
        if( rtl::isAsciiDigit( pDate[20 + gab] ) )
        {
            if( rtl::isAsciiDigit( pDate[21 + gab])) {
                minute = ( pDate[20 + gab] - 0x30 ) * 10 + (pDate[20+ gab + 1]-0x30);
                ++gab;
            } else
                minute = (pDate[20+gab]-0x30);
        }
        else {
            minute = 0;
        }
        o3tl::sprintf(d->buf,"%d-%02d-%02dT%02d:%02d:00",year,month,day,hour,minute);

        startEl(u"meta:creation-date"_ustr);
        chars( OUString::createFromAscii(d->buf));
        endEl(u"meta:creation-date"_ustr);
    }

    if (hwpinfo.summary.keyword[0][0] || hwpinfo.summary.etc[0][0])
    {
        startEl(u"meta:keywords"_ustr);
        if (hwpinfo.summary.keyword[0][0])
        {
            startEl(u"meta:keyword"_ustr);
            chars(hstr2OUString(reinterpret_cast<hchar *>(hwpinfo.summary.keyword[0])));
            endEl(u"meta:keyword"_ustr);
        }
        if (hwpinfo.summary.keyword[1][0])
        {
            startEl(u"meta:keyword"_ustr);
            chars(hstr2OUString(reinterpret_cast<hchar *>(hwpinfo.summary.keyword[1])));
            endEl(u"meta:keyword"_ustr);
        }
        if (hwpinfo.summary.etc[0][0])
        {
            startEl(u"meta:keyword"_ustr);
            chars(hstr2OUString(reinterpret_cast<hchar *>(hwpinfo.summary.etc[0])));
            endEl(u"meta:keyword"_ustr);
        }
        if (hwpinfo.summary.etc[1][0])
        {
            startEl(u"meta:keyword"_ustr);
            chars(hstr2OUString(reinterpret_cast<hchar *>(hwpinfo.summary.etc[1])));
            endEl(u"meta:keyword"_ustr);
        }
        if (hwpinfo.summary.etc[2][0])
        {
            startEl(u"meta:keyword"_ustr);
            chars(hstr2OUString(reinterpret_cast<hchar *>(hwpinfo.summary.etc[2])));
            endEl(u"meta:keyword"_ustr);
        }
        endEl(u"meta:keywords"_ustr);
    }
    endEl(u"office:meta"_ustr);
}


static struct
{
    const char *name;
    bool bMade;
}
ArrowShape[] =
{
    { "", false },
    { "Arrow", false },
    { "Line Arrow", false },
    { "Square", false }
};

struct
{
    double dots1;
    double dots2;
    double distance;
}


const LineStyle[] =
{
    { 0.0, 0.0, 0.0 },
    {
        0.34, 0., 0.272
    },
    { 0.17, 0., 0.136},
    {
        0.612, 0.17, 0.136
    },
    { 0.85, 0.17, 0.136}
};

void HwpReader::makeDrawMiscStyle( HWPDrawingObject *hdo )
{
    while( hdo )
    {
        if( hdo->child )
            makeDrawMiscStyle( hdo->child.get() );

        HWPDOProperty *prop = &hdo->property;
        if( hdo->type == HWPDO_CONTAINER )
        {
            hdo = hdo->next.get();
            continue;
        }

        if( prop->line_pstyle > 0 && prop->line_pstyle < 5 && prop->line_color <= 0xffffff)
        {
            mxList->addAttribute( u"draw:name"_ustr, sXML_CDATA, "LineType" + OUString::number(hdo->index));
            mxList->addAttribute( u"draw:style"_ustr, sXML_CDATA, u"round"_ustr);
            mxList->addAttribute( u"draw:dots1"_ustr, sXML_CDATA, u"1"_ustr);
            mxList->addAttribute( u"draw:dots1-length"_ustr, sXML_CDATA, OUString::number( LineStyle[prop->line_pstyle].dots1 * WTMM(prop->line_width) ) + "cm");
            if( prop->line_pstyle == 3 )
            {
                mxList->addAttribute( u"draw:dots2"_ustr, sXML_CDATA, u"1"_ustr);
                mxList->addAttribute( u"draw:dots2-length"_ustr, sXML_CDATA, OUString::number( LineStyle[prop->line_pstyle].dots2 * WTMM(prop->line_width) ) + "cm");
            }
            else if( prop->line_pstyle == 4 )
            {
                mxList->addAttribute( u"draw:dots2"_ustr, sXML_CDATA, u"2"_ustr);
                mxList->addAttribute( u"draw:dots2-length"_ustr, sXML_CDATA, OUString::number( LineStyle[prop->line_pstyle].dots2 * WTMM(prop->line_width)) + "cm");
            }
            mxList->addAttribute( u"draw:distance"_ustr, sXML_CDATA, OUString::number( LineStyle[prop->line_pstyle].distance * WTMM(prop->line_width)) + "cm");
            startEl(u"draw:stroke-dash"_ustr);
            mxList->clear();
            endEl(u"draw:stroke-dash"_ustr);
        }

        if( hdo->type == HWPDO_LINE || hdo->type == HWPDO_ARC || hdo->type == HWPDO_FREEFORM ||
            hdo->type == HWPDO_ADVANCED_ARC )
        {
            if( prop->line_tstyle > 0 &&
                o3tl::make_unsigned(prop->line_tstyle) < std::size(ArrowShape) &&
                !ArrowShape[prop->line_tstyle].bMade  )
            {
                ArrowShape[prop->line_tstyle].bMade = true;
                mxList->addAttribute(u"draw:name"_ustr, sXML_CDATA,
                    OUString::createFromAscii(ArrowShape[prop->line_tstyle].name));
                if( prop->line_tstyle == 1 )
                {
                    mxList->addAttribute(u"svg:viewBox"_ustr, sXML_CDATA, u"0 0 20 30"_ustr);
                    mxList->addAttribute(u"svg:d"_ustr, sXML_CDATA, u"m10 0-10 30h20z"_ustr);
                }
                else if( prop->line_tstyle == 2 )
                {
                    mxList->addAttribute(u"svg:viewBox"_ustr, sXML_CDATA, u"0 0 1122 2243"_ustr);
                    mxList->addAttribute(u"svg:d"_ustr, sXML_CDATA, u"m0 2108v17 17l12 42 30 34 38 21 43 4 29-8 30-21 25-26 13-34 343-1532 339 1520 13 42 29 34 39 21 42 4 42-12 34-30 21-42v-39-12l-4 4-440-1998-9-42-25-39-38-25-43-8-42 8-38 25-26 39-8 42z"_ustr);
                }
                else if( prop->line_tstyle == 3 )
                {
                    mxList->addAttribute(u"svg:viewBox"_ustr, sXML_CDATA, u"0 0 30 30"_ustr);
                    mxList->addAttribute(u"svg:d"_ustr, sXML_CDATA, u"m0 0h30v30h-30z"_ustr);
                }
                startEl(u"draw:marker"_ustr);
                mxList->clear();
                endEl(u"draw:marker"_ustr);
            }
            if (prop->line_hstyle > 0 &&
                o3tl::make_unsigned(prop->line_hstyle) < std::size(ArrowShape) &&
                !ArrowShape[prop->line_hstyle].bMade)
            {
                ArrowShape[prop->line_hstyle].bMade = true;
                mxList->addAttribute(u"draw:name"_ustr, sXML_CDATA,
                    OUString::createFromAscii(ArrowShape[prop->line_hstyle].name));
                if( prop->line_hstyle == 1 )
                {
                    mxList->addAttribute(u"svg:viewBox"_ustr, sXML_CDATA, u"0 0 20 30"_ustr);
                    mxList->addAttribute(u"svg:d"_ustr, sXML_CDATA, u"m10 0-10 30h20z"_ustr);
                }
                else if( prop->line_hstyle == 2 )
                {
                    mxList->addAttribute(u"svg:viewBox"_ustr, sXML_CDATA, u"0 0 1122 2243"_ustr);
                    mxList->addAttribute(u"svg:d"_ustr, sXML_CDATA, u"m0 2108v17 17l12 42 30 34 38 21 43 4 29-8 30-21 25-26 13-34 343-1532 339 1520 13 42 29 34 39 21 42 4 42-12 34-30 21-42v-39-12l-4 4-440-1998-9-42-25-39-38-25-43-8-42 8-38 25-26 39-8 42z"_ustr);
                }
                else if( prop->line_hstyle == 3 )
                {
                    mxList->addAttribute(u"svg:viewBox"_ustr, sXML_CDATA, u"0 0 20 20"_ustr);
                    mxList->addAttribute(u"svg:d"_ustr, sXML_CDATA, u"m0 0h20v20h-20z"_ustr);
                }
                startEl(u"draw:marker"_ustr);
                mxList->clear();
                endEl(u"draw:marker"_ustr);
            }
        }

        if( hdo->type != HWPDO_LINE )
        {
            if( prop->flag >> 18  & 0x01 )
            {
                mxList->addAttribute( u"draw:name"_ustr, sXML_CDATA, "fillimage" + OUString::number(hdo->index));

                EmPicture *emp = nullptr;
                if (prop->pictype && strlen(prop->szPatternFile) > 3)
                    emp = hwpfile.GetEmPictureByName(prop->szPatternFile);
                if (!emp)
                {
                    mxList->addAttribute( u"xlink:href"_ustr, sXML_CDATA,
                        hstr2OUString(kstr2hstr( reinterpret_cast<uchar const *>(urltounix(prop->szPatternFile).c_str())).c_str()));
                    mxList->addAttribute( u"xlink:type"_ustr, sXML_CDATA, u"simple"_ustr);
                    mxList->addAttribute( u"xlink:show"_ustr, sXML_CDATA, u"embed"_ustr);
                    mxList->addAttribute( u"xlink:actuate"_ustr, sXML_CDATA, u"onLoad"_ustr);
                }

                startEl(u"draw:fill-image"_ustr);
                mxList->clear();
                if (emp)
                {
                    startEl(u"office:binary-data"_ustr);
                    chars(base64_encode_string(emp->data.data(), emp->size));
                    endEl(u"office:binary-data"_ustr);
                }
                endEl(u"draw:fill-image"_ustr);
            }
/*  If there is a gradient, when a bitmap file is present, this is the first. */
            else if( prop->flag >> 16  & 0x01 )   /* existence gradient */
            {
                mxList->addAttribute( u"draw:name"_ustr, sXML_CDATA, "Grad" + OUString::number(hdo->index));
                switch( prop->gstyle )
                {
                    case 1 :
                        if( prop->center_y == 50 )
                            mxList->addAttribute( u"draw:style"_ustr, sXML_CDATA, u"axial"_ustr);
                        else
                            mxList->addAttribute( u"draw:style"_ustr, sXML_CDATA, u"linear"_ustr);
                        break;
                    case 2:
                    case 3:
                        mxList->addAttribute( u"draw:style"_ustr, sXML_CDATA, u"radial"_ustr);
                        break;
                    case 4:
                        mxList->addAttribute( u"draw:style"_ustr, sXML_CDATA, u"square"_ustr);
                        break;
                    default:
                        mxList->addAttribute( u"draw:style"_ustr, sXML_CDATA, u"linear"_ustr);
                        break;
                }
                mxList->addAttribute( u"draw:cx"_ustr, sXML_CDATA, OUString::number(prop->center_x) + "%");
                mxList->addAttribute( u"draw:cy"_ustr, sXML_CDATA, OUString::number(prop->center_y) + "%");

                HWPInfo& hwpinfo = hwpfile.GetHWPInfo();
                int default_color = 0xffffff;
                if( hwpinfo.back_info.isset )
                {
                             if( hwpinfo.back_info.color[0] > 0 || hwpinfo.back_info.color[1] > 0
                                     || hwpinfo.back_info.color[2] > 0 )
                                 default_color = hwpinfo.back_info.color[0] << 16 |
                                     hwpinfo.back_info.color[1] << 8 | hwpinfo.back_info.color[2];
                }

                if( prop->fromcolor > 0xffffff )
                    prop->fromcolor = default_color;
                if( prop->tocolor > 0xffffff )
                    prop->tocolor = default_color;

                if( prop->gstyle  == 1)
                {
                    if( prop->center_y == 100 )
                    {
                        mxList->addAttribute( u"draw:start-color"_ustr, sXML_CDATA, rgb2str( prop->tocolor ));
                        mxList->addAttribute( u"draw:end-color"_ustr, sXML_CDATA, rgb2str( prop->fromcolor ));
                    }
                    else
                    {
                        mxList->addAttribute( u"draw:start-color"_ustr, sXML_CDATA, rgb2str( prop->fromcolor ));
                        mxList->addAttribute( u"draw:end-color"_ustr, sXML_CDATA, rgb2str( prop->tocolor ));
                    }
                }
                else
                {
                    mxList->addAttribute( u"draw:start-color"_ustr, sXML_CDATA,rgb2str( prop->tocolor ));
                    mxList->addAttribute( u"draw:end-color"_ustr, sXML_CDATA,rgb2str( prop->fromcolor ));
                }
                if( prop->angle > 0 && ( prop->gstyle == 1 || prop->gstyle == 4))
                {
                    auto normangle = prop->angle % 360;
                    int angle = 1800 - normangle * 10;
                    mxList->addAttribute( u"draw:angle"_ustr, sXML_CDATA, OUString::number(angle));
                }
                startEl(u"draw:gradient"_ustr);
                mxList->clear();
                endEl(u"draw:gradient"_ustr);
            }
                                                  /* hatch */
            else if( prop->pattern_type >> 24 & 0x01 )
            {
                int type = prop->pattern_type & 0xffffff;
                mxList->addAttribute( u"draw:name"_ustr, sXML_CDATA, "Hatch" + OUString::number(hdo->index));
                if( type < 4 )
                    mxList->addAttribute( u"draw:style"_ustr, sXML_CDATA, u"single"_ustr );
                else
                    mxList->addAttribute( u"draw:style"_ustr, sXML_CDATA, u"double"_ustr );
                mxList->addAttribute( u"draw:color"_ustr, sXML_CDATA, rgb2str( static_cast<int32_t>(prop->pattern_color) ));
                mxList->addAttribute( u"draw:distance"_ustr, sXML_CDATA, u"0.12cm"_ustr);
                switch( type )
                {
                    case 0 :
                    case 4 :
                        mxList->addAttribute( u"draw:rotation"_ustr, sXML_CDATA, u"0"_ustr);
                        break;
                    case 1 :
                        mxList->addAttribute( u"draw:rotation"_ustr, sXML_CDATA, u"900"_ustr);
                        break;
                    case 2 :
                        mxList->addAttribute( u"draw:rotation"_ustr, sXML_CDATA, u"1350"_ustr);
                        break;
                    case 3 :
                    case 5 :
                        mxList->addAttribute( u"draw:rotation"_ustr, sXML_CDATA, u"450"_ustr);
                        break;
                }
                startEl(u"draw:hatch"_ustr);
                mxList->clear();
                endEl(u"draw:hatch"_ustr);
            }
        }
        hdo = hdo->next.get();
    }
}


void HwpReader::makeStyles()
{
    HWPStyle& hwpstyle = hwpfile.GetHWPStyle();

    startEl(u"office:styles"_ustr);

    int i;
    for (i = 0; i < hwpfile.getFBoxStyleCount(); i++)
    {
        if( hwpfile.getFBoxStyle(i)->boxtype == 'D' )
        {
            makeDrawMiscStyle(static_cast<HWPDrawingObject *>(hwpfile.getFBoxStyle(i)->cell) );
        }
    }

    mxList->addAttribute(u"style:name"_ustr, sXML_CDATA, u"Standard"_ustr);
    mxList->addAttribute(u"style:family"_ustr, sXML_CDATA, u"paragraph"_ustr);
    mxList->addAttribute(u"style:class"_ustr, sXML_CDATA, u"text"_ustr);
    startEl(u"style:style"_ustr);
    mxList->clear();

    mxList->addAttribute(u"fo:line-height"_ustr, sXML_CDATA, u"160%"_ustr);
    mxList->addAttribute(u"fo:text-align"_ustr, sXML_CDATA, u"justify"_ustr);
    startEl(u"style:properties"_ustr);
    mxList->clear();
    startEl(u"style:tab-stops"_ustr);

    for( i = 1 ; i < 40 ; i++)
    {
        mxList->addAttribute(u"style:position"_ustr, sXML_CDATA,
            OUString::number( WTI(1000 * i)) + "inch");
        startEl(u"style:tab-stop"_ustr);
        mxList->clear();
        endEl(u"style:tab-stop"_ustr);
    }
    endEl(u"style:tab-stops"_ustr);
    endEl(u"style:properties"_ustr);

    endEl(u"style:style"_ustr);

    for (int ii = 0; ii < hwpstyle.Num(); ii++)
    {
        unsigned char *stylename = reinterpret_cast<unsigned char *>(hwpstyle.GetName(ii));
        mxList->addAttribute(u"style:name"_ustr, sXML_CDATA, hstr2OUString(kstr2hstr(stylename).c_str()));
        mxList->addAttribute(u"style:family"_ustr, sXML_CDATA, u"paragraph"_ustr);
        mxList->addAttribute(u"style:parent-style-name"_ustr, sXML_CDATA, u"Standard"_ustr);

        startEl(u"style:style"_ustr);

        mxList->clear();

        parseCharShape(hwpstyle.GetCharShape(ii));
        parseParaShape(hwpstyle.GetParaShape(ii));

        startEl(u"style:properties"_ustr);
        mxList->clear();
        endEl(u"style:properties"_ustr);

        endEl(u"style:style"_ustr);
    }

    {
        mxList->addAttribute( u"style:name"_ustr, sXML_CDATA, u"Header"_ustr);
        mxList->addAttribute( u"style:family"_ustr, sXML_CDATA, u"paragraph"_ustr);
        mxList->addAttribute( u"style:parent-style-name"_ustr, sXML_CDATA, u"Standard"_ustr);
        mxList->addAttribute( u"style:class"_ustr, sXML_CDATA, u"extra"_ustr);
        startEl(u"style:style"_ustr);
        mxList->clear();
        endEl(u"style:style"_ustr);
    }

    {
        mxList->addAttribute( u"style:name"_ustr, sXML_CDATA, u"Footer"_ustr);
        mxList->addAttribute( u"style:family"_ustr, sXML_CDATA, u"paragraph"_ustr);
        mxList->addAttribute( u"style:parent-style-name"_ustr, sXML_CDATA, u"Standard"_ustr);
        mxList->addAttribute( u"style:class"_ustr, sXML_CDATA, u"extra"_ustr);
        startEl(u"style:style"_ustr);
        mxList->clear();

        endEl(u"style:style"_ustr);
    }

    if( hwpfile.linenumber > 0)
    {
        mxList->addAttribute( u"style:name"_ustr, sXML_CDATA, u"Horizontal Line"_ustr);
        mxList->addAttribute( u"style:family"_ustr, sXML_CDATA, u"paragraph"_ustr);
        mxList->addAttribute( u"style:parent-style-name"_ustr, sXML_CDATA, u"Standard"_ustr);
        mxList->addAttribute( u"style:class"_ustr, sXML_CDATA, u"html"_ustr);
        startEl(u"style:style"_ustr);
        mxList->clear();
        mxList->addAttribute( u"fo:font-size"_ustr, sXML_CDATA, u"6pt"_ustr);
        mxList->addAttribute( u"fo:margin-top"_ustr, sXML_CDATA, u"0cm"_ustr);
        mxList->addAttribute( u"fo:margin-bottom"_ustr, sXML_CDATA, u"0cm"_ustr);
        mxList->addAttribute( u"style:border-line-width-bottom"_ustr, sXML_CDATA, u"0.02cm 0.035cm 0.002cm"_ustr);
        mxList->addAttribute( u"fo:padding"_ustr, sXML_CDATA, u"0cm"_ustr);
        mxList->addAttribute( u"fo:border-bottom"_ustr, sXML_CDATA, u"0.039cm double #808080"_ustr);
        mxList->addAttribute( u"text:number-lines"_ustr, sXML_CDATA, u"false"_ustr);
        mxList->addAttribute( u"text:line-number"_ustr, sXML_CDATA, u"0"_ustr);
        mxList->addAttribute(u"fo:line-height"_ustr, sXML_CDATA, u"100%"_ustr);
        startEl(u"style:properties"_ustr);
        mxList->clear();
        endEl(u"style:properties"_ustr);
        endEl(u"style:style"_ustr);
    }

    HWPInfo& hwpinfo = hwpfile.GetHWPInfo();

    mxList->addAttribute(u"text:num-suffix"_ustr, sXML_CDATA, u")"_ustr);
    mxList->addAttribute(u"text:num-format"_ustr, sXML_CDATA, u"1"_ustr);
    if( hwpinfo.beginfnnum != 1)
        mxList->addAttribute(u"text:offset"_ustr, sXML_CDATA, OUString::number(hwpinfo.beginfnnum - 1));
    startEl(u"text:footnotes-configuration"_ustr);
    mxList->clear();
    endEl(u"text:footnotes-configuration"_ustr);

    endEl(u"office:styles"_ustr);
}


/**
 * parse automatic styles from hwpfile
 * Define a style that is automatically reflected. For example, defining styles of each paragraph, tables, header, and etc,. at here. In Body, use the defined style.
 * 1. supports for the styles of paragraph, text, fbox, and page.
 */
void HwpReader::makeAutoStyles()
{
    int i;

    startEl(u"office:automatic-styles"_ustr);

    for (i = 0; i < hwpfile.getParaShapeCount(); i++)
        makePStyle(hwpfile.getParaShape(i));

    for (i = 0; i < hwpfile.getCharShapeCount(); i++)
        makeTStyle(hwpfile.getCharShape(i));

    for( i = 0 ; i < hwpfile.getTableCount(); i++)
        makeTableStyle(hwpfile.getTable(i));

    for (i = 0; i < hwpfile.getFBoxStyleCount(); i++)
    {
        if( hwpfile.getFBoxStyle(i)->boxtype == 'D' )
            makeDrawStyle(static_cast<HWPDrawingObject *>(hwpfile.getFBoxStyle(i)->cell), hwpfile.getFBoxStyle(i));
        else
            makeFStyle(hwpfile.getFBoxStyle(i));
    }

    bool bIsLeft = false, bIsMiddle = false, bIsRight = false;
    for( i = 0 ; i < hwpfile.getPageNumberCount() ; i++ )
    {
        ShowPageNum *pn = hwpfile.getPageNumber(i);
        if( pn->where == 7 || pn->where == 8 )
        {
            bIsLeft = true;
            bIsRight = true;
        }
        else if( pn->where == 1 || pn->where == 4 )
        {
            bIsLeft = true;
        }
        else if( pn->where == 2 || pn->where == 5 )
        {
            bIsMiddle = true;
        }
        else if( pn->where == 3 || pn->where == 6 )
        {
            bIsRight = true;
        }
    }

    for( i = 1; i <= 3 ; i++ )
    {
        if( i == 1 && !bIsLeft )
            continue;
        if( i == 2 && !bIsMiddle )
            continue;
        if( i == 3 && !bIsRight )
            continue;
        mxList->addAttribute(u"style:name"_ustr, sXML_CDATA, "PNPara" + OUString::number(i));
        mxList->addAttribute(u"style:family"_ustr, sXML_CDATA, u"paragraph"_ustr);
        mxList->addAttribute(u"style:parent-style-name"_ustr, sXML_CDATA, u"Standard"_ustr);
        startEl(u"style:style"_ustr);
        mxList->clear();
        if( i == 1 )
            mxList->addAttribute(u"fo:text-align"_ustr, sXML_CDATA, u"start"_ustr);
        else if ( i == 2 )
            mxList->addAttribute(u"fo:text-align"_ustr, sXML_CDATA, u"center"_ustr);
        else if ( i == 3 )
            mxList->addAttribute(u"fo:text-align"_ustr, sXML_CDATA, u"end"_ustr);
        startEl(u"style:properties"_ustr);
        mxList->clear();
        endEl(u"style:properties"_ustr);
        endEl(u"style:style"_ustr);

        mxList->addAttribute(u"style:name"_ustr, sXML_CDATA, "PNBox" + OUString::number(i));
        mxList->addAttribute(u"style:family"_ustr, sXML_CDATA, u"graphics"_ustr);
        startEl(u"style:style"_ustr);
        mxList->clear();

        mxList->addAttribute(u"fo:margin-top"_ustr, sXML_CDATA, u"0cm"_ustr);
        mxList->addAttribute(u"fo:margin-bottom"_ustr, sXML_CDATA, u"0cm"_ustr);
        mxList->addAttribute(u"style:wrap"_ustr, sXML_CDATA, u"run-through"_ustr);
        mxList->addAttribute(u"style:vertical-pos"_ustr, sXML_CDATA, u"from-top"_ustr);
        mxList->addAttribute(u"style:vertical-rel"_ustr, sXML_CDATA, u"paragraph"_ustr);

        if( i == 1 )
            mxList->addAttribute(u"style:horizontal-pos"_ustr, sXML_CDATA, u"left"_ustr);
        else if ( i == 2 )
            mxList->addAttribute(u"style:horizontal-pos"_ustr, sXML_CDATA, u"center"_ustr);
        else if ( i == 3 )
            mxList->addAttribute(u"style:horizontal-pos"_ustr, sXML_CDATA, u"right"_ustr);
        mxList->addAttribute(u"style:horizontal-rel"_ustr, sXML_CDATA, u"paragraph"_ustr);
        mxList->addAttribute(u"fo:padding"_ustr, sXML_CDATA, u"0cm"_ustr);
        mxList->addAttribute(u"stylefamily"_ustr, sXML_CDATA, u"graphics"_ustr);
        startEl(u"style:properties"_ustr);
        mxList->clear();
        endEl(u"style:properties"_ustr);
        endEl(u"style:style"_ustr);
    }

    for (i = 0; i < hwpfile.getDateFormatCount(); i++)
        makeDateFormat(hwpfile.getDateCode(i));

    makePageStyle();

    endEl(u"office:automatic-styles"_ustr);
}

namespace {

struct PageSetting
{
    PageSetting()
    {
        header = nullptr;
        header_odd = nullptr;
        header_even = nullptr;
        footer = nullptr;
        footer_odd = nullptr;
        footer_even = nullptr;
        pagenumber=nullptr;
        bIsSet = false;
    }
    HeaderFooter *header ;
    HeaderFooter *header_odd ;
    HeaderFooter *header_even ;
    HeaderFooter *footer ;
    HeaderFooter *footer_odd ;
    HeaderFooter *footer_even ;
    ShowPageNum *pagenumber;
    bool bIsSet;
};

}

void HwpReader::makeMasterStyles()
{
    startEl(u"office:master-styles"_ustr);

    int i;
    int nMax = hwpfile.getMaxSettedPage();
    std::vector<PageSetting> aSet(nMax + 1);

    for( i = 0 ; i < hwpfile.getPageNumberCount() ; i++ )
    {
        ShowPageNum *pn = hwpfile.getPageNumber(i);
        aSet[pn->m_nPageNumber].pagenumber = pn;
        aSet[pn->m_nPageNumber].bIsSet = true;
    }
    for( i = 0 ; i < hwpfile.getHeaderFooterCount() ; i++ )
    {
        HeaderFooter* hf = hwpfile.getHeaderFooter(i);
        aSet[hf->m_nPageNumber].bIsSet = true;
        if( hf->type == 0 )                       // header
        {
            switch( hf->where )
            {
                case 0 :
                    aSet[hf->m_nPageNumber].header = hf;
                    aSet[hf->m_nPageNumber].header_even = nullptr;
                    aSet[hf->m_nPageNumber].header_odd = nullptr;
                    break;
                case 1:
                    aSet[hf->m_nPageNumber].header_even = hf;
                    if( aSet[hf->m_nPageNumber].header )
                    {
                        aSet[hf->m_nPageNumber].header_odd =
                            aSet[hf->m_nPageNumber].header;
                        aSet[hf->m_nPageNumber].header = nullptr;
                    }
                    break;
                case 2:
                    aSet[hf->m_nPageNumber].header_odd = hf;
                    if( aSet[hf->m_nPageNumber].header )
                    {
                        aSet[hf->m_nPageNumber].header_even =
                            aSet[hf->m_nPageNumber].header;
                        aSet[hf->m_nPageNumber].header = nullptr;
                    }
                    break;
            }
        }
        else                                      // footer
        {
            switch( hf->where )
            {
                case 0 :
                    aSet[hf->m_nPageNumber].footer = hf;
                    aSet[hf->m_nPageNumber].footer_even = nullptr;
                    aSet[hf->m_nPageNumber].footer_odd = nullptr;
                    break;
                case 1:
                    aSet[hf->m_nPageNumber].footer_even = hf;
                    if( aSet[hf->m_nPageNumber].footer )
                    {
                        aSet[hf->m_nPageNumber].footer_odd =
                            aSet[hf->m_nPageNumber].footer;
                        aSet[hf->m_nPageNumber].footer = nullptr;
                    }
                    break;
                case 2:
                    aSet[hf->m_nPageNumber].footer_odd = hf;
                    if( aSet[hf->m_nPageNumber].footer )
                    {
                        aSet[hf->m_nPageNumber].footer_even =
                            aSet[hf->m_nPageNumber].footer;
                        aSet[hf->m_nPageNumber].footer = nullptr;
                    }
                    break;
            }
        }
    }

    PageSetting *pPrevSet = nullptr;
    PageSetting *pPage = nullptr;

    if (nMax > 512 && comphelper::IsFuzzing())
    {
        SAL_WARN("filter.hwp", "too many pages: " << nMax << " clip to " << 512);
        nMax = 512;
    }

    for( i = 1; i <= nMax ; i++ )
    {
        if( i == 1 )
            mxList->addAttribute(u"style:name"_ustr, sXML_CDATA, u"Standard"_ustr);
        else
            mxList->addAttribute(u"style:name"_ustr, sXML_CDATA, "p" + OUString::number(i));
        mxList->addAttribute(u"style:page-master-name"_ustr, sXML_CDATA,
                "pm" + OUString::number(hwpfile.GetPageMasterNum(i)));
        if( i < nMax )
            mxList->addAttribute(u"style:next-style-name"_ustr, sXML_CDATA, "p" + OUString::number(i + 1));
        mxList->addAttribute(u"draw:style-name"_ustr, sXML_CDATA, "master" + OUString::number(i));
        startEl(u"style:master-page"_ustr);
        mxList->clear();

        if( aSet[i].bIsSet )                      /* If you've changed the current setting */
        {
            if( !aSet[i].pagenumber ){
                    if( pPrevSet && pPrevSet->pagenumber )
                         aSet[i].pagenumber = pPrevSet->pagenumber;
            }
            if( aSet[i].pagenumber )
            {
                if( aSet[i].pagenumber->where == 7 && aSet[i].header )
                {
                    aSet[i].header_even = aSet[i].header;
                    aSet[i].header_odd = aSet[i].header;
                    aSet[i].header = nullptr;
                }
                if( aSet[i].pagenumber->where == 8 && aSet[i].footer )
                {
                    aSet[i].footer_even = aSet[i].footer;
                    aSet[i].footer_odd = aSet[i].footer;
                    aSet[i].footer = nullptr;
                }
            }

            if( !aSet[i].header_even && pPrevSet && pPrevSet->header_even )
            {
                aSet[i].header_even = pPrevSet->header_even;
            }
            if( !aSet[i].header_odd && pPrevSet && pPrevSet->header_odd )
            {
                aSet[i].header_odd = pPrevSet->header_odd;
            }
            if( !aSet[i].footer_even && pPrevSet && pPrevSet->footer_even )
            {
                aSet[i].footer_even = pPrevSet->footer_even;
            }
            if( !aSet[i].footer_odd && pPrevSet && pPrevSet->footer_odd )
            {
                aSet[i].footer_odd = pPrevSet->footer_odd;
            }

            pPage = &aSet[i];
            pPrevSet = &aSet[i];
        }
        else if( pPrevSet )                       /* If the previous setting exists */
        {
            pPage = pPrevSet;
        }
        else                                      /* If the previous settings doesn't exist, set to the default settings */
        {
            startEl(u"style:header"_ustr);
            mxList->addAttribute(u"text:style-name"_ustr, sXML_CDATA, u"Standard"_ustr);
            startEl(u"text:p"_ustr);
            mxList->clear();
            endEl(u"text:p"_ustr);
            endEl(u"style:header"_ustr);

            startEl(u"style:footer"_ustr);
            mxList->addAttribute(u"text:style-name"_ustr, sXML_CDATA, u"Standard"_ustr);
            startEl(u"text:p"_ustr);
            mxList->clear();
            endEl(u"text:p"_ustr);
            endEl(u"style:footer"_ustr);

            endEl(u"style:master-page"_ustr);

            continue;
        }
// header
        if( pPage->header )
        {
            startEl(u"style:header"_ustr);
            if( pPage->pagenumber && pPage->pagenumber->where < 4 )
            {
                d->bInHeader = true;
                d->pPn = pPage->pagenumber;
            }
            if (!pPage->header->plist.empty())
                parsePara(pPage->header->plist.front().get());
            d->bInHeader = false;
            d->pPn = nullptr;
            endEl(u"style:header"_ustr);
        }
        if( pPage->header_even )
        {
            startEl(u"style:header"_ustr);
            if( pPage->pagenumber && ( pPage->pagenumber->where < 4
                || pPage->pagenumber->where == 7 ) )
            {
                d->bInHeader = true;
                d->pPn = pPage->pagenumber;
                d->nPnPos = 3;
            }
            if (!pPage->header_even->plist.empty())
                parsePara(pPage->header_even->plist.front().get());
            d->bInHeader = false;
            d->pPn = nullptr;
            d->nPnPos = 0;
            endEl(u"style:header"_ustr);
        }
                                                  /* Will be the default. */
        else if (pPage->header_odd)
        {
            startEl(u"style:header"_ustr);
            mxList->addAttribute(u"text:style-name"_ustr, sXML_CDATA, u"Standard"_ustr);
            startEl(u"text:p"_ustr);
            mxList->clear();
            if( pPage->pagenumber && ( pPage->pagenumber->where < 4 ||
                pPage->pagenumber->where == 7 ) )
            {
                d->pPn = pPage->pagenumber;
                d->nPnPos = 3;
                makeShowPageNum();
                d->pPn = nullptr;
                d->nPnPos = 0;
            }
            endEl(u"text:p"_ustr);
            endEl(u"style:header"_ustr);
        }
        if( pPage->header_odd )
        {
            startEl(u"style:header-left"_ustr);
            if( pPage->pagenumber && ( pPage->pagenumber->where < 4
                || pPage->pagenumber->where == 7 ) )
            {
                d->bInHeader = true;
                d->nPnPos = 1;
                d->pPn = pPage->pagenumber;
            }
            if (!pPage->header_odd->plist.empty())
                parsePara(pPage->header_odd->plist.front().get());
            d->bInHeader = false;
            d->pPn = nullptr;
            d->nPnPos = 0;
            endEl(u"style:header-left"_ustr);
        }
                                                  /* Will be the default.  */
        else if (pPage->header_even)
        {
            startEl(u"style:header-left"_ustr);
            mxList->addAttribute(u"text:style-name"_ustr, sXML_CDATA, u"Standard"_ustr);
            startEl(u"text:p"_ustr);
            mxList->clear();
            if( pPage->pagenumber && ( pPage->pagenumber->where < 4 ||
                pPage->pagenumber->where == 7 ) )
            {
                d->pPn = pPage->pagenumber;
                d->nPnPos = 1;
                makeShowPageNum();
                d->pPn = nullptr;
                d->nPnPos = 0;
            }
            endEl(u"text:p"_ustr);
            endEl(u"style:header-left"_ustr);
        }
        if( !pPage->header && !pPage->header_even && !pPage->header_odd )
        {
            startEl(u"style:header"_ustr);
            mxList->addAttribute(u"text:style-name"_ustr, sXML_CDATA, u"Standard"_ustr);
            startEl(u"text:p"_ustr);
            mxList->clear();
            if( pPage->pagenumber && (pPage->pagenumber->where < 4 ||
                pPage->pagenumber->where == 7 ) )
            {
                d->pPn = pPage->pagenumber;
                makeShowPageNum();
                d->pPn = nullptr;
            }
            endEl(u"text:p"_ustr);
            endEl(u"style:header"_ustr);
        }
// footer
        if( pPage->footer )
        {
            startEl(u"style:footer"_ustr);
            if( pPage->pagenumber && pPage->pagenumber->where >= 4
                && pPage->pagenumber->where != 7 )
            {
                d->bInHeader = true;
                d->pPn = pPage->pagenumber;
            }
            if (!pPage->footer->plist.empty())
                parsePara(pPage->footer->plist.front().get());
            d->bInHeader = false;
            d->pPn = nullptr;
            endEl(u"style:footer"_ustr);
        }
        if( pPage->footer_even )
        {
            startEl(u"style:footer"_ustr);
            if( pPage->pagenumber && pPage->pagenumber->where >= 4
                && pPage->pagenumber->where != 7 )
            {
                d->bInHeader = true;
                d->pPn = pPage->pagenumber;
                d->nPnPos = 3;
            }
            if (!pPage->footer_even->plist.empty())
                parsePara(pPage->footer_even->plist.front().get());
            d->bInHeader = false;
            d->pPn = nullptr;
            d->nPnPos = 0;
            endEl(u"style:footer"_ustr);
        }
                                                  /* Will be the default. */
        else if (pPage->footer_odd)
        {
            startEl(u"style:footer"_ustr);
            mxList->addAttribute(u"text:style-name"_ustr, sXML_CDATA, u"Standard"_ustr);
            startEl(u"text:p"_ustr);
            mxList->clear();
            if( pPage->pagenumber && pPage->pagenumber->where >= 4
                && pPage->pagenumber->where != 7 )
            {
                d->pPn = pPage->pagenumber;
                d->nPnPos = 3;
                makeShowPageNum();
                d->pPn = nullptr;
                d->nPnPos = 0;
            }
            endEl(u"text:p"_ustr);
            endEl(u"style:footer"_ustr);
        }
        if( pPage->footer_odd )
        {
            startEl(u"style:footer-left"_ustr);
            if( pPage->pagenumber && pPage->pagenumber->where >= 4
                && pPage->pagenumber->where != 7 )
            {
                d->bInHeader = true;
                d->pPn = pPage->pagenumber;
                d->nPnPos = 1;
            }
            if (!pPage->footer_odd->plist.empty())
                parsePara(pPage->footer_odd->plist.front().get());
            d->bInHeader = false;
            d->pPn = nullptr;
            d->nPnPos = 0;
            endEl(u"style:footer-left"_ustr);
        }
                                                  /* Will be the default. */
        else if (pPage->footer_even)
        {
            startEl(u"style:footer-left"_ustr);
            mxList->addAttribute(u"text:style-name"_ustr, sXML_CDATA, u"Standard"_ustr);
            startEl(u"text:p"_ustr);
            mxList->clear();
            if( pPage->pagenumber && pPage->pagenumber->where >= 4
                && pPage->pagenumber->where != 7 )
            {
                d->pPn = pPage->pagenumber;
                d->nPnPos = 1;
                makeShowPageNum();
                d->pPn = nullptr;
                d->nPnPos = 0;
            }
            endEl(u"text:p"_ustr);
            endEl(u"style:footer-left"_ustr);
        }
        if( !pPage->footer && !pPage->footer_even && !pPage->footer_odd )
        {
            startEl(u"style:footer"_ustr);
            mxList->addAttribute(u"text:style-name"_ustr, sXML_CDATA, u"Standard"_ustr);
            startEl(u"text:p"_ustr);
            mxList->clear();
            if( pPage->pagenumber && pPage->pagenumber->where >= 4
                && pPage->pagenumber->where != 7 )
            {
                d->pPn = pPage->pagenumber;
                makeShowPageNum();
                d->pPn = nullptr;
            }
            endEl(u"text:p"_ustr);
            endEl(u"style:footer"_ustr);
        }

        endEl(u"style:master-page"_ustr);
    }
    endEl(u"office:master-styles"_ustr);
}


/**
 * Create the properties for text styles.
 * 1. fo:font-size, fo:font-family, fo:letter-spacing, fo:color,
 *    style:text-background-color, fo:font-style, fo:font-weight,
 *    style:text-underline,style:text-outline,fo:text-shadow,style:text-position
 *    Support them.
 */
void HwpReader::parseCharShape(CharShape const * cshape)
{
    HWPFont& hwpfont = hwpfile.GetHWPFont();

    mxList->addAttribute(u"fo:font-size"_ustr, sXML_CDATA, OUString::number(cshape->size / 25) + "pt");
    mxList->addAttribute(u"style:font-size-asian"_ustr, sXML_CDATA, OUString::number(cshape->size / 25) + "pt");

    ::std::string const tmp = hstr2ksstr(kstr2hstr(
        reinterpret_cast<unsigned char const *>(hwpfont.GetFontName(0, cshape->font))).c_str());
    double fRatio = 1.0;
    int size = getRepFamilyName(tmp.c_str(), d->buf, fRatio);

    mxList->addAttribute(u"fo:font-family"_ustr, sXML_CDATA,
        OUString(d->buf, size, RTL_TEXTENCODING_EUC_KR));
    mxList->addAttribute(u"style:font-family-asian"_ustr, sXML_CDATA,
        OUString(d->buf, size, RTL_TEXTENCODING_EUC_KR));

    mxList->addAttribute(u"style:text-scale"_ustr, sXML_CDATA,
        OUString::number(static_cast<int>(cshape->ratio * fRatio)) + "%");

    double sspace = (cshape->size / 25) * cshape->space / 100.;

    if (sspace != 0.)
    {
        mxList->addAttribute(u"fo:letter-spacing"_ustr, sXML_CDATA,
            OUString::number(sspace) + "pt");
    }
    if (cshape->color[1] != 0)
        mxList->addAttribute(u"fo:color"_ustr, sXML_CDATA,
            hcolor2str(cshape->color[1], 100, true));
    if (cshape->shade != 0)
        mxList->addAttribute(u"style:text-background-color"_ustr, sXML_CDATA,
            hcolor2str(cshape->color[0], cshape->shade));
    if (cshape->attr & 0x01)
    {
        mxList->addAttribute(u"fo:font-style"_ustr, sXML_CDATA, u"italic"_ustr);
        mxList->addAttribute(u"style:font-style-asian"_ustr, sXML_CDATA, u"italic"_ustr);
    }
    else{
        mxList->addAttribute(u"fo:font-style"_ustr, sXML_CDATA, u"normal"_ustr);
        mxList->addAttribute(u"style:font-style-asian"_ustr, sXML_CDATA, u"normal"_ustr);
    }
    if (cshape->attr >> 1 & 0x01)
    {
        mxList->addAttribute(u"fo:font-weight"_ustr, sXML_CDATA, u"bold"_ustr);
        mxList->addAttribute(u"style:font-weight-asian"_ustr, sXML_CDATA, u"bold"_ustr);
    }
    else{
        mxList->addAttribute(u"fo:font-weight"_ustr, sXML_CDATA, u"normal"_ustr);
        mxList->addAttribute(u"style:font-weight-asian"_ustr, sXML_CDATA, u"normal"_ustr);
    }
    if (cshape->attr >> 2 & 0x01)
        mxList->addAttribute(u"style:text-underline"_ustr, sXML_CDATA, u"single"_ustr);
    if (cshape->attr >> 3 & 0x01)
        mxList->addAttribute(u"style:text-outline"_ustr, sXML_CDATA, u"true"_ustr);
    if (cshape->attr >> 4 & 0x01)
        mxList->addAttribute(u"fo:text-shadow"_ustr, sXML_CDATA, u"1pt 1pt"_ustr);
    if (cshape->attr >> 5 & 0x01)
        mxList->addAttribute(u"style:text-position"_ustr, sXML_CDATA, u"super 58%"_ustr);
    if (cshape->attr >> 6 & 0x01)
        mxList->addAttribute(u"style:text-position"_ustr, sXML_CDATA, u"sub 58%"_ustr);

}


/**
 * Create the properties that correspond to the real Paragraph.
 * 1. fo:margin-left,fo:margin-right,fo:margin-top, fo:margin-bottom,
 *    fo:text-indent, fo:line-height, fo:text-align, fo:border
 *    are implemented.
 * TODO: Tab Settings => set values of properties only which doesn't have the default value
 */
void HwpReader::parseParaShape(ParaShape const * pshape)
{

    if (pshape->left_margin != 0)
        mxList->addAttribute(u"fo:margin-left"_ustr, sXML_CDATA, OUString::number
            (WTI(pshape->left_margin )) + "inch");
    if (pshape->right_margin != 0)
        mxList->addAttribute(u"fo:margin-right"_ustr, sXML_CDATA, OUString::number
            (WTI(pshape->right_margin)) + "inch");
    if (pshape->pspacing_prev != 0)
        mxList->addAttribute(u"fo:margin-top"_ustr, sXML_CDATA, OUString::number
            (WTI(pshape->pspacing_prev)) + "inch");
    if (pshape->pspacing_next != 0)
        mxList->addAttribute(u"fo:margin-bottom"_ustr, sXML_CDATA, OUString::number
            (WTI(pshape->pspacing_next)) + "inch");
    if (pshape->indent != 0)
        mxList->addAttribute(u"fo:text-indent"_ustr, sXML_CDATA, OUString::number
            (WTI(pshape->indent)) + "inch");
    if (pshape->lspacing != 0)
        mxList->addAttribute(u"fo:line-height"_ustr, sXML_CDATA, OUString::number(pshape->lspacing) + "%");

    const char* align = nullptr;

    switch (static_cast<int>(pshape->arrange_type))
    {
        case 1:
            align = "start";
            break;
        case 2:
            align = "end";
            break;
        case 3:
            align = "center";
            break;
        case 4:
        case 5:
        case 6:
            align = "justify";
            break;
    }

    if (align)
        mxList->addAttribute(u"fo:text-align"_ustr, sXML_CDATA, OUString::createFromAscii(align));

    if (pshape->outline)
        mxList->addAttribute(u"fo:border"_ustr, sXML_CDATA, u"0.002cm solid #000000"_ustr);
    if( pshape->shade > 0 )
    {
        mxList->addAttribute(u"fo:background-color"_ustr, sXML_CDATA,
            hcolor2str(0, pshape->shade));
    }

    if( pshape->pagebreak & 0x02 || pshape->pagebreak & 0x04)
        mxList->addAttribute(u"fo:break-before"_ustr, sXML_CDATA, u"page"_ustr);
    else if( pshape->pagebreak & 0x01 )
        mxList->addAttribute(u"fo:break-before"_ustr, sXML_CDATA, u"column"_ustr);

}


/**
 * Make the style of the Paragraph.
 */
void HwpReader::makePStyle(ParaShape const * pshape)
{
    int nscount = pshape->tabs[MAXTABS -1].type;
    mxList->addAttribute(u"style:name"_ustr, sXML_CDATA, "P" + OUString::number(pshape->index));
    mxList->addAttribute(u"style:family"_ustr, sXML_CDATA, u"paragraph"_ustr);
    startEl(u"style:style"_ustr);
    mxList->clear();
    parseParaShape(pshape);
    if (pshape->cshape)
        parseCharShape(pshape->cshape.get());
    startEl(u"style:properties"_ustr);
    mxList->clear();

    if( nscount )
    {
        unsigned char tf = 0;
        startEl(u"style:tab-stops"_ustr);

        int tab_margin = pshape->left_margin + pshape->indent;
        if( tab_margin < 0 )
              tab_margin = 0;
        for( int i = 0 ; i < MAXTABS -1 ; i++)
        {
            if( i > 0 && pshape->tabs[i].position == 0. )
                break;
            if( pshape->tabs[i].position <= tab_margin )
                continue;
            mxList->addAttribute(u"style:position"_ustr, sXML_CDATA,
                OUString::number(WTMM(pshape->tabs[i].position - tab_margin )) + "mm");
            if( pshape->tabs[i].type )
            {
                tf = 1;
                switch(pshape->tabs[i].type)
                {
                    case 1 :
                        mxList->addAttribute(u"style:type"_ustr, sXML_CDATA, u"right"_ustr);
                        break;
                    case 2:
                        mxList->addAttribute(u"style:type"_ustr, sXML_CDATA, u"center"_ustr);
                        break;
                    case 3:
                        mxList->addAttribute(u"style:type"_ustr, sXML_CDATA, u"char"_ustr);
                        mxList->addAttribute(u"style:char"_ustr, sXML_CDATA, u"."_ustr);
                        break;
                }
            }
            if( pshape->tabs[i].dot_continue )
            {
                tf = 1;
                mxList->addAttribute(u"style:leader-char"_ustr, sXML_CDATA, u"."_ustr);
            }
            startEl(u"style:tab-stop"_ustr);
            mxList->clear();
            endEl(u"style:tab-stop"_ustr);

            if( (pshape->tabs[i].position != 1000 * i ) || tf )
            {
                if( !--nscount ) break;
            }
        }
        endEl(u"style:tab-stops"_ustr);
    }
    endEl(u"style:properties"_ustr);
    endEl(u"style:style"_ustr);
}


/**
 * Create a style for the page. This includes the header/footer, footnote and more.
 * TODO: fo: background-color (no information)
 */
void HwpReader::makePageStyle()
{
     HWPInfo& hwpinfo = hwpfile.GetHWPInfo();
     int pmCount = hwpfile.getColumnCount();

     if (pmCount > 512 && comphelper::IsFuzzing())
     {
         SAL_WARN("filter.hwp", "too many pages: " << pmCount << " clip to " << 512);
         pmCount = 512;
     }

     for( int i = 0 ; i < pmCount ; i++ ){
         mxList->addAttribute(u"style:name"_ustr, sXML_CDATA, "pm" + OUString::number(i + 1));
         startEl(u"style:page-master"_ustr);
         mxList->clear();


         switch( hwpinfo.paper.paper_kind )
         {
              case 3:                                   // A4
                    if( hwpinfo.paper.paper_direction )
                    {
                         mxList->addAttribute(u"fo:page-height"_ustr,sXML_CDATA, u"210mm"_ustr);
                         mxList->addAttribute(u"fo:page-width"_ustr,sXML_CDATA, u"297mm"_ustr);
                    }
                    else
                    {
                         mxList->addAttribute(u"fo:page-width"_ustr,sXML_CDATA, u"210mm"_ustr);
                         mxList->addAttribute(u"fo:page-height"_ustr,sXML_CDATA, u"297mm"_ustr);
                    }
                    break;
              case 4:                                   // 80 column
                    if( hwpinfo.paper.paper_direction )
                    {
                         mxList->addAttribute(u"fo:page-height"_ustr,sXML_CDATA, u"8.5inch"_ustr);
                         mxList->addAttribute(u"fo:page-width"_ustr,sXML_CDATA, u"11inch"_ustr);
                    }
                    else
                    {
                         mxList->addAttribute(u"fo:page-width"_ustr,sXML_CDATA, u"8.5inch"_ustr);
                         mxList->addAttribute(u"fo:page-height"_ustr,sXML_CDATA, u"11inch"_ustr);
                    }
                    break;
              case 5:                                   // B5
                    if( hwpinfo.paper.paper_direction )
                    {
                         mxList->addAttribute(u"fo:page-height"_ustr,sXML_CDATA, u"176mm"_ustr);
                         mxList->addAttribute(u"fo:page-width"_ustr,sXML_CDATA, u"250mm"_ustr);
                    }
                    else
                    {
                         mxList->addAttribute(u"fo:page-width"_ustr,sXML_CDATA, u"176mm"_ustr);
                         mxList->addAttribute(u"fo:page-height"_ustr,sXML_CDATA, u"250mm"_ustr);
                    }
                    break;
              case 6:                                   // B4
                    if( hwpinfo.paper.paper_direction )
                    {
                         mxList->addAttribute(u"fo:page-height"_ustr,sXML_CDATA, u"250mm"_ustr);
                         mxList->addAttribute(u"fo:page-width"_ustr,sXML_CDATA, u"353mm"_ustr);
                    }
                    else
                    {
                         mxList->addAttribute(u"fo:page-width"_ustr,sXML_CDATA, u"250mm"_ustr);
                         mxList->addAttribute(u"fo:page-height"_ustr,sXML_CDATA, u"353mm"_ustr);
                    }
                    break;
              case 7:
                    if( hwpinfo.paper.paper_direction )
                    {
                         mxList->addAttribute(u"fo:page-height"_ustr,sXML_CDATA, u"8.5inch"_ustr);
                         mxList->addAttribute(u"fo:page-width"_ustr,sXML_CDATA, u"14inch"_ustr);
                    }
                    else
                    {
                         mxList->addAttribute(u"fo:page-width"_ustr,sXML_CDATA, u"8.5inch"_ustr);
                         mxList->addAttribute(u"fo:page-height"_ustr,sXML_CDATA, u"14inch"_ustr);
                    }
                    break;
              case 8:
                    if( hwpinfo.paper.paper_direction )
                    {
                         mxList->addAttribute(u"fo:page-height"_ustr,sXML_CDATA, u"297mm"_ustr);
                         mxList->addAttribute(u"fo:page-width"_ustr,sXML_CDATA, u"420mm"_ustr);
                    }
                    else
                    {
                         mxList->addAttribute(u"fo:page-width"_ustr,sXML_CDATA, u"297mm"_ustr);
                         mxList->addAttribute(u"fo:page-height"_ustr,sXML_CDATA, u"420mm"_ustr);
                    }
                    break;
              case 0:
              case 1:
              case 2:
              default:
                    if( hwpinfo.paper.paper_direction )
                    {
                         mxList->addAttribute(u"fo:page-width"_ustr,sXML_CDATA,
                              OUString::number(WTI(hwpinfo.paper.paper_height)) + "inch");
                         mxList->addAttribute(u"fo:page-height"_ustr,sXML_CDATA,
                              OUString::number(WTI(hwpinfo.paper.paper_width)) + "inch");
                    }
                    else
                    {
                         mxList->addAttribute(u"fo:page-width"_ustr,sXML_CDATA,
                              OUString::number(WTI(hwpinfo.paper.paper_width)) + "inch");
                         mxList->addAttribute(u"fo:page-height"_ustr,sXML_CDATA,
                              OUString::number(WTI(hwpinfo.paper.paper_height)) + "inch");
                    }
                    break;

         }

         mxList->addAttribute(u"style:print-orientation"_ustr,sXML_CDATA,
              OUString::createFromAscii(hwpinfo.paper.paper_direction ? "landscape" : "portrait"));
         if( hwpinfo.beginpagenum != 1)
              mxList->addAttribute(u"style:first-page-number"_ustr,sXML_CDATA, OUString::number(hwpinfo.beginpagenum));

         if( hwpinfo.borderline ){
             mxList->addAttribute(u"fo:margin-left"_ustr,sXML_CDATA,
                  OUString::number(WTI(hwpinfo.paper.left_margin - hwpinfo.bordermargin[0] + hwpinfo.paper.gutter_length)) + "inch");
             mxList->addAttribute(u"fo:margin-right"_ustr,sXML_CDATA,
                  OUString::number(WTI(hwpinfo.paper.right_margin - hwpinfo.bordermargin[1])) + "inch");
             mxList->addAttribute(u"fo:margin-top"_ustr,sXML_CDATA,
                  OUString::number(WTI(hwpinfo.paper.top_margin - hwpinfo.bordermargin[2])) + "inch");
             mxList->addAttribute(u"fo:margin-bottom"_ustr,sXML_CDATA,
                  OUString::number(WTI(hwpinfo.paper.bottom_margin - hwpinfo.bordermargin[3])) + "inch");
         }
         else{
             mxList->addAttribute(u"fo:margin-left"_ustr,sXML_CDATA,
                  OUString::number(WTI(hwpinfo.paper.left_margin + hwpinfo.paper.gutter_length)) + "inch");
             mxList->addAttribute(u"fo:margin-right"_ustr,sXML_CDATA,
                  OUString::number(WTI(hwpinfo.paper.right_margin)) + "inch");
             mxList->addAttribute(u"fo:margin-top"_ustr,sXML_CDATA,
                  OUString::number(WTI(hwpinfo.paper.top_margin)) + "inch");
             mxList->addAttribute(u"fo:margin-bottom"_ustr,sXML_CDATA,
                  OUString::number(WTI(hwpinfo.paper.bottom_margin)) + "inch");
         }

         switch( hwpinfo.borderline )
         {
              case 1:
                    mxList->addAttribute(u"fo:border"_ustr, sXML_CDATA,u"0.002cm solid #000000"_ustr);
                    break;
              case 3:
                    mxList->addAttribute(u"fo:border"_ustr, sXML_CDATA,u"0.002cm dotted #000000"_ustr);
                    break;
              case 2:
                    mxList->addAttribute(u"fo:border"_ustr, sXML_CDATA,u"0.035cm solid #000000"_ustr);
                    break;
              case 4:
                    mxList->addAttribute(u"style:border-line-width"_ustr, sXML_CDATA,u"0.002cm 0.035cm 0.002cm"_ustr);
                    mxList->addAttribute(u"fo:border"_ustr, sXML_CDATA,u"0.039cm double #000000"_ustr);
                    break;
         }

         mxList->addAttribute(u"fo:padding-left"_ustr, sXML_CDATA,
              OUString::number(WTI(hwpinfo.bordermargin[0])) + "inch");
         mxList->addAttribute(u"fo:padding-right"_ustr, sXML_CDATA,
              OUString::number(WTI(hwpinfo.bordermargin[1])) + "inch");
         mxList->addAttribute(u"fo:padding-top"_ustr, sXML_CDATA,
              OUString::number(WTI(hwpinfo.bordermargin[2])) + "inch");
         mxList->addAttribute(u"fo:padding-bottom"_ustr, sXML_CDATA,
              OUString::number(WTI(hwpinfo.bordermargin[3])) + "inch");

     /* background color */
         if( hwpinfo.back_info.isset )
         {
             if( hwpinfo.back_info.color[0] > 0 || hwpinfo.back_info.color[1] > 0
                     || hwpinfo.back_info.color[2] > 0 ){
                 mxList->addAttribute(u"fo:background-color"_ustr, sXML_CDATA,
                                      rgb2str(hwpinfo.back_info.color[0],
                                              hwpinfo.back_info.color[1],
                                              hwpinfo.back_info.color[2]));
             }
         }

         startEl(u"style:properties"_ustr);
         mxList->clear();

     /* background image */
         if( hwpinfo.back_info.isset && hwpinfo.back_info.type > 0 )
         {
             if( hwpinfo.back_info.type == 1 ){
#ifdef _WIN32
                 mxList->addAttribute("xlink:href", sXML_CDATA,
                      hstr2OUString(kstr2hstr(reinterpret_cast<uchar const *>(urltowin(hwpinfo.back_info.filename).c_str())).c_str()));
#else
                 mxList->addAttribute(u"xlink:href"_ustr, sXML_CDATA,
                    hstr2OUString(kstr2hstr( reinterpret_cast<uchar const *>(urltounix(hwpinfo.back_info.filename).c_str())).c_str()));
#endif
                 mxList->addAttribute(u"xlink:type"_ustr, sXML_CDATA, u"simple"_ustr);
                 mxList->addAttribute(u"xlink:actuate"_ustr, sXML_CDATA, u"onLoad"_ustr);
             }
             if( hwpinfo.back_info.flag >= 2)
                 mxList->addAttribute(u"style:repeat"_ustr, sXML_CDATA, u"stretch"_ustr);
             else if( hwpinfo.back_info.flag == 1 ){
                 mxList->addAttribute(u"style:repeat"_ustr, sXML_CDATA, u"no-repeat"_ustr);
                 mxList->addAttribute(u"style:position"_ustr, sXML_CDATA, u"center"_ustr);
             }
             startEl(u"style:background-image"_ustr);

             if( hwpinfo.back_info.type == 2 ){
                 startEl(u"office:binary-data"_ustr);
                 mxList->clear();
                 chars(base64_encode_string(reinterpret_cast<unsigned char*>(hwpinfo.back_info.data.data()), hwpinfo.back_info.size));
                 endEl(u"office:binary-data"_ustr);
             }
             endEl(u"style:background-image"_ustr);
         }

         makeColumns( hwpfile.GetColumnDef(i) );

         endEl(u"style:properties"_ustr);

    /* header style */
         startEl(u"style:header-style"_ustr);
         mxList->addAttribute(u"svg:height"_ustr, sXML_CDATA,
              OUString::number(WTI(hwpinfo.paper.header_length)) + "inch");
         mxList->addAttribute(u"fo:margin-bottom"_ustr, sXML_CDATA, u"0mm"_ustr);

         startEl(u"style:properties"_ustr);
         mxList->clear();
         endEl(u"style:properties"_ustr);
         endEl(u"style:header-style"_ustr);

    /* footer style */
         startEl(u"style:footer-style"_ustr);
         mxList->addAttribute(u"svg:height"_ustr, sXML_CDATA,
              OUString::number(WTI(hwpinfo.paper.footer_length)) + "inch");
         mxList->addAttribute(u"fo:margin-top"_ustr, sXML_CDATA, u"0mm"_ustr);
         startEl(u"style:properties"_ustr);
         mxList->clear();
         endEl(u"style:properties"_ustr);
         endEl(u"style:footer-style"_ustr);

    /* Footnote style */
         startEl(u"style:page-layout-properties"_ustr);

         mxList->addAttribute(u"style:distance-before-sep"_ustr, sXML_CDATA,
              OUString::number(WTI(hwpinfo.splinetext)) + "inch");
         mxList->addAttribute(u"style:distance-after-sep"_ustr, sXML_CDATA,
              OUString::number(WTI(hwpinfo.splinefn)) + "inch");
         startEl(u"style:properties"_ustr);
         mxList->clear();
         endEl(u"style:properties"_ustr);
         if ( hwpinfo.fnlinetype == 2 )
              mxList->addAttribute(u"style:width"_ustr, sXML_CDATA, u"15cm"_ustr);
         else if ( hwpinfo.fnlinetype == 1)
              mxList->addAttribute(u"style:width"_ustr, sXML_CDATA, u"2cm"_ustr);
         else if ( hwpinfo.fnlinetype == 3)
              mxList->addAttribute(u"style:width"_ustr, sXML_CDATA, u"0cm"_ustr);
         else
              mxList->addAttribute(u"style:width"_ustr, sXML_CDATA, u"5cm"_ustr);

         startEl(u"style:footnote-sep"_ustr);
         mxList->clear();
         endEl(u"style:footnote-sep"_ustr);

         endEl(u"style:page-layout-properties"_ustr);

         endEl(u"style:page-master"_ustr);
     }
}

void HwpReader::makeColumns(ColumnDef const *coldef)
{
    if( !coldef ) return;
    mxList->addAttribute(u"fo:column-count"_ustr, sXML_CDATA, OUString::number(coldef->ncols));
    startEl(u"style:columns"_ustr);
    mxList->clear();
    if( coldef->separator != 0 )
    {
        switch( coldef->separator )
        {
             case 1:                           /* thin line */
                  mxList->addAttribute(u"style:width"_ustr, sXML_CDATA, u"0.02mm"_ustr);
                  [[fallthrough]];
             case 3:                           /* dotted line */
                  mxList->addAttribute(u"style:style"_ustr, sXML_CDATA, u"dotted"_ustr);
                  mxList->addAttribute(u"style:width"_ustr, sXML_CDATA, u"0.02mm"_ustr);
                  break;
             case 2:                           /* thick line */
             case 4:                           /* double line */
                  mxList->addAttribute(u"style:width"_ustr, sXML_CDATA, u"0.35mm"_ustr);
                  break;
             case 0:                           /* None */
             default:
                  mxList->addAttribute(u"style:style"_ustr, sXML_CDATA, u"none"_ustr);
                  break;
        }
        startEl(u"style:column-sep"_ustr);
        mxList->clear();
        endEl(u"style:column-sep"_ustr);
    }
    double spacing = WTI(coldef->spacing)/ 2. ;
    for(int ii = 0 ; ii < coldef->ncols ; ii++)
    {
        if( ii == 0 )
             mxList->addAttribute(u"fo:margin-left"_ustr, sXML_CDATA, u"0mm"_ustr);
        else
             mxList->addAttribute(u"fo:margin-left"_ustr, sXML_CDATA,
                  OUString::number( spacing) + "inch");
        if( ii == ( coldef->ncols -1) )
             mxList->addAttribute(u"fo:margin-right"_ustr, sXML_CDATA,u"0mm"_ustr);
        else
             mxList->addAttribute(u"fo:margin-right"_ustr, sXML_CDATA,
                  OUString::number( spacing) + "inch");
        startEl(u"style:column"_ustr);
        mxList->clear();
        endEl(u"style:column"_ustr);
    }
    endEl(u"style:columns"_ustr);
}

void HwpReader::makeTStyle(CharShape const * cshape)
{
    mxList->addAttribute(u"style:name"_ustr, sXML_CDATA, "T" + OUString::number(cshape->index));
    mxList->addAttribute(u"style:family"_ustr, sXML_CDATA, u"text"_ustr);
    startEl(u"style:style"_ustr);
    mxList->clear();
    parseCharShape(cshape);
    startEl(u"style:properties"_ustr);
    mxList->clear();
    endEl(u"style:properties"_ustr);
    endEl(u"style:style"_ustr);
}


void HwpReader::makeTableStyle(Table *tbl)
{
// table
    TxtBox *hbox = tbl->box;

    mxList->addAttribute(u"style:name"_ustr, sXML_CDATA, "Table" + OUString::number(hbox->style.boxnum));
    mxList->addAttribute(u"style:family"_ustr, sXML_CDATA,u"table"_ustr);
    startEl(u"style:style"_ustr);
    mxList->clear();
    mxList->addAttribute(u"style:width"_ustr, sXML_CDATA,
        OUString::number(WTMM(hbox->box_xs)) + "mm");
    mxList->addAttribute(u"table:align"_ustr, sXML_CDATA,u"left"_ustr);
    mxList->addAttribute(u"fo:keep-with-next"_ustr, sXML_CDATA,u"false"_ustr);
    startEl(u"style:properties"_ustr);
    mxList->clear();
    endEl(u"style:properties"_ustr);
    endEl(u"style:style"_ustr);

// column
    for (size_t i = 0 ; i < tbl->columns.nCount -1 ; i++)
    {
        mxList->addAttribute(
            u"style:name"_ustr, sXML_CDATA,
            "Table" + OUString::number(hbox->style.boxnum) + "."
                + OUStringChar(static_cast<char>('A'+i)));
        mxList->addAttribute(u"style:family"_ustr, sXML_CDATA,u"table-column"_ustr);
        startEl(u"style:style"_ustr);
        mxList->clear();
        mxList->addAttribute(u"style:column-width"_ustr, sXML_CDATA,
            OUString::number(WTMM(tbl->columns.data[i+1] - tbl->columns.data[i])) + "mm");
        startEl(u"style:properties"_ustr);
        mxList->clear();
        endEl(u"style:properties"_ustr);
        endEl(u"style:style"_ustr);
    }

// row
    for (size_t i = 0 ; i < tbl->rows.nCount -1 ; i++)
    {
        mxList->addAttribute(
            u"style:name"_ustr, sXML_CDATA,
            "Table" + OUString::number(hbox->style.boxnum) + ".row" + OUString::number(i + 1));
        mxList->addAttribute(u"style:family"_ustr, sXML_CDATA,u"table-row"_ustr);
        startEl(u"style:style"_ustr);
        mxList->clear();
        mxList->addAttribute(u"style:row-height"_ustr, sXML_CDATA,
            OUString::number(WTMM(tbl->rows.data[i+1] - tbl->rows.data[i])) + "mm");
        startEl(u"style:properties"_ustr);
        mxList->clear();
        endEl(u"style:properties"_ustr);
        endEl(u"style:style"_ustr);
    }

// cell
    for (auto const& tcell : tbl->cells)
    {
        mxList->addAttribute(
            u"style:name"_ustr, sXML_CDATA,
            "Table" + OUString::number(hbox->style.boxnum) + "."
                + OUStringChar(char('A'+ tcell->nColumnIndex))
                + OUString::number(tcell->nRowIndex +1));
        mxList->addAttribute(u"style:family"_ustr, sXML_CDATA,u"table-cell"_ustr);
        startEl(u"style:style"_ustr);
        mxList->clear();
        Cell *cl = tcell->pCell;
        if( cl->ver_align == 1 )
            mxList->addAttribute(u"fo:vertical-align"_ustr, sXML_CDATA,u"middle"_ustr);

        if(cl->linetype[2] == cl->linetype[3] && cl->linetype[2] == cl->linetype[0]
            && cl->linetype[2] == cl->linetype[1])
        {
            switch( cl->linetype[2] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute(u"fo:border"_ustr, sXML_CDATA,u"0.002cm solid #000000"_ustr);
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute(u"fo:border"_ustr, sXML_CDATA,u"0.035cm solid #000000"_ustr);
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute(u"style:border-line-width"_ustr, sXML_CDATA,u"0.002cm 0.035cm 0.002cm"_ustr);
                    mxList->addAttribute(u"fo:border"_ustr, sXML_CDATA,u"0.039cm double #000000"_ustr);
                    break;
            }
        }
        else
        {
            switch( cl->linetype[0] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute(u"fo:border-left"_ustr, sXML_CDATA,u"0.002cm solid #000000"_ustr);
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute(u"fo:border-left"_ustr, sXML_CDATA,u"0.035cm solid #000000"_ustr);
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute(u"style:border-line-width-left"_ustr, sXML_CDATA,u"0.002cm 0.035cm 0.002cm"_ustr);
                    mxList->addAttribute(u"fo:border-left"_ustr, sXML_CDATA,u"0.039cm double #000000"_ustr);
                    break;
            }
            switch( cl->linetype[1] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute(u"fo:border-right"_ustr, sXML_CDATA,u"0.002cm solid #000000"_ustr);
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute(u"fo:border-right"_ustr, sXML_CDATA,u"0.035cm solid #000000"_ustr);
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute(u"style:border-line-width-right"_ustr, sXML_CDATA,u"0.002cm 0.035cm 0.002cm"_ustr);
                    mxList->addAttribute(u"fo:border-right"_ustr, sXML_CDATA,u"0.039cm double #000000"_ustr);
                    break;
            }
            switch( cl->linetype[2] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute(u"fo:border-top"_ustr, sXML_CDATA,u"0.002cm solid #000000"_ustr);
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute(u"fo:border-top"_ustr, sXML_CDATA,u"0.035cm solid #000000"_ustr);
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute(u"style:border-line-width-top"_ustr, sXML_CDATA,u"0.002cm 0.035cm 0.002cm"_ustr);
                    mxList->addAttribute(u"fo:border-top"_ustr, sXML_CDATA,u"0.039cm double #000000"_ustr);
                    break;
            }
            switch( cl->linetype[3] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute(u"fo:border-bottom"_ustr, sXML_CDATA,u"0.002cm solid #000000"_ustr);
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute(u"fo:border-bottom"_ustr, sXML_CDATA,u"0.035cm solid #000000"_ustr);
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute(u"style:border-line-width-bottom"_ustr, sXML_CDATA,u"0.002cm 0.035cm 0.002cm"_ustr);
                    mxList->addAttribute(u"fo:border-bottom"_ustr, sXML_CDATA,u"0.039cm double #000000"_ustr);
                    break;
            }
        }
        if(cl->shade != 0)
            mxList->addAttribute(u"fo:background-color"_ustr, sXML_CDATA,
                hcolor2str(sal::static_int_cast<uchar>(cl->color),
                                sal::static_int_cast<uchar>(cl->shade)));

        startEl(u"style:properties"_ustr);
        mxList->clear();
        endEl(u"style:properties"_ustr);

        endEl(u"style:style"_ustr);
    }
}


void HwpReader::makeDrawStyle( HWPDrawingObject * hdo, FBoxStyle * fstyle)
{
    while( hdo )
    {
        mxList->addAttribute(u"style:name"_ustr, sXML_CDATA, "Draw" + OUString::number(hdo->index));
        mxList->addAttribute(u"style:family"_ustr, sXML_CDATA, u"graphics"_ustr);

        startEl(u"style:style"_ustr);
        mxList->clear();

        switch (fstyle->txtflow)
        {
            case 0:
                break;
            case 1:
                mxList->addAttribute(u"style:wrap"_ustr, sXML_CDATA, u"run-through"_ustr);
                break;
            case 2:
                mxList->addAttribute(u"style:wrap"_ustr, sXML_CDATA, u"dynamic"_ustr);
                break;
        }
        tools::Long color;
// invisible line
        if( hdo->property.line_color > 0xffffff )
        {
            mxList->addAttribute(u"draw:stroke"_ustr, sXML_CDATA, u"none"_ustr );
        }
        else
        {

            if( hdo->property.line_pstyle == 0 )
                mxList->addAttribute(u"draw:stroke"_ustr, sXML_CDATA, u"solid"_ustr );
            else if( hdo->property.line_pstyle < 5 )
            {
                mxList->addAttribute(u"draw:stroke"_ustr, sXML_CDATA, u"dash"_ustr );
                mxList->addAttribute(u"draw:stroke-dash"_ustr, sXML_CDATA, "LineType" + OUString::number(hdo->index));
            }
            mxList->addAttribute(u"svg:stroke-width"_ustr, sXML_CDATA,
                OUString::number( WTMM(hdo->property.line_width)) + "mm");
            mxList->addAttribute(u"svg:stroke-color"_ustr, sXML_CDATA,
                                 rgb2str(static_cast<int32_t>(hdo->property.line_color)));
        }

        if( hdo->type == HWPDO_LINE || hdo->type == HWPDO_ARC ||
            hdo->type == HWPDO_FREEFORM || hdo->type == HWPDO_ADVANCED_ARC )
        {

            if( hdo->property.line_tstyle > 0 &&
                o3tl::make_unsigned(hdo->property.line_tstyle) < std::size(ArrowShape) )
            {
                mxList->addAttribute(u"draw:marker-start"_ustr, sXML_CDATA,
                    OUString::createFromAscii(ArrowShape[hdo->property.line_tstyle].name) );
                if( hdo->property.line_width > 100 )
                         mxList->addAttribute(u"draw:marker-start-width"_ustr, sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 3.0)) + "mm");
                else if( hdo->property.line_width > 80 )
                         mxList->addAttribute(u"draw:marker-start-width"_ustr, sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 4.0)) + "mm");
                else if( hdo->property.line_width > 60 )
                         mxList->addAttribute(u"draw:marker-start-width"_ustr, sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 5.0)) + "mm");
                else if( hdo->property.line_width > 40 )
                         mxList->addAttribute(u"draw:marker-start-width"_ustr, sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 6.0)) + "mm");
                else
                         mxList->addAttribute(u"draw:marker-start-width"_ustr, sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 7.0)) + "mm");
            }

            if( hdo->property.line_hstyle > 0 &&
                o3tl::make_unsigned(hdo->property.line_hstyle) < std::size(ArrowShape) )
            {
                mxList->addAttribute(u"draw:marker-end"_ustr, sXML_CDATA,
                    OUString::createFromAscii(ArrowShape[hdo->property.line_hstyle].name) );
                if( hdo->property.line_width > 100 )
                         mxList->addAttribute(u"draw:marker-end-width"_ustr, sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 3.0)) + "mm");
                else if( hdo->property.line_width > 80 )
                         mxList->addAttribute(u"draw:marker-end-width"_ustr, sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 4.0)) + "mm");
                else if( hdo->property.line_width > 60 )
                         mxList->addAttribute(u"draw:marker-end-width"_ustr, sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 5.0)) + "mm");
                else if( hdo->property.line_width > 40 )
                         mxList->addAttribute(u"draw:marker-end-width"_ustr, sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 6.0)) + "mm");
                else
                         mxList->addAttribute(u"draw:marker-end-width"_ustr, sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 7.0)) + "mm");
            }
        }

        if(hdo->type != HWPDO_LINE )
        {
            if( hdo->property.flag >> 19 & 0x01 )
            {
                mxList->addAttribute( u"draw:textarea-horizontal-align"_ustr, sXML_CDATA, u"center"_ustr);
            }

            color = hdo->property.fill_color;

            if( hdo->property.flag >> 18 & 0x01 ) // bitmap pattern
            {
                mxList->addAttribute(u"draw:fill"_ustr, sXML_CDATA, u"bitmap"_ustr);
                mxList->addAttribute(u"draw:fill-image-name"_ustr, sXML_CDATA,
                    "fillimage" + OUString::number(hdo->index));
                                                  // bitmap resizing
                if( hdo->property.flag >> 3 & 0x01 )
                {
                    mxList->addAttribute(u"style:repeat"_ustr, sXML_CDATA, u"stretch"_ustr);
                }
                else
                {
                    mxList->addAttribute(u"style:repeat"_ustr, sXML_CDATA, u"repeat"_ustr);
                    mxList->addAttribute(u"draw:fill-image-ref-point"_ustr, sXML_CDATA, u"top-left"_ustr);
                }
                if( hdo->property.flag >> 20 & 0x01 )
                {
                    if( hdo->property.luminance > 0 )
                    {
                        mxList->addAttribute(u"draw:transparency"_ustr, sXML_CDATA,
                            OUString::number(hdo->property.luminance) + "%");
                    }
                }

            }
                                                  // Gradation
            else if( hdo->property.flag >> 16 & 0x01 )
            {
                mxList->addAttribute(u"draw:fill"_ustr, sXML_CDATA, u"gradient"_ustr);
                mxList->addAttribute(u"draw:fill-gradient-name"_ustr, sXML_CDATA, "Grad" + OUString::number(hdo->index));
                mxList->addAttribute(u"draw:gradient-step-count"_ustr, sXML_CDATA, OUString::number(hdo->property.nstep));

            }
                                                  // Hatching
            else if( hdo->property.pattern_type >> 24 & 0x01 )
            {
                mxList->addAttribute(u"draw:fill"_ustr, sXML_CDATA, u"hatch"_ustr);
                mxList->addAttribute(u"draw:fill-hatch-name"_ustr, sXML_CDATA, "Hatch" + OUString::number(hdo->index));
                if( color < 0xffffff )
                {
                    mxList->addAttribute(u"draw:fill-color"_ustr, sXML_CDATA,
                                         rgb2str(static_cast<int32_t>(color)));
                    mxList->addAttribute(u"draw:fill-hatch-solid"_ustr, sXML_CDATA, u"true"_ustr);
                }
            }
            else if( color <= 0xffffff )
            {
                mxList->addAttribute(u"draw:fill"_ustr, sXML_CDATA, u"solid"_ustr);
                mxList->addAttribute(u"draw:fill-color"_ustr, sXML_CDATA,
                                     rgb2str(static_cast<int32_t>(color)));
            }
            else
                mxList->addAttribute(u"draw:fill"_ustr, sXML_CDATA, u"none"_ustr);
        }

        if( fstyle->anchor_type == CHAR_ANCHOR )
        {
            mxList->addAttribute(u"style:vertical-pos"_ustr, sXML_CDATA, u"top"_ustr);
            mxList->addAttribute(u"style:vertical-rel"_ustr, sXML_CDATA, u"baseline"_ustr);
        }

        startEl(u"style:properties"_ustr);
        mxList->clear();
        endEl(u"style:properties"_ustr);
        endEl(u"style:style"_ustr);

        if( hdo->type == 0 )
        {
            makeDrawStyle( hdo->child.get(), fstyle );
        }
        hdo = hdo->next.get();
    }
}


void HwpReader::makeCaptionStyle(FBoxStyle * fstyle)
{
    mxList->addAttribute(u"style:name"_ustr, sXML_CDATA, "CapBox" + OUString::number(fstyle->boxnum));
    mxList->addAttribute(u"style:family"_ustr, sXML_CDATA, u"graphics"_ustr);
    startEl(u"style:style"_ustr);
    mxList->clear();
    mxList->addAttribute(u"fo:margin-left"_ustr, sXML_CDATA, u"0cm"_ustr);
    mxList->addAttribute(u"fo:margin-right"_ustr, sXML_CDATA, u"0cm"_ustr);
    mxList->addAttribute(u"fo:margin-top"_ustr, sXML_CDATA, u"0cm"_ustr);
    mxList->addAttribute(u"fo:margin-bottom"_ustr, sXML_CDATA, u"0cm"_ustr);
    mxList->addAttribute(u"fo:padding"_ustr, sXML_CDATA, u"0cm"_ustr);
    switch (fstyle->txtflow)
    {
        case 0:
            mxList->addAttribute(u"style:wrap"_ustr, sXML_CDATA, u"none"_ustr);
            break;
        case 1:
            if( fstyle->boxtype == 'G' )
                mxList->addAttribute(u"style:run-through"_ustr, sXML_CDATA, u"background"_ustr);
            mxList->addAttribute(u"style:wrap"_ustr, sXML_CDATA, u"run-through"_ustr);
            break;
        case 2:
            mxList->addAttribute(u"style:wrap"_ustr, sXML_CDATA, u"dynamic"_ustr);
            break;
    }
    if (fstyle->anchor_type == CHAR_ANCHOR)
    {
        mxList->addAttribute(u"style:vertical-pos"_ustr, sXML_CDATA, u"top"_ustr);
        mxList->addAttribute(u"style:vertical-rel"_ustr, sXML_CDATA, u"baseline"_ustr);
        mxList->addAttribute(u"style:horizontal-pos"_ustr, sXML_CDATA, u"center"_ustr);
        mxList->addAttribute(u"style:horizontal-rel"_ustr, sXML_CDATA, u"paragraph"_ustr);
    }
    else
    {

        switch (-(fstyle->xpos))
        {
            case 2:
                mxList->addAttribute(u"style:horizontal-pos"_ustr, sXML_CDATA, u"right"_ustr);
                break;
            case 3:
                mxList->addAttribute(u"style:horizontal-pos"_ustr, sXML_CDATA, u"center"_ustr);
                break;
            case 1:
            default:
                mxList->addAttribute(u"style:horizontal-pos"_ustr, sXML_CDATA, u"from-left"_ustr);
                break;
        }
        switch (-(fstyle->ypos))
        {
            case 2:
                mxList->addAttribute(u"style:vertical-pos"_ustr, sXML_CDATA, u"bottom"_ustr);
                break;
            case 3:
                mxList->addAttribute(u"style:vertical-pos"_ustr, sXML_CDATA, u"middle"_ustr);
                break;
            case 1:
            default:
                mxList->addAttribute(u"style:vertical-pos"_ustr, sXML_CDATA, u"from-top"_ustr);
                break;
        }
        if ( fstyle->anchor_type == PARA_ANCHOR )
        {
            mxList->addAttribute(u"style:vertical-rel"_ustr, sXML_CDATA, u"paragraph"_ustr);
            mxList->addAttribute(u"style:horizontal-rel"_ustr, sXML_CDATA, u"paragraph"_ustr);
        }
        else
        {
            mxList->addAttribute(u"style:vertical-rel"_ustr, sXML_CDATA, u"page-content"_ustr);
            mxList->addAttribute(u"style:horizontal-rel"_ustr, sXML_CDATA, u"page-content"_ustr);
        }
    }
    startEl(u"style:properties"_ustr);
    mxList->clear();
    endEl(u"style:properties"_ustr);
    endEl(u"style:style"_ustr);
    if( fstyle->boxtype == 'G' )
    {
        mxList->addAttribute(u"style:name"_ustr, sXML_CDATA, "G" + OUString::number(fstyle->boxnum));
    }
    else
    {
        mxList->addAttribute(u"style:name"_ustr, sXML_CDATA, "Txtbox" + OUString::number(fstyle->boxnum));
    }

    mxList->addAttribute(u"style:family"_ustr, sXML_CDATA, u"graphics"_ustr);
    startEl(u"style:style"_ustr);
    mxList->clear();

    mxList->addAttribute(u"fo:margin-left"_ustr, sXML_CDATA, u"0cm"_ustr);
    mxList->addAttribute(u"fo:margin-right"_ustr, sXML_CDATA, u"0cm"_ustr);
    mxList->addAttribute(u"fo:margin-top"_ustr, sXML_CDATA, u"0cm"_ustr);
    mxList->addAttribute(u"fo:margin-bottom"_ustr, sXML_CDATA, u"0cm"_ustr);
    mxList->addAttribute(u"fo:padding"_ustr, sXML_CDATA, u"0cm"_ustr);
    mxList->addAttribute(u"style:wrap"_ustr, sXML_CDATA, u"none"_ustr);
    mxList->addAttribute(u"style:vertical-pos"_ustr, sXML_CDATA, u"from-top"_ustr);
    mxList->addAttribute(u"style:vertical-rel"_ustr, sXML_CDATA, u"paragraph"_ustr);
    mxList->addAttribute(u"style:horizontal-pos"_ustr, sXML_CDATA, u"from-left"_ustr);
    mxList->addAttribute(u"style:horizontal-rel"_ustr, sXML_CDATA, u"paragraph"_ustr);
    if (fstyle->boxtype == 'G' && fstyle->cell)
    {
        char *cell = static_cast<char *>(fstyle->cell);
        mxList->addAttribute(u"draw:luminance"_ustr, sXML_CDATA, OUString::number(cell[0]) + "%");
        mxList->addAttribute(u"draw:contrast"_ustr, sXML_CDATA, OUString::number(cell[1]) + "%");
        if( cell[2] == 0 )
            mxList->addAttribute(u"draw:color-mode"_ustr, sXML_CDATA, u"standard"_ustr);
        else if( cell[2] == 1 )
            mxList->addAttribute(u"draw:color-mode"_ustr, sXML_CDATA, u"greyscale"_ustr);
        else if( cell[2] == 2 )
            mxList->addAttribute(u"draw:color-mode"_ustr, sXML_CDATA, u"mono"_ustr);
    }
    else if (fstyle->cell)
    {
        Cell *cell = static_cast<Cell *>(fstyle->cell);
        if(cell->linetype[0] == cell->linetype[1] &&
            cell->linetype[0] == cell->linetype[2] &&
            cell->linetype[0] == cell->linetype[3])
        {
            switch( cell->linetype[0] )
            {
                case 0:
                    mxList->addAttribute(u"fo:padding"_ustr, sXML_CDATA,u"0mm"_ustr);
                    break;
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute(u"fo:border"_ustr, sXML_CDATA,u"0.002cm solid #000000"_ustr);
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute(u"fo:border"_ustr, sXML_CDATA,u"0.035cm solid #000000"_ustr);
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute(u"style:border-line-width"_ustr, sXML_CDATA,u"0.002cm 0.035cm 0.002cm"_ustr);
                    mxList->addAttribute(u"fo:border"_ustr, sXML_CDATA,u"0.039cm double #000000"_ustr);
                    break;
            }
        }
        else
        {
            switch( cell->linetype[0] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute(u"fo:border-left"_ustr, sXML_CDATA,u"0.002cm solid #000000"_ustr);
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute(u"fo:border-left"_ustr, sXML_CDATA,u"0.035cm solid #000000"_ustr);
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute(u"style:border-line-width-left"_ustr, sXML_CDATA,u"0.002cm 0.035cm 0.002cm"_ustr);
                    mxList->addAttribute(u"fo:border-left"_ustr, sXML_CDATA,u"0.039cm double #000000"_ustr);
                    break;
            }
            switch( cell->linetype[1] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute(u"fo:border-right"_ustr, sXML_CDATA,u"0.002cm solid #000000"_ustr);
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute(u"fo:border-right"_ustr, sXML_CDATA,u"0.035cm solid #000000"_ustr);
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute(u"style:border-line-width-right"_ustr, sXML_CDATA,u"0.002cm 0.035cm 0.002cm"_ustr);
                    mxList->addAttribute(u"fo:border-right"_ustr, sXML_CDATA,u"0.039cm double #000000"_ustr);
                    break;
            }
            switch( cell->linetype[2] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute(u"fo:border-top"_ustr, sXML_CDATA,u"0.002cm solid #000000"_ustr);
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute(u"fo:border-top"_ustr, sXML_CDATA,u"0.035cm solid #000000"_ustr);
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute(u"style:border-line-width-top"_ustr, sXML_CDATA,u"0.002cm 0.035cm 0.002cm"_ustr);
                    mxList->addAttribute(u"fo:border-top"_ustr, sXML_CDATA,u"0.039cm double #000000"_ustr);
                    break;
            }
            switch( cell->linetype[3] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute(u"fo:border-bottom"_ustr, sXML_CDATA,u"0.002cm solid #000000"_ustr);
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute(u"fo:border-bottom"_ustr, sXML_CDATA,u"0.035cm solid #000000"_ustr);
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute(u"style:border-line-width-bottom"_ustr, sXML_CDATA,u"0.002cm 0.035cm 0.002cm"_ustr);
                    mxList->addAttribute(u"fo:border-bottom"_ustr, sXML_CDATA,u"0.039cm double #000000"_ustr);
                    break;
            }
        }
        if(cell->shade != 0)
            mxList->addAttribute(u"fo:background-color"_ustr, sXML_CDATA, hcolor2str(
            sal::static_int_cast<uchar>(cell->color),
            sal::static_int_cast<uchar>(cell->shade)));
    }
    startEl(u"style:properties"_ustr);
    mxList->clear();
    endEl(u"style:properties"_ustr);
    endEl(u"style:style"_ustr);
}


/**
 * Create a style for the Floating objects.
 */
void HwpReader::makeFStyle(FBoxStyle * fstyle)
{
                                                  /* caption exist */
    if( ( fstyle->boxtype == 'G' || fstyle->boxtype == 'X' ) && fstyle->cap_len > 0 )
    {
        makeCaptionStyle(fstyle);
        return;
    }
    switch( fstyle->boxtype )
    {
        case 'X' :                                // txtbox
        case 'E' :                                // equation
        case 'B' :                                // button
        case 'O' :                                // other
        case 'T' :                                // table
            mxList->addAttribute(u"style:name"_ustr, sXML_CDATA, "Txtbox" + OUString::number(fstyle->boxnum));
            mxList->addAttribute(u"style:family"_ustr, sXML_CDATA, u"graphics"_ustr);
            break;
        case 'G' :                                // graphics
            mxList->addAttribute(u"style:name"_ustr, sXML_CDATA, "G" + OUString::number(fstyle->boxnum));
            mxList->addAttribute(u"style:family"_ustr, sXML_CDATA, u"graphics"_ustr);
            break;
        case 'L' :                                // line TODO : all
            mxList->addAttribute(u"style:name"_ustr, sXML_CDATA, "L" + OUString::number(fstyle->boxnum));
            mxList->addAttribute( u"style:family"_ustr , sXML_CDATA , u"paragraph"_ustr );
            break;
    }

    startEl(u"style:style"_ustr);
    mxList->clear();

    if ( fstyle->boxtype == 'T')
    {
        mxList->addAttribute(u"fo:padding"_ustr, sXML_CDATA, u"0cm"_ustr);
    }

    if( fstyle->boxtype != 'G' || fstyle->cap_len <= 0 )
    {
        mxList->addAttribute(u"fo:margin-left"_ustr, sXML_CDATA,
            OUString::number(WTMM(fstyle->margin[0][0]) ) + "mm");
        mxList->addAttribute(u"fo:margin-right"_ustr, sXML_CDATA,
            OUString::number(WTMM(fstyle->margin[0][1])) + "mm");
        mxList->addAttribute(u"fo:margin-top"_ustr, sXML_CDATA,
            OUString::number(WTMM(fstyle->margin[0][2])) + "mm");
        mxList->addAttribute(u"fo:margin-bottom"_ustr, sXML_CDATA,
            OUString::number(WTMM(fstyle->margin[0][3])) + "mm");
    }

    switch (fstyle->txtflow)
    {
        case 0:
            mxList->addAttribute(u"style:wrap"_ustr, sXML_CDATA, u"none"_ustr);
            break;
        case 1:
            if( fstyle->boxtype == 'G' || fstyle->boxtype == 'B' || fstyle->boxtype == 'O')
                mxList->addAttribute(u"style:run-through"_ustr, sXML_CDATA, u"background"_ustr);
            mxList->addAttribute(u"style:wrap"_ustr, sXML_CDATA, u"run-through"_ustr);
            break;
        case 2:
            mxList->addAttribute(u"style:wrap"_ustr, sXML_CDATA, u"dynamic"_ustr);
            break;
    }
    if (fstyle->anchor_type == CHAR_ANCHOR)
    {
        mxList->addAttribute(u"style:vertical-pos"_ustr, sXML_CDATA, u"top"_ustr);
        mxList->addAttribute(u"style:vertical-rel"_ustr, sXML_CDATA, u"baseline"_ustr);
        mxList->addAttribute(u"style:horizontal-pos"_ustr, sXML_CDATA, u"center"_ustr);
        mxList->addAttribute(u"style:horizontal-rel"_ustr, sXML_CDATA, u"paragraph"_ustr);
    }
    else
    {

        switch (-(fstyle->xpos))
        {
            case 2:
                mxList->addAttribute(u"style:horizontal-pos"_ustr, sXML_CDATA, u"right"_ustr);
                break;
            case 3:
                mxList->addAttribute(u"style:horizontal-pos"_ustr, sXML_CDATA, u"center"_ustr);
                break;
            case 1:
            default:
                mxList->addAttribute(u"style:horizontal-pos"_ustr, sXML_CDATA, u"from-left"_ustr);
                break;
        }
        switch (-(fstyle->ypos))
        {
            case 2:
                mxList->addAttribute(u"style:vertical-pos"_ustr, sXML_CDATA, u"bottom"_ustr);
                break;
            case 3:
                mxList->addAttribute(u"style:vertical-pos"_ustr, sXML_CDATA, u"middle"_ustr);
                break;
            case 1:
            default:
                mxList->addAttribute(u"style:vertical-pos"_ustr, sXML_CDATA, u"from-top"_ustr);
                break;
        }
        if ( fstyle->anchor_type == PARA_ANCHOR )
        {
            mxList->addAttribute(u"style:vertical-rel"_ustr, sXML_CDATA, u"paragraph"_ustr);
            mxList->addAttribute(u"style:horizontal-rel"_ustr, sXML_CDATA, u"paragraph"_ustr);
        }
        else
        {
            mxList->addAttribute(u"style:vertical-rel"_ustr, sXML_CDATA, u"page-content"_ustr);
            mxList->addAttribute(u"style:horizontal-rel"_ustr, sXML_CDATA, u"page-content"_ustr);
        }
    }
    if (fstyle->cell && (fstyle->boxtype == 'X' || fstyle->boxtype == 'B'))
    {
        Cell *cell = static_cast<Cell *>(fstyle->cell);
        if(cell->linetype[0] == cell->linetype[1] &&
            cell->linetype[0] == cell->linetype[2] &&
            cell->linetype[0] == cell->linetype[3])
        {
            switch( cell->linetype[0] )
            {
                case 0:
                          mxList->addAttribute(u"fo:border"_ustr, sXML_CDATA, u"none"_ustr);
                    break;
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute(u"fo:border"_ustr, sXML_CDATA,u"0.002cm solid #000000"_ustr);
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute(u"fo:border"_ustr, sXML_CDATA,u"0.035cm solid #000000"_ustr);
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute(u"style:border-line-width"_ustr, sXML_CDATA,u"0.002cm 0.035cm 0.002cm"_ustr);
                    mxList->addAttribute(u"fo:border"_ustr, sXML_CDATA,u"0.039cm double #000000"_ustr);
                    break;
            }
        }
        else
        {
            switch( cell->linetype[0] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute(u"fo:border-left"_ustr, sXML_CDATA,u"0.002cm solid #000000"_ustr);
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute(u"fo:border-left"_ustr, sXML_CDATA,u"0.035cm solid #000000"_ustr);
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute(u"style:border-line-width-left"_ustr, sXML_CDATA,u"0.002cm 0.035cm 0.002cm"_ustr);
                    mxList->addAttribute(u"fo:border-left"_ustr, sXML_CDATA,u"0.039cm double #000000"_ustr);
                    break;
            }
            switch( cell->linetype[1] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute(u"fo:border-right"_ustr, sXML_CDATA,u"0.002cm solid #000000"_ustr);
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute(u"fo:border-right"_ustr, sXML_CDATA,u"0.035cm solid #000000"_ustr);
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute(u"style:border-line-width-right"_ustr, sXML_CDATA,u"0.002cm 0.035cm 0.002cm"_ustr);
                    mxList->addAttribute(u"fo:border-right"_ustr, sXML_CDATA,u"0.039cm double #000000"_ustr);
                    break;
            }
            switch( cell->linetype[2] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute(u"fo:border-top"_ustr, sXML_CDATA,u"0.002cm solid #000000"_ustr);
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute(u"fo:border-top"_ustr, sXML_CDATA,u"0.035cm solid #000000"_ustr);
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute(u"style:border-line-width-top"_ustr, sXML_CDATA,u"0.002cm 0.035cm 0.002cm"_ustr);
                    mxList->addAttribute(u"fo:border-top"_ustr, sXML_CDATA,u"0.039cm double #000000"_ustr);
                    break;
            }
            switch( cell->linetype[3] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute(u"fo:border-bottom"_ustr, sXML_CDATA,u"0.002cm solid #000000"_ustr);
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute(u"fo:border-bottom"_ustr, sXML_CDATA,u"0.035cm solid #000000"_ustr);
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute(u"style:border-line-width-bottom"_ustr, sXML_CDATA,u"0.002cm 0.035cm 0.002cm"_ustr);
                    mxList->addAttribute(u"fo:border-bottom"_ustr, sXML_CDATA,u"0.039cm double #000000"_ustr);
                    break;
            }
        }

        if( cell->linetype[0] == 0 && cell->linetype[1] == 0 &&
                  cell->linetype[2] == 0 && cell->linetype[3] == 0 ){
              mxList->addAttribute(u"fo:padding"_ustr, sXML_CDATA,u"0mm"_ustr);
        }
        else{
              mxList->addAttribute(u"fo:padding-left"_ustr, sXML_CDATA,
                      OUString::number(WTMM(fstyle->margin[1][0])) + "mm");
              mxList->addAttribute(u"fo:padding-right"_ustr, sXML_CDATA,
                      OUString::number(WTMM(fstyle->margin[1][1])) + "mm");
              mxList->addAttribute(u"fo:padding-top"_ustr, sXML_CDATA,
                      OUString::number(WTMM(fstyle->margin[1][2])) + "mm");
              mxList->addAttribute(u"fo:padding-bottom"_ustr, sXML_CDATA,
                      OUString::number(WTMM(fstyle->margin[1][3])) + "mm");
        }
        if(cell->shade != 0)
            mxList->addAttribute(u"fo:background-color"_ustr, sXML_CDATA,
            hcolor2str(
                sal::static_int_cast<uchar>(cell->color),
                sal::static_int_cast<uchar>(cell->shade)));
    }
    else if( fstyle->boxtype == 'E' )
     {
          mxList->addAttribute(u"fo:padding"_ustr, sXML_CDATA,u"0mm"_ustr);
     }
    else if( fstyle->boxtype == 'L' )
    {
        mxList->addAttribute( u"style:border-line-width-bottom"_ustr, sXML_CDATA, u"0.02mm 0.35mm 0.02mm"_ustr);
        mxList->addAttribute(u"fo:border-bottom"_ustr, sXML_CDATA,u"0.039cm double #808080"_ustr);
    }
    else if( fstyle->boxtype == 'G' && fstyle->cell )
    {
        if( fstyle->margin[1][0] || fstyle->margin[1][1] || fstyle->margin[1][2] || fstyle->margin[1][3] ){
             OUString clip = "rect(" +
                OUString::number(WTMM(-fstyle->margin[1][0]) ) + "mm " +
                OUString::number(WTMM(-fstyle->margin[1][1]) ) + "mm " +
                OUString::number(WTMM(-fstyle->margin[1][2]) ) + "mm " +
                OUString::number(WTMM(-fstyle->margin[1][3]) ) + "mm)";
             mxList->addAttribute(u"style:mirror"_ustr, sXML_CDATA, u"none"_ustr);
             mxList->addAttribute(u"fo:clip"_ustr, sXML_CDATA, clip);
        }
        char *cell = static_cast<char *>(fstyle->cell);
        mxList->addAttribute(u"draw:luminance"_ustr, sXML_CDATA, OUString::number(cell[0]) + "%");
        mxList->addAttribute(u"draw:contrast"_ustr, sXML_CDATA, OUString::number(cell[1]) + "%");
        if( cell[2] == 0 )
            mxList->addAttribute(u"draw:color-mode"_ustr, sXML_CDATA, u"standard"_ustr);
        else if( cell[2] == 1 )
            mxList->addAttribute(u"draw:color-mode"_ustr, sXML_CDATA, u"greyscale"_ustr);
        else if( cell[2] == 2 )
            mxList->addAttribute(u"draw:color-mode"_ustr, sXML_CDATA, u"mono"_ustr);

    }
    startEl(u"style:properties"_ustr);
    mxList->clear();
    endEl(u"style:properties"_ustr);
    endEl(u"style:style"_ustr);
}


OUString HwpReader::getTStyleName(int index)
{
    return "T" + OUString::number(index);
}


OUString HwpReader::getPStyleName(int index)
{
    return "P" + OUString::number(index);
}


void HwpReader::makeChars(hchar_string & rStr)
{
    chars(fromHcharStringToOUString(rStr));
    rStr.clear();
}


/**
 * If no special characters in the paragraph and all characters use the same CharShape
 */
void HwpReader::make_text_p0(HWPPara * para, bool bParaStart)
{
    hchar_string str;
    int res;
    hchar dest[3];
    unsigned char firstspace = 0;
    if( !bParaStart)
    {
        mxList->addAttribute(u"text:style-name"_ustr, sXML_CDATA,
            getPStyleName(para->GetParaShape().index));
        startEl(u"text:p"_ustr);
        mxList->clear();
    }
    if( d->bFirstPara && d->bInBody )
    {
        mxList->addAttribute(u"text:name"_ustr, sXML_CDATA, sBeginOfDoc);
        startEl(u"text:bookmark"_ustr);
        mxList->clear();
        endEl(u"text:bookmark"_ustr);
        d->bFirstPara = false;
    }
    if( d->bInHeader )
    {
        makeShowPageNum();
        d->bInHeader = false;
    }
    mxList->addAttribute(u"text:style-name"_ustr, sXML_CDATA,
        getTStyleName(para->cshape->index));
    startEl(u"text:span"_ustr);
    mxList->clear();

    for (const auto& box : para->hhstr)
    {
        if (!box->hh)
            break;

        if (box->hh == CH_SPACE && !firstspace)
        {
            makeChars(str);
            startEl(u"text:s"_ustr);
            endEl(u"text:s"_ustr);
        }
        else if (box->hh == CH_END_PARA)
        {
            makeChars(str);
            endEl(u"text:span"_ustr);
            endEl(u"text:p"_ustr);
            break;
        }
        else
        {
            if (box->hh == CH_SPACE)
                firstspace = 0;
            else
                firstspace = 1;
            res = hcharconv(box->hh, dest, UNICODE);
            for( int j = 0 ; j < res; j++ )
            {
                str.push_back(dest[j]);
            }
        }
    }
}


/*
 * There is no special characters in the paragraph, but characters use different CharShapes
 */
void HwpReader::make_text_p1(HWPPara * para,bool bParaStart)
{
    hchar_string str;
    int res;
    hchar dest[3];
    int curr = para->cshape->index;
    unsigned char firstspace = 0;

    if( !bParaStart )
    {
        mxList->addAttribute(u"text:style-name"_ustr, sXML_CDATA,
            getPStyleName(para->GetParaShape().index));
        startEl(u"text:p"_ustr);
        mxList->clear();
    }
    if( d->bFirstPara && d->bInBody )
    {
/* for HWP's Bookmark */
        mxList->addAttribute(u"text:name"_ustr, sXML_CDATA, sBeginOfDoc);
        startEl(u"text:bookmark"_ustr);
        mxList->clear();
        endEl(u"text:bookmark"_ustr);
        d->bFirstPara = false;
    }
    if( d->bInHeader )
    {
        makeShowPageNum();
        d->bInHeader = false;
    }
    mxList->addAttribute(u"text:style-name"_ustr, sXML_CDATA,
        getTStyleName(curr));
    startEl(u"text:span"_ustr);
    mxList->clear();

    int n = 0;
    for (const auto& box : para->hhstr)
    {
        if (!box->hh)
            break;

        if (para->GetCharShape(n)->index != curr)
        {
            makeChars(str);
            endEl(u"text:span"_ustr);
            curr = para->GetCharShape(n)->index;
            mxList->addAttribute(u"text:style-name"_ustr, sXML_CDATA,
                getTStyleName(curr));
            startEl(u"text:span"_ustr);
            mxList->clear();
        }
        if (box->hh == CH_SPACE && !firstspace)
        {
            makeChars(str);
            startEl(u"text:s"_ustr);
            endEl(u"text:s"_ustr);
        }
        else if (box->hh == CH_END_PARA)
        {
            makeChars(str);
            endEl(u"text:span"_ustr);
            endEl(u"text:p"_ustr);
            break;
        }
        else
        {
            if( box->hh < CH_SPACE )
                  continue;
            if (box->hh == CH_SPACE)
                firstspace = 0;
            else
                firstspace = 1;
            res = hcharconv(box->hh, dest, UNICODE);
            for( int j = 0 ; j < res; j++ )
            {
                str.push_back(dest[j]);
            }
        }
        n += box->WSize();
    }
}


/**
 * Special characters are in the paragraph and characters use different CharShapes
 */
void HwpReader::make_text_p3(HWPPara * para,bool bParaStart)
{
    hchar_string str;
    int res;
    hchar dest[3];
    unsigned char firstspace = 0;
    bool pstart = bParaStart;
    bool tstart = false;
    bool infield = false;

    const auto STARTP = [this, para, &pstart]()
    {
        mxList->addAttribute(u"text:style-name"_ustr, u"CDATA"_ustr,
                             getPStyleName(para->GetParaShape().index));
        startEl(u"text:p"_ustr);
        mxList->clear();
        pstart = true;
    };
    const auto STARTT = [this, para, &tstart](int pos)
    {
        auto curr = para->GetCharShape(pos > 0 ? pos - 1 : 0)->index;
        mxList->addAttribute(u"text:style-name"_ustr, u"CDATA"_ustr, getTStyleName(curr));
        startEl(u"text:span"_ustr);
        mxList->clear();
        tstart = true;
    };
    const auto ENDP = [this, &pstart]()
    {
        endEl(u"text:p"_ustr);
        pstart = false;
    };
    const auto ENDT = [this, &tstart]()
    {
        endEl(u"text:span"_ustr);
        tstart = false;
    };

    if( d->bFirstPara && d->bInBody )
    {
        if ( !pstart ) {
            STARTP();
        }
        mxList->addAttribute(u"text:name"_ustr, sXML_CDATA, sBeginOfDoc);
        startEl(u"text:bookmark"_ustr);
        mxList->clear();
        endEl(u"text:bookmark"_ustr);
        d->bFirstPara = false;
    }
    if( d->bInHeader )
    {
        if ( !pstart ) {
            STARTP();
        }
        makeShowPageNum();
        d->bInHeader = false;
    }

    int n = 0;
    for (const auto& box : para->hhstr)
    {
        if (!box->hh)
            break;

        if (box->hh == CH_END_PARA)
        {
            if (!str.empty())
            {
                if( !pstart ){ STARTP(); }
                if( !tstart ){ STARTT(n);}
                makeChars(str);
            }
            if( tstart ){ ENDT();}
            if( !pstart ){ STARTP(); }
            if( pstart ){ ENDP(); }
            break;
        }
        else if (box->hh == CH_SPACE  && !firstspace)
        {
            if( !pstart ) {STARTP(); }
            if( !tstart ) {STARTT(n);}
            makeChars(str);
            startEl(u"text:s"_ustr);
            mxList->clear();
            endEl(u"text:s"_ustr);
        }
        else if (box->hh >= CH_SPACE)
        {
            if( n > 0 )
                if( para->GetCharShape(n)->index != para->GetCharShape(n-1)->index && !infield )
                {
                         if( !pstart ) {STARTP(); }
                         if( !tstart ) {STARTT(n);}
                         makeChars(str);
                         ENDT();
                }
            if (box->hh == CH_SPACE)
                firstspace = 0;
            else
                firstspace = 1;
            res = hcharconv(box->hh, dest, UNICODE);
            for( int j = 0 ; j < res; j++ )
            {
                str.push_back(dest[j]);
            }
        }
        else if (box->hh == CH_FIELD)
        {
            FieldCode *hbox = static_cast<FieldCode*>(box.get());
            if( hbox->location_info == 1)
            {
                if( !pstart ) {STARTP(); }
                if( !tstart ) {STARTT(n);}
                makeChars(str);
                firstspace = 1;
                if( hbox->type[0] == 4 && hbox->type[1] == 0 )
                {
                     d->pField = hbox->str3.get();
                }
                else{
                     makeFieldCode(str, hbox);
                }
                infield = true;
            }
            else
            {
                firstspace = 1;
                if( hbox->type[0] == 4 && hbox->type[1] == 0 )
                {
                     makeFieldCode(str, hbox);
                     d->pField = nullptr;
                }
                infield = false;
                str.clear();
            }
        }
        else
        {
            switch (box->hh)
            {
                case CH_BOOKMARK:
                    if( !pstart ) {STARTP(); }
                    if( !tstart ) {STARTT(n);}
                    makeChars(str);
                    makeBookmark(static_cast<Bookmark*>(box.get()));
                    break;
                case CH_DATE_FORM:                // 7
                    break;
                case CH_DATE_CODE:                // 8
                    if( !pstart ) {STARTP(); }
                    if( !tstart ) {STARTT(n);}
                    makeChars(str);
                    makeDateCode(static_cast<DateCode*>(box.get()));
                    break;
                case CH_TAB:                      // 9
                    if( !pstart ) {STARTP(); }
                    if (!str.empty())
                    {
                        if( !tstart ) {STARTT(n);}
                        makeChars(str);
                    }
                    makeTab();
                    break;
                case CH_TEXT_BOX:                 /* 10 - ordered by Table/text box/formula/button/hypertext */
                {
                    /* produce tables first, and treat formula as being in text:p. */
                    TxtBox *hbox = static_cast<TxtBox*>(box.get());

                    if( hbox->style.anchor_type == 0 )
                    {
                        if( !pstart ) {STARTP(); }
                        if( !tstart ) {STARTT(n);}
                        makeChars(str);
                    }
                    else
                    {
                        if( !pstart ) {STARTP(); }
                        if (!str.empty())
                        {
                            if( !tstart ) {STARTT(n);}
                            makeChars(str);
                        }
                        if( tstart ) {ENDT();}
                    }
                    switch (hbox->type)
                    {
                        case TBL_TYPE:            // table
                        case TXT_TYPE:            // text box
                        case EQU_TYPE:            // formula
                            makeTextBox(hbox);
                            break;
                        case BUTTON_TYPE:         // text button
                        case HYPERTEXT_TYPE:      // hypertext
                            makeHyperText(hbox);
                            break;
                    }
                    break;
                }
                case CH_PICTURE:                  // 11
                {
                    Picture *hbox = static_cast<Picture*>(box.get());
                    if( hbox->style.anchor_type == 0 )
                    {
                        if( !pstart ) {STARTP(); }
                        if( !tstart ) {STARTT(n);}
                        makeChars(str);
                    }
                    else
                    {
                        if( !pstart ) {STARTP(); }
                        if (!str.empty())
                        {
                            if( !tstart ) {STARTT(n);}
                            makeChars(str);
                        }
                        if( tstart ) {ENDT();}
                    }
                    makePicture(hbox);
                    break;
                }
                case CH_LINE:                     // 14
                {
                    if (!str.empty())
                    {
                        if( !pstart ) {STARTP();}
                        if( !tstart ) {STARTT(n);}
                        makeChars(str);
                    }
                    if( tstart ) {ENDT();}
                    if( pstart ) {ENDP();}
                    makeLine();
                    pstart = true;
                    break;
                }
                case CH_HIDDEN:                   // 15
                    if( !pstart ) {STARTP();}
                    if( !tstart ) {STARTT(n);}
                    makeChars(str);
                    makeHidden(static_cast<Hidden*>(box.get()));
                    break;
                case CH_FOOTNOTE:                 // 17
                    if( !pstart ) {STARTP();}
                    if( !tstart ) {STARTT(n);}
                    makeChars(str);
                    makeFootnote(static_cast<Footnote*>(box.get()));
                    break;
                case CH_AUTO_NUM:                 // 18
                    if( !pstart ) {STARTP();}
                    if( !tstart ) {STARTT(n);}
                    makeChars(str);
                    makeAutoNum(static_cast<AutoNum*>(box.get()));
                    break;
                case CH_NEW_NUM:                  // 19 -skip
                    break;
                case CH_PAGE_NUM_CTRL:            // 21
                    break;
                case CH_MAIL_MERGE:               // 22
                    if( !pstart ) {STARTP();}
                    if( !tstart ) {STARTT(n);}
                    makeChars(str);
                    makeMailMerge(static_cast<MailMerge*>(box.get()));
                    break;
                case CH_COMPOSE:                  /* 23 - overlapping letters */
                    break;
                case CH_HYPHEN:                   // 24
                    break;
                case CH_TOC_MARK:                 /* 25 Need to fix below 3 */
                    if( !pstart ) {STARTP();}
                    if( !tstart ) {STARTT(n);}
                    makeChars(str);
                    break;
                case CH_INDEX_MARK:               // 26
                    if( !pstart ) {STARTP();}
                    if( !tstart ) {STARTT(n);}
                    makeChars(str);
                    break;
                case CH_OUTLINE:                  // 28
                    if( !pstart ) {STARTP();}
                    if( !tstart ) {STARTT(n);}
                    makeChars(str);
                    makeOutline(static_cast<Outline *>(box.get()));
                    break;
                     case CH_FIXED_SPACE:
                     case CH_KEEP_SPACE:
                          str.push_back(0x0020);
                          break;
                }
        }
        n += box->WSize();
    }
}


void HwpReader::makeFieldCode(hchar_string const & rStr, FieldCode const *hbox)
{
/* Push frame */
    if( hbox->type[0] == 4 && hbox->type[1] == 0 )
    {
        mxList->addAttribute(u"text:placeholder-type"_ustr, sXML_CDATA, u"text"_ustr);
        if (d->pField)
            mxList->addAttribute(u"text:description"_ustr, sXML_CDATA, hstr2OUString(d->pField));
        startEl(u"text:placeholder"_ustr);
        mxList->clear();
        chars( fromHcharStringToOUString(rStr) );
        endEl(u"text:placeholder"_ustr);
    }
/* Document Summary */
    else if( hbox->type[0] == 3 && hbox->type[1] == 0 )
    {
        const OUString uStr3 = hstr2OUString(hbox->str3.get());
        if (uStr3 == "title")
        {
            startEl(u"text:title"_ustr);
            chars( hstr2OUString(hbox->str2.get()) );
            endEl(u"text:title"_ustr);
        }
        else if (uStr3 == "subject")
        {
            startEl(u"text:subject"_ustr);
            chars( hstr2OUString(hbox->str2.get()) );
            endEl(u"text:subject"_ustr);
        }
        else if (uStr3 == "author")
        {
            startEl(u"text:author-name"_ustr);
            chars( hstr2OUString(hbox->str2.get()) );
            endEl(u"text:author-name"_ustr);
        }
        else if (uStr3 == "keywords")
        {
            startEl(u"text:keywords"_ustr);
            chars( hstr2OUString(hbox->str2.get()) );
            endEl(u"text:keywords"_ustr);
        }
    }
/* Personal Information */
    else if( hbox->type[0] == 3 && hbox->type[1] == 1 )
    {
        const OUString uStr3 = hstr2OUString(hbox->str3.get());
        if (uStr3 == "User")
        {
            startEl(u"text:sender-lastname"_ustr);
            chars( hstr2OUString(hbox->str2.get()) );
            endEl(u"text:sender-lastname"_ustr);
        }
        else if (uStr3 == "Company")
        {
            startEl(u"text:sender-company"_ustr);
            chars( hstr2OUString(hbox->str2.get()) );
            endEl(u"text:sender-company"_ustr);
        }
        else if (uStr3 == "Position")
        {
            startEl(u"text:sender-title"_ustr);
            chars( hstr2OUString(hbox->str2.get()) );
            endEl(u"text:sender-title"_ustr);
        }
        else if (uStr3 == "Division")
        {
            startEl(u"text:sender-position"_ustr);
            chars( hstr2OUString(hbox->str2.get()) );
            endEl(u"text:sender-position"_ustr);
        }
        else if (uStr3 == "Fax")
        {
            startEl(u"text:sender-fax"_ustr);
            chars( hstr2OUString(hbox->str2.get()) );
            endEl(u"text:sender-fax"_ustr);
        }
        else if (uStr3 == "Pager")
        {
            startEl(u"text:phone-private"_ustr);
            chars( hstr2OUString(hbox->str2.get()) );
            endEl(u"text:phone-private"_ustr);
        }
        else if (uStr3 == "E-mail")
        {
            startEl(u"text:sender-email"_ustr);
            chars( hstr2OUString(hbox->str2.get()) );
            endEl(u"text:sender-email"_ustr);
        }
        else if (uStr3 == "Zipcode(office)")
        {
            startEl(u"text:sender-postal-code"_ustr);
            chars( hstr2OUString(hbox->str2.get()) );
            endEl(u"text:sender-postal-code"_ustr);
        }
        else if (uStr3 == "Phone(office)")
        {
            startEl(u"text:sender-phone-work"_ustr);
            chars( hstr2OUString(hbox->str2.get()) );
            endEl(u"text:sender-phone-work"_ustr);
        }
        else if (uStr3 == "Address(office)")
        {
            startEl(u"text:sender-street"_ustr);
            chars( hstr2OUString(hbox->str2.get()) );
            endEl(u"text:sender-street"_ustr);
        }

    }
    else if( hbox->type[0] == 3 && hbox->type[1] == 2 ) /* creation date */
     {
         if( hbox->m_pDate )
             mxList->addAttribute(u"style:data-style-name"_ustr, sXML_CDATA, "N" + OUString::number(hbox->m_pDate->key));
         startEl(u"text:creation-date"_ustr);
         mxList->clear();
         chars( hstr2OUString(hbox->str2.get()) );
         endEl(u"text:creation-date"_ustr);
     }
}


/**
 * Completed
 * In LibreOffice, refer bookmarks as reference, but hwp doesn't have the sort of feature.
 */
void HwpReader::makeBookmark(Bookmark const * hbox)
{
    if (hbox->type == 0)
    {
        mxList->addAttribute(u"text:name"_ustr, sXML_CDATA, hstr2OUString(hbox->id));
        startEl(u"text:bookmark"_ustr);
        mxList->clear();
        endEl(u"text:bookmark"_ustr);
    }
    else if (hbox->type == 1)                     /* Block bookmarks days begin and end there if */
    {
        mxList->addAttribute(u"text:name"_ustr, sXML_CDATA, hstr2OUString(hbox->id));
        startEl(u"text:bookmark-start"_ustr);
        mxList->clear();
        endEl(u"text:bookmark-start"_ustr);
    }
    else if (hbox->type == 2)
    {
        mxList->addAttribute(u"text:name"_ustr, sXML_CDATA, hstr2OUString(hbox->id));
        startEl(u"text:bookmark-end"_ustr);
        mxList->clear();
        endEl(u"text:bookmark-end"_ustr);
    }
}


void HwpReader::makeDateFormat(DateCode * hbox)
{
    mxList->addAttribute(u"style:name"_ustr, sXML_CDATA, "N" + OUString::number(hbox->key));
    mxList->addAttribute(u"style:family"_ustr, sXML_CDATA,u"data-style"_ustr);
    mxList->addAttribute(u"number:language"_ustr, sXML_CDATA,u"ko"_ustr);
    mxList->addAttribute(u"number:country"_ustr, sXML_CDATA,u"KR"_ustr);

    startEl(u"number:date-style"_ustr);
    mxList->clear();

    bool add_zero = false;
    int zero_check = 0;
    hbox->format[DATE_SIZE -1] = 0;

    const hchar *fmt = hbox->format[0] ? hbox->format : defaultform;

    for( ; *fmt ; fmt++ )
    {
        if( zero_check == 1 )
        {
            zero_check = 0;
        }
        else
            add_zero = false;

        switch( *fmt )
        {
            case '0':
                zero_check = 1;
                add_zero = true;
                break;
            case '1':
                mxList->addAttribute(u"number:style"_ustr, sXML_CDATA, u"long"_ustr);
                startEl(u"number:year"_ustr);
                mxList->clear();
                endEl(u"number:year"_ustr);
                break;
            case '!':
                startEl(u"number:year"_ustr);
                mxList->clear();
                endEl(u"number:year"_ustr);
                break;
            case '2':
                if( add_zero )
                    mxList->addAttribute(u"number:style"_ustr, sXML_CDATA, u"long"_ustr);
                startEl(u"number:month"_ustr);
                mxList->clear();
                endEl(u"number:month"_ustr);
                break;
            case '@':
                mxList->addAttribute(u"number:textual"_ustr, sXML_CDATA, u"true"_ustr);
                startEl(u"number:month"_ustr);
                mxList->clear();
                endEl(u"number:month"_ustr);
                break;
            case '*':
                mxList->addAttribute(u"number:textual"_ustr, sXML_CDATA, u"true"_ustr);
                mxList->addAttribute(u"number:style"_ustr, sXML_CDATA, u"long"_ustr);
                startEl(u"number:month"_ustr);
                mxList->clear();
                endEl(u"number:month"_ustr);
                break;
            case '3':
                if( add_zero )
                    mxList->addAttribute(u"number:style"_ustr, sXML_CDATA, u"long"_ustr);
                startEl(u"number:day"_ustr);
                mxList->clear();
                endEl(u"number:day"_ustr);
                break;
            case '#':
                if( add_zero )
                    mxList->addAttribute(u"number:style"_ustr, sXML_CDATA, u"long"_ustr);
                startEl(u"number:day"_ustr);
                mxList->clear();
                endEl(u"number:day"_ustr);
                switch( hbox->date[DateCode::DAY]  % 10)
                {
                    case 1:
                        startEl(u"number:text"_ustr);
                        chars(u"st"_ustr);
                        endEl(u"number:text"_ustr);
                        break;
                    case 2:
                        startEl(u"number:text"_ustr);
                        chars(u"nd"_ustr);
                        endEl(u"number:text"_ustr);
                        break;
                    case 3:
                        startEl(u"number:text"_ustr);
                        chars(u"rd"_ustr);
                        endEl(u"number:text"_ustr);
                        break;
                    default:
                        startEl(u"number:text"_ustr);
                        chars(u"th"_ustr);
                        endEl(u"number:text"_ustr);
                        break;
                }
                break;
            case '4':
            case '$':
                if( add_zero )
                    mxList->addAttribute(u"number:style"_ustr, sXML_CDATA, u"long"_ustr);
                startEl(u"number:hours"_ustr);
                mxList->clear();
                endEl(u"number:hours"_ustr);
                break;
            case '5':
            case '%':
                if( add_zero )
                    mxList->addAttribute(u"number:style"_ustr, sXML_CDATA, u"long"_ustr);
                startEl(u"number:minutes"_ustr);
                mxList->clear();
                endEl(u"number:minutes"_ustr);
                break;
            case '_':
                mxList->addAttribute(u"number:style"_ustr, sXML_CDATA, u"long"_ustr);
                [[fallthrough]];
            case '6':
            case '^':
                startEl(u"number:day-of-week"_ustr);
                mxList->clear();
                endEl(u"number:day-of-week"_ustr);
                break;
            case '7':
            case '&':
            case '+':
                startEl(u"number:am-pm"_ustr);
                mxList->clear();
                endEl(u"number:am-pm"_ustr);
                break;
            case '~':                             // Chinese Locale
                break;
            default:
                hchar sbuf[2];
                sbuf[0] = *fmt;
                sbuf[1] = 0;
                startEl(u"number:text"_ustr);
                chars(hstr2OUString(sbuf));
                endEl(u"number:text"_ustr);
                break;
        }
    }
    mxList->clear();
    endEl(u"number:date-style"_ustr);
}


void HwpReader::makeDateCode(DateCode * hbox)
{
    mxList->addAttribute(u"style:data-style-name"_ustr, sXML_CDATA, "N" + OUString::number(hbox->key));
    startEl(u"text:date"_ustr);
    mxList->clear();
    hchar_string const boxstr = hbox->GetString();
    chars(hstr2OUString(boxstr.c_str()));
    endEl(u"text:date"_ustr);
}


void HwpReader::makeTab()
{
    startEl(u"text:tab-stop"_ustr);
    endEl(u"text:tab-stop"_ustr);
}


void HwpReader::makeTable(TxtBox * hbox)
{
    mxList->addAttribute(u"table:name"_ustr, sXML_CDATA, "Table" + OUString::number(hbox->style.boxnum));
    mxList->addAttribute(u"table:style-name"_ustr, sXML_CDATA, "Table" + OUString::number(hbox->style.boxnum));
    startEl(u"table:table"_ustr);
    mxList->clear();

    Table *tbl = hbox->m_pTable;
// column
    for (size_t i = 0 ; i < tbl->columns.nCount -1 ; i++)
    {
        mxList->addAttribute(
            u"table:style-name"_ustr, sXML_CDATA,
            "Table" + OUString::number(hbox->style.boxnum) + "."
                + OUStringChar(static_cast<char>('A'+i)));
        startEl(u"table:table-column"_ustr);
        mxList->clear();
        endEl(u"table:table-column"_ustr);
    }

// cell
    int j = -1, k = -1;
    for (auto const& tcell : tbl->cells)
    {
        if( tcell->nRowIndex > j )
        {
            if( j > k )
            {
                endEl(u"table:table-row"_ustr);
                k = j;
            }
// row
            mxList->addAttribute(
                u"table:style-name"_ustr, sXML_CDATA,
                "Table" + OUString::number(hbox->style.boxnum) + ".row"
                    + OUString::number(tcell->nRowIndex + 1));
            startEl(u"table:table-row"_ustr);
            mxList->clear();
            j = tcell->nRowIndex;
        }

        mxList->addAttribute(
            u"table:style-name"_ustr, sXML_CDATA,
            "Table" + OUString::number(hbox->style.boxnum) + "."
                + OUStringChar(char('A'+ tcell->nColumnIndex))
                + OUString::number(tcell->nRowIndex +1));
        if( tcell->nColumnSpan > 1 )
            mxList->addAttribute(u"table:number-columns-spanned"_ustr, sXML_CDATA, OUString::number(tcell->nColumnSpan));
        if( tcell->nRowSpan > 1 )
            mxList->addAttribute(u"table:number-rows-spanned"_ustr, sXML_CDATA, OUString::number(tcell->nRowSpan));
        mxList->addAttribute(u"table:value-type"_ustr, sXML_CDATA,u"string"_ustr);
        if( tcell->pCell->protect )
            mxList->addAttribute(u"table:protected"_ustr, sXML_CDATA,u"true"_ustr);
        startEl(u"table:table-cell"_ustr);
        mxList->clear();
        TxtBox::plist_t& rVec = hbox->plists[tcell->pCell->key];
        if (!rVec.empty())
            parsePara(rVec.front().get());
        endEl(u"table:table-cell"_ustr);
    }
    endEl(u"table:table-row"_ustr);
    endEl(u"table:table"_ustr);
}


/**
 * Parses the text boxes and tables.
 * 1. draw: style-name, draw: name, text: anchor-type, svg: width,
 * Fo: min-height, svg: x, svg: y
 * TODO: fo:background-color <= no idea whether the value of color setting->style is in it or not
 */
void HwpReader::makeTextBox(TxtBox * hbox)
{
    if( hbox->style.cap_len > 0  && hbox->type == TXT_TYPE)
    {
        mxList->addAttribute(u"draw:style-name"_ustr, sXML_CDATA, "CapBox" + OUString::number(hbox->style.boxnum));
        mxList->addAttribute(u"draw:name"_ustr, sXML_CDATA, "CaptionBox" + OUString::number(hbox->style.boxnum));
        mxList->addAttribute(u"draw:z-index"_ustr, sXML_CDATA, OUString::number(hbox->zorder));
        switch (hbox->style.anchor_type)
        {
            case CHAR_ANCHOR:
                mxList->addAttribute(u"text:anchor-type"_ustr, sXML_CDATA, u"as-char"_ustr);
                break;
            case PARA_ANCHOR:
                mxList->addAttribute(u"text:anchor-type"_ustr, sXML_CDATA, u"paragraph"_ustr);
                break;
            case PAGE_ANCHOR:
            case PAPER_ANCHOR:
            {
                mxList->addAttribute(u"text:anchor-type"_ustr, sXML_CDATA, u"page"_ustr);
                mxList->addAttribute(u"text:anchor-page-number"_ustr, sXML_CDATA, OUString::number(hbox->pgno + 1));
                break;
            }
        }
        if (hbox->style.anchor_type != CHAR_ANCHOR)
        {
            mxList->addAttribute(u"svg:x"_ustr, sXML_CDATA,
                OUString::number(WTMM( hbox->pgx + hbox->style.margin[0][0] )) + "mm");
            mxList->addAttribute(u"svg:y"_ustr, sXML_CDATA,
                OUString::number(WTMM( hbox->pgy + hbox->style.margin[0][2] )) + "mm");
        }
        mxList->addAttribute(u"svg:width"_ustr, sXML_CDATA,
            OUString::number(WTMM( hbox->box_xs + hbox->cap_xs )) + "mm");
        mxList->addAttribute(u"fo:min-height"_ustr, sXML_CDATA,
            OUString::number(WTMM( hbox->box_ys + hbox->cap_ys )) + "mm");
        startEl(u"draw:text-box"_ustr);
        mxList->clear();
        if (!hbox->caption.empty() && hbox->cap_pos % 2)  /* The caption is on the top */
        {
            parsePara(hbox->caption.front().get());
        }
        mxList->addAttribute( u"text:style-name"_ustr, sXML_CDATA, u"Standard"_ustr);
        startEl(u"text:p"_ustr);
        mxList->clear();
    }
    else{
         mxList->addAttribute(u"draw:z-index"_ustr, sXML_CDATA, OUString::number(hbox->zorder));
    }

    mxList->addAttribute(u"draw:style-name"_ustr, sXML_CDATA, "Txtbox" + OUString::number(hbox->style.boxnum));
    mxList->addAttribute(u"draw:name"_ustr, sXML_CDATA, "Frame" + OUString::number(hbox->style.boxnum));

    if( hbox->style.cap_len <= 0 || hbox->type != TXT_TYPE )
    {
        int x = 0;
        int y = 0;
        switch (hbox->style.anchor_type)
        {
            case CHAR_ANCHOR:
                mxList->addAttribute(u"text:anchor-type"_ustr, sXML_CDATA, u"as-char"_ustr);
                break;
            case PARA_ANCHOR:
                mxList->addAttribute(u"text:anchor-type"_ustr, sXML_CDATA, u"paragraph"_ustr);
                break;
            case PAGE_ANCHOR:
            case PAPER_ANCHOR:
            {
                mxList->addAttribute(u"text:anchor-type"_ustr, sXML_CDATA, u"page"_ustr);
                mxList->addAttribute(u"text:anchor-page-number"_ustr, sXML_CDATA, OUString::number(hbox->pgno + 1));
                break;
            }
        }
        if( hbox->style.anchor_type != CHAR_ANCHOR )
        {
            x += hbox->style.margin[0][0];
            y += hbox->style.margin[0][2];
        }
        mxList->addAttribute(u"svg:x"_ustr, sXML_CDATA,
            OUString::number(WTMM( hbox->pgx + x )) + "mm");
        mxList->addAttribute(u"svg:y"_ustr, sXML_CDATA,
            OUString::number(WTMM( hbox->pgy + y )) + "mm");
    }
    else
    {
        mxList->addAttribute(u"text:anchor-type"_ustr, sXML_CDATA, u"as-char"_ustr);
        mxList->addAttribute(u"svg:y"_ustr, sXML_CDATA, u"0cm"_ustr);
    }
    mxList->addAttribute(u"svg:width"_ustr, sXML_CDATA,
        OUString::number(WTMM( hbox->box_xs )) + "mm");
    if( hbox->style.cap_len > 0 && hbox->type != TXT_TYPE)
        mxList->addAttribute(u"fo:min-height"_ustr, sXML_CDATA,
            OUString::number(WTMM( hbox->box_ys + hbox->cap_ys)) + "mm");
    else
        mxList->addAttribute(u"svg:height"_ustr, sXML_CDATA,
            OUString::number(WTMM(hbox->box_ys )) + "mm");

    if( hbox->type != EQU_TYPE )
    {
        startEl(u"draw:text-box"_ustr);
        mxList->clear();
/* If captions are present and it is on the top */
        if (hbox->style.cap_len > 0 && (hbox->cap_pos % 2) && hbox->type == TBL_TYPE && !hbox->caption.empty())
        {
            parsePara(hbox->caption.front().get());
        }
        if( hbox->type == TBL_TYPE)               // Is Table
        {
            makeTable(hbox);
        }
        else if (!hbox->plists[0].empty())        // Is TextBox
        {
            parsePara(hbox->plists[0].front().get());
        }
/* If captions are present and it is on the bottom */
        if (hbox->style.cap_len > 0 && !(hbox->cap_pos % 2) && hbox->type == TBL_TYPE && !hbox->caption.empty())
        {
            parsePara(hbox->caption.front().get());
        }
        endEl(u"draw:text-box"_ustr);
// Caption exist and it is text-box
        if( hbox->style.cap_len > 0 && hbox->type == TXT_TYPE)
        {
            endEl(u"text:p"_ustr);
            if (!(hbox->cap_pos % 2) && !hbox->caption.empty())
            {
                parsePara(hbox->caption.front().get());
            }
            endEl(u"draw:text-box"_ustr);
        }
    }
    else                                          // is Formula
    {
        startEl(u"draw:object"_ustr);
        mxList->clear();
        makeFormula(hbox);
        endEl(u"draw:object"_ustr);
    }
}


/**
 * It must be converted into MathML.
 *
 */
void HwpReader::makeFormula(TxtBox * hbox)
{
    char mybuf[3000];
    HWPPara* pPar;

    hchar dest[3];
    size_t l = 0;

    pPar = hbox->plists[0].empty() ? nullptr : hbox->plists[0].front().get();
    while( pPar )
    {
        for (const auto& box : pPar->hhstr)
        {
            if (!box->hh)
                break;

            if (l >= sizeof(mybuf)-7)
                break;
            int res = hcharconv(box->hh, dest, UNICODE);
            for( int j = 0 ; j < res; j++ ){
                int c = dest[j];
                if( c < 32 )
                    c = ' ';
                if( c < 256 )
                    mybuf[l++] = sal::static_int_cast<char>(c);
                else
                {
                    mybuf[l++] = sal::static_int_cast<char>((c >> 8) & 0xff);
                    mybuf[l++] = sal::static_int_cast<char>(c & 0xff);
                }
            }
        }
        if (l >= sizeof(mybuf)-7)
            break;
        mybuf[l++] = '\n';
        pPar = pPar->Next();
    }
    mybuf[l] = '\0';

    Formula form( mybuf );
    form.setDocumentHandler(m_rxDocumentHandler);
    form.setAttributeListImpl(mxList.get());
    form.parse();
}

/**
 * Read the platform information. if the platform is Linux or Solaris, it needs to change
 * C: \ => Home, D: \ => changed to root (/). Because HWP uses DOS emulator.
 */

void HwpReader::makeHyperText(TxtBox * hbox)
{
    HyperText *hypert = hwpfile.GetHyperText();
    if( !hypert ) return;

    if (hypert->filename[0] != '\0') {
          ::std::string const tmp = hstr2ksstr(hypert->bookmark);
          ::std::string const tmp2 = hstr2ksstr(kstr2hstr(
#ifdef _WIN32
              reinterpret_cast<uchar const *>(urltowin(reinterpret_cast<char *>(hypert->filename)).c_str())).c_str());
#else
              reinterpret_cast<uchar const *>(urltounix(reinterpret_cast<char *>(hypert->filename)).c_str())).c_str());
#endif
          mxList->addAttribute(u"xlink:type"_ustr, sXML_CDATA, u"simple"_ustr);
          if (!tmp.empty() && strcmp(tmp.c_str(), "[HTML]")) {
              ::std::string tmp3(tmp2);
              tmp3.push_back('#');
              tmp3.append(tmp);
              mxList->addAttribute(u"xlink:href"_ustr, sXML_CDATA,
                  OUString(tmp3.c_str(), tmp3.size()+1, RTL_TEXTENCODING_EUC_KR));
          }
          else{
              mxList->addAttribute(u"xlink:href"_ustr, sXML_CDATA,
                  OUString(tmp2.c_str(), tmp2.size()+1, RTL_TEXTENCODING_EUC_KR));

          }
    }
    else
    {
        mxList->addAttribute(u"xlink:type"_ustr, sXML_CDATA, u"simple"_ustr);
        ::std::string tmp;
        tmp.push_back('#');
        tmp.append(hstr2ksstr(hypert->bookmark));
        mxList->addAttribute(u"xlink:href"_ustr, sXML_CDATA,
                OUString(tmp.c_str(), tmp.size()+1, RTL_TEXTENCODING_EUC_KR));
    }
    startEl(u"draw:a"_ustr);
    mxList->clear();
    makeTextBox(hbox);
    endEl(u"draw:a"_ustr);
}


/**
 * Read the platform information. if the platform is Linux or Solaris, it needs to change
 * C: \ => Home, D: \ => changed to root (/). Because HWP uses DOS emulator.
 */

void HwpReader::makePicture(Picture * hbox)
{
    switch (hbox->pictype)
    {
         case PICTYPE_OLE:
        case PICTYPE_EMBED:
        case PICTYPE_FILE:
        {
            if( hbox->style.cap_len > 0 )
            {
                mxList->addAttribute(u"draw:style-name"_ustr, sXML_CDATA,
                    "CapBox" + OUString::number(hbox->style.boxnum));
                mxList->addAttribute(u"draw:name"_ustr, sXML_CDATA, "CaptionBox" + OUString::number(hbox->style.boxnum));
                mxList->addAttribute(u"draw:z-index"_ustr, sXML_CDATA, OUString::number(hbox->zorder));
                switch (hbox->style.anchor_type)
                {
                    case CHAR_ANCHOR:
                        mxList->addAttribute(u"text:anchor-type"_ustr, sXML_CDATA, u"as-char"_ustr);
                        break;
                    case PARA_ANCHOR:
                        mxList->addAttribute(u"text:anchor-type"_ustr, sXML_CDATA, u"paragraph"_ustr);
                        break;
                    case PAGE_ANCHOR:
                    case PAPER_ANCHOR:
                    {
                        mxList->addAttribute(u"text:anchor-type"_ustr, sXML_CDATA, u"page"_ustr);
                        mxList->addAttribute(u"text:anchor-page-number"_ustr, sXML_CDATA,
                            OUString::number(hbox->pgno + 1));
                        break;
                    }
                }
                if (hbox->style.anchor_type != CHAR_ANCHOR)
                {
                    mxList->addAttribute(u"svg:x"_ustr, sXML_CDATA,
                        OUString::number(WTMM(  hbox->pgx + hbox->style.margin[0][0] )) + "mm");
                    mxList->addAttribute(u"svg:y"_ustr, sXML_CDATA,
                        OUString::number(WTMM(  hbox->pgy + hbox->style.margin[0][2] )) + "mm");
                }
                mxList->addAttribute(u"svg:width"_ustr, sXML_CDATA,
                    OUString::number(WTMM( hbox->box_xs + hbox->style.margin[1][0] + hbox->style.margin[1][1] )) + "mm");
                mxList->addAttribute(u"fo:min-height"_ustr, sXML_CDATA,
                    OUString::number(WTMM( hbox->box_ys + hbox->style.margin[1][2] + hbox->style.margin[1][3] + hbox->cap_ys )) + "mm");
                startEl(u"draw:text-box"_ustr);
                mxList->clear();
                if (!hbox->caption.empty() && hbox->cap_pos % 2)           /* Caption is on the top */
                {
                    parsePara(hbox->caption.front().get());
                }
                mxList->addAttribute( u"text:style-name"_ustr, sXML_CDATA, u"Standard"_ustr);
                startEl(u"text:p"_ustr);
                mxList->clear();
            }
            if( hbox->ishyper )
            {
                mxList->addAttribute(u"xlink:type"_ustr, sXML_CDATA, u"simple"_ustr);
#ifdef _WIN32
                if( hbox->follow[4] != 0 )
                    mxList->addAttribute("xlink:href", sXML_CDATA, hstr2OUString(kstr2hstr(hbox->follow.data() + 4).c_str()));
                else
                    mxList->addAttribute("xlink:href", sXML_CDATA, hstr2OUString(kstr2hstr(hbox->follow.data() + 5).c_str()));
#else
                if( hbox->follow[4] != 0 )
                    mxList->addAttribute(u"xlink:href"_ustr, sXML_CDATA,
                        hstr2OUString(kstr2hstr(reinterpret_cast<uchar const *>(urltounix(reinterpret_cast<char *>(hbox->follow.data() + 4)).c_str())).c_str()));
                else
                    mxList->addAttribute(u"xlink:href"_ustr, sXML_CDATA,
                        hstr2OUString(kstr2hstr(reinterpret_cast<uchar const *>(urltounix(reinterpret_cast<char *>(hbox->follow.data() + 5)).c_str())).c_str()));
#endif
                startEl(u"draw:a"_ustr);
                mxList->clear();
            }
            mxList->addAttribute(u"draw:style-name"_ustr, sXML_CDATA, "G" + OUString::number(hbox->style.boxnum));
            mxList->addAttribute(u"draw:name"_ustr, sXML_CDATA, "Image" + OUString::number(hbox->style.boxnum));

            if( hbox->style.cap_len <= 0 )
            {
                mxList->addAttribute(u"draw:z-index"_ustr, sXML_CDATA, OUString::number(hbox->zorder));
                switch (hbox->style.anchor_type)
                {
                    case CHAR_ANCHOR:
                        mxList->addAttribute(u"text:anchor-type"_ustr, sXML_CDATA, u"as-char"_ustr);
                        break;
                    case PARA_ANCHOR:
                        mxList->addAttribute(u"text:anchor-type"_ustr, sXML_CDATA, u"paragraph"_ustr);
                        break;
                    case PAGE_ANCHOR:
                    case PAPER_ANCHOR:
                    {
                        mxList->addAttribute(u"text:anchor-type"_ustr, sXML_CDATA, u"page"_ustr);
                        mxList->addAttribute(u"text:anchor-page-number"_ustr, sXML_CDATA,
                            OUString::number(hbox->pgno + 1));
                        break;
                    }
                }
                if (hbox->style.anchor_type != CHAR_ANCHOR)
                {
                    mxList->addAttribute(u"svg:x"_ustr, sXML_CDATA,
                        OUString::number(WTMM( hbox->pgx + hbox->style.margin[0][0] )) + "mm");
                    mxList->addAttribute(u"svg:y"_ustr, sXML_CDATA,
                        OUString::number(WTMM( hbox->pgy + hbox->style.margin[0][2] )) + "mm");
                }
            }
            else
            {
                mxList->addAttribute(u"text:anchor-type"_ustr, sXML_CDATA, u"as-char"_ustr);
                mxList->addAttribute(u"svg:y"_ustr, sXML_CDATA, u"0cm"_ustr);
            }
            mxList->addAttribute(u"svg:width"_ustr, sXML_CDATA,
                OUString::number(WTMM( hbox->box_xs + hbox->style.margin[1][0] + hbox->style.margin[1][1])) + "mm");
            mxList->addAttribute(u"svg:height"_ustr, sXML_CDATA,
                OUString::number(WTMM( hbox->box_ys + hbox->style.margin[1][2] + hbox->style.margin[1][3])) + "mm");

            if ( hbox->pictype == PICTYPE_FILE ){
#ifdef _WIN32
                sprintf(d->buf, "file:///%s", hbox->picinfo.picun.path );
                mxList->addAttribute("xlink:href", sXML_CDATA, hstr2OUString(kstr2hstr(reinterpret_cast<uchar *>(d->buf)).c_str()));
#else
                mxList->addAttribute(u"xlink:href"_ustr, sXML_CDATA,
                    hstr2OUString(kstr2hstr(reinterpret_cast<uchar const *>(urltounix(hbox->picinfo.picun.path).c_str())).c_str()));
#endif
                mxList->addAttribute(u"xlink:type"_ustr, sXML_CDATA, u"simple"_ustr);
                mxList->addAttribute(u"xlink:show"_ustr, sXML_CDATA, u"embed"_ustr);
                mxList->addAttribute(u"xlink:actuate"_ustr, sXML_CDATA, u"onLoad"_ustr);
            }

            if( hbox->pictype == PICTYPE_OLE )
                    startEl(u"draw:object-ole"_ustr);
            else
                    startEl(u"draw:image"_ustr);
            mxList->clear();
            if (hbox->pictype == PICTYPE_EMBED || hbox->pictype == PICTYPE_OLE)
            {
                startEl(u"office:binary-data"_ustr);
                mxList->clear();
                if( hbox->pictype == PICTYPE_EMBED ){
                         EmPicture *emp = hwpfile.GetEmPicture(hbox);
                         if( emp )
                         {
                             chars(base64_encode_string(emp->data.data(), emp->size));
                         }
                }
                else{
                         if( hwpfile.oledata ){
#ifdef _WIN32
                             LPSTORAGE srcsto;
                             LPUNKNOWN pObj;
                             wchar_t pathname[200];

                             MultiByteToWideChar(CP_ACP, 0, hbox->picinfo.picembed.embname, -1, pathname, 200);
                             int rc = hwpfile.oledata->pis->OpenStorage(pathname, nullptr,
                                     STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_TRANSACTED, nullptr, 0, &srcsto);
                             if (rc != S_OK) {
                                 chars("");
                             }
                             else{
                                 rc = OleLoad(srcsto, IID_IUnknown, nullptr, reinterpret_cast<LPVOID*>(&pObj));
                                 if( rc != S_OK ){
                                     srcsto->Release();
                                     chars("");
                                 }
                                 else{
                                     chars(base64_encode_string(reinterpret_cast<uchar*>(pObj), strlen(reinterpret_cast<char*>(pObj))));
                                     pObj->Release();
                                     srcsto->Release();
                                 }
                             }
#else
                             chars(u""_ustr);
#endif
                         }
                }
                endEl(u"office:binary-data"_ustr);
            }
            if( hbox->pictype == PICTYPE_OLE )
                    endEl(u"draw:object-ole"_ustr);
            else
                    endEl(u"draw:image"_ustr);
            if( hbox->ishyper )
            {
                endEl(u"draw:a"_ustr);
            }
            if( hbox->style.cap_len > 0 )
            {
                endEl(u"text:p"_ustr);
                if (!hbox->caption.empty() && !(hbox->cap_pos % 2))         /* Caption is at the bottom, */
                {
                    parsePara(hbox->caption.front().get());
                }
                endEl(u"draw:text-box"_ustr);
            }
            break;
        }
        case PICTYPE_DRAW:
              if( hbox->picinfo.picdraw.zorder > 0 )
                 mxList->addAttribute(u"draw:z-index"_ustr, sXML_CDATA,
                      OUString::number(hbox->picinfo.picdraw.zorder + 10000));
            makePictureDRAW(hbox->picinfo.picdraw.hdo, hbox);
            break;
        case PICTYPE_UNKNOWN:
            break;
    }
}

void HwpReader::makePictureDRAW(HWPDrawingObject *drawobj, const Picture* hbox)
{
    bool bIsRotate = false;

    while (drawobj)
    {
        mxList->addAttribute(u"draw:style-name"_ustr, sXML_CDATA, "Draw" + OUString::number(drawobj->index));
        int a = 0;
        int b = 0;

        switch (hbox->style.anchor_type)
        {
            case CHAR_ANCHOR:
                mxList->addAttribute(u"text:anchor-type"_ustr, sXML_CDATA, u"as-char"_ustr);
                break;
            case PARA_ANCHOR:
                mxList->addAttribute(u"text:anchor-type"_ustr, sXML_CDATA, u"paragraph"_ustr);
                break;
            case PAGE_ANCHOR:
            case PAPER_ANCHOR:
            {
                HWPInfo& hwpinfo = hwpfile.GetHWPInfo();
                mxList->addAttribute(u"text:anchor-type"_ustr, sXML_CDATA, u"page"_ustr);
                mxList->addAttribute(u"text:anchor-page-number"_ustr, sXML_CDATA, OUString::number(hbox->pgno + 1));
                a = hwpinfo.paper.left_margin;
                b = hwpinfo.paper.top_margin + hwpinfo.paper.header_length;
                break;
            }
        }

        if (drawobj->type == HWPDO_CONTAINER)
        {
            startEl(u"draw:g"_ustr);
            mxList->clear();
            makePictureDRAW(drawobj->child.get(), hbox);
            endEl(u"draw:g"_ustr);
        }
        else
        {
            double x = hbox->pgx;
            double y = hbox->pgy;

            bIsRotate = false;
            if( (drawobj->property.flag & HWPDO_FLAG_ROTATION) &&
                    (drawobj->property.parall.pt[0].y != drawobj->property.parall.pt[1].y) &&
                    //(drawobj->type == HWPDO_RECT || drawobj->type == HWPDO_ADVANCED_ELLIPSE || drawobj->type == HWPDO_ADVANCED_ARC )
                    (drawobj->type == HWPDO_RECT || drawobj->type == HWPDO_ADVANCED_ELLIPSE )
                    )
            {

                int i;
                ZZParall& pal = drawobj->property.parall;

                basegfx::B2DPoint pt[3], r_pt[3];
                for(i = 0 ; i < 3 ; i++ ){
                         basegfx::B2DPoint rot_origin(drawobj->property.rot_originx, drawobj->property.rot_originy);
                         pt[i].setX(pal.pt[i].x - rot_origin.getX());
                         /* Convert to a physical coordinate */
                         pt[i].setY(-(pal.pt[i].y - rot_origin.getY()));
                }

                double skewX;

                /* 2 - rotation angle calculation */
                double rotate = atan2(pt[1].getY() - pt[0].getY(), pt[1].getX() - pt[0].getX());

                for( i = 0 ; i < 3 ; i++){
                         r_pt[i].setX(pt[i].getX() * cos(-rotate) - pt[i].getY() * sin(-rotate));
                         r_pt[i].setY(pt[i].getY() * cos(-rotate) + pt[i].getX() * sin(-rotate));
                }

                /* 4 - Calculation of reflex angle */
                if (r_pt[2].getY() == r_pt[1].getY())
                         skewX = 0;
                else
                         skewX = atan((r_pt[2].getX() - r_pt[1].getX()) / (r_pt[2].getY() - r_pt[1].getY()));
                if( skewX >= M_PI_2 )
                         skewX -= M_PI;
                if( skewX <= -M_PI_2 )
                         skewX += M_PI;

                OUString trans;
                if( skewX != 0.0 && rotate != 0.0 ){
                    trans = "skewX (" + OUString::number(skewX)
                             + ") rotate (" + OUString::number(rotate)
                             + ") translate (" + OUString::number(WTMM(x + a + drawobj->offset2.x + pal.pt[0].x)) + "mm "
                             + OUString::number(WTMM(y + b + drawobj->offset2.y + pal.pt[0].y)) + "mm)";
                    bIsRotate = true;
                }
                else if( skewX != 0.0 ){
                    trans = "skewX (" + OUString::number(skewX)
                             + ") translate (" + OUString::number(WTMM(x + a + drawobj->offset2.x + pal.pt[0].x)) + "mm "
                             + OUString::number(WTMM(y + b + drawobj->offset2.y + pal.pt[0].y)) + "mm)";
                    bIsRotate = true;
                }
                else if( rotate != 0.0 ){
                    trans = "rotate (" + OUString::number(rotate)
                             + ") translate (" + OUString::number(WTMM(x + a + drawobj->offset2.x + pal.pt[0].x)) + "mm "
                             + OUString::number(WTMM(y + b + drawobj->offset2.y + pal.pt[0].y)) + "mm)";
                    bIsRotate = true;
                }
                if( bIsRotate ){
                    drawobj->extent.w = static_cast<int>(std::hypot(pt[1].getX() - pt[0].getX(), pt[1].getY() - pt[0].getY()));
                    drawobj->extent.h = static_cast<int>(std::hypot(pt[2].getX() - pt[1].getX(), pt[2].getY() - pt[1].getY()));
                    mxList->addAttribute(u"draw:transform"_ustr, sXML_CDATA, trans);
                }
            }
            switch (drawobj->type)
            {
                case HWPDO_LINE:                  /* Line-starting coordinates, ending coordinates. */
                    if( drawobj->u.line_arc.flip & 0x01 )
                    {
                        mxList->addAttribute(u"svg:x1"_ustr, sXML_CDATA,
                            OUString::number (WTMM(x + a + drawobj->offset2.x + drawobj->extent.w)) + "mm");
                        mxList->addAttribute(u"svg:x2"_ustr, sXML_CDATA,
                            OUString::number (WTMM( x + a + drawobj->offset2.x )) + "mm");
                    }
                    else
                    {
                        mxList->addAttribute(u"svg:x1"_ustr, sXML_CDATA,
                            OUString::number (WTMM( x + a + drawobj->offset2.x )) + "mm");
                        mxList->addAttribute(u"svg:x2"_ustr, sXML_CDATA,
                            OUString::number (WTMM(x + a + drawobj->offset2.x + drawobj->extent.w)) + "mm");
                    }
                    if( drawobj->u.line_arc.flip & 0x02 )
                    {
                        mxList->addAttribute(u"svg:y1"_ustr, sXML_CDATA,
                            OUString::number (WTMM( y + b + drawobj->offset2.y + drawobj->extent.h ) ) + "mm");
                        mxList->addAttribute(u"svg:y2"_ustr, sXML_CDATA,
                            OUString::number (WTMM( y + b  + drawobj->offset2.y )) + "mm");
                    }
                    else
                    {
                        mxList->addAttribute(u"svg:y1"_ustr, sXML_CDATA,
                            OUString::number (WTMM( y + b  + drawobj->offset2.y)) + "mm");
                        mxList->addAttribute(u"svg:y2"_ustr, sXML_CDATA,
                            OUString::number (WTMM(y + b + drawobj->offset2.y + drawobj->extent.h)) + "mm");
                    }

                    startEl(u"draw:line"_ustr);
                    mxList->clear();
                    endEl(u"draw:line"_ustr);
                    break;
                case HWPDO_RECT:                  /* rectangle - the starting position, vertical/horizontal  */
                    if( !bIsRotate )
                    {
                        mxList->addAttribute(u"svg:x"_ustr, sXML_CDATA,
                            OUString::number (WTMM( x + a + drawobj->offset2.x)) + "mm");
                        mxList->addAttribute(u"svg:y"_ustr, sXML_CDATA,
                            OUString::number (WTMM( y + b + drawobj->offset2.y)) + "mm");
                    }
                    mxList->addAttribute(u"svg:width"_ustr, sXML_CDATA,
                        OUString::number (WTMM( drawobj->extent.w )) + "mm");
                    mxList->addAttribute(u"svg:height"_ustr, sXML_CDATA,
                        OUString::number (WTMM( drawobj->extent.h )) + "mm");
                    if( drawobj->property.flag & 0x01 )
                    {
                        int value = drawobj->extent.w < drawobj->extent.h ?
                            drawobj->extent.w : drawobj->extent.h ;
                        mxList->addAttribute(u"draw:corner-radius"_ustr, sXML_CDATA,
                            OUString::number (WTMM( value/10 )) + "mm");
                    }
                    else if( drawobj->property.flag & 0x04 )
                    {
                        int value = drawobj->extent.w < drawobj->extent.h ?
                            drawobj->extent.w : drawobj->extent.h ;
                        mxList->addAttribute(u"draw:corner-radius"_ustr, sXML_CDATA,
                            OUString::number (WTMM( value / 2)) + "mm");
                    }

                    startEl(u"draw:rect"_ustr);
                    mxList->clear();
                    if( (drawobj->property.flag & HWPDO_FLAG_AS_TEXTBOX) &&
                        drawobj->property.pPara ) // As Textbox
                    {
                        HWPPara *pPara = drawobj->property.pPara;
                                //parsePara(pPara);
                        while(pPara)
                        {
                            make_text_p1( pPara, false );
                            pPara = pPara->Next();
                        }
                    }
                    endEl(u"draw:rect"_ustr);
                    break;
                case HWPDO_ELLIPSE:               /* Ellipse - the starting position, vertical/horizontal */
                case HWPDO_ADVANCED_ELLIPSE:      /* modified ellipse */
                {
                    if( !bIsRotate )
                    {
                        mxList->addAttribute(u"svg:x"_ustr, sXML_CDATA,
                            OUString::number (WTMM( x + a + drawobj->offset2.x)) + "mm");
                        mxList->addAttribute(u"svg:y"_ustr, sXML_CDATA,
                            OUString::number (WTMM( y + b + drawobj->offset2.y)) + "mm");
                    }

                    mxList->addAttribute(u"svg:width"_ustr, sXML_CDATA,
                                 OUString::number (WTMM( drawobj->extent.w )) + "mm");
                    mxList->addAttribute(u"svg:height"_ustr, sXML_CDATA,
                                 OUString::number (WTMM( drawobj->extent.h )) + "mm");
                    if( drawobj->type == HWPDO_ADVANCED_ELLIPSE ){
                        if (drawobj->u.arc.radial[0].x != drawobj->u.arc.radial[1].x ||
                            drawobj->u.arc.radial[0].y != drawobj->u.arc.radial[1].y) {

                            int Cx, Cy;
                            if (!o3tl::checked_add(drawobj->offset2.x, drawobj->extent.w, Cx) &&
                                !o3tl::checked_add(drawobj->offset2.y, drawobj->extent.h, Cy))
                            {
                                Cx /= 2;
                                Cy /= 2;

                                double start_angle = calcAngle( Cx, Cy, drawobj->u.arc.radial[0].x, drawobj->u.arc.radial[0].y );
                                double end_angle = calcAngle( Cx, Cy, drawobj->u.arc.radial[1].x, drawobj->u.arc.radial[1].y );
                                if( drawobj->property.fill_color < 0xffffff )
                                    mxList->addAttribute(u"draw:kind"_ustr, sXML_CDATA, u"section"_ustr);
                                else
                                    mxList->addAttribute(u"draw:kind"_ustr, sXML_CDATA, u"arc"_ustr);
                                mxList->addAttribute(u"draw:start-angle"_ustr, sXML_CDATA, OUString::number(start_angle ));
                                mxList->addAttribute(u"draw:end-angle"_ustr, sXML_CDATA, OUString::number(end_angle));
                            }
                        }
                    }
                    startEl(u"draw:ellipse"_ustr);
                    mxList->clear();
                    if( drawobj->property.flag >> 19 & 0x01 &&
                        drawobj->property.pPara ) // As Textbox
                    {
                        HWPPara *pPara = drawobj->property.pPara;
                                //parsePara(pPara);
                        while(pPara)
                        {
                            make_text_p1( pPara, false );
                            pPara = pPara->Next();
                        }
                    }
                    endEl(u"draw:ellipse"_ustr);
                    break;

                }
                case HWPDO_ARC:                   /* Arc */
                case HWPDO_ADVANCED_ARC:
                {
                    /*  If it is the arc, LibreOffice assumes the size as the entire ellipse size */
                    uint flip = drawobj->u.line_arc.flip;
                    if( !bIsRotate )
                    {
                              if( ( flip == 0 || flip == 2 ) && drawobj->type == HWPDO_ARC)
                                    mxList->addAttribute(u"svg:x"_ustr, sXML_CDATA,
                                         OUString::number (WTMM( x + a + drawobj->offset2.x - drawobj->extent.w)) + "mm");
                              else
                                    mxList->addAttribute(u"svg:x"_ustr, sXML_CDATA,
                                         OUString::number (WTMM( x + a + drawobj->offset2.x)) + "mm");
                              if( ( flip == 0 || flip == 1 ) && drawobj->type == HWPDO_ARC)
                                    mxList->addAttribute(u"svg:y"_ustr, sXML_CDATA,
                                         OUString::number (WTMM( y + b + drawobj->offset2.y - drawobj->extent.h)) + "mm");
                              else
                                    mxList->addAttribute(u"svg:y"_ustr, sXML_CDATA,
                                         OUString::number (WTMM( y + b + drawobj->offset2.y)) + "mm");
                    }

                    mxList->addAttribute(u"svg:width"_ustr, sXML_CDATA,
                                 OUString::number (WTMM( drawobj->extent.w * 2)) + "mm");
                    mxList->addAttribute(u"svg:height"_ustr, sXML_CDATA,
                                 OUString::number (WTMM( drawobj->extent.h * 2)) + "mm");
                    if( drawobj->property.flag & HWPDO_FLAG_DRAW_PIE ||
                                     drawobj->property.fill_color < 0xffffff )
                                mxList->addAttribute(u"draw:kind"_ustr, sXML_CDATA, u"section"_ustr);
                    else
                                mxList->addAttribute(u"draw:kind"_ustr, sXML_CDATA, u"arc"_ustr);

                    if( drawobj->type == HWPDO_ADVANCED_ARC ){
                                double start_angle, end_angle;
                                ZZParall& pal = drawobj->property.parall;

                                start_angle = atan2(pal.pt[0].y - pal.pt[1].y,pal.pt[1].x - pal.pt[0].x );
                                end_angle = atan2(pal.pt[2].y - pal.pt[1].y, pal.pt[1].x - pal.pt[2].x);

                                if( ( start_angle > end_angle ) && (start_angle - end_angle < M_PI ))
                                    std::swap( start_angle, end_angle );
                                mxList->addAttribute(u"draw:start-angle"_ustr, sXML_CDATA, OUString::number(basegfx::rad2deg(start_angle)));
                                mxList->addAttribute(u"draw:end-angle"_ustr, sXML_CDATA, OUString::number(basegfx::rad2deg(end_angle)));

                    }
                    else
                    {
                            if( drawobj->u.line_arc.flip == 0 )
                            {
                                 mxList->addAttribute(u"draw:start-angle"_ustr, sXML_CDATA, u"270"_ustr);
                                 mxList->addAttribute(u"draw:end-angle"_ustr, sXML_CDATA, u"0"_ustr);
                            }
                            else if( drawobj->u.line_arc.flip == 1 )
                            {
                                 mxList->addAttribute(u"draw:start-angle"_ustr, sXML_CDATA, u"180"_ustr);
                                 mxList->addAttribute(u"draw:end-angle"_ustr, sXML_CDATA, u"270"_ustr);
                            }
                            else if( drawobj->u.line_arc.flip == 2 )
                            {
                                 mxList->addAttribute(u"draw:start-angle"_ustr, sXML_CDATA, u"0"_ustr);
                                 mxList->addAttribute(u"draw:end-angle"_ustr, sXML_CDATA, u"90"_ustr);
                            }
                            else
                            {
                                 mxList->addAttribute(u"draw:start-angle"_ustr, sXML_CDATA, u"90"_ustr);
                                 mxList->addAttribute(u"draw:end-angle"_ustr, sXML_CDATA, u"180"_ustr);
                            }
                            }
                    startEl(u"draw:ellipse"_ustr);
                    mxList->clear();
                    if( drawobj->property.flag >> 19 & 0x01 &&
                        drawobj->property.pPara ) // As Textbox
                    {
                        HWPPara *pPara = drawobj->property.pPara;
                                //parsePara(pPara);
                        while(pPara)
                        {
                            make_text_p1( pPara, false );
                            pPara = pPara->Next();
                        }
                    }
                    endEl(u"draw:ellipse"_ustr);
                    break;

                }
                     case HWPDO_CURVE: /* Curve: converts to polygons. */
                {
                    bool bIsNatural = true;
                    if( drawobj->property.flag >> 5 & 0x01){
                        bIsNatural = false;
                    }
                    if( !bIsRotate )
                    {
                        mxList->addAttribute(u"svg:x"_ustr, sXML_CDATA,
                            OUString::number (WTMM( x + a + drawobj->offset2.x)) + "mm");
                        mxList->addAttribute(u"svg:y"_ustr, sXML_CDATA,
                            OUString::number (WTMM( y + b + drawobj->offset2.y)) + "mm");
                    }
                    mxList->addAttribute(u"svg:width"_ustr, sXML_CDATA,
                        OUString::number (WTMM( drawobj->extent.w )) + "mm");
                    mxList->addAttribute(u"svg:height"_ustr, sXML_CDATA,
                        OUString::number (WTMM( drawobj->extent.h )) + "mm");
                    mxList->addAttribute(
                        u"svg:viewBox"_ustr, sXML_CDATA,
                        "0 0 " + OUString::number(WTSM(drawobj->extent.w)) + " "
                            + OUString::number(WTSM(drawobj->extent.h)));

                    OUStringBuffer oustr;

                    if ((drawobj->u.freeform.npt > 2) &&
                        (o3tl::make_unsigned(drawobj->u.freeform.npt) <
                         (::std::numeric_limits<int>::max() / sizeof(double))))
                    {
                              int n, i;
                              n = drawobj->u.freeform.npt;

                              std::unique_ptr<double[]> xarr( new double[n+1] );
                              std::unique_ptr<double[]> yarr( new double[n+1] );
                              std::unique_ptr<double[]> tarr( new double[n+1] );

                              std::unique_ptr<double[]> xb;
                              std::unique_ptr<double[]> yb;

                              std::unique_ptr<double[]> carr;
                              std::unique_ptr<double[]> darr;


                              for( i = 0 ; i < n ; i++ ){
                                  xarr[i] = drawobj->u.freeform.pt[i].x;
                                  yarr[i] = drawobj->u.freeform.pt[i].y;
                                  tarr[i] = i;
                              }
                              xarr[n] = xarr[0];
                              yarr[n] = yarr[0];
                              tarr[n] = n;

                              if( !bIsNatural ){
                                  PeriodicSpline(n, tarr.get(), xarr.get(), xb, carr, darr);
                                  // prevent memory leak
                                  carr.reset();
                                  darr.reset();
                                  PeriodicSpline(n, tarr.get(), yarr.get(), yb, carr, darr);
                              }
                              else{
                                  NaturalSpline(n, tarr.get(), xarr.get(), xb, carr, darr);
                                  // prevent memory leak
                                  carr.reset();
                                  darr.reset();
                                  NaturalSpline(n, tarr.get(), yarr.get(), yb, carr, darr);
                              }

                              oustr.append(
                                  "M" + OUString::number(WTSM(xarr[0])) + " "
                                  + OUString::number(WTSM(yarr[0])) + "C"
                                  + OUString::number(WTSM(xarr[0] + xb[0]/3)) + " "
                                  + OUString::number(WTSM(yarr[0] + yb[0]/3)));

                              for( i = 1 ; i < n  ; i++ ){
                                  if( i == n -1 ){
                                      oustr.append(
                                          " " + OUString::number(WTSM(xarr[i] - xb[i]/3)) + " "
                                          + OUString::number(WTSM(yarr[i] - yb[i]/3)) + " "
                                          + OUString::number(WTSM(xarr[i])) + " "
                                          + OUString::number(WTSM(yarr[i])) + "z");
                                  }
                                  else{
                                      oustr.append(
                                          " " + OUString::number(WTSM(xarr[i] - xb[i]/3)) + " "
                                          + OUString::number(WTSM(yarr[i] - yb[i]/3)) + " "
                                          + OUString::number(WTSM(xarr[i])) + " "
                                          + OUString::number(WTSM(yarr[i])) + " "
                                          + OUString::number(WTSM(xarr[i] + xb[i]/3)) + " "
                                          + OUString::number(WTSM(yarr[i] + yb[i]/3)));
                                  }
                              }
                    }

                    mxList->addAttribute(u"svg:d"_ustr, sXML_CDATA, oustr.makeStringAndClear());

                    startEl(u"draw:path"_ustr);
                    mxList->clear();
                                                  // As Textbox
                    if( drawobj->property.flag >> 19 & 0x01 && drawobj->property.pPara )
                    {
                        HWPPara *pPara = drawobj->property.pPara;
                        while(pPara)
                        {
                            make_text_p1( pPara, false );
                            pPara = pPara->Next();
                        }
                    }
                    endEl(u"draw:path"_ustr);
                    break;
                }
                case HWPDO_CLOSED_FREEFORM:
                case HWPDO_FREEFORM:              /* polygon */
                {
                    bool bIsPolygon = false;

                    mxList->addAttribute(u"svg:x"_ustr, sXML_CDATA,
                                 OUString::number (WTMM( x + a + drawobj->offset2.x)) + "mm");
                    mxList->addAttribute(u"svg:y"_ustr, sXML_CDATA,
                                 OUString::number (WTMM( y + b + drawobj->offset2.y)) + "mm");

                    mxList->addAttribute(u"svg:width"_ustr, sXML_CDATA,
                        OUString::number (WTMM( drawobj->extent.w )) + "mm");
                    mxList->addAttribute(u"svg:height"_ustr, sXML_CDATA,
                        OUString::number (WTMM( drawobj->extent.h )) + "mm");

                    mxList->addAttribute(u"svg:viewBox"_ustr, sXML_CDATA, "0 0 " + OUString::number(WTSM(drawobj->extent.w)) + " " + OUString::number(WTSM(drawobj->extent.h)));

                    OUStringBuffer oustr;

                    if (drawobj->u.freeform.npt > 0)
                    {
                        oustr.append(
                            OUString::number(WTSM(drawobj->u.freeform.pt[0].x)) + ","
                            + OUString::number(WTSM(drawobj->u.freeform.pt[0].y)));
                        int i;
                        for (i = 1; i < drawobj->u.freeform.npt  ; i++)
                        {
                            oustr.append(
                                " " + OUString::number(WTSM(drawobj->u.freeform.pt[i].x)) + ","
                                + OUString::number(WTSM(drawobj->u.freeform.pt[i].y)));
                        }
                        if( drawobj->u.freeform.pt[0].x == drawobj->u.freeform.pt[i-1].x &&
                            drawobj->u.freeform.pt[0].y == drawobj->u.freeform.pt[i-1].y )
                        {
                            bIsPolygon = true;
                        }
                    }
                    mxList->addAttribute(u"draw:points"_ustr, sXML_CDATA, oustr.makeStringAndClear());

                    if( drawobj->property.fill_color <=  0xffffff ||
                        drawobj->property.pattern_type != 0)
                    {
                        bIsPolygon = true;
                    }

                    if(bIsPolygon)
                    {
                        startEl(u"draw:polygon"_ustr);
                        mxList->clear();
                        if( drawobj->property.flag >> 19 & 0x01 &&
                                                  // As Textbox
                            drawobj->property.pPara )
                        {
                            HWPPara *pPara = drawobj->property.pPara;
                            //  parsePara(pPara);
                            while(pPara)
                            {
                                make_text_p1( pPara, false );
                                pPara = pPara->Next();
                            }
                        }
                        endEl(u"draw:polygon"_ustr);
                    }
                    else
                    {
                        startEl(u"draw:polyline"_ustr);
                        mxList->clear();
                        if( drawobj->property.flag >> 19 & 0x01 &&
                                                  // As Textbox
                            drawobj->property.pPara )
                        {
                            HWPPara *pPara = drawobj->property.pPara;
                                //parsePara(pPara);
                            while(pPara)
                            {
                                make_text_p1( pPara, false );
                                pPara = pPara->Next();
                            }
                        }
                        endEl(u"draw:polyline"_ustr);
                    }
                    break;
                }
                case HWPDO_TEXTBOX:
                    if( !bIsRotate )
                    {
                        mxList->addAttribute(u"svg:x"_ustr, sXML_CDATA,
                            OUString::number (WTMM( x + a + drawobj->offset2.x)) + "mm");
                        mxList->addAttribute(u"svg:y"_ustr, sXML_CDATA,
                            OUString::number (WTMM( y + b + drawobj->offset2.y)) + "mm");
                    }
                    mxList->addAttribute(u"svg:width"_ustr, sXML_CDATA,
                        OUString::number (WTMM( drawobj->extent.w )) + "mm");
                    mxList->addAttribute(u"svg:height"_ustr, sXML_CDATA,
                        OUString::number (WTMM( drawobj->extent.h )) + "mm");
                    if( drawobj->property.flag & 0x01 )
                    {
                        int value = drawobj->extent.w < drawobj->extent.h ?
                            drawobj->extent.w : drawobj->extent.h ;
                        mxList->addAttribute(u"draw:corner-radius"_ustr, sXML_CDATA,
                            OUString::number (WTMM( value/10 )) + "mm");
                    }
                    else if( drawobj->property.flag & 0x04 )
                    {
                        int value = drawobj->extent.w < drawobj->extent.h ?
                            drawobj->extent.w : drawobj->extent.h ;
                        mxList->addAttribute(u"draw:corner-radius"_ustr, sXML_CDATA,
                            OUString::number (WTMM( value / 2)) + "mm");
                    }

                    startEl(u"draw:text-box"_ustr);
                    mxList->clear();

                    HWPPara *pPara = drawobj->u.textbox.h;
                                //parsePara(pPara);
                    while(pPara)
                    {
                        make_text_p1( pPara, false );
                        pPara = pPara->Next();
                    }

                    endEl(u"draw:text-box"_ustr);
                    break;
            }
        }
        mxList->clear();
        drawobj = drawobj->next.get();
    }
}

void HwpReader::makeLine()
{
    mxList->addAttribute(u"text:style-name"_ustr, sXML_CDATA, u"Horizontal Line"_ustr);
    startEl(u"text:p"_ustr);
    mxList->clear();
}

/**
 * Input-comment-hidden description: shows a hidden explanation to the users.
 * Parse out only strings, but it may contain paragraphs.
 */
void HwpReader::makeHidden(Hidden * hbox)
{
    hchar_string str;
    int res;
    hchar dest[3];

    mxList->addAttribute(u"text:condition"_ustr, sXML_CDATA, u""_ustr);
    mxList->addAttribute(u"text:string-value"_ustr, sXML_CDATA, u""_ustr);
    startEl(u"text:hidden-text"_ustr);
    mxList->clear();
    HWPPara *para = !hbox->plist.empty() ? hbox->plist.front().get() : nullptr;

    while (para)
    {
        for (const auto& box : para->hhstr)
        {
              if (!box->hh)
                  break;

              res = hcharconv(box->hh, dest, UNICODE);
              for( int j = 0 ; j < res ; j++ )
              {
                    str.push_back(dest[j]);
              }
        }
        para = para->Next();
    }
    makeChars(str);
    endEl(u"text:hidden-text"_ustr);
}


/**
 * Converts footnote to text:footnote, endnote to text:endnote
 */
void HwpReader::makeFootnote(Footnote * hbox)
{
    if (hbox->type)
    {
        mxList->addAttribute(u"text:id"_ustr, sXML_CDATA, "edn" + OUString::number(hbox->number));
        startEl(u"text:endnote"_ustr);
        mxList->clear();
        mxList->addAttribute(u"text:label"_ustr, sXML_CDATA, OUString::number(hbox->number));
        startEl(u"text:endnote-citation"_ustr);
        mxList->clear();
        chars(OUString::number(hbox->number));
        endEl(u"text:endnote-citation"_ustr);
        startEl(u"text:endnote-body"_ustr);
        if (!hbox->plist.empty())
            parsePara(hbox->plist.front().get());
        endEl(u"text:endnote-body"_ustr);
        endEl(u"text:endnote"_ustr);
    }
    else
    {
        mxList->addAttribute(u"text:id"_ustr, sXML_CDATA, "ftn" + OUString::number(hbox->number));
        startEl(u"text:footnote"_ustr);
        mxList->clear();
        mxList->addAttribute(u"text:label"_ustr, sXML_CDATA, OUString::number(hbox->number));
        startEl(u"text:footnote-citation"_ustr);
        mxList->clear();
        chars(OUString::number(hbox->number));
        endEl(u"text:footnote-citation"_ustr);
        startEl(u"text:footnote-body"_ustr);
        if (!hbox->plist.empty())
            parsePara(hbox->plist.front().get());
        endEl(u"text:footnote-body"_ustr);
        endEl(u"text:footnote"_ustr);
    }
}


/**
 * page/footnote/endnote/picture/table/formula number
 */
void HwpReader::makeAutoNum(AutoNum const * hbox)
{
    switch (hbox->type)
    {
        case PGNUM_AUTO:
            startEl(u"text:page-number"_ustr);
            chars(OUString::number(hbox->number));
            endEl(u"text:page-number"_ustr);
            break;
        case FNNUM_AUTO:
            break;
        case ENNUM_AUTO:
            break;
        case EQUNUM_AUTO:
        case PICNUM_AUTO:
            mxList->addAttribute(u"text:ref-name"_ustr,sXML_CDATA, "refIllustration" + OUString::number(hbox->number));
            mxList->addAttribute(u"text:name"_ustr,sXML_CDATA, u"Illustration"_ustr);
            mxList->addAttribute(u"style:num-format"_ustr,sXML_CDATA, u"1"_ustr);
            startEl(u"text:sequence"_ustr);
            chars(OUString::number(hbox->number));
            endEl(u"text:sequence"_ustr);
            break;
        case TBLNUM_AUTO:
            mxList->addAttribute(u"text:ref-name"_ustr,sXML_CDATA, "refTable" + OUString::number(hbox->number));
            mxList->addAttribute(u"text:name"_ustr,sXML_CDATA, u"Table"_ustr);
            mxList->addAttribute(u"style:num-format"_ustr,sXML_CDATA, u"1"_ustr);
            startEl(u"text:sequence"_ustr);
            chars(OUString::number(hbox->number));
            endEl(u"text:sequence"_ustr);
            break;
    }
}


void HwpReader::makeShowPageNum()
{
    ShowPageNum *hbox = d->pPn;
    int nPos = 0;
    if( hbox->where == 1 || hbox->where == 4 )
        nPos = 1;
    else if( hbox->where == 2 || hbox->where == 5 )
        nPos = 2;
    else if( hbox->where == 3 || hbox->where == 6 )
        nPos = 3;
    else                                          /* should not exist in this case. */
    {
        if( d->nPnPos == 1 )
            nPos = 1;
        else if( d->nPnPos == 3 )
            nPos = 3;
    }

    mxList->addAttribute(u"draw:style-name"_ustr, sXML_CDATA, "PNBox" + OUString::number(nPos));
    mxList->addAttribute(u"draw:name"_ustr, sXML_CDATA, "PageNumber" + OUString::number(nPos));
    mxList->addAttribute(u"text:anchor-type"_ustr, sXML_CDATA, u"paragraph"_ustr);
    mxList->addAttribute(u"svg:y"_ustr, sXML_CDATA, u"0cm"_ustr);
    mxList->addAttribute(u"svg:width"_ustr, sXML_CDATA, u"2.0cm"_ustr);
    mxList->addAttribute(u"fo:min-height"_ustr, sXML_CDATA, u"0.5cm"_ustr);
    startEl(u"draw:text-box"_ustr);
    mxList->clear();

    mxList->addAttribute(u"text:style-name"_ustr, sXML_CDATA, "PNPara" + OUString::number(nPos));
    startEl(u"text:p"_ustr);
    mxList->clear();
    if( hbox->shape > 2 )
        chars(u"- "_ustr);
    if( hbox->shape % 3 == 0 )
        mxList->addAttribute(u"style:num-format"_ustr, sXML_CDATA, u"1"_ustr);
    else if( hbox->shape % 3 == 1 )
        mxList->addAttribute(u"style:num-format"_ustr, sXML_CDATA, u"I"_ustr);
    else
        mxList->addAttribute(u"style:num-format"_ustr, sXML_CDATA, u"i"_ustr);
    mxList->addAttribute(u"text:select-page"_ustr, sXML_CDATA, u"current"_ustr);
    startEl(u"text:page-number"_ustr);
    mxList->clear();
    chars(u"2"_ustr);
    endEl(u"text:page-number"_ustr);
    if( hbox->shape > 2 )
        chars(u" -"_ustr);
    endEl(u"text:p"_ustr);
    endEl(u"draw:text-box"_ustr);
}


/**
 * mail merge operation using hwp addressbook and hwp data form.
 * not support operation in OO writer.
 */
void HwpReader::makeMailMerge(MailMerge *)
{
    hchar_string const boxstr = MailMerge::GetString();
    chars(hstr2OUString(boxstr.c_str()));
}


void HwpReader::makeOutline(Outline const * hbox)
{
    if( hbox->kind == 1 )
        chars( hbox->GetUnicode() );
}


void HwpReader::parsePara(HWPPara * para)
{
    bool bParaStart = false;
    while (para)
    {
        if( para->nch == 1)
        {
            if( !bParaStart )
            {
                mxList->addAttribute(u"text:style-name"_ustr, sXML_CDATA,
                    getPStyleName(para->GetParaShape().index));
                startEl(u"text:p"_ustr);
                mxList->clear();
            }
            if( d->bFirstPara && d->bInBody )
            {
/* for HWP's Bookmark */
                mxList->addAttribute(u"text:name"_ustr, sXML_CDATA, sBeginOfDoc);
                startEl(u"text:bookmark"_ustr);
                mxList->clear();
                endEl(u"text:bookmark"_ustr);
                d->bFirstPara = false;
            }
            if( d->bInHeader )
            {
                makeShowPageNum();
                d->bInHeader = false;
            }

            endEl(u"text:p"_ustr);
        }
        else
        {
            if (!para->ctrlflag)
            {
                if (para->contain_cshape)
                    make_text_p1(para, bParaStart);
                else
                    make_text_p0(para, bParaStart);
            }
            else
                make_text_p3(para, bParaStart);
        }
        bParaStart = false;
        para = para->Next();
    }
}


void HwpReader::startEl(const OUString& el)
{
    if (m_rxDocumentHandler)
        m_rxDocumentHandler->startElement(el, mxList);
}


void HwpReader::endEl(const OUString& el)
{
    if (m_rxDocumentHandler)
        m_rxDocumentHandler->endElement(el);
}


void HwpReader::chars(const OUString& s)
{
    if (m_rxDocumentHandler)
        m_rxDocumentHandler->characters(s);
}


namespace
{

constexpr OUStringLiteral IMPLEMENTATION_NAME = u"com.sun.comp.hwpimport.HwpImportFilter";
constexpr OUString SERVICE_NAME1 = u"com.sun.star.document.ImportFilter"_ustr;
constexpr OUString SERVICE_NAME2 = u"com.sun.star.document.ExtendedTypeDetection"_ustr;

class HwpImportFilter : public WeakImplHelper< XFilter, XImporter, XServiceInfo, XExtendedFilterDetection >
{
public:
    explicit HwpImportFilter(const Reference< XComponentContext >& );

public:
    // XFilter
    virtual sal_Bool SAL_CALL filter( const Sequence< PropertyValue >& aDescriptor ) override;
    virtual void SAL_CALL cancel() override;

    // XImporter
    virtual void SAL_CALL setTargetDocument( const Reference< XComponent >& xDoc) override;

    // XServiceInfo
    OUString SAL_CALL getImplementationName() override;
    Sequence< OUString > SAL_CALL getSupportedServiceNames() override;
    sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override;

    //XExtendedFilterDetection
    virtual OUString SAL_CALL detect( css::uno::Sequence< css::beans::PropertyValue >& rDescriptor ) override;

private:
    Reference< XFilter > rFilter;
    Reference< XImporter > rImporter;
};


HwpImportFilter::HwpImportFilter(const Reference< XComponentContext >& rxContext)
{
    try {
        Reference< XDocumentHandler > xHandler( rxContext->getServiceManager()->createInstanceWithContext( WRITER_IMPORTER_NAME, rxContext ), UNO_QUERY );

        rtl::Reference<HwpReader> p = new HwpReader;
        p->setDocumentHandler( xHandler );

        rImporter.set(xHandler, UNO_QUERY);
        rFilter = p;
    }
    catch( Exception & )
    {
        printf(" fail to instantiate %s\n", WRITER_IMPORTER_NAME.toUtf8().getStr() );
        exit( 1 );
    }
}

sal_Bool HwpImportFilter::filter( const Sequence< PropertyValue >& aDescriptor )
{
    // delegate to IchitaroImporter
    return rFilter->filter( aDescriptor );
}

void HwpImportFilter::cancel()
{
    rFilter->cancel();
}

void HwpImportFilter::setTargetDocument( const Reference< XComponent >& xDoc )
{
        // delegate
    rImporter->setTargetDocument( xDoc );
}

OUString HwpImportFilter::getImplementationName()
{
    return IMPLEMENTATION_NAME;
}

sal_Bool HwpImportFilter::supportsService( const OUString& ServiceName )
{
    return cppu::supportsService(this, ServiceName);
}

//XExtendedFilterDetection
OUString HwpImportFilter::detect( css::uno::Sequence< css::beans::PropertyValue >& rDescriptor )
{
    OUString sTypeName;

    utl::MediaDescriptor aDescriptor(rDescriptor);
    aDescriptor.addInputStream();

    Reference< XInputStream > xInputStream(
        aDescriptor[utl::MediaDescriptor::PROP_INPUTSTREAM], UNO_QUERY);

    if (xInputStream.is())
    {
        Sequence< sal_Int8 > aData;
        sal_Int32 nLen = HWPIDLen;
        if (
             nLen == xInputStream->readBytes(aData, nLen) &&
             detect_hwp_version(reinterpret_cast<const char*>(aData.getConstArray()))
           )
        {
            sTypeName = "writer_MIZI_Hwp_97";
        }
    }

    return sTypeName;
}

Sequence< OUString> HwpImportFilter::getSupportedServiceNames()
{
    return { SERVICE_NAME1, SERVICE_NAME2 };
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
hwpfilter_HwpImportFilter_get_implementation(
    css::uno::XComponentContext* context, css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new HwpImportFilter(context));
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
