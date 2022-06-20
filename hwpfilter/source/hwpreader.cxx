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
#include <osl/diagnose.h>
#include <tools/stream.hxx>
#include <basegfx/numeric/ftools.hxx>
#include <basegfx/point/b2dpoint.hxx>
#include <unotools/configmgr.hxx>

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
constexpr OUStringLiteral sXML_CDATA = u"CDATA";

namespace
{
double WTI(double x) { return x / 1800.; } // unit => inch
double WTMM(double x) { return x / 1800. * 25.4; } // unit => mm
int WTSM(double x) { return x / 1800. * 2540; } // unit ==> 1/100 mm

constexpr OUStringLiteral sBeginOfDoc(u"[\uBB38\uC11C\uC758 \uCC98\uC74C]");
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

    mxList->addAttribute("office:class", sXML_CDATA, "text");
    mxList->addAttribute("office:version", sXML_CDATA, "0.9");

    mxList->addAttribute("xmlns:office", "CDATA", "http://openoffice.org/2000/office");
    mxList->addAttribute("xmlns:style", "CDATA", "http://openoffice.org/2000/style");
    mxList->addAttribute("xmlns:text", "CDATA", "http://openoffice.org/2000/text");
    mxList->addAttribute("xmlns:table", "CDATA", "http://openoffice.org/2000/table");
    mxList->addAttribute("xmlns:draw", "CDATA", "http://openoffice.org/2000/drawing");
    mxList->addAttribute("xmlns:fo", "CDATA", "http://www.w3.org/1999/XSL/Format");
    mxList->addAttribute("xmlns:xlink", "CDATA", "http://www.w3.org/1999/xlink");
    mxList->addAttribute("xmlns:dc", "CDATA", "http://purl.org/dc/elements/1.1/");
    mxList->addAttribute("xmlns:meta", "CDATA", "http://openoffice.org/2000/meta");
    mxList->addAttribute("xmlns:number", "CDATA", "http://openoffice.org/2000/datastyle");
    mxList->addAttribute("xmlns:svg", "CDATA", "http://www.w3.org/2000/svg");
    mxList->addAttribute("xmlns:chart", "CDATA", "http://openoffice.org/2000/chart");
    mxList->addAttribute("xmlns:dr3d", "CDATA", "http://openoffice.org/2000/dr3d");
    mxList->addAttribute("xmlns:math", "CDATA", "http://www.w3.org/1998/Math/MathML");
    mxList->addAttribute("xmlns:form", "CDATA", "http://openoffice.org/2000/form");
    mxList->addAttribute("xmlns:script", "CDATA", "http://openoffice.org/2000/script");

    startEl("office:document");
    mxList->clear();

    makeMeta();
    makeStyles();
    makeAutoStyles();
    makeMasterStyles();
    makeBody();

    endEl("office:document");

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
    startEl("office:body");
    makeTextDecls();
    HWPPara *hwppara = hwpfile.GetFirstPara();
    d->bInBody = true;
    parsePara(hwppara);
    endEl("office:body");
    d->bInBody = false;
}


/**
 * make text decls
 */
void HwpReader::makeTextDecls()
{
    startEl("text:sequence-decls");
    mxList->addAttribute("text:display-outline-level", sXML_CDATA, "0");
    mxList->addAttribute("text:name", sXML_CDATA, "Illustration");
    startEl("text:sequence-decl");
    mxList->clear();
    endEl("text:sequence-decl");
    mxList->addAttribute("text:display-outline-level", sXML_CDATA, "0");
    mxList->addAttribute("text:name", sXML_CDATA, "Table");
    startEl("text:sequence-decl");
    mxList->clear();
    endEl("text:sequence-decl");
    mxList->addAttribute("text:display-outline-level", sXML_CDATA, "0");
    mxList->addAttribute("text:name", sXML_CDATA, "Text");
    startEl("text:sequence-decl");
    mxList->clear();
    endEl("text:sequence-decl");
    mxList->addAttribute("text:display-outline-level", sXML_CDATA, "0");
    mxList->addAttribute("text:name", sXML_CDATA, "Drawing");
    startEl("text:sequence-decl");
    mxList->clear();
    endEl("text:sequence-decl");
    endEl("text:sequence-decls");
}


/**
 * make office:meta
 * Completed
 */
void HwpReader::makeMeta()
{
    HWPInfo& hwpinfo = hwpfile.GetHWPInfo();

    startEl("office:meta");

    if (hwpinfo.summary.title[0])
    {
        startEl("dc:title");
        chars(hstr2OUString(hwpinfo.summary.title));
        endEl("dc:title");
    }

    if (hwpinfo.summary.subject[0])
    {
        startEl("dc:subject");
        chars(hstr2OUString(hwpinfo.summary.subject));
        endEl("dc:subject");
    }

    if (hwpinfo.summary.author[0])
    {
        startEl("meta:initial-creator");
        chars(hstr2OUString(hwpinfo.summary.author));
        endEl("meta:initial-creator");
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
        sprintf(d->buf,"%d-%02d-%02dT%02d:%02d:00",year,month,day,hour,minute);

        startEl("meta:creation-date");
        chars( OUString::createFromAscii(d->buf));
        endEl("meta:creation-date");
    }

    if (hwpinfo.summary.keyword[0][0] || hwpinfo.summary.etc[0][0])
    {
        startEl("meta:keywords");
        if (hwpinfo.summary.keyword[0][0])
        {
            startEl("meta:keyword");
            chars(hstr2OUString(hwpinfo.summary.keyword[0]));
            endEl("meta:keyword");
        }
        if (hwpinfo.summary.keyword[1][0])
        {
            startEl("meta:keyword");
            chars(hstr2OUString(hwpinfo.summary.keyword[1]));
            endEl("meta:keyword");
        }
        if (hwpinfo.summary.etc[0][0])
        {
            startEl("meta:keyword");
            chars(hstr2OUString(hwpinfo.summary.etc[0]));
            endEl("meta:keyword");
        }
        if (hwpinfo.summary.etc[1][0])
        {
            startEl("meta:keyword");
            chars(hstr2OUString(hwpinfo.summary.etc[1]));
            endEl("meta:keyword");
        }
        if (hwpinfo.summary.etc[2][0])
        {
            startEl("meta:keyword");
            chars(hstr2OUString(hwpinfo.summary.etc[2]));
            endEl("meta:keyword");
        }
        endEl("meta:keywords");
    }
    endEl("office:meta");
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
            mxList->addAttribute( "draw:name", sXML_CDATA, "LineType" + OUString::number(hdo->index));
            mxList->addAttribute( "draw:style", sXML_CDATA, "round");
            mxList->addAttribute( "draw:dots1", sXML_CDATA, "1");
            mxList->addAttribute( "draw:dots1-length", sXML_CDATA, OUString::number( LineStyle[prop->line_pstyle].dots1 * WTMM(prop->line_width) ) + "cm");
            if( prop->line_pstyle == 3 )
            {
                mxList->addAttribute( "draw:dots2", sXML_CDATA, "1");
                mxList->addAttribute( "draw:dots2-length", sXML_CDATA, OUString::number( LineStyle[prop->line_pstyle].dots2 * WTMM(prop->line_width) ) + "cm");
            }
            else if( prop->line_pstyle == 4 )
            {
                mxList->addAttribute( "draw:dots2", sXML_CDATA, "2");
                mxList->addAttribute( "draw:dots2-length", sXML_CDATA, OUString::number( LineStyle[prop->line_pstyle].dots2 * WTMM(prop->line_width)) + "cm");
            }
            mxList->addAttribute( "draw:distance", sXML_CDATA, OUString::number( LineStyle[prop->line_pstyle].distance * WTMM(prop->line_width)) + "cm");
            startEl("draw:stroke-dash");
            mxList->clear();
            endEl("draw:stroke-dash");
        }

        if( hdo->type == HWPDO_LINE || hdo->type == HWPDO_ARC || hdo->type == HWPDO_FREEFORM ||
            hdo->type == HWPDO_ADVANCED_ARC )
        {
            if( prop->line_tstyle > 0 &&
                o3tl::make_unsigned(prop->line_tstyle) < std::size(ArrowShape) &&
                !ArrowShape[prop->line_tstyle].bMade  )
            {
                ArrowShape[prop->line_tstyle].bMade = true;
                mxList->addAttribute("draw:name", sXML_CDATA,
                    OUString::createFromAscii(ArrowShape[prop->line_tstyle].name));
                if( prop->line_tstyle == 1 )
                {
                    mxList->addAttribute("svg:viewBox", sXML_CDATA, "0 0 20 30");
                    mxList->addAttribute("svg:d", sXML_CDATA, "m10 0-10 30h20z");
                }
                else if( prop->line_tstyle == 2 )
                {
                    mxList->addAttribute("svg:viewBox", sXML_CDATA, "0 0 1122 2243");
                    mxList->addAttribute("svg:d", sXML_CDATA, "m0 2108v17 17l12 42 30 34 38 21 43 4 29-8 30-21 25-26 13-34 343-1532 339 1520 13 42 29 34 39 21 42 4 42-12 34-30 21-42v-39-12l-4 4-440-1998-9-42-25-39-38-25-43-8-42 8-38 25-26 39-8 42z");
                }
                else if( prop->line_tstyle == 3 )
                {
                    mxList->addAttribute("svg:viewBox", sXML_CDATA, "0 0 30 30");
                    mxList->addAttribute("svg:d", sXML_CDATA, "m0 0h30v30h-30z");
                }
                startEl("draw:marker");
                mxList->clear();
                endEl("draw:marker");
            }
            if (prop->line_hstyle > 0 &&
                o3tl::make_unsigned(prop->line_hstyle) < std::size(ArrowShape) &&
                !ArrowShape[prop->line_hstyle].bMade)
            {
                ArrowShape[prop->line_hstyle].bMade = true;
                mxList->addAttribute("draw:name", sXML_CDATA,
                    OUString::createFromAscii(ArrowShape[prop->line_hstyle].name));
                if( prop->line_hstyle == 1 )
                {
                    mxList->addAttribute("svg:viewBox", sXML_CDATA, "0 0 20 30");
                    mxList->addAttribute("svg:d", sXML_CDATA, "m10 0-10 30h20z");
                }
                else if( prop->line_hstyle == 2 )
                {
                    mxList->addAttribute("svg:viewBox", sXML_CDATA, "0 0 1122 2243");
                    mxList->addAttribute("svg:d", sXML_CDATA, "m0 2108v17 17l12 42 30 34 38 21 43 4 29-8 30-21 25-26 13-34 343-1532 339 1520 13 42 29 34 39 21 42 4 42-12 34-30 21-42v-39-12l-4 4-440-1998-9-42-25-39-38-25-43-8-42 8-38 25-26 39-8 42z");
                }
                else if( prop->line_hstyle == 3 )
                {
                    mxList->addAttribute("svg:viewBox", sXML_CDATA, "0 0 20 20");
                    mxList->addAttribute("svg:d", sXML_CDATA, "m0 0h20v20h-20z");
                }
                startEl("draw:marker");
                mxList->clear();
                endEl("draw:marker");
            }
        }

        if( hdo->type != HWPDO_LINE )
        {
            if( prop->flag >> 18  & 0x01 )
            {
                mxList->addAttribute( "draw:name", sXML_CDATA, "fillimage" + OUString::number(hdo->index));

                EmPicture *emp = nullptr;
                if (prop->pictype && strlen(prop->szPatternFile) > 3)
                    emp = hwpfile.GetEmPictureByName(prop->szPatternFile);
                if (!emp)
                {
                    mxList->addAttribute( "xlink:href", sXML_CDATA,
                        hstr2OUString(kstr2hstr( reinterpret_cast<uchar const *>(urltounix(prop->szPatternFile).c_str())).c_str()));
                    mxList->addAttribute( "xlink:type", sXML_CDATA, "simple");
                    mxList->addAttribute( "xlink:show", sXML_CDATA, "embed");
                    mxList->addAttribute( "xlink:actuate", sXML_CDATA, "onLoad");
                }

                startEl("draw:fill-image");
                mxList->clear();
                if (emp)
                {
                    startEl("office:binary-data");
                    chars(base64_encode_string(emp->data.get(), emp->size));
                    endEl("office:binary-data");
                }
                endEl("draw:fill-image");
            }
/*  If there is a gradient, when a bitmap file is present, this is the first. */
            else if( prop->flag >> 16  & 0x01 )   /* existence gradient */
            {
                mxList->addAttribute( "draw:name", sXML_CDATA, "Grad" + OUString::number(hdo->index));
                switch( prop->gstyle )
                {
                    case 1 :
                        if( prop->center_y == 50 )
                            mxList->addAttribute( "draw:style", sXML_CDATA, "axial");
                        else
                            mxList->addAttribute( "draw:style", sXML_CDATA, "linear");
                        break;
                    case 2:
                    case 3:
                        mxList->addAttribute( "draw:style", sXML_CDATA, "radial");
                        break;
                    case 4:
                        mxList->addAttribute( "draw:style", sXML_CDATA, "square");
                        break;
                    default:
                        mxList->addAttribute( "draw:style", sXML_CDATA, "linear");
                        break;
                }
                mxList->addAttribute( "draw:cx", sXML_CDATA, OUString::number(prop->center_x) + "%");
                mxList->addAttribute( "draw:cy", sXML_CDATA, OUString::number(prop->center_y) + "%");

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
                        mxList->addAttribute( "draw:start-color", sXML_CDATA, rgb2str( prop->tocolor ));
                        mxList->addAttribute( "draw:end-color", sXML_CDATA, rgb2str( prop->fromcolor ));
                    }
                    else
                    {
                        mxList->addAttribute( "draw:start-color", sXML_CDATA, rgb2str( prop->fromcolor ));
                        mxList->addAttribute( "draw:end-color", sXML_CDATA, rgb2str( prop->tocolor ));
                    }
                }
                else
                {
                    mxList->addAttribute( "draw:start-color", sXML_CDATA,rgb2str( prop->tocolor ));
                    mxList->addAttribute( "draw:end-color", sXML_CDATA,rgb2str( prop->fromcolor ));
                }
                if( prop->angle > 0 && ( prop->gstyle == 1 || prop->gstyle == 4))
                {
                    int angle = 1800 - prop->angle * 10;
                    mxList->addAttribute( "draw:angle", sXML_CDATA, OUString::number(angle));
                }
                startEl("draw:gradient");
                mxList->clear();
                endEl("draw:gradient");
            }
                                                  /* hatch */
            else if( prop->pattern_type >> 24 & 0x01 )
            {
                int type = prop->pattern_type & 0xffffff;
                mxList->addAttribute( "draw:name", sXML_CDATA, "Hatch" + OUString::number(hdo->index));
                if( type < 4 )
                    mxList->addAttribute( "draw:style", sXML_CDATA, "single" );
                else
                    mxList->addAttribute( "draw:style", sXML_CDATA, "double" );
                mxList->addAttribute( "draw:color", sXML_CDATA, rgb2str( static_cast<int32_t>(prop->pattern_color) ));
                mxList->addAttribute( "draw:distance", sXML_CDATA, "0.12cm");
                switch( type )
                {
                    case 0 :
                    case 4 :
                        mxList->addAttribute( "draw:rotation", sXML_CDATA, "0");
                        break;
                    case 1 :
                        mxList->addAttribute( "draw:rotation", sXML_CDATA, "900");
                        break;
                    case 2 :
                        mxList->addAttribute( "draw:rotation", sXML_CDATA, "1350");
                        break;
                    case 3 :
                    case 5 :
                        mxList->addAttribute( "draw:rotation", sXML_CDATA, "450");
                        break;
                }
                startEl("draw:hatch");
                mxList->clear();
                endEl("draw:hatch");
            }
        }
        hdo = hdo->next.get();
    }
}


void HwpReader::makeStyles()
{
    HWPStyle& hwpstyle = hwpfile.GetHWPStyle();

    startEl("office:styles");

    int i;
    for (i = 0; i < hwpfile.getFBoxStyleCount(); i++)
    {
        if( hwpfile.getFBoxStyle(i)->boxtype == 'D' )
        {
            makeDrawMiscStyle(static_cast<HWPDrawingObject *>(hwpfile.getFBoxStyle(i)->cell) );
        }
    }

    mxList->addAttribute("style:name", sXML_CDATA, "Standard");
    mxList->addAttribute("style:family", sXML_CDATA, "paragraph");
    mxList->addAttribute("style:class", sXML_CDATA, "text");
    startEl("style:style");
    mxList->clear();

    mxList->addAttribute("fo:line-height", sXML_CDATA, "160%");
    mxList->addAttribute("fo:text-align", sXML_CDATA, "justify");
    startEl("style:properties");
    mxList->clear();
    startEl("style:tab-stops");

    for( i = 1 ; i < 40 ; i++)
    {
        mxList->addAttribute("style:position", sXML_CDATA,
            OUString::number( WTI(1000 * i)) + "inch");
        startEl("style:tab-stop");
        mxList->clear();
        endEl("style:tab-stop");
    }
    endEl("style:tab-stops");
    endEl("style:properties");

    endEl("style:style");

    for (int ii = 0; ii < hwpstyle.Num(); ii++)
    {
        unsigned char *stylename = reinterpret_cast<unsigned char *>(hwpstyle.GetName(ii));
        mxList->addAttribute("style:name", sXML_CDATA, hstr2OUString(kstr2hstr(stylename).c_str()));
        mxList->addAttribute("style:family", sXML_CDATA, "paragraph");
        mxList->addAttribute("style:parent-style-name", sXML_CDATA, "Standard");

        startEl("style:style");

        mxList->clear();

        parseCharShape(hwpstyle.GetCharShape(ii));
        parseParaShape(hwpstyle.GetParaShape(ii));

        startEl("style:properties");
        mxList->clear();
        endEl("style:properties");

        endEl("style:style");
    }

    {
        mxList->addAttribute( "style:name", sXML_CDATA, "Header");
        mxList->addAttribute( "style:family", sXML_CDATA, "paragraph");
        mxList->addAttribute( "style:parent-style-name", sXML_CDATA, "Standard");
        mxList->addAttribute( "style:class", sXML_CDATA, "extra");
        startEl("style:style");
        mxList->clear();
        endEl("style:style");
    }

    {
        mxList->addAttribute( "style:name", sXML_CDATA, "Footer");
        mxList->addAttribute( "style:family", sXML_CDATA, "paragraph");
        mxList->addAttribute( "style:parent-style-name", sXML_CDATA, "Standard");
        mxList->addAttribute( "style:class", sXML_CDATA, "extra");
        startEl("style:style");
        mxList->clear();

        endEl("style:style");
    }

    if( hwpfile.linenumber > 0)
    {
        mxList->addAttribute( "style:name", sXML_CDATA, "Horizontal Line");
        mxList->addAttribute( "style:family", sXML_CDATA, "paragraph");
        mxList->addAttribute( "style:parent-style-name", sXML_CDATA, "Standard");
        mxList->addAttribute( "style:class", sXML_CDATA, "html");
        startEl("style:style");
        mxList->clear();
        mxList->addAttribute( "fo:font-size", sXML_CDATA, "6pt");
        mxList->addAttribute( "fo:margin-top", sXML_CDATA, "0cm");
        mxList->addAttribute( "fo:margin-bottom", sXML_CDATA, "0cm");
        mxList->addAttribute( "style:border-line-width-bottom", sXML_CDATA, "0.02cm 0.035cm 0.002cm");
        mxList->addAttribute( "fo:padding", sXML_CDATA, "0cm");
        mxList->addAttribute( "fo:border-bottom", sXML_CDATA, "0.039cm double #808080");
        mxList->addAttribute( "text:number-lines", sXML_CDATA, "false");
        mxList->addAttribute( "text:line-number", sXML_CDATA, "0");
        mxList->addAttribute("fo:line-height", sXML_CDATA, "100%");
        startEl("style:properties");
        mxList->clear();
        endEl("style:properties");
        endEl("style:style");
    }

    HWPInfo& hwpinfo = hwpfile.GetHWPInfo();

    mxList->addAttribute("text:num-suffix", sXML_CDATA, ")");
    mxList->addAttribute("text:num-format", sXML_CDATA, "1");
    if( hwpinfo.beginfnnum != 1)
        mxList->addAttribute("text:offset", sXML_CDATA, OUString::number(hwpinfo.beginfnnum - 1));
    startEl("text:footnotes-configuration");
    mxList->clear();
    endEl("text:footnotes-configuration");

    endEl("office:styles");
}


/**
 * parse automatic styles from hwpfile
 * Define a style that is automatically reflected. For example, defining styles of each paragraph, tables, header, and etc,. at here. In Body, use the defined style.
 * 1. supports for the styles of paragraph, text, fbox, and page.
 */
void HwpReader::makeAutoStyles()
{
    int i;

    startEl("office:automatic-styles");

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
        mxList->addAttribute("style:name", sXML_CDATA, "PNPara" + OUString::number(i));
        mxList->addAttribute("style:family", sXML_CDATA, "paragraph");
        mxList->addAttribute("style:parent-style-name", sXML_CDATA, "Standard");
        startEl("style:style");
        mxList->clear();
        if( i == 1 )
            mxList->addAttribute("fo:text-align", sXML_CDATA, "start");
        else if ( i == 2 )
            mxList->addAttribute("fo:text-align", sXML_CDATA, "center");
        else if ( i == 3 )
            mxList->addAttribute("fo:text-align", sXML_CDATA, "end");
        startEl("style:properties");
        mxList->clear();
        endEl("style:properties");
        endEl("style:style");

        mxList->addAttribute("style:name", sXML_CDATA, "PNBox" + OUString::number(i));
        mxList->addAttribute("style:family", sXML_CDATA, "graphics");
        startEl("style:style");
        mxList->clear();

        mxList->addAttribute("fo:margin-top", sXML_CDATA, "0cm");
        mxList->addAttribute("fo:margin-bottom", sXML_CDATA, "0cm");
        mxList->addAttribute("style:wrap", sXML_CDATA, "run-through");
        mxList->addAttribute("style:vertical-pos", sXML_CDATA, "from-top");
        mxList->addAttribute("style:vertical-rel", sXML_CDATA, "paragraph");

        if( i == 1 )
            mxList->addAttribute("style:horizontal-pos", sXML_CDATA, "left");
        else if ( i == 2 )
            mxList->addAttribute("style:horizontal-pos", sXML_CDATA, "center");
        else if ( i == 3 )
            mxList->addAttribute("style:horizontal-pos", sXML_CDATA, "right");
        mxList->addAttribute("style:horizontal-rel", sXML_CDATA, "paragraph");
        mxList->addAttribute("fo:padding", sXML_CDATA, "0cm");
        mxList->addAttribute("stylefamily", sXML_CDATA, "graphics");
        startEl("style:properties");
        mxList->clear();
        endEl("style:properties");
        endEl("style:style");
    }

    for (i = 0; i < hwpfile.getDateFormatCount(); i++)
        makeDateFormat(hwpfile.getDateCode(i));

    makePageStyle();

    endEl("office:automatic-styles");
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
    startEl("office:master-styles");

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

    if (nMax > SAL_MAX_UINT16 && utl::ConfigManager::IsFuzzing())
    {
        SAL_WARN("filter.hwp", "too many pages: " << nMax << " clip to " << SAL_MAX_UINT16);
        nMax = SAL_MAX_UINT16;
    }

    for( i = 1; i <= nMax ; i++ )
    {
        if( i == 1 )
            mxList->addAttribute("style:name", sXML_CDATA, "Standard");
        else
            mxList->addAttribute("style:name", sXML_CDATA, "p" + OUString::number(i));
        mxList->addAttribute("style:page-master-name", sXML_CDATA,
                "pm" + OUString::number(hwpfile.GetPageMasterNum(i)));
        if( i < nMax )
            mxList->addAttribute("style:next-style-name", sXML_CDATA, "p" + OUString::number(i + 1));
        mxList->addAttribute("draw:style-name", sXML_CDATA, "master" + OUString::number(i));
        startEl("style:master-page");
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
            startEl("style:header");
            mxList->addAttribute("text:style-name", sXML_CDATA, "Standard");
            startEl("text:p");
            mxList->clear();
            endEl("text:p");
            endEl("style:header");

            startEl("style:footer");
            mxList->addAttribute("text:style-name", sXML_CDATA, "Standard");
            startEl("text:p");
            mxList->clear();
            endEl("text:p");
            endEl("style:footer");

            endEl("style:master-page");

            continue;
        }
// header
        if( pPage->header )
        {
            startEl("style:header");
            if( pPage->pagenumber && pPage->pagenumber->where < 4 )
            {
                d->bInHeader = true;
                d->pPn = pPage->pagenumber;
            }
            if (!pPage->header->plist.empty())
                parsePara(pPage->header->plist.front().get());
            d->bInHeader = false;
            d->pPn = nullptr;
            endEl("style:header");
        }
        if( pPage->header_even )
        {
            startEl("style:header");
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
            endEl("style:header");
        }
                                                  /* Will be the default. */
        else if (pPage->header_odd)
        {
            startEl("style:header");
            mxList->addAttribute("text:style-name", sXML_CDATA, "Standard");
            startEl("text:p");
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
            endEl("text:p");
            endEl("style:header");
        }
        if( pPage->header_odd )
        {
            startEl("style:header-left");
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
            endEl("style:header-left");
        }
                                                  /* Will be the default.  */
        else if (pPage->header_even)
        {
            startEl("style:header-left");
            mxList->addAttribute("text:style-name", sXML_CDATA, "Standard");
            startEl("text:p");
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
            endEl("text:p");
            endEl("style:header-left");
        }
        if( !pPage->header && !pPage->header_even && !pPage->header_odd )
        {
            startEl("style:header");
            mxList->addAttribute("text:style-name", sXML_CDATA, "Standard");
            startEl("text:p");
            mxList->clear();
            if( pPage->pagenumber && (pPage->pagenumber->where < 4 ||
                pPage->pagenumber->where == 7 ) )
            {
                d->pPn = pPage->pagenumber;
                makeShowPageNum();
                d->pPn = nullptr;
            }
            endEl("text:p");
            endEl("style:header");
        }
// footer
        if( pPage->footer )
        {
            startEl("style:footer");
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
            endEl("style:footer");
        }
        if( pPage->footer_even )
        {
            startEl("style:footer");
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
            endEl("style:footer");
        }
                                                  /* Will be the default. */
        else if (pPage->footer_odd)
        {
            startEl("style:footer");
            mxList->addAttribute("text:style-name", sXML_CDATA, "Standard");
            startEl("text:p");
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
            endEl("text:p");
            endEl("style:footer");
        }
        if( pPage->footer_odd )
        {
            startEl("style:footer-left");
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
            endEl("style:footer-left");
        }
                                                  /* Will be the default. */
        else if (pPage->footer_even)
        {
            startEl("style:footer-left");
            mxList->addAttribute("text:style-name", sXML_CDATA, "Standard");
            startEl("text:p");
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
            endEl("text:p");
            endEl("style:footer-left");
        }
        if( !pPage->footer && !pPage->footer_even && !pPage->footer_odd )
        {
            startEl("style:footer");
            mxList->addAttribute("text:style-name", sXML_CDATA, "Standard");
            startEl("text:p");
            mxList->clear();
            if( pPage->pagenumber && pPage->pagenumber->where >= 4
                && pPage->pagenumber->where != 7 )
            {
                d->pPn = pPage->pagenumber;
                makeShowPageNum();
                d->pPn = nullptr;
            }
            endEl("text:p");
            endEl("style:footer");
        }

        endEl("style:master-page");
    }
    endEl("office:master-styles");
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

    mxList->addAttribute("fo:font-size", sXML_CDATA, OUString::number(cshape->size / 25) + "pt");
    mxList->addAttribute("style:font-size-asian", sXML_CDATA, OUString::number(cshape->size / 25) + "pt");

    ::std::string const tmp = hstr2ksstr(kstr2hstr(
        reinterpret_cast<unsigned char const *>(hwpfont.GetFontName(0, cshape->font))).c_str());
    double fRatio = 1.0;
    int size = getRepFamilyName(tmp.c_str(), d->buf, fRatio);

    mxList->addAttribute("fo:font-family", sXML_CDATA,
        OUString(d->buf, size, RTL_TEXTENCODING_EUC_KR));
    mxList->addAttribute("style:font-family-asian", sXML_CDATA,
        OUString(d->buf, size, RTL_TEXTENCODING_EUC_KR));

    mxList->addAttribute("style:text-scale", sXML_CDATA,
        OUString::number(static_cast<int>(cshape->ratio * fRatio)) + "%");

    double sspace = (cshape->size / 25) * cshape->space / 100.;

    if (sspace != 0.)
    {
        mxList->addAttribute("fo:letter-spacing", sXML_CDATA,
            OUString::number(sspace) + "pt");
    }
    if (cshape->color[1] != 0)
        mxList->addAttribute("fo:color", sXML_CDATA,
            hcolor2str(cshape->color[1], 100, true));
    if (cshape->shade != 0)
        mxList->addAttribute("style:text-background-color", sXML_CDATA,
            hcolor2str(cshape->color[0], cshape->shade));
    if (cshape->attr & 0x01)
    {
        mxList->addAttribute("fo:font-style", sXML_CDATA, "italic");
        mxList->addAttribute("style:font-style-asian", sXML_CDATA, "italic");
    }
    else{
        mxList->addAttribute("fo:font-style", sXML_CDATA, "normal");
        mxList->addAttribute("style:font-style-asian", sXML_CDATA, "normal");
    }
    if (cshape->attr >> 1 & 0x01)
    {
        mxList->addAttribute("fo:font-weight", sXML_CDATA, "bold");
        mxList->addAttribute("style:font-weight-asian", sXML_CDATA, "bold");
    }
    else{
        mxList->addAttribute("fo:font-weight", sXML_CDATA, "normal");
        mxList->addAttribute("style:font-weight-asian", sXML_CDATA, "normal");
    }
    if (cshape->attr >> 2 & 0x01)
        mxList->addAttribute("style:text-underline", sXML_CDATA, "single");
    if (cshape->attr >> 3 & 0x01)
        mxList->addAttribute("style:text-outline", sXML_CDATA, "true");
    if (cshape->attr >> 4 & 0x01)
        mxList->addAttribute("fo:text-shadow", sXML_CDATA, "1pt 1pt");
    if (cshape->attr >> 5 & 0x01)
        mxList->addAttribute("style:text-position", sXML_CDATA, "super 58%");
    if (cshape->attr >> 6 & 0x01)
        mxList->addAttribute("style:text-position", sXML_CDATA, "sub 58%");

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
        mxList->addAttribute("fo:margin-left", sXML_CDATA, OUString::number
            (WTI(pshape->left_margin )) + "inch");
    if (pshape->right_margin != 0)
        mxList->addAttribute("fo:margin-right", sXML_CDATA, OUString::number
            (WTI(pshape->right_margin)) + "inch");
    if (pshape->pspacing_prev != 0)
        mxList->addAttribute("fo:margin-top", sXML_CDATA, OUString::number
            (WTI(pshape->pspacing_prev)) + "inch");
    if (pshape->pspacing_next != 0)
        mxList->addAttribute("fo:margin-bottom", sXML_CDATA, OUString::number
            (WTI(pshape->pspacing_next)) + "inch");
    if (pshape->indent != 0)
        mxList->addAttribute("fo:text-indent", sXML_CDATA, OUString::number
            (WTI(pshape->indent)) + "inch");
    if (pshape->lspacing != 0)
        mxList->addAttribute("fo:line-height", sXML_CDATA, OUString::number(pshape->lspacing) + "%");

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
        mxList->addAttribute("fo:text-align", sXML_CDATA, OUString::createFromAscii(align));

    if (pshape->outline)
        mxList->addAttribute("fo:border", sXML_CDATA, "0.002cm solid #000000");
    if( pshape->shade > 0 )
    {
        mxList->addAttribute("fo:background-color", sXML_CDATA,
            hcolor2str(0, pshape->shade));
    }

    if( pshape->pagebreak & 0x02 || pshape->pagebreak & 0x04)
        mxList->addAttribute("fo:break-before", sXML_CDATA, "page");
    else if( pshape->pagebreak & 0x01 )
        mxList->addAttribute("fo:break-before", sXML_CDATA, "column");

}


/**
 * Make the style of the Paragraph.
 */
void HwpReader::makePStyle(ParaShape const * pshape)
{
    int nscount = pshape->tabs[MAXTABS -1].type;
    mxList->addAttribute("style:name", sXML_CDATA, "P" + OUString::number(pshape->index));
    mxList->addAttribute("style:family", sXML_CDATA, "paragraph");
    startEl("style:style");
    mxList->clear();
    parseParaShape(pshape);
    if (pshape->cshape)
        parseCharShape(pshape->cshape.get());
    startEl("style:properties");
    mxList->clear();

    if( nscount )
    {
        unsigned char tf = 0;
        startEl("style:tab-stops");

        int tab_margin = pshape->left_margin + pshape->indent;
        if( tab_margin < 0 )
              tab_margin = 0;
        for( int i = 0 ; i < MAXTABS -1 ; i++)
        {
            if( i > 0 && pshape->tabs[i].position == 0. )
                break;
            if( pshape->tabs[i].position <= tab_margin )
                continue;
            mxList->addAttribute("style:position", sXML_CDATA,
                OUString::number(WTMM(pshape->tabs[i].position - tab_margin )) + "mm");
            if( pshape->tabs[i].type )
            {
                tf = 1;
                switch(pshape->tabs[i].type)
                {
                    case 1 :
                        mxList->addAttribute("style:type", sXML_CDATA, "right");
                        break;
                    case 2:
                        mxList->addAttribute("style:type", sXML_CDATA, "center");
                        break;
                    case 3:
                        mxList->addAttribute("style:type", sXML_CDATA, "char");
                        mxList->addAttribute("style:char", sXML_CDATA, ".");
                        break;
                }
            }
            if( pshape->tabs[i].dot_continue )
            {
                tf = 1;
                mxList->addAttribute("style:leader-char", sXML_CDATA, ".");
            }
            startEl("style:tab-stop");
            mxList->clear();
            endEl("style:tab-stop");

            if( (pshape->tabs[i].position != 1000 * i ) || tf )
            {
                if( !--nscount ) break;
            }
        }
        endEl("style:tab-stops");
    }
    endEl("style:properties");
    endEl("style:style");
}


/**
 * Create a style for the page. This includes the header/footer, footnote and more.
 * TODO: fo: background-color (no information)
 */
void HwpReader::makePageStyle()
{
     HWPInfo& hwpinfo = hwpfile.GetHWPInfo();
     int pmCount = hwpfile.getColumnCount();

     for( int i = 0 ; i < pmCount ; i++ ){
         mxList->addAttribute("style:name", sXML_CDATA, "pm" + OUString::number(i + 1));
         startEl("style:page-master");
         mxList->clear();


         switch( hwpinfo.paper.paper_kind )
         {
              case 3:                                   // A4
                    if( hwpinfo.paper.paper_direction )
                    {
                         mxList->addAttribute("fo:page-height",sXML_CDATA, "210mm");
                         mxList->addAttribute("fo:page-width",sXML_CDATA, "297mm");
                    }
                    else
                    {
                         mxList->addAttribute("fo:page-width",sXML_CDATA, "210mm");
                         mxList->addAttribute("fo:page-height",sXML_CDATA, "297mm");
                    }
                    break;
              case 4:                                   // 80 column
                    if( hwpinfo.paper.paper_direction )
                    {
                         mxList->addAttribute("fo:page-height",sXML_CDATA, "8.5inch");
                         mxList->addAttribute("fo:page-width",sXML_CDATA, "11inch");
                    }
                    else
                    {
                         mxList->addAttribute("fo:page-width",sXML_CDATA, "8.5inch");
                         mxList->addAttribute("fo:page-height",sXML_CDATA, "11inch");
                    }
                    break;
              case 5:                                   // B5
                    if( hwpinfo.paper.paper_direction )
                    {
                         mxList->addAttribute("fo:page-height",sXML_CDATA, "176mm");
                         mxList->addAttribute("fo:page-width",sXML_CDATA, "250mm");
                    }
                    else
                    {
                         mxList->addAttribute("fo:page-width",sXML_CDATA, "176mm");
                         mxList->addAttribute("fo:page-height",sXML_CDATA, "250mm");
                    }
                    break;
              case 6:                                   // B4
                    if( hwpinfo.paper.paper_direction )
                    {
                         mxList->addAttribute("fo:page-height",sXML_CDATA, "250mm");
                         mxList->addAttribute("fo:page-width",sXML_CDATA, "353mm");
                    }
                    else
                    {
                         mxList->addAttribute("fo:page-width",sXML_CDATA, "250mm");
                         mxList->addAttribute("fo:page-height",sXML_CDATA, "353mm");
                    }
                    break;
              case 7:
                    if( hwpinfo.paper.paper_direction )
                    {
                         mxList->addAttribute("fo:page-height",sXML_CDATA, "8.5inch");
                         mxList->addAttribute("fo:page-width",sXML_CDATA, "14inch");
                    }
                    else
                    {
                         mxList->addAttribute("fo:page-width",sXML_CDATA, "8.5inch");
                         mxList->addAttribute("fo:page-height",sXML_CDATA, "14inch");
                    }
                    break;
              case 8:
                    if( hwpinfo.paper.paper_direction )
                    {
                         mxList->addAttribute("fo:page-height",sXML_CDATA, "297mm");
                         mxList->addAttribute("fo:page-width",sXML_CDATA, "420mm");
                    }
                    else
                    {
                         mxList->addAttribute("fo:page-width",sXML_CDATA, "297mm");
                         mxList->addAttribute("fo:page-height",sXML_CDATA, "420mm");
                    }
                    break;
              case 0:
              case 1:
              case 2:
              default:
                    if( hwpinfo.paper.paper_direction )
                    {
                         mxList->addAttribute("fo:page-width",sXML_CDATA,
                              OUString::number(WTI(hwpinfo.paper.paper_height)) + "inch");
                         mxList->addAttribute("fo:page-height",sXML_CDATA,
                              OUString::number(WTI(hwpinfo.paper.paper_width)) + "inch");
                    }
                    else
                    {
                         mxList->addAttribute("fo:page-width",sXML_CDATA,
                              OUString::number(WTI(hwpinfo.paper.paper_width)) + "inch");
                         mxList->addAttribute("fo:page-height",sXML_CDATA,
                              OUString::number(WTI(hwpinfo.paper.paper_height)) + "inch");
                    }
                    break;

         }

         mxList->addAttribute("style:print-orientation",sXML_CDATA,
              OUString::createFromAscii(hwpinfo.paper.paper_direction ? "landscape" : "portrait"));
         if( hwpinfo.beginpagenum != 1)
              mxList->addAttribute("style:first-page-number",sXML_CDATA, OUString::number(hwpinfo.beginpagenum));

         if( hwpinfo.borderline ){
             mxList->addAttribute("fo:margin-left",sXML_CDATA,
                  OUString::number(WTI(hwpinfo.paper.left_margin - hwpinfo.bordermargin[0] + hwpinfo.paper.gutter_length)) + "inch");
             mxList->addAttribute("fo:margin-right",sXML_CDATA,
                  OUString::number(WTI(hwpinfo.paper.right_margin - hwpinfo.bordermargin[1])) + "inch");
             mxList->addAttribute("fo:margin-top",sXML_CDATA,
                  OUString::number(WTI(hwpinfo.paper.top_margin - hwpinfo.bordermargin[2])) + "inch");
             mxList->addAttribute("fo:margin-bottom",sXML_CDATA,
                  OUString::number(WTI(hwpinfo.paper.bottom_margin - hwpinfo.bordermargin[3])) + "inch");
         }
         else{
             mxList->addAttribute("fo:margin-left",sXML_CDATA,
                  OUString::number(WTI(hwpinfo.paper.left_margin + hwpinfo.paper.gutter_length)) + "inch");
             mxList->addAttribute("fo:margin-right",sXML_CDATA,
                  OUString::number(WTI(hwpinfo.paper.right_margin)) + "inch");
             mxList->addAttribute("fo:margin-top",sXML_CDATA,
                  OUString::number(WTI(hwpinfo.paper.top_margin)) + "inch");
             mxList->addAttribute("fo:margin-bottom",sXML_CDATA,
                  OUString::number(WTI(hwpinfo.paper.bottom_margin)) + "inch");
         }

         switch( hwpinfo.borderline )
         {
              case 1:
                    mxList->addAttribute("fo:border", sXML_CDATA,"0.002cm solid #000000");
                    break;
              case 3:
                    mxList->addAttribute("fo:border", sXML_CDATA,"0.002cm dotted #000000");
                    break;
              case 2:
                    mxList->addAttribute("fo:border", sXML_CDATA,"0.035cm solid #000000");
                    break;
              case 4:
                    mxList->addAttribute("style:border-line-width", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    mxList->addAttribute("fo:border", sXML_CDATA,"0.039cm double #000000");
                    break;
         }

         mxList->addAttribute("fo:padding-left", sXML_CDATA,
              OUString::number(WTI(hwpinfo.bordermargin[0])) + "inch");
         mxList->addAttribute("fo:padding-right", sXML_CDATA,
              OUString::number(WTI(hwpinfo.bordermargin[1])) + "inch");
         mxList->addAttribute("fo:padding-top", sXML_CDATA,
              OUString::number(WTI(hwpinfo.bordermargin[2])) + "inch");
         mxList->addAttribute("fo:padding-bottom", sXML_CDATA,
              OUString::number(WTI(hwpinfo.bordermargin[3])) + "inch");

     /* background color */
         if( hwpinfo.back_info.isset )
         {
             if( hwpinfo.back_info.color[0] > 0 || hwpinfo.back_info.color[1] > 0
                     || hwpinfo.back_info.color[2] > 0 ){
                 mxList->addAttribute("fo:background-color", sXML_CDATA,
                                      rgb2str(hwpinfo.back_info.color[0],
                                              hwpinfo.back_info.color[1],
                                              hwpinfo.back_info.color[2]));
             }
         }

         startEl("style:properties");
         mxList->clear();

     /* background image */
         if( hwpinfo.back_info.isset && hwpinfo.back_info.type > 0 )
         {
             if( hwpinfo.back_info.type == 1 ){
#ifdef _WIN32
                 mxList->addAttribute("xlink:href", sXML_CDATA,
                      hstr2OUString(kstr2hstr(reinterpret_cast<uchar const *>(urltowin(hwpinfo.back_info.filename).c_str())).c_str()));
#else
                 mxList->addAttribute("xlink:href", sXML_CDATA,
                    hstr2OUString(kstr2hstr( reinterpret_cast<uchar const *>(urltounix(hwpinfo.back_info.filename).c_str())).c_str()));
#endif
                 mxList->addAttribute("xlink:type", sXML_CDATA, "simple");
                 mxList->addAttribute("xlink:actuate", sXML_CDATA, "onLoad");
             }
             if( hwpinfo.back_info.flag >= 2)
                 mxList->addAttribute("style:repeat", sXML_CDATA, "stretch");
             else if( hwpinfo.back_info.flag == 1 ){
                 mxList->addAttribute("style:repeat", sXML_CDATA, "no-repeat");
                 mxList->addAttribute("style:position", sXML_CDATA, "center");
             }
             startEl("style:background-image");

             if( hwpinfo.back_info.type == 2 ){
                 startEl("office:binary-data");
                 mxList->clear();
                 chars(base64_encode_string(reinterpret_cast<unsigned char*>(hwpinfo.back_info.data.data()), hwpinfo.back_info.size));
                 endEl("office:binary-data");
             }
             endEl("style:background-image");
         }

         makeColumns( hwpfile.GetColumnDef(i) );

         endEl("style:properties");

    /* header style */
         startEl("style:header-style");
         mxList->addAttribute("svg:height", sXML_CDATA,
              OUString::number(WTI(hwpinfo.paper.header_length)) + "inch");
         mxList->addAttribute("fo:margin-bottom", sXML_CDATA, "0mm");

         startEl("style:properties");
         mxList->clear();
         endEl("style:properties");
         endEl("style:header-style");

    /* footer style */
         startEl("style:footer-style");
         mxList->addAttribute("svg:height", sXML_CDATA,
              OUString::number(WTI(hwpinfo.paper.footer_length)) + "inch");
         mxList->addAttribute("fo:margin-top", sXML_CDATA, "0mm");
         startEl("style:properties");
         mxList->clear();
         endEl("style:properties");
         endEl("style:footer-style");

    /* Footnote style */
         startEl("style:page-layout-properties");

         mxList->addAttribute("style:distance-before-sep", sXML_CDATA,
              OUString::number(WTI(hwpinfo.splinetext)) + "inch");
         mxList->addAttribute("style:distance-after-sep", sXML_CDATA,
              OUString::number(WTI(hwpinfo.splinefn)) + "inch");
         startEl("style:properties");
         mxList->clear();
         endEl("style:properties");
         if ( hwpinfo.fnlinetype == 2 )
              mxList->addAttribute("style:width", sXML_CDATA, "15cm");
         else if ( hwpinfo.fnlinetype == 1)
              mxList->addAttribute("style:width", sXML_CDATA, "2cm");
         else if ( hwpinfo.fnlinetype == 3)
              mxList->addAttribute("style:width", sXML_CDATA, "0cm");
         else
              mxList->addAttribute("style:width", sXML_CDATA, "5cm");

         startEl("style:footnote-sep");
         mxList->clear();
         endEl("style:footnote-sep");

         endEl("style:page-layout-properties");

         endEl("style:page-master");
     }
}

void HwpReader::makeColumns(ColumnDef const *coldef)
{
    if( !coldef ) return;
    mxList->addAttribute("fo:column-count", sXML_CDATA, OUString::number(coldef->ncols));
    startEl("style:columns");
    mxList->clear();
    if( coldef->separator != 0 )
    {
        switch( coldef->separator )
        {
             case 1:                           /* thin line */
                  mxList->addAttribute("style:width", sXML_CDATA, "0.02mm");
                  [[fallthrough]];
             case 3:                           /* dotted line */
                  mxList->addAttribute("style:style", sXML_CDATA, "dotted");
                  mxList->addAttribute("style:width", sXML_CDATA, "0.02mm");
                  break;
             case 2:                           /* thick line */
             case 4:                           /* double line */
                  mxList->addAttribute("style:width", sXML_CDATA, "0.35mm");
                  break;
             case 0:                           /* None */
             default:
                  mxList->addAttribute("style:style", sXML_CDATA, "none");
                  break;
        }
        startEl("style:column-sep");
        mxList->clear();
        endEl("style:column-sep");
    }
    double spacing = WTI(coldef->spacing)/ 2. ;
    for(int ii = 0 ; ii < coldef->ncols ; ii++)
    {
        if( ii == 0 )
             mxList->addAttribute("fo:margin-left", sXML_CDATA, "0mm");
        else
             mxList->addAttribute("fo:margin-left", sXML_CDATA,
                  OUString::number( spacing) + "inch");
        if( ii == ( coldef->ncols -1) )
             mxList->addAttribute("fo:margin-right", sXML_CDATA,"0mm");
        else
             mxList->addAttribute("fo:margin-right", sXML_CDATA,
                  OUString::number( spacing) + "inch");
        startEl("style:column");
        mxList->clear();
        endEl("style:column");
    }
    endEl("style:columns");
}

void HwpReader::makeTStyle(CharShape const * cshape)
{
    mxList->addAttribute("style:name", sXML_CDATA, "T" + OUString::number(cshape->index));
    mxList->addAttribute("style:family", sXML_CDATA, "text");
    startEl("style:style");
    mxList->clear();
    parseCharShape(cshape);
    startEl("style:properties");
    mxList->clear();
    endEl("style:properties");
    endEl("style:style");
}


void HwpReader::makeTableStyle(Table *tbl)
{
// table
    TxtBox *hbox = tbl->box;

    mxList->addAttribute("style:name", sXML_CDATA, "Table" + OUString::number(hbox->style.boxnum));
    mxList->addAttribute("style:family", sXML_CDATA,"table");
    startEl("style:style");
    mxList->clear();
    mxList->addAttribute("style:width", sXML_CDATA,
        OUString::number(WTMM(hbox->box_xs)) + "mm");
    mxList->addAttribute("table:align", sXML_CDATA,"left");
    mxList->addAttribute("fo:keep-with-next", sXML_CDATA,"false");
    startEl("style:properties");
    mxList->clear();
    endEl("style:properties");
    endEl("style:style");

// column
    for (size_t i = 0 ; i < tbl->columns.nCount -1 ; i++)
    {
        sprintf(d->buf,"Table%d.%c",hbox->style.boxnum, static_cast<char>('A'+i));
        mxList->addAttribute("style:name", sXML_CDATA, OUString::createFromAscii(d->buf));
        mxList->addAttribute("style:family", sXML_CDATA,"table-column");
        startEl("style:style");
        mxList->clear();
        mxList->addAttribute("style:column-width", sXML_CDATA,
            OUString::number(WTMM(tbl->columns.data[i+1] - tbl->columns.data[i])) + "mm");
        startEl("style:properties");
        mxList->clear();
        endEl("style:properties");
        endEl("style:style");
    }

// row
    for (size_t i = 0 ; i < tbl->rows.nCount -1 ; i++)
    {
        sprintf(d->buf,"Table%d.row%" SAL_PRI_SIZET "u",hbox->style.boxnum, i + 1);
        mxList->addAttribute("style:name", sXML_CDATA, OUString::createFromAscii(d->buf));
        mxList->addAttribute("style:family", sXML_CDATA,"table-row");
        startEl("style:style");
        mxList->clear();
        mxList->addAttribute("style:row-height", sXML_CDATA,
            OUString::number(WTMM(tbl->rows.data[i+1] - tbl->rows.data[i])) + "mm");
        startEl("style:properties");
        mxList->clear();
        endEl("style:properties");
        endEl("style:style");
    }

// cell
    for (auto const& tcell : tbl->cells)
    {
        sprintf(d->buf,"Table%d.%c%d",hbox->style.boxnum, 'A'+ tcell->nColumnIndex, tcell->nRowIndex +1);
        mxList->addAttribute("style:name", sXML_CDATA, OUString::createFromAscii(d->buf));
        mxList->addAttribute("style:family", sXML_CDATA,"table-cell");
        startEl("style:style");
        mxList->clear();
        Cell *cl = tcell->pCell;
        if( cl->ver_align == 1 )
            mxList->addAttribute("fo:vertical-align", sXML_CDATA,"middle");

        if(cl->linetype[2] == cl->linetype[3] && cl->linetype[2] == cl->linetype[0]
            && cl->linetype[2] == cl->linetype[1])
        {
            switch( cl->linetype[2] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute("fo:border", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute("fo:border", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute("style:border-line-width", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    mxList->addAttribute("fo:border", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
        }
        else
        {
            switch( cl->linetype[0] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute("fo:border-left", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute("fo:border-left", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute("style:border-line-width-left", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    mxList->addAttribute("fo:border-left", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
            switch( cl->linetype[1] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute("fo:border-right", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute("fo:border-right", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute("style:border-line-width-right", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    mxList->addAttribute("fo:border-right", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
            switch( cl->linetype[2] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute("fo:border-top", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute("fo:border-top", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute("style:border-line-width-top", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    mxList->addAttribute("fo:border-top", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
            switch( cl->linetype[3] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute("fo:border-bottom", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute("fo:border-bottom", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute("style:border-line-width-bottom", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    mxList->addAttribute("fo:border-bottom", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
        }
        if(cl->shade != 0)
            mxList->addAttribute("fo:background-color", sXML_CDATA,
                hcolor2str(sal::static_int_cast<uchar>(cl->color),
                                sal::static_int_cast<uchar>(cl->shade)));

        startEl("style:properties");
        mxList->clear();
        endEl("style:properties");

        endEl("style:style");
    }
}


void HwpReader::makeDrawStyle( HWPDrawingObject * hdo, FBoxStyle * fstyle)
{
    while( hdo )
    {
        mxList->addAttribute("style:name", sXML_CDATA, "Draw" + OUString::number(hdo->index));
        mxList->addAttribute("style:family", sXML_CDATA, "graphics");

        startEl("style:style");
        mxList->clear();

        switch (fstyle->txtflow)
        {
            case 0:
                break;
            case 1:
                mxList->addAttribute("style:wrap", sXML_CDATA, "run-through");
                break;
            case 2:
                mxList->addAttribute("style:wrap", sXML_CDATA, "dynamic");
                break;
        }
        tools::Long color;
// invisible line
        if( hdo->property.line_color > 0xffffff )
        {
            mxList->addAttribute("draw:stroke", sXML_CDATA, "none" );
        }
        else
        {

            if( hdo->property.line_pstyle == 0 )
                mxList->addAttribute("draw:stroke", sXML_CDATA, "solid" );
            else if( hdo->property.line_pstyle < 5 )
            {
                mxList->addAttribute("draw:stroke", sXML_CDATA, "dash" );
                mxList->addAttribute("draw:stroke-dash", sXML_CDATA, "LineType" + OUString::number(hdo->index));
            }
            mxList->addAttribute("svg:stroke-width", sXML_CDATA,
                OUString::number( WTMM(hdo->property.line_width)) + "mm");
            mxList->addAttribute("svg:stroke-color", sXML_CDATA,
                                 rgb2str(static_cast<int32_t>(hdo->property.line_color)));
        }

        if( hdo->type == HWPDO_LINE || hdo->type == HWPDO_ARC ||
            hdo->type == HWPDO_FREEFORM || hdo->type == HWPDO_ADVANCED_ARC )
        {

            if( hdo->property.line_tstyle > 0 &&
                o3tl::make_unsigned(hdo->property.line_tstyle) < std::size(ArrowShape) )
            {
                mxList->addAttribute("draw:marker-start", sXML_CDATA,
                    OUString::createFromAscii(ArrowShape[hdo->property.line_tstyle].name) );
                if( hdo->property.line_width > 100 )
                         mxList->addAttribute("draw:marker-start-width", sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 3)) + "mm");
                else if( hdo->property.line_width > 80 )
                         mxList->addAttribute("draw:marker-start-width", sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 4)) + "mm");
                else if( hdo->property.line_width > 60 )
                         mxList->addAttribute("draw:marker-start-width", sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 5)) + "mm");
                else if( hdo->property.line_width > 40 )
                         mxList->addAttribute("draw:marker-start-width", sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 6)) + "mm");
                else
                         mxList->addAttribute("draw:marker-start-width", sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 7)) + "mm");
            }

            if( hdo->property.line_hstyle > 0 &&
                o3tl::make_unsigned(hdo->property.line_hstyle) < std::size(ArrowShape) )
            {
                mxList->addAttribute("draw:marker-end", sXML_CDATA,
                    OUString::createFromAscii(ArrowShape[hdo->property.line_hstyle].name) );
                if( hdo->property.line_width > 100 )
                         mxList->addAttribute("draw:marker-end-width", sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 3)) + "mm");
                else if( hdo->property.line_width > 80 )
                         mxList->addAttribute("draw:marker-end-width", sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 4)) + "mm");
                else if( hdo->property.line_width > 60 )
                         mxList->addAttribute("draw:marker-end-width", sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 5)) + "mm");
                else if( hdo->property.line_width > 40 )
                         mxList->addAttribute("draw:marker-end-width", sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 6)) + "mm");
                else
                         mxList->addAttribute("draw:marker-end-width", sXML_CDATA,
                              OUString::number( WTMM(hdo->property.line_width * 7)) + "mm");
            }
        }

        if(hdo->type != HWPDO_LINE )
        {
            if( hdo->property.flag >> 19 & 0x01 )
            {
                mxList->addAttribute( "draw:textarea-horizontal-align", sXML_CDATA, "center");
            }

            color = hdo->property.fill_color;

            if( hdo->property.flag >> 18 & 0x01 ) // bitmap pattern
            {
                mxList->addAttribute("draw:fill", sXML_CDATA, "bitmap");
                mxList->addAttribute("draw:fill-image-name", sXML_CDATA,
                    "fillimage" + OUString::number(hdo->index));
                                                  // bitmap resizing
                if( hdo->property.flag >> 3 & 0x01 )
                {
                    mxList->addAttribute("style:repeat", sXML_CDATA, "stretch");
                }
                else
                {
                    mxList->addAttribute("style:repeat", sXML_CDATA, "repeat");
                    mxList->addAttribute("draw:fill-image-ref-point", sXML_CDATA, "top-left");
                }
                if( hdo->property.flag >> 20 & 0x01 )
                {
                    if( hdo->property.luminance > 0 )
                    {
                        mxList->addAttribute("draw:transparency", sXML_CDATA,
                            OUString::number(hdo->property.luminance) + "%");
                    }
                }

            }
                                                  // Gradation
            else if( hdo->property.flag >> 16 & 0x01 )
            {
                mxList->addAttribute("draw:fill", sXML_CDATA, "gradient");
                mxList->addAttribute("draw:fill-gradient-name", sXML_CDATA, "Grad" + OUString::number(hdo->index));
                mxList->addAttribute("draw:gradient-step-count", sXML_CDATA, OUString::number(hdo->property.nstep));

            }
                                                  // Hatching
            else if( hdo->property.pattern_type >> 24 & 0x01 )
            {
                mxList->addAttribute("draw:fill", sXML_CDATA, "hatch");
                mxList->addAttribute("draw:fill-hatch-name", sXML_CDATA, "Hatch" + OUString::number(hdo->index));
                if( color < 0xffffff )
                {
                    mxList->addAttribute("draw:fill-color", sXML_CDATA,
                                         rgb2str(static_cast<int32_t>(color)));
                    mxList->addAttribute("draw:fill-hatch-solid", sXML_CDATA, "true");
                }
            }
            else if( color <= 0xffffff )
            {
                mxList->addAttribute("draw:fill", sXML_CDATA, "solid");
                mxList->addAttribute("draw:fill-color", sXML_CDATA,
                                     rgb2str(static_cast<int32_t>(color)));
            }
            else
                mxList->addAttribute("draw:fill", sXML_CDATA, "none");
        }

        if( fstyle->anchor_type == CHAR_ANCHOR )
        {
            mxList->addAttribute("style:vertical-pos", sXML_CDATA, "top");
            mxList->addAttribute("style:vertical-rel", sXML_CDATA, "baseline");
        }

        startEl("style:properties");
        mxList->clear();
        endEl("style:properties");
        endEl("style:style");

        if( hdo->type == 0 )
        {
            makeDrawStyle( hdo->child.get(), fstyle );
        }
        hdo = hdo->next.get();
    }
}


void HwpReader::makeCaptionStyle(FBoxStyle * fstyle)
{
    mxList->addAttribute("style:name", sXML_CDATA, "CapBox" + OUString::number(fstyle->boxnum));
    mxList->addAttribute("style:family", sXML_CDATA, "graphics");
    startEl("style:style");
    mxList->clear();
    mxList->addAttribute("fo:margin-left", sXML_CDATA, "0cm");
    mxList->addAttribute("fo:margin-right", sXML_CDATA, "0cm");
    mxList->addAttribute("fo:margin-top", sXML_CDATA, "0cm");
    mxList->addAttribute("fo:margin-bottom", sXML_CDATA, "0cm");
    mxList->addAttribute("fo:padding", sXML_CDATA, "0cm");
    switch (fstyle->txtflow)
    {
        case 0:
            mxList->addAttribute("style:wrap", sXML_CDATA, "none");
            break;
        case 1:
            if( fstyle->boxtype == 'G' )
                mxList->addAttribute("style:run-through", sXML_CDATA, "background");
            mxList->addAttribute("style:wrap", sXML_CDATA, "run-through");
            break;
        case 2:
            mxList->addAttribute("style:wrap", sXML_CDATA, "dynamic");
            break;
    }
    if (fstyle->anchor_type == CHAR_ANCHOR)
    {
        mxList->addAttribute("style:vertical-pos", sXML_CDATA, "top");
        mxList->addAttribute("style:vertical-rel", sXML_CDATA, "baseline");
        mxList->addAttribute("style:horizontal-pos", sXML_CDATA, "center");
        mxList->addAttribute("style:horizontal-rel", sXML_CDATA, "paragraph");
    }
    else
    {

        switch (-(fstyle->xpos))
        {
            case 2:
                mxList->addAttribute("style:horizontal-pos", sXML_CDATA, "right");
                break;
            case 3:
                mxList->addAttribute("style:horizontal-pos", sXML_CDATA, "center");
                break;
            case 1:
            default:
                mxList->addAttribute("style:horizontal-pos", sXML_CDATA, "from-left");
                break;
        }
        switch (-(fstyle->ypos))
        {
            case 2:
                mxList->addAttribute("style:vertical-pos", sXML_CDATA, "bottom");
                break;
            case 3:
                mxList->addAttribute("style:vertical-pos", sXML_CDATA, "middle");
                break;
            case 1:
            default:
                mxList->addAttribute("style:vertical-pos", sXML_CDATA, "from-top");
                break;
        }
        if ( fstyle->anchor_type == PARA_ANCHOR )
        {
            mxList->addAttribute("style:vertical-rel", sXML_CDATA, "paragraph");
            mxList->addAttribute("style:horizontal-rel", sXML_CDATA, "paragraph");
        }
        else
        {
            mxList->addAttribute("style:vertical-rel", sXML_CDATA, "page-content");
            mxList->addAttribute("style:horizontal-rel", sXML_CDATA, "page-content");
        }
    }
    startEl("style:properties");
    mxList->clear();
    endEl("style:properties");
    endEl("style:style");
    if( fstyle->boxtype == 'G' )
    {
        mxList->addAttribute("style:name", sXML_CDATA, "G" + OUString::number(fstyle->boxnum));
    }
    else
    {
        mxList->addAttribute("style:name", sXML_CDATA, "Txtbox" + OUString::number(fstyle->boxnum));
    }

    mxList->addAttribute("style:family", sXML_CDATA, "graphics");
    startEl("style:style");
    mxList->clear();

    mxList->addAttribute("fo:margin-left", sXML_CDATA, "0cm");
    mxList->addAttribute("fo:margin-right", sXML_CDATA, "0cm");
    mxList->addAttribute("fo:margin-top", sXML_CDATA, "0cm");
    mxList->addAttribute("fo:margin-bottom", sXML_CDATA, "0cm");
    mxList->addAttribute("fo:padding", sXML_CDATA, "0cm");
    mxList->addAttribute("style:wrap", sXML_CDATA, "none");
    mxList->addAttribute("style:vertical-pos", sXML_CDATA, "from-top");
    mxList->addAttribute("style:vertical-rel", sXML_CDATA, "paragraph");
    mxList->addAttribute("style:horizontal-pos", sXML_CDATA, "from-left");
    mxList->addAttribute("style:horizontal-rel", sXML_CDATA, "paragraph");
    if (fstyle->boxtype == 'G' && fstyle->cell)
    {
        char *cell = static_cast<char *>(fstyle->cell);
        mxList->addAttribute("draw:luminance", sXML_CDATA, OUString::number(cell[0]) + "%");
        mxList->addAttribute("draw:contrast", sXML_CDATA, OUString::number(cell[1]) + "%");
        if( cell[2] == 0 )
            mxList->addAttribute("draw:color-mode", sXML_CDATA, "standard");
        else if( cell[2] == 1 )
            mxList->addAttribute("draw:color-mode", sXML_CDATA, "greyscale");
        else if( cell[2] == 2 )
            mxList->addAttribute("draw:color-mode", sXML_CDATA, "mono");
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
                    mxList->addAttribute("fo:padding", sXML_CDATA,"0mm");
                    break;
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute("fo:border", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute("fo:border", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute("style:border-line-width", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    mxList->addAttribute("fo:border", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
        }
        else
        {
            switch( cell->linetype[0] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute("fo:border-left", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute("fo:border-left", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute("style:border-line-width-left", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    mxList->addAttribute("fo:border-left", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
            switch( cell->linetype[1] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute("fo:border-right", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute("fo:border-right", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute("style:border-line-width-right", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    mxList->addAttribute("fo:border-right", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
            switch( cell->linetype[2] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute("fo:border-top", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute("fo:border-top", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute("style:border-line-width-top", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    mxList->addAttribute("fo:border-top", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
            switch( cell->linetype[3] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute("fo:border-bottom", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute("fo:border-bottom", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute("style:border-line-width-bottom", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    mxList->addAttribute("fo:border-bottom", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
        }
        if(cell->shade != 0)
            mxList->addAttribute("fo:background-color", sXML_CDATA, hcolor2str(
            sal::static_int_cast<uchar>(cell->color),
            sal::static_int_cast<uchar>(cell->shade)));
    }
    startEl("style:properties");
    mxList->clear();
    endEl("style:properties");
    endEl("style:style");
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
            mxList->addAttribute("style:name", sXML_CDATA, "Txtbox" + OUString::number(fstyle->boxnum));
            mxList->addAttribute("style:family", sXML_CDATA, "graphics");
            break;
        case 'G' :                                // graphics
            mxList->addAttribute("style:name", sXML_CDATA, "G" + OUString::number(fstyle->boxnum));
            mxList->addAttribute("style:family", sXML_CDATA, "graphics");
            break;
        case 'L' :                                // line TODO : all
            mxList->addAttribute("style:name", sXML_CDATA, "L" + OUString::number(fstyle->boxnum));
            mxList->addAttribute( "style:family" , sXML_CDATA , "paragraph" );
            break;
    }

    startEl("style:style");
    mxList->clear();

    if ( fstyle->boxtype == 'T')
    {
        mxList->addAttribute("fo:padding", sXML_CDATA, "0cm");
    }

    if( fstyle->boxtype != 'G' || fstyle->cap_len <= 0 )
    {
        mxList->addAttribute("fo:margin-left", sXML_CDATA,
            OUString::number(WTMM(fstyle->margin[0][0]) ) + "mm");
        mxList->addAttribute("fo:margin-right", sXML_CDATA,
            OUString::number(WTMM(fstyle->margin[0][1])) + "mm");
        mxList->addAttribute("fo:margin-top", sXML_CDATA,
            OUString::number(WTMM(fstyle->margin[0][2])) + "mm");
        mxList->addAttribute("fo:margin-bottom", sXML_CDATA,
            OUString::number(WTMM(fstyle->margin[0][3])) + "mm");
    }

    switch (fstyle->txtflow)
    {
        case 0:
            mxList->addAttribute("style:wrap", sXML_CDATA, "none");
            break;
        case 1:
            if( fstyle->boxtype == 'G' || fstyle->boxtype == 'B' || fstyle->boxtype == 'O')
                mxList->addAttribute("style:run-through", sXML_CDATA, "background");
            mxList->addAttribute("style:wrap", sXML_CDATA, "run-through");
            break;
        case 2:
            mxList->addAttribute("style:wrap", sXML_CDATA, "dynamic");
            break;
    }
    if (fstyle->anchor_type == CHAR_ANCHOR)
    {
        mxList->addAttribute("style:vertical-pos", sXML_CDATA, "top");
        mxList->addAttribute("style:vertical-rel", sXML_CDATA, "baseline");
        mxList->addAttribute("style:horizontal-pos", sXML_CDATA, "center");
        mxList->addAttribute("style:horizontal-rel", sXML_CDATA, "paragraph");
    }
    else
    {

        switch (-(fstyle->xpos))
        {
            case 2:
                mxList->addAttribute("style:horizontal-pos", sXML_CDATA, "right");
                break;
            case 3:
                mxList->addAttribute("style:horizontal-pos", sXML_CDATA, "center");
                break;
            case 1:
            default:
                mxList->addAttribute("style:horizontal-pos", sXML_CDATA, "from-left");
                break;
        }
        switch (-(fstyle->ypos))
        {
            case 2:
                mxList->addAttribute("style:vertical-pos", sXML_CDATA, "bottom");
                break;
            case 3:
                mxList->addAttribute("style:vertical-pos", sXML_CDATA, "middle");
                break;
            case 1:
            default:
                mxList->addAttribute("style:vertical-pos", sXML_CDATA, "from-top");
                break;
        }
        if ( fstyle->anchor_type == PARA_ANCHOR )
        {
            mxList->addAttribute("style:vertical-rel", sXML_CDATA, "paragraph");
            mxList->addAttribute("style:horizontal-rel", sXML_CDATA, "paragraph");
        }
        else
        {
            mxList->addAttribute("style:vertical-rel", sXML_CDATA, "page-content");
            mxList->addAttribute("style:horizontal-rel", sXML_CDATA, "page-content");
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
                          mxList->addAttribute("fo:border", sXML_CDATA, "none");
                    break;
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute("fo:border", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute("fo:border", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute("style:border-line-width", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    mxList->addAttribute("fo:border", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
        }
        else
        {
            switch( cell->linetype[0] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute("fo:border-left", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute("fo:border-left", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute("style:border-line-width-left", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    mxList->addAttribute("fo:border-left", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
            switch( cell->linetype[1] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute("fo:border-right", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute("fo:border-right", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute("style:border-line-width-right", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    mxList->addAttribute("fo:border-right", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
            switch( cell->linetype[2] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute("fo:border-top", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute("fo:border-top", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute("style:border-line-width-top", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    mxList->addAttribute("fo:border-top", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
            switch( cell->linetype[3] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    mxList->addAttribute("fo:border-bottom", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    mxList->addAttribute("fo:border-bottom", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    mxList->addAttribute("style:border-line-width-bottom", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    mxList->addAttribute("fo:border-bottom", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
        }

        if( cell->linetype[0] == 0 && cell->linetype[1] == 0 &&
                  cell->linetype[2] == 0 && cell->linetype[3] == 0 ){
              mxList->addAttribute("fo:padding", sXML_CDATA,"0mm");
        }
        else{
              mxList->addAttribute("fo:padding-left", sXML_CDATA,
                      OUString::number(WTMM(fstyle->margin[1][0])) + "mm");
              mxList->addAttribute("fo:padding-right", sXML_CDATA,
                      OUString::number(WTMM(fstyle->margin[1][1])) + "mm");
              mxList->addAttribute("fo:padding-top", sXML_CDATA,
                      OUString::number(WTMM(fstyle->margin[1][2])) + "mm");
              mxList->addAttribute("fo:padding-bottom", sXML_CDATA,
                      OUString::number(WTMM(fstyle->margin[1][3])) + "mm");
        }
        if(cell->shade != 0)
            mxList->addAttribute("fo:background-color", sXML_CDATA,
            hcolor2str(
                sal::static_int_cast<uchar>(cell->color),
                sal::static_int_cast<uchar>(cell->shade)));
    }
    else if( fstyle->boxtype == 'E' )
     {
          mxList->addAttribute("fo:padding", sXML_CDATA,"0mm");
     }
    else if( fstyle->boxtype == 'L' )
    {
        mxList->addAttribute( "style:border-line-width-bottom", sXML_CDATA, "0.02mm 0.35mm 0.02mm");
        mxList->addAttribute("fo:border-bottom", sXML_CDATA,"0.039cm double #808080");
    }
    else if( fstyle->boxtype == 'G' && fstyle->cell )
    {
        if( fstyle->margin[1][0] || fstyle->margin[1][1] || fstyle->margin[1][2] || fstyle->margin[1][3] ){
             OUString clip = "rect(" +
                OUString::number(WTMM(-fstyle->margin[1][0]) ) + "mm " +
                OUString::number(WTMM(-fstyle->margin[1][1]) ) + "mm " +
                OUString::number(WTMM(-fstyle->margin[1][2]) ) + "mm " +
                OUString::number(WTMM(-fstyle->margin[1][3]) ) + "mm)";
             mxList->addAttribute("style:mirror", sXML_CDATA, "none");
             mxList->addAttribute("fo:clip", sXML_CDATA, clip);
        }
        char *cell = static_cast<char *>(fstyle->cell);
        mxList->addAttribute("draw:luminance", sXML_CDATA, OUString::number(cell[0]) + "%");
        mxList->addAttribute("draw:contrast", sXML_CDATA, OUString::number(cell[1]) + "%");
        if( cell[2] == 0 )
            mxList->addAttribute("draw:color-mode", sXML_CDATA, "standard");
        else if( cell[2] == 1 )
            mxList->addAttribute("draw:color-mode", sXML_CDATA, "greyscale");
        else if( cell[2] == 2 )
            mxList->addAttribute("draw:color-mode", sXML_CDATA, "mono");

    }
    startEl("style:properties");
    mxList->clear();
    endEl("style:properties");
    endEl("style:style");
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
        mxList->addAttribute("text:style-name", sXML_CDATA,
            getPStyleName(para->GetParaShape().index));
        startEl("text:p");
        mxList->clear();
    }
    if( d->bFirstPara && d->bInBody )
    {
        mxList->addAttribute("text:name", sXML_CDATA, sBeginOfDoc);
        startEl("text:bookmark");
        mxList->clear();
        endEl("text:bookmark");
        d->bFirstPara = false;
    }
    if( d->bInHeader )
    {
        makeShowPageNum();
        d->bInHeader = false;
    }
    mxList->addAttribute("text:style-name", sXML_CDATA,
        getTStyleName(para->cshape->index));
    startEl("text:span");
    mxList->clear();

    for (const auto& box : para->hhstr)
    {
        if (!box->hh)
            break;

        if (box->hh == CH_SPACE && !firstspace)
        {
            makeChars(str);
            startEl("text:s");
            endEl("text:s");
        }
        else if (box->hh == CH_END_PARA)
        {
            makeChars(str);
            endEl("text:span");
            endEl("text:p");
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
        mxList->addAttribute("text:style-name", sXML_CDATA,
            getPStyleName(para->GetParaShape().index));
        startEl("text:p");
        mxList->clear();
    }
    if( d->bFirstPara && d->bInBody )
    {
/* for HWP's Bookmark */
        mxList->addAttribute("text:name", sXML_CDATA, sBeginOfDoc);
        startEl("text:bookmark");
        mxList->clear();
        endEl("text:bookmark");
        d->bFirstPara = false;
    }
    if( d->bInHeader )
    {
        makeShowPageNum();
        d->bInHeader = false;
    }
    mxList->addAttribute("text:style-name", sXML_CDATA,
        getTStyleName(curr));
    startEl("text:span");
    mxList->clear();

    int n = 0;
    for (const auto& box : para->hhstr)
    {
        if (!box->hh)
            break;

        if (para->GetCharShape(n)->index != curr)
        {
            makeChars(str);
            endEl("text:span");
            curr = para->GetCharShape(n)->index;
            mxList->addAttribute("text:style-name", sXML_CDATA,
                getTStyleName(curr));
            startEl("text:span");
            mxList->clear();
        }
        if (box->hh == CH_SPACE && !firstspace)
        {
            makeChars(str);
            startEl("text:s");
            endEl("text:s");
        }
        else if (box->hh == CH_END_PARA)
        {
            makeChars(str);
            endEl("text:span");
            endEl("text:p");
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
        mxList->addAttribute("text:style-name", "CDATA",
                             getPStyleName(para->GetParaShape().index));
        startEl("text:p");
        mxList->clear();
        pstart = true;
    };
    const auto STARTT = [this, para, &tstart](int pos)
    {
        auto curr = para->GetCharShape(pos > 0 ? pos - 1 : 0)->index;
        mxList->addAttribute("text:style-name", "CDATA", getTStyleName(curr));
        startEl("text:span");
        mxList->clear();
        tstart = true;
    };
    const auto ENDP = [this, &pstart]()
    {
        endEl("text:p");
        pstart = false;
    };
    const auto ENDT = [this, &tstart]()
    {
        endEl("text:span");
        tstart = false;
    };

    if( d->bFirstPara && d->bInBody )
    {
        if ( !pstart ) {
            STARTP();
        }
        mxList->addAttribute("text:name", sXML_CDATA, sBeginOfDoc);
        startEl("text:bookmark");
        mxList->clear();
        endEl("text:bookmark");
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
    while (n < para->nch)
    {
        const auto& box = para->hhstr[n];

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
            startEl("text:s");
            mxList->clear();
            endEl("text:s");
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
        mxList->addAttribute("text:placeholder-type", sXML_CDATA, "text");
        if (d->pField)
            mxList->addAttribute("text:description", sXML_CDATA, hstr2OUString(d->pField));
        startEl("text:placeholder");
        mxList->clear();
        chars( fromHcharStringToOUString(rStr) );
        endEl("text:placeholder");
    }
/* Document Summary */
    else if( hbox->type[0] == 3 && hbox->type[1] == 0 )
    {
        const OUString uStr3 = hstr2OUString(hbox->str3.get());
        if (uStr3 == "title")
        {
            startEl("text:title");
            chars( hstr2OUString(hbox->str2.get()) );
            endEl("text:title");
        }
        else if (uStr3 == "subject")
        {
            startEl("text:subject");
            chars( hstr2OUString(hbox->str2.get()) );
            endEl("text:subject");
        }
        else if (uStr3 == "author")
        {
            startEl("text:author-name");
            chars( hstr2OUString(hbox->str2.get()) );
            endEl("text:author-name");
        }
        else if (uStr3 == "keywords")
        {
            startEl("text:keywords");
            chars( hstr2OUString(hbox->str2.get()) );
            endEl("text:keywords");
        }
    }
/* Personal Information */
    else if( hbox->type[0] == 3 && hbox->type[1] == 1 )
    {
        const OUString uStr3 = hstr2OUString(hbox->str3.get());
        if (uStr3 == "User")
        {
            startEl("text:sender-lastname");
            chars( hstr2OUString(hbox->str2.get()) );
            endEl("text:sender-lastname");
        }
        else if (uStr3 == "Company")
        {
            startEl("text:sender-company");
            chars( hstr2OUString(hbox->str2.get()) );
            endEl("text:sender-company");
        }
        else if (uStr3 == "Position")
        {
            startEl("text:sender-title");
            chars( hstr2OUString(hbox->str2.get()) );
            endEl("text:sender-title");
        }
        else if (uStr3 == "Division")
        {
            startEl("text:sender-position");
            chars( hstr2OUString(hbox->str2.get()) );
            endEl("text:sender-position");
        }
        else if (uStr3 == "Fax")
        {
            startEl("text:sender-fax");
            chars( hstr2OUString(hbox->str2.get()) );
            endEl("text:sender-fax");
        }
        else if (uStr3 == "Pager")
        {
            startEl("text:phone-private");
            chars( hstr2OUString(hbox->str2.get()) );
            endEl("text:phone-private");
        }
        else if (uStr3 == "E-mail")
        {
            startEl("text:sender-email");
            chars( hstr2OUString(hbox->str2.get()) );
            endEl("text:sender-email");
        }
        else if (uStr3 == "Zipcode(office)")
        {
            startEl("text:sender-postal-code");
            chars( hstr2OUString(hbox->str2.get()) );
            endEl("text:sender-postal-code");
        }
        else if (uStr3 == "Phone(office)")
        {
            startEl("text:sender-phone-work");
            chars( hstr2OUString(hbox->str2.get()) );
            endEl("text:sender-phone-work");
        }
        else if (uStr3 == "Address(office)")
        {
            startEl("text:sender-street");
            chars( hstr2OUString(hbox->str2.get()) );
            endEl("text:sender-street");
        }

    }
    else if( hbox->type[0] == 3 && hbox->type[1] == 2 ) /* creation date */
     {
         if( hbox->m_pDate )
             mxList->addAttribute("style:data-style-name", sXML_CDATA, "N" + OUString::number(hbox->m_pDate->key));
         startEl("text:creation-date");
         mxList->clear();
         chars( hstr2OUString(hbox->str2.get()) );
         endEl("text:creation-date");
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
        mxList->addAttribute("text:name", sXML_CDATA, hstr2OUString(hbox->id));
        startEl("text:bookmark");
        mxList->clear();
        endEl("text:bookmark");
    }
    else if (hbox->type == 1)                     /* Block bookmarks days begin and end there if */
    {
        mxList->addAttribute("text:name", sXML_CDATA, hstr2OUString(hbox->id));
        startEl("text:bookmark-start");
        mxList->clear();
        endEl("text:bookmark-start");
    }
    else if (hbox->type == 2)
    {
        mxList->addAttribute("text:name", sXML_CDATA, hstr2OUString(hbox->id));
        startEl("text:bookmark-end");
        mxList->clear();
        endEl("text:bookmark-end");
    }
}


void HwpReader::makeDateFormat(DateCode * hbox)
{
    mxList->addAttribute("style:name", sXML_CDATA, "N" + OUString::number(hbox->key));
    mxList->addAttribute("style:family", sXML_CDATA,"data-style");
    mxList->addAttribute("number:language", sXML_CDATA,"ko");
    mxList->addAttribute("number:country", sXML_CDATA,"KR");

    startEl("number:date-style");
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
                mxList->addAttribute("number:style", sXML_CDATA, "long");
                startEl("number:year");
                mxList->clear();
                endEl("number:year");
                break;
            case '!':
                startEl("number:year");
                mxList->clear();
                endEl("number:year");
                break;
            case '2':
                if( add_zero )
                    mxList->addAttribute("number:style", sXML_CDATA, "long");
                startEl("number:month");
                mxList->clear();
                endEl("number:month");
                break;
            case '@':
                mxList->addAttribute("number:textual", sXML_CDATA, "true");
                startEl("number:month");
                mxList->clear();
                endEl("number:month");
                break;
            case '*':
                mxList->addAttribute("number:textual", sXML_CDATA, "true");
                mxList->addAttribute("number:style", sXML_CDATA, "long");
                startEl("number:month");
                mxList->clear();
                endEl("number:month");
                break;
            case '3':
                if( add_zero )
                    mxList->addAttribute("number:style", sXML_CDATA, "long");
                startEl("number:day");
                mxList->clear();
                endEl("number:day");
                break;
            case '#':
                if( add_zero )
                    mxList->addAttribute("number:style", sXML_CDATA, "long");
                startEl("number:day");
                mxList->clear();
                endEl("number:day");
                switch( hbox->date[DateCode::DAY]  % 10)
                {
                    case 1:
                        startEl("number:text");
                        chars("st");
                        endEl("number:text");
                        break;
                    case 2:
                        startEl("number:text");
                        chars("nd");
                        endEl("number:text");
                        break;
                    case 3:
                        startEl("number:text");
                        chars("rd");
                        endEl("number:text");
                        break;
                    default:
                        startEl("number:text");
                        chars("th");
                        endEl("number:text");
                        break;
                }
                break;
            case '4':
            case '$':
                if( add_zero )
                    mxList->addAttribute("number:style", sXML_CDATA, "long");
                startEl("number:hours");
                mxList->clear();
                endEl("number:hours");
                break;
            case '5':
            case '%':
                if( add_zero )
                    mxList->addAttribute("number:style", sXML_CDATA, "long");
                startEl("number:minutes");
                mxList->clear();
                endEl("number:minutes");
                break;
            case '_':
                mxList->addAttribute("number:style", sXML_CDATA, "long");
                [[fallthrough]];
            case '6':
            case '^':
                startEl("number:day-of-week");
                mxList->clear();
                endEl("number:day-of-week");
                break;
            case '7':
            case '&':
            case '+':
                startEl("number:am-pm");
                mxList->clear();
                endEl("number:am-pm");
                break;
            case '~':                             // Chinese Locale
                break;
            default:
                hchar sbuf[2];
                sbuf[0] = *fmt;
                sbuf[1] = 0;
                startEl("number:text");
                chars(hstr2OUString(sbuf));
                endEl("number:text");
                break;
        }
    }
    mxList->clear();
    endEl("number:date-style");
}


void HwpReader::makeDateCode(DateCode * hbox)
{
    mxList->addAttribute("style:data-style-name", sXML_CDATA, "N" + OUString::number(hbox->key));
    startEl("text:date");
    mxList->clear();
    hchar_string const boxstr = hbox->GetString();
    chars(hstr2OUString(boxstr.c_str()));
    endEl("text:date");
}


void HwpReader::makeTab()
{
    startEl("text:tab-stop");
    endEl("text:tab-stop");
}


void HwpReader::makeTable(TxtBox * hbox)
{
    mxList->addAttribute("table:name", sXML_CDATA, "Table" + OUString::number(hbox->style.boxnum));
    mxList->addAttribute("table:style-name", sXML_CDATA, "Table" + OUString::number(hbox->style.boxnum));
    startEl("table:table");
    mxList->clear();

    Table *tbl = hbox->m_pTable;
// column
    for (size_t i = 0 ; i < tbl->columns.nCount -1 ; i++)
    {
        sprintf(d->buf,"Table%d.%c",hbox->style.boxnum, static_cast<char>('A'+i));
        mxList->addAttribute("table:style-name", sXML_CDATA, OUString::createFromAscii(d->buf));
        startEl("table:table-column");
        mxList->clear();
        endEl("table:table-column");
    }

// cell
    int j = -1, k = -1;
    for (auto const& tcell : tbl->cells)
    {
        if( tcell->nRowIndex > j )
        {
            if( j > k )
            {
                endEl("table:table-row");
                k = j;
            }
// row
            sprintf(d->buf,"Table%d.row%d",hbox->style.boxnum, tcell->nRowIndex + 1);
            mxList->addAttribute("table:style-name", sXML_CDATA, OUString::createFromAscii(d->buf));
            startEl("table:table-row");
            mxList->clear();
            j = tcell->nRowIndex;
        }

        sprintf(d->buf,"Table%d.%c%d",hbox->style.boxnum, 'A'+ tcell->nColumnIndex, tcell->nRowIndex +1);
        mxList->addAttribute("table:style-name", sXML_CDATA, OUString::createFromAscii(d->buf));
        if( tcell->nColumnSpan > 1 )
            mxList->addAttribute("table:number-columns-spanned", sXML_CDATA, OUString::number(tcell->nColumnSpan));
        if( tcell->nRowSpan > 1 )
            mxList->addAttribute("table:number-rows-spanned", sXML_CDATA, OUString::number(tcell->nRowSpan));
        mxList->addAttribute("table:value-type", sXML_CDATA,"string");
        if( tcell->pCell->protect )
            mxList->addAttribute("table:protected", sXML_CDATA,"true");
        startEl("table:table-cell");
        mxList->clear();
        TxtBox::plist_t& rVec = hbox->plists[tcell->pCell->key];
        if (!rVec.empty())
            parsePara(rVec.front().get());
        endEl("table:table-cell");
    }
    endEl("table:table-row");
    endEl("table:table");
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
        mxList->addAttribute("draw:style-name", sXML_CDATA, "CapBox" + OUString::number(hbox->style.boxnum));
        mxList->addAttribute("draw:name", sXML_CDATA, "CaptionBox" + OUString::number(hbox->style.boxnum));
        mxList->addAttribute("draw:z-index", sXML_CDATA, OUString::number(hbox->zorder));
        switch (hbox->style.anchor_type)
        {
            case CHAR_ANCHOR:
                mxList->addAttribute("text:anchor-type", sXML_CDATA, "as-char");
                break;
            case PARA_ANCHOR:
                mxList->addAttribute("text:anchor-type", sXML_CDATA, "paragraph");
                break;
            case PAGE_ANCHOR:
            case PAPER_ANCHOR:
            {
                mxList->addAttribute("text:anchor-type", sXML_CDATA, "page");
                mxList->addAttribute("text:anchor-page-number", sXML_CDATA, OUString::number(hbox->pgno + 1));
                break;
            }
        }
        if (hbox->style.anchor_type != CHAR_ANCHOR)
        {
            mxList->addAttribute("svg:x", sXML_CDATA,
                OUString::number(WTMM( hbox->pgx + hbox->style.margin[0][0] )) + "mm");
            mxList->addAttribute("svg:y", sXML_CDATA,
                OUString::number(WTMM( hbox->pgy + hbox->style.margin[0][2] )) + "mm");
        }
        mxList->addAttribute("svg:width", sXML_CDATA,
            OUString::number(WTMM( hbox->box_xs + hbox->cap_xs )) + "mm");
        mxList->addAttribute("fo:min-height", sXML_CDATA,
            OUString::number(WTMM( hbox->box_ys + hbox->cap_ys )) + "mm");
        startEl("draw:text-box");
        mxList->clear();
        if (!hbox->caption.empty() && hbox->cap_pos % 2)  /* The caption is on the top */
        {
            parsePara(hbox->caption.front().get());
        }
        mxList->addAttribute( "text:style-name", sXML_CDATA, "Standard");
        startEl("text:p");
        mxList->clear();
    }
    else{
         mxList->addAttribute("draw:z-index", sXML_CDATA, OUString::number(hbox->zorder));
    }

    mxList->addAttribute("draw:style-name", sXML_CDATA, "Txtbox" + OUString::number(hbox->style.boxnum));
    mxList->addAttribute("draw:name", sXML_CDATA, "Frame" + OUString::number(hbox->style.boxnum));

    if( hbox->style.cap_len <= 0 || hbox->type != TXT_TYPE )
    {
        int x = 0;
        int y = 0;
        switch (hbox->style.anchor_type)
        {
            case CHAR_ANCHOR:
                mxList->addAttribute("text:anchor-type", sXML_CDATA, "as-char");
                break;
            case PARA_ANCHOR:
                mxList->addAttribute("text:anchor-type", sXML_CDATA, "paragraph");
                break;
            case PAGE_ANCHOR:
            case PAPER_ANCHOR:
            {
                mxList->addAttribute("text:anchor-type", sXML_CDATA, "page");
                mxList->addAttribute("text:anchor-page-number", sXML_CDATA, OUString::number(hbox->pgno + 1));
                break;
            }
        }
        if( hbox->style.anchor_type != CHAR_ANCHOR )
        {
            x += hbox->style.margin[0][0];
            y += hbox->style.margin[0][2];
        }
        mxList->addAttribute("svg:x", sXML_CDATA,
            OUString::number(WTMM( hbox->pgx + x )) + "mm");
        mxList->addAttribute("svg:y", sXML_CDATA,
            OUString::number(WTMM( hbox->pgy + y )) + "mm");
    }
    else
    {
        mxList->addAttribute("text:anchor-type", sXML_CDATA, "as-char");
        mxList->addAttribute("svg:y", sXML_CDATA, "0cm");
    }
    mxList->addAttribute("svg:width", sXML_CDATA,
        OUString::number(WTMM( hbox->box_xs )) + "mm");
    if( hbox->style.cap_len > 0 && hbox->type != TXT_TYPE)
        mxList->addAttribute("fo:min-height", sXML_CDATA,
            OUString::number(WTMM( hbox->box_ys + hbox->cap_ys)) + "mm");
    else
        mxList->addAttribute("svg:height", sXML_CDATA,
            OUString::number(WTMM(hbox->box_ys )) + "mm");

    if( hbox->type != EQU_TYPE )
    {
        startEl("draw:text-box");
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
        endEl("draw:text-box");
// Caption exist and it is text-box
        if( hbox->style.cap_len > 0 && hbox->type == TXT_TYPE)
        {
            endEl("text:p");
            if (!(hbox->cap_pos % 2) && !hbox->caption.empty())
            {
                parsePara(hbox->caption.front().get());
            }
            endEl("draw:text-box");
        }
    }
    else                                          // is Formula
    {
        startEl("draw:object");
        mxList->clear();
        makeFormula(hbox);
        endEl("draw:object");
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
          mxList->addAttribute("xlink:type", sXML_CDATA, "simple");
          if (!tmp.empty() && strcmp(tmp.c_str(), "[HTML]")) {
              ::std::string tmp3(tmp2);
              tmp3.push_back('#');
              tmp3.append(tmp);
              mxList->addAttribute("xlink:href", sXML_CDATA,
                  OUString(tmp3.c_str(), tmp3.size()+1, RTL_TEXTENCODING_EUC_KR));
          }
          else{
              mxList->addAttribute("xlink:href", sXML_CDATA,
                  OUString(tmp2.c_str(), tmp2.size()+1, RTL_TEXTENCODING_EUC_KR));

          }
    }
    else
    {
        mxList->addAttribute("xlink:type", sXML_CDATA, "simple");
        ::std::string tmp;
        tmp.push_back('#');
        tmp.append(hstr2ksstr(hypert->bookmark));
        mxList->addAttribute("xlink:href", sXML_CDATA,
                OUString(tmp.c_str(), tmp.size()+1, RTL_TEXTENCODING_EUC_KR));
    }
    startEl("draw:a");
    mxList->clear();
    makeTextBox(hbox);
    endEl("draw:a");
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
                mxList->addAttribute("draw:style-name", sXML_CDATA,
                    "CapBox" + OUString::number(hbox->style.boxnum));
                mxList->addAttribute("draw:name", sXML_CDATA, "CaptionBox" + OUString::number(hbox->style.boxnum));
                mxList->addAttribute("draw:z-index", sXML_CDATA, OUString::number(hbox->zorder));
                switch (hbox->style.anchor_type)
                {
                    case CHAR_ANCHOR:
                        mxList->addAttribute("text:anchor-type", sXML_CDATA, "as-char");
                        break;
                    case PARA_ANCHOR:
                        mxList->addAttribute("text:anchor-type", sXML_CDATA, "paragraph");
                        break;
                    case PAGE_ANCHOR:
                    case PAPER_ANCHOR:
                    {
                        mxList->addAttribute("text:anchor-type", sXML_CDATA, "page");
                        mxList->addAttribute("text:anchor-page-number", sXML_CDATA,
                            OUString::number(hbox->pgno + 1));
                        break;
                    }
                }
                if (hbox->style.anchor_type != CHAR_ANCHOR)
                {
                    mxList->addAttribute("svg:x", sXML_CDATA,
                        OUString::number(WTMM(  hbox->pgx + hbox->style.margin[0][0] )) + "mm");
                    mxList->addAttribute("svg:y", sXML_CDATA,
                        OUString::number(WTMM(  hbox->pgy + hbox->style.margin[0][2] )) + "mm");
                }
                mxList->addAttribute("svg:width", sXML_CDATA,
                    OUString::number(WTMM( hbox->box_xs + hbox->style.margin[1][0] + hbox->style.margin[1][1] )) + "mm");
                mxList->addAttribute("fo:min-height", sXML_CDATA,
                    OUString::number(WTMM( hbox->box_ys + hbox->style.margin[1][2] + hbox->style.margin[1][3] + hbox->cap_ys )) + "mm");
                startEl("draw:text-box");
                mxList->clear();
                if (!hbox->caption.empty() && hbox->cap_pos % 2)           /* Caption is on the top */
                {
                    parsePara(hbox->caption.front().get());
                }
                mxList->addAttribute( "text:style-name", sXML_CDATA, "Standard");
                startEl("text:p");
                mxList->clear();
            }
            if( hbox->ishyper )
            {
                mxList->addAttribute("xlink:type", sXML_CDATA, "simple");
#ifdef _WIN32
                if( hbox->follow[4] != 0 )
                    mxList->addAttribute("xlink:href", sXML_CDATA, hstr2OUString(kstr2hstr(hbox->follow.data() + 4).c_str()));
                else
                    mxList->addAttribute("xlink:href", sXML_CDATA, hstr2OUString(kstr2hstr(hbox->follow.data() + 5).c_str()));
#else
                if( hbox->follow[4] != 0 )
                    mxList->addAttribute("xlink:href", sXML_CDATA,
                        hstr2OUString(kstr2hstr(reinterpret_cast<uchar const *>(urltounix(reinterpret_cast<char *>(hbox->follow.data() + 4)).c_str())).c_str()));
                else
                    mxList->addAttribute("xlink:href", sXML_CDATA,
                        hstr2OUString(kstr2hstr(reinterpret_cast<uchar const *>(urltounix(reinterpret_cast<char *>(hbox->follow.data() + 5)).c_str())).c_str()));
#endif
                startEl("draw:a");
                mxList->clear();
            }
            mxList->addAttribute("draw:style-name", sXML_CDATA, "G" + OUString::number(hbox->style.boxnum));
            mxList->addAttribute("draw:name", sXML_CDATA, "Image" + OUString::number(hbox->style.boxnum));

            if( hbox->style.cap_len <= 0 )
            {
                mxList->addAttribute("draw:z-index", sXML_CDATA, OUString::number(hbox->zorder));
                switch (hbox->style.anchor_type)
                {
                    case CHAR_ANCHOR:
                        mxList->addAttribute("text:anchor-type", sXML_CDATA, "as-char");
                        break;
                    case PARA_ANCHOR:
                        mxList->addAttribute("text:anchor-type", sXML_CDATA, "paragraph");
                        break;
                    case PAGE_ANCHOR:
                    case PAPER_ANCHOR:
                    {
                        mxList->addAttribute("text:anchor-type", sXML_CDATA, "page");
                        mxList->addAttribute("text:anchor-page-number", sXML_CDATA,
                            OUString::number(hbox->pgno + 1));
                        break;
                    }
                }
                if (hbox->style.anchor_type != CHAR_ANCHOR)
                {
                    mxList->addAttribute("svg:x", sXML_CDATA,
                        OUString::number(WTMM( hbox->pgx + hbox->style.margin[0][0] )) + "mm");
                    mxList->addAttribute("svg:y", sXML_CDATA,
                        OUString::number(WTMM( hbox->pgy + hbox->style.margin[0][2] )) + "mm");
                }
            }
            else
            {
                mxList->addAttribute("text:anchor-type", sXML_CDATA, "as-char");
                mxList->addAttribute("svg:y", sXML_CDATA, "0cm");
            }
            mxList->addAttribute("svg:width", sXML_CDATA,
                OUString::number(WTMM( hbox->box_xs + hbox->style.margin[1][0] + hbox->style.margin[1][1])) + "mm");
            mxList->addAttribute("svg:height", sXML_CDATA,
                OUString::number(WTMM( hbox->box_ys + hbox->style.margin[1][2] + hbox->style.margin[1][3])) + "mm");

            if ( hbox->pictype == PICTYPE_FILE ){
#ifdef _WIN32
                sprintf(d->buf, "file:///%s", hbox->picinfo.picun.path );
                mxList->addAttribute("xlink:href", sXML_CDATA, hstr2OUString(kstr2hstr(reinterpret_cast<uchar *>(d->buf)).c_str()));
#else
                mxList->addAttribute("xlink:href", sXML_CDATA,
                    hstr2OUString(kstr2hstr(reinterpret_cast<uchar const *>(urltounix(hbox->picinfo.picun.path).c_str())).c_str()));
#endif
                mxList->addAttribute("xlink:type", sXML_CDATA, "simple");
                mxList->addAttribute("xlink:show", sXML_CDATA, "embed");
                mxList->addAttribute("xlink:actuate", sXML_CDATA, "onLoad");
            }

            if( hbox->pictype == PICTYPE_OLE )
                    startEl("draw:object-ole");
            else
                    startEl("draw:image");
            mxList->clear();
            if (hbox->pictype == PICTYPE_EMBED || hbox->pictype == PICTYPE_OLE)
            {
                startEl("office:binary-data");
                mxList->clear();
                if( hbox->pictype == PICTYPE_EMBED ){
                         EmPicture *emp = hwpfile.GetEmPicture(hbox);
                         if( emp )
                         {
                             chars(base64_encode_string(emp->data.get(), emp->size));
                         }
                }
                else{
                         if( hwpfile.oledata ){
#ifdef _WIN32
                             LPSTORAGE srcsto;
                             LPUNKNOWN pObj;
                             wchar_t pathname[200];

                             MultiByteToWideChar(CP_ACP, 0, hbox->picinfo.picole.embname, -1, pathname, 200);
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
                             chars("");
#endif
                         }
                }
                endEl("office:binary-data");
            }
            if( hbox->pictype == PICTYPE_OLE )
                    endEl("draw:object-ole");
            else
                    endEl("draw:image");
            if( hbox->ishyper )
            {
                endEl("draw:a");
            }
            if( hbox->style.cap_len > 0 )
            {
                endEl("text:p");
                if (!hbox->caption.empty() && !(hbox->cap_pos % 2))         /* Caption is at the bottom, */
                {
                    parsePara(hbox->caption.front().get());
                }
                endEl("draw:text-box");
            }
            break;
        }
        case PICTYPE_DRAW:
              if( hbox->picinfo.picdraw.zorder > 0 )
                 mxList->addAttribute("draw:z-index", sXML_CDATA,
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
        mxList->addAttribute("draw:style-name", sXML_CDATA, "Draw" + OUString::number(drawobj->index));
        int a = 0;
        int b = 0;

        switch (hbox->style.anchor_type)
        {
            case CHAR_ANCHOR:
                mxList->addAttribute("text:anchor-type", sXML_CDATA, "as-char");
                break;
            case PARA_ANCHOR:
                mxList->addAttribute("text:anchor-type", sXML_CDATA, "paragraph");
                break;
            case PAGE_ANCHOR:
            case PAPER_ANCHOR:
            {
                HWPInfo& hwpinfo = hwpfile.GetHWPInfo();
                mxList->addAttribute("text:anchor-type", sXML_CDATA, "page");
                mxList->addAttribute("text:anchor-page-number", sXML_CDATA, OUString::number(hbox->pgno + 1));
                a = hwpinfo.paper.left_margin;
                b = hwpinfo.paper.top_margin + hwpinfo.paper.header_length;
                break;
            }
        }

        if (drawobj->type == HWPDO_CONTAINER)
        {
            startEl("draw:g");
            mxList->clear();
            makePictureDRAW(drawobj->child.get(), hbox);
            endEl("draw:g");
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
                    mxList->addAttribute("draw:transform", sXML_CDATA, trans);
                }
            }
            switch (drawobj->type)
            {
                case HWPDO_LINE:                  /* Line-starting coordinates, ending coordinates. */
                    if( drawobj->u.line_arc.flip & 0x01 )
                    {
                        mxList->addAttribute("svg:x1", sXML_CDATA,
                            OUString::number (WTMM(x + a + drawobj->offset2.x + drawobj->extent.w)) + "mm");
                        mxList->addAttribute("svg:x2", sXML_CDATA,
                            OUString::number (WTMM( x + a + drawobj->offset2.x )) + "mm");
                    }
                    else
                    {
                        mxList->addAttribute("svg:x1", sXML_CDATA,
                            OUString::number (WTMM( x + a + drawobj->offset2.x )) + "mm");
                        mxList->addAttribute("svg:x2", sXML_CDATA,
                            OUString::number (WTMM(x + a + drawobj->offset2.x + drawobj->extent.w)) + "mm");
                    }
                    if( drawobj->u.line_arc.flip & 0x02 )
                    {
                        mxList->addAttribute("svg:y1", sXML_CDATA,
                            OUString::number (WTMM( y + b + drawobj->offset2.y + drawobj->extent.h ) ) + "mm");
                        mxList->addAttribute("svg:y2", sXML_CDATA,
                            OUString::number (WTMM( y + b  + drawobj->offset2.y )) + "mm");
                    }
                    else
                    {
                        mxList->addAttribute("svg:y1", sXML_CDATA,
                            OUString::number (WTMM( y + b  + drawobj->offset2.y)) + "mm");
                        mxList->addAttribute("svg:y2", sXML_CDATA,
                            OUString::number (WTMM(y + b + drawobj->offset2.y + drawobj->extent.h)) + "mm");
                    }

                    startEl("draw:line");
                    mxList->clear();
                    endEl("draw:line");
                    break;
                case HWPDO_RECT:                  /* rectangle - the starting position, vertical/horizontal  */
                    if( !bIsRotate )
                    {
                        mxList->addAttribute("svg:x", sXML_CDATA,
                            OUString::number (WTMM( x + a + drawobj->offset2.x)) + "mm");
                        mxList->addAttribute("svg:y", sXML_CDATA,
                            OUString::number (WTMM( y + b + drawobj->offset2.y)) + "mm");
                    }
                    mxList->addAttribute("svg:width", sXML_CDATA,
                        OUString::number (WTMM( drawobj->extent.w )) + "mm");
                    mxList->addAttribute("svg:height", sXML_CDATA,
                        OUString::number (WTMM( drawobj->extent.h )) + "mm");
                    if( drawobj->property.flag & 0x01 )
                    {
                        int value = drawobj->extent.w < drawobj->extent.h ?
                            drawobj->extent.w : drawobj->extent.h ;
                        mxList->addAttribute("draw:corner-radius", sXML_CDATA,
                            OUString::number (WTMM( value/10 )) + "mm");
                    }
                    else if( drawobj->property.flag & 0x04 )
                    {
                        int value = drawobj->extent.w < drawobj->extent.h ?
                            drawobj->extent.w : drawobj->extent.h ;
                        mxList->addAttribute("draw:corner-radius", sXML_CDATA,
                            OUString::number (WTMM( value / 2)) + "mm");
                    }

                    startEl("draw:rect");
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
                    endEl("draw:rect");
                    break;
                case HWPDO_ELLIPSE:               /* Ellipse - the starting position, vertical/horizontal */
                case HWPDO_ADVANCED_ELLIPSE:      /* modified ellipse */
                {
                    if( !bIsRotate )
                    {
                        mxList->addAttribute("svg:x", sXML_CDATA,
                            OUString::number (WTMM( x + a + drawobj->offset2.x)) + "mm");
                        mxList->addAttribute("svg:y", sXML_CDATA,
                            OUString::number (WTMM( y + b + drawobj->offset2.y)) + "mm");
                    }

                    mxList->addAttribute("svg:width", sXML_CDATA,
                                 OUString::number (WTMM( drawobj->extent.w )) + "mm");
                    mxList->addAttribute("svg:height", sXML_CDATA,
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
                                    mxList->addAttribute("draw:kind", sXML_CDATA, "section");
                                else
                                    mxList->addAttribute("draw:kind", sXML_CDATA, "arc");
                                mxList->addAttribute("draw:start-angle", sXML_CDATA, OUString::number(start_angle ));
                                mxList->addAttribute("draw:end-angle", sXML_CDATA, OUString::number(end_angle));
                            }
                        }
                    }
                    startEl("draw:ellipse");
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
                    endEl("draw:ellipse");
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
                                    mxList->addAttribute("svg:x", sXML_CDATA,
                                         OUString::number (WTMM( x + a + drawobj->offset2.x - drawobj->extent.w)) + "mm");
                              else
                                    mxList->addAttribute("svg:x", sXML_CDATA,
                                         OUString::number (WTMM( x + a + drawobj->offset2.x)) + "mm");
                              if( ( flip == 0 || flip == 1 ) && drawobj->type == HWPDO_ARC)
                                    mxList->addAttribute("svg:y", sXML_CDATA,
                                         OUString::number (WTMM( y + b + drawobj->offset2.y - drawobj->extent.h)) + "mm");
                              else
                                    mxList->addAttribute("svg:y", sXML_CDATA,
                                         OUString::number (WTMM( y + b + drawobj->offset2.y)) + "mm");
                    }

                    mxList->addAttribute("svg:width", sXML_CDATA,
                                 OUString::number (WTMM( drawobj->extent.w * 2)) + "mm");
                    mxList->addAttribute("svg:height", sXML_CDATA,
                                 OUString::number (WTMM( drawobj->extent.h * 2)) + "mm");
                    if( drawobj->property.flag & HWPDO_FLAG_DRAW_PIE ||
                                     drawobj->property.fill_color < 0xffffff )
                                mxList->addAttribute("draw:kind", sXML_CDATA, "section");
                    else
                                mxList->addAttribute("draw:kind", sXML_CDATA, "arc");

                    if( drawobj->type == HWPDO_ADVANCED_ARC ){
                                double start_angle, end_angle;
                                ZZParall& pal = drawobj->property.parall;

                                start_angle = atan2(pal.pt[0].y - pal.pt[1].y,pal.pt[1].x - pal.pt[0].x );
                                end_angle = atan2(pal.pt[2].y - pal.pt[1].y, pal.pt[1].x - pal.pt[2].x);

                                if( ( start_angle > end_angle ) && (start_angle - end_angle < M_PI )){
                                    double tmp_angle = start_angle;
                                    start_angle = end_angle;
                                    end_angle = tmp_angle;
                                }
                                mxList->addAttribute("draw:start-angle", sXML_CDATA, OUString::number(basegfx::rad2deg(start_angle)));
                                mxList->addAttribute("draw:end-angle", sXML_CDATA, OUString::number(basegfx::rad2deg(end_angle)));

                    }
                    else
                    {
                            if( drawobj->u.line_arc.flip == 0 )
                            {
                                 mxList->addAttribute("draw:start-angle", sXML_CDATA, "270");
                                 mxList->addAttribute("draw:end-angle", sXML_CDATA, "0");
                            }
                            else if( drawobj->u.line_arc.flip == 1 )
                            {
                                 mxList->addAttribute("draw:start-angle", sXML_CDATA, "180");
                                 mxList->addAttribute("draw:end-angle", sXML_CDATA, "270");
                            }
                            else if( drawobj->u.line_arc.flip == 2 )
                            {
                                 mxList->addAttribute("draw:start-angle", sXML_CDATA, "0");
                                 mxList->addAttribute("draw:end-angle", sXML_CDATA, "90");
                            }
                            else
                            {
                                 mxList->addAttribute("draw:start-angle", sXML_CDATA, "90");
                                 mxList->addAttribute("draw:end-angle", sXML_CDATA, "180");
                            }
                            }
                    startEl("draw:ellipse");
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
                    endEl("draw:ellipse");
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
                        mxList->addAttribute("svg:x", sXML_CDATA,
                            OUString::number (WTMM( x + a + drawobj->offset2.x)) + "mm");
                        mxList->addAttribute("svg:y", sXML_CDATA,
                            OUString::number (WTMM( y + b + drawobj->offset2.y)) + "mm");
                    }
                    mxList->addAttribute("svg:width", sXML_CDATA,
                        OUString::number (WTMM( drawobj->extent.w )) + "mm");
                    mxList->addAttribute("svg:height", sXML_CDATA,
                        OUString::number (WTMM( drawobj->extent.h )) + "mm");
                    sprintf(d->buf, "0 0 %d %d", WTSM(drawobj->extent.w) , WTSM(drawobj->extent.h) );
                    mxList->addAttribute("svg:viewBox", sXML_CDATA, OUString::createFromAscii(d->buf));

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

                              sprintf(d->buf, "M%d %dC%d %d", WTSM(xarr[0]), WTSM(yarr[0]),
                                      WTSM(xarr[0] + xb[0]/3), WTSM(yarr[0] + yb[0]/3) );
                              oustr.appendAscii(d->buf);

                              for( i = 1 ; i < n  ; i++ ){
                                  if( i == n -1 ){
                                      sprintf(d->buf, " %d %d %d %dz",
                                              WTSM(xarr[i] - xb[i]/3), WTSM(yarr[i] - yb[i]/3),
                                              WTSM(xarr[i]), WTSM(yarr[i]) );
                                  }
                                  else{
                                      sprintf(d->buf, " %d %d %d %d %d %d",
                                              WTSM(xarr[i] - xb[i]/3), WTSM(yarr[i] - yb[i]/3),
                                              WTSM(xarr[i]), WTSM(yarr[i]),
                                              WTSM(xarr[i] + xb[i]/3), WTSM(yarr[i] + yb[i]/3) );
                                  }

                                  oustr.appendAscii(d->buf);
                              }
                    }

                    mxList->addAttribute("svg:d", sXML_CDATA, oustr.makeStringAndClear());

                    startEl("draw:path");
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
                    endEl("draw:path");
                    break;
                }
                case HWPDO_CLOSED_FREEFORM:
                case HWPDO_FREEFORM:              /* polygon */
                {
                    bool bIsPolygon = false;

                    mxList->addAttribute("svg:x", sXML_CDATA,
                                 OUString::number (WTMM( x + a + drawobj->offset2.x)) + "mm");
                    mxList->addAttribute("svg:y", sXML_CDATA,
                                 OUString::number (WTMM( y + b + drawobj->offset2.y)) + "mm");

                    mxList->addAttribute("svg:width", sXML_CDATA,
                        OUString::number (WTMM( drawobj->extent.w )) + "mm");
                    mxList->addAttribute("svg:height", sXML_CDATA,
                        OUString::number (WTMM( drawobj->extent.h )) + "mm");

                    sprintf(d->buf, "0 0 %d %d", WTSM(drawobj->extent.w), WTSM(drawobj->extent.h));
                    mxList->addAttribute("svg:viewBox", sXML_CDATA, OUString::createFromAscii(d->buf));

                    OUStringBuffer oustr;

                    if (drawobj->u.freeform.npt > 0)
                    {
                        sprintf(d->buf, "%d,%d", WTSM(drawobj->u.freeform.pt[0].x), WTSM(drawobj->u.freeform.pt[0].y));
                        oustr.appendAscii(d->buf);
                        int i;
                        for (i = 1; i < drawobj->u.freeform.npt  ; i++)
                        {
                            sprintf(d->buf, " %d,%d",
                                WTSM(drawobj->u.freeform.pt[i].x),
                                WTSM(drawobj->u.freeform.pt[i].y));
                            oustr.appendAscii(d->buf);
                        }
                        if( drawobj->u.freeform.pt[0].x == drawobj->u.freeform.pt[i-1].x &&
                            drawobj->u.freeform.pt[0].y == drawobj->u.freeform.pt[i-1].y )
                        {
                            bIsPolygon = true;
                        }
                    }
                    mxList->addAttribute("draw:points", sXML_CDATA, oustr.makeStringAndClear());

                    if( drawobj->property.fill_color <=  0xffffff ||
                        drawobj->property.pattern_type != 0)
                    {
                        bIsPolygon = true;
                    }

                    if(bIsPolygon)
                    {
                        startEl("draw:polygon");
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
                        endEl("draw:polygon");
                    }
                    else
                    {
                        startEl("draw:polyline");
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
                        endEl("draw:polyline");
                    }
                    break;
                }
                case HWPDO_TEXTBOX:
                    if( !bIsRotate )
                    {
                        mxList->addAttribute("svg:x", sXML_CDATA,
                            OUString::number (WTMM( x + a + drawobj->offset2.x)) + "mm");
                        mxList->addAttribute("svg:y", sXML_CDATA,
                            OUString::number (WTMM( y + b + drawobj->offset2.y)) + "mm");
                    }
                    mxList->addAttribute("svg:width", sXML_CDATA,
                        OUString::number (WTMM( drawobj->extent.w )) + "mm");
                    mxList->addAttribute("svg:height", sXML_CDATA,
                        OUString::number (WTMM( drawobj->extent.h )) + "mm");
                    if( drawobj->property.flag & 0x01 )
                    {
                        int value = drawobj->extent.w < drawobj->extent.h ?
                            drawobj->extent.w : drawobj->extent.h ;
                        mxList->addAttribute("draw:corner-radius", sXML_CDATA,
                            OUString::number (WTMM( value/10 )) + "mm");
                    }
                    else if( drawobj->property.flag & 0x04 )
                    {
                        int value = drawobj->extent.w < drawobj->extent.h ?
                            drawobj->extent.w : drawobj->extent.h ;
                        mxList->addAttribute("draw:corner-radius", sXML_CDATA,
                            OUString::number (WTMM( value / 2)) + "mm");
                    }

                    startEl("draw:text-box");
                    mxList->clear();

                    HWPPara *pPara = drawobj->u.textbox.h;
                                //parsePara(pPara);
                    while(pPara)
                    {
                        make_text_p1( pPara, false );
                        pPara = pPara->Next();
                    }

                    endEl("draw:text-box");
                    break;
            }
        }
        mxList->clear();
        drawobj = drawobj->next.get();
    }
}

void HwpReader::makeLine()
{
    mxList->addAttribute("text:style-name", sXML_CDATA, "Horizontal Line");
    startEl("text:p");
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

    mxList->addAttribute("text:condition", sXML_CDATA, "");
    mxList->addAttribute("text:string-value", sXML_CDATA, "");
    startEl("text:hidden-text");
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
    endEl("text:hidden-text");
}


/**
 * Converts footnote to text:footnote, endnote to text:endnote
 */
void HwpReader::makeFootnote(Footnote * hbox)
{
    if (hbox->type)
    {
        mxList->addAttribute("text:id", sXML_CDATA, "edn" + OUString::number(hbox->number));
        startEl("text:endnote");
        mxList->clear();
        mxList->addAttribute("text:label", sXML_CDATA, OUString::number(hbox->number));
        startEl("text:endnote-citation");
        mxList->clear();
        chars(OUString::number(hbox->number));
        endEl("text:endnote-citation");
        startEl("text:endnote-body");
        if (!hbox->plist.empty())
            parsePara(hbox->plist.front().get());
        endEl("text:endnote-body");
        endEl("text:endnote");
    }
    else
    {
        mxList->addAttribute("text:id", sXML_CDATA, "ftn" + OUString::number(hbox->number));
        startEl("text:footnote");
        mxList->clear();
        mxList->addAttribute("text:label", sXML_CDATA, OUString::number(hbox->number));
        startEl("text:footnote-citation");
        mxList->clear();
        chars(OUString::number(hbox->number));
        endEl("text:footnote-citation");
        startEl("text:footnote-body");
        if (!hbox->plist.empty())
            parsePara(hbox->plist.front().get());
        endEl("text:footnote-body");
        endEl("text:footnote");
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
            startEl("text:page-number");
            chars(OUString::number(hbox->number));
            endEl("text:page-number");
            break;
        case FNNUM_AUTO:
            break;
        case ENNUM_AUTO:
            break;
        case EQUNUM_AUTO:
        case PICNUM_AUTO:
            mxList->addAttribute("text:ref-name",sXML_CDATA, "refIllustration" + OUString::number(hbox->number));
            mxList->addAttribute("text:name",sXML_CDATA, "Illustration");
            mxList->addAttribute("style:num-format",sXML_CDATA, "1");
            startEl("text:sequence");
            chars(OUString::number(hbox->number));
            endEl("text:sequence");
            break;
        case TBLNUM_AUTO:
            mxList->addAttribute("text:ref-name",sXML_CDATA, "refTable" + OUString::number(hbox->number));
            mxList->addAttribute("text:name",sXML_CDATA, "Table");
            mxList->addAttribute("style:num-format",sXML_CDATA, "1");
            startEl("text:sequence");
            chars(OUString::number(hbox->number));
            endEl("text:sequence");
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

    mxList->addAttribute("draw:style-name", sXML_CDATA, "PNBox" + OUString::number(nPos));
    mxList->addAttribute("draw:name", sXML_CDATA, "PageNumber" + OUString::number(nPos));
    mxList->addAttribute("text:anchor-type", sXML_CDATA, "paragraph");
    mxList->addAttribute("svg:y", sXML_CDATA, "0cm");
    mxList->addAttribute("svg:width", sXML_CDATA, "2.0cm");
    mxList->addAttribute("fo:min-height", sXML_CDATA, "0.5cm");
    startEl("draw:text-box");
    mxList->clear();

    mxList->addAttribute("text:style-name", sXML_CDATA, "PNPara" + OUString::number(nPos));
    startEl("text:p");
    mxList->clear();
    if( hbox->shape > 2 )
        chars("- ");
    if( hbox->shape % 3 == 0 )
        mxList->addAttribute("style:num-format", sXML_CDATA, "1");
    else if( hbox->shape % 3 == 1 )
        mxList->addAttribute("style:num-format", sXML_CDATA, "I");
    else
        mxList->addAttribute("style:num-format", sXML_CDATA, "i");
    mxList->addAttribute("text:select-page", sXML_CDATA, "current");
    startEl("text:page-number");
    mxList->clear();
    chars("2");
    endEl("text:page-number");
    if( hbox->shape > 2 )
        chars(" -");
    endEl("text:p");
    endEl("draw:text-box");
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
                mxList->addAttribute("text:style-name", sXML_CDATA,
                    getPStyleName(para->GetParaShape().index));
                startEl("text:p");
                mxList->clear();
            }
            if( d->bFirstPara && d->bInBody )
            {
/* for HWP's Bookmark */
                mxList->addAttribute("text:name", sXML_CDATA, sBeginOfDoc);
                startEl("text:bookmark");
                mxList->clear();
                endEl("text:bookmark");
                d->bFirstPara = false;
            }
            if( d->bInHeader )
            {
                makeShowPageNum();
                d->bInHeader = false;
            }

            endEl("text:p");
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
constexpr OUStringLiteral SERVICE_NAME1 = u"com.sun.star.document.ImportFilter";
constexpr OUStringLiteral SERVICE_NAME2 = u"com.sun.star.document.ExtendedTypeDetection";

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

        Reference< XImporter > xImporter( xHandler, UNO_QUERY );
        rImporter = xImporter;
        rFilter = p;
    }
    catch( Exception & )
    {
        printf(" fail to instantiate %s\n", OUString(WRITER_IMPORTER_NAME).toUtf8().getStr() );
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
