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

#include "hwpreader.hxx"
#include <math.h>

#include <osl/diagnose.h>
#include <tools/stream.hxx>
#include <comphelper/newarray.hxx>

#include "fontmap.hxx"
#include "formula.h"
#include "cspline.h"

#include <iostream>
#include <locale.h>
#include <sal/types.h>

// To be shorten source code by realking
#define hconv(x)        hstr2ucsstr(x).c_str()
#define ascii(x)        OUString::createFromAscii(x)
#define rstartEl(x,y)   do { if (m_rxDocumentHandler.is()) m_rxDocumentHandler->startElement(x,y); } while(false)
#define rendEl(x)       do { if (m_rxDocumentHandler.is()) m_rxDocumentHandler->endElement(x); } while(false)
#define rchars(x)       do { if (m_rxDocumentHandler.is()) m_rxDocumentHandler->characters(x); } while(false)
#define padd(x,y,z)     pList->addAttribute(x,y,z)
#define Double2Str(x)   OUString::number((double)(x))
#define WTI(x)          ((double)(x) / 1800.)     // unit => inch
#define WTMM(x)     ((double)(x) / 1800. * 25.4)  // unit => mm
#define WTSM(x)     ((int)((x) / 1800. * 2540))   // unit ==> 1/100 mm

#define PI 3.14159265358979323846

// xmloff/xmlkyd.hxx
#define sXML_CDATA "CDATA"

#define STARTP  padd( "text:style-name", "CDATA", ascii(getPStyleName(((ParaShape &)para->GetParaShape()).index,buf))); \
    rstartEl( "text:p",rList ); \
    pList->clear(); \
    pstart = true
#define STARTT \
    curr = para->GetCharShape(n > 0 ? n-1 : 0)->index; \
    padd( "text:style-name", "CDATA" , ascii( getTStyleName(curr, buf) ) ); \
    rstartEl( "text:span",rList ); \
    pList->clear(); \
    tstart = true
#define ENDP \
    rendEl("text:p"); \
    pstart = false
#define ENDT \
    rendEl("text:span"); \
    tstart = false

static hchar *field = nullptr;
static char buf[1024];

namespace
{

template<typename T>
struct Free
{
    void operator()(T* const ptr)
    {
        free(ptr);
    }
};

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

    }
    bool bFirstPara;
    bool bInBody;
    bool bInHeader;
    ShowPageNum *pPn;
    int nPnPos;
};

HwpReader::HwpReader()
{
    pList = new AttributeListImpl;
    rList = static_cast<XAttributeList *>(pList);
    d = new HwpReaderPrivate;
}


HwpReader::~HwpReader()
{
    rList = nullptr;
    delete d;
}

extern "C" SAL_DLLPUBLIC_EXPORT bool SAL_CALL TestImportHWP(const OUString &rURL)
{
    SvFileStream aFileStream(rURL, StreamMode::READ);
    std::unique_ptr<HStream> stream(new HStream);
    byte aData[32768];
    sal_Size nRead, nBlock = 32768;

    while (true)
    {
        nRead = aFileStream.Read(aData, nBlock);
        if (nRead == 0)
            break;
        stream->addData(aData, (int)nRead);
    }

    HWPFile hwpfile;
    if (hwpfile.ReadHwpFile(stream.release()))
          return false;
    return true;
}

sal_Bool HwpReader::filter(const Sequence< PropertyValue >& rDescriptor) throw(RuntimeException, std::exception)
{
    utl::MediaDescriptor aDescriptor(rDescriptor);
    aDescriptor.addInputStream();

    Reference< XInputStream > xInputStream(
        aDescriptor[utl::MediaDescriptor::PROP_INPUTSTREAM()], UNO_QUERY_THROW);

    std::unique_ptr<HStream> stream(new HStream);
    Sequence < sal_Int8 > aBuffer;
    sal_Int32 nRead, nBlock = 32768, nTotal = 0;
    while( true )
    {
        nRead = xInputStream->readBytes(aBuffer, nBlock);
        if( nRead == 0 )
            break;
        stream->addData( reinterpret_cast<const byte *>(aBuffer.getConstArray()), nRead );
        nTotal += nRead;
    }

    if( nTotal == 0 ) return sal_False;

    if (hwpfile.ReadHwpFile(stream.release()))
          return sal_False;

    if (m_rxDocumentHandler.is())
        m_rxDocumentHandler->startDocument();

    padd("office:class", sXML_CDATA, "text");
    padd("office:version", sXML_CDATA, "0.9");

    padd("xmlns:office", "CDATA", "http://openoffice.org/2000/office");
    padd("xmlns:style", "CDATA", "http://openoffice.org/2000/style");
    padd("xmlns:text", "CDATA", "http://openoffice.org/2000/text");
    padd("xmlns:table", "CDATA", "http://openoffice.org/2000/table");
    padd("xmlns:draw", "CDATA", "http://openoffice.org/2000/drawing");
    padd("xmlns:fo", "CDATA", "http://www.w3.org/1999/XSL/Format");
    padd("xmlns:xlink", "CDATA", "http://www.w3.org/1999/xlink");
    padd("xmlns:dc", "CDATA", "http://purl.org/dc/elements/1.1/");
    padd("xmlns:meta", "CDATA", "http://openoffice.org/2000/meta");
    padd("xmlns:number", "CDATA", "http://openoffice.org/2000/datastyle");
    padd("xmlns:svg", "CDATA", "http://www.w3.org/2000/svg");
    padd("xmlns:chart", "CDATA", "http://openoffice.org/2000/chart");
    padd("xmlns:dr3d", "CDATA", "http://openoffice.org/2000/dr3d");
    padd("xmlns:math", "CDATA", "http://www.w3.org/1998/Math/MathML");
    padd("xmlns:form", "CDATA", "http://openoffice.org/2000/form");
    padd("xmlns:script", "CDATA", "http://openoffice.org/2000/script");

    rstartEl("office:document", rList);
    pList->clear();

    makeMeta();
    makeStyles();
    makeAutoStyles();
    makeMasterStyles();
    makeBody();

    rendEl("office:document");

    if (m_rxDocumentHandler.is())
        m_rxDocumentHandler->endDocument();
    return sal_True;
}


/**
 * make office:body
 */
void HwpReader::makeBody()
{
    rstartEl("office:body", rList);
    makeTextDecls();
    HWPPara *hwppara = hwpfile.GetFirstPara();
    d->bInBody = true;
    parsePara(hwppara);
    rendEl("office:body");
    d->bInBody = false;
}


/**
 * make text decls
 */
void HwpReader::makeTextDecls()
{
    rstartEl("text:sequence-decls", rList);
    padd("text:display-outline-level", sXML_CDATA, "0");
    padd("text:name", sXML_CDATA, "Illustration");
    rstartEl("text:sequence-decl", rList);
    pList->clear();
    rendEl("text:sequence-decl");
    padd("text:display-outline-level", sXML_CDATA, "0");
    padd("text:name", sXML_CDATA, "Table");
    rstartEl("text:sequence-decl", rList);
    pList->clear();
    rendEl("text:sequence-decl");
    padd("text:display-outline-level", sXML_CDATA, "0");
    padd("text:name", sXML_CDATA, "Text");
    rstartEl("text:sequence-decl", rList);
    pList->clear();
    rendEl("text:sequence-decl");
    padd("text:display-outline-level", sXML_CDATA, "0");
    padd("text:name", sXML_CDATA, "Drawing");
    rstartEl("text:sequence-decl", rList);
    pList->clear();
    rendEl("text:sequence-decl");
    rendEl("text:sequence-decls");
}


#define ISNUMBER(x) ( (x) <= 0x39 && (x) >= 0x30 )
/**
 * make office:meta
 * Completed
 */
void HwpReader::makeMeta()
{
    HWPInfo& hwpinfo = hwpfile.GetHWPInfo();

    rstartEl("office:meta", rList);

    if (hwpinfo.summary.title[0])
    {
        rstartEl("dc:title", rList);
        rchars(reinterpret_cast<sal_Unicode const *>(hconv(hwpinfo.summary.title)));
        rendEl("dc:title");
    }

    if (hwpinfo.summary.subject[0])
    {
        rstartEl("dc:subject", rList);
        rchars(reinterpret_cast<sal_Unicode const *>(hconv(hwpinfo.summary.subject)));
        rendEl("dc:subject");
    }

    if (hwpinfo.summary.author[0])
    {
        rstartEl("meta:initial-creator", rList);
        rchars(reinterpret_cast<sal_Unicode const *>(hconv(hwpinfo.summary.author)));
        rendEl("meta:initial-creator");
    }

    if (hwpinfo.summary.date[0])
    {
        unsigned short *pDate = hwpinfo.summary.date;
        int year,month,day,hour,minute;
        int gab = 0;
        if( ISNUMBER( pDate[0] ) && ISNUMBER( pDate[1] ) &&
            ISNUMBER( pDate[2] ) && ISNUMBER( pDate[3] ))
        {
            year = (pDate[0]-0x30) * 1000 + (pDate[1]-0x30) * 100 +
                (pDate[2]-0x30) * 10 + (pDate[3]-0x30);
        }
        else {
            year = 0;
        }
        if( ISNUMBER( pDate[6] ))
        {
            if( ISNUMBER( pDate[7] ) )
                month = (pDate[6] - 0x30) * 10 + (pDate[6+ ++gab]-0x30);
            else
                month = (pDate[6] - 0x30);
        }
        else {
            month = 0;
        }
        if( ISNUMBER( pDate[9 + gab] ) )
        {
            if( ISNUMBER( pDate[10 + gab])) {
                day = ( pDate[9 + gab] - 0x30 ) * 10 + (pDate[9+ gab + 1]-0x30);
                ++gab;
            } else
                day = (pDate[9+gab]-0x30);
        }
        else {
            day = 0;
        }
        if( ISNUMBER( pDate[17 + gab] ) )
        {
            if( ISNUMBER( pDate[18 + gab])) {
                hour = ( pDate[17 + gab] - 0x30 ) * 10 + (pDate[17+ gab + 1]-0x30);
                ++gab;
            } else
                hour = (pDate[17+gab]-0x30);
        }
        else {
            hour = 0;
        }
        if( ISNUMBER( pDate[20 + gab] ) )
        {
            if( ISNUMBER( pDate[21 + gab])) {
                minute = ( pDate[20 + gab] - 0x30 ) * 10 + (pDate[20+ gab + 1]-0x30);
                ++gab;
            } else
                minute = (pDate[20+gab]-0x30);
        }
        else {
            minute = 0;
        }
        sprintf(buf,"%d-%02d-%02dT%02d:%02d:00",year,month,day,hour,minute);

        rstartEl( "meta:creation-date", rList );
        rchars( ascii(buf));
        rendEl( "meta:creation-date" );
    }

    if (hwpinfo.summary.keyword[0][0] || hwpinfo.summary.etc[0][0])
    {
        rstartEl("meta:keywords", rList);
        if (hwpinfo.summary.keyword[0][0])
        {
            rstartEl("meta:keyword", rList);
            rchars(reinterpret_cast<sal_Unicode const *>(hconv(hwpinfo.summary.keyword[0])));
            rendEl("meta:keyword");
        }
        if (hwpinfo.summary.keyword[1][0])
        {
            rstartEl("meta:keyword", rList);
            rchars(reinterpret_cast<sal_Unicode const *>(hconv(hwpinfo.summary.keyword[1])));
            rendEl("meta:keyword");
        }
        if (hwpinfo.summary.etc[0][0])
        {
            rstartEl("meta:keyword", rList);
            rchars(reinterpret_cast<sal_Unicode const *>(hconv(hwpinfo.summary.etc[0])));
            rendEl("meta:keyword");
        }
        if (hwpinfo.summary.etc[1][0])
        {
            rstartEl("meta:keyword", rList);
            rchars(reinterpret_cast<sal_Unicode const *>(hconv(hwpinfo.summary.etc[1])));
            rendEl("meta:keyword");
        }
        if (hwpinfo.summary.etc[2][0])
        {
            rstartEl("meta:keyword", rList);
            rchars(reinterpret_cast<sal_Unicode const *>(hconv(hwpinfo.summary.etc[2])));
            rendEl("meta:keyword");
        }
        rendEl("meta:keywords");
    }
    rendEl("office:meta");
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

static struct
{
    double dots1;
    double dots2;
    double distance;
}


LineStyle[] =
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
            makeDrawMiscStyle( hdo->child );

        HWPDOProperty *prop = &hdo->property;
        if( hdo->type == HWPDO_CONTAINER )
        {
            hdo = hdo->next;
            continue;
        }

        if( prop->line_pstyle > 0 && prop->line_pstyle < 5 && prop->line_color <= 0xffffff)
        {
            padd( "draw:name", sXML_CDATA, ascii(Int2Str(hdo->index, "LineType%d", buf)));
            padd( "draw:style", sXML_CDATA, "round");
            padd( "draw:dots1", sXML_CDATA, "1");
            padd( "draw:dots1-length", sXML_CDATA, Double2Str( LineStyle[prop->line_pstyle].dots1 * WTMM(prop->line_width) ) + "cm");
            if( prop->line_pstyle == 3 )
            {
                padd( "draw:dots2", sXML_CDATA, "1");
                padd( "draw:dots2-length", sXML_CDATA, Double2Str( LineStyle[prop->line_pstyle].dots2 * WTMM(prop->line_width) ) + "cm");
            }
            else if( prop->line_pstyle == 4 )
            {
                padd( "draw:dots2", sXML_CDATA, "2");
                padd( "draw:dots2-length", sXML_CDATA, Double2Str( LineStyle[prop->line_pstyle].dots2 * WTMM(prop->line_width)) + "cm");
            }
            padd( "draw:distance", sXML_CDATA, Double2Str( LineStyle[prop->line_pstyle].distance * WTMM(prop->line_width)) + "cm");
            rstartEl( "draw:stroke-dash", rList);
            pList->clear();
            rendEl( "draw:stroke-dash" );
        }

        if( hdo->type == HWPDO_LINE || hdo->type == HWPDO_ARC || hdo->type == HWPDO_FREEFORM ||
            hdo->type == HWPDO_ADVANCED_ARC )
        {
            if( prop->line_tstyle && !ArrowShape[prop->line_tstyle].bMade  )
            {
                ArrowShape[prop->line_tstyle].bMade = true;
                padd("draw:name", sXML_CDATA,
                    ascii(ArrowShape[prop->line_tstyle].name));
                if( prop->line_tstyle == 1 )
                {
                    padd("svg:viewBox", sXML_CDATA, "0 0 20 30");
                    padd("svg:d", sXML_CDATA, "m10 0-10 30h20z");
                }
                else if( prop->line_tstyle == 2 )
                {
                    padd("svg:viewBox", sXML_CDATA, "0 0 1122 2243");
                    padd("svg:d", sXML_CDATA, "m0 2108v17 17l12 42 30 34 38 21 43 4 29-8 30-21 25-26 13-34 343-1532 339 1520 13 42 29 34 39 21 42 4 42-12 34-30 21-42v-39-12l-4 4-440-1998-9-42-25-39-38-25-43-8-42 8-38 25-26 39-8 42z");
                }
                else if( prop->line_tstyle == 3 )
                {
                    padd("svg:viewBox", sXML_CDATA, "0 0 30 30");
                    padd("svg:d", sXML_CDATA, "m0 0h30v30h-30z");
                }
                rstartEl("draw:marker", rList);
                pList->clear();
                rendEl("draw:marker");
            }
            if( prop->line_hstyle && !ArrowShape[prop->line_hstyle].bMade)
            {
                ArrowShape[prop->line_hstyle].bMade = true;
                padd("draw:name", sXML_CDATA,
                    ascii(ArrowShape[prop->line_hstyle].name));
                if( prop->line_hstyle == 1 )
                {
                    padd("svg:viewBox", sXML_CDATA, "0 0 20 30");
                    padd("svg:d", sXML_CDATA, "m10 0-10 30h20z");
                }
                else if( prop->line_hstyle == 2 )
                {
                    padd("svg:viewBox", sXML_CDATA, "0 0 1122 2243");
                    padd("svg:d", sXML_CDATA, "m0 2108v17 17l12 42 30 34 38 21 43 4 29-8 30-21 25-26 13-34 343-1532 339 1520 13 42 29 34 39 21 42 4 42-12 34-30 21-42v-39-12l-4 4-440-1998-9-42-25-39-38-25-43-8-42 8-38 25-26 39-8 42z");
                }
                else if( prop->line_hstyle == 3 )
                {
                    padd("svg:viewBox", sXML_CDATA, "0 0 20 20");
                    padd("svg:d", sXML_CDATA, "m0 0h20v20h-20z");
                }
                rstartEl("draw:marker", rList);
                pList->clear();
                rendEl("draw:marker");
            }
        }

        if( hdo->type != HWPDO_LINE )
        {
            if( prop->flag >> 18  & 0x01 )
            {
                padd( "draw:name", sXML_CDATA, ascii(Int2Str(hdo->index, "fillimage%d", buf)));
                if( !prop->pictype )
                {
                    padd( "xlink:href", sXML_CDATA,
                        reinterpret_cast<sal_Unicode const *>(hconv(kstr2hstr( reinterpret_cast<uchar const *>(urltounix(prop->szPatternFile).c_str())).c_str())));
                }
                else
                {
                    EmPicture *emp = nullptr;
                    if ( strlen( prop->szPatternFile ) > 3)
                        emp = hwpfile.GetEmPictureByName(prop->szPatternFile);
                    if( emp )
                    {
                        char filename[128+17+9];
                        char dirname[128];
                        int fd;
#ifdef _WIN32
                        GetTempPath(sizeof(dirname), dirname);
                        sprintf(filename, "%s%s",dirname, emp->name);
                        if( (fd = open( filename , _O_CREAT | _O_WRONLY | _O_BINARY , 0666)) >= 0 )
#else
                            strcpy(dirname, "/tmp/");
                        sprintf(filename, "%s%s", dirname, emp->name);
                        if( (fd = open( filename , O_CREAT | O_WRONLY , 0666)) >= 0 )
#endif
                        {
                            size_t nWritten = write(fd, emp->data, emp->size);
                            OSL_VERIFY(nWritten == emp->size);
                            close(fd);
                        }
#ifdef _WIN32
                        int j;
                        for(j = 0 ; j < (int)strlen( dirname ) ; j++)
                        {
                            if( dirname[j] == '\\' ) buf[j] = '/';
                            else buf[j] = dirname[j];
                        }
                        buf[j] = '\0';
                        sprintf(filename, "file:///%s%s",buf, emp->name );
#else
                        sprintf(filename, "file://%s%s",dirname, emp->name );
#endif
                        padd( "xlink:href", sXML_CDATA, ascii(filename));
                    }
                    else
                    {
                        padd( "xlink:href", sXML_CDATA,
                            reinterpret_cast<sal_Unicode const *>(hconv(kstr2hstr( reinterpret_cast<uchar const *>(urltounix(prop->szPatternFile).c_str())).c_str())));
                    }

                }
                padd( "xlink:type", sXML_CDATA, "simple");
                padd( "xlink:show", sXML_CDATA, "embed");
                padd( "xlink:actuate", sXML_CDATA, "onLoad");

                rstartEl( "draw:fill-image", rList);
                pList->clear();
                rendEl( "draw:fill-image");
            }
/*  If there is a gradient, when a bitmap file is present, this is the first. */
            else if( prop->flag >> 16  & 0x01 )   /* existence gradient */
            {
                padd( "draw:name", sXML_CDATA, ascii(Int2Str(hdo->index, "Grad%d", buf)));
                switch( prop->gstyle )
                {
                    case 1 :
                        if( prop->center_y == 50 )
                            padd( "draw:style", sXML_CDATA, "axial");
                        else
                            padd( "draw:style", sXML_CDATA, "linear");
                        break;
                    case 2:
                    case 3:
                        padd( "draw:style", sXML_CDATA, "radial");
                        break;
                    case 4:
                        padd( "draw:style", sXML_CDATA, "square");
                        break;
                    default:
                        padd( "draw:style", sXML_CDATA, "linear");
                        break;
                }
                padd( "draw:cx", sXML_CDATA,ascii(Int2Str(prop->center_x, "%d%%", buf)));
                padd( "draw:cy", sXML_CDATA,ascii(Int2Str(prop->center_y, "%d%%", buf)));

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
                        sprintf( buf, "#%02x%02x%02x", prop->tocolor & 0xff,
                            (prop->tocolor >> 8) & 0xff, (prop->tocolor >>16) & 0xff );
                        padd( "draw:start-color", sXML_CDATA, ascii( buf ));
                        sprintf( buf, "#%02x%02x%02x", prop->fromcolor & 0xff,
                            (prop->fromcolor >> 8) & 0xff, (prop->fromcolor >>16) & 0xff );
                        padd( "draw:end-color", sXML_CDATA, ascii( buf ));
                    }
                    else
                    {
                        sprintf( buf, "#%02x%02x%02x", prop->fromcolor & 0xff,
                            (prop->fromcolor >> 8) & 0xff, (prop->fromcolor >>16) & 0xff );
                        padd( "draw:start-color", sXML_CDATA, ascii( buf ));
                        sprintf( buf, "#%02x%02x%02x", prop->tocolor & 0xff,
                            (prop->tocolor >> 8) & 0xff, (prop->tocolor >>16) & 0xff );
                        padd( "draw:end-color", sXML_CDATA, ascii( buf ));
                    }
                }
                else
                {
                    sprintf( buf, "#%02x%02x%02x", prop->tocolor & 0xff,
                        (prop->tocolor >> 8) & 0xff, (prop->tocolor >>16) & 0xff );
                    padd( "draw:start-color", sXML_CDATA,ascii( buf ));

                    sprintf( buf, "#%02x%02x%02x", prop->fromcolor & 0xff,
                        (prop->fromcolor >> 8) & 0xff, (prop->fromcolor >>16) & 0xff );
                    padd( "draw:end-color", sXML_CDATA,ascii( buf ));
                }
                if( prop->angle > 0 && ( prop->gstyle == 1 || prop->gstyle == 4))
                {
                    int angle = 1800 - prop->angle * 10;
                    padd( "draw:angle", sXML_CDATA,
                        ascii(Int2Str( angle, "%d", buf)));
                }
                rstartEl( "draw:gradient", rList );
                pList->clear();
                rendEl( "draw:gradient");
            }
                                                  /* hatch */
            else if( prop->pattern_type >> 24 & 0x01 )
            {
                int type = prop->pattern_type & 0xffffff;
                padd( "draw:name", sXML_CDATA,
                    ascii(Int2Str(hdo->index, "Hatch%d", buf)));
                if( type < 4 )
                    padd( "draw:style", sXML_CDATA, "single" );
                else
                    padd( "draw:style", sXML_CDATA, "double" );
                sprintf( buf, "#%02x%02x%02x",
                    sal_uInt16(prop->pattern_color & 0xff),
                    sal_uInt16((prop->pattern_color >> 8) & 0xff),
                    sal_uInt16((prop->pattern_color >>16) & 0xff) );
                padd( "draw:color", sXML_CDATA, ascii( buf ));
                padd( "draw:distance", sXML_CDATA, "0.12cm");
                switch( type )
                {
                    case 0 :
                    case 4 :
                        padd( "draw:rotation", sXML_CDATA, "0");
                        break;
                    case 1 :
                        padd( "draw:rotation", sXML_CDATA, "900");
                        break;
                    case 2 :
                        padd( "draw:rotation", sXML_CDATA, "1350");
                        break;
                    case 3 :
                    case 5 :
                        padd( "draw:rotation", sXML_CDATA, "450");
                        break;
                }
                rstartEl( "draw:hatch", rList);
                pList->clear();
                rendEl( "draw:hatch");
            }
        }
        hdo = hdo->next;
    }
}


void HwpReader::makeStyles()
{
    HWPStyle& hwpstyle = hwpfile.GetHWPStyle();

    rstartEl("office:styles", rList);

    int i;
    for (i = 0; i < hwpfile.getFBoxStyleCount(); i++)
    {
        if( hwpfile.getFBoxStyle(i)->boxtype == 'D' )
        {
            makeDrawMiscStyle(static_cast<HWPDrawingObject *>(hwpfile.getFBoxStyle(i)->cell) );
        }
    }

    padd("style:name", sXML_CDATA, "Standard");
    padd("style:family", sXML_CDATA, "paragraph");
    padd("style:class", sXML_CDATA, "text");
    rstartEl("style:style", rList);
    pList->clear();

    padd("fo:line-height", sXML_CDATA, "160%");
    padd("fo:text-align", sXML_CDATA, "justify");
    rstartEl("style:properties", rList);
    pList->clear();
    rstartEl("style:tab-stops", rList);

    for( i = 1 ; i < 40 ; i++)
    {
        padd("style:position", sXML_CDATA,
            Double2Str( WTI(1000 * i)) + "inch");
        rstartEl("style:tab-stop", rList);
        pList->clear();
        rendEl("style:tab-stop");
    }
    rendEl("style:tab-stops");
    rendEl("style:properties");

    rendEl("style:style");

    for (int ii = 0; ii < hwpstyle.Num(); ii++)
    {
        unsigned char *stylename = reinterpret_cast<unsigned char *>(hwpstyle.GetName(ii));
        padd("style:name", sXML_CDATA, reinterpret_cast<sal_Unicode const *>(hconv(kstr2hstr(stylename).c_str())));
        padd("style:family", sXML_CDATA, "paragraph");
        padd("style:parent-style-name", sXML_CDATA, "Standard");

        rstartEl("style:style", rList);

        pList->clear();

        parseCharShape(hwpstyle.GetCharShape(ii));
        parseParaShape(hwpstyle.GetParaShape(ii));

        rstartEl("style:properties", rList);
        pList->clear();
        rendEl("style:properties");

        rendEl("style:style");
    }

    {
        padd( "style:name", sXML_CDATA, "Header");
        padd( "style:family", sXML_CDATA, "paragraph");
        padd( "style:parent-style-name", sXML_CDATA, "Standard");
        padd( "style:class", sXML_CDATA, "extra");
        rstartEl("style:style", rList);
        pList->clear();
        rendEl("style:style");
    }

    {
        padd( "style:name", sXML_CDATA, "Footer");
        padd( "style:family", sXML_CDATA, "paragraph");
        padd( "style:parent-style-name", sXML_CDATA, "Standard");
        padd( "style:class", sXML_CDATA, "extra");
        rstartEl("style:style", rList);
        pList->clear();

        rendEl("style:style");
    }

    if( hwpfile.linenumber > 0)
    {
        padd( "style:name", sXML_CDATA, "Horizontal Line");
        padd( "style:family", sXML_CDATA, "paragraph");
        padd( "style:parent-style-name", sXML_CDATA, "Standard");
        padd( "style:class", sXML_CDATA, "html");
        rstartEl( "style:style", rList);
        pList->clear();
        padd( "fo:font-size", sXML_CDATA, "6pt");
        padd( "fo:margin-top", sXML_CDATA, "0cm");
        padd( "fo:margin-bottom", sXML_CDATA, "0cm");
        padd( "style:border-line-width-bottom", sXML_CDATA, "0.02cm 0.035cm 0.002cm");
        padd( "fo:padding", sXML_CDATA, "0cm");
        padd( "fo:border-bottom", sXML_CDATA, "0.039cm double #808080");
        padd( "text:number-lines", sXML_CDATA, "false");
        padd( "text:line-number", sXML_CDATA, "0");
        padd("fo:line-height", sXML_CDATA, "100%");
        rstartEl( "style:properties", rList);
        pList->clear();
        rendEl( "style:properties");
        rendEl( "style:style");
    }

    HWPInfo& hwpinfo = hwpfile.GetHWPInfo();

    padd("text:num-suffix", sXML_CDATA, ")");
    padd("text:num-format", sXML_CDATA, "1");
    if( hwpinfo.beginfnnum != 1)
        padd("text:offset", sXML_CDATA, ascii(Int2Str(hwpinfo.beginfnnum -1, "%d", buf)));
    rstartEl("text:footnotes-configuration", rList);
    pList->clear();
    rendEl("text:footnotes-configuration");

    rendEl("office:styles");
}


/**
 * parse automatic styles from hwpfile
 * Define a style that is automatically reflected. For example, defining styles of each paragraph, tables, header, and etc,. at here. In Body, use the defined style.
 * 1. supports for the styles of paragraph, text, fbox, and page.
 */
void HwpReader::makeAutoStyles()
{
    int i;

    rstartEl("office:automatic-styles", rList);

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
        padd("style:name", sXML_CDATA,
            ascii(Int2Str(i,"PNPara%d", buf)));
        padd("style:family", sXML_CDATA, "paragraph");
        padd("style:parent-style-name", sXML_CDATA, "Standard");
        rstartEl("style:style", rList);
        pList->clear();
        if( i == 1 )
            padd("fo:text-align", sXML_CDATA, "start");
        else if ( i == 2 )
            padd("fo:text-align", sXML_CDATA, "center");
        else if ( i == 3 )
            padd("fo:text-align", sXML_CDATA, "end");
        rstartEl("style:properties", rList);
        pList->clear();
        rendEl( "style:properties");
        rendEl( "style:style");

        padd("style:name", sXML_CDATA, ascii(Int2Str(i,"PNBox%d",buf)));
        padd("style:family", sXML_CDATA, "graphics");
        rstartEl("style:style", rList);
        pList->clear();

        padd("fo:margin-top", sXML_CDATA, "0cm");
        padd("fo:margin-bottom", sXML_CDATA, "0cm");
        padd("style:wrap", sXML_CDATA, "run-through");
        padd("style:vertical-pos", sXML_CDATA, "from-top");
        padd("style:vertical-rel", sXML_CDATA, "paragraph");

        if( i == 1 )
            padd("style:horizontal-pos", sXML_CDATA, "left");
        else if ( i == 2 )
            padd("style:horizontal-pos", sXML_CDATA, "center");
        else if ( i == 3 )
            padd("style:horizontal-pos", sXML_CDATA, "right");
        padd("style:horizontal-rel", sXML_CDATA, "paragraph");
        padd("fo:padding", sXML_CDATA, "0cm");
        padd("stylefamily", sXML_CDATA, "graphics");
        rstartEl("style:properties", rList);
        pList->clear();
        rendEl("style:properties");
        rendEl("style:style");
    }

    for (i = 0; i < hwpfile.getDateFormatCount(); i++)
        makeDateFormat(hwpfile.getDateCode(i));

    makePageStyle();

    rendEl("office:automatic-styles");
}


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

void HwpReader::makeMasterStyles()
{
    rstartEl("office:master-styles", rList);

    int i;
    int nMax = hwpfile.getMaxSettedPage();
    std::deque<PageSetting> pSet(nMax + 1);

    for( i = 0 ; i < hwpfile.getPageNumberCount() ; i++ )
    {
        ShowPageNum *pn = hwpfile.getPageNumber(i);
        pSet[pn->m_nPageNumber].pagenumber = pn;
        pSet[pn->m_nPageNumber].bIsSet = true;
    }
    for( i = 0 ; i < hwpfile.getHeaderFooterCount() ; i++ )
    {
        HeaderFooter* hf = hwpfile.getHeaderFooter(i);
        pSet[hf->m_nPageNumber].bIsSet = true;
        if( hf->type == 0 )                       // header
        {
            switch( hf->where )
            {
                case 0 :
                    pSet[hf->m_nPageNumber].header = hf;
                    pSet[hf->m_nPageNumber].header_even = nullptr;
                    pSet[hf->m_nPageNumber].header_odd = nullptr;
                    break;
                case 1:
                    pSet[hf->m_nPageNumber].header_even = hf;
                    if( pSet[hf->m_nPageNumber].header )
                    {
                        pSet[hf->m_nPageNumber].header_odd =
                            pSet[hf->m_nPageNumber].header;
                        pSet[hf->m_nPageNumber].header = nullptr;
                    }
                    break;
                case 2:
                    pSet[hf->m_nPageNumber].header_odd = hf;
                    if( pSet[hf->m_nPageNumber].header )
                    {
                        pSet[hf->m_nPageNumber].header_even =
                            pSet[hf->m_nPageNumber].header;
                        pSet[hf->m_nPageNumber].header = nullptr;
                    }
                    break;
            }
        }
        else                                      // footer
        {
            switch( hf->where )
            {
                case 0 :
                    pSet[hf->m_nPageNumber].footer = hf;
                    pSet[hf->m_nPageNumber].footer_even = nullptr;
                    pSet[hf->m_nPageNumber].footer_odd = nullptr;
                    break;
                case 1:
                    pSet[hf->m_nPageNumber].footer_even = hf;
                    if( pSet[hf->m_nPageNumber].footer )
                    {
                        pSet[hf->m_nPageNumber].footer_odd =
                            pSet[hf->m_nPageNumber].footer;
                        pSet[hf->m_nPageNumber].footer = nullptr;
                    }
                    break;
                case 2:
                    pSet[hf->m_nPageNumber].footer_odd = hf;
                    if( pSet[hf->m_nPageNumber].footer )
                    {
                        pSet[hf->m_nPageNumber].footer_even =
                            pSet[hf->m_nPageNumber].footer;
                        pSet[hf->m_nPageNumber].footer = nullptr;
                    }
                    break;
            }
        }
    }

    PageSetting *pPrevSet = nullptr;
    PageSetting *pPage = nullptr;

    for( i = 1; i <= nMax ; i++ )
    {
        if( i == 1 )
            padd("style:name", sXML_CDATA, "Standard");
        else
            padd("style:name", sXML_CDATA,
                ascii(Int2Str(i, "p%d", buf)));
        padd("style:page-master-name", sXML_CDATA,
                ascii(Int2Str(hwpfile.GetPageMasterNum(i), "pm%d", buf)));
        if( i < nMax )
            padd("style:next-style-name", sXML_CDATA,
                ascii(Int2Str(i+1, "p%d", buf)));
        padd("draw:style-name", sXML_CDATA,
            ascii(Int2Str(i, "master%d", buf)));
        rstartEl("style:master-page", rList);
        pList->clear();

        if( pSet[i].bIsSet )                      /* If you've changed the current setting */
        {
              if( !pSet[i].pagenumber ){
                    if( pPrevSet && pPrevSet->pagenumber )
                         pSet[i].pagenumber = pPrevSet->pagenumber;
              }
            if( pSet[i].pagenumber )
            {
                if( pSet[i].pagenumber->where == 7 && pSet[i].header )
                {
                    pSet[i].header_even = pSet[i].header;
                    pSet[i].header_odd = pSet[i].header;
                    pSet[i].header = nullptr;
                }
                if( pSet[i].pagenumber->where == 8 && pSet[i].footer )
                {
                    pSet[i].footer_even = pSet[i].footer;
                    pSet[i].footer_odd = pSet[i].footer;
                    pSet[i].footer = nullptr;
                }
            }

            if( !pSet[i].header_even && pPrevSet && pPrevSet->header_even )
            {
                pSet[i].header_even = pPrevSet->header_even;
            }
            if( !pSet[i].header_odd && pPrevSet && pPrevSet->header_odd )
            {
                pSet[i].header_odd = pPrevSet->header_odd;
            }
            if( !pSet[i].footer_even && pPrevSet && pPrevSet->footer_even )
            {
                pSet[i].footer_even = pPrevSet->footer_even;
            }
            if( !pSet[i].footer_odd && pPrevSet && pPrevSet->footer_odd )
            {
                pSet[i].footer_odd = pPrevSet->footer_odd;
            }

            pPage = &pSet[i];
            pPrevSet = &pSet[i];
        }
        else if( pPrevSet )                       /* If the previous setting exists */
        {
            pPage = pPrevSet;
        }
        else                                      /* If the previous settings doesn't exist, set to the default settings */
        {
            rstartEl("style:header", rList);
            padd("text:style-name", sXML_CDATA, "Standard");
            rstartEl("text:p", rList);
            pList->clear();
            rendEl("text:p");
            rendEl("style:header");

            rstartEl("style:footer", rList);
            padd("text:style-name", sXML_CDATA, "Standard");
            rstartEl("text:p", rList);
            pList->clear();
            rendEl("text:p");
            rendEl("style:footer");

            rendEl("style:master-page");

            continue;
        }
// header
        if( pPage->header )
        {
            rstartEl("style:header", rList);
            if( pPage->pagenumber && pPage->pagenumber->where < 4 )
            {
                d->bInHeader = true;
                d->pPn = pPage->pagenumber;
            }
            parsePara(pPage->header->plist.front());
            d->bInHeader = false;
            d->pPn = nullptr;
            rendEl("style:header");
        }
        if( pPage->header_even )
        {
            rstartEl("style:header", rList);
            if( pPage->pagenumber && ( pPage->pagenumber->where < 4
                || pPage->pagenumber->where == 7 ) )
            {
                d->bInHeader = true;
                d->pPn = pPage->pagenumber;
                d->nPnPos = 3;
            }
            parsePara(pPage->header_even->plist.front());
            d->bInHeader = false;
            d->pPn = nullptr;
            d->nPnPos = 0;
            rendEl("style:header");
        }
                                                  /* Will be the default. */
        else if( pPage->header_odd && !pPage->header_even )
        {
            rstartEl("style:header", rList);
            padd("text:style-name", sXML_CDATA, "Standard");
            rstartEl("text:p", rList);
            pList->clear();
            if( pPage->pagenumber && ( pPage->pagenumber->where < 4 ||
                pPage->pagenumber->where == 7 ) )
            {
                d->pPn = pPage->pagenumber;
                d->nPnPos = 3;
                makeShowPageNum();
                d->pPn = nullptr;
                d->nPnPos = 0;
            }
            rendEl("text:p");
            rendEl("style:header");
        }
        if( pPage->header_odd )
        {
            rstartEl("style:header-left", rList);
            if( pPage->pagenumber && ( pPage->pagenumber->where < 4
                || pPage->pagenumber->where == 7 ) )
            {
                d->bInHeader = true;
                d->nPnPos = 1;
                d->pPn = pPage->pagenumber;
            }
            parsePara(pPage->header_odd->plist.front());
            d->bInHeader = false;
            d->pPn = nullptr;
            d->nPnPos = 0;
            rendEl("style:header-left");
        }
                                                  /* Will be the default.  */
        else if( pPage->header_even && !pPage->header_odd )
        {
            rstartEl("style:header-left", rList);
            padd("text:style-name", sXML_CDATA, "Standard");
            rstartEl("text:p", rList);
            pList->clear();
            if( pPage->pagenumber && ( pPage->pagenumber->where < 4 ||
                pPage->pagenumber->where == 7 ) )
            {
                d->pPn = pPage->pagenumber;
                d->nPnPos = 1;
                makeShowPageNum();
                d->pPn = nullptr;
                d->nPnPos = 0;
            }
            rendEl("text:p");
            rendEl("style:header-left");
        }
        if( !pPage->header && !pPage->header_even && !pPage->header_odd )
        {
            rstartEl("style:header", rList);
            padd("text:style-name", sXML_CDATA, "Standard");
            rstartEl("text:p", rList);
            pList->clear();
            if( pPage->pagenumber && (pPage->pagenumber->where < 4 ||
                pPage->pagenumber->where == 7 ) )
            {
                d->pPn = pPage->pagenumber;
                makeShowPageNum();
                d->pPn = nullptr;
            }
            rendEl("text:p");
            rendEl("style:header");
        }
// footer
        if( pPage->footer )
        {
            rstartEl("style:footer", rList);
            if( pPage->pagenumber && pPage->pagenumber->where >= 4
                && pPage->pagenumber->where != 7 )
            {
                d->bInHeader = true;
                d->pPn = pPage->pagenumber;
            }
            parsePara(pPage->footer->plist.front());
            d->bInHeader = false;
            d->pPn = nullptr;
            rendEl("style:footer");
        }
        if( pPage->footer_even )
        {
            rstartEl("style:footer", rList);
            if( pPage->pagenumber && pPage->pagenumber->where >= 4
                && pPage->pagenumber->where != 7 )
            {
                d->bInHeader = true;
                d->pPn = pPage->pagenumber;
                d->nPnPos = 3;
            }
            parsePara(pPage->footer_even->plist.front());
            d->bInHeader = false;
            d->pPn = nullptr;
            d->nPnPos = 0;
            rendEl("style:footer");
        }
                                                  /* Will be the default. */
        else if( pPage->footer_odd && !pPage->footer_even )
        {
            rstartEl("style:footer", rList);
            padd("text:style-name", sXML_CDATA, "Standard");
            rstartEl("text:p", rList);
            pList->clear();
            if( pPage->pagenumber && pPage->pagenumber->where >= 4
                && pPage->pagenumber->where != 7 )
            {
                d->pPn = pPage->pagenumber;
                d->nPnPos = 3;
                makeShowPageNum();
                d->pPn = nullptr;
                d->nPnPos = 0;
            }
            rendEl("text:p");
            rendEl("style:footer");
        }
        if( pPage->footer_odd )
        {
            rstartEl("style:footer-left", rList);
            if( pPage->pagenumber && pPage->pagenumber->where >= 4
                && pPage->pagenumber->where != 7 )
            {
                d->bInHeader = true;
                d->pPn = pPage->pagenumber;
                d->nPnPos = 1;
            }
            parsePara(pPage->footer_odd->plist.front());
            d->bInHeader = false;
            d->pPn = nullptr;
            d->nPnPos = 0;
            rendEl("style:footer-left");
        }
                                                  /* Will be the default. */
        else if( pPage->footer_even && !pPage->footer_odd )
        {
            rstartEl("style:footer-left", rList);
            padd("text:style-name", sXML_CDATA, "Standard");
            rstartEl("text:p", rList);
            pList->clear();
            if( pPage->pagenumber && pPage->pagenumber->where >= 4
                && pPage->pagenumber->where != 7 )
            {
                d->pPn = pPage->pagenumber;
                d->nPnPos = 1;
                makeShowPageNum();
                d->pPn = nullptr;
                d->nPnPos = 0;
            }
            rendEl("text:p");
            rendEl("style:footer-left");
        }
        if( !pPage->footer && !pPage->footer_even && !pPage->footer_odd )
        {
            rstartEl("style:footer", rList);
            padd("text:style-name", sXML_CDATA, "Standard");
            rstartEl("text:p", rList);
            pList->clear();
            if( pPage->pagenumber && pPage->pagenumber->where >= 4
                && pPage->pagenumber->where != 7 )
            {
                d->pPn = pPage->pagenumber;
                makeShowPageNum();
                d->pPn = nullptr;
            }
            rendEl("text:p");
            rendEl("style:footer");
        }

        rendEl("style:master-page");
    }
    rendEl("office:master-styles");
}


/**
 * Create the properties for text styles.
 * 1. fo:font-size, fo:font-family, fo:letter-spacing, fo:color,
 *    style:text-background-color, fo:font-style, fo:font-weight,
 *    style:text-underline,style:text-outline,fo:text-shadow,style:text-position
 *    Support them.
 */
void HwpReader::parseCharShape(CharShape * cshape)
{
    HWPFont& hwpfont = hwpfile.GetHWPFont();

    padd("fo:font-size", sXML_CDATA,
        ascii(Int2Str(cshape->size / 25, "%dpt", buf)));
    padd("style:font-size-asian", sXML_CDATA,
        ascii(Int2Str(cshape->size / 25, "%dpt", buf)));

    ::std::string const tmp = hstr2ksstr(kstr2hstr(
        reinterpret_cast<unsigned char const *>(hwpfont.GetFontName(0, cshape->font[0]))).c_str());
    double fRatio = 1.0;
    int size = getRepFamilyName(tmp.c_str(), buf, fRatio);

    padd("fo:font-family", sXML_CDATA,
        OUString(buf, size, RTL_TEXTENCODING_EUC_KR));
    padd("style:font-family-asian", sXML_CDATA,
        OUString(buf, size, RTL_TEXTENCODING_EUC_KR));

    padd("style:text-scale", sXML_CDATA,
        ascii(Int2Str((int)(cshape->ratio[0] * fRatio), "%d%%", buf)));

    double sspace = (cshape->size / 25) * cshape->space[0] / 100.;

    if (sspace != 0.)
    {
        padd("fo:letter-spacing", sXML_CDATA,
            Double2Str(sspace) + "pt");
    }
    if (cshape->color[1] != 0)
        padd("fo:color", sXML_CDATA,
            ascii(hcolor2str(cshape->color[1], 100, buf, true)));
    if (cshape->shade != 0)
        padd("style:text-background-color", sXML_CDATA,
            ascii(hcolor2str(cshape->color[0], cshape->shade, buf)));
    if (cshape->attr & 0x01)
    {
        padd("fo:font-style", sXML_CDATA, "italic");
        padd("style:font-style-asian", sXML_CDATA, "italic");
    }
     else{
        padd("fo:font-style", sXML_CDATA, "normal");
        padd("style:font-style-asian", sXML_CDATA, "normal");
     }
    if (cshape->attr >> 1 & 0x01)
    {
        padd("fo:font-weight", sXML_CDATA, "bold");
        padd("style:font-weight-asian", sXML_CDATA, "bold");
    }
     else{
        padd("fo:font-weight", sXML_CDATA, "normal");
        padd("style:font-weight-asian", sXML_CDATA, "normal");
     }
    if (cshape->attr >> 2 & 0x01)
        padd("style:text-underline", sXML_CDATA, "single");
    if (cshape->attr >> 3 & 0x01)
        padd("style:text-outline", sXML_CDATA, "true");
    if (cshape->attr >> 4 & 0x01)
        padd("fo:text-shadow", sXML_CDATA, "1pt 1pt");
    if (cshape->attr >> 5 & 0x01)
        padd("style:text-position", sXML_CDATA, "super 58%");
    if (cshape->attr >> 6 & 0x01)
        padd("style:text-position", sXML_CDATA, "sub 58%");

}


/**
 * Create the properties that correspond to the real Paragraph.
 * 1. fo:margin-left,fo:margin-right,fo:margin-top, fo:margin-bottom,
 *    fo:text-indent, fo:line-height, fo:text-align, fo:border
 *    are implemented.
 * TODO: Tab Settings => set values of properties only which doesn't have the default value
 */
void HwpReader::parseParaShape(ParaShape * pshape)
{

    if (pshape->left_margin != 0)
        padd("fo:margin-left", sXML_CDATA, Double2Str
            (WTI(pshape->left_margin )) + "inch");
    if (pshape->right_margin != 0)
        padd("fo:margin-right", sXML_CDATA, Double2Str
            (WTI(pshape->right_margin)) + "inch");
    if (pshape->pspacing_prev != 0)
        padd("fo:margin-top", sXML_CDATA, Double2Str
            (WTI(pshape->pspacing_prev)) + "inch");
    if (pshape->pspacing_next != 0)
        padd("fo:margin-bottom", sXML_CDATA, Double2Str
            (WTI(pshape->pspacing_next)) + "inch");
    if (pshape->indent != 0)
        padd("fo:text-indent", sXML_CDATA, Double2Str
            (WTI(pshape->indent)) + "inch");
    if (pshape->lspacing != 0)
        padd("fo:line-height", sXML_CDATA,
            ascii(Int2Str (pshape->lspacing, "%d%%", buf)));

    unsigned char set_align = 0;

    switch ((int) pshape->arrange_type)
    {
        case 1:
            strcpy(buf, "start");
            set_align = 1;
            break;
        case 2:
            strcpy(buf, "end");
            set_align = 1;
            break;
        case 3:
            strcpy(buf, "center");
            set_align = 1;
            break;
        case 4:
        case 5:
        case 6:
            strcpy(buf, "justify");
            set_align = 1;
            break;
    }

    if (set_align)
        padd("fo:text-align", sXML_CDATA, ascii(buf));

    if (pshape->outline)
        padd("fo:border", sXML_CDATA, "0.002cm solid #000000");
    if( pshape->shade > 0 )
    {
        padd("fo:background-color", sXML_CDATA,
            ascii(hcolor2str(0, pshape->shade, buf)));
    }

    if( pshape->pagebreak & 0x02 || pshape->pagebreak & 0x04)
        padd("fo:break-before", sXML_CDATA, "page");
     else if( pshape->pagebreak & 0x01 )
        padd("fo:break-before", sXML_CDATA, "column");

}


/**
 * Make the style of the Paragraph.
 */
void HwpReader::makePStyle(ParaShape * pshape)
{
    int nscount = pshape->tabs[MAXTABS -1].type;
    padd("style:name", sXML_CDATA,
        ascii(Int2Str(pshape->index, "P%d", buf)));
    padd("style:family", sXML_CDATA, "paragraph");
    rstartEl("style:style", rList);
    pList->clear();
    parseParaShape(pshape);
    parseCharShape(pshape->cshape);
    rstartEl("style:properties", rList);
    pList->clear();

    if( nscount )
    {
        unsigned char tf = 0;
        rstartEl("style:tab-stops",rList);

          int tab_margin = pshape->left_margin + pshape->indent;
          if( tab_margin < 0 )
              tab_margin = 0;
        for( int i = 0 ; i < MAXTABS -1 ; i++)
        {
            if( i > 0 && pshape->tabs[i].position == 0. )
                break;
            if( pshape->tabs[i].position <= tab_margin )
                continue;
            padd("style:position", sXML_CDATA,
                Double2Str(WTMM(pshape->tabs[i].position - tab_margin )) + "mm");
            if( pshape->tabs[i].type )
            {
                tf = 1;
                switch(pshape->tabs[i].type)
                {
                    case 1 :
                        padd("style:type", sXML_CDATA, "right");
                        break;
                    case 2:
                        padd("style:type", sXML_CDATA, "center");
                        break;
                    case 3:
                        padd("style:type", sXML_CDATA, "char");
                        padd("style:char", sXML_CDATA, ".");
                        break;
                }
            }
            if( pshape->tabs[i].dot_continue )
            {
                tf = 1;
                padd("style:leader-char", sXML_CDATA, ".");
            }
            rstartEl( "style:tab-stop", rList);
            pList->clear();
            rendEl( "style:tab-stop" );

            if( (pshape->tabs[i].position != 1000 * i ) || tf )
            {
                if( !--nscount ) break;
            }
        }
        rendEl( "style:tab-stops");
    }
    rendEl("style:properties");
    rendEl("style:style");
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
         padd("style:name", sXML_CDATA, ascii(Int2Str(i + 1, "pm%d", buf)));
         rstartEl("style:page-master",rList);
         pList->clear();


         switch( hwpinfo.paper.paper_kind )
         {
              case 3:                                   // A4
                    if( hwpinfo.paper.paper_direction )
                    {
                         padd("fo:page-height",sXML_CDATA, "210mm");
                         padd("fo:page-width",sXML_CDATA, "297mm");
                    }
                    else
                    {
                         padd("fo:page-width",sXML_CDATA, "210mm");
                         padd("fo:page-height",sXML_CDATA, "297mm");
                    }
                    break;
              case 4:                                   // 80 column
                    if( hwpinfo.paper.paper_direction )
                    {
                         padd("fo:page-height",sXML_CDATA, "8.5inch");
                         padd("fo:page-width",sXML_CDATA, "11inch");
                    }
                    else
                    {
                         padd("fo:page-width",sXML_CDATA, "8.5inch");
                         padd("fo:page-height",sXML_CDATA, "11inch");
                    }
                    break;
              case 5:                                   // B5
                    if( hwpinfo.paper.paper_direction )
                    {
                         padd("fo:page-height",sXML_CDATA, "176mm");
                         padd("fo:page-width",sXML_CDATA, "250mm");
                    }
                    else
                    {
                         padd("fo:page-width",sXML_CDATA, "176mm");
                         padd("fo:page-height",sXML_CDATA, "250mm");
                    }
                    break;
              case 6:                                   // B4
                    if( hwpinfo.paper.paper_direction )
                    {
                         padd("fo:page-height",sXML_CDATA, "250mm");
                         padd("fo:page-width",sXML_CDATA, "353mm");
                    }
                    else
                    {
                         padd("fo:page-width",sXML_CDATA, "250mm");
                         padd("fo:page-height",sXML_CDATA, "353mm");
                    }
                    break;
              case 7:
                    if( hwpinfo.paper.paper_direction )
                    {
                         padd("fo:page-height",sXML_CDATA, "8.5inch");
                         padd("fo:page-width",sXML_CDATA, "14inch");
                    }
                    else
                    {
                         padd("fo:page-width",sXML_CDATA, "8.5inch");
                         padd("fo:page-height",sXML_CDATA, "14inch");
                    }
                    break;
              case 8:
                    if( hwpinfo.paper.paper_direction )
                    {
                         padd("fo:page-height",sXML_CDATA, "297mm");
                         padd("fo:page-width",sXML_CDATA, "420mm");
                    }
                    else
                    {
                         padd("fo:page-width",sXML_CDATA, "297mm");
                         padd("fo:page-height",sXML_CDATA, "420mm");
                    }
                    break;
              case 0:
              case 1:
              case 2:
              default:
                    if( hwpinfo.paper.paper_direction )
                    {
                         padd("fo:page-width",sXML_CDATA,
                              Double2Str(WTI(hwpinfo.paper.paper_height)) + "inch");
                         padd("fo:page-height",sXML_CDATA,
                              Double2Str(WTI(hwpinfo.paper.paper_width)) + "inch");
                    }
                    else
                    {
                         padd("fo:page-width",sXML_CDATA,
                              Double2Str(WTI(hwpinfo.paper.paper_width)) + "inch");
                         padd("fo:page-height",sXML_CDATA,
                              Double2Str(WTI(hwpinfo.paper.paper_height)) + "inch");
                    }
                    break;

         }

         padd("style:print-orientation",sXML_CDATA,
              ascii(hwpinfo.paper.paper_direction ? "landscape" : "portrait"));
         if( hwpinfo.beginpagenum != 1)
              padd("style:first-page-number",sXML_CDATA,
                    ascii(Int2Str(hwpinfo.beginpagenum, "%d", buf)));

         if( hwpinfo.borderline ){
             padd("fo:margin-left",sXML_CDATA,
                  Double2Str(WTI(hwpinfo.paper.left_margin - hwpinfo.bordermargin[0] + hwpinfo.paper.gutter_length)) + "inch");
             padd("fo:margin-right",sXML_CDATA,
                  Double2Str(WTI(hwpinfo.paper.right_margin - hwpinfo.bordermargin[1])) + "inch");
             padd("fo:margin-top",sXML_CDATA,
                  Double2Str(WTI(hwpinfo.paper.top_margin - hwpinfo.bordermargin[2])) + "inch");
             padd("fo:margin-bottom",sXML_CDATA,
                  Double2Str(WTI(hwpinfo.paper.bottom_margin - hwpinfo.bordermargin[3])) + "inch");
         }
         else{
             padd("fo:margin-left",sXML_CDATA,
                  Double2Str(WTI(hwpinfo.paper.left_margin + hwpinfo.paper.gutter_length)) + "inch");
             padd("fo:margin-right",sXML_CDATA,
                  Double2Str(WTI(hwpinfo.paper.right_margin)) + "inch");
             padd("fo:margin-top",sXML_CDATA,
                  Double2Str(WTI(hwpinfo.paper.top_margin)) + "inch");
             padd("fo:margin-bottom",sXML_CDATA,
                  Double2Str(WTI(hwpinfo.paper.bottom_margin)) + "inch");
         }

         switch( hwpinfo.borderline )
         {
              case 1:
                    padd("fo:border", sXML_CDATA,"0.002cm solid #000000");
                    break;
              case 3:
                    padd("fo:border", sXML_CDATA,"0.002cm dotted #000000");
                    break;
              case 2:
                    padd("fo:border", sXML_CDATA,"0.035cm solid #000000");
                    break;
              case 4:
                    padd("style:border-line-width", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    padd("fo:border", sXML_CDATA,"0.039cm double #000000");
                    break;
         }

         padd("fo:padding-left", sXML_CDATA,
              Double2Str(WTI(hwpinfo.bordermargin[0])) + "inch");
         padd("fo:padding-right", sXML_CDATA,
              Double2Str(WTI(hwpinfo.bordermargin[1])) + "inch");
         padd("fo:padding-top", sXML_CDATA,
              Double2Str(WTI(hwpinfo.bordermargin[2])) + "inch");
         padd("fo:padding-bottom", sXML_CDATA,
              Double2Str(WTI(hwpinfo.bordermargin[3])) + "inch");

     /* background color */
         if( hwpinfo.back_info.isset )
         {
             if( hwpinfo.back_info.color[0] > 0 || hwpinfo.back_info.color[1] > 0
                     || hwpinfo.back_info.color[2] > 0 ){
                 sprintf(buf,"#%02x%02x%02x",hwpinfo.back_info.color[0],
                         hwpinfo.back_info.color[1],hwpinfo.back_info.color[2] );
                 padd("fo:background-color", sXML_CDATA, ascii(buf));
             }
         }

         rstartEl("style:properties",rList);
         pList->clear();

     /* background image */
         if( hwpinfo.back_info.isset && hwpinfo.back_info.type > 0 )
         {
             if( hwpinfo.back_info.type == 1 ){
#ifdef _WIN32
                 padd("xlink:href", sXML_CDATA,
                      reinterpret_cast<sal_Unicode const *>(hconv(kstr2hstr((uchar*) urltowin(hwpinfo.back_info.filename).c_str()).c_str())));
#else
                 padd("xlink:href", sXML_CDATA,
                    reinterpret_cast<sal_Unicode const *>(hconv(kstr2hstr( reinterpret_cast<uchar const *>(urltounix(hwpinfo.back_info.filename).c_str())).c_str())));
#endif
                 padd("xlink:type", sXML_CDATA, "simple");
                 padd("xlink:actuate", sXML_CDATA, "onLoad");
             }
             if( hwpinfo.back_info.flag >= 2)
                 padd("style:repeat", sXML_CDATA, "stretch");
             else if( hwpinfo.back_info.flag == 1 ){
                 padd("style:repeat", sXML_CDATA, "no-repeat");
                 padd("style:position", sXML_CDATA, "center");
             }
             rstartEl("style:background-image",rList);

             if( hwpinfo.back_info.type == 2 ){
                 rstartEl("office:binary-data", rList);
                 pList->clear();
                 std::shared_ptr<char> pStr(base64_encode_string(reinterpret_cast<unsigned char *>(hwpinfo.back_info.data), hwpinfo.back_info.size ), Free<char>());
                 rchars(ascii(pStr.get()));
                 rendEl("office:binary-data");
             }
             rendEl("style:background-image");
         }

         makeColumns( hwpfile.GetColumnDef(i) );

         rendEl("style:properties");

    /* header style */
         rstartEl("style:header-style", rList);
         padd("svg:height", sXML_CDATA,
              Double2Str(WTI(hwpinfo.paper.header_length)) + "inch");
         padd("fo:margin-bottom", sXML_CDATA, "0mm");

         rstartEl("style:properties",rList);
         pList->clear();
         rendEl("style:properties");
         rendEl("style:header-style");

    /* footer style */
         rstartEl("style:footer-style", rList);
         padd("svg:height", sXML_CDATA,
              Double2Str(WTI(hwpinfo.paper.footer_length)) + "inch");
         padd("fo:margin-top", sXML_CDATA, "0mm");
         rstartEl("style:properties",rList);
         pList->clear();
         rendEl("style:properties");
         rendEl("style:footer-style");

    /* Footnote style, but it fell in the dtd, the specification has been defined. REALKING */
         rstartEl("style:footnote-layout", rList);

         padd("style:distance-before-sep", sXML_CDATA,
              Double2Str(WTI(hwpinfo.splinetext)) + "inch");
         padd("style:distance-after-sep", sXML_CDATA,
              Double2Str(WTI(hwpinfo.splinefn)) + "inch");
         rstartEl("style:properties",rList);
         pList->clear();
         rendEl("style:properties");
         if ( hwpinfo.fnlinetype == 2 )
              padd("style:width", sXML_CDATA, "15cm");
         else if ( hwpinfo.fnlinetype == 1)
              padd("style:width", sXML_CDATA, "2cm");
         else if ( hwpinfo.fnlinetype == 3)
              padd("style:width", sXML_CDATA, "0cm");
         else
              padd("style:width", sXML_CDATA, "5cm");

         rstartEl("style:footnote-sep",rList);
         pList->clear();
         rendEl("style:footnote-sep");

         rendEl("style:footnote-layout");

         rendEl("style:page-master");
     }
}

void HwpReader::makeColumns(ColumnDef *coldef)
{
    if( !coldef ) return;
  padd("fo:column-count", sXML_CDATA, ascii(Int2Str(coldef->ncols, "%d", buf)));
  rstartEl("style:columns",rList);
  pList->clear();
  if( coldef->separator != 0 )
  {
        switch( coldef->separator )
        {
             case 1:                           /* thin line */
                  padd("style:width", sXML_CDATA, "0.02mm");
                  //fall-through
             case 3:                           /* dotted line */
                  padd("style:style", sXML_CDATA, "dotted");
                  padd("style:width", sXML_CDATA, "0.02mm");
                  break;
             case 2:                           /* thick line */
             case 4:                           /* double line */
                  padd("style:width", sXML_CDATA, "0.35mm");
                  break;
             case 0:                           /* None */
             default:
                  padd("style:style", sXML_CDATA, "none");
                  break;
        }
        rstartEl("style:column-sep",rList);
        pList->clear();
        rendEl("style:column-sep");
  }
  double spacing = WTI(coldef->spacing)/ 2. ;
  for(int ii = 0 ; ii < coldef->ncols ; ii++)
  {
        if( ii == 0 )
             padd("fo:margin-left", sXML_CDATA, "0mm");
        else
             padd("fo:margin-left", sXML_CDATA,
                  Double2Str( spacing) + "inch");
        if( ii == ( coldef->ncols -1) )
             padd("fo:margin-right", sXML_CDATA,"0mm");
        else
             padd("fo:margin-right", sXML_CDATA,
                  Double2Str( spacing) + "inch");
        rstartEl("style:column",rList);
        pList->clear();
        rendEl("style:column");
  }
  rendEl("style:columns");
}

void HwpReader::makeTStyle(CharShape * cshape)
{
    padd("style:name", sXML_CDATA,
        ascii(Int2Str(cshape->index, "T%d", buf)));
    padd("style:family", sXML_CDATA, "text");
    rstartEl("style:style", rList);
    pList->clear();
    parseCharShape(cshape);
    rstartEl("style:properties", rList);
    pList->clear();
    rendEl("style:properties");
    rendEl("style:style");
}


void HwpReader::makeTableStyle(Table *tbl)
{
// table
    TxtBox *hbox = tbl->box;

    padd("style:name", sXML_CDATA,
        ascii(Int2Str(hbox->style.boxnum, "Table%d", buf)));
    padd("style:family", sXML_CDATA,"table");
    rstartEl("style:style", rList);
    pList->clear();
    padd("style:width", sXML_CDATA,
        Double2Str(WTMM(hbox->box_xs)) + "mm");
    padd("table:align", sXML_CDATA,"left");
    padd("fo:keep-with-next", sXML_CDATA,"false");
    rstartEl("style:properties", rList);
    pList->clear();
    rendEl("style:properties");
    rendEl("style:style");

// column
    for (size_t i = 0 ; i < tbl->columns.nCount -1 ; i++)
    {
        sprintf(buf,"Table%d.%c",hbox->style.boxnum, static_cast<char>('A'+i));
        padd("style:name", sXML_CDATA, ascii( buf ));
        padd("style:family", sXML_CDATA,"table-column");
        rstartEl("style:style", rList);
        pList->clear();
        padd("style:column-width", sXML_CDATA,
            Double2Str(WTMM(tbl->columns.data[i+1] - tbl->columns.data[i])) + "mm");
        rstartEl("style:properties", rList);
        pList->clear();
        rendEl("style:properties");
        rendEl("style:style");
    }

// row
    for (size_t i = 0 ; i < tbl->rows.nCount -1 ; i++)
    {
        sprintf(buf,"Table%d.row%" SAL_PRI_SIZET "u",hbox->style.boxnum, i + 1);
        padd("style:name", sXML_CDATA, ascii( buf ));
        padd("style:family", sXML_CDATA,"table-row");
        rstartEl("style:style", rList);
        pList->clear();
        padd("style:row-height", sXML_CDATA,
            Double2Str(WTMM(tbl->rows.data[i+1] - tbl->rows.data[i])) + "mm");
        rstartEl("style:properties", rList);
        pList->clear();
        rendEl("style:properties");
        rendEl("style:style");
    }

// cell
    for (std::list<TCell*>::iterator it = tbl->cells.begin(), aEnd = tbl->cells.end(); it != aEnd; ++it)
    {
        TCell *tcell = *it;
        sprintf(buf,"Table%d.%c%d",hbox->style.boxnum, 'A'+ tcell->nColumnIndex, tcell->nRowIndex +1);
        padd("style:name", sXML_CDATA, ascii( buf ));
        padd("style:family", sXML_CDATA,"table-cell");
        rstartEl("style:style", rList);
        pList->clear();
        Cell *cl = tcell->pCell;
        if( cl->ver_align == 1 )
            padd("fo:vertical-align", sXML_CDATA,"middle");

        if(cl->linetype[2] == cl->linetype[3] && cl->linetype[2] == cl->linetype[0]
            && cl->linetype[2] == cl->linetype[1])
        {
            switch( cl->linetype[2] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    padd("fo:border", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    padd("fo:border", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    padd("style:border-line-width", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    padd("fo:border", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
        }
        else
        {
            switch( cl->linetype[0] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    padd("fo:border-left", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    padd("fo:border-left", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    padd("style:border-line-width-left", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    padd("fo:border-left", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
            switch( cl->linetype[1] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    padd("fo:border-right", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    padd("fo:border-right", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    padd("style:border-line-width-right", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    padd("fo:border-right", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
            switch( cl->linetype[2] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    padd("fo:border-top", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    padd("fo:border-top", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    padd("style:border-line-width-top", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    padd("fo:border-top", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
            switch( cl->linetype[3] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    padd("fo:border-bottom", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    padd("fo:border-bottom", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    padd("style:border-line-width-bottom", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    padd("fo:border-bottom", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
        }
        if(cl->shade != 0)
            padd("fo:background-color", sXML_CDATA,
                ascii(hcolor2str(sal::static_int_cast<uchar>(cl->color),
                                sal::static_int_cast<uchar>(cl->shade), buf)));

        rstartEl("style:properties", rList);
        pList->clear();
        rendEl("style:properties");

        rendEl("style:style");
    }
}


void HwpReader::makeDrawStyle( HWPDrawingObject * hdo, FBoxStyle * fstyle)
{
    while( hdo )
    {
        padd("style:name", sXML_CDATA,
            ascii(Int2Str(hdo->index, "Draw%d", buf)));
        padd("style:family", sXML_CDATA, "graphics");

        rstartEl("style:style", rList);
        pList->clear();

        switch (fstyle->txtflow)
        {
            case 0:
                break;
            case 1:
                padd("style:wrap", sXML_CDATA, "run-through");
                break;
            case 2:
                padd("style:wrap", sXML_CDATA, "dynamic");
                break;
        }
        long color;
// invisible line
        if( hdo->property.line_color > 0xffffff )
        {
            padd("draw:stroke", sXML_CDATA, "none" );
        }
        else
        {

            if( hdo->property.line_pstyle == 0 )
                padd("draw:stroke", sXML_CDATA, "solid" );
            else if( hdo->property.line_pstyle < 5 )
            {
                padd("draw:stroke", sXML_CDATA, "dash" );
                padd("draw:stroke-dash", sXML_CDATA,
                    ascii(Int2Str(hdo->index, "LineType%d", buf)));
            }
            padd("svg:stroke-width", sXML_CDATA,
                Double2Str( WTMM(hdo->property.line_width)) + "mm");
            color = hdo->property.line_color;
            sprintf( buf, "#%02x%02x%02x",
                    sal_uInt16(color & 0xff),
                    sal_uInt16((color >> 8) & 0xff),
                    sal_uInt16((color >>16) & 0xff) );
            padd("svg:stroke-color", sXML_CDATA, ascii( buf) );
        }

        if( hdo->type == HWPDO_LINE || hdo->type == HWPDO_ARC ||
            hdo->type == HWPDO_FREEFORM || hdo->type == HWPDO_ADVANCED_ARC )
        {

            if( hdo->property.line_tstyle > 0 )
            {
                padd("draw:marker-start", sXML_CDATA,
                    ascii(ArrowShape[hdo->property.line_tstyle].name) );
                     if( hdo->property.line_width > 100 )
                         padd("draw:marker-start-width", sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 3)) + "mm");
                     else if( hdo->property.line_width > 80 )
                         padd("draw:marker-start-width", sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 4)) + "mm");
                     else if( hdo->property.line_width > 60 )
                         padd("draw:marker-start-width", sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 5)) + "mm");
                     else if( hdo->property.line_width > 40 )
                         padd("draw:marker-start-width", sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 6)) + "mm");
                     else
                         padd("draw:marker-start-width", sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 7)) + "mm");
            }

            if( hdo->property.line_hstyle > 0 )
            {
                padd("draw:marker-end", sXML_CDATA,
                    ascii(ArrowShape[hdo->property.line_hstyle].name) );
                     if( hdo->property.line_width > 100 )
                         padd("draw:marker-end-width", sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 3)) + "mm");
                     else if( hdo->property.line_width > 80 )
                         padd("draw:marker-end-width", sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 4)) + "mm");
                     else if( hdo->property.line_width > 60 )
                         padd("draw:marker-end-width", sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 5)) + "mm");
                     else if( hdo->property.line_width > 40 )
                         padd("draw:marker-end-width", sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 6)) + "mm");
                     else
                         padd("draw:marker-end-width", sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 7)) + "mm");
            }
        }

        if(hdo->type != HWPDO_LINE )
        {
            if( hdo->property.flag >> 19 & 0x01 )
            {
                padd( "draw:textarea-horizontal-align", sXML_CDATA, "center");
            }

            color = hdo->property.fill_color;

            if( hdo->property.flag >> 18 & 0x01 ) // bitmap pattern
            {
                padd("draw:fill", sXML_CDATA, "bitmap");
                padd("draw:fill-image-name", sXML_CDATA,
                    ascii(Int2Str(hdo->index, "fillimage%d", buf)));
                                                  // bitmap resizing
                if( hdo->property.flag >> 3 & 0x01 )
                {
                    padd("style:repeat", sXML_CDATA, "stretch");
                }
                else
                {
                    padd("style:repeat", sXML_CDATA, "repeat");
                    padd("draw:fill-image-ref-point", sXML_CDATA, "top-left");
                }
                if( hdo->property.flag >> 20 & 0x01 )
                {
                    if( hdo->property.luminance > 0 )
                    {
                        padd("draw:transparency", sXML_CDATA,
                            ascii(Int2Str(hdo->property.luminance, "%d%%", buf)));
                    }
                }

            }
                                                  // Gradation
            else if( hdo->property.flag >> 16 & 0x01 )
            {
                padd("draw:fill", sXML_CDATA, "gradient");
                padd("draw:fill-gradient-name", sXML_CDATA,
                    ascii(Int2Str(hdo->index, "Grad%d", buf)));
                padd("draw:gradient-step-count", sXML_CDATA,
                    ascii(Int2Str(hdo->property.nstep, "%d", buf)));

            }
                                                  // Hatching
            else if( hdo->property.pattern_type >> 24 & 0x01 )
            {
                padd("draw:fill", sXML_CDATA, "hatch");
                padd("draw:fill-hatch-name", sXML_CDATA,
                    ascii(Int2Str(hdo->index, "Hatch%d", buf)));
                if( color < 0xffffff )
                {
                    sprintf( buf, "#%02x%02x%02x",
                        sal_uInt16(color & 0xff),
                        sal_uInt16((color >> 8) & 0xff),
                        sal_uInt16((color >>16) & 0xff) );
                    padd("draw:fill-color", sXML_CDATA, ascii( buf) );
                    padd("draw:fill-hatch-solid", sXML_CDATA, "true");
                }
            }
            else if( color <= 0xffffff )
            {
                padd("draw:fill", sXML_CDATA, "solid");
                sprintf( buf, "#%02x%02x%02x",
                    sal_uInt16(color & 0xff),
                    sal_uInt16((color >> 8) & 0xff),
                    sal_uInt16((color >>16) & 0xff) );
                padd("draw:fill-color", sXML_CDATA, ascii( buf) );
            }
            else
                padd("draw:fill", sXML_CDATA, "none");
        }

        if( fstyle->anchor_type == CHAR_ANCHOR )
        {
            padd("style:vertical-pos", sXML_CDATA, "top");
            padd("style:vertical-rel", sXML_CDATA, "baseline");
        }

        rstartEl("style:properties", rList);
        pList->clear();
        rendEl("style:properties");
        rendEl("style:style");

        if( hdo->type == 0 )
        {
            makeDrawStyle( hdo->child, fstyle );
        }
        hdo = hdo->next;
    }
}


void HwpReader::makeCaptionStyle(FBoxStyle * fstyle)
{
    padd("style:name", sXML_CDATA,
        ascii(Int2Str(fstyle->boxnum, "CapBox%d", buf)));
    padd("style:family", sXML_CDATA, "graphics");
    rstartEl("style:style", rList);
    pList->clear();
    padd("fo:margin-left", sXML_CDATA, "0cm");
    padd("fo:margin-right", sXML_CDATA, "0cm");
    padd("fo:margin-top", sXML_CDATA, "0cm");
    padd("fo:margin-bottom", sXML_CDATA, "0cm");
    padd("fo:padding", sXML_CDATA, "0cm");
    switch (fstyle->txtflow)
    {
        case 0:
            padd("style:wrap", sXML_CDATA, "none");
            break;
        case 1:
            if( fstyle->boxtype == 'G' )
                padd("style:run-through", sXML_CDATA, "background");
            padd("style:wrap", sXML_CDATA, "run-through");
            break;
        case 2:
            padd("style:wrap", sXML_CDATA, "dynamic");
            break;
    }
    if (fstyle->anchor_type == CHAR_ANCHOR)
    {
        padd("style:vertical-pos", sXML_CDATA, "top");
        padd("style:vertical-rel", sXML_CDATA, "baseline");
        padd("style:horizontal-pos", sXML_CDATA, "center");
        padd("style:horizontal-rel", sXML_CDATA, "paragraph");
    }
    else
    {

        switch (-(fstyle->xpos))
        {
            case 2:
                padd("style:horizontal-pos", sXML_CDATA, "right");
                break;
            case 3:
                padd("style:horizontal-pos", sXML_CDATA, "center");
                break;
            case 1:
            default:
                padd("style:horizontal-pos", sXML_CDATA, "from-left");
                break;
        }
        switch (-(fstyle->ypos))
        {
            case 2:
                padd("style:vertical-pos", sXML_CDATA, "bottom");
                break;
            case 3:
                padd("style:vertical-pos", sXML_CDATA, "middle");
                break;
            case 1:
            default:
                padd("style:vertical-pos", sXML_CDATA, "from-top");
                break;
        }
        if ( fstyle->anchor_type == PARA_ANCHOR )
        {
            padd("style:vertical-rel", sXML_CDATA, "paragraph");
            padd("style:horizontal-rel", sXML_CDATA, "paragraph");
        }
        else
        {
            padd("style:vertical-rel", sXML_CDATA, "page-content");
            padd("style:horizontal-rel", sXML_CDATA, "page-content");
        }
    }
    rstartEl("style:properties", rList);
    pList->clear();
    rendEl("style:properties");
    rendEl("style:style");
    if( fstyle->boxtype == 'G' )
    {
        padd("style:name", sXML_CDATA,
            ascii(Int2Str(fstyle->boxnum, "G%d", buf)));
    }
    else
    {
        padd("style:name", sXML_CDATA,
            ascii(Int2Str(fstyle->boxnum, "Txtbox%d", buf)));
    }

    padd("style:family", sXML_CDATA, "graphics");
    rstartEl("style:style", rList);
    pList->clear();

    padd("fo:margin-left", sXML_CDATA, "0cm");
    padd("fo:margin-right", sXML_CDATA, "0cm");
    padd("fo:margin-top", sXML_CDATA, "0cm");
    padd("fo:margin-bottom", sXML_CDATA, "0cm");
    padd("fo:padding", sXML_CDATA, "0cm");
    padd("style:wrap", sXML_CDATA, "none");
    padd("style:vertical-pos", sXML_CDATA, "from-top");
    padd("style:vertical-rel", sXML_CDATA, "paragraph");
    padd("style:horizontal-pos", sXML_CDATA, "from-left");
    padd("style:horizontal-rel", sXML_CDATA, "paragraph");
    if( fstyle->boxtype == 'G' )
    {
        char *cell = static_cast<char *>(fstyle->cell);
        padd("draw:luminance", sXML_CDATA,
            ascii(Int2Str(cell[0], "%d%%", buf)));
        padd("draw:contrast", sXML_CDATA,
            ascii(Int2Str(cell[1], "%d%%", buf)));
        if( cell[2] == 0 )
            padd("draw:color-mode", sXML_CDATA, "standard");
        else if( cell[2] == 1 )
            padd("draw:color-mode", sXML_CDATA, "greyscale");
        else if( cell[2] == 2 )
            padd("draw:color-mode", sXML_CDATA, "mono");
    }
    else
    {
        Cell *cell = static_cast<Cell *>(fstyle->cell);
        if(cell->linetype[0] == cell->linetype[1] &&
            cell->linetype[0] == cell->linetype[2] &&
            cell->linetype[0] == cell->linetype[3])
        {
            switch( cell->linetype[0] )
            {
                case 0:
                    padd("fo:padding", sXML_CDATA,"0mm");
                    break;
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    padd("fo:border", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    padd("fo:border", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    padd("style:border-line-width", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    padd("fo:border", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
        }
        else
        {
            switch( cell->linetype[0] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    padd("fo:border-left", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    padd("fo:border-left", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    padd("style:border-line-width-left", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    padd("fo:border-left", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
            switch( cell->linetype[1] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    padd("fo:border-right", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    padd("fo:border-right", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    padd("style:border-line-width-right", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    padd("fo:border-right", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
            switch( cell->linetype[2] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    padd("fo:border-top", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    padd("fo:border-top", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    padd("style:border-line-width-top", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    padd("fo:border-top", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
            switch( cell->linetype[3] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    padd("fo:border-bottom", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    padd("fo:border-bottom", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    padd("style:border-line-width-bottom", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    padd("fo:border-bottom", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
        }
        if(cell->shade != 0)
            padd("fo:background-color", sXML_CDATA, ascii(hcolor2str(
            sal::static_int_cast<uchar>(cell->color),
            sal::static_int_cast<uchar>(cell->shade), buf)));
    }
    rstartEl("style:properties", rList);
    pList->clear();
    rendEl("style:properties");
    rendEl("style:style");
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
            padd("style:name", sXML_CDATA,
                ascii(Int2Str(fstyle->boxnum, "Txtbox%d", buf)));
            padd("style:family", sXML_CDATA, "graphics");
            break;
        case 'G' :                                // graphics
            padd("style:name", sXML_CDATA,
                ascii(Int2Str(fstyle->boxnum, "G%d", buf)));
            padd("style:family", sXML_CDATA, "graphics");
            break;
        case 'L' :                                // line TODO : all
            padd("style:name", sXML_CDATA,
                ascii(Int2Str(fstyle->boxnum, "L%d", buf)));
            padd( "style:family" , sXML_CDATA , "paragraph" );
            break;
    }

    rstartEl("style:style", rList);
    pList->clear();

    if ( fstyle->boxtype == 'T')
    {
        padd("fo:padding", sXML_CDATA, "0cm");
    }

    if( !(fstyle->boxtype == 'G' && fstyle->cap_len > 0 ))
    {
        padd("fo:margin-left", sXML_CDATA,
            Double2Str(WTMM(fstyle->margin[0][0]) ) + "mm");
        padd("fo:margin-right", sXML_CDATA,
            Double2Str(WTMM(fstyle->margin[0][1])) + "mm");
        padd("fo:margin-top", sXML_CDATA,
            Double2Str(WTMM(fstyle->margin[0][2])) + "mm");
        padd("fo:margin-bottom", sXML_CDATA,
            Double2Str(WTMM(fstyle->margin[0][3])) + "mm");
    }

    switch (fstyle->txtflow)
    {
        case 0:
            padd("style:wrap", sXML_CDATA, "none");
            break;
        case 1:
            if( fstyle->boxtype == 'G' || fstyle->boxtype == 'B' || fstyle->boxtype == 'O')
                padd("style:run-through", sXML_CDATA, "background");
            padd("style:wrap", sXML_CDATA, "run-through");
            break;
        case 2:
            padd("style:wrap", sXML_CDATA, "dynamic");
            break;
    }
    if (fstyle->anchor_type == CHAR_ANCHOR)
    {
        padd("style:vertical-pos", sXML_CDATA, "top");
        padd("style:vertical-rel", sXML_CDATA, "baseline");
        padd("style:horizontal-pos", sXML_CDATA, "center");
        padd("style:horizontal-rel", sXML_CDATA, "paragraph");
    }
    else
    {

        switch (-(fstyle->xpos))
        {
            case 2:
                padd("style:horizontal-pos", sXML_CDATA, "right");
                break;
            case 3:
                padd("style:horizontal-pos", sXML_CDATA, "center");
                break;
            case 1:
            default:
                padd("style:horizontal-pos", sXML_CDATA, "from-left");
                break;
        }
        switch (-(fstyle->ypos))
        {
            case 2:
                padd("style:vertical-pos", sXML_CDATA, "bottom");
                break;
            case 3:
                padd("style:vertical-pos", sXML_CDATA, "middle");
                break;
            case 1:
            default:
                padd("style:vertical-pos", sXML_CDATA, "from-top");
                break;
        }
        if ( fstyle->anchor_type == PARA_ANCHOR )
        {
            padd("style:vertical-rel", sXML_CDATA, "paragraph");
            padd("style:horizontal-rel", sXML_CDATA, "paragraph");
        }
        else
        {
            padd("style:vertical-rel", sXML_CDATA, "page-content");
            padd("style:horizontal-rel", sXML_CDATA, "page-content");
        }
    }
    if( fstyle->boxtype == 'X' || fstyle->boxtype == 'B' )
    {
        Cell *cell = static_cast<Cell *>(fstyle->cell);
        if(cell->linetype[0] == cell->linetype[1] &&
            cell->linetype[0] == cell->linetype[2] &&
            cell->linetype[0] == cell->linetype[3])
        {
            switch( cell->linetype[0] )
            {
                case 0:
                          padd("fo:border", sXML_CDATA, "none");
                    break;
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    padd("fo:border", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    padd("fo:border", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    padd("style:border-line-width", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    padd("fo:border", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
        }
        else
        {
            switch( cell->linetype[0] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    padd("fo:border-left", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    padd("fo:border-left", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    padd("style:border-line-width-left", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    padd("fo:border-left", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
            switch( cell->linetype[1] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    padd("fo:border-right", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    padd("fo:border-right", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    padd("style:border-line-width-right", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    padd("fo:border-right", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
            switch( cell->linetype[2] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    padd("fo:border-top", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    padd("fo:border-top", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    padd("style:border-line-width-top", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    padd("fo:border-top", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
            switch( cell->linetype[3] )
            {
                case 1:                           /* A thin solid line */
                case 3:                           /* Dotted line -> LibreOffice, there is no dotted line */
                    padd("fo:border-bottom", sXML_CDATA,"0.002cm solid #000000");
                    break;
                case 2:                           /* Bold lines */
                    padd("fo:border-bottom", sXML_CDATA,"0.035cm solid #000000");
                    break;
                case 4:                           /* Double line */
                    padd("style:border-line-width-bottom", sXML_CDATA,"0.002cm 0.035cm 0.002cm");
                    padd("fo:border-bottom", sXML_CDATA,"0.039cm double #000000");
                    break;
            }
        }

          if( cell->linetype[0] == 0 && cell->linetype[1] == 0 &&
                  cell->linetype[2] == 0 && cell->linetype[3] == 0 ){
              padd("fo:padding", sXML_CDATA,"0mm");
          }
          else{
              padd("fo:padding-left", sXML_CDATA,
                      Double2Str(WTMM(fstyle->margin[1][0])) + "mm");
              padd("fo:padding-right", sXML_CDATA,
                      Double2Str(WTMM(fstyle->margin[1][1])) + "mm");
              padd("fo:padding-top", sXML_CDATA,
                      Double2Str(WTMM(fstyle->margin[1][2])) + "mm");
              padd("fo:padding-bottom", sXML_CDATA,
                      Double2Str(WTMM(fstyle->margin[1][3])) + "mm");
          }
        if(cell->shade != 0)
            padd("fo:background-color", sXML_CDATA,
            ascii(hcolor2str(
                sal::static_int_cast<uchar>(cell->color),
                sal::static_int_cast<uchar>(cell->shade),
                buf)));
    }
    else if( fstyle->boxtype == 'E' )
     {
          padd("fo:padding", sXML_CDATA,"0mm");
     }
    else if( fstyle->boxtype == 'L' )
    {
        padd( "style:border-line-width-bottom", sXML_CDATA, "0.02mm 0.35mm 0.02mm");
        padd("fo:border-bottom", sXML_CDATA,"0.039cm double #808080");
    }
    else if( fstyle->boxtype == 'G' )
    {
         if( fstyle->margin[1][0] || fstyle->margin[1][1] || fstyle->margin[1][2] || fstyle->margin[1][3] ){
             OUString clip = "rect(";
             clip += Double2Str(WTMM(-fstyle->margin[1][0]) ) + "mm ";
             clip += Double2Str(WTMM(-fstyle->margin[1][1]) ) + "mm ";
             clip += Double2Str(WTMM(-fstyle->margin[1][2]) ) + "mm ";
             clip += Double2Str(WTMM(-fstyle->margin[1][3]) ) + "mm)";
             padd("style:mirror", sXML_CDATA, "none");
             padd("fo:clip", sXML_CDATA, clip);
         }
        char *cell = static_cast<char *>(fstyle->cell);
        padd("draw:luminance", sXML_CDATA,
            ascii(Int2Str(cell[0], "%d%%", buf)));
        padd("draw:contrast", sXML_CDATA,
            ascii(Int2Str(cell[1], "%d%%", buf)));
        if( cell[2] == 0 )
            padd("draw:color-mode", sXML_CDATA, "standard");
        else if( cell[2] == 1 )
            padd("draw:color-mode", sXML_CDATA, "greyscale");
        else if( cell[2] == 2 )
            padd("draw:color-mode", sXML_CDATA, "mono");

    }
    rstartEl("style:properties", rList);
    pList->clear();
    rendEl("style:properties");
    rendEl("style:style");
}


char *HwpReader::getTStyleName(int index, char *_buf)
{
    return Int2Str(index, "T%d", _buf);
}


char *HwpReader::getPStyleName(int index, char *_buf)
{
    return Int2Str(index, "P%d", _buf);
}


void HwpReader::makeChars(hchar_string & rStr)
{
    rchars(reinterpret_cast<sal_Unicode const *>(rStr.c_str()));
    rStr.clear();
}


/**
 * If no special characters in the paragraph and all characters use the same CharShape
 */
void HwpReader::make_text_p0(HWPPara * para, bool bParaStart)
{
    hchar_string str;
    int n;
    int res;
     hchar dest[3];
    unsigned char firstspace = 0;
    if( !bParaStart)
    {
        padd("text:style-name", sXML_CDATA,
            ascii(getPStyleName(para->GetParaShape().index, buf)));
        rstartEl("text:p", rList);
        pList->clear();
    }
    if( d->bFirstPara && d->bInBody )
    {
        strcpy(
            buf,
            "[\xEB\xAC\xB8\xEC\x84\x9C\xEC\x9D\x98 \xEC\xB2\x98\xEC\x9D\x8C]");
            // U+BB38 HANGUL SYLLABLE MUN, U+C11C HANGUL SYLLABLE SEO,
            // U+C758 HANGUL SYLLABLE YI, U+CC98 HANGUL SYLLABLE CEO,
            // U+C74C HANGUL SYLLABLE EUM: "Begin of Document"
        padd("text:name", sXML_CDATA, OUString(buf, strlen(buf), RTL_TEXTENCODING_UTF8));
        rstartEl("text:bookmark", rList);
        pList->clear();
        rendEl("text:bookmark");
        d->bFirstPara = false;
    }
    if( d->bInHeader )
    {
        makeShowPageNum();
        d->bInHeader = false;
    }
    padd("text:style-name", sXML_CDATA,
        ascii(getTStyleName(para->cshape.index, buf)));
    rstartEl("text:span", rList);
    pList->clear();

    for (n = 0; n < para->nch && para->hhstr[n]->hh;
        n += para->hhstr[n]->WSize())
    {
        if (para->hhstr[n]->hh == CH_SPACE && !firstspace)
        {
            makeChars(str);
            rstartEl("text:s", rList);
            rendEl("text:s");
        }
        else if (para->hhstr[n]->hh == CH_END_PARA)
        {
            makeChars(str);
            rendEl("text:span");
            rendEl("text:p");
            break;
        }
        else
        {
            if (para->hhstr[n]->hh == CH_SPACE)
                firstspace = 0;
            else
                firstspace = 1;
            res = hcharconv(para->hhstr[n]->hh, dest, UNICODE);
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
    int n;
    int res;
     hchar dest[3];
    int curr = para->cshape.index;
    unsigned char firstspace = 0;

    if( !bParaStart )
    {
        padd("text:style-name", sXML_CDATA,
            ascii(getPStyleName(para->GetParaShape().index, buf)));
        rstartEl("text:p", rList);
        pList->clear();
    }
    if( d->bFirstPara && d->bInBody )
    {
/* for HWP's Bookmark */
        strcpy(
            buf,
            "[\xEB\xAC\xB8\xEC\x84\x9C\xEC\x9D\x98 \xEC\xB2\x98\xEC\x9D\x8C]");
            // U+BB38 HANGUL SYLLABLE MUN, U+C11C HANGUL SYLLABLE SEO,
            // U+C758 HANGUL SYLLABLE YI, U+CC98 HANGUL SYLLABLE CEO,
            // U+C74C HANGUL SYLLABLE EUM: "Begin of Document"
        padd("text:name", sXML_CDATA, OUString(buf, strlen(buf), RTL_TEXTENCODING_UTF8));
        rstartEl("text:bookmark", rList);
        pList->clear();
        rendEl("text:bookmark");
        d->bFirstPara = false;
    }
    if( d->bInHeader )
    {
        makeShowPageNum();
        d->bInHeader = false;
    }
    padd("text:style-name", sXML_CDATA,
        ascii(getTStyleName(curr, buf)));
    rstartEl("text:span", rList);
    pList->clear();

    for (n = 0; n < para->nch && para->hhstr[n]->hh;
        n += para->hhstr[n]->WSize())
    {
        if (para->GetCharShape(n)->index != curr)
        {
            makeChars(str);
            rendEl("text:span");
            curr = para->GetCharShape(n)->index;
            padd("text:style-name", sXML_CDATA,
                ascii(getTStyleName(curr, buf)));
            rstartEl("text:span", rList);
            pList->clear();
        }
        if (para->hhstr[n]->hh == CH_SPACE && !firstspace)
        {
            makeChars(str);
            rstartEl("text:s", rList);
            rendEl("text:s");
        }
        else if (para->hhstr[n]->hh == CH_END_PARA)
        {
            makeChars(str);
            rendEl("text:span");
            rendEl("text:p");
            break;
        }
        else
        {
              if( para->hhstr[n]->hh < CH_SPACE )
                  continue;
            if (para->hhstr[n]->hh == CH_SPACE)
                firstspace = 0;
            else
                firstspace = 1;
            res = hcharconv(para->hhstr[n]->hh, dest, UNICODE);
            for( int j = 0 ; j < res; j++ )
            {
                str.push_back(dest[j]);
            }
        }
    }
}


/**
 * Special characters are in the paragraph and characters use different CharShapes
 */
void HwpReader::make_text_p3(HWPPara * para,bool bParaStart)
{
    hchar_string str;
    int n, res;
     hchar dest[3];
    unsigned char firstspace = 0;
    bool pstart = bParaStart;
    bool tstart = false;
    bool infield = false;
    int curr;
    if( d->bFirstPara && d->bInBody )
    {
        if ( !pstart ) {
            STARTP;
        }
        strcpy(
            buf,
            "[\xEB\xAC\xB8\xEC\x84\x9C\xEC\x9D\x98 \xEC\xB2\x98\xEC\x9D\x8C]");
            // U+BB38 HANGUL SYLLABLE MUN, U+C11C HANGUL SYLLABLE SEO,
            // U+C758 HANGUL SYLLABLE YI, U+CC98 HANGUL SYLLABLE CEO,
            // U+C74C HANGUL SYLLABLE EUM: "Begin of Document"
        padd("text:name", sXML_CDATA, OUString(buf, strlen(buf), RTL_TEXTENCODING_UTF8));
        rstartEl("text:bookmark", rList);
        pList->clear();
        rendEl("text:bookmark");
        d->bFirstPara = false;
    }
    if( d->bInHeader )
    {
        if ( !pstart ) {
            STARTP;
        }
        makeShowPageNum();
        d->bInHeader = false;
    }

    for (n = 0; n < para->nch && para->hhstr[n]->hh;
        n += para->hhstr[n]->WSize())
    {
        if( para->hhstr[n]->hh == CH_END_PARA )
        {
            if (str.size() > 0)
            {
                if( !pstart ){ STARTP;}
                if( !tstart ){ STARTT;}
                makeChars(str);
            }
            if( tstart ){ ENDT;}
            if( !pstart ){ STARTP;}
            if( pstart ){ ENDP;}
            break;
        }
        else if( para->hhstr[n]->hh == CH_SPACE  && !firstspace)
        {
            if( !pstart ) {STARTP;}
            if( !tstart ) {STARTT;}
            makeChars(str);
            rstartEl("text:s", rList);
            pList->clear();
            rendEl("text:s");
        }
        else if ( para->hhstr[n]->hh >= CH_SPACE )
        {
            if( n > 0 )
                if( para->GetCharShape(n)->index != para->GetCharShape(n-1)->index && !infield )
                    {
                         if( !pstart ) {STARTP;}
                         if( !tstart ) {STARTT;}
                         makeChars(str);
                         ENDT;
                    }
            if( para->hhstr[n]->hh == CH_SPACE )
                firstspace = 0;
            else
                firstspace = 1;
            res = hcharconv(para->hhstr[n]->hh, dest, UNICODE);
            for( int j = 0 ; j < res; j++ )
            {
                str.push_back(dest[j]);
            }
        }
        else if (para->hhstr[n]->hh == CH_FIELD)
        {
            FieldCode *hbox = static_cast<FieldCode *>(para->hhstr[n]);
            if( hbox->location_info == 1)
            {
                if( !pstart ) {STARTP;}
                if( !tstart ) {STARTT;}
                makeChars(str);
                firstspace = 1;
                     if( hbox->type[0] == 4 && hbox->type[1] == 0 )
                     {
                         field = hbox->str3;
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
                         field = nullptr;
                     }
                infield = false;
                str.clear();
            }
        }
        else
        {
            switch (para->hhstr[n]->hh)
            {
                case CH_BOOKMARK:
                    if( !pstart ) {STARTP;}
                    if( !tstart ) {STARTT;}
                    makeChars(str);
                    makeBookmark(static_cast<Bookmark *>(para->hhstr[n]));
                    break;
                case CH_DATE_FORM:                // 7
                    break;
                case CH_DATE_CODE:                // 8
                    if( !pstart ) {STARTP;}
                    if( !tstart ) {STARTT;}
                    makeChars(str);
                    makeDateCode(static_cast<DateCode *>(para->hhstr[n]));
                    break;
                case CH_TAB:                      // 9
                    if( !pstart ) {STARTP;}
                    if (str.size() > 0)
                    {
                        if( !tstart ) {STARTT;}
                        makeChars(str);
                    }
                    makeTab(static_cast<Tab *>(para->hhstr[n]));
                    break;
                case CH_TEXT_BOX:                 /* 10 - ordered by Table/text box/formula/button/hypertext */
                {
/* produce tables first, and treat formula as being in text:p. */
                    TxtBox *hbox = static_cast<TxtBox *>(para->hhstr[n]);

                    if( hbox->style.anchor_type == 0 )
                    {
                        if( !pstart ) {STARTP;}
                        if( !tstart ) {STARTT;}
                        makeChars(str);
                    }
                    else
                    {
                        if( !pstart ) {STARTP;}
                        if (str.size() > 0)
                        {
                            if( !tstart ) {STARTT;}
                            makeChars(str);
                        }
                        if( tstart ) {ENDT;}
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
                    Picture *hbox = static_cast<Picture *>(para->hhstr[n]);
                    if( hbox->style.anchor_type == 0 )
                    {
                        if( !pstart ) {STARTP;}
                        if( !tstart ) {STARTT;}
                        makeChars(str);
                    }
                    else
                    {
                        if( !pstart ) {STARTP;}
                        if (str.size() > 0)
                        {
                            if( !tstart ) {STARTT;}
                            makeChars(str);
                        }
                        if( tstart ) {ENDT;}
                    }
                    makePicture(hbox);
                    break;
                }
                case CH_LINE:                     // 14
                {
                    Line *hbox = static_cast<Line *>(para->hhstr[n]);
                    if (str.size() > 0)
                    {
                        if( !pstart ) {STARTP;}
                        if( !tstart ) {STARTT;}
                        makeChars(str);
                    }
                    if( tstart ) {ENDT;}
                    if( pstart ) {ENDP;}
                    makeLine(hbox);
                    pstart = true;
                    break;
                }
                case CH_HIDDEN:                   // 15
                    if( !pstart ) {STARTP;}
                    if( !tstart ) {STARTT;}
                    makeChars(str);
                    makeHidden(static_cast<Hidden *>(para->hhstr[n]));
                    break;
                case CH_FOOTNOTE:                 // 17
                    if( !pstart ) {STARTP;}
                    if( !tstart ) {STARTT;}
                    makeChars(str);
                    makeFootnote(static_cast<Footnote *>(para->hhstr[n]));
                    break;
                case CH_AUTO_NUM:                 // 18
                    if( !pstart ) {STARTP;}
                    if( !tstart ) {STARTT;}
                    makeChars(str);
                    makeAutoNum(static_cast<AutoNum *>(para->hhstr[n]));
                    break;
                case CH_NEW_NUM:                  // 19 -skip
                    break;
                case CH_PAGE_NUM_CTRL:            // 21
                    break;
                case CH_MAIL_MERGE:               // 22
                    if( !pstart ) {STARTP;}
                    if( !tstart ) {STARTT;}
                    makeChars(str);
                    makeMailMerge(static_cast<MailMerge *>(para->hhstr[n]));
                    break;
                case CH_COMPOSE:                  /* 23 - overlapping letters */
                    break;
                case CH_HYPHEN:                   // 24
                    break;
                case CH_TOC_MARK:                 /* 25 Need to fix below 3 */
                    if( !pstart ) {STARTP;}
                    if( !tstart ) {STARTT;}
                    makeChars(str);
                    break;
                case CH_INDEX_MARK:               // 26
                    if( !pstart ) {STARTP;}
                    if( !tstart ) {STARTT;}
                    makeChars(str);
                    break;
                case CH_OUTLINE:                  // 28
                    if( !pstart ) {STARTP;}
                    if( !tstart ) {STARTT;}
                    makeChars(str);
                    makeOutline(static_cast<Outline *>(para->hhstr[n]));
                    break;
                     case CH_FIXED_SPACE:
                     case CH_KEEP_SPACE:
                          str.push_back(0x0020);
                          break;
                }
        }
    }
}


void HwpReader::makeFieldCode(hchar_string & rStr, FieldCode *hbox)
{
/* Push frame */
    if( hbox->type[0] == 4 && hbox->type[1] == 0 )
    {
        padd("text:placeholder-type", sXML_CDATA, "text");
          if( field )
              padd("text:description", sXML_CDATA, reinterpret_cast<sal_Unicode const *>(hconv(field)));
        rstartEl( "text:placeholder", rList);
        pList->clear();
        rchars( reinterpret_cast<sal_Unicode const *>(rStr.c_str()) );
        rendEl( "text:placeholder" );
    }
/* Document Summary */
    else if( hbox->type[0] == 3 && hbox->type[1] == 0 )
    {
        if (OUString(reinterpret_cast<sal_Unicode const *>(hconv(hbox->str3))) == "title")
        {
            rstartEl( "text:title", rList );
            rchars( reinterpret_cast<sal_Unicode const *>(hconv(hbox->str2)) );
            rendEl( "text:title" );
        }
        else if (OUString(reinterpret_cast<sal_Unicode const *>(hconv(hbox->str3))) == "subject")
        {
            rstartEl( "text:subject", rList );
            rchars( reinterpret_cast<sal_Unicode const *>(hconv(hbox->str2)) );
            rendEl( "text:subject" );
        }
        else if (OUString(reinterpret_cast<sal_Unicode const *>(hconv(hbox->str3))) == "author")
        {
            rstartEl( "text:author-name", rList );
            rchars( reinterpret_cast<sal_Unicode const *>(hconv(hbox->str2)) );
            rendEl( "text:author-name" );
        }
        else if (OUString(reinterpret_cast<sal_Unicode const *>(hconv(hbox->str3))) == "keywords")
        {
            rstartEl( "text:keywords", rList );
            rchars( reinterpret_cast<sal_Unicode const *>(hconv(hbox->str2)) );
            rendEl( "text:keywords" );
        }
    }
/* Personal Information */
    else if( hbox->type[0] == 3 && hbox->type[1] == 1 )
    {
        if (OUString(reinterpret_cast<sal_Unicode const *>(hconv(hbox->str3))) == "User")
        {
            rstartEl( "text:sender-lastname", rList );
            rchars( reinterpret_cast<sal_Unicode const *>(hconv(hbox->str2)) );
            rendEl( "text:sender-lastname" );
        }
        else if (OUString(reinterpret_cast<sal_Unicode const *>(hconv(hbox->str3))) == "Company")
        {
            rstartEl( "text:sender-company", rList );
            rchars( reinterpret_cast<sal_Unicode const *>(hconv(hbox->str2)) );
            rendEl( "text:sender-company" );
        }
        else if (OUString(reinterpret_cast<sal_Unicode const *>(hconv(hbox->str3))) == "Position")
        {
            rstartEl( "text:sender-title", rList );
            rchars( reinterpret_cast<sal_Unicode const *>(hconv(hbox->str2)) );
            rendEl( "text:sender-title" );
        }
        else if (OUString(reinterpret_cast<sal_Unicode const *>(hconv(hbox->str3))) == "Division")
        {
            rstartEl( "text:sender-position", rList );
            rchars( reinterpret_cast<sal_Unicode const *>(hconv(hbox->str2)) );
            rendEl( "text:sender-position" );
        }
        else if (OUString(reinterpret_cast<sal_Unicode const *>(hconv(hbox->str3))) == "Fax")
        {
            rstartEl( "text:sender-fax", rList );
            rchars( reinterpret_cast<sal_Unicode const *>(hconv(hbox->str2)) );
            rendEl( "text:sender-fax" );
        }
        else if (OUString(reinterpret_cast<sal_Unicode const *>(hconv(hbox->str3))) == "Pager")
        {
            rstartEl( "text:phone-private", rList );
            rchars( reinterpret_cast<sal_Unicode const *>(hconv(hbox->str2)) );
            rendEl( "text:phone-private" );
        }
        else if (OUString(reinterpret_cast<sal_Unicode const *>(hconv(hbox->str3))) == "E-mail")
        {
            rstartEl( "text:sender-email", rList );
            rchars( reinterpret_cast<sal_Unicode const *>(hconv(hbox->str2)) );
            rendEl( "text:sender-email" );
        }
        else if (OUString(reinterpret_cast<sal_Unicode const *>(hconv(hbox->str3))) == "Zipcode(office)")
        {
            rstartEl( "text:sender-postal-code", rList );
            rchars( reinterpret_cast<sal_Unicode const *>(hconv(hbox->str2)) );
            rendEl( "text:sender-postal-code" );
        }
        else if (OUString(reinterpret_cast<sal_Unicode const *>(hconv(hbox->str3))) == "Phone(office)")
        {
            rstartEl( "text:sender-phone-work", rList );
            rchars( reinterpret_cast<sal_Unicode const *>(hconv(hbox->str2)) );
            rendEl( "text:sender-phone-work" );
        }
        else if (OUString(reinterpret_cast<sal_Unicode const *>(hconv(hbox->str3))) == "Address(office)")
        {
            rstartEl( "text:sender-street", rList );
            rchars( reinterpret_cast<sal_Unicode const *>(hconv(hbox->str2)) );
            rendEl( "text:sender-street" );
        }

    }
    else if( hbox->type[0] == 3 && hbox->type[1] == 2 ) /* creation date */
     {
         if( hbox->m_pDate )
             padd("style:data-style-name", sXML_CDATA,
                     ascii(Int2Str(hbox->m_pDate->key, "N%d", buf)));
            rstartEl( "text:creation-date", rList );
            pList->clear();
            rchars( reinterpret_cast<sal_Unicode const *>(hconv(hbox->str2)) );
            rendEl( "text:creation-date" );
     }
}


/**
 * Completed
 * In LibreOffice, refer bookmarks as reference, but hwp doesn't have the sort of feature.
 */
void HwpReader::makeBookmark(Bookmark * hbox)
{
    if (hbox->type == 0)
    {
        padd("text:name", sXML_CDATA, reinterpret_cast<sal_Unicode const *>(hconv(hbox->id)));
        rstartEl("text:bookmark", rList);
        pList->clear();
        rendEl("text:bookmark");
    }
    else if (hbox->type == 1)                     /* Block bookmarks days begin and end there if */
    {
        padd("text:name", sXML_CDATA, reinterpret_cast<sal_Unicode const *>(hconv(hbox->id)));
        rstartEl("text:bookmark-start", rList);
        pList->clear();
        rendEl("text:bookmark-start");
    }
    else if (hbox->type == 2)
    {
        padd("text:name", sXML_CDATA, reinterpret_cast<sal_Unicode const *>(hconv(hbox->id)));
        rstartEl("text:bookmark-end", rList);
        pList->clear();
        rendEl("text:bookmark-end");
    }
}


#include "datecode.h"

void HwpReader::makeDateFormat(DateCode * hbox)
{
    padd("style:name", sXML_CDATA,
        ascii(Int2Str(hbox->key, "N%d", buf)));
    padd("style:family", sXML_CDATA,"data-style");
    padd("number:language", sXML_CDATA,"ko");
    padd("number:country", sXML_CDATA,"KR");

    rstartEl("number:date-style", rList);
    pList->clear();

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
                padd("number:style", sXML_CDATA, "long");
                rstartEl("number:year", rList);
                pList->clear();
                rendEl("number:year");
                break;
            case '!':
                rstartEl("number:year", rList);
                pList->clear();
                rendEl("number:year");
                break;
            case '2':
                if( add_zero )
                    padd("number:style", sXML_CDATA, "long");
                rstartEl("number:month", rList);
                pList->clear();
                rendEl("number:month");
                break;
            case '@':
                padd("number:textual", sXML_CDATA, "true");
                rstartEl("number:month", rList);
                pList->clear();
                rendEl("number:month");
                break;
            case '*':
                padd("number:textual", sXML_CDATA, "true");
                padd("number:style", sXML_CDATA, "long");
                rstartEl("number:month", rList);
                pList->clear();
                rendEl("number:month");
                break;
            case '3':
                if( add_zero )
                    padd("number:style", sXML_CDATA, "long");
                rstartEl("number:day", rList);
                pList->clear();
                rendEl("number:day");
                break;
            case '#':
                if( add_zero )
                    padd("number:style", sXML_CDATA, "long");
                rstartEl("number:day", rList);
                pList->clear();
                rendEl("number:day");
                switch( hbox->date[DateCode::DAY]  % 10)
                {
                    case 1:
                        rstartEl("number:text", rList);
                        rchars("st");
                        rendEl("number:text");
                        break;
                    case 2:
                        rstartEl("number:text", rList);
                        rchars("nd");
                        rendEl("number:text");
                        break;
                    case 3:
                        rstartEl("number:text", rList);
                        rchars("rd");
                        rendEl("number:text");
                        break;
                    default:
                        rstartEl("number:text", rList);
                        rchars("th");
                        rendEl("number:text");
                        break;
                }
                break;
            case '4':
            case '$':
                if( add_zero )
                    padd("number:style", sXML_CDATA, "long");
                rstartEl("number:hours", rList);
                pList->clear();
                rendEl("number:hours");
                break;
            case '5':
            case '%':
                if( add_zero )
                    padd("number:style", sXML_CDATA, "long");
                rstartEl("number:minutes", rList);
                pList->clear();
                rendEl("number:minutes");
                break;
            case '_':
                padd("number:style", sXML_CDATA, "long");
                //fall-through
            case '6':
            case '^':
                rstartEl("number:day-of-week", rList);
                pList->clear();
                rendEl("number:day-of-week");
                break;
            case '7':
            case '&':
            case '+':
                rstartEl("number:am-pm", rList);
                pList->clear();
                rendEl("number:am-pm");
                break;
            case '~':                             // Chinese Locale
                break;
            default:
                hchar sbuf[2];
                sbuf[0] = *fmt;
                sbuf[1] = 0;
                rstartEl("number:text", rList);
                rchars(reinterpret_cast<sal_Unicode const *>(hconv(sbuf)));
                rendEl("number:text");
                break;
        }
    }
    pList->clear();
    rendEl("number:date-style");
}


void HwpReader::makeDateCode(DateCode * hbox)
{
    padd("style:data-style-name", sXML_CDATA,
        ascii(Int2Str(hbox->key, "N%d", buf)));
    rstartEl( "text:date", rList );
    pList->clear();
    hchar_string const boxstr = hbox->GetString();
    rchars(reinterpret_cast<sal_Unicode const *>(hconv(boxstr.c_str())));
    rendEl( "text:date" );
}


void HwpReader::makeTab(Tab *  )                  /*hbox */
{
    rstartEl("text:tab-stop", rList);
    rendEl("text:tab-stop");
}


void HwpReader::makeTable(TxtBox * hbox)
{
    padd("table:name", sXML_CDATA,
        ascii(Int2Str(hbox->style.boxnum, "Table%d", buf)));
    padd("table:style-name", sXML_CDATA,
        ascii(Int2Str(hbox->style.boxnum, "Table%d", buf)));
    rstartEl("table:table", rList);
    pList->clear();

    Table *tbl = hbox->m_pTable;
// column
    for (size_t i = 0 ; i < tbl->columns.nCount -1 ; i++)
    {
        sprintf(buf,"Table%d.%c",hbox->style.boxnum, static_cast<char>('A'+i));
        padd("table:style-name", sXML_CDATA, ascii( buf ));
        rstartEl("table:table-column", rList);
        pList->clear();
        rendEl("table:table-column");
    }

// cell
    int j = -1, k = -1;
    for (std::list<TCell*>::iterator it = tbl->cells.begin(), aEnd = tbl->cells.end(); it != aEnd; ++it)
    {
        TCell *tcell = *it;
        if( tcell->nRowIndex > j )
        {
            if( j > k )
            {
                rendEl("table:table-row");
                k = j;
            }
// row
            sprintf(buf,"Table%d.row%d",hbox->style.boxnum, tcell->nRowIndex + 1);
            padd("table:style-name", sXML_CDATA, ascii( buf ));
            rstartEl("table:table-row", rList);
            pList->clear();
            j = tcell->nRowIndex;
        }

        sprintf(buf,"Table%d.%c%d",hbox->style.boxnum, 'A'+ tcell->nColumnIndex, tcell->nRowIndex +1);
        padd("table:style-name", sXML_CDATA, ascii( buf ));
        if( tcell->nColumnSpan > 1 )
            padd("table:number-columns-spanned", sXML_CDATA,
                ascii(Int2Str(tcell->nColumnSpan, "%d", buf)));
        if( tcell->nRowSpan > 1 )
            padd("table:number-rows-spanned", sXML_CDATA,
                ascii(Int2Str(tcell->nRowSpan, "%d", buf)));
        padd("table:value-type", sXML_CDATA,"string");
        if( tcell->pCell->protect )
            padd("table:protected", sXML_CDATA,"true");
        rstartEl("table:table-cell", rList);
        pList->clear();
        parsePara(hbox->plists[tcell->pCell->key].front());
        rendEl("table:table-cell");
    }
    rendEl("table:table-row");
    rendEl("table:table");
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
        padd("draw:style-name", sXML_CDATA,
            ascii(Int2Str(hbox->style.boxnum, "CapBox%d", buf)));
        padd("draw:name", sXML_CDATA,
            ascii(Int2Str(hbox->style.boxnum, "CaptionBox%d", buf)));
         padd("draw:z-index", sXML_CDATA,
              ascii(Int2Str(hbox->zorder, "%d", buf)));
        switch (hbox->style.anchor_type)
        {
            case CHAR_ANCHOR:
                padd("text:anchor-type", sXML_CDATA, "as-char");
                break;
            case PARA_ANCHOR:
                padd("text:anchor-type", sXML_CDATA, "paragraph");
                break;
            case PAGE_ANCHOR:
            case PAPER_ANCHOR:
            {
                padd("text:anchor-type", sXML_CDATA, "page");
                padd("text:anchor-page-number", sXML_CDATA,
                    ascii(Int2Str(hbox->pgno +1, "%d", buf)));
                break;
            }
        }
        if (hbox->style.anchor_type != CHAR_ANCHOR)
        {
            padd("svg:x", sXML_CDATA,
                Double2Str(WTMM( ( hbox->pgx + hbox->style.margin[0][0] ) )) + "mm");
            padd("svg:y", sXML_CDATA,
                Double2Str(WTMM( ( hbox->pgy + hbox->style.margin[0][2] ) )) + "mm");
        }
        padd("svg:width", sXML_CDATA,
            Double2Str(WTMM(( hbox->box_xs + hbox->cap_xs) )) + "mm");
        padd("fo:min-height", sXML_CDATA,
            Double2Str(WTMM(( hbox->box_ys + hbox->cap_ys) )) + "mm");
        rstartEl("draw:text-box", rList);
        pList->clear();
        if( hbox->cap_pos % 2 )                   /* The caption is on the top */
        {
            parsePara(hbox->caption.front());
        }
        padd( "text:style-name", sXML_CDATA, "Standard");
        rstartEl("text:p", rList);
        pList->clear();
    }
     else{
         padd("draw:z-index", sXML_CDATA,
              ascii(Int2Str(hbox->zorder, "%d", buf)));
     }

    padd("draw:style-name", sXML_CDATA,
        ascii(Int2Str(hbox->style.boxnum, "Txtbox%d", buf)));
    padd("draw:name", sXML_CDATA,
        ascii(Int2Str(hbox->style.boxnum, "Frame%d", buf)));

    if( hbox->style.cap_len <= 0 || hbox->type != TXT_TYPE )
    {
        int x = 0;
        int y = 0;
        switch (hbox->style.anchor_type)
        {
            case CHAR_ANCHOR:
                padd("text:anchor-type", sXML_CDATA, "as-char");
                break;
            case PARA_ANCHOR:
                padd("text:anchor-type", sXML_CDATA, "paragraph");
                break;
            case PAGE_ANCHOR:
            case PAPER_ANCHOR:
            {
                padd("text:anchor-type", sXML_CDATA, "page");
                padd("text:anchor-page-number", sXML_CDATA,
                    ascii(Int2Str(hbox->pgno +1, "%d", buf)));
                break;
            }
        }
        if( hbox->style.anchor_type != CHAR_ANCHOR )
        {
            x += hbox->style.margin[0][0];
            y += hbox->style.margin[0][2];
        }
        padd("svg:x", sXML_CDATA,
            Double2Str(WTMM( hbox->pgx + x )) + "mm");
        padd("svg:y", sXML_CDATA,
            Double2Str(WTMM( hbox->pgy + y )) + "mm");
    }
    else
    {
        padd("text:anchor-type", sXML_CDATA, "as-char");
        padd("svg:y", sXML_CDATA, "0cm");
    }
    padd("svg:width", sXML_CDATA,
        Double2Str(WTMM( hbox->box_xs )) + "mm");
    if( hbox->style.cap_len > 0 && hbox->type != TXT_TYPE)
        padd("fo:min-height", sXML_CDATA,
            Double2Str(WTMM( hbox->box_ys + hbox->cap_ys)) + "mm");
    else
        padd("svg:height", sXML_CDATA,
            Double2Str(WTMM(hbox->box_ys )) + "mm");

    if( hbox->type != EQU_TYPE )
    {
        rstartEl("draw:text-box", rList);
        pList->clear();
/* If captions are present and it is on the top */
        if( hbox->style.cap_len > 0 && (hbox->cap_pos % 2) && hbox->type == TBL_TYPE )
        {
            parsePara(hbox->caption.front());
        }
        if( hbox->type == TBL_TYPE)               // Is Table
        {
            makeTable(hbox);
        }
        else                                      // Is TextBox
        {
            parsePara(hbox->plists[0].front());
        }
/* If captions are present and it is on the bottom */
        if( hbox->style.cap_len > 0 && !(hbox->cap_pos % 2) && hbox->type == TBL_TYPE)
        {
            parsePara(hbox->caption.front());
        }
        rendEl("draw:text-box");
// Caption exist and it is text-box
        if( hbox->style.cap_len > 0 && hbox->type == TXT_TYPE)
        {
            rendEl( "text:p");
            if( !(hbox->cap_pos % 2))
            {
                parsePara(hbox->caption.front());
            }
            rendEl( "draw:text-box");
        }
    }
    else                                          // is Formula
    {
        rstartEl("draw:object", rList);
        pList->clear();
        makeFormula(hbox);
        rendEl("draw:object");
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
    CharShape *cshape = nullptr;

    int n, c, res;
     hchar dest[3];
    size_t l = 0;

    pPar = hbox->plists[0].front();
    while( pPar )
    {
        for( n = 0; n < pPar->nch && pPar->hhstr[n]->hh;
            n += pPar->hhstr[n]->WSize() )
        {
            if (!cshape)
                cshape = pPar->GetCharShape(n);
            if (l >= sizeof(mybuf)-7)
                break;
            res = hcharconv(pPar->hhstr[n]->hh, dest, UNICODE);
            for( int j = 0 ; j < res; j++ ){
                c = dest[j];
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

    Formula *form = new Formula(mybuf);
    form->setDocumentHandler(m_rxDocumentHandler);
    form->setAttributeListImpl(pList);
    form->parse();

    delete form;
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
              (uchar *) urltowin((char *)hypert->filename).c_str()).c_str());
#else
              reinterpret_cast<uchar const *>(urltounix(reinterpret_cast<char *>(hypert->filename)).c_str())).c_str());
#endif
          padd("xlink:type", sXML_CDATA, "simple");
          if (tmp.size() > 0 && strcmp(tmp.c_str(), "[HTML]")) {
              ::std::string tmp3(tmp2);
              tmp3.push_back('#');
              tmp3.append(tmp);
              padd("xlink:href", sXML_CDATA,
                  OUString(tmp3.c_str(), tmp3.size()+1, RTL_TEXTENCODING_EUC_KR));
          }
          else{
              padd("xlink:href", sXML_CDATA,
                  OUString(tmp2.c_str(), tmp2.size()+1, RTL_TEXTENCODING_EUC_KR));

          }
     }
    else
    {
        padd("xlink:type", sXML_CDATA, "simple");
        ::std::string tmp;
        tmp.push_back('#');
        tmp.append(hstr2ksstr(hypert->bookmark));
        padd("xlink:href", sXML_CDATA,
                OUString(tmp.c_str(), tmp.size()+1, RTL_TEXTENCODING_EUC_KR));
    }
    rstartEl("draw:a", rList);
    pList->clear();
    makeTextBox(hbox);
    rendEl("draw:a");
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
                padd("draw:style-name", sXML_CDATA,
                    ascii(Int2Str(hbox->style.boxnum, "CapBox%d", buf)));
                padd("draw:name", sXML_CDATA,
                    ascii(Int2Str(hbox->style.boxnum, "CaptionBox%d", buf)));
                     padd("draw:z-index", sXML_CDATA,
                    ascii(Int2Str(hbox->zorder, "%d", buf)));
                switch (hbox->style.anchor_type)
                {
                    case CHAR_ANCHOR:
                        padd("text:anchor-type", sXML_CDATA, "as-char");
                        break;
                    case PARA_ANCHOR:
                        padd("text:anchor-type", sXML_CDATA, "paragraph");
                        break;
                    case PAGE_ANCHOR:
                    case PAPER_ANCHOR:
                    {
                        padd("text:anchor-type", sXML_CDATA, "page");
                        padd("text:anchor-page-number", sXML_CDATA,
                            ascii(Int2Str(hbox->pgno +1, "%d", buf)));
                        break;
                    }
                }
                if (hbox->style.anchor_type != CHAR_ANCHOR)
                {
                    padd("svg:x", sXML_CDATA,
                        Double2Str(WTMM(  hbox->pgx + hbox->style.margin[0][0] )) + "mm");
                    padd("svg:y", sXML_CDATA,
                        Double2Str(WTMM(  hbox->pgy + hbox->style.margin[0][2] )) + "mm");
                }
                padd("svg:width", sXML_CDATA,
                    Double2Str(WTMM( hbox->box_xs + hbox->style.margin[1][0] + hbox->style.margin[1][1] )) + "mm");
                padd("fo:min-height", sXML_CDATA,
                    Double2Str(WTMM( hbox->box_ys + hbox->style.margin[1][2] + hbox->style.margin[1][3] + hbox->cap_ys )) + "mm");
                rstartEl("draw:text-box", rList);
                pList->clear();
                if( hbox->cap_pos % 2 )           /* Caption is on the top */
                {
                    parsePara(hbox->caption.front());
                }
                padd( "text:style-name", sXML_CDATA, "Standard");
                rstartEl("text:p", rList);
                pList->clear();
            }
            if( hbox->ishyper )
            {
                padd("xlink:type", sXML_CDATA, "simple");
#ifdef _WIN32
                if( hbox->follow[4] != 0 )
                    padd("xlink:href", sXML_CDATA, reinterpret_cast<sal_Unicode const *>(hconv(kstr2hstr(hbox->follow + 4).c_str())));
                else
                    padd("xlink:href", sXML_CDATA, reinterpret_cast<sal_Unicode const *>(hconv(kstr2hstr(hbox->follow + 5).c_str())));
#else
                if( hbox->follow[4] != 0 )
                    padd("xlink:href", sXML_CDATA,
                        reinterpret_cast<sal_Unicode const *>(hconv(kstr2hstr(reinterpret_cast<uchar const *>(urltounix(reinterpret_cast<char *>(hbox->follow + 4)).c_str())).c_str())));
                else
                    padd("xlink:href", sXML_CDATA,
                        reinterpret_cast<sal_Unicode const *>(hconv(kstr2hstr(reinterpret_cast<uchar const *>(urltounix(reinterpret_cast<char *>(hbox->follow + 5)).c_str())).c_str())));
#endif
                rstartEl("draw:a", rList);
                pList->clear();
            }
            padd("draw:style-name", sXML_CDATA,
                ascii(Int2Str(hbox->style.boxnum, "G%d", buf)));
            padd("draw:name", sXML_CDATA,
                ascii(Int2Str(hbox->style.boxnum, "Image%d", buf)));

            if( hbox->style.cap_len <= 0 )
            {
                     padd("draw:z-index", sXML_CDATA,
                    ascii(Int2Str(hbox->zorder, "%d", buf)));
                switch (hbox->style.anchor_type)
                {
                    case CHAR_ANCHOR:
                        padd("text:anchor-type", sXML_CDATA, "as-char");
                        break;
                    case PARA_ANCHOR:
                        padd("text:anchor-type", sXML_CDATA, "paragraph");
                        break;
                    case PAGE_ANCHOR:
                    case PAPER_ANCHOR:
                    {
                        padd("text:anchor-type", sXML_CDATA, "page");
                        padd("text:anchor-page-number", sXML_CDATA,
                            ascii(Int2Str(hbox->pgno +1, "%d", buf)));
                        break;
                    }
                }
                if (hbox->style.anchor_type != CHAR_ANCHOR)
                {
                    padd("svg:x", sXML_CDATA,
                        Double2Str(WTMM( hbox->pgx + hbox->style.margin[0][0] )) + "mm");
                    padd("svg:y", sXML_CDATA,
                        Double2Str(WTMM( hbox->pgy + hbox->style.margin[0][2] )) + "mm");
                }
            }
            else
            {
                padd("text:anchor-type", sXML_CDATA, "as-char");
                padd("svg:y", sXML_CDATA, "0cm");
            }
            padd("svg:width", sXML_CDATA,
                Double2Str(WTMM( hbox->box_xs + hbox->style.margin[1][0] + hbox->style.margin[1][1])) + "mm");
            padd("svg:height", sXML_CDATA,
                Double2Str(WTMM( hbox->box_ys + hbox->style.margin[1][2] + hbox->style.margin[1][3])) + "mm");

            if ( hbox->pictype == PICTYPE_FILE ){
#ifdef _WIN32
                sprintf(buf, "file:///%s", hbox->picinfo.picun.path );
                padd("xlink:href", sXML_CDATA, reinterpret_cast<sal_Unicode const *>(hconv(kstr2hstr((uchar *) buf).c_str())));
#else
                padd("xlink:href", sXML_CDATA,
                    reinterpret_cast<sal_Unicode const *>(hconv(kstr2hstr(reinterpret_cast<uchar const *>(urltounix(hbox->picinfo.picun.path).c_str())).c_str())));
#endif
                padd("xlink:type", sXML_CDATA, "simple");
                padd("xlink:show", sXML_CDATA, "embed");
                padd("xlink:actuate", sXML_CDATA, "onLoad");
            }

                if( hbox->pictype == PICTYPE_OLE )
                    rstartEl("draw:object-ole", rList);
                else
                    rstartEl("draw:image", rList);
            pList->clear();
            if (hbox->pictype == PICTYPE_EMBED || hbox->pictype == PICTYPE_OLE)
            {
                rstartEl("office:binary-data", rList);
                pList->clear();
                     if( hbox->pictype == PICTYPE_EMBED ){
                         EmPicture *emp = hwpfile.GetEmPicture(hbox);
                         if( emp )
                         {
                             std::shared_ptr<char> pStr(base64_encode_string( emp->data, emp->size ), Free<char>());
                             rchars(ascii(pStr.get()));
                         }
                     }
                     else{
                         if( hwpfile.oledata ){
#ifdef WIN32
                             LPSTORAGE srcsto;
                             LPUNKNOWN pObj;
                             wchar_t pathname[200];

                             MultiByteToWideChar(CP_ACP, 0, hbox->picinfo.picole.embname, -1, pathname, 200);
                             int rc = hwpfile.oledata->pis->OpenStorage(pathname, 0,
                                     STGM_READWRITE | STGM_SHARE_EXCLUSIVE | STGM_TRANSACTED, NULL, 0, &srcsto);
                             if (rc != S_OK) {
                                 rchars("");
                             }
                             else{
                                 rc = OleLoad(srcsto, IID_IUnknown, NULL, (LPVOID*)&pObj);
                                 if( rc != S_OK ){
                                     srcsto->Release();
                                     rchars("");
                                 }
                                 else{
                                     std::shared_ptr<char> pStr(base64_encode_string( (uchar *)pObj, strlen((char *)pObj)), Free<char>());
                                     rchars(ascii(pStr.get()));
                                     pObj->Release();
                                     srcsto->Release();
                                 }
                             }
#else
                             rchars("");
#endif
                         }
                     }
                rendEl("office:binary-data");
            }
                if( hbox->pictype == PICTYPE_OLE )
                    rendEl("draw:object-ole");
                else
                    rendEl("draw:image");
            if( hbox->ishyper )
            {
                rendEl("draw:a");
            }
            if( hbox->style.cap_len > 0 )
            {
                rendEl( "text:p");
                if( !(hbox->cap_pos % 2))         /* Caption is at the bottom, */
                {
                    parsePara(hbox->caption.front());
                }
                rendEl( "draw:text-box");
            }
            break;
        }
        case PICTYPE_DRAW:
              if( hbox->picinfo.picdraw.zorder > 0 )
                 padd("draw:z-index", sXML_CDATA,
                      ascii(Int2Str( hbox->picinfo.picdraw.zorder + 10000, "%d", buf)));
            makePictureDRAW( static_cast<HWPDrawingObject *>(hbox->picinfo.picdraw.hdo), hbox);
            break;
        case PICTYPE_UNKNOWN:
            break;
    }
}


#define DBL(x)  ((x) * (x))
void HwpReader::makePictureDRAW(HWPDrawingObject *drawobj, Picture * hbox)
{
    int x = hbox->pgx;
    int y = hbox->pgy;
    bool bIsRotate = false;

    while (drawobj)
    {
        padd("draw:style-name", sXML_CDATA,
            ascii(Int2Str(drawobj->index, "Draw%d", buf)));
        int a = 0;
        int b = 0;

        switch (hbox->style.anchor_type)
        {
            case CHAR_ANCHOR:
                padd("text:anchor-type", sXML_CDATA, "as-char");
                break;
            case PARA_ANCHOR:
                padd("text:anchor-type", sXML_CDATA, "paragraph");
                break;
            case PAGE_ANCHOR:
            case PAPER_ANCHOR:
            {
                HWPInfo& hwpinfo = hwpfile.GetHWPInfo();
                padd("text:anchor-type", sXML_CDATA, "page");
                padd("text:anchor-page-number", sXML_CDATA,
                    ascii(Int2Str(hbox->pgno +1, "%d", buf)));
                a = hwpinfo.paper.left_margin;
                b = hwpinfo.paper.top_margin + hwpinfo.paper.header_length;
                break;
            }
        }

        if (drawobj->type == HWPDO_CONTAINER)
        {
            rstartEl("draw:g", rList);
            pList->clear();
            makePictureDRAW(drawobj->child, hbox);
            rendEl("draw:g");
        }
        else
        {
                bIsRotate = false;
            if( (drawobj->property.flag & HWPDO_FLAG_ROTATION) &&
                    (drawobj->property.parall.pt[0].y != drawobj->property.parall.pt[1].y) &&
                    //(drawobj->type == HWPDO_RECT || drawobj->type == HWPDO_ADVANCED_ELLIPSE || drawobj->type == HWPDO_ADVANCED_ARC )
                    (drawobj->type == HWPDO_RECT || drawobj->type == HWPDO_ADVANCED_ELLIPSE )
                    )
            {

                     int i;
                ZZParall *pal = &drawobj->property.parall;

                     ZZPoint pt[3], r_pt[3];
                     for(i = 0 ; i < 3 ; i++ ){
                         pt[i].x = pal->pt[i].x - drawobj->property.rot_originx;
                         /* Convert to a physical coordinate */
                         pt[i].y = -(pal->pt[i].y - drawobj->property.rot_originy);
                     }

                double rotate, skewX ;

                     /* 2 - rotation angle calculation */
                     if( pt[1].x == pt[0].x ){
                         if( pt[1].y > pt[0].y )
                             rotate = PI/2;
                         else
                             rotate = -(PI/2);
                     }
                     else
                         rotate = atan((double)( pt[1].y - pt[0].y )/(pt[1].x - pt[0].x ));
                     if( pt[1].x < pt[0].x )
                         rotate += PI;

                     for( i = 0 ; i < 3 ; i++){
                         r_pt[i].x = (int)(pt[i].x * cos(-(rotate)) - pt[i].y * sin(-(rotate)));
                         r_pt[i].y = (int)(pt[i].y * cos(-(rotate)) + pt[i].x * sin(-(rotate)));
                     }

                     /* 4 - Calculation of reflex angle */
                     if( r_pt[2].y == r_pt[1].y )
                         skewX = 0;
                     else
                         skewX = atan((double)(r_pt[2].x - r_pt[1].x )/( r_pt[2].y - r_pt[1].y ));
                     if( skewX >= PI/2 )
                         skewX -= PI;
                     if( skewX <= -PI/2 )
                         skewX += PI;

                     OUString trans;
                     if( skewX != 0.0 && rotate != 0.0 ){
                         trans = "skewX (" + Double2Str(skewX)
                                  + ") rotate (" + Double2Str(rotate)
                                  + ") translate (" + Double2Str(WTMM(x + a + drawobj->offset2.x + pal->pt[0].x)) + "mm "
                                  + Double2Str(WTMM(y + b + drawobj->offset2.y + pal->pt[0].y)) + "mm)";
                          bIsRotate = true;
                     }
                     else if( skewX != 0.0 ){
                         trans = "skewX (" + Double2Str(skewX)
                                  + ") translate (" + Double2Str(WTMM(x + a + drawobj->offset2.x + pal->pt[0].x)) + "mm "
                                  + Double2Str(WTMM(y + b + drawobj->offset2.y + pal->pt[0].y)) + "mm)";
                          bIsRotate = true;
                     }
                     else if( rotate != 0.0 ){
                         trans = "rotate (" + Double2Str(rotate)
                                  + ") translate (" + Double2Str(WTMM(x + a + drawobj->offset2.x + pal->pt[0].x)) + "mm "
                                  + Double2Str(WTMM(y + b + drawobj->offset2.y + pal->pt[0].y)) + "mm)";
                          bIsRotate = true;
                     }
                     if( bIsRotate ){
                         drawobj->extent.w = (int)sqrt(double(DBL(pt[1].x-pt[0].x)+DBL(pt[1].y-pt[0].y)));
                         drawobj->extent.h = (int)sqrt(double(DBL(pt[2].x-pt[1].x)+DBL(pt[2].y-pt[1].y)));
                         padd("draw:transform", sXML_CDATA, trans);
                     }
            }
            switch (drawobj->type)
            {
                case HWPDO_LINE:                  /* Line-starting coordinates, ending coordinates. */
                    if( drawobj->u.line_arc.flip & 0x01 )
                    {
                        padd("svg:x1", sXML_CDATA,
                            Double2Str (WTMM(x + a + drawobj->offset2.x + drawobj->extent.w)) + "mm");
                        padd("svg:x2", sXML_CDATA,
                            Double2Str (WTMM( x + a + drawobj->offset2.x )) + "mm");
                    }
                    else
                    {
                        padd("svg:x1", sXML_CDATA,
                            Double2Str (WTMM( x + a + drawobj->offset2.x )) + "mm");
                        padd("svg:x2", sXML_CDATA,
                            Double2Str (WTMM(x + a + drawobj->offset2.x + drawobj->extent.w)) + "mm");
                    }
                    if( drawobj->u.line_arc.flip & 0x02 )
                    {
                        padd("svg:y1", sXML_CDATA,
                            Double2Str (WTMM( y + b + drawobj->offset2.y + drawobj->extent.h ) ) + "mm");
                        padd("svg:y2", sXML_CDATA,
                            Double2Str (WTMM( y + b  + drawobj->offset2.y )) + "mm");
                    }
                    else
                    {
                        padd("svg:y1", sXML_CDATA,
                            Double2Str (WTMM( y + b  + drawobj->offset2.y)) + "mm");
                        padd("svg:y2", sXML_CDATA,
                            Double2Str (WTMM(y + b + drawobj->offset2.y + drawobj->extent.h)) + "mm");
                    }

                    rstartEl("draw:line", rList);
                    pList->clear();
                    rendEl("draw:line");
                    break;
                case HWPDO_RECT:                  /* rectangle - the starting position, vertical/horizontal  */
                    if( !bIsRotate )
                    {
                        padd("svg:x", sXML_CDATA,
                            Double2Str (WTMM( x + a + drawobj->offset2.x)) + "mm");
                        padd("svg:y", sXML_CDATA,
                            Double2Str (WTMM( y + b + drawobj->offset2.y)) + "mm");
                    }
                    padd("svg:width", sXML_CDATA,
                        Double2Str (WTMM( drawobj->extent.w )) + "mm");
                    padd("svg:height", sXML_CDATA,
                        Double2Str (WTMM( drawobj->extent.h )) + "mm");
                    if( drawobj->property.flag & 0x01 )
                    {
                        int value = drawobj->extent.w < drawobj->extent.h ?
                            drawobj->extent.w : drawobj->extent.h ;
                        padd("draw:corner-radius", sXML_CDATA,
                            Double2Str (WTMM( value/10 )) + "mm");
                    }
                    else if( drawobj->property.flag & 0x04 )
                    {
                        int value = drawobj->extent.w < drawobj->extent.h ?
                            drawobj->extent.w : drawobj->extent.h ;
                        padd("draw:corner-radius", sXML_CDATA,
                            Double2Str (WTMM( value / 2)) + "mm");
                    }

                    rstartEl("draw:rect", rList);
                    pList->clear();
                    if( (drawobj->property.flag & HWPDO_FLAG_AS_TEXTBOX) &&
                        drawobj->property.pPara ) // As Textbox
                    {
                        HWPPara *pPara = drawobj->property.pPara;
                                //parsePara(pPara);
                        while(pPara)
                        {
                            make_text_p1( pPara );
                            pPara = pPara->Next();
                        }
                    }
                    rendEl("draw:rect");
                    break;
                case HWPDO_ELLIPSE:               /* Ellipse - the starting position, vertical/horizontal */
                case HWPDO_ADVANCED_ELLIPSE:      /* modified ellipse */
                {
                    if( !bIsRotate )
                    {
                        padd("svg:x", sXML_CDATA,
                            Double2Str (WTMM( x + a + drawobj->offset2.x)) + "mm");
                        padd("svg:y", sXML_CDATA,
                            Double2Str (WTMM( y + b + drawobj->offset2.y)) + "mm");
                    }

                            padd("svg:width", sXML_CDATA,
                                 Double2Str (WTMM( drawobj->extent.w )) + "mm");
                            padd("svg:height", sXML_CDATA,
                                 Double2Str (WTMM( drawobj->extent.h )) + "mm");
                            if( drawobj->type == HWPDO_ADVANCED_ELLIPSE ){
                                if( drawobj->u.arc.radial[0].x != drawobj->u.arc.radial[1].x
                                        || drawobj->u.arc.radial[0].y != drawobj->u.arc.radial[1].y ){
                                    int Cx,Cy;
                                    Cx = ( drawobj->offset2.x + drawobj->extent.w ) / 2;
                                    Cy = ( drawobj->offset2.y + drawobj->extent.h ) / 2;

                                    double start_angle, end_angle;
                                    start_angle = calcAngle( Cx, Cy, drawobj->u.arc.radial[0].x, drawobj->u.arc.radial[0].y );
                                    end_angle = calcAngle( Cx, Cy, drawobj->u.arc.radial[1].x, drawobj->u.arc.radial[1].y );
                                    if( drawobj->property.fill_color < 0xffffff )
                                        padd("draw:kind", sXML_CDATA, "section");
                                    else
                                        padd("draw:kind", sXML_CDATA, "arc");
                                    padd("draw:start-angle", sXML_CDATA, Double2Str(start_angle ));
                                    padd("draw:end-angle", sXML_CDATA, Double2Str(end_angle));
                                }
                          }
                    rstartEl("draw:ellipse", rList);
                    pList->clear();
                    if( drawobj->property.flag >> 19 & 0x01 &&
                        drawobj->property.pPara ) // As Textbox
                    {
                        HWPPara *pPara = drawobj->property.pPara;
                                //parsePara(pPara);
                        while(pPara)
                        {
                            make_text_p1( pPara );
                            pPara = pPara->Next();
                        }
                    }
                    rendEl("draw:ellipse");
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
                                    padd("svg:x", sXML_CDATA,
                                         Double2Str (WTMM( x + a + drawobj->offset2.x - drawobj->extent.w)) + "mm");
                              else
                                    padd("svg:x", sXML_CDATA,
                                         Double2Str (WTMM( x + a + drawobj->offset2.x)) + "mm");
                              if( ( flip == 0 || flip == 1 ) && drawobj->type == HWPDO_ARC)
                                    padd("svg:y", sXML_CDATA,
                                         Double2Str (WTMM( y + b + drawobj->offset2.y - drawobj->extent.h)) + "mm");
                              else
                                    padd("svg:y", sXML_CDATA,
                                         Double2Str (WTMM( y + b + drawobj->offset2.y)) + "mm");
                    }

                            padd("svg:width", sXML_CDATA,
                                 Double2Str (WTMM( drawobj->extent.w * 2)) + "mm");
                            padd("svg:height", sXML_CDATA,
                                 Double2Str (WTMM( drawobj->extent.h * 2)) + "mm");
                            if( drawobj->property.flag & HWPDO_FLAG_DRAW_PIE ||
                                     drawobj->property.fill_color < 0xffffff )
                                padd("draw:kind", sXML_CDATA, "section");
                            else
                                padd("draw:kind", sXML_CDATA, "arc");

                            if( drawobj->type == HWPDO_ADVANCED_ARC ){
                                double start_angle, end_angle;
                                ZZParall *pal = &drawobj->property.parall;

                                if( pal->pt[1].x == pal->pt[0].x ){
                                    if( pal->pt[0].y < pal->pt[1].y )
                                        start_angle = 1.5 * PI;
                                    else
                                        start_angle = 0.5 * PI;
                                }
                                else{
                                     start_angle = atan((double)( pal->pt[0].y - pal->pt[1].y )/( pal->pt[1].x - pal->pt[0].x ));
                                     if( pal->pt[1].x < pal->pt[0].x )
                                         start_angle += PI;
                                }
                                if( pal->pt[1].x == pal->pt[2].x ){
                                    if( pal->pt[2].y < pal->pt[1].y )
                                        end_angle = 1.5 * PI;
                                    else
                                        end_angle = 0.5 * PI;
                                }
                                else{
                                     end_angle = atan((double)( pal->pt[2].y - pal->pt[1].y )/( pal->pt[1].x - pal->pt[2].x ));
                                     if( pal->pt[1].x < pal->pt[2].x )
                                         end_angle += PI;
                                }

                                if( start_angle >= 2 * PI )
                                    start_angle -= 2 * PI;
                                if( end_angle >= 2 * PI )
                                    end_angle -= 2 * PI;
                                if( ( start_angle > end_angle ) && (start_angle - end_angle < PI )){
                                    double tmp_angle = start_angle;
                                    start_angle = end_angle;
                                    end_angle = tmp_angle;
                                }
                                 padd("draw:start-angle", sXML_CDATA, Double2Str(start_angle * 180. / PI));
                                 padd("draw:end-angle", sXML_CDATA, Double2Str(end_angle * 180. / PI));

                            }
                            else{
                            if( drawobj->u.line_arc.flip == 0 )
                            {
                                 padd("draw:start-angle", sXML_CDATA, "270");
                                 padd("draw:end-angle", sXML_CDATA, "0");
                            }
                            else if( drawobj->u.line_arc.flip == 1 )
                            {
                                 padd("draw:start-angle", sXML_CDATA, "180");
                                 padd("draw:end-angle", sXML_CDATA, "270");
                            }
                            else if( drawobj->u.line_arc.flip == 2 )
                            {
                                 padd("draw:start-angle", sXML_CDATA, "0");
                                 padd("draw:end-angle", sXML_CDATA, "90");
                            }
                            else
                            {
                                 padd("draw:start-angle", sXML_CDATA, "90");
                                 padd("draw:end-angle", sXML_CDATA, "180");
                            }
                            }
                    rstartEl("draw:ellipse", rList);
                    pList->clear();
                    if( drawobj->property.flag >> 19 & 0x01 &&
                        drawobj->property.pPara ) // As Textbox
                    {
                        HWPPara *pPara = drawobj->property.pPara;
                                //parsePara(pPara);
                        while(pPara)
                        {
                            make_text_p1( pPara );
                            pPara = pPara->Next();
                        }
                    }
                    rendEl("draw:ellipse");
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
                        padd("svg:x", sXML_CDATA,
                            Double2Str (WTMM( x + a + drawobj->offset2.x)) + "mm");
                        padd("svg:y", sXML_CDATA,
                            Double2Str (WTMM( y + b + drawobj->offset2.y)) + "mm");
                    }
                    padd("svg:width", sXML_CDATA,
                        Double2Str (WTMM( drawobj->extent.w )) + "mm");
                    padd("svg:height", sXML_CDATA,
                        Double2Str (WTMM( drawobj->extent.h )) + "mm");
                    sprintf(buf, "0 0 %d %d", WTSM(drawobj->extent.w) , WTSM(drawobj->extent.h) );
                    padd("svg:viewBox", sXML_CDATA, ascii(buf) );

                    OUString oustr;

                    if ((drawobj->u.freeform.npt > 2) &&
                        (static_cast<size_t>(drawobj->u.freeform.npt) <
                         ((::std::numeric_limits<int>::max)() / sizeof(double))))
                    {
                              int n, i;
                              n = drawobj->u.freeform.npt;

                              double *xarr = new double[n+1];
                              double *yarr = new double[n+1];
                              double *tarr = new double[n+1];

                              double *xb = nullptr;
                              double *yb = nullptr;

                              double *carr = nullptr;
                              double *darr = nullptr;


                              for( i = 0 ; i < n ; i++ ){
                                  xarr[i] = drawobj->u.freeform.pt[i].x;
                                  yarr[i] = drawobj->u.freeform.pt[i].y;
                                  tarr[i] = i;
                              }
                              xarr[n] = xarr[0];
                              yarr[n] = yarr[0];
                              tarr[n] = n;

                              if( !bIsNatural ){
                                  PeriodicSpline(n, tarr, xarr, xb, carr, darr);
                                  // prevent memory leak
                                  delete[] carr;
                                  carr = nullptr;
                                  delete[] darr;
                                  darr = nullptr;
                                  PeriodicSpline(n, tarr, yarr, yb, carr, darr);
                              }
                              else{
                                  NaturalSpline(n, tarr, xarr, xb, carr, darr);
                                  // prevent memory leak
                                  delete[] carr;
                                  carr = nullptr;
                                  delete[] darr;
                                  darr = nullptr;
                                  NaturalSpline(n, tarr, yarr, yb, carr, darr);
                              }

                              sprintf(buf, "M%d %dC%d %d", WTSM((int)xarr[0]), WTSM((int)yarr[0]),
                                      WTSM((int)(xarr[0] + xb[0]/3)), WTSM((int)(yarr[0] + yb[0]/3)) );
                              oustr += ascii(buf);

                              for( i = 1 ; i < n  ; i++ ){
                                  if( i == n -1 ){
                                      sprintf(buf, " %d %d %d %dz",
                                              WTSM((int)(xarr[i] - xb[i]/3)), WTSM((int)(yarr[i] - yb[i]/3)),
                                              WTSM((int)xarr[i]), WTSM((int)yarr[i]) );
                                  }
                                  else{
                                      sprintf(buf, " %d %d %d %d %d %d",
                                              WTSM((int)(xarr[i] - xb[i]/3)), WTSM((int)(yarr[i] - yb[i]/3)),
                                              WTSM((int)xarr[i]), WTSM((int)yarr[i]),
                                              WTSM((int)xarr[i] + xb[i]/3), WTSM((int)(yarr[i] + yb[i]/3)) );
                                  }

                                  oustr += ascii(buf);
                              }
                              delete[] tarr;
                              delete[] xarr;
                              delete[] yarr;

                              delete[] xb;
                              delete[] yb;

                              delete[] carr;
                              delete[] darr;
                          }

                    padd("svg:d", sXML_CDATA, oustr);

                    rstartEl("draw:path", rList);
                    pList->clear();
                                                  // As Textbox
                    if( drawobj->property.flag >> 19 & 0x01 && drawobj->property.pPara )
                    {
                        HWPPara *pPara = drawobj->property.pPara;
                        while(pPara)
                        {
                            make_text_p1( pPara );
                            pPara = pPara->Next();
                        }
                    }
                    rendEl("draw:path");
                    break;
                }
                case HWPDO_CLOSED_FREEFORM:
                case HWPDO_FREEFORM:              /* polygon */
                {
                    bool bIsPolygon = false;

                            padd("svg:x", sXML_CDATA,
                                 Double2Str (WTMM( x + a + drawobj->offset2.x)) + "mm");
                            padd("svg:y", sXML_CDATA,
                                 Double2Str (WTMM( y + b + drawobj->offset2.y)) + "mm");

                    padd("svg:width", sXML_CDATA,
                        Double2Str (WTMM( drawobj->extent.w )) + "mm");
                    padd("svg:height", sXML_CDATA,
                        Double2Str (WTMM( drawobj->extent.h )) + "mm");

                    sprintf(buf, "0 0 %d %d", WTSM(drawobj->extent.w), WTSM(drawobj->extent.h));
                    padd("svg:viewBox", sXML_CDATA, ascii(buf) );

                    OUString oustr;

                    if (drawobj->u.freeform.npt > 0)
                    {
                        sprintf(buf, "%d,%d", WTSM(drawobj->u.freeform.pt[0].x), WTSM(drawobj->u.freeform.pt[0].y));
                        oustr += ascii(buf);
                        int i;
                        for (i = 1; i < drawobj->u.freeform.npt  ; i++)
                        {
                            sprintf(buf, " %d,%d",
                                WTSM(drawobj->u.freeform.pt[i].x),
                                WTSM(drawobj->u.freeform.pt[i].y));
                            oustr += ascii(buf);
                        }
                        if( drawobj->u.freeform.pt[0].x == drawobj->u.freeform.pt[i-1].x &&
                            drawobj->u.freeform.pt[0].y == drawobj->u.freeform.pt[i-1].y )
                        {
                            bIsPolygon = true;
                        }
                    }
                    padd("draw:points", sXML_CDATA, oustr);

                    if( drawobj->property.fill_color <=  0xffffff ||
                        drawobj->property.pattern_type != 0)
                    {
                        bIsPolygon = true;
                    }

                    if(bIsPolygon)
                    {
                        rstartEl("draw:polygon", rList);
                        pList->clear();
                        if( drawobj->property.flag >> 19 & 0x01 &&
                                                  // As Textbox
                            drawobj->property.pPara )
                        {
                            HWPPara *pPara = drawobj->property.pPara;
                            //  parsePara(pPara);
                            while(pPara)
                            {
                                make_text_p1( pPara );
                                pPara = pPara->Next();
                            }
                        }
                        rendEl("draw:polygon");
                    }
                    else
                    {
                        rstartEl("draw:polyline", rList);
                        pList->clear();
                        if( drawobj->property.flag >> 19 & 0x01 &&
                                                  // As Textbox
                            drawobj->property.pPara )
                        {
                            HWPPara *pPara = drawobj->property.pPara;
                                //parsePara(pPara);
                            while(pPara)
                            {
                                make_text_p1( pPara );
                                pPara = pPara->Next();
                            }
                        }
                        rendEl("draw:polyline");
                    }
                    break;
                }
                case HWPDO_TEXTBOX:
                    if( !bIsRotate )
                    {
                        padd("svg:x", sXML_CDATA,
                            Double2Str (WTMM( x + a + drawobj->offset2.x)) + "mm");
                        padd("svg:y", sXML_CDATA,
                            Double2Str (WTMM( y + b + drawobj->offset2.y)) + "mm");
                    }
                    padd("svg:width", sXML_CDATA,
                        Double2Str (WTMM( drawobj->extent.w )) + "mm");
                    padd("svg:height", sXML_CDATA,
                        Double2Str (WTMM( drawobj->extent.h )) + "mm");
                    if( drawobj->property.flag & 0x01 )
                    {
                        int value = drawobj->extent.w < drawobj->extent.h ?
                            drawobj->extent.w : drawobj->extent.h ;
                        padd("draw:corner-radius", sXML_CDATA,
                            Double2Str (WTMM( value/10 )) + "mm");
                    }
                    else if( drawobj->property.flag & 0x04 )
                    {
                        int value = drawobj->extent.w < drawobj->extent.h ?
                            drawobj->extent.w : drawobj->extent.h ;
                        padd("draw:corner-radius", sXML_CDATA,
                            Double2Str (WTMM( value / 2)) + "mm");
                    }

                    rstartEl("draw:text-box", rList);
                    pList->clear();

                    HWPPara *pPara = drawobj->u.textbox.h;
                                //parsePara(pPara);
                    while(pPara)
                    {
                        make_text_p1( pPara );
                        pPara = pPara->Next();
                    }

                    rendEl("draw:text-box");
                    break;
            }
        }
        pList->clear();
        drawobj = drawobj->next;
    }
}


/**
 *
 */
void HwpReader::makeLine(Line *   )
{
    padd("text:style-name", sXML_CDATA, "Horizontal Line");
    rstartEl( "text:p", rList);
    pList->clear();
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

    padd("text:condition", sXML_CDATA, "");
    padd("text:string-value", sXML_CDATA, "");
    rstartEl("text:hidden-text", rList);
    pList->clear();
    HWPPara *para = hbox->plist.front();

    while (para)
    {
        for (int n = 0; n < para->nch && para->hhstr[n]->hh;
            n += para->hhstr[n]->WSize())
        {
              res = hcharconv(para->hhstr[n]->hh, dest, UNICODE);
              for( int j = 0 ; j < res ; j++ )
              {
                    str.push_back(dest[j]);
              }
        }
        para = para->Next();
    }
    makeChars(str);
    rendEl("text:hidden-text");
}


/**
 * Converts footnote to text:footnote, endnote to text:endnote
 */
void HwpReader::makeFootnote(Footnote * hbox)
{
    if (hbox->type)
    {
        padd("text:id", sXML_CDATA,
            ascii(Int2Str(hbox->number, "edn%d", buf)));
        rstartEl("text:endnote", rList);
        pList->clear();
        padd("text:label", sXML_CDATA,
            ascii(Int2Str(hbox->number, "%d", buf)));
        rstartEl("text:endnote-citation", rList);
        pList->clear();
        rchars(ascii(Int2Str(hbox->number, "%d", buf)));
        rendEl("text:endnote-citation");
        rstartEl("text:endnote-body", rList);
        parsePara(hbox->plist.front());
        rendEl("text:endnote-body");
        rendEl("text:endnote");
    }
    else
    {
        padd("text:id", sXML_CDATA,
            ascii(Int2Str(hbox->number, "ftn%d", buf)));
        rstartEl("text:footnote", rList);
        pList->clear();
        padd("text:label", sXML_CDATA,
            ascii(Int2Str(hbox->number, "%d", buf)));
        rstartEl("text:footnote-citation", rList);
        pList->clear();
        rchars(ascii(Int2Str(hbox->number, "%d", buf)));
        rendEl("text:footnote-citation");
        rstartEl("text:footnote-body", rList);
        parsePara(hbox->plist.front());
        rendEl("text:footnote-body");
        rendEl("text:footnote");
    }
}


/**
 * page/footnote/endnote/picture/table/formula number
 */
void HwpReader::makeAutoNum(AutoNum * hbox)
{
    switch (hbox->type)
    {
        case PGNUM_AUTO:
            rstartEl("text:page-number", rList);
            rchars(ascii(Int2Str(hbox->number, "%d", buf)));
            rendEl("text:page-number");
            break;
        case FNNUM_AUTO:
            break;
        case ENNUM_AUTO:
            break;
        case EQUNUM_AUTO:
        case PICNUM_AUTO:
            padd("text:ref-name",sXML_CDATA,
                ascii(Int2Str(hbox->number, "refIllustration%d", buf)));
            padd("text:name",sXML_CDATA, "Illustration");
            padd("style:num-format",sXML_CDATA, "1");
            rstartEl("text:sequence", rList);
            rchars(ascii(Int2Str(hbox->number, "%d", buf)));
            rendEl("text:sequence");
            break;
        case TBLNUM_AUTO:
            padd("text:ref-name",sXML_CDATA,
                ascii(Int2Str(hbox->number, "refTable%d", buf)));
            padd("text:name",sXML_CDATA, "Table");
            padd("style:num-format",sXML_CDATA, "1");
            rstartEl("text:sequence", rList);
            rchars(ascii(Int2Str(hbox->number, "%d", buf)));
            rendEl("text:sequence");
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

    padd("draw:style-name", sXML_CDATA,
        ascii(Int2Str(nPos, "PNBox%d", buf)));
    padd("draw:name", sXML_CDATA,
        ascii(Int2Str(nPos, "PageNumber%d", buf)));
    padd("text:anchor-type", sXML_CDATA, "paragraph");
    padd("svg:y", sXML_CDATA, "0cm");
    padd("svg:width", sXML_CDATA, "2.0cm");
    padd("fo:min-height", sXML_CDATA, "0.5cm");
    rstartEl("draw:text-box", rList);
    pList->clear();

    padd("text:style-name", sXML_CDATA,
        ascii(Int2Str(nPos, "PNPara%d", buf)));
    rstartEl("text:p", rList);
    pList->clear();
    if( hbox->shape > 2 )
        rchars("- ");
    if( hbox->shape % 3 == 0 )
        padd("style:num-format", sXML_CDATA, "1");
    else if( hbox->shape % 3 == 1 )
        padd("style:num-format", sXML_CDATA, "I");
    else
        padd("style:num-format", sXML_CDATA, "i");
    padd("text:select-page", sXML_CDATA, "current");
    rstartEl("text:page-number", rList);
    pList->clear();
    rchars("2");
    rendEl("text:page-number");
    if( hbox->shape > 2 )
        rchars(" -");
    rendEl("text:p");
    rendEl("draw:text-box");
}


/**
 * mail merge operation using hwp addressbook and hwp data form.
 * not support operation in OO writer.
 */
void HwpReader::makeMailMerge(MailMerge * hbox)
{
    hchar_string const boxstr = hbox->GetString();
    rchars(reinterpret_cast<sal_Unicode const *>(hconv(boxstr.c_str())));
}


void HwpReader::makeOutline(Outline * hbox)
{
    if( hbox->kind == 1 )
        rchars( reinterpret_cast<sal_Unicode const *>(hbox->GetUnicode().c_str()) );
}


void HwpReader::parsePara(HWPPara * para, bool bParaStart)
{

    while (para)
    {
        if( para->nch == 1)
        {
            if( !bParaStart )
            {
                padd("text:style-name", sXML_CDATA,
                    ascii(getPStyleName(para->GetParaShape().index, buf)));
                rstartEl( "text:p",rList);
                pList->clear();
            }
            if( d->bFirstPara && d->bInBody )
            {
/* for HWP's Bookmark */
                strcpy(
                    buf,
                    "[\xEB\xAC\xB8\xEC\x84\x9C\xEC\x9D\x98"
                        " \xEC\xB2\x98\xEC\x9D\x8C]");
                    // U+BB38 HANGUL SYLLABLE MUN, U+C11C HANGUL SYLLABLE SEO,
                    // U+C758 HANGUL SYLLABLE YI, U+CC98 HANGUL SYLLABLE CEO,
                    // U+C74C HANGUL SYLLABLE EUM: "Begin of Document"
                padd("text:name", sXML_CDATA, OUString(buf, strlen(buf), RTL_TEXTENCODING_UTF8));
                rstartEl("text:bookmark", rList);
                pList->clear();
                rendEl("text:bookmark");
                d->bFirstPara = false;
            }
            if( d->bInHeader )
            {
                makeShowPageNum();
                d->bInHeader = false;
            }

            rendEl( "text:p" );
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
