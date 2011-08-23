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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_hwpfilter.hxx"

#include "hwpreader.hxx"
#include <math.h>
//#ifndef UDK100
#include "formula.h"
#include "cspline.h"
//#endif
extern int getRepFamilyName(const char* , char *, double &ratio);

#include <iostream>
#include <locale.h>
#include <sal/types.h>
// #i42367# prevent MS compiler from using system locale for parsing
#ifdef _MSC_VER
#pragma setlocale("C")
#endif

// To be shorten source code by realking
#define hconv(x,y) OUString(hstr2ucsstr(x,y))
#define ascii(x)        OUString::createFromAscii(x)
#define rstartEl(x,y)   rDocumentHandler->startElement(x,y)
#define rendEl(x)       rDocumentHandler->endElement(x)
#define rchars(x)       rDocumentHandler->characters(x)
#define padd(x,y,z)     pList->addAttribute(x,y,z)
#define Double2Str(x)	OUString::valueOf((double)(x))
#define WTI(x)          ((double)(x) / 1800.)     // unit => inch
#define WTMM(x)     ((double)(x) / 1800. * 25.4)  // unit => mm
#define WTSM(x)     ((int)((x) / 1800. * 2540))   // unit ==> 1/100 mm

#define PI 3.14159265358979323846

// xmloff/xmlkyd.hxx
#define sXML_CDATA ascii("CDATA")

#define STARTP  padd( ascii("text:style-name"), ascii("CDATA"), ascii(getPStyleName(((ParaShape *)para->GetParaShape())->index,buf))); \
    rstartEl( ascii("text:p"),rList ); \
    pList->clear(); \
    pstart = true
#define STARTT \
    curr = para->GetCharShape(n > 0 ? n-1 : 0)->index; \
    padd( ascii("text:style-name"), ascii("CDATA") , ascii( getTStyleName(curr, buf) ) ); \
    rstartEl( ascii("text:span"),rList ); \
    pList->clear(); \
    tstart = true
#define ENDP \
    rendEl(ascii("text:p")); \
    pstart = false
#define ENDT \
    rendEl(ascii("text:span")); \
    tstart = false

static hchar gstr[1024];
static hchar sbuf[256];
static hchar *field = 0L;
static char buf[1024];

struct HwpReaderPrivate
{
    HwpReaderPrivate()
    {
        bFirstPara = sal_True;
        bInBody = sal_False;
        bInHeader = sal_False;
        nPnPos = 0;
        pPn = 0L;

    }
    sal_Bool bFirstPara;
    sal_Bool bInBody;
    sal_Bool bInHeader;
    ShowPageNum *pPn;
    int nPnPos;
};

HwpReader::HwpReader()
{
    pList = new AttributeListImpl;
    rList = (XAttributeList *) pList;
    d = new HwpReaderPrivate;
}


HwpReader::~HwpReader()
{
    rList = 0;
    delete d;
}


#ifndef UDK100
sal_Bool HwpReader::filter(const Sequence< PropertyValue >& aDescriptor) throw(RuntimeException)
#else
void HwpReader::parseStream(const InputSource & structSource)
throw(SAXException, IOException, RuntimeException)
#endif
{
#ifndef UDK100
    Reference< XInputStream> rInputStream;
    for ( sal_Int32 i = 0; i < aDescriptor.getLength(); i++ )
    {
        if ( aDescriptor[i].Name == OUString::createFromAscii( "InputStream" ) )
        {
            aDescriptor[i].Value >>= rInputStream;
            break;
        }
        else if ( aDescriptor[i].Name == OUString::createFromAscii( "URL" ) )
        {
            OUString sURL;
            aDescriptor[i].Value >>= sURL;

            Reference< XContentIdentifierFactory > xIdFactory( rUCB, UNO_QUERY );
            Reference< XContentProvider > xProvider( rUCB, UNO_QUERY );
            Reference< XContentIdentifier > xId = xIdFactory->createContentIdentifier( sURL );
            Reference< XContent > xContent = xProvider->queryContent( xId );
            MyDataSink * pSink = new MyDataSink();
            OpenCommandArgument2 aArgument;
            aArgument.Sink = Reference< XInterface > ((OWeakObject *)pSink);
            aArgument.Mode = OpenMode::DOCUMENT;
            Command aCommand;
            aCommand.Name = OUString( RTL_CONSTASCII_USTRINGPARAM("open"));
            aCommand.Handle = -1;
            aCommand.Argument <<= aArgument;
            Reference< XCommandProcessor > xCmdProcessor( xContent, UNO_QUERY );
            xCmdProcessor->execute( aCommand, 0, Reference< XCommandEnvironment > () );

            rInputStream = pSink->getInputStream();

            break;
        }
    }
#else
    Reference< XInputStream> rInputStream = structSource.aInputStream;
#endif

    HStream stream;
    Sequence < sal_Int8 > aBuffer;
    sal_Int32 nRead, nBlock = 32768, nTotal = 0;
    while( 1 )
    {
        nRead = rInputStream.get()->readBytes(aBuffer, nBlock);
        if( nRead == 0 )
            break;
        stream.addData( (byte *)aBuffer.getConstArray(), nRead );
        nTotal += nRead;
    }
#ifndef UDK100
    if( nTotal == 0 ) return sal_False;
#endif

    if (hwpfile.ReadHwpFile(stream))
    {
#ifdef UDK100
        throw SAXException();
#else
          return sal_False;
#endif
    }

    rDocumentHandler->startDocument();

    padd(ascii("office:class"), sXML_CDATA, ascii("text"));
    padd(ascii("office:version"), sXML_CDATA, ascii("0.9"));

    padd(ascii("xmlns:office"), ascii("CDATA"), ascii("http://openoffice.org/2000/office"));
    padd(ascii("xmlns:style"), ascii("CDATA"), ascii("http://openoffice.org/2000/style"));
    padd(ascii("xmlns:text"), ascii("CDATA"), ascii("http://openoffice.org/2000/text"));
    padd(ascii("xmlns:table"), ascii("CDATA"), ascii("http://openoffice.org/2000/table"));
    padd(ascii("xmlns:draw"), ascii("CDATA"), ascii("http://openoffice.org/2000/drawing"));
    padd(ascii("xmlns:fo"), ascii("CDATA"), ascii("http://www.w3.org/1999/XSL/Format"));
    padd(ascii("xmlns:xlink"), ascii("CDATA"), ascii("http://www.w3.org/1999/xlink"));
    padd(ascii("xmlns:dc"), ascii("CDATA"), ascii("http://purl.org/dc/elements/1.1/"));
    padd(ascii("xmlns:meta"), ascii("CDATA"), ascii("http://openoffice.org/2000/meta"));
    padd(ascii("xmlns:number"), ascii("CDATA"), ascii("http://openoffice.org/2000/datastyle"));
    padd(ascii("xmlns:svg"), ascii("CDATA"), ascii("http://www.w3.org/2000/svg"));
    padd(ascii("xmlns:chart"), ascii("CDATA"), ascii("http://openoffice.org/2000/chart"));
    padd(ascii("xmlns:dr3d"), ascii("CDATA"), ascii("http://openoffice.org/2000/dr3d"));
    padd(ascii("xmlns:math"), ascii("CDATA"), ascii("http://www.w3.org/1998/Math/MathML"));
    padd(ascii("xmlns:form"), ascii("CDATA"), ascii("http://openoffice.org/2000/form"));
    padd(ascii("xmlns:script"), ascii("CDATA"), ascii("http://openoffice.org/2000/script"));

    rstartEl(ascii("office:document"), rList);
    pList->clear();

    makeMeta();
    makeStyles();
    makeAutoStyles();
    makeMasterStyles();
    makeBody();

    rendEl(ascii("office:document"));

    rDocumentHandler->endDocument();
#ifndef UDK100
    return sal_True;
#endif

}


/**
 * make office:body
 */
void HwpReader::makeBody()
{
    rstartEl(ascii("office:body"), rList);
    makeTextDecls();
    HWPPara *hwppara = hwpfile.GetFirstPara();
    d->bInBody = sal_True;
    parsePara(hwppara);
    rendEl(ascii("office:body"));
    d->bInBody = sal_False;
}


/**
 * make text decls
 */
void HwpReader::makeTextDecls()
{
    rstartEl(ascii("text:sequence-decls"), rList);
    padd(ascii("text:display-outline-level"), sXML_CDATA, ascii("0"));
    padd(ascii("text:name"), sXML_CDATA, ascii("Illustration"));
    rstartEl(ascii("text:sequence-decl"), rList);
    pList->clear();
    rendEl(ascii("text:sequence-decl"));
    padd(ascii("text:display-outline-level"), sXML_CDATA, ascii("0"));
    padd(ascii("text:name"), sXML_CDATA, ascii("Table"));
    rstartEl(ascii("text:sequence-decl"), rList);
    pList->clear();
    rendEl(ascii("text:sequence-decl"));
    padd(ascii("text:display-outline-level"), sXML_CDATA, ascii("0"));
    padd(ascii("text:name"), sXML_CDATA, ascii("Text"));
    rstartEl(ascii("text:sequence-decl"), rList);
    pList->clear();
    rendEl(ascii("text:sequence-decl"));
    padd(ascii("text:display-outline-level"), sXML_CDATA, ascii("0"));
    padd(ascii("text:name"), sXML_CDATA, ascii("Drawing"));
    rstartEl(ascii("text:sequence-decl"), rList);
    pList->clear();
    rendEl(ascii("text:sequence-decl"));
    rendEl(ascii("text:sequence-decls"));
}


#define ISNUMBER(x) ( (x) <= 0x39 && (x) >= 0x30 )
/**
 * make office:meta
 * Completed
 */
void HwpReader::makeMeta()
{
    HWPInfo *hwpinfo = hwpfile.GetHWPInfo();

    rstartEl(ascii("office:meta"), rList);

    if (hwpinfo->summary.title[0])
    {
        rstartEl(ascii("dc:title"), rList);
        rchars((hconv(hwpinfo->summary.title, gstr)));
        rendEl(ascii("dc:title"));
    }

    if (hwpinfo->summary.subject[0])
    {
        rstartEl(ascii("dc:subject"), rList);
        rchars((hconv(hwpinfo->summary.subject, gstr)));
        rendEl(ascii("dc:subject"));
    }

    if (hwpinfo->summary.author[0])
    {
        rstartEl(ascii("meta:initial-creator"), rList);
        rchars((hconv(hwpinfo->summary.author, gstr)));
        rendEl(ascii("meta:initial-creator"));
    }

    if (hwpinfo->summary.date[0])
    {
        unsigned short *pDate = hwpinfo->summary.date;
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

/* 2001?? 9?? 8?? ??????, 14?? 16?? */
// 2001-09-07T11:16:47
        rstartEl( ascii("meta:creation-date"), rList );
        rchars( ascii(buf));
        rendEl( ascii("meta:creation-date") );
    }

    if (hwpinfo->summary.keyword[0][0] || hwpinfo->summary.etc[0][0])
    {
/* ???????? dc?? ????????. */
        rstartEl(ascii("meta:keywords"), rList);
        if (hwpinfo->summary.keyword[0][0])
        {
            rstartEl(ascii("meta:keyword"), rList);
            rchars((hconv(hwpinfo->summary.keyword[0], gstr)));
            rendEl(ascii("meta:keyword"));
        }
        if (hwpinfo->summary.keyword[1][0])
        {
            rstartEl(ascii("meta:keyword"), rList);
            rchars((hconv(hwpinfo->summary.keyword[1], gstr)));
            rendEl(ascii("meta:keyword"));
        }
        if (hwpinfo->summary.keyword[2][0])
        {
            rstartEl(ascii("meta:keyword"), rList);
            rchars((hconv(hwpinfo->summary.etc[0], gstr)));
            rendEl(ascii("meta:keyword"));
        }
        if (hwpinfo->summary.etc[0][0])
        {
            rstartEl(ascii("meta:keyword"), rList);
            rchars((hconv(hwpinfo->summary.etc[1], gstr)));
            rendEl(ascii("meta:keyword"));
        }
        if (hwpinfo->summary.etc[1][0])
        {
            rstartEl(ascii("meta:keyword"), rList);
            rchars((hconv(hwpinfo->summary.etc[2], gstr)));
            rendEl(ascii("meta:keyword"));
        }
        rendEl(ascii("meta:keywords"));
    }
/*
    padd(ascii("meta:name"), sXML_CDATA, ascii("BBSID"));
    rstartEl(ascii("meta:user-defined"), rList);
    pList->clear();
    rchars((hconv(hwpinfo->summary.etc[2], gstr)));
    rendEl(ascii("meta:user-defined"));

    rstartEl(ascii("dc:language"), rList);
    rchars(ascii("C"));
    rendEl(ascii("dc:language"));
*/
    rendEl(ascii("office:meta"));
}


static struct
{
    const char *name;
    sal_Bool bMade;
}


ArrowShape[] =
{
    { "", sal_False },
    {
        "Arrow", sal_False
    },
    { "Line Arrow", sal_False },
    {
        "Square", sal_False
    }
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
            padd( ascii("draw:name"), sXML_CDATA, ascii(Int2Str(hdo->index, "LineType%d", buf)));
            padd( ascii("draw:style"), sXML_CDATA, ascii("round"));
            padd( ascii("draw:dots1"), sXML_CDATA, ascii("1"));
            padd( ascii("draw:dots1-length"), sXML_CDATA, Double2Str( LineStyle[prop->line_pstyle].dots1 * WTMM(prop->line_width) ) + ascii("cm"));
            if( prop->line_pstyle == 3 )
            {
                padd( ascii("draw:dots2"), sXML_CDATA, ascii("1"));
                padd( ascii("draw:dots2-length"), sXML_CDATA, Double2Str( LineStyle[prop->line_pstyle].dots2 * WTMM(prop->line_width) ) + ascii("cm"));
            }
            else if( prop->line_pstyle == 4 )
            {
                padd( ascii("draw:dots2"), sXML_CDATA, ascii("2"));
                padd( ascii("draw:dots2-length"), sXML_CDATA, Double2Str( LineStyle[prop->line_pstyle].dots2 * WTMM(prop->line_width)) + ascii("cm"));
            }
            padd( ascii("draw:distance"), sXML_CDATA, Double2Str( LineStyle[prop->line_pstyle].distance * WTMM(prop->line_width)) + ascii("cm"));
            rstartEl( ascii("draw:stroke-dash"), rList);
            pList->clear();
            rendEl( ascii("draw:stroke-dash") );
        }

        if( hdo->type == HWPDO_LINE || hdo->type == HWPDO_ARC || hdo->type == HWPDO_FREEFORM ||
            hdo->type == HWPDO_ADVANCED_ARC )
        {
                                                  /* ???????? ???? */
            if( prop->line_tstyle && !ArrowShape[prop->line_tstyle].bMade  )
            {
                ArrowShape[prop->line_tstyle].bMade = sal_True;
                padd(ascii("draw:name"), sXML_CDATA,
                    ascii(ArrowShape[prop->line_tstyle].name));
                if( prop->line_tstyle == 1 )
                {
                    padd(ascii("svg:viewBox"), sXML_CDATA, ascii("0 0 20 30"));
                    padd(ascii("svg:d"), sXML_CDATA, ascii("m10 0-10 30h20z"));
                }
                else if( prop->line_tstyle == 2 )
                {
                    padd(ascii("svg:viewBox"), sXML_CDATA, ascii("0 0 1122 2243"));
                    padd(ascii("svg:d"), sXML_CDATA, ascii("m0 2108v17 17l12 42 30 34 38 21 43 4 29-8 30-21 25-26 13-34 343-1532 339 1520 13 42 29 34 39 21 42 4 42-12 34-30 21-42v-39-12l-4 4-440-1998-9-42-25-39-38-25-43-8-42 8-38 25-26 39-8 42z"));
                }
                else if( prop->line_tstyle == 3 )
                {
                    padd(ascii("svg:viewBox"), sXML_CDATA, ascii("0 0 30 30"));
                    padd(ascii("svg:d"), sXML_CDATA, ascii("m0 0h30v30h-30z"));
                }
                rstartEl(ascii("draw:marker"), rList);
                pList->clear();
                rendEl(ascii("draw:marker"));
            }
            if( prop->line_hstyle && !ArrowShape[prop->line_hstyle].bMade)
            {
                ArrowShape[prop->line_hstyle].bMade = sal_True;
                padd(ascii("draw:name"), sXML_CDATA,
                    ascii(ArrowShape[prop->line_hstyle].name));
                if( prop->line_hstyle == 1 )
                {
                    padd(ascii("svg:viewBox"), sXML_CDATA, ascii("0 0 20 30"));
                    padd(ascii("svg:d"), sXML_CDATA, ascii("m10 0-10 30h20z"));
                }
                else if( prop->line_hstyle == 2 )
                {
                    padd(ascii("svg:viewBox"), sXML_CDATA, ascii("0 0 1122 2243"));
                    padd(ascii("svg:d"), sXML_CDATA, ascii("m0 2108v17 17l12 42 30 34 38 21 43 4 29-8 30-21 25-26 13-34 343-1532 339 1520 13 42 29 34 39 21 42 4 42-12 34-30 21-42v-39-12l-4 4-440-1998-9-42-25-39-38-25-43-8-42 8-38 25-26 39-8 42z"));
                }
                else if( prop->line_hstyle == 3 )
                {
                    padd(ascii("svg:viewBox"), sXML_CDATA, ascii("0 0 20 20"));
                    padd(ascii("svg:d"), sXML_CDATA, ascii("m0 0h20v20h-20z"));
                }
                rstartEl(ascii("draw:marker"), rList);
                pList->clear();
                rendEl(ascii("draw:marker"));
            }
        }

        if( hdo->type != HWPDO_LINE )
        {
            if( prop->flag >> 18  & 0x01 )        /* ?????? ???? ???? ???? */
            {
                padd( ascii("draw:name"), sXML_CDATA, ascii(Int2Str(hdo->index, "fillimage%d", buf)));
                if( !prop->pictype )
                {
                    padd( ascii("xlink:href"), sXML_CDATA,
                        hconv(kstr2hstr( (uchar *)urltounix(prop->szPatternFile, buf), sbuf), gstr));
                }
                else                              /* ???????? image???? ?????? ???????? ????. */
                {
                    EmPicture *emp = 0L;
                    if ( strlen( prop->szPatternFile ) > 3)
                        emp = hwpfile.GetEmPictureByName(prop->szPatternFile);
                    if( emp )
                    {
                        char filename[128];
                        char dirname[128];
                        int fd, res;
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
                            res = write( fd, emp->data, emp->size );
                            close(fd);
                        }
#ifdef _WIN32
                        int j;
                        for( j = 0 ; j < (int)strlen( dirname ) ; j++)
                        {
                            if( dirname[j] == '\\' ) buf[j] = '/';
                            else buf[j] = dirname[j];
                        }
                        buf[j] = '\0';
                        sprintf(filename, "file:///%s%s",buf, emp->name );
#else
                        sprintf(filename, "file://%s%s",dirname, emp->name );
#endif
                        padd( ascii("xlink:href"), sXML_CDATA, ascii(filename));
                    }
                    else
                    {
                        padd( ascii("xlink:href"), sXML_CDATA,
                            hconv(kstr2hstr( (uchar *)urltounix(prop->szPatternFile, buf), sbuf), gstr));
                    }

                }
                padd( ascii("xlink:type"), sXML_CDATA, ascii("simple"));
                padd( ascii("xlink:show"), sXML_CDATA, ascii("embed"));
                padd( ascii("xlink:actuate"), sXML_CDATA, ascii("onLoad"));

                rstartEl( ascii("draw:fill-image"), rList);
                pList->clear();
                rendEl( ascii("draw:fill-image"));
            }
/* ???????????? ????????, ???????????? ????????, ?????? ????????. */
            else if( prop->flag >> 16  & 0x01 )   /* ?????????? ???????? */
            {
                padd( ascii("draw:name"), sXML_CDATA, ascii(Int2Str(hdo->index, "Grad%d", buf)));
                switch( prop->gstyle )
                {
                    case 1 :
                        if( prop->center_y == 50 )
                            padd( ascii("draw:style"), sXML_CDATA, ascii("axial"));
                        else
                            padd( ascii("draw:style"), sXML_CDATA, ascii("linear"));
                        break;
                    case 2:
                    case 3:
                        padd( ascii("draw:style"), sXML_CDATA, ascii("radial"));
                        break;
                    case 4:
                        padd( ascii("draw:style"), sXML_CDATA, ascii("square"));
                        break;
                    default:
                        padd( ascii("draw:style"), sXML_CDATA, ascii("linear"));
                        break;
                }
                padd( ascii("draw:cx"), sXML_CDATA,ascii(Int2Str(prop->center_x, "%d%%", buf)));
                padd( ascii("draw:cy"), sXML_CDATA,ascii(Int2Str(prop->center_y, "%d%%", buf)));

                     HWPInfo *hwpinfo = hwpfile.GetHWPInfo();
                     int default_color = 0xffffff;
                     if( hwpinfo->back_info.isset )
                     {
                             if( hwpinfo->back_info.color[0] > 0 || hwpinfo->back_info.color[1] > 0
                                     || hwpinfo->back_info.color[2] > 0 )
                                 default_color = hwpinfo->back_info.color[0] << 16 |
                                     hwpinfo->back_info.color[1] << 8 | hwpinfo->back_info.color[2];
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
                        padd( ascii("draw:start-color"), sXML_CDATA, ascii( buf ));
                        sprintf( buf, "#%02x%02x%02x", prop->fromcolor & 0xff,
                            (prop->fromcolor >> 8) & 0xff, (prop->fromcolor >>16) & 0xff );
                        padd( ascii("draw:end-color"), sXML_CDATA, ascii( buf ));
                    }
                    else
                    {
                        sprintf( buf, "#%02x%02x%02x", prop->fromcolor & 0xff,
                            (prop->fromcolor >> 8) & 0xff, (prop->fromcolor >>16) & 0xff );
                        padd( ascii("draw:start-color"), sXML_CDATA, ascii( buf ));
                        sprintf( buf, "#%02x%02x%02x", prop->tocolor & 0xff,
                            (prop->tocolor >> 8) & 0xff, (prop->tocolor >>16) & 0xff );
                        padd( ascii("draw:end-color"), sXML_CDATA, ascii( buf ));
                    }
                }
                else
                {
                    sprintf( buf, "#%02x%02x%02x", prop->tocolor & 0xff,
                        (prop->tocolor >> 8) & 0xff, (prop->tocolor >>16) & 0xff );
                    padd( ascii("draw:start-color"), sXML_CDATA,ascii( buf ));

                    sprintf( buf, "#%02x%02x%02x", prop->fromcolor & 0xff,
                        (prop->fromcolor >> 8) & 0xff, (prop->fromcolor >>16) & 0xff );
                    padd( ascii("draw:end-color"), sXML_CDATA,ascii( buf ));
                }
                if( prop->angle > 0 && ( prop->gstyle == 1 || prop->gstyle == 4))
                {
                    int angle = prop->angle >= 180 ? prop->angle - 180 : prop->angle;
                    angle = 1800 - prop->angle * 10;
                    padd( ascii("draw:angle"), sXML_CDATA,
                        ascii(Int2Str( angle, "%d", buf)));
                }
                rstartEl( ascii("draw:gradient"), rList );
                pList->clear();
                rendEl( ascii("draw:gradient"));
            }
                                                  /* ???? */
            else if( prop->pattern_type >> 24 & 0x01 )
            {
                int type = prop->pattern_type & 0xffffff;
                padd( ascii("draw:name"), sXML_CDATA,
                    ascii(Int2Str(hdo->index, "Hatch%d", buf)));
                if( type < 4 )
                    padd( ascii("draw:style"), sXML_CDATA, ascii("single") );
                else
                    padd( ascii("draw:style"), sXML_CDATA, ascii("double") );
                sprintf( buf, "#%02x%02x%02x",
                    sal_uInt16(prop->pattern_color & 0xff),
                    sal_uInt16((prop->pattern_color >> 8) & 0xff),
                    sal_uInt16((prop->pattern_color >>16) & 0xff) );
                padd( ascii("draw:color"), sXML_CDATA, ascii( buf ));
                padd( ascii("draw:distance"), sXML_CDATA, ascii("0.12cm"));
                switch( type )
                {
                    case 0 :
                    case 4 :
                        padd( ascii("draw:rotation"), sXML_CDATA, ascii("0"));
                        break;
                    case 1 :
                        padd( ascii("draw:rotation"), sXML_CDATA, ascii("900"));
                        break;
                    case 2 :
                        padd( ascii("draw:rotation"), sXML_CDATA, ascii("1350"));
                        break;
                    case 3 :
                    case 5 :
                        padd( ascii("draw:rotation"), sXML_CDATA, ascii("450"));
                        break;
                }
                rstartEl( ascii("draw:hatch"), rList);
                pList->clear();
                rendEl( ascii("draw:hatch"));
            }
        }
        hdo = hdo->next;
    }
}


void HwpReader::makeStyles()
{
    HWPStyle *hwpstyle = hwpfile.GetHWPStyle();

    rstartEl(ascii("office:styles"), rList);

    int i;
    for (i = 0; i < hwpfile.getFBoxStyleCount(); i++)
    {
        if( hwpfile.getFBoxStyle(i)->boxtype == 'D' )
        {
            makeDrawMiscStyle((HWPDrawingObject *)hwpfile.getFBoxStyle(i)->cell );
        }
    }

    padd(ascii("style:name"), sXML_CDATA, ascii("Standard"));
    padd(ascii("style:family"), sXML_CDATA, ascii("paragraph"));
    padd(ascii("style:class"), sXML_CDATA, ascii("text"));
    rstartEl(ascii("style:style"), rList);
    pList->clear();

    padd(ascii("fo:line-height"), sXML_CDATA, ascii("160%"));
    padd(ascii("fo:text-align"), sXML_CDATA, ascii("justify"));
    rstartEl(ascii("style:properties"), rList);
    pList->clear();
    rstartEl(ascii("style:tab-stops"), rList);

    for( i = 1 ; i < 40 ; i++)
    {
        padd(ascii("style:position"), sXML_CDATA,
            Double2Str( WTI(1000 * i)) + ascii("inch"));
        rstartEl(ascii("style:tab-stop"), rList);
        pList->clear();
        rendEl(ascii("style:tab-stop"));
    }
    rendEl(ascii("style:tab-stops"));
    rendEl(ascii("style:properties"));

    rendEl(ascii("style:style"));

    for (int ii = 0; ii < hwpstyle->Num(); ii++)
    {
        unsigned char *stylename = (unsigned char *) hwpstyle->GetName(ii);
        padd(ascii("style:name"), sXML_CDATA, (hconv(kstr2hstr(stylename, sbuf), gstr)));
        padd(ascii("style:family"), sXML_CDATA, ascii("paragraph"));
        padd(ascii("style:parent-style-name"), sXML_CDATA, ascii("Standard"));

        rstartEl(ascii("style:style"), rList);

        pList->clear();

        parseCharShape(hwpstyle->GetCharShape(ii));
        parseParaShape(hwpstyle->GetParaShape(ii));

        rstartEl(ascii("style:properties"), rList);
        pList->clear();
        rendEl(ascii("style:properties"));

        rendEl(ascii("style:style"));
    }

    {
        padd( ascii("style:name"), sXML_CDATA, ascii("Header"));
        padd( ascii("style:family"), sXML_CDATA, ascii("paragraph"));
        padd( ascii("style:parent-style-name"), sXML_CDATA, ascii("Standard"));
        padd( ascii("style:class"), sXML_CDATA, ascii("extra"));
        rstartEl(ascii("style:style"), rList);
        pList->clear();
        rendEl(ascii("style:style"));
    }

    {
        padd( ascii("style:name"), sXML_CDATA, ascii("Footer"));
        padd( ascii("style:family"), sXML_CDATA, ascii("paragraph"));
        padd( ascii("style:parent-style-name"), sXML_CDATA, ascii("Standard"));
        padd( ascii("style:class"), sXML_CDATA, ascii("extra"));
        rstartEl(ascii("style:style"), rList);
        pList->clear();

        rendEl(ascii("style:style"));
    }

    if( hwpfile.linenumber > 0)
    {
        padd( ascii("style:name"), sXML_CDATA, ascii("Horizontal Line"));
        padd( ascii("style:family"), sXML_CDATA, ascii("paragraph"));
        padd( ascii("style:parent-style-name"), sXML_CDATA, ascii("Standard"));
        padd( ascii("style:class"), sXML_CDATA, ascii("html"));
        rstartEl( ascii("style:style"), rList);
        pList->clear();
        padd( ascii("fo:font-size"), sXML_CDATA, ascii("6pt"));
        padd( ascii("fo:margin-top"), sXML_CDATA, ascii("0cm"));
        padd( ascii("fo:margin-bottom"), sXML_CDATA, ascii("0cm"));
        padd( ascii("style:border-line-width-bottom"), sXML_CDATA, ascii("0.02cm 0.035cm 0.002cm"));
        padd( ascii("fo:padding"), sXML_CDATA, ascii("0cm"));
        padd( ascii("fo:border-bottom"), sXML_CDATA, ascii("0.039cm double #808080"));
        padd( ascii("text:number-lines"), sXML_CDATA, ascii("false"));
        padd( ascii("text:line-number"), sXML_CDATA, ascii("0"));
        padd(ascii("fo:line-height"), sXML_CDATA, ascii("100%"));
        rstartEl( ascii("style:properties"), rList);
        pList->clear();
        rendEl( ascii("style:properties"));
        rendEl( ascii("style:style"));
    }

    HWPInfo *hwpinfo = hwpfile.GetHWPInfo();

    padd(ascii("text:num-suffix"), sXML_CDATA, ascii(")"));
    padd(ascii("text:num-format"), sXML_CDATA, ascii("1"));
    if( hwpinfo->beginfnnum != 1)
        padd(ascii("text:offset"), sXML_CDATA, ascii(Int2Str(hwpinfo->beginfnnum -1, "%d", buf)));
    rstartEl(ascii("text:footnotes-configuration"), rList);
    pList->clear();
    rendEl(ascii("text:footnotes-configuration"));

    rendEl(ascii("office:styles"));
}


/**
 * parse automatic styles from hwpfile
 * ?????????? ?????? ???? ???????? ????????. ???????? ?????? ????????, ??????, ???? ?????? ???????? ???????? ????????, Body?????? ?????? ?????? ???????? ????????.
 * 1. paragraph, text, fbox, page???????? ???? ????????.
 */
void HwpReader::makeAutoStyles()
{
    int i;

    rstartEl(ascii("office:automatic-styles"), rList);

    for (i = 0; i < hwpfile.getParaShapeCount(); i++)
        makePStyle(hwpfile.getParaShape(i));

    for (i = 0; i < hwpfile.getCharShapeCount(); i++)
        makeTStyle(hwpfile.getCharShape(i));

    for( i = 0 ; i < hwpfile.getTableCount(); i++)
        makeTableStyle(hwpfile.getTable(i));

    for (i = 0; i < hwpfile.getFBoxStyleCount(); i++)
    {
        if( hwpfile.getFBoxStyle(i)->boxtype == 'D' )
            makeDrawStyle((HWPDrawingObject *)hwpfile.getFBoxStyle(i)->cell, hwpfile.getFBoxStyle(i));
        else
            makeFStyle(hwpfile.getFBoxStyle(i));
    }

// sal_Bool bIsLeft = sal_True, bIsMiddle = sal_True, bIsRight = sal_True;

    sal_Bool bIsLeft = sal_False, bIsMiddle = sal_False, bIsRight = sal_False;
    for( i = 0 ; i < hwpfile.getPageNumberCount() ; i++ )
    {
        ShowPageNum *pn = hwpfile.getPageNumber(i);
        if( pn->where == 7 || pn->where == 8 )
        {
            bIsLeft = sal_True;
            bIsRight = sal_True;
        }
        else if( pn->where == 1 || pn->where == 4 )
        {
            bIsLeft = sal_True;
        }
        else if( pn->where == 2 || pn->where == 5 )
        {
            bIsMiddle = sal_True;
        }
        else if( pn->where == 3 || pn->where == 6 )
        {
            bIsRight = sal_True;
        }
    }

    for( i = 1; i <= 3 ; i++ )
    {
        if( i == 1 && bIsLeft == sal_False )
            continue;
        if( i == 2 && bIsMiddle == sal_False )
            continue;
        if( i == 3 && bIsRight == sal_False )
            continue;
        padd(ascii("style:name"), sXML_CDATA,
            ascii(Int2Str(i,"PNPara%d", buf)));
        padd(ascii("style:family"), sXML_CDATA, ascii("paragraph"));
        padd(ascii("style:parent-style-name"), sXML_CDATA, ascii("Standard"));
        rstartEl(ascii("style:style"), rList);
        pList->clear();
        if( i == 1 )
            padd(ascii("fo:text-align"), sXML_CDATA, ascii("start"));
        else if ( i == 2 )
            padd(ascii("fo:text-align"), sXML_CDATA, ascii("center"));
        else if ( i == 3 )
            padd(ascii("fo:text-align"), sXML_CDATA, ascii("end"));
        rstartEl(ascii("style:properties"), rList);
        pList->clear();
        rendEl( ascii("style:properties"));
        rendEl( ascii("style:style"));

        padd(ascii("style:name"), sXML_CDATA, ascii(Int2Str(i,"PNBox%d",buf)));
        padd(ascii("style:family"), sXML_CDATA, ascii("graphics"));
        rstartEl(ascii("style:style"), rList);
        pList->clear();

        padd(ascii("fo:margin-top"), sXML_CDATA, ascii("0cm"));
        padd(ascii("fo:margin-bottom"), sXML_CDATA, ascii("0cm"));
        padd(ascii("style:wrap"), sXML_CDATA, ascii("run-through"));
        padd(ascii("style:vertical-pos"), sXML_CDATA, ascii("from-top"));
        padd(ascii("style:vertical-rel"), sXML_CDATA, ascii("paragraph"));

        if( i == 1 )
            padd(ascii("style:horizontal-pos"), sXML_CDATA, ascii("left"));
        else if ( i == 2 )
            padd(ascii("style:horizontal-pos"), sXML_CDATA, ascii("center"));
        else if ( i == 3 )
            padd(ascii("style:horizontal-pos"), sXML_CDATA, ascii("right"));
        padd(ascii("style:horizontal-rel"), sXML_CDATA, ascii("paragraph"));
        padd(ascii("fo:padding"), sXML_CDATA, ascii("0cm"));
        padd(ascii("stylefamily"), sXML_CDATA, ascii("graphics"));
        rstartEl(ascii("style:properties"), rList);
        pList->clear();
        rendEl(ascii("style:properties"));
        rendEl(ascii("style:style"));
    }

    for (i = 0; i < hwpfile.getDateFormatCount(); i++)
        makeDateFormat(hwpfile.getDateCode(i));

    makePageStyle();

    rendEl(ascii("office:automatic-styles"));
}


struct PageSetting
{
    PageSetting()
    {
        header = 0L;
        header_odd = 0L;
        header_even = 0L;
        footer = 0L;
        footer_odd = 0L;
        footer_even = 0L;
        pagenumber=0L;
        bIsSet = sal_False;
    }
    HeaderFooter *header ;
    HeaderFooter *header_odd ;
    HeaderFooter *header_even ;
    HeaderFooter *footer ;
    HeaderFooter *footer_odd ;
    HeaderFooter *footer_even ;
    ShowPageNum *pagenumber;
    sal_Bool bIsSet;
};

void HwpReader::makeMasterStyles()
{
    rstartEl(ascii("office:master-styles"), rList);

    int i;
    int nMax = hwpfile.getMaxSettedPage();
    PageSetting *pSet = new PageSetting[nMax + 1];

    for( i = 0 ; i < hwpfile.getPageNumberCount() ; i++ )
    {
        ShowPageNum *pn = hwpfile.getPageNumber(i);
        pSet[pn->m_nPageNumber].pagenumber = pn;
        pSet[pn->m_nPageNumber].bIsSet = sal_True;
    }
    for( i = 0 ; i < hwpfile.getHeaderFooterCount() ; i++ )
    {
        HeaderFooter* hf = hwpfile.getHeaderFooter(i);
        pSet[hf->m_nPageNumber].bIsSet = sal_True;
        if( hf->type == 0 )                       // header
        {
            switch( hf->where )
            {
                case 0 :
                    pSet[hf->m_nPageNumber].header = hf;
                    pSet[hf->m_nPageNumber].header_even = 0L;
                    pSet[hf->m_nPageNumber].header_odd = 0L;
                    break;
                case 1:
                    pSet[hf->m_nPageNumber].header_even = hf;
                    if( pSet[hf->m_nPageNumber].header )
                    {
                        pSet[hf->m_nPageNumber].header_odd =
                            pSet[hf->m_nPageNumber].header;
                        pSet[hf->m_nPageNumber].header = 0L;
                    }
                    break;
                case 2:
                    pSet[hf->m_nPageNumber].header_odd = hf;
                    if( pSet[hf->m_nPageNumber].header )
                    {
                        pSet[hf->m_nPageNumber].header_even =
                            pSet[hf->m_nPageNumber].header;
                        pSet[hf->m_nPageNumber].header = 0L;
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
                    pSet[hf->m_nPageNumber].footer_even = 0L;
                    pSet[hf->m_nPageNumber].footer_odd = 0L;
                    break;
                case 1:
                    pSet[hf->m_nPageNumber].footer_even = hf;
                    if( pSet[hf->m_nPageNumber].footer )
                    {
                        pSet[hf->m_nPageNumber].footer_odd =
                            pSet[hf->m_nPageNumber].footer;
                        pSet[hf->m_nPageNumber].footer = 0L;
                    }
                    break;
                case 2:
                    pSet[hf->m_nPageNumber].footer_odd = hf;
                    if( pSet[hf->m_nPageNumber].footer )
                    {
                        pSet[hf->m_nPageNumber].footer_even =
                            pSet[hf->m_nPageNumber].footer;
                        pSet[hf->m_nPageNumber].footer = 0L;
                    }
                    break;
            }
        }
    }

    PageSetting *pPrevSet = 0L;
    PageSetting *pPage = 0L;

    for( i = 1; i <= nMax ; i++ )
    {
        if( i == 1 )
            padd(ascii("style:name"), sXML_CDATA, ascii("Standard"));
        else
            padd(ascii("style:name"), sXML_CDATA,
                ascii(Int2Str(i, "p%d", buf)));
        padd(ascii("style:page-master-name"), sXML_CDATA,
                ascii(Int2Str(hwpfile.GetPageMasterNum(i), "pm%d", buf)));
        if( i < nMax )
            padd(ascii("style:next-style-name"), sXML_CDATA,
                ascii(Int2Str(i+1, "p%d", buf)));
        padd(ascii("draw:style-name"), sXML_CDATA,
            ascii(Int2Str(i, "master%d", buf)));
        rstartEl(ascii("style:master-page"), rList);
        pList->clear();

        if( pSet[i].bIsSet )                      /* ???? ?????? ?????????? */
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
                    pSet[i].header = 0L;
                }
                if( pSet[i].pagenumber->where == 8 && pSet[i].footer )
                {
                    pSet[i].footer_even = pSet[i].footer;
                    pSet[i].footer_odd = pSet[i].footer;
                    pSet[i].footer = 0L;
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
        else if( pPrevSet )                       /* ?????? ?????? ???? ??????. */
        {
            pPage = pPrevSet;
        }
        else                                      /* ???? ?????? ?????? ???????????? */
        {
            rstartEl(ascii("style:header"), rList);
            padd(ascii("text:style-name"), sXML_CDATA, ascii("Standard"));
            rstartEl(ascii("text:p"), rList);
            pList->clear();
            rendEl(ascii("text:p"));
            rendEl(ascii("style:header"));

            rstartEl(ascii("style:footer"), rList);
            padd(ascii("text:style-name"), sXML_CDATA, ascii("Standard"));
            rstartEl(ascii("text:p"), rList);
            pList->clear();
            rendEl(ascii("text:p"));
            rendEl(ascii("style:footer"));

            rendEl(ascii("style:master-page"));

            continue;
        }
// ------------- header ------------- //
        if( pPage->header )
        {
            rstartEl(ascii("style:header"), rList);
            if( pPage->pagenumber && pPage->pagenumber->where < 4 )
            {
                d->bInHeader = sal_True;
                d->pPn = pPage->pagenumber;
            }
            parsePara(pPage->header->plist.first());
            d->bInHeader = sal_False;
            d->pPn = 0L;
            rendEl(ascii("style:header"));
        }
        if( pPage->header_even )
        {
            rstartEl(ascii("style:header"), rList);
            if( pPage->pagenumber && ( pPage->pagenumber->where < 4
                || pPage->pagenumber->where == 7 ) )
            {
                d->bInHeader = sal_True;
                d->pPn = pPage->pagenumber;
                d->nPnPos = 3;
            }
            parsePara(pPage->header_even->plist.first());
            d->bInHeader = sal_False;
            d->pPn = 0L;
            d->nPnPos = 0;
            rendEl(ascii("style:header"));
        }
                                                  /* ???????? ????. */
        else if( pPage->header_odd && !pPage->header_even )
        {
            rstartEl(ascii("style:header"), rList);
            padd(ascii("text:style-name"), sXML_CDATA, ascii("Standard"));
            rstartEl(ascii("text:p"), rList);
            pList->clear();
            if( pPage->pagenumber && ( pPage->pagenumber->where < 4 ||
                pPage->pagenumber->where == 7 ) )
            {
                d->pPn = pPage->pagenumber;
                d->nPnPos = 3;
                makeShowPageNum();
                d->pPn = 0L;
                d->nPnPos = 0;
            }
            rendEl(ascii("text:p"));
            rendEl(ascii("style:header"));
        }
        if( pPage->header_odd )
        {
            rstartEl(ascii("style:header-left"), rList);
            if( pPage->pagenumber && ( pPage->pagenumber->where < 4
                || pPage->pagenumber->where == 7 ) )
            {
                d->bInHeader = sal_True;
                d->nPnPos = 1;
                d->pPn = pPage->pagenumber;
            }
            parsePara(pPage->header_odd->plist.first());
            d->bInHeader = sal_False;
            d->pPn = 0L;
            d->nPnPos = 0;
            rendEl(ascii("style:header-left"));
        }
                                                  /* ???????? ????. */
        else if( pPage->header_even && !pPage->header_odd )
        {
            rstartEl(ascii("style:header-left"), rList);
            padd(ascii("text:style-name"), sXML_CDATA, ascii("Standard"));
            rstartEl(ascii("text:p"), rList);
            pList->clear();
            if( pPage->pagenumber && ( pPage->pagenumber->where < 4 ||
                pPage->pagenumber->where == 7 ) )
            {
                d->pPn = pPage->pagenumber;
                d->nPnPos = 1;
                makeShowPageNum();
                d->pPn = 0L;
                d->nPnPos = 0;
            }
            rendEl(ascii("text:p"));
            rendEl(ascii("style:header-left"));
        }
        if( !pPage->header && !pPage->header_even && !pPage->header_odd )
        {
            rstartEl(ascii("style:header"), rList);
            padd(ascii("text:style-name"), sXML_CDATA, ascii("Standard"));
            rstartEl(ascii("text:p"), rList);
            pList->clear();
            if( pPage->pagenumber && pPage->pagenumber->where < 4
                && pPage->pagenumber->where == 7 )
            {
                d->pPn = pPage->pagenumber;
                makeShowPageNum();
                d->pPn = 0L;
            }
            rendEl(ascii("text:p"));
            rendEl(ascii("style:header"));
        }
// ------------- footer ------------- //
        if( pPage->footer )
        {
            rstartEl(ascii("style:footer"), rList);
            if( pPage->pagenumber && pPage->pagenumber->where >= 4
                && pPage->pagenumber->where != 7 )
            {
                d->bInHeader = sal_True;
                d->pPn = pPage->pagenumber;
            }
            parsePara(pPage->footer->plist.first());
            d->bInHeader = sal_False;
            d->pPn = 0L;
            rendEl(ascii("style:footer"));
        }
        if( pPage->footer_even )
        {
            rstartEl(ascii("style:footer"), rList);
            if( pPage->pagenumber && pPage->pagenumber->where >= 4
                && pPage->pagenumber->where != 7 )
            {
                d->bInHeader = sal_True;
                d->pPn = pPage->pagenumber;
                d->nPnPos = 3;
            }
            parsePara(pPage->footer_even->plist.first());
            d->bInHeader = sal_False;
            d->pPn = 0L;
            d->nPnPos = 0;
            rendEl(ascii("style:footer"));
        }
                                                  /* ???????? ????. */
        else if( pPage->footer_odd && !pPage->footer_even )
        {
            rstartEl(ascii("style:footer"), rList);
            padd(ascii("text:style-name"), sXML_CDATA, ascii("Standard"));
            rstartEl(ascii("text:p"), rList);
            pList->clear();
            if( pPage->pagenumber && pPage->pagenumber->where >= 4
                && pPage->pagenumber->where != 7 )
            {
                d->pPn = pPage->pagenumber;
                d->nPnPos = 3;
                makeShowPageNum();
                d->pPn = 0L;
                d->nPnPos = 0;
            }
            rendEl(ascii("text:p"));
            rendEl(ascii("style:footer"));
        }
        if( pPage->footer_odd )
        {
            rstartEl(ascii("style:footer-left"), rList);
            if( pPage->pagenumber && pPage->pagenumber->where >= 4
                && pPage->pagenumber->where != 7 )
            {
                d->bInHeader = sal_True;
                d->pPn = pPage->pagenumber;
                d->nPnPos = 1;
            }
            parsePara(pPage->footer_odd->plist.first());
            d->bInHeader = sal_False;
            d->pPn = 0L;
            d->nPnPos = 0;
            rendEl(ascii("style:footer-left"));
        }
                                                  /* ???????? ????. */
        else if( pPage->footer_even && !pPage->footer_odd )
        {
            rstartEl(ascii("style:footer-left"), rList);
            padd(ascii("text:style-name"), sXML_CDATA, ascii("Standard"));
            rstartEl(ascii("text:p"), rList);
            pList->clear();
            if( pPage->pagenumber && pPage->pagenumber->where >= 4
                && pPage->pagenumber->where != 7 )
            {
                d->pPn = pPage->pagenumber;
                d->nPnPos = 1;
                makeShowPageNum();
                d->pPn = 0L;
                d->nPnPos = 0;
            }
            rendEl(ascii("text:p"));
            rendEl(ascii("style:footer-left"));
        }
        if( !pPage->footer && !pPage->footer_even && !pPage->footer_odd )
        {
            rstartEl(ascii("style:footer"), rList);
            padd(ascii("text:style-name"), sXML_CDATA, ascii("Standard"));
            rstartEl(ascii("text:p"), rList);
            pList->clear();
            if( pPage->pagenumber && pPage->pagenumber->where >= 4
                && pPage->pagenumber->where != 7 )
            {
                d->pPn = pPage->pagenumber;
                makeShowPageNum();
                d->pPn = 0L;
            }
            rendEl(ascii("text:p"));
            rendEl(ascii("style:footer"));
        }

        rendEl(ascii("style:master-page"));
    }
    rendEl(ascii("office:master-styles"));
}


/**
 * ?????? ???????? ???? ???????????? ??????.
 * 1. fo:font-size, fo:font-family, fo:letter-spacing, fo:color,
 *    style:text-background-color, fo:font-style, fo:font-weight,
 *    style:text-underline,style:text-outline,fo:text-shadow,style:text-position
 *    ?? ????????.
 */
void HwpReader::parseCharShape(CharShape * cshape)
{
    HWPFont *hwpfont = hwpfile.GetHWPFont();

    padd(ascii("fo:font-size"), sXML_CDATA,
        ascii(Int2Str(cshape->size / 25, "%dpt", buf)));
    padd(ascii("style:font-size-asian"), sXML_CDATA,
        ascii(Int2Str(cshape->size / 25, "%dpt", buf)));

    char tmp[128];
    hstr2ksstr(kstr2hstr((unsigned char *) hwpfont->GetFontName(0, cshape->font[0]), sbuf), tmp);
    double fRatio = 1.0;
    int size = getRepFamilyName(tmp,buf, fRatio);

    padd(ascii("fo:font-family"), sXML_CDATA,
        OUString(buf, size, RTL_TEXTENCODING_EUC_KR));
    padd(ascii("style:font-family-asian"), sXML_CDATA,
        OUString(buf, size, RTL_TEXTENCODING_EUC_KR));

    padd(ascii("style:text-scale"), sXML_CDATA,
        ascii(Int2Str((int)(cshape->ratio[0] * fRatio), "%d%%", buf)));

    double sspace = (cshape->size / 25) * cshape->space[0] / 100.;

    if (sspace != 0.)
    {
        padd(ascii("fo:letter-spacing"), sXML_CDATA,
            Double2Str(sspace) + ascii("pt"));
    }
    if (cshape->color[1] != 0)
        padd(ascii("fo:color"), sXML_CDATA,
            ascii(hcolor2str(cshape->color[1], 100, buf, true)));
    if (cshape->shade != 0)
        padd(ascii("style:text-background-color"), sXML_CDATA,
            ascii(hcolor2str(cshape->color[0], cshape->shade, buf)));
    if (cshape->attr & 0x01)
    {
        padd(ascii("fo:font-style"), sXML_CDATA, ascii("italic"));
        padd(ascii("style:font-style-asian"), sXML_CDATA, ascii("italic"));
    }
     else{
        padd(ascii("fo:font-style"), sXML_CDATA, ascii("normal"));
        padd(ascii("style:font-style-asian"), sXML_CDATA, ascii("normal"));
     }
    if (cshape->attr >> 1 & 0x01)
    {
        padd(ascii("fo:font-weight"), sXML_CDATA, ascii("bold"));
        padd(ascii("style:font-weight-asian"), sXML_CDATA, ascii("bold"));
    }
     else{
        padd(ascii("fo:font-weight"), sXML_CDATA, ascii("normal"));
        padd(ascii("style:font-weight-asian"), sXML_CDATA, ascii("normal"));
     }
    if (cshape->attr >> 2 & 0x01)
        padd(ascii("style:text-underline"), sXML_CDATA, ascii("single"));
    if (cshape->attr >> 3 & 0x01)
        padd(ascii("style:text-outline"), sXML_CDATA, ascii("true"));
    if (cshape->attr >> 4 & 0x01)
        padd(ascii("fo:text-shadow"), sXML_CDATA, ascii("1pt 1pt"));
    if (cshape->attr >> 5 & 0x01)
        padd(ascii("style:text-position"), sXML_CDATA, ascii("super 58%"));
    if (cshape->attr >> 6 & 0x01)
        padd(ascii("style:text-position"), sXML_CDATA, ascii("sub 58%"));

}


/**
 * ???? Paragraph?? ???????? properties???? ??????.
 * 1. fo:margin-left,fo:margin-right,fo:margin-top, fo:margin-bottom,
 *    fo:text-indent, fo:line-height, fo:text-align, fo:border
 *    ?? ??????.
 * TODO : ?????? => ???????? ?????????? ?????????? ???????? ????.
 */
void HwpReader::parseParaShape(ParaShape * pshape)
{

    if (pshape->left_margin != 0)
        padd(ascii("fo:margin-left"), sXML_CDATA, Double2Str
            (WTI(pshape->left_margin )) + ascii("inch"));
    if (pshape->right_margin != 0)
        padd(ascii("fo:margin-right"), sXML_CDATA, Double2Str
            (WTI(pshape->right_margin)) + ascii("inch"));
    if (pshape->pspacing_prev != 0)
        padd(ascii("fo:margin-top"), sXML_CDATA, Double2Str
            (WTI(pshape->pspacing_prev)) + ascii("inch"));
    if (pshape->pspacing_next != 0)
        padd(ascii("fo:margin-bottom"), sXML_CDATA, Double2Str
            (WTI(pshape->pspacing_next)) + ascii("inch"));
    if (pshape->indent != 0)
        padd(ascii("fo:text-indent"), sXML_CDATA, Double2Str
            (WTI(pshape->indent)) + ascii("inch"));
    if (pshape->lspacing != 0)
        padd(ascii("fo:line-height"), sXML_CDATA,
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
        padd(ascii("fo:text-align"), sXML_CDATA, ascii(buf));

    if (pshape->outline)
        padd(ascii("fo:border"), sXML_CDATA, ascii("0.002cm solid #000000"));
    if( pshape->shade > 0 )
    {
        padd(ascii("fo:background-color"), sXML_CDATA,
            ascii(hcolor2str(0, pshape->shade, buf)));
    }

    if( pshape->pagebreak & 0x02 || pshape->pagebreak & 0x04)
        padd(ascii("fo:break-before"), sXML_CDATA, ascii("page"));
     else if( pshape->pagebreak & 0x01 )
        padd(ascii("fo:break-before"), sXML_CDATA, ascii("column"));

}


/**
 * Paragraph?? ???? ???????? ??????.
 */
void HwpReader::makePStyle(ParaShape * pshape)
{
    unsigned char tf = 0;
    int nscount = pshape->tabs[MAXTABS -1].type;
    padd(ascii("style:name"), sXML_CDATA,
        ascii(Int2Str(pshape->index, "P%d", buf)));
    padd(ascii("style:family"), sXML_CDATA, ascii("paragraph"));
    rstartEl(ascii("style:style"), rList);
    pList->clear();
    parseParaShape(pshape);
    parseCharShape(pshape->cshape);
    rstartEl(ascii("style:properties"), rList);
    pList->clear();

    if( nscount )
    {
        rstartEl(ascii("style:tab-stops"),rList);

          int tab_margin = pshape->left_margin + pshape->indent;
          if( tab_margin < 0 )
              tab_margin = 0;
        for( int i = 0 ; i < MAXTABS -1 ; i++)
        {
            if( i > 0 && pshape->tabs[i].position == 0. )
                break;
                if( pshape->tabs[i].position <= tab_margin )
                    continue;
            padd(ascii("style:position"), sXML_CDATA,
                Double2Str(WTMM(pshape->tabs[i].position - tab_margin )) + ascii("mm"));
            if( pshape->tabs[i].type )
            {
                tf = 1;
                switch(pshape->tabs[i].type)
                {
                    case 1 :
                        padd(ascii("style:type"), sXML_CDATA, ascii("right"));
                        break;
                    case 2:
                        padd(ascii("style:type"), sXML_CDATA, ascii("center"));
                        break;
                    case 3:
                        padd(ascii("style:type"), sXML_CDATA, ascii("char"));
                        padd(ascii("style:char"), sXML_CDATA, ascii("."));
                        break;
                }
            }
            if( pshape->tabs[i].dot_continue )
            {
                tf = 1;
                padd(ascii("style:leader-char"), sXML_CDATA, ascii("."));
            }
            rstartEl( ascii("style:tab-stop"), rList);
            pList->clear();
            rendEl( ascii("style:tab-stop") );

            if( (pshape->tabs[i].position != 1000 * i ) || tf )
            {
                if( !--nscount ) break;
            }
        }
        rendEl( ascii("style:tab-stops"));
    }
    rendEl(ascii("style:properties"));
    rendEl(ascii("style:style"));
}


/**
 * ???????? ???????? ??????. ???????? header/footer, footnote???? ????????.
 * TODO : , fo:background-color(?????? ????)
 */
void HwpReader::makePageStyle()
{
    HWPInfo *hwpinfo = hwpfile.GetHWPInfo();
     int pmCount = hwpfile.getColumnCount();

     for( int i = 0 ; i < pmCount ; i++ ){
         padd(ascii("style:name"), sXML_CDATA, ascii(Int2Str(i + 1, "pm%d", buf)));
         rstartEl(ascii("style:page-master"),rList);
         pList->clear();


         switch( hwpinfo->paper.paper_kind )
         {
              case 3:                                   // A4
                    if( hwpinfo->paper.paper_direction )
                    {
                         padd(ascii("fo:page-height"),sXML_CDATA, ascii("210mm"));
                         padd(ascii("fo:page-width"),sXML_CDATA, ascii("297mm"));
                    }
                    else
                    {
                         padd(ascii("fo:page-width"),sXML_CDATA, ascii("210mm"));
                         padd(ascii("fo:page-height"),sXML_CDATA, ascii("297mm"));
                    }
                    break;
              case 4:                                   // 80 column
                    if( hwpinfo->paper.paper_direction )
                    {
                         padd(ascii("fo:page-height"),sXML_CDATA, ascii("8.5inch"));
                         padd(ascii("fo:page-width"),sXML_CDATA, ascii("11inch"));
                    }
                    else
                    {
                         padd(ascii("fo:page-width"),sXML_CDATA, ascii("8.5inch"));
                         padd(ascii("fo:page-height"),sXML_CDATA, ascii("11inch"));
                    }
                    break;
              case 5:                                   // B5
                    if( hwpinfo->paper.paper_direction )
                    {
                         padd(ascii("fo:page-height"),sXML_CDATA, ascii("176mm"));
                         padd(ascii("fo:page-width"),sXML_CDATA, ascii("250mm"));
                    }
                    else
                    {
                         padd(ascii("fo:page-width"),sXML_CDATA, ascii("176mm"));
                         padd(ascii("fo:page-height"),sXML_CDATA, ascii("250mm"));
                    }
                    break;
              case 6:                                   // B4
                    if( hwpinfo->paper.paper_direction )
                    {
                         padd(ascii("fo:page-height"),sXML_CDATA, ascii("250mm"));
                         padd(ascii("fo:page-width"),sXML_CDATA, ascii("353mm"));
                    }
                    else
                    {
                         padd(ascii("fo:page-width"),sXML_CDATA, ascii("250mm"));
                         padd(ascii("fo:page-height"),sXML_CDATA, ascii("353mm"));
                    }
                    break;
              case 7:
                    if( hwpinfo->paper.paper_direction )
                    {
                         padd(ascii("fo:page-height"),sXML_CDATA, ascii("8.5inch"));
                         padd(ascii("fo:page-width"),sXML_CDATA, ascii("14inch"));
                    }
                    else
                    {
                         padd(ascii("fo:page-width"),sXML_CDATA, ascii("8.5inch"));
                         padd(ascii("fo:page-height"),sXML_CDATA, ascii("14inch"));
                    }
                    break;
              case 8:
                    if( hwpinfo->paper.paper_direction )
                    {
                         padd(ascii("fo:page-height"),sXML_CDATA, ascii("297mm"));
                         padd(ascii("fo:page-width"),sXML_CDATA, ascii("420mm"));
                    }
                    else
                    {
                         padd(ascii("fo:page-width"),sXML_CDATA, ascii("297mm"));
                         padd(ascii("fo:page-height"),sXML_CDATA, ascii("420mm"));
                    }
                    break;
              case 0:
              case 1:
              case 2:
              default:
                    if( hwpinfo->paper.paper_direction )
                    {
                         padd(ascii("fo:page-width"),sXML_CDATA,
                              Double2Str(WTI(hwpinfo->paper.paper_height)) + ascii("inch"));
                         padd(ascii("fo:page-height"),sXML_CDATA,
                              Double2Str(WTI(hwpinfo->paper.paper_width)) + ascii("inch"));
                    }
                    else
                    {
                         padd(ascii("fo:page-width"),sXML_CDATA,
                              Double2Str(WTI(hwpinfo->paper.paper_width)) + ascii("inch"));
                         padd(ascii("fo:page-height"),sXML_CDATA,
                              Double2Str(WTI(hwpinfo->paper.paper_height)) + ascii("inch"));
                    }
                    break;

         }

         padd(ascii("style:print-orientation"),sXML_CDATA,
              ascii(hwpinfo->paper.paper_direction ? "landscape" : "portrait"));
         if( hwpinfo->beginpagenum != 1)
              padd(ascii("style:first-page-number"),sXML_CDATA,
                    ascii(Int2Str(hwpinfo->beginpagenum, "%d", buf)));

         if( hwpinfo->borderline ){
             padd(ascii("fo:margin-left"),sXML_CDATA,
                  Double2Str(WTI(hwpinfo->paper.left_margin - hwpinfo->bordermargin[0] + hwpinfo->paper.gutter_length)) + ascii("inch"));
             padd(ascii("fo:margin-right"),sXML_CDATA,
                  Double2Str(WTI(hwpinfo->paper.right_margin - hwpinfo->bordermargin[1])) + ascii("inch"));
             padd(ascii("fo:margin-top"),sXML_CDATA,
                  Double2Str(WTI(hwpinfo->paper.top_margin - hwpinfo->bordermargin[2])) + ascii("inch"));
             padd(ascii("fo:margin-bottom"),sXML_CDATA,
                  Double2Str(WTI(hwpinfo->paper.bottom_margin - hwpinfo->bordermargin[3])) + ascii("inch"));
         }
         else{
             padd(ascii("fo:margin-left"),sXML_CDATA,
                  Double2Str(WTI(hwpinfo->paper.left_margin + hwpinfo->paper.gutter_length)) + ascii("inch"));
             padd(ascii("fo:margin-right"),sXML_CDATA,
                  Double2Str(WTI(hwpinfo->paper.right_margin)) + ascii("inch"));
             padd(ascii("fo:margin-top"),sXML_CDATA,
                  Double2Str(WTI(hwpinfo->paper.top_margin)) + ascii("inch"));
             padd(ascii("fo:margin-bottom"),sXML_CDATA,
                  Double2Str(WTI(hwpinfo->paper.bottom_margin)) + ascii("inch"));
         }

         switch( hwpinfo->borderline )
         {
              case 1:
                    padd(ascii("fo:border"), sXML_CDATA,ascii("0.002cm solid #000000"));
                    break;
              case 3:
                    padd(ascii("fo:border"), sXML_CDATA,ascii("0.002cm dotted #000000"));
                    break;
              case 2:
                    padd(ascii("fo:border"), sXML_CDATA,ascii("0.035cm solid #000000"));
                    break;
              case 4:
                    padd(ascii("style:border-line-width"), sXML_CDATA,ascii("0.002cm 0.035cm 0.002cm"));
                    padd(ascii("fo:border"), sXML_CDATA,ascii("0.039cm double #000000"));
                    break;
         }

         padd(ascii("fo:padding-left"), sXML_CDATA,
              Double2Str(WTI(hwpinfo->bordermargin[0])) + ascii("inch"));
         padd(ascii("fo:padding-right"), sXML_CDATA,
              Double2Str(WTI(hwpinfo->bordermargin[1])) + ascii("inch"));
         padd(ascii("fo:padding-top"), sXML_CDATA,
              Double2Str(WTI(hwpinfo->bordermargin[2])) + ascii("inch"));
         padd(ascii("fo:padding-bottom"), sXML_CDATA,
              Double2Str(WTI(hwpinfo->bordermargin[3])) + ascii("inch"));

     /* background color */
         if( hwpinfo->back_info.isset )
         {
             if( hwpinfo->back_info.color[0] > 0 || hwpinfo->back_info.color[1] > 0
                     || hwpinfo->back_info.color[2] > 0 ){
                 sprintf(buf,"#%02x%02x%02x",hwpinfo->back_info.color[0],
                         hwpinfo->back_info.color[1],hwpinfo->back_info.color[2] );
                 padd(ascii("fo:background-color"), sXML_CDATA, ascii(buf));
             }
         }

         rstartEl(ascii("style:properties"),rList);
         pList->clear();

     /* background image */
         if( hwpinfo->back_info.isset && hwpinfo->back_info.type > 0 )
         {
             if( hwpinfo->back_info.type == 1 ){
#ifdef _WIN32
                 padd(ascii("xlink:href"), sXML_CDATA,
                      hconv(kstr2hstr((uchar *) urltowin(hwpinfo->back_info.filename, buf), sbuf), gstr));
#else
                 padd(ascii("xlink:href"), sXML_CDATA,
                    hconv(kstr2hstr( (uchar *)urltounix(hwpinfo->back_info.filename, buf), sbuf), gstr));
#endif
                 padd(ascii("xlink:type"), sXML_CDATA, ascii("simple"));
                 padd(ascii("xlink:actuate"), sXML_CDATA, ascii("onLoad"));
             }
             if( hwpinfo->back_info.flag >= 2)
                 padd(ascii("style:repeat"), sXML_CDATA, ascii("stretch"));
             else if( hwpinfo->back_info.flag == 1 ){
                 padd(ascii("style:repeat"), sXML_CDATA, ascii("no-repeat"));
                 padd(ascii("style:position"), sXML_CDATA, ascii("center"));
             }
             rstartEl(ascii("style:background-image"),rList);

             if( hwpinfo->back_info.type == 2 ){
                 rstartEl(ascii("office:binary-data"), rList);
                 pList->clear();
                 rchars(ascii(base64_encode_string((unsigned char *) hwpinfo->back_info.data, hwpinfo->back_info.size )));
                 rendEl(ascii("office:binary-data"));
             }
             rendEl(ascii("style:background-image"));
         }

         makeColumns( hwpfile.GetColumnDef(i) );

         rendEl(ascii("style:properties"));

    /* header style */
         rstartEl(ascii("style:header-style"), rList);
         padd(ascii("svg:height"), sXML_CDATA,
              Double2Str(WTI(hwpinfo->paper.header_length)) + ascii("inch"));
         padd(ascii("fo:margin-bottom"), sXML_CDATA, ascii("0mm"));

         rstartEl(ascii("style:properties"),rList);
         pList->clear();
         rendEl(ascii("style:properties"));
         rendEl(ascii("style:header-style"));

    /* footer style */
         rstartEl(ascii("style:footer-style"), rList);
         padd(ascii("svg:height"), sXML_CDATA,
              Double2Str(WTI(hwpinfo->paper.footer_length)) + ascii("inch"));
         padd(ascii("fo:margin-top"), sXML_CDATA, ascii("0mm"));
         rstartEl(ascii("style:properties"),rList);
         pList->clear();
         rendEl(ascii("style:properties"));
         rendEl(ascii("style:footer-style"));

    /* footnote style ???? dtd?????? ????????, ???????? ???????? ????. REALKING */
         rstartEl(ascii("style:footnote-layout"), rList);

         padd(ascii("style:distance-before-sep"), sXML_CDATA,
              Double2Str(WTI(hwpinfo->splinetext)) + ascii("inch"));
         padd(ascii("style:distance-after-sep"), sXML_CDATA,
              Double2Str(WTI(hwpinfo->splinefn)) + ascii("inch"));
         rstartEl(ascii("style:properties"),rList);
         pList->clear();
         rendEl(ascii("style:properties"));
         if ( hwpinfo->fnlinetype == 2 )
              padd(ascii("style:width"), sXML_CDATA, ascii("15cm"));
         else if ( hwpinfo->fnlinetype == 1)
              padd(ascii("style:width"), sXML_CDATA, ascii("2cm"));
         else if ( hwpinfo->fnlinetype == 3)
              padd(ascii("style:width"), sXML_CDATA, ascii("0cm"));
         else
              padd(ascii("style:width"), sXML_CDATA, ascii("5cm"));

         rstartEl(ascii("style:footnote-sep"),rList);
         pList->clear();
         rendEl(ascii("style:footnote-sep"));

         rendEl(ascii("style:footnote-layout"));

         rendEl(ascii("style:page-master"));
     }
}

void HwpReader::makeColumns(ColumnDef *coldef)
{
    if( !coldef ) return;
  padd(ascii("fo:column-count"), sXML_CDATA, ascii(Int2Str(coldef->ncols, "%d", buf)));
  rstartEl(ascii("style:columns"),rList);
  pList->clear();
  if( coldef->separator != 0 )
  {
        switch( coldef->separator )
        {
             case 1:                           /* ?????? */
                  padd(ascii("style:width"), sXML_CDATA, ascii("0.02mm"));
             case 3:                           /* ???? */
                  padd(ascii("style:style"), sXML_CDATA, ascii("dotted"));
                  padd(ascii("style:width"), sXML_CDATA, ascii("0.02mm"));
                  break;
             case 2:                           /* ???????? */
             case 4:                           /* 2???? */
                  padd(ascii("style:width"), sXML_CDATA, ascii("0.35mm"));
                  break;
             case 0:                           /* ???? */
             default:
                  padd(ascii("style:style"), sXML_CDATA, ascii("none"));
                  break;
        }
        rstartEl(ascii("style:column-sep"),rList);
        pList->clear();
        rendEl(ascii("style:column-sep"));
  }
  double spacing = WTI(coldef->spacing)/ 2. ;
  for(int ii = 0 ; ii < coldef->ncols ; ii++)
  {
        if( ii == 0 )
             padd(ascii("fo:margin-left"), sXML_CDATA, ascii("0mm"));
        else
             padd(ascii("fo:margin-left"), sXML_CDATA,
                  Double2Str( spacing) + ascii("inch"));
        if( ii == ( coldef->ncols -1) )
             padd(ascii("fo:margin-right"), sXML_CDATA,ascii("0mm"));
        else
             padd(ascii("fo:margin-right"), sXML_CDATA,
                  Double2Str( spacing) + ascii("inch"));
        rstartEl(ascii("style:column"),rList);
        pList->clear();
        rendEl(ascii("style:column"));
  }
  rendEl(ascii("style:columns"));
}

void HwpReader::makeTStyle(CharShape * cshape)
{
    padd(ascii("style:name"), sXML_CDATA,
        ascii(Int2Str(cshape->index, "T%d", buf)));
    padd(ascii("style:family"), sXML_CDATA, ascii("text"));
    rstartEl(ascii("style:style"), rList);
    pList->clear();
    parseCharShape(cshape);
    rstartEl(ascii("style:properties"), rList);
    pList->clear();
    rendEl(ascii("style:properties"));
    rendEl(ascii("style:style"));
}


void HwpReader::makeTableStyle(Table *tbl)
{
// --------------- table ---------------- //
    TxtBox *hbox = tbl->box;

    padd(ascii("style:name"), sXML_CDATA,
        ascii(Int2Str(hbox->style.boxnum, "Table%d", buf)));
    padd(ascii("style:family"), sXML_CDATA,ascii("table"));
    rstartEl(ascii("style:style"), rList);
    pList->clear();
    padd(ascii("style:width"), sXML_CDATA,
        Double2Str(WTMM(hbox->box_xs)) + ascii("mm"));
    padd(ascii("table:align"), sXML_CDATA,ascii("left"));
    padd(ascii("fo:keep-with-next"), sXML_CDATA,ascii("false"));
    rstartEl(ascii("style:properties"), rList);
    pList->clear();
    rendEl(ascii("style:properties"));
    rendEl(ascii("style:style"));

// --------------- column ---------------- //
    int i ;
    for( i = 0 ; i < tbl->columns.nCount -1 ; i++ )
    {
        sprintf(buf,"Table%d.%c",hbox->style.boxnum, 'A'+i);
        padd(ascii("style:name"), sXML_CDATA, ascii( buf ));
        padd(ascii("style:family"), sXML_CDATA,ascii("table-column"));
        rstartEl(ascii("style:style"), rList);
        pList->clear();
        padd(ascii("style:column-width"), sXML_CDATA,
            Double2Str(WTMM(tbl->columns.data[i+1] - tbl->columns.data[i])) + ascii("mm"));
        rstartEl(ascii("style:properties"), rList);
        pList->clear();
        rendEl(ascii("style:properties"));
        rendEl(ascii("style:style"));
    }

// --------------- row ---------------- //
    for( i = 0 ; i < tbl->rows.nCount -1 ; i++ )
    {
        sprintf(buf,"Table%d.row%d",hbox->style.boxnum, i + 1);
        padd(ascii("style:name"), sXML_CDATA, ascii( buf ));
        padd(ascii("style:family"), sXML_CDATA,ascii("table-row"));
        rstartEl(ascii("style:style"), rList);
        pList->clear();
        padd(ascii("style:row-height"), sXML_CDATA,
            Double2Str(WTMM(tbl->rows.data[i+1] - tbl->rows.data[i])) + ascii("mm"));
        rstartEl(ascii("style:properties"), rList);
        pList->clear();
        rendEl(ascii("style:properties"));
        rendEl(ascii("style:style"));
    }

// --------------- cell --------------------- //
    for( i = 0 ; i < tbl->cells.count(); i++ )
    {
        TCell *tcell = tbl->cells.find(i);
        sprintf(buf,"Table%d.%c%d",hbox->style.boxnum, 'A'+ tcell->nColumnIndex, tcell->nRowIndex +1);
        padd(ascii("style:name"), sXML_CDATA, ascii( buf ));
        padd(ascii("style:family"), sXML_CDATA,ascii("table-cell"));
        rstartEl(ascii("style:style"), rList);
        pList->clear();
        Cell *cl = tcell->pCell;
        if( cl->ver_align == 1 )
            padd(ascii("fo:vertical-align"), sXML_CDATA,ascii("middle"));

        if(cl->linetype[2] == cl->linetype[3] && cl->linetype[2] == cl->linetype[0]
            && cl->linetype[2] == cl->linetype[1])
        {
            switch( cl->linetype[2] )
            {
                case 1:                           /* ???????? */
                case 3:                           /* ???? -> ?????????????? ?????? ????. */
                    padd(ascii("fo:border"), sXML_CDATA,ascii("0.002cm solid #000000"));
                    break;
                case 2:                           /* ???????? */
                    padd(ascii("fo:border"), sXML_CDATA,ascii("0.035cm solid #000000"));
                    break;
                case 4:                           /* 2???? */
                    padd(ascii("style:border-line-width"), sXML_CDATA,ascii("0.002cm 0.035cm 0.002cm"));
                    padd(ascii("fo:border"), sXML_CDATA,ascii("0.039cm double #000000"));
                    break;
            }
        }
        else
        {
            switch( cl->linetype[0] )
            {
                case 1:                           /* ???????? */
                case 3:                           /* ???? -> ?????????????? ?????? ????. */
                    padd(ascii("fo:border-left"), sXML_CDATA,ascii("0.002cm solid #000000"));
                    break;
                case 2:                           /* ???????? */
                    padd(ascii("fo:border-left"), sXML_CDATA,ascii("0.035cm solid #000000"));
                    break;
                case 4:                           /* 2???? */
                    padd(ascii("style:border-line-width-left"), sXML_CDATA,ascii("0.002cm 0.035cm 0.002cm"));
                    padd(ascii("fo:border-left"), sXML_CDATA,ascii("0.039cm double #000000"));
                    break;
            }
            switch( cl->linetype[1] )
            {
                case 1:                           /* ???????? */
                case 3:                           /* ???? -> ?????????????? ?????? ????. */
                    padd(ascii("fo:border-right"), sXML_CDATA,ascii("0.002cm solid #000000"));
                    break;
                case 2:                           /* ???????? */
                    padd(ascii("fo:border-right"), sXML_CDATA,ascii("0.035cm solid #000000"));
                    break;
                case 4:                           /* 2???? */
                    padd(ascii("style:border-line-width-right"), sXML_CDATA,ascii("0.002cm 0.035cm 0.002cm"));
                    padd(ascii("fo:border-right"), sXML_CDATA,ascii("0.039cm double #000000"));
                    break;
            }
            switch( cl->linetype[2] )
            {
                case 1:                           /* ???????? */
                case 3:                           /* ???? -> ?????????????? ?????? ????. */
                    padd(ascii("fo:border-top"), sXML_CDATA,ascii("0.002cm solid #000000"));
                    break;
                case 2:                           /* ???????? */
                    padd(ascii("fo:border-top"), sXML_CDATA,ascii("0.035cm solid #000000"));
                    break;
                case 4:                           /* 2???? */
                    padd(ascii("style:border-line-width-top"), sXML_CDATA,ascii("0.002cm 0.035cm 0.002cm"));
                    padd(ascii("fo:border-top"), sXML_CDATA,ascii("0.039cm double #000000"));
                    break;
            }
            switch( cl->linetype[3] )
            {
                case 1:                           /* ???????? */
                case 3:                           /* ???? -> ?????????????? ?????? ????. */
                    padd(ascii("fo:border-bottom"), sXML_CDATA,ascii("0.002cm solid #000000"));
                    break;
                case 2:                           /* ???????? */
                    padd(ascii("fo:border-bottom"), sXML_CDATA,ascii("0.035cm solid #000000"));
                    break;
                case 4:                           /* 2???? */
                    padd(ascii("style:border-line-width-bottom"), sXML_CDATA,ascii("0.002cm 0.035cm 0.002cm"));
                    padd(ascii("fo:border-bottom"), sXML_CDATA,ascii("0.039cm double #000000"));
                    break;
            }
        }
        if(cl->shade != 0)
            padd(ascii("fo:background-color"), sXML_CDATA, 
                ascii(hcolor2str(sal::static_int_cast<uchar>(cl->color), 
                                sal::static_int_cast<uchar>(cl->shade), buf)));

        rstartEl(ascii("style:properties"), rList);
        pList->clear();
        rendEl(ascii("style:properties"));

        rendEl(ascii("style:style"));
    }
}


void HwpReader::makeDrawStyle( HWPDrawingObject * hdo, FBoxStyle * fstyle)
{
    while( hdo )
    {
        padd(ascii("style:name"), sXML_CDATA,
            ascii(Int2Str(hdo->index, "Draw%d", buf)));
        padd(ascii("style:family"), sXML_CDATA, ascii("graphics"));

        rstartEl(ascii("style:style"), rList);
        pList->clear();

        switch (fstyle->txtflow)
        {
            case 0:
//padd(ascii("style:wrap"), sXML_CDATA, ascii("none"));
                break;
            case 1:
                     /*
                padd(ascii("style:run-through"), sXML_CDATA, ascii("foreground"));
                padd(ascii("style:wrap"), sXML_CDATA, ascii("parallel"));
                padd(ascii("style:number-wrapped-paragraphs"), sXML_CDATA, ascii("no-limit"));
                padd(ascii("style:wrap-contour"), sXML_CDATA, ascii("true"));
                padd(ascii("style:wrap-contour-mode"), sXML_CDATA, ascii("full"));
                     */
                padd(ascii("style:wrap"), sXML_CDATA, ascii("run-through"));
                break;
            case 2:
                padd(ascii("style:wrap"), sXML_CDATA, ascii("dynamic"));
                break;
        }
        long color;
// invisible line
        if( hdo->property.line_color > 0xffffff )
        {
            padd(ascii("draw:stroke"), sXML_CDATA, ascii("none") );
        }
        else
        {

            if( hdo->property.line_pstyle == 0 )
                padd(ascii("draw:stroke"), sXML_CDATA, ascii("solid") );
            else if( hdo->property.line_pstyle < 5 )
            {
                padd(ascii("draw:stroke"), sXML_CDATA, ascii("dash") );
                padd(ascii("draw:stroke-dash"), sXML_CDATA,
                    ascii(Int2Str(hdo->index, "LineType%d", buf)));
            }
            padd(ascii("svg:stroke-width"), sXML_CDATA,
                Double2Str( WTMM(hdo->property.line_width)) + ascii("mm" ));
            color = hdo->property.line_color;
            sprintf( buf, "#%02x%02x%02x", 
                    sal_uInt16(color & 0xff), 
                    sal_uInt16((color >> 8) & 0xff), 
                    sal_uInt16((color >>16) & 0xff) );
            padd(ascii("svg:stroke-color"), sXML_CDATA, ascii( buf) );
        }

        if( hdo->type == HWPDO_LINE || hdo->type == HWPDO_ARC ||
            hdo->type == HWPDO_FREEFORM || hdo->type == HWPDO_ADVANCED_ARC )
        {

            if( hdo->property.line_tstyle > 0 )
            {
                padd(ascii("draw:marker-start"), sXML_CDATA,
                    ascii(ArrowShape[hdo->property.line_tstyle].name) );
                     if( hdo->property.line_width > 100 )
                         padd(ascii("draw:marker-start-width"), sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 3)) + ascii("mm" ));
                     else if( hdo->property.line_width > 80 )
                         padd(ascii("draw:marker-start-width"), sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 4)) + ascii("mm" ));
                     else if( hdo->property.line_width > 60 )
                         padd(ascii("draw:marker-start-width"), sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 5)) + ascii("mm" ));
                     else if( hdo->property.line_width > 40 )
                         padd(ascii("draw:marker-start-width"), sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 6)) + ascii("mm" ));
                     else
                         padd(ascii("draw:marker-start-width"), sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 7)) + ascii("mm" ));
            }

            if( hdo->property.line_hstyle > 0 )
            {
                padd(ascii("draw:marker-end"), sXML_CDATA,
                    ascii(ArrowShape[hdo->property.line_hstyle].name) );
                     if( hdo->property.line_width > 100 )
                         padd(ascii("draw:marker-end-width"), sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 3)) + ascii("mm" ));
                     else if( hdo->property.line_width > 80 )
                         padd(ascii("draw:marker-end-width"), sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 4)) + ascii("mm" ));
                     else if( hdo->property.line_width > 60 )
                         padd(ascii("draw:marker-end-width"), sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 5)) + ascii("mm" ));
                     else if( hdo->property.line_width > 40 )
                         padd(ascii("draw:marker-end-width"), sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 6)) + ascii("mm" ));
                     else
                         padd(ascii("draw:marker-end-width"), sXML_CDATA,
                              Double2Str( WTMM(hdo->property.line_width * 7)) + ascii("mm" ));
            }
        }

        if(hdo->type != HWPDO_LINE )
        {
            if( hdo->property.flag >> 19 & 0x01 )
            {
                padd( ascii("draw:textarea-horizontal-align"), sXML_CDATA, ascii("center"));
            }

            color = hdo->property.fill_color;

            if( hdo->property.flag >> 18 & 0x01 ) // bitmap pattern
            {
                padd(ascii("draw:fill"), sXML_CDATA, ascii("bitmap"));
                padd(ascii("draw:fill-image-name"), sXML_CDATA,
                    ascii(Int2Str(hdo->index, "fillimage%d", buf)));
                                                  // bitmap resizing
                if( hdo->property.flag >> 3 & 0x01 )
                {
                    padd(ascii("style:repeat"), sXML_CDATA, ascii("stretch"));
                }
                else
                {
                    padd(ascii("style:repeat"), sXML_CDATA, ascii("repeat"));
                    padd(ascii("draw:fill-image-ref-point"), sXML_CDATA, ascii("top-left"));
                }
                if( hdo->property.flag >> 20 & 0x01 )
                {
                    if( hdo->property.luminance > 0 )
                    {
                        padd(ascii("draw:transparency"), sXML_CDATA,
                            ascii(Int2Str(hdo->property.luminance, "%d%%", buf)));
                    }
                }

            }
                                                  // Gradation
            else if( hdo->property.flag >> 16 & 0x01 )
            {
                padd(ascii("draw:fill"), sXML_CDATA, ascii("gradient"));
                padd(ascii("draw:fill-gradient-name"), sXML_CDATA,
                    ascii(Int2Str(hdo->index, "Grad%d", buf)));
                padd(ascii("draw:gradient-step-count"), sXML_CDATA,
                    ascii(Int2Str(hdo->property.nstep, "%d", buf)));

            }
                                                  // Hatching
            else if( hdo->property.pattern_type >> 24 & 0x01 )
            {
                padd(ascii("draw:fill"), sXML_CDATA, ascii("hatch"));
                padd(ascii("draw:fill-hatch-name"), sXML_CDATA,
                    ascii(Int2Str(hdo->index, "Hatch%d", buf)));
                if( color < 0xffffff )
                {
                    sprintf( buf, "#%02x%02x%02x",
                        sal_uInt16(color & 0xff), 
                        sal_uInt16((color >> 8) & 0xff), 
                        sal_uInt16((color >>16) & 0xff) );
                    padd(ascii("draw:fill-color"), sXML_CDATA, ascii( buf) );
                    padd(ascii("draw:fill-hatch-solid"), sXML_CDATA, ascii("true"));
                }
            }
            else if( color <= 0xffffff )
            {
                padd(ascii("draw:fill"), sXML_CDATA, ascii("solid"));
                sprintf( buf, "#%02x%02x%02x",
                    sal_uInt16(color & 0xff), 
                    sal_uInt16((color >> 8) & 0xff), 
                    sal_uInt16((color >>16) & 0xff) );
                padd(ascii("draw:fill-color"), sXML_CDATA, ascii( buf) );
            }
            else
                padd(ascii("draw:fill"), sXML_CDATA, ascii("none"));
        }

        if( fstyle->anchor_type == CHAR_ANCHOR )
        {
            padd(ascii("style:vertical-pos"), sXML_CDATA, ascii("top"));
            padd(ascii("style:vertical-rel"), sXML_CDATA, ascii("baseline"));
        }

        rstartEl(ascii("style:properties"), rList);
        pList->clear();
        rendEl(ascii("style:properties"));
        rendEl(ascii("style:style"));

        if( hdo->type == 0 )
        {
            makeDrawStyle( hdo->child, fstyle );
        }
        hdo = hdo->next;
    }
}


void HwpReader::makeCaptionStyle(FBoxStyle * fstyle)
{
    padd(ascii("style:name"), sXML_CDATA,
        ascii(Int2Str(fstyle->boxnum, "CapBox%d", buf)));
    padd(ascii("style:family"), sXML_CDATA, ascii("graphics"));
    rstartEl(ascii("style:style"), rList);
    pList->clear();
    padd(ascii("fo:margin-left"), sXML_CDATA, ascii("0cm"));
    padd(ascii("fo:margin-right"), sXML_CDATA, ascii("0cm"));
    padd(ascii("fo:margin-top"), sXML_CDATA, ascii("0cm"));
    padd(ascii("fo:margin-bottom"), sXML_CDATA, ascii("0cm"));
    padd(ascii("fo:padding"), sXML_CDATA, ascii("0cm"));
    switch (fstyle->txtflow)
    {
        case 0:
            padd(ascii("style:wrap"), sXML_CDATA, ascii("none"));
            break;
        case 1:
            if( fstyle->boxtype == 'G' )
                padd(ascii("style:run-through"), sXML_CDATA, ascii("background"));
            padd(ascii("style:wrap"), sXML_CDATA, ascii("run-through"));
            break;
        case 2:
            padd(ascii("style:wrap"), sXML_CDATA, ascii("dynamic"));
            break;
    }
    if (fstyle->anchor_type == CHAR_ANCHOR)
    {
        padd(ascii("style:vertical-pos"), sXML_CDATA, ascii("top"));
        padd(ascii("style:vertical-rel"), sXML_CDATA, ascii("baseline"));
        padd(ascii("style:horizontal-pos"), sXML_CDATA, ascii("center"));
        padd(ascii("style:horizontal-rel"), sXML_CDATA, ascii("paragraph"));
    }
    else
    {

        switch (-(fstyle->xpos))
        {
            case 2:
                padd(ascii("style:horizontal-pos"), sXML_CDATA, ascii("right"));
                break;
            case 3:
                padd(ascii("style:horizontal-pos"), sXML_CDATA, ascii("center"));
                break;
            case 1:
            default:
                padd(ascii("style:horizontal-pos"), sXML_CDATA, ascii("from-left"));
                break;
        }
        switch (-(fstyle->ypos))
        {
            case 2:
                padd(ascii("style:vertical-pos"), sXML_CDATA, ascii("bottom"));
                break;
            case 3:
                padd(ascii("style:vertical-pos"), sXML_CDATA, ascii("middle"));
                break;
            case 1:
            default:
                padd(ascii("style:vertical-pos"), sXML_CDATA, ascii("from-top"));
                break;
        }
        if ( fstyle->anchor_type == PARA_ANCHOR )
        {
            padd(ascii("style:vertical-rel"), sXML_CDATA, ascii("paragraph"));
            padd(ascii("style:horizontal-rel"), sXML_CDATA, ascii("paragraph"));
        }
        else
        {
            padd(ascii("style:vertical-rel"), sXML_CDATA, ascii("page-content"));
            padd(ascii("style:horizontal-rel"), sXML_CDATA, ascii("page-content"));
        }
    }
    rstartEl(ascii("style:properties"), rList);
    pList->clear();
    rendEl(ascii("style:properties"));
    rendEl(ascii("style:style"));
    if( fstyle->boxtype == 'G' )
    {
        padd(ascii("style:name"), sXML_CDATA,
            ascii(Int2Str(fstyle->boxnum, "G%d", buf)));
    }
    else
    {
        padd(ascii("style:name"), sXML_CDATA,
            ascii(Int2Str(fstyle->boxnum, "Txtbox%d", buf)));
    }

    padd(ascii("style:family"), sXML_CDATA, ascii("graphics"));
    rstartEl(ascii("style:style"), rList);
    pList->clear();

    padd(ascii("fo:margin-left"), sXML_CDATA, ascii("0cm"));
    padd(ascii("fo:margin-right"), sXML_CDATA, ascii("0cm"));
    padd(ascii("fo:margin-top"), sXML_CDATA, ascii("0cm"));
    padd(ascii("fo:margin-bottom"), sXML_CDATA, ascii("0cm"));
    padd(ascii("fo:padding"), sXML_CDATA, ascii("0cm"));
    padd(ascii("style:wrap"), sXML_CDATA, ascii("none"));
    padd(ascii("style:vertical-pos"), sXML_CDATA, ascii("from-top"));
    padd(ascii("style:vertical-rel"), sXML_CDATA, ascii("paragraph"));
    padd(ascii("style:horizontal-pos"), sXML_CDATA, ascii("from-left"));
    padd(ascii("style:horizontal-rel"), sXML_CDATA, ascii("paragraph"));
    if( fstyle->boxtype == 'G' )
    {
        char *cell = (char *)fstyle->cell;
        padd(ascii("draw:luminance"), sXML_CDATA,
            ascii(Int2Str(cell[0], "%d%%", buf)));
        padd(ascii("draw:contrast"), sXML_CDATA,
            ascii(Int2Str(cell[1], "%d%%", buf)));
        if( cell[2] == 0 )
            padd(ascii("draw:color-mode"), sXML_CDATA, ascii("standard"));
        else if( cell[2] == 1 )
            padd(ascii("draw:color-mode"), sXML_CDATA, ascii("greyscale"));
        else if( cell[2] == 2 )
            padd(ascii("draw:color-mode"), sXML_CDATA, ascii("mono"));
    }
    else
    {
        Cell *cell = (Cell *)fstyle->cell;
        if(cell->linetype[0] == cell->linetype[1] &&
            cell->linetype[0] == cell->linetype[2] &&
            cell->linetype[0] == cell->linetype[3])
        {
            switch( cell->linetype[0] )
            {
                case 0:
                    padd(ascii("fo:padding"), sXML_CDATA,ascii("0mm"));
                    break;
                case 1:                           /* ???????? */
                case 3:                           /* ???? -> ?????????????? ?????? ????. */
                    padd(ascii("fo:border"), sXML_CDATA,ascii("0.002cm solid #000000"));
                    break;
                case 2:                           /* ???????? */
                    padd(ascii("fo:border"), sXML_CDATA,ascii("0.035cm solid #000000"));
                    break;
                case 4:                           /* 2???? */
                    padd(ascii("style:border-line-width"), sXML_CDATA,ascii("0.002cm 0.035cm 0.002cm"));
                    padd(ascii("fo:border"), sXML_CDATA,ascii("0.039cm double #000000"));
                    break;
            }
        }
        else
        {
            switch( cell->linetype[0] )
            {
                case 1:                           /* ???????? */
                case 3:                           /* ???? -> ?????????????? ?????? ????. */
                    padd(ascii("fo:border-left"), sXML_CDATA,ascii("0.002cm solid #000000"));
                    break;
                case 2:                           /* ???????? */
                    padd(ascii("fo:border-left"), sXML_CDATA,ascii("0.035cm solid #000000"));
                    break;
                case 4:                           /* 2???? */
                    padd(ascii("style:border-line-width-left"), sXML_CDATA,ascii("0.002cm 0.035cm 0.002cm"));
                    padd(ascii("fo:border-left"), sXML_CDATA,ascii("0.039cm double #000000"));
                    break;
            }
            switch( cell->linetype[1] )
            {
                case 1:                           /* ???????? */
                case 3:                           /* ???? -> ?????????????? ?????? ????. */
                    padd(ascii("fo:border-right"), sXML_CDATA,ascii("0.002cm solid #000000"));
                    break;
                case 2:                           /* ???????? */
                    padd(ascii("fo:border-right"), sXML_CDATA,ascii("0.035cm solid #000000"));
                    break;
                case 4:                           /* 2???? */
                    padd(ascii("style:border-line-width-right"), sXML_CDATA,ascii("0.002cm 0.035cm 0.002cm"));
                    padd(ascii("fo:border-right"), sXML_CDATA,ascii("0.039cm double #000000"));
                    break;
            }
            switch( cell->linetype[2] )
            {
                case 1:                           /* ???????? */
                case 3:                           /* ???? -> ?????????????? ?????? ????. */
                    padd(ascii("fo:border-top"), sXML_CDATA,ascii("0.002cm solid #000000"));
                    break;
                case 2:                           /* ???????? */
                    padd(ascii("fo:border-top"), sXML_CDATA,ascii("0.035cm solid #000000"));
                    break;
                case 4:                           /* 2???? */
                    padd(ascii("style:border-line-width-top"), sXML_CDATA,ascii("0.002cm 0.035cm 0.002cm"));
                    padd(ascii("fo:border-top"), sXML_CDATA,ascii("0.039cm double #000000"));
                    break;
            }
            switch( cell->linetype[3] )
            {
                case 1:                           /* ???????? */
                case 3:                           /* ???? -> ?????????????? ?????? ????. */
                    padd(ascii("fo:border-bottom"), sXML_CDATA,ascii("0.002cm solid #000000"));
                    break;
                case 2:                           /* ???????? */
                    padd(ascii("fo:border-bottom"), sXML_CDATA,ascii("0.035cm solid #000000"));
                    break;
                case 4:                           /* 2???? */
                    padd(ascii("style:border-line-width-bottom"), sXML_CDATA,ascii("0.002cm 0.035cm 0.002cm"));
                    padd(ascii("fo:border-bottom"), sXML_CDATA,ascii("0.039cm double #000000"));
                    break;
            }
        }
        if(cell->shade != 0)
            padd(ascii("fo:background-color"), sXML_CDATA, ascii(hcolor2str(
            sal::static_int_cast<uchar>(cell->color), 
            sal::static_int_cast<uchar>(cell->shade), buf)));
    }
    rstartEl(ascii("style:properties"), rList);
    pList->clear();
    rendEl(ascii("style:properties"));
    rendEl(ascii("style:style"));
}


/**
 * Floating ?????? ???? ???????? ??????.
 */
void HwpReader::makeFStyle(FBoxStyle * fstyle)
{
                                                  /* ???? exist */
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
            padd(ascii("style:name"), sXML_CDATA,
                ascii(Int2Str(fstyle->boxnum, "Txtbox%d", buf)));
            padd(ascii("style:family"), sXML_CDATA, ascii("graphics"));
            break;
        case 'G' :                                // graphics
            padd(ascii("style:name"), sXML_CDATA,
                ascii(Int2Str(fstyle->boxnum, "G%d", buf)));
            padd(ascii("style:family"), sXML_CDATA, ascii("graphics"));
            break;
        case 'L' :                                // line TODO : all
            padd(ascii("style:name"), sXML_CDATA,
                ascii(Int2Str(fstyle->boxnum, "L%d", buf)));
            padd( ascii("style:family") , sXML_CDATA , ascii("paragraph") );
            break;
    }

    rstartEl(ascii("style:style"), rList);
    pList->clear();

    if ( fstyle->boxtype == 'T')
    {
        padd(ascii("fo:padding"), sXML_CDATA, ascii("0cm"));
    }

    if( !(fstyle->boxtype == 'G' && fstyle->cap_len > 0 ))
    {
        padd(ascii("fo:margin-left"), sXML_CDATA,
            Double2Str(WTMM(fstyle->margin[0][0]) ) + ascii("mm"));
        padd(ascii("fo:margin-right"), sXML_CDATA,
            Double2Str(WTMM(fstyle->margin[0][1])) + ascii("mm"));
        padd(ascii("fo:margin-top"), sXML_CDATA,
            Double2Str(WTMM(fstyle->margin[0][2])) + ascii("mm"));
        padd(ascii("fo:margin-bottom"), sXML_CDATA,
            Double2Str(WTMM(fstyle->margin[0][3])) + ascii("mm"));
    }

    switch (fstyle->txtflow)
    {
        case 0:
            padd(ascii("style:wrap"), sXML_CDATA, ascii("none"));
            break;
        case 1:
            if( fstyle->boxtype == 'G' || fstyle->boxtype == 'B' || fstyle->boxtype == 'O')
                padd(ascii("style:run-through"), sXML_CDATA, ascii("background"));
            padd(ascii("style:wrap"), sXML_CDATA, ascii("run-through"));
            break;
        case 2:
            padd(ascii("style:wrap"), sXML_CDATA, ascii("dynamic"));
            break;
    }
    if (fstyle->anchor_type == CHAR_ANCHOR)
    {
        padd(ascii("style:vertical-pos"), sXML_CDATA, ascii("top"));
        padd(ascii("style:vertical-rel"), sXML_CDATA, ascii("baseline"));
        padd(ascii("style:horizontal-pos"), sXML_CDATA, ascii("center"));
        padd(ascii("style:horizontal-rel"), sXML_CDATA, ascii("paragraph"));
    }
    else
    {

        switch (-(fstyle->xpos))
        {
            case 2:
                padd(ascii("style:horizontal-pos"), sXML_CDATA, ascii("right"));
                break;
            case 3:
                padd(ascii("style:horizontal-pos"), sXML_CDATA, ascii("center"));
                break;
            case 1:
            default:
                padd(ascii("style:horizontal-pos"), sXML_CDATA, ascii("from-left"));
                break;
        }
        switch (-(fstyle->ypos))
        {
            case 2:
                padd(ascii("style:vertical-pos"), sXML_CDATA, ascii("bottom"));
                break;
            case 3:
                padd(ascii("style:vertical-pos"), sXML_CDATA, ascii("middle"));
                break;
            case 1:
            default:
                padd(ascii("style:vertical-pos"), sXML_CDATA, ascii("from-top"));
                break;
        }
        if ( fstyle->anchor_type == PARA_ANCHOR )
        {
            padd(ascii("style:vertical-rel"), sXML_CDATA, ascii("paragraph"));
            padd(ascii("style:horizontal-rel"), sXML_CDATA, ascii("paragraph"));
        }
        else
        {
            padd(ascii("style:vertical-rel"), sXML_CDATA, ascii("page-content"));
            padd(ascii("style:horizontal-rel"), sXML_CDATA, ascii("page-content"));
        }
    }
    if( fstyle->boxtype == 'X' || fstyle->boxtype == 'B' )
    {
        Cell *cell = (Cell *)fstyle->cell;
        if(cell->linetype[0] == cell->linetype[1] &&
            cell->linetype[0] == cell->linetype[2] &&
            cell->linetype[0] == cell->linetype[3])
        {
            switch( cell->linetype[0] )
            {
                case 0:
                          padd(ascii("fo:border"), sXML_CDATA, ascii("none"));
                    break;
                case 1:                           /* ???????? */
                case 3:                           /* ???? -> ?????????????? ?????? ????. */
                    padd(ascii("fo:border"), sXML_CDATA,ascii("0.002cm solid #000000"));
                    break;
                case 2:                           /* ???????? */
                    padd(ascii("fo:border"), sXML_CDATA,ascii("0.035cm solid #000000"));
                    break;
                case 4:                           /* 2???? */
                    padd(ascii("style:border-line-width"), sXML_CDATA,ascii("0.002cm 0.035cm 0.002cm"));
                    padd(ascii("fo:border"), sXML_CDATA,ascii("0.039cm double #000000"));
                    break;
            }
        }
        else
        {
            switch( cell->linetype[0] )
            {
                case 1:                           /* ???????? */
                case 3:                           /* ???? -> ?????????????? ?????? ????. */
                    padd(ascii("fo:border-left"), sXML_CDATA,ascii("0.002cm solid #000000"));
                    break;
                case 2:                           /* ???????? */
                    padd(ascii("fo:border-left"), sXML_CDATA,ascii("0.035cm solid #000000"));
                    break;
                case 4:                           /* 2???? */
                    padd(ascii("style:border-line-width-left"), sXML_CDATA,ascii("0.002cm 0.035cm 0.002cm"));
                    padd(ascii("fo:border-left"), sXML_CDATA,ascii("0.039cm double #000000"));
                    break;
            }
            switch( cell->linetype[1] )
            {
                case 1:                           /* ???????? */
                case 3:                           /* ???? -> ?????????????? ?????? ????. */
                    padd(ascii("fo:border-right"), sXML_CDATA,ascii("0.002cm solid #000000"));
                    break;
                case 2:                           /* ???????? */
                    padd(ascii("fo:border-right"), sXML_CDATA,ascii("0.035cm solid #000000"));
                    break;
                case 4:                           /* 2???? */
                    padd(ascii("style:border-line-width-right"), sXML_CDATA,ascii("0.002cm 0.035cm 0.002cm"));
                    padd(ascii("fo:border-right"), sXML_CDATA,ascii("0.039cm double #000000"));
                    break;
            }
            switch( cell->linetype[2] )
            {
                case 1:                           /* ???????? */
                case 3:                           /* ???? -> ?????????????? ?????? ????. */
                    padd(ascii("fo:border-top"), sXML_CDATA,ascii("0.002cm solid #000000"));
                    break;
                case 2:                           /* ???????? */
                    padd(ascii("fo:border-top"), sXML_CDATA,ascii("0.035cm solid #000000"));
                    break;
                case 4:                           /* 2???? */
                    padd(ascii("style:border-line-width-top"), sXML_CDATA,ascii("0.002cm 0.035cm 0.002cm"));
                    padd(ascii("fo:border-top"), sXML_CDATA,ascii("0.039cm double #000000"));
                    break;
            }
            switch( cell->linetype[3] )
            {
                case 1:                           /* ???????? */
                case 3:                           /* ???? -> ?????????????? ?????? ????. */
                    padd(ascii("fo:border-bottom"), sXML_CDATA,ascii("0.002cm solid #000000"));
                    break;
                case 2:                           /* ???????? */
                    padd(ascii("fo:border-bottom"), sXML_CDATA,ascii("0.035cm solid #000000"));
                    break;
                case 4:                           /* 2???? */
                    padd(ascii("style:border-line-width-bottom"), sXML_CDATA,ascii("0.002cm 0.035cm 0.002cm"));
                    padd(ascii("fo:border-bottom"), sXML_CDATA,ascii("0.039cm double #000000"));
                    break;
            }
        }

          if( cell->linetype[0] == 0 && cell->linetype[1] == 0 &&
                  cell->linetype[2] == 0 && cell->linetype[3] == 0 ){
              padd(ascii("fo:padding"), sXML_CDATA,ascii("0mm"));
          }
          else{
              padd(ascii("fo:padding-left"), sXML_CDATA,
                      Double2Str(WTMM(fstyle->margin[1][0])) + ascii("mm"));
              padd(ascii("fo:padding-right"), sXML_CDATA,
                      Double2Str(WTMM(fstyle->margin[1][1])) + ascii("mm"));
              padd(ascii("fo:padding-top"), sXML_CDATA,
                      Double2Str(WTMM(fstyle->margin[1][2])) + ascii("mm"));
              padd(ascii("fo:padding-bottom"), sXML_CDATA,
                      Double2Str(WTMM(fstyle->margin[1][3])) + ascii("mm"));
          }
        if(cell->shade != 0)
            padd(ascii("fo:background-color"), sXML_CDATA, 
            ascii(hcolor2str(
                sal::static_int_cast<uchar>(cell->color), 
                sal::static_int_cast<uchar>(cell->shade), 
                buf)));
    }
    else if( fstyle->boxtype == 'E' )
     {
          padd(ascii("fo:padding"), sXML_CDATA,ascii("0mm"));
     }
    else if( fstyle->boxtype == 'L' )
    {
        padd( ascii("style:border-line-width-bottom"), sXML_CDATA, ascii("0.02mm 0.35mm 0.02mm"));
        padd(ascii("fo:border-bottom"), sXML_CDATA,ascii("0.039cm double #808080"));
    }
    else if( fstyle->boxtype == 'G' )
    {
         if( fstyle->margin[1][0] || fstyle->margin[1][1] || fstyle->margin[1][2] || fstyle->margin[1][3] ){
             OUString clip = ascii("rect(");
             clip += Double2Str(WTMM(-fstyle->margin[1][0]) ) + ascii("mm ");
             clip += Double2Str(WTMM(-fstyle->margin[1][1]) ) + ascii("mm ");
             clip += Double2Str(WTMM(-fstyle->margin[1][2]) ) + ascii("mm ");
             clip += Double2Str(WTMM(-fstyle->margin[1][3]) ) + ascii("mm)");
             padd(ascii("style:mirror"), sXML_CDATA, ascii("none"));
             padd(ascii("fo:clip"), sXML_CDATA, clip);
         }
        char *cell = (char *)fstyle->cell;
        padd(ascii("draw:luminance"), sXML_CDATA,
            ascii(Int2Str(cell[0], "%d%%", buf)));
        padd(ascii("draw:contrast"), sXML_CDATA,
            ascii(Int2Str(cell[1], "%d%%", buf)));
        if( cell[2] == 0 )
            padd(ascii("draw:color-mode"), sXML_CDATA, ascii("standard"));
        else if( cell[2] == 1 )
            padd(ascii("draw:color-mode"), sXML_CDATA, ascii("greyscale"));
        else if( cell[2] == 2 )
            padd(ascii("draw:color-mode"), sXML_CDATA, ascii("mono"));

    }
    rstartEl(ascii("style:properties"), rList);
    pList->clear();
    rendEl(ascii("style:properties"));
    rendEl(ascii("style:style"));
}


char *HwpReader::getTStyleName(int index, char *_buf)
{
    return Int2Str(index, "T%d", _buf);
}


char *HwpReader::getPStyleName(int index, char *_buf)
{
    return Int2Str(index, "P%d", _buf);
}


//void HwpReader::makeChars(char *str, int size)
void HwpReader::makeChars(hchar *str, int size)
{
    if (size)
    {
        str[size] = '\0';
        rchars(OUString(str));
    }
}


/**
 * ???????? ?????????? ???? ???? ?????? ?????? CharShape?? ???????? ????
 */
void HwpReader::make_text_p0(HWPPara * para, sal_Bool bParaStart)
{
    int n;
    int l = 0, res;
     hchar dest[3];
    unsigned char firstspace = 0;
    if( !bParaStart)
    {
        padd(ascii("text:style-name"), sXML_CDATA,
            ascii(getPStyleName(para->GetParaShape()->index, buf)));
        rstartEl(ascii("text:p"), rList);
        pList->clear();
    }
    if( d->bFirstPara && d->bInBody )
    {
        strcpy(buf,"[?????? ????]");
        padd(ascii("text:name"), sXML_CDATA, OUString(buf, strlen(buf), RTL_TEXTENCODING_EUC_KR));
        rstartEl(ascii("text:bookmark"), rList);
        pList->clear();
        rendEl(ascii("text:bookmark"));
        d->bFirstPara = sal_False;
    }
    if( d->bInHeader )
    {
        makeShowPageNum();
        d->bInHeader = sal_False;
    }
    padd(ascii("text:style-name"), sXML_CDATA,
        ascii(getTStyleName(para->cshape.index, buf)));
    rstartEl(ascii("text:span"), rList);
    pList->clear();

    for (n = 0; n < para->nch && para->hhstr[n]->hh;
        n += para->hhstr[n]->WSize())
    {
        if (para->hhstr[n]->hh == CH_SPACE && !firstspace)
        {
            makeChars(gstr, l);
            l = 0;
            rstartEl(ascii("text:s"), rList);
            rendEl(ascii("text:s"));
        }
        else if (para->hhstr[n]->hh == CH_END_PARA)
        {
            makeChars(gstr, l);
            l = 0;
            rendEl(ascii("text:span"));
            rendEl(ascii("text:p"));
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
                    gstr[l++] =  dest[j];
        }
    }
}


/**
 * ???????? ?????????? ?????? ???????? ???? CharShape?? ???????? ????
 */
void HwpReader::make_text_p1(HWPPara * para,sal_Bool bParaStart)
{
    int n;
    int l = 0, res;
     hchar dest[3];
    int curr = para->cshape.index;
    unsigned char firstspace = 0;

    if( !bParaStart )
    {
        padd(ascii("text:style-name"), sXML_CDATA,
            ascii(getPStyleName(para->GetParaShape()->index, buf)));
        rstartEl(ascii("text:p"), rList);
        pList->clear();
    }
    if( d->bFirstPara && d->bInBody )
    {
/* for HWP's Bookmark */
        strcpy(buf,"[?????? ????]");
        padd(ascii("text:name"), sXML_CDATA, OUString(buf, strlen(buf), RTL_TEXTENCODING_EUC_KR));
        rstartEl(ascii("text:bookmark"), rList);
        pList->clear();
        rendEl(ascii("text:bookmark"));
        d->bFirstPara = sal_False;
    }
    if( d->bInHeader )
    {
        makeShowPageNum();
        d->bInHeader = sal_False;
    }
    padd(ascii("text:style-name"), sXML_CDATA,
        ascii(getTStyleName(curr, buf)));
    rstartEl(ascii("text:span"), rList);
    pList->clear();

    for (n = 0; n < para->nch && para->hhstr[n]->hh;
        n += para->hhstr[n]->WSize())
    {
        if (para->GetCharShape(n)->index != curr)
        {
            makeChars(gstr, l);
            l = 0;
            rendEl(ascii("text:span"));
            curr = para->GetCharShape(n)->index;
            padd(ascii("text:style-name"), sXML_CDATA,
                ascii(getTStyleName(curr, buf)));
            rstartEl(ascii("text:span"), rList);
            pList->clear();
        }
        if (para->hhstr[n]->hh == CH_SPACE && !firstspace)
        {
            makeChars(gstr, l);
            l = 0;
            rstartEl(ascii("text:s"), rList);
            rendEl(ascii("text:s"));
        }
        else if (para->hhstr[n]->hh == CH_END_PARA)
        {
            makeChars(gstr, l);
            l = 0;
            rendEl(ascii("text:span"));
            rendEl(ascii("text:p"));
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
                    gstr[l++] =  dest[j];
        }
    }
}


/**
 * ???? ???? ?????????? ?????? ???????? ???? CharShape?? ???? ?????? ???? ????
 */
void HwpReader::make_text_p3(HWPPara * para,sal_Bool bParaStart)
{
    int n, res;
     hchar dest[3];
    size_t l = 0;
    unsigned char firstspace = 0;
    bool pstart = bParaStart;
    bool tstart = false;
    bool infield = false;
    int curr = para->cshape.index;
    if( d->bFirstPara && d->bInBody )
    {
        if( !pstart )
            STARTP;
        strcpy(buf,"[?????? ????]");
        padd(ascii("text:name"), sXML_CDATA, OUString(buf, strlen(buf), RTL_TEXTENCODING_EUC_KR));
        rstartEl(ascii("text:bookmark"), rList);
        pList->clear();
        rendEl(ascii("text:bookmark"));
        d->bFirstPara = sal_False;
    }
    if( d->bInHeader )
    {
        if( !pstart )
            STARTP;
        makeShowPageNum();
        d->bInHeader = sal_False;
    }

    for (n = 0; n < para->nch && para->hhstr[n]->hh;
        n += para->hhstr[n]->WSize())
    {
        if( para->hhstr[n]->hh == CH_END_PARA )
        {
            if( l > 0 )
            {
                if( !pstart ){ STARTP;}
                if( !tstart ){ STARTT;}
                makeChars(gstr, l);
                l = 0;
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
            makeChars(gstr, l);
            l = 0;
            rstartEl(ascii("text:s"), rList);
            pList->clear();
            rendEl(ascii("text:s"));
        }
        else if ( para->hhstr[n]->hh >= CH_SPACE )
        {
            if( n > 0 )
                if( para->GetCharShape(n)->index != para->GetCharShape(n-1)->index && !infield )
                    {
                         if( !pstart ) {STARTP;}
                         if( !tstart ) {STARTT;}
                         makeChars(gstr, l);
                         ENDT;
                         l = 0;
                    }
            if( para->hhstr[n]->hh == CH_SPACE )
                firstspace = 0;
            else
                firstspace = 1;
                res = hcharconv(para->hhstr[n]->hh, dest, UNICODE);
                for( int j = 0 ; j < res; j++ )
                    gstr[l++] =  dest[j];
        }
        else if (para->hhstr[n]->hh == CH_FIELD)
        {
            FieldCode *hbox = (FieldCode *) para->hhstr[n];
            if( hbox->location_info == 1)
            {
                if( !pstart ) {STARTP;}
                if( !tstart ) {STARTT;}
                makeChars(gstr, l);
                l = 0;
                firstspace = 1;
                     if( hbox->type[0] == 4 && hbox->type[1] == 0 )
                     {
                         field = hbox->str3;
                     }
                     else{
                         makeFieldCode(hbox);
                     }
                infield = true;
            }
            else
            {
                firstspace = 1;
                     if( hbox->type[0] == 4 && hbox->type[1] == 0 )
                     {
                         gstr[l] = 0;
                         makeFieldCode(hbox);
                         field = 0L;
                     }
                infield = false;
                l = 0;
            }
        }
        else
        {
            switch (para->hhstr[n]->hh)
            {
                case CH_BOOKMARK:
                    if( !pstart ) {STARTP;}
                    if( !tstart ) {STARTT;}
                    makeChars(gstr, l);
                    l = 0;
                    makeBookmark((Bookmark *) para->hhstr[n]);
                    break;
                case CH_DATE_FORM:                // 7
                    break;
                case CH_DATE_CODE:                // 8
                    if( !pstart ) {STARTP;}
                    if( !tstart ) {STARTT;}
                    makeChars(gstr, l);
                    l = 0;
                    makeDateCode((DateCode *) para->hhstr[n]);
                    break;
                case CH_TAB:                      // 9
                    if( !pstart ) {STARTP;}
                    if( l > 0 )
                    {
                        if( !tstart ) {STARTT;}
                        makeChars(gstr, l);
                        l = 0;
                    }
                    makeTab((Tab *) para->hhstr[n]);
                    break;
                case CH_TEXT_BOX:                 /* 10 - ??/??????????/????/????/???????????? ?? */
                {
/* ?????? ???? ????????, ?????? text:p???? ???????? ?????? ????. */
                    TxtBox *hbox = (TxtBox *) para->hhstr[n];

                    if( hbox->style.anchor_type == 0 )
                    {
                        if( !pstart ) {STARTP;}
                        if( !tstart ) {STARTT;}
                        makeChars(gstr, l);
                        l = 0;
                    }
                    else
                    {
                        if( !pstart ) {STARTP;}
                        if( l > 0 )
                        {
                            if( !tstart ) {STARTT;}
                            makeChars(gstr, l);
                            l = 0;
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
                    Picture *hbox = (Picture *) para->hhstr[n];
                    if( hbox->style.anchor_type == 0 )
                    {
                        if( !pstart ) {STARTP;}
                        if( !tstart ) {STARTT;}
                        makeChars(gstr, l);
                        l = 0;
                    }
                    else
                    {
                        if( !pstart ) {STARTP;}
                        if( l > 0 )
                        {
                            if( !tstart ) {STARTT;}
                            makeChars(gstr, l);
                            l = 0;
                        }
                        if( tstart ) {ENDT;}
                    }
                    makePicture(hbox);
                    break;
                }
                case CH_LINE:                     // 14
                {
                    Line *hbox = (Line *) para->hhstr[n];
                    if( l > 0 )
                    {
                        if( !pstart ) {STARTP;}
                        if( !tstart ) {STARTT;}
                        makeChars(gstr, l);
                        l = 0;
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
                    makeChars(gstr, l);
                    l = 0;
                    makeHidden((Hidden *) para->hhstr[n]);
                    break;
                case CH_FOOTNOTE:                 // 17
                    if( !pstart ) {STARTP;}
                    if( !tstart ) {STARTT;}
                    makeChars(gstr, l);
                    l = 0;
                    makeFootnote((Footnote *) para->hhstr[n]);
                    break;
                case CH_AUTO_NUM:                 // 18
                    if( !pstart ) {STARTP;}
                    if( !tstart ) {STARTT;}
                    makeChars(gstr, l);
                    l = 0;
                    makeAutoNum((AutoNum *) para->hhstr[n]);
                    break;
                case CH_NEW_NUM:                  // 19 -skip
                    break;
                case CH_PAGE_NUM_CTRL:            // 21
                    break;
                case CH_MAIL_MERGE:               // 22
                    if( !pstart ) {STARTP;}
                    if( !tstart ) {STARTT;}
                    makeChars(gstr, l);
                    l = 0;
                    makeMailMerge((MailMerge *) para->hhstr[n]);
                    break;
                case CH_COMPOSE:                  /* 23 - ???????? */
                    break;
                case CH_HYPHEN:                   // 24
                    break;
                case CH_TOC_MARK:                 /* 25 ?????? 3???? ???????? ????. */
                    if( !pstart ) {STARTP;}
                    if( !tstart ) {STARTT;}
                    makeChars(gstr, l);
                    l = 0;
                    makeTocMark((TocMark *) para->hhstr[n]);
                    break;
                case CH_INDEX_MARK:               // 26
                    if( !pstart ) {STARTP;}
                    if( !tstart ) {STARTT;}
                    makeChars(gstr, l);
                    l = 0;
                    makeIndexMark((IndexMark *) para->hhstr[n]);
                    break;
                case CH_OUTLINE:                  // 28
                    if( !pstart ) {STARTP;}
                    if( !tstart ) {STARTT;}
                    makeChars(gstr, l);
                    l = 0;
                    makeOutline((Outline *) para->hhstr[n]);
                    break;
                     case CH_FIXED_SPACE:
                     case CH_KEEP_SPACE:
                          gstr[l++] = 0x0020;
                          break;
                }
        }
    }
}


void HwpReader::makeFieldCode(FieldCode *hbox)
{
/* ?????? */
    if( hbox->type[0] == 4 && hbox->type[1] == 0 )
    {
        padd(ascii("text:placeholder-type"), sXML_CDATA, ascii("text"));
          if( field )
              padd(ascii("text:description"), sXML_CDATA, hconv(field, sbuf));
        rstartEl( ascii("text:placeholder"), rList);
        pList->clear();
        rchars( OUString(gstr));
        rendEl( ascii("text:placeholder") );
    }
/* ???????? */
    else if( hbox->type[0] == 3 && hbox->type[1] == 0 )
    {
        if( hconv( hbox->str3, gstr ).equals(OUString::createFromAscii("title")))
        {
            rstartEl( ascii("text:title"), rList );
            rchars(  hconv(hbox->str2, gstr) );
            rendEl( ascii("text:title") );
        }
        else if( hconv( hbox->str3, gstr ).equals(OUString::createFromAscii("subject")))
        {
            rstartEl( ascii("text:subject"), rList );
            rchars(  hconv(hbox->str2, gstr) );
            rendEl( ascii("text:subject") );
        }
        else if( hconv( hbox->str3, gstr ).equals(OUString::createFromAscii("author")))
        {
            rstartEl( ascii("text:author-name"), rList );
            rchars(  hconv(hbox->str2, gstr) );
            rendEl( ascii("text:author-name") );
        }
        else if( hconv( hbox->str3, gstr ).equals(OUString::createFromAscii("keywords")))
        {
            rstartEl( ascii("text:keywords"), rList );
            rchars(  hconv(hbox->str2, gstr) );
            rendEl( ascii("text:keywords") );
        }
    }
/* ???????? */
    else if( hbox->type[0] == 3 && hbox->type[1] == 1 )
    {
        if( hconv( hbox->str3, gstr ).equals(OUString::createFromAscii("User")))
        {
            rstartEl( ascii("text:sender-lastname"), rList );
            rchars(  hconv(hbox->str2, gstr) );
            rendEl( ascii("text:sender-lastname") );
        }
        else if( hconv( hbox->str3, gstr ).equals(OUString::createFromAscii("Company")))
        {
            rstartEl( ascii("text:sender-company"), rList );
            rchars(  hconv(hbox->str2, gstr) );
            rendEl( ascii("text:sender-company") );
        }
        else if( hconv( hbox->str3, gstr ).equals(OUString::createFromAscii("Position")))
        {
            rstartEl( ascii("text:sender-title"), rList );
            rchars(  hconv(hbox->str2, gstr) );
            rendEl( ascii("text:sender-title") );
        }
        else if( hconv( hbox->str3, gstr ).equals(OUString::createFromAscii("Division")))
        {
            rstartEl( ascii("text:sender-position"), rList );
            rchars(  hconv(hbox->str2, gstr) );
            rendEl( ascii("text:sender-position") );
        }
        else if( hconv( hbox->str3, gstr ).equals(OUString::createFromAscii("Fax")))
        {
            rstartEl( ascii("text:sender-fax"), rList );
            rchars(  hconv(hbox->str2, gstr) );
            rendEl( ascii("text:sender-fax") );
        }
        else if( hconv( hbox->str3, gstr ).equals(OUString::createFromAscii("Pager")))
        {
            rstartEl( ascii("text:phone-private"), rList );
            rchars(  hconv(hbox->str2, gstr) );
            rendEl( ascii("text:phone-private") );
        }
        else if( hconv( hbox->str3, gstr ).equals(OUString::createFromAscii("E-mail")))
        {
            rstartEl( ascii("text:sender-email"), rList );
            rchars(  hconv(hbox->str2, gstr) );
            rendEl( ascii("text:sender-email") );
        }
        else if( hconv( hbox->str3, gstr ).equals(OUString::createFromAscii("Zipcode(office)")))
        {
            rstartEl( ascii("text:sender-postal-code"), rList );
            rchars(  hconv(hbox->str2, gstr) );
            rendEl( ascii("text:sender-postal-code") );
        }
        else if( hconv( hbox->str3, gstr ).equals(OUString::createFromAscii("Phone(office)")))
        {
            rstartEl( ascii("text:sender-phone-work"), rList );
            rchars(  hconv(hbox->str2, gstr) );
            rendEl( ascii("text:sender-phone-work") );
        }
        else if( hconv( hbox->str3, gstr ).equals(OUString::createFromAscii("Address(office)")))
        {
            rstartEl( ascii("text:sender-street"), rList );
            rchars(  hconv(hbox->str2, gstr) );
            rendEl( ascii("text:sender-street") );
        }

    }
    else if( hbox->type[0] == 3 && hbox->type[1] == 2 ) /* ???????? */
     {
         if( hbox->m_pDate )
             padd(ascii("style:data-style-name"), sXML_CDATA,
                     ascii(Int2Str(hbox->m_pDate->key, "N%d", buf)));
            rstartEl( ascii("text:creation-date"), rList );
            pList->clear();
            rchars(  hconv(hbox->str2, gstr) );
            rendEl( ascii("text:creation-date") );
     }
}


/**
 * Completed
 * ???????????????? ???????? Reference?? ???????? hwp???? ?? ?????? ????.
 */
void HwpReader::makeBookmark(Bookmark * hbox)
{
    if (hbox->type == 0)
    {
        padd(ascii("text:name"), sXML_CDATA, (hconv(hbox->id, gstr)));
        rstartEl(ascii("text:bookmark"), rList);
        pList->clear();
        rendEl(ascii("text:bookmark"));
    }
    else if (hbox->type == 1)                     /* ???? ???????? ???? ?????? ???? ???? */
    {
        padd(ascii("text:name"), sXML_CDATA, (hconv(hbox->id, gstr)));
        rstartEl(ascii("text:bookmark-start"), rList);
        pList->clear();
        rendEl(ascii("text:bookmark-start"));
    }
    else if (hbox->type == 2)
    {
        padd(ascii("text:name"), sXML_CDATA, (hconv(hbox->id, gstr)));
        rstartEl(ascii("text:bookmark-end"), rList);
        pList->clear();
        rendEl(ascii("text:bookmark-end"));
    }
}


#include "datecode.h"

void HwpReader::makeDateFormat(DateCode * hbox)
{
    padd(ascii("style:name"), sXML_CDATA,
        ascii(Int2Str(hbox->key, "N%d", buf)));
    padd(ascii("style:family"), sXML_CDATA,ascii("data-style"));
    padd(ascii("number:language"), sXML_CDATA,ascii("ko"));
    padd(ascii("number:country"), sXML_CDATA,ascii("KR"));

    rstartEl(ascii("number:date-style"), rList);
    pList->clear();

    bool is_pm;
    bool add_zero = false;
    int zero_check = 0/*, i=0*/;
    hbox->format[DATE_SIZE -1] = 0;

    hchar *fmt = hbox->format[0] ? hbox->format : defaultform;
//hstr2ksstr(fmt, buf);

    for( ; *fmt ; fmt++ )
    {
        is_pm = (hbox->date[DateCode::HOUR] >= 12 );

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
                padd(ascii("number:style"), sXML_CDATA, ascii("long"));
                rstartEl(ascii("number:year"), rList);
                pList->clear();
                rendEl(ascii("number:year"));
                break;
            case '!':
                rstartEl(ascii("number:year"), rList);
                pList->clear();
                rendEl(ascii("number:year"));
                break;
            case '2':
                if( add_zero )
                    padd(ascii("number:style"), sXML_CDATA, ascii("long"));
                rstartEl(ascii("number:month"), rList);
                pList->clear();
                rendEl(ascii("number:month"));
                break;
            case '@':
                padd(ascii("number:textual"), sXML_CDATA, ascii("true"));
                rstartEl(ascii("number:month"), rList);
                pList->clear();
                rendEl(ascii("number:month"));
                break;
            case '*':
                padd(ascii("number:textual"), sXML_CDATA, ascii("true"));
                padd(ascii("number:style"), sXML_CDATA, ascii("long"));
                rstartEl(ascii("number:month"), rList);
                pList->clear();
                rendEl(ascii("number:month"));
                break;
            case '3':
                if( add_zero )
                    padd(ascii("number:style"), sXML_CDATA, ascii("long"));
                rstartEl(ascii("number:day"), rList);
                pList->clear();
                rendEl(ascii("number:day"));
                break;
            case '#':
                if( add_zero )
                    padd(ascii("number:style"), sXML_CDATA, ascii("long"));
                rstartEl(ascii("number:day"), rList);
                pList->clear();
                rendEl(ascii("number:day"));
                switch( hbox->date[DateCode::DAY]  % 10)
                {
                    case 1:
                        rstartEl(ascii("number:text"), rList);
                        rchars(ascii("st"));
                        rendEl(ascii("number:text"));
                        break;
                    case 2:
                        rstartEl(ascii("number:text"), rList);
                        rchars(ascii("nd"));
                        rendEl(ascii("number:text"));
                        break;
                    case 3:
                        rstartEl(ascii("number:text"), rList);
                        rchars(ascii("rd"));
                        rendEl(ascii("number:text"));
                        break;
                    default:
                        rstartEl(ascii("number:text"), rList);
                        rchars(ascii("th"));
                        rendEl(ascii("number:text"));
                        break;
                }
                break;
            case '4':
            case '$':
                if( add_zero )
                    padd(ascii("number:style"), sXML_CDATA, ascii("long"));
                rstartEl(ascii("number:hours"), rList);
                pList->clear();
                rendEl(ascii("number:hours"));
                break;
            case '5':
            case '%':
                if( add_zero )
                    padd(ascii("number:style"), sXML_CDATA, ascii("long"));
                rstartEl(ascii("number:minutes"), rList);
                pList->clear();
                rendEl(ascii("number:minutes"));
                break;
            case '_':
                padd(ascii("number:style"), sXML_CDATA, ascii("long"));
            case '6':
            case '^':
                rstartEl(ascii("number:day-of-week"), rList);
                pList->clear();
                rendEl(ascii("number:day-of-week"));
                break;
            case '7':
            case '&':
            case '+':
                rstartEl(ascii("number:am-pm"), rList);
                pList->clear();
                rendEl(ascii("number:am-pm"));
                break;
            case '~':                             // Chiness Locale
/*
if( fmt[1] == 0 ) break;
fmt++;
if( *fmt == '6' ){
    rstartEl(ascii("number:day-of-week"), rList);
    pList->clear();
    rendEl(ascii("number:day-of-week"));
}
*/
                break;
            default:
                sbuf[0] = *fmt;
                sbuf[1] = 0;
                rstartEl(ascii("number:text"), rList);
                rchars((hconv(sbuf, gstr)));
                rendEl(ascii("number:text"));
                break;
        }
    }
    pList->clear();
    rendEl(ascii("number:date-style"));
}


void HwpReader::makeDateCode(DateCode * hbox)
{
    padd(ascii("style:data-style-name"), sXML_CDATA,
        ascii(Int2Str(hbox->key, "N%d", buf)));
    rstartEl( ascii("text:date"), rList );
    pList->clear();
    hbox->GetString(sbuf, 255);
    rchars((hconv(sbuf, gstr)));
    rendEl( ascii("text:date") );
}


void HwpReader::makeTab(Tab *  )                  /*hbox */
{
    rstartEl(ascii("text:tab-stop"), rList);
    rendEl(ascii("text:tab-stop"));
}


void HwpReader::makeTable(TxtBox * hbox)
{
    padd(ascii("table:name"), sXML_CDATA,
        ascii(Int2Str(hbox->style.boxnum, "Table%d", buf)));
    padd(ascii("table:style-name"), sXML_CDATA,
        ascii(Int2Str(hbox->style.boxnum, "Table%d", buf)));
    rstartEl(ascii("table:table"), rList);
    pList->clear();

    Table *tbl = hbox->m_pTable;
// ----------- column ---------------- //
    int i ;
    for( i = 0 ; i < tbl->columns.nCount -1 ; i++ )
    {
        sprintf(buf,"Table%d.%c",hbox->style.boxnum, 'A'+i);
        padd(ascii("table:style-name"), sXML_CDATA, ascii( buf ));
        rstartEl(ascii("table:table-column"), rList);
        pList->clear();
        rendEl(ascii("table:table-column"));
    }

// ----------- cell ---------------- //
    int j = -1, k = -1;
    for( i = 0 ; i < tbl->cells.count(); i++ )
    {
        TCell *tcell = tbl->cells.find(i);
        if( tcell->nRowIndex > j )
        {
            if( j > k )
            {
                rendEl(ascii("table:table-row"));
                k = j;
            }
// --------------- row ---------------- //
            sprintf(buf,"Table%d.row%d",hbox->style.boxnum, tcell->nRowIndex + 1);
            padd(ascii("table:style-name"), sXML_CDATA, ascii( buf ));
            rstartEl(ascii("table:table-row"), rList);
            pList->clear();
            j = tcell->nRowIndex;
        }

        sprintf(buf,"Table%d.%c%d",hbox->style.boxnum, 'A'+ tcell->nColumnIndex, tcell->nRowIndex +1);
        padd(ascii("table:style-name"), sXML_CDATA, ascii( buf ));
        if( tcell->nColumnSpan > 1 )
            padd(ascii("table:number-columns-spanned"), sXML_CDATA,
                ascii(Int2Str(tcell->nColumnSpan, "%d", buf)));
        if( tcell->nRowSpan > 1 )
            padd(ascii("table:number-rows-spanned"), sXML_CDATA,
                ascii(Int2Str(tcell->nRowSpan, "%d", buf)));
        padd(ascii("table:value-type"), sXML_CDATA,ascii("string"));
        if( tcell->pCell->protect )
            padd(ascii("table:protected"), sXML_CDATA,ascii("true"));
        rstartEl(ascii("table:table-cell"), rList);
        pList->clear();
        parsePara(hbox->plists[tcell->pCell->key].first());
        rendEl(ascii("table:table-cell"));
    }
    rendEl(ascii("table:table-row"));
    rendEl(ascii("table:table"));
}


/**
 * ???????????? ???????? ????????.
 * 1. draw:style-name, draw:name, text:anchor-type, svg:width,
 *    fo:min-height, svg:x, svg:y
 * TODO : fo:background-color?? ???? ???? ????=>???????? ???????? ?? ???? ????????.
 */
void HwpReader::makeTextBox(TxtBox * hbox)
{
    if( hbox->style.cap_len > 0  && hbox->type == TXT_TYPE)
    {
        padd(ascii("draw:style-name"), sXML_CDATA,
            ascii(Int2Str(hbox->style.boxnum, "CapBox%d", buf)));
        padd(ascii("draw:name"), sXML_CDATA,
            ascii(Int2Str(hbox->style.boxnum, "CaptionBox%d", buf)));
         padd(ascii("draw:z-index"), sXML_CDATA,
              ascii(Int2Str(hbox->zorder, "%d", buf)));
        switch (hbox->style.anchor_type)
        {
            case CHAR_ANCHOR:
                padd(ascii("text:anchor-type"), sXML_CDATA, ascii("as-char"));
                break;
            case PARA_ANCHOR:
                padd(ascii("text:anchor-type"), sXML_CDATA, ascii("paragraph"));
                break;
            case PAGE_ANCHOR:
            case PAPER_ANCHOR:
            {
                // os unused
                // HWPInfo *hwpinfo = hwpfile.GetHWPInfo();
                padd(ascii("text:anchor-type"), sXML_CDATA, ascii("page"));
                padd(ascii("text:anchor-page-number"), sXML_CDATA,
                    ascii(Int2Str(hbox->pgno +1, "%d", buf)));
                break;
            }
        }
        if (hbox->style.anchor_type != CHAR_ANCHOR)
        {
            padd(ascii("svg:x"), sXML_CDATA,
                Double2Str(WTMM( ( hbox->pgx + hbox->style.margin[0][0] ) )) + ascii("mm"));
            padd(ascii("svg:y"), sXML_CDATA,
                Double2Str(WTMM( ( hbox->pgy + hbox->style.margin[0][2] ) )) + ascii("mm"));
        }
        padd(ascii("svg:width"), sXML_CDATA,
            Double2Str(WTMM(( hbox->box_xs + hbox->cap_xs) )) + ascii("mm"));
        padd(ascii("fo:min-height"), sXML_CDATA,
            Double2Str(WTMM(( hbox->box_ys + hbox->cap_ys) )) + ascii("mm"));
        rstartEl(ascii("draw:text-box"), rList);
        pList->clear();
        if( hbox->cap_pos % 2 )                   /* ?????? ?????? ???????? */
        {
            parsePara(hbox->caption.first());
        }
        padd( ascii("text:style-name"), sXML_CDATA, ascii("Standard"));
        rstartEl(ascii("text:p"), rList);
        pList->clear();
    }
     else{
         padd(ascii("draw:z-index"), sXML_CDATA,
              ascii(Int2Str(hbox->zorder, "%d", buf)));
     }

    // os unused
    // static int draw_name_id = 0;
    padd(ascii("draw:style-name"), sXML_CDATA,
        ascii(Int2Str(hbox->style.boxnum, "Txtbox%d", buf)));
    padd(ascii("draw:name"), sXML_CDATA,
        ascii(Int2Str(hbox->style.boxnum, "Frame%d", buf)));
    int x = 0;
    int y = 0;

    if( hbox->style.cap_len <= 0 || hbox->type != TXT_TYPE )
    {
        switch (hbox->style.anchor_type)
        {
            case CHAR_ANCHOR:
                padd(ascii("text:anchor-type"), sXML_CDATA, ascii("as-char"));
                break;
            case PARA_ANCHOR:
                padd(ascii("text:anchor-type"), sXML_CDATA, ascii("paragraph"));
                break;
            case PAGE_ANCHOR:
            case PAPER_ANCHOR:
            {
                // os unused
                // HWPInfo *hwpinfo = hwpfile.GetHWPInfo();
                padd(ascii("text:anchor-type"), sXML_CDATA, ascii("page"));
                padd(ascii("text:anchor-page-number"), sXML_CDATA,
                    ascii(Int2Str(hbox->pgno +1, "%d", buf)));
                break;
            }
        }
        if( hbox->style.anchor_type != CHAR_ANCHOR )
        {
            x += hbox->style.margin[0][0];
            y += hbox->style.margin[0][2];
        }
        padd(ascii("svg:x"), sXML_CDATA,
            Double2Str(WTMM( hbox->pgx + x )) + ascii("mm"));
        padd(ascii("svg:y"), sXML_CDATA,
            Double2Str(WTMM( hbox->pgy + y )) + ascii("mm"));
    }
    else
    {
        padd(ascii("text:anchor-type"), sXML_CDATA, ascii("as-char"));
        padd(ascii("svg:y"), sXML_CDATA, ascii("0cm"));
    }
    padd(ascii("svg:width"), sXML_CDATA,
        Double2Str(WTMM( hbox->box_xs )) + ascii("mm"));
    if( hbox->style.cap_len > 0 && hbox->type != TXT_TYPE)
        padd(ascii("fo:min-height"), sXML_CDATA,
            Double2Str(WTMM( hbox->box_ys + hbox->cap_ys)) + ascii("mm"));
    else
        padd(ascii("svg:height"), sXML_CDATA,
            Double2Str(WTMM(hbox->box_ys )) + ascii("mm"));

    if( hbox->type != EQU_TYPE )
    {
        rstartEl(ascii("draw:text-box"), rList);
        pList->clear();
/* ?????? ????????, ?????? ?????? */
        if( hbox->style.cap_len > 0 && (hbox->cap_pos % 2) && hbox->type == TBL_TYPE )
        {
            parsePara(hbox->caption.first());
        }
        if( hbox->type == TBL_TYPE)               // Is Table
        {
            makeTable(hbox);
        }
        else                                      // Is TextBox
        {
            parsePara(hbox->plists[0].first());
        }
/* ?????? ????????, ???????? ?????? */
        if( hbox->style.cap_len > 0 && !(hbox->cap_pos % 2) && hbox->type == TBL_TYPE)
        {
            parsePara(hbox->caption.first());
        }
        rendEl(ascii("draw:text-box"));
// Caption exist and it is text-box
        if( hbox->style.cap_len > 0 && hbox->type == TXT_TYPE)
        {
            rendEl( ascii("text:p"));
            if( !(hbox->cap_pos % 2))
            {
                parsePara(hbox->caption.first());
            }
            rendEl( ascii("draw:text-box"));
        }
    }
    else                                          // is Formula
    {
        rstartEl(ascii("draw:object"), rList);
        pList->clear();
        makeFormula(hbox);
        rendEl(ascii("draw:object"));
    }
}


/**
 * MathML?? ???????? ????.
 *
 */
void HwpReader::makeFormula(TxtBox * hbox)
{
    char mybuf[3000];
    HWPPara* pPar;
    CharShape *cshape = 0;

    int n, c, res;
     hchar dest[3];
    size_t l = 0;

    pPar = hbox->plists[0].first();
    while( pPar )
    {
        for( n = 0; n < pPar->nch && pPar->hhstr[n]->hh;
            n += pPar->hhstr[n]->WSize() )
        {
            if (!cshape)
                cshape = pPar->GetCharShape(n);
            if (l >= sizeof(mybuf)-1)
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
        if (l >= sizeof(mybuf)-1)
            break;
        mybuf[l++] = '\n';
        pPar = pPar->Next();
    }
    mybuf[l] = '\0';
//	 rchars(ascii(mybuf));
//#ifndef UDK100
    Formula *form = new Formula(mybuf);
    form->setDocumentHandler(rDocumentHandler);
    form->setAttributeListImpl(pList);
    form->parse();

    delete form;
//#endif


}


/**
 * platform?????? ?????????? href?? C:\?? D:\?? ?????? ???? ???????? ????????????
 * C:\ => ??????, D:\ => ????(/)?? ?????????? ?????? ????????. ??????
 * ?????? ???????????????? ???? ????????.
 */
void HwpReader::makeHyperText(TxtBox * hbox)
{
    HyperText *hypert = hwpfile.GetHyperText();
     if( !hypert ) return;

    if( strlen((char *)hypert->filename) > 0 ){
              char tmp[256];
              char tmp2[256];
              int nSize = hstr2ksstr(hypert->bookmark, tmp);
#ifdef _WIN32
              int nSize2 = hstr2ksstr(kstr2hstr((uchar *) urltowin((char *)hypert->filename, buf), sbuf), tmp2);
#else
              int nSize2 = hstr2ksstr(kstr2hstr( (uchar *)urltounix((char *)hypert->filename, buf), sbuf), tmp2);
#endif
          padd(ascii("xlink:type"), sXML_CDATA, ascii("simple"));
          if( strlen(tmp) > 0 && strcmp( tmp, "[HTML]") ){
              sprintf( buf, "%s#%s",tmp2, tmp);
              padd(ascii("xlink:href"), sXML_CDATA, OUString(buf, nSize2 + nSize+1, RTL_TEXTENCODING_EUC_KR));
          }
          else{
              sprintf( buf, "%s",tmp2);
              padd(ascii("xlink:href"), sXML_CDATA, OUString(buf, nSize2, RTL_TEXTENCODING_EUC_KR));

          }
     }
    else
    {
        char tmp[256];
        padd(ascii("xlink:type"), sXML_CDATA, ascii("simple"));
        int nSize = hstr2ksstr(hypert->bookmark, tmp);
        sprintf( buf, "#%s", tmp);
        padd(ascii("xlink:href"), sXML_CDATA, OUString(buf, nSize+1, RTL_TEXTENCODING_EUC_KR));
    }
    rstartEl(ascii("draw:a"), rList);
    pList->clear();
    makeTextBox(hbox);
    rendEl(ascii("draw:a"));
}


/**
 * platform?????? ?????????? href?? C:\?? D:\?? ?????? ???? ???????? ????????????
 * C:\ => ??????, D:\ => ????(/)?? ????????. ??????
 * ?????? ???????????????? ???? ????????.
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
                padd(ascii("draw:style-name"), sXML_CDATA,
                    ascii(Int2Str(hbox->style.boxnum, "CapBox%d", buf)));
                padd(ascii("draw:name"), sXML_CDATA,
                    ascii(Int2Str(hbox->style.boxnum, "CaptionBox%d", buf)));
                     padd(ascii("draw:z-index"), sXML_CDATA,
                    ascii(Int2Str(hbox->zorder, "%d", buf)));
                switch (hbox->style.anchor_type)
                {
                    case CHAR_ANCHOR:
                        padd(ascii("text:anchor-type"), sXML_CDATA, ascii("as-char"));
                        break;
                    case PARA_ANCHOR:
                        padd(ascii("text:anchor-type"), sXML_CDATA, ascii("paragraph"));
                        break;
                    case PAGE_ANCHOR:
                    case PAPER_ANCHOR:
                    {
                        // os unused
                        // HWPInfo *hwpinfo = hwpfile.GetHWPInfo();
                        padd(ascii("text:anchor-type"), sXML_CDATA, ascii("page"));
                        padd(ascii("text:anchor-page-number"), sXML_CDATA,
                            ascii(Int2Str(hbox->pgno +1, "%d", buf)));
                        break;
                    }
                }
                if (hbox->style.anchor_type != CHAR_ANCHOR)
                {
                    padd(ascii("svg:x"), sXML_CDATA,
                        Double2Str(WTMM(  hbox->pgx + hbox->style.margin[0][0] )) + ascii("mm"));
                    padd(ascii("svg:y"), sXML_CDATA,
                        Double2Str(WTMM(  hbox->pgy + hbox->style.margin[0][2] )) + ascii("mm"));
                }
                padd(ascii("svg:width"), sXML_CDATA,
                    Double2Str(WTMM( hbox->box_xs + hbox->style.margin[1][0] + hbox->style.margin[1][1] )) + ascii("mm"));
                padd(ascii("fo:min-height"), sXML_CDATA,
                    Double2Str(WTMM( hbox->box_ys + hbox->style.margin[1][2] + hbox->style.margin[1][3] + hbox->cap_ys )) + ascii("mm"));
                rstartEl(ascii("draw:text-box"), rList);
                pList->clear();
                if( hbox->cap_pos % 2 )           /* ?????? ?????? ???????? */
                {
                    parsePara(hbox->caption.first());
                }
                padd( ascii("text:style-name"), sXML_CDATA, ascii("Standard"));
                rstartEl(ascii("text:p"), rList);
                pList->clear();
            }
            if( hbox->ishyper )
            {
                padd(ascii("xlink:type"), sXML_CDATA, ascii("simple"));
#ifdef _WIN32
                if( hbox->follow[4] != 0 )
                    padd(ascii("xlink:href"), sXML_CDATA, (hconv(kstr2hstr(hbox->follow + 4, sbuf), gstr)));
                else
                    padd(ascii("xlink:href"), sXML_CDATA, (hconv(kstr2hstr(hbox->follow + 5, sbuf), gstr)));
#else
                if( hbox->follow[4] != 0 )
                    padd(ascii("xlink:href"), sXML_CDATA,
                        (hconv(kstr2hstr((uchar *)urltounix((char *)(hbox->follow + 4), buf), sbuf), gstr)));
                else
                    padd(ascii("xlink:href"), sXML_CDATA,
                        (hconv(kstr2hstr((uchar *)urltounix((char *)(hbox->follow + 5), buf), sbuf), gstr)));
#endif
                rstartEl(ascii("draw:a"), rList);
                pList->clear();
            }
            padd(ascii("draw:style-name"), sXML_CDATA,
                ascii(Int2Str(hbox->style.boxnum, "G%d", buf)));
            padd(ascii("draw:name"), sXML_CDATA,
                ascii(Int2Str(hbox->style.boxnum, "Image%d", buf)));

            // os unused
            // int x = 0;
            // int y = 0;

            if( hbox->style.cap_len <= 0 )
            {
                     padd(ascii("draw:z-index"), sXML_CDATA,
                    ascii(Int2Str(hbox->zorder, "%d", buf)));
                switch (hbox->style.anchor_type)
                {
                    case CHAR_ANCHOR:
                        padd(ascii("text:anchor-type"), sXML_CDATA, ascii("as-char"));
                        break;
                    case PARA_ANCHOR:
                        padd(ascii("text:anchor-type"), sXML_CDATA, ascii("paragraph"));
                        break;
                    case PAGE_ANCHOR:
                    case PAPER_ANCHOR:
                    {
                        // os unused
                        // HWPInfo *hwpinfo = hwpfile.GetHWPInfo();
                        padd(ascii("text:anchor-type"), sXML_CDATA, ascii("page"));
                        padd(ascii("text:anchor-page-number"), sXML_CDATA,
                            ascii(Int2Str(hbox->pgno +1, "%d", buf)));
                        break;
                    }
                }
                if (hbox->style.anchor_type != CHAR_ANCHOR)
                {
                    padd(ascii("svg:x"), sXML_CDATA,
                        Double2Str(WTMM( hbox->pgx + hbox->style.margin[0][0] )) + ascii("mm"));
                    padd(ascii("svg:y"), sXML_CDATA,
                        Double2Str(WTMM( hbox->pgy + hbox->style.margin[0][2] )) + ascii("mm"));
                }
            }
            else
            {
                padd(ascii("text:anchor-type"), sXML_CDATA, ascii("as-char"));
                padd(ascii("svg:y"), sXML_CDATA, ascii("0cm"));
            }
            padd(ascii("svg:width"), sXML_CDATA,
                Double2Str(WTMM( hbox->box_xs + hbox->style.margin[1][0] + hbox->style.margin[1][1])) + ascii("mm"));
            padd(ascii("svg:height"), sXML_CDATA,
                Double2Str(WTMM( hbox->box_ys + hbox->style.margin[1][2] + hbox->style.margin[1][3])) + ascii("mm"));

            if ( hbox->pictype == PICTYPE_FILE ){
#ifdef _WIN32
                sprintf(buf, "file:///%s", hbox->picinfo.picun.path );
                padd(ascii("xlink:href"), sXML_CDATA, (hconv(kstr2hstr((uchar *) buf, sbuf), gstr)));
#else
                padd(ascii("xlink:href"), sXML_CDATA,
                    (hconv(kstr2hstr((uchar *) urltounix( hbox->picinfo.picun.path, buf), sbuf), gstr)));
#endif
                padd(ascii("xlink:type"), sXML_CDATA, ascii("simple"));
                padd(ascii("xlink:show"), sXML_CDATA, ascii("embed"));
                padd(ascii("xlink:actuate"), sXML_CDATA, ascii("onLoad"));
            }

                if( hbox->pictype == PICTYPE_OLE )
                    rstartEl(ascii("draw:object-ole"), rList);
                else
                    rstartEl(ascii("draw:image"), rList);
            pList->clear();
            if (hbox->pictype == PICTYPE_EMBED || hbox->pictype == PICTYPE_OLE)
            {
                rstartEl(ascii("office:binary-data"), rList);
                pList->clear();
                     if( hbox->pictype == PICTYPE_EMBED ){
                         EmPicture *emp = hwpfile.GetEmPicture(hbox);
                         if( emp )
                              rchars(ascii(base64_encode_string( emp->data, emp->size )));
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
                                 rchars(ascii(""));
                             }
                             else{
                                 rc = OleLoad(srcsto, IID_IUnknown, NULL, (LPVOID*)&pObj);
                                 if( rc != S_OK ){
                                     srcsto->Release();
                                     rchars(ascii(""));
                                 }
                                 else{
                                     rchars(ascii(base64_encode_string( (uchar *)pObj, strlen((char *)pObj) )));
                                     pObj->Release();
                                     srcsto->Release();
                                 }
                             }
#else
                             rchars(ascii(""));
#endif
                         }
                     }
                rendEl(ascii("office:binary-data"));
            }
                if( hbox->pictype == PICTYPE_OLE )
                    rendEl(ascii("draw:object-ole"));
                else
                    rendEl(ascii("draw:image"));
            if( hbox->ishyper )
            {
                rendEl(ascii("draw:a"));
            }
            if( hbox->style.cap_len > 0 )
            {
                rendEl( ascii("text:p"));
                if( !(hbox->cap_pos % 2))         /* ?????? ???????? ????????, */
                {
                    parsePara(hbox->caption.first());
                }
                rendEl( ascii("draw:text-box"));
            }
            break;
        }
        case PICTYPE_DRAW:
              if( hbox->picinfo.picdraw.zorder > 0 )
                 padd(ascii("draw:z-index"), sXML_CDATA,
                      ascii(Int2Str( hbox->picinfo.picdraw.zorder + 10000, "%d", buf)));
            makePictureDRAW( (HWPDrawingObject *) hbox->picinfo.picdraw.hdo, hbox);
            break;
        case PICTYPE_UNKNOWN:
            break;
    }
}


#define DBL(x)	((x) * (x))
void HwpReader::makePictureDRAW(HWPDrawingObject *drawobj, Picture * hbox)
{
    int x = hbox->pgx;
    int y = hbox->pgy;
    int a, b;
    sal_Bool bIsRotate = sal_False;

    while (drawobj)
    {
        padd(ascii("draw:style-name"), sXML_CDATA,
            ascii(Int2Str(drawobj->index, "Draw%d", buf)));
          a = 0; b = 0;

        switch (hbox->style.anchor_type)
        {
            case CHAR_ANCHOR:
                padd(ascii("text:anchor-type"), sXML_CDATA, ascii("as-char"));
                break;
            case PARA_ANCHOR:
                padd(ascii("text:anchor-type"), sXML_CDATA, ascii("paragraph"));
                break;
            case PAGE_ANCHOR:
            case PAPER_ANCHOR:
            {
                HWPInfo *hwpinfo = hwpfile.GetHWPInfo();
                padd(ascii("text:anchor-type"), sXML_CDATA, ascii("page"));
                padd(ascii("text:anchor-page-number"), sXML_CDATA,
                    ascii(Int2Str(hbox->pgno +1, "%d", buf)));
                a = hwpinfo->paper.left_margin;
                b = hwpinfo->paper.top_margin + hwpinfo->paper.header_length;
                break;
            }
        }

        if (drawobj->type == HWPDO_CONTAINER)
        {
            rstartEl(ascii("draw:g"), rList);
            pList->clear();
            makePictureDRAW(drawobj->child, hbox);
            rendEl(ascii("draw:g"));
        }
        else
        {
                bIsRotate = sal_False;
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
                         /* ???????????? ???? */
                         pt[i].y = -(pal->pt[i].y - drawobj->property.rot_originy);
                     }

                double rotate, skewX ;

                     /* 2 - ?????? ???? */
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

                     /* 4 - ???? ???? */
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
                         trans = ascii("skewX (") + Double2Str(skewX)
                                  + ascii(") rotate (") + Double2Str(rotate)
                                  + ascii(") translate (") + Double2Str(WTMM(x + a + drawobj->offset2.x + pal->pt[0].x)) + ascii("mm ")
                                  + Double2Str(WTMM(y + b + drawobj->offset2.y + pal->pt[0].y)) + ascii("mm)");
                          bIsRotate = sal_True;
                     }
                     else if( skewX != 0.0 ){
                         trans = ascii("skewX (") + Double2Str(skewX)
                                  + ascii(") translate (") + Double2Str(WTMM(x + a + drawobj->offset2.x + pal->pt[0].x)) + ascii("mm ")
                                  + Double2Str(WTMM(y + b + drawobj->offset2.y + pal->pt[0].y)) + ascii("mm)");
                          bIsRotate = sal_True;
                     }
                     else if( rotate != 0.0 ){
                         trans = ascii("rotate (") + Double2Str(rotate)
                                  + ascii(") translate (") + Double2Str(WTMM(x + a + drawobj->offset2.x + pal->pt[0].x)) + ascii("mm ")
                                  + Double2Str(WTMM(y + b + drawobj->offset2.y + pal->pt[0].y)) + ascii("mm)");
                          bIsRotate = sal_True;
                     }
                     if( bIsRotate == sal_True ){
                         drawobj->extent.w = (int)sqrt(double(DBL(pt[1].x-pt[0].x)+DBL(pt[1].y-pt[0].y)));
                         drawobj->extent.h = (int)sqrt(double(DBL(pt[2].x-pt[1].x)+DBL(pt[2].y-pt[1].y)));
                         padd(ascii("draw:transform"), sXML_CDATA, trans);
                     }
            }
            switch (drawobj->type)
            {
                case HWPDO_LINE:                  /* ?? - ????????, ??????. */
                    if( drawobj->u.line_arc.flip & 0x01 )
                    {
                        padd(ascii("svg:x1"), sXML_CDATA,
                            Double2Str (WTMM(x + a + drawobj->offset2.x + drawobj->extent.w)) + ascii("mm"));
                        padd(ascii("svg:x2"), sXML_CDATA,
                            Double2Str (WTMM( x + a + drawobj->offset2.x )) + ascii("mm"));
                    }
                    else
                    {
                        padd(ascii("svg:x1"), sXML_CDATA,
                            Double2Str (WTMM( x + a + drawobj->offset2.x )) + ascii("mm"));
                        padd(ascii("svg:x2"), sXML_CDATA,
                            Double2Str (WTMM(x + a + drawobj->offset2.x + drawobj->extent.w)) + ascii("mm"));
                    }
                    if( drawobj->u.line_arc.flip & 0x02 )
                    {
                        padd(ascii("svg:y1"), sXML_CDATA,
                            Double2Str (WTMM( y + b + drawobj->offset2.y + drawobj->extent.h ) ) + ascii("mm"));
                        padd(ascii("svg:y2"), sXML_CDATA,
                            Double2Str (WTMM( y + b  + drawobj->offset2.y )) + ascii("mm"));
                    }
                    else
                    {
                        padd(ascii("svg:y1"), sXML_CDATA,
                            Double2Str (WTMM( y + b  + drawobj->offset2.y)) + ascii("mm"));
                        padd(ascii("svg:y2"), sXML_CDATA,
                            Double2Str (WTMM(y + b + drawobj->offset2.y + drawobj->extent.h)) + ascii("mm"));
                    }

                    rstartEl(ascii("draw:line"), rList);
                    pList->clear();
                    rendEl(ascii("draw:line"));
                    break;
                case HWPDO_RECT:                  /* ?????? - ????????, ????/???? */
                    if( !bIsRotate )
                    {
                        padd(ascii("svg:x"), sXML_CDATA,
                            Double2Str (WTMM( x + a + drawobj->offset2.x)) + ascii("mm"));
                        padd(ascii("svg:y"), sXML_CDATA,
                            Double2Str (WTMM( y + b + drawobj->offset2.y)) + ascii("mm"));
                    }
                    padd(ascii("svg:width"), sXML_CDATA,
                        Double2Str (WTMM( drawobj->extent.w )) + ascii("mm"));
                    padd(ascii("svg:height"), sXML_CDATA,
                        Double2Str (WTMM( drawobj->extent.h )) + ascii("mm"));
                    if( drawobj->property.flag & 0x01 )
                    {
                        int value = drawobj->extent.w < drawobj->extent.h ?
                            drawobj->extent.w : drawobj->extent.h ;
                        padd(ascii("draw:corner-radius"), sXML_CDATA,
                            Double2Str (WTMM( value/10 )) + ascii("mm"));
                    }
                    else if( drawobj->property.flag & 0x04 )
                    {
                        int value = drawobj->extent.w < drawobj->extent.h ?
                            drawobj->extent.w : drawobj->extent.h ;
                        padd(ascii("draw:corner-radius"), sXML_CDATA,
                            Double2Str (WTMM( value / 2)) + ascii("mm"));
                    }

                    rstartEl(ascii("draw:rect"), rList);
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
                    rendEl(ascii("draw:rect"));
                    break;
                case HWPDO_ELLIPSE:               /* ???? - ????????, ????/???? */
                case HWPDO_ADVANCED_ELLIPSE:      /* ?????? ???? */
                {
                    if( !bIsRotate )
                    {
                        padd(ascii("svg:x"), sXML_CDATA,
                            Double2Str (WTMM( x + a + drawobj->offset2.x)) + ascii("mm"));
                        padd(ascii("svg:y"), sXML_CDATA,
                            Double2Str (WTMM( y + b + drawobj->offset2.y)) + ascii("mm"));
                    }

                            padd(ascii("svg:width"), sXML_CDATA,
                                 Double2Str (WTMM( drawobj->extent.w )) + ascii("mm"));
                            padd(ascii("svg:height"), sXML_CDATA,
                                 Double2Str (WTMM( drawobj->extent.h )) + ascii("mm"));
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
                                        padd(ascii("draw:kind"), sXML_CDATA, ascii("section"));
                                    else
                                        padd(ascii("draw:kind"), sXML_CDATA, ascii("arc"));
                                    padd(ascii("draw:start-angle"), sXML_CDATA, Double2Str(start_angle ));
                                    padd(ascii("draw:end-angle"), sXML_CDATA, Double2Str(end_angle));
                                }
                          }
                    rstartEl(ascii("draw:ellipse"), rList);
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
                    rendEl(ascii("draw:ellipse"));
                    break;

                }
                case HWPDO_ARC:                   /* ?? */
                case HWPDO_ADVANCED_ARC:
                {
                    /* ??????????, ???????????? ???? ?????? ?????? ???????? ????. */
                         uint flip = drawobj->u.line_arc.flip;
                    if( !bIsRotate )
                    {
                              if( ( flip == 0 || flip == 2 ) && drawobj->type == HWPDO_ARC)
                                    padd(ascii("svg:x"), sXML_CDATA,
                                         Double2Str (WTMM( x + a + drawobj->offset2.x - drawobj->extent.w)) + ascii("mm"));
                              else
                                    padd(ascii("svg:x"), sXML_CDATA,
                                         Double2Str (WTMM( x + a + drawobj->offset2.x)) + ascii("mm"));
                              if( ( flip == 0 || flip == 1 ) && drawobj->type == HWPDO_ARC)
                                    padd(ascii("svg:y"), sXML_CDATA,
                                         Double2Str (WTMM( y + b + drawobj->offset2.y - drawobj->extent.h)) + ascii("mm"));
                              else
                                    padd(ascii("svg:y"), sXML_CDATA,
                                         Double2Str (WTMM( y + b + drawobj->offset2.y)) + ascii("mm"));
                    }

                            padd(ascii("svg:width"), sXML_CDATA,
                                 Double2Str (WTMM( drawobj->extent.w * 2)) + ascii("mm"));
                            padd(ascii("svg:height"), sXML_CDATA,
                                 Double2Str (WTMM( drawobj->extent.h * 2)) + ascii("mm"));
                            if( drawobj->property.flag & HWPDO_FLAG_DRAW_PIE ||
                                     drawobj->property.fill_color < 0xffffff )
                                padd(ascii("draw:kind"), sXML_CDATA, ascii("section"));
                            else
                                padd(ascii("draw:kind"), sXML_CDATA, ascii("arc"));

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
                                 padd(ascii("draw:start-angle"), sXML_CDATA, Double2Str(start_angle * 180. / PI));
                                 padd(ascii("draw:end-angle"), sXML_CDATA, Double2Str(end_angle * 180. / PI));

                            }
                            else{
                            if( drawobj->u.line_arc.flip == 0 )
                            {
                                 padd(ascii("draw:start-angle"), sXML_CDATA, ascii("270"));
                                 padd(ascii("draw:end-angle"), sXML_CDATA, ascii("0"));
                            }
                            else if( drawobj->u.line_arc.flip == 1 )
                            {
                                 padd(ascii("draw:start-angle"), sXML_CDATA, ascii("180"));
                                 padd(ascii("draw:end-angle"), sXML_CDATA, ascii("270"));
                            }
                            else if( drawobj->u.line_arc.flip == 2 )
                            {
                                 padd(ascii("draw:start-angle"), sXML_CDATA, ascii("0"));
                                 padd(ascii("draw:end-angle"), sXML_CDATA, ascii("90"));
                            }
                            else
                            {
                                 padd(ascii("draw:start-angle"), sXML_CDATA, ascii("90"));
                                 padd(ascii("draw:end-angle"), sXML_CDATA, ascii("180"));
                            }
                            }
                    rstartEl(ascii("draw:ellipse"), rList);
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
                    rendEl(ascii("draw:ellipse"));
                    break;

                }
                     case HWPDO_CURVE: /* ???? : ?????????? ????. */
                {
                         sal_Bool bIsNatural = sal_True;
                         // os unused
                         // int nCount = drawobj->u.freeform.npt;
                   if( drawobj->property.flag >> 5 & 0x01){
                             bIsNatural = sal_False;
                         }
                    if( !bIsRotate )
                    {
                        padd(ascii("svg:x"), sXML_CDATA,
                            Double2Str (WTMM( x + a + drawobj->offset2.x)) + ascii("mm"));
                        padd(ascii("svg:y"), sXML_CDATA,
                            Double2Str (WTMM( y + b + drawobj->offset2.y)) + ascii("mm"));
                    }
                    padd(ascii("svg:width"), sXML_CDATA,
                        Double2Str (WTMM( drawobj->extent.w )) + ascii("mm"));
                    padd(ascii("svg:height"), sXML_CDATA,
                        Double2Str (WTMM( drawobj->extent.h )) + ascii("mm"));
                    sprintf(buf, "0 0 %d %d", WTSM(drawobj->extent.w) , WTSM(drawobj->extent.h) );
                    padd(ascii("svg:viewBox"), sXML_CDATA, ascii(buf) );

                    OUString oustr;

                    if (drawobj->u.freeform.npt > 2){
                              int n, i;
                              if( bIsNatural == sal_True )
                                  n = drawobj->u.freeform.npt;
                              else
                                  n = drawobj->u.freeform.npt ;

                              double *xarr = new double[n+1];
                              double *yarr = new double[n+1];
                              double *tarr = new double[n+1];

                              double *xb = 0L;
                              double *yb = 0L;

                              double *carr = 0L;
                              double *darr = 0L;


                              for( i = 0 ; i < n ; i++ ){
                                  xarr[i] = drawobj->u.freeform.pt[i].x;
                                  yarr[i] = drawobj->u.freeform.pt[i].y;
                                  tarr[i] = i;
                              }
                              xarr[n] = xarr[0];
                              yarr[n] = yarr[0];
                              tarr[n] = n;

                              if( bIsNatural == sal_False ){
                                  PeriodicSpline(n, tarr, xarr, xb, carr, darr);
                                  // prevent memory leak
                                  delete[] carr;
                                  carr = 0;
                                  delete[] darr;
                                  darr = 0;
                                  PeriodicSpline(n, tarr, yarr, yb, carr, darr);
                              }
                              else{
                                  NaturalSpline(n, tarr, xarr, xb, carr, darr);
                                  // prevent memory leak
                                  delete[] carr;
                                  carr = 0;
                                  delete[] darr;
                                  darr = 0;
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

                    padd(ascii("svg:d"), sXML_CDATA, oustr);

                    rstartEl(ascii("draw:path"), rList);
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
                    rendEl(ascii("draw:path"));
                    break;
                }
                case HWPDO_CLOSED_FREEFORM:
                case HWPDO_FREEFORM:              /* ?????? */
                {
                    bool bIsPolygon = false;

                            padd(ascii("svg:x"), sXML_CDATA,
                                 Double2Str (WTMM( x + a + drawobj->offset2.x)) + ascii("mm"));
                            padd(ascii("svg:y"), sXML_CDATA,
                                 Double2Str (WTMM( y + b + drawobj->offset2.y)) + ascii("mm"));

                    padd(ascii("svg:width"), sXML_CDATA,
                        Double2Str (WTMM( drawobj->extent.w )) + ascii("mm"));
                    padd(ascii("svg:height"), sXML_CDATA,
                        Double2Str (WTMM( drawobj->extent.h )) + ascii("mm"));

                    sprintf(buf, "0 0 %d %d", WTSM(drawobj->extent.w), WTSM(drawobj->extent.h));
                    padd(ascii("svg:viewBox"), sXML_CDATA, ascii(buf) );

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
                    padd(ascii("draw:points"), sXML_CDATA, oustr);

                    if( drawobj->property.fill_color <=  0xffffff ||
                        drawobj->property.pattern_type != 0)
                    {
                        bIsPolygon = true;
                    }

                    if(bIsPolygon)
                    {
                        rstartEl(ascii("draw:polygon"), rList);
                        pList->clear();
                        if( drawobj->property.flag >> 19 & 0x01 &&
                                                  // As Textbox
                            drawobj->property.pPara )
                        {
                            HWPPara *pPara = drawobj->property.pPara;
                            //	parsePara(pPara);
                            while(pPara)
                            {
                                make_text_p1( pPara );
                                pPara = pPara->Next();
                            }
                        }
                        rendEl(ascii("draw:polygon"));
                    }
                    else
                    {
                        rstartEl(ascii("draw:polyline"), rList);
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
                        rendEl(ascii("draw:polyline"));
                    }
                    break;
                }
                case HWPDO_TEXTBOX:
                    if( !bIsRotate )
                    {
                        padd(ascii("svg:x"), sXML_CDATA,
                            Double2Str (WTMM( x + a + drawobj->offset2.x)) + ascii("mm"));
                        padd(ascii("svg:y"), sXML_CDATA,
                            Double2Str (WTMM( y + b + drawobj->offset2.y)) + ascii("mm"));
                    }
                    padd(ascii("svg:width"), sXML_CDATA,
                        Double2Str (WTMM( drawobj->extent.w )) + ascii("mm"));
                    padd(ascii("svg:height"), sXML_CDATA,
                        Double2Str (WTMM( drawobj->extent.h )) + ascii("mm"));
                    if( drawobj->property.flag & 0x01 )
                    {
                        int value = drawobj->extent.w < drawobj->extent.h ?
                            drawobj->extent.w : drawobj->extent.h ;
                        padd(ascii("draw:corner-radius"), sXML_CDATA,
                            Double2Str (WTMM( value/10 )) + ascii("mm"));
                    }
                    else if( drawobj->property.flag & 0x04 )
                    {
                        int value = drawobj->extent.w < drawobj->extent.h ?
                            drawobj->extent.w : drawobj->extent.h ;
                        padd(ascii("draw:corner-radius"), sXML_CDATA,
                            Double2Str (WTMM( value / 2)) + ascii("mm"));
                    }

                    rstartEl(ascii("draw:text-box"), rList);
                    pList->clear();

                    HWPPara *pPara = drawobj->u.textbox.h;
                                //parsePara(pPara);
                    while(pPara)
                    {
                        make_text_p1( pPara );
                        pPara = pPara->Next();
                    }

                    rendEl(ascii("draw:text-box"));
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
    padd(ascii("text:style-name"), sXML_CDATA, ascii("Horizontal Line"));
    rstartEl( ascii("text:p"), rList);
    pList->clear();
}


/**
 * ????-????-???????? : ?????????? ???? ?????? ????????.
 * ?????? ?????? ?? ??????, ???? ???????? ???????? ????????.
 */
void HwpReader::makeHidden(Hidden * hbox)
{
    int l = 0, res;
     hchar dest[3];

    padd(ascii("text:condition"), sXML_CDATA, ascii(""));
    padd(ascii("text:string-value"), sXML_CDATA, ascii(""));
    rstartEl(ascii("text:hidden-text"), rList);
    pList->clear();
    HWPPara *para = hbox->plist.first();

    while (para)
    {
        for (int n = 0; n < para->nch && para->hhstr[n]->hh;
            n += para->hhstr[n]->WSize())
        {
              res = hcharconv(para->hhstr[n]->hh, dest, UNICODE);
              for( int j = 0 ; j < res ; j++ )
                    gstr[l++] = dest[j];
        }
        para = para->Next();
    }
    makeChars(gstr, l);
    rendEl(ascii("text:hidden-text"));
}


/**
 * ?????? text:footnote, ?????? text:endnote?? ????
 */
void HwpReader::makeFootnote(Footnote * hbox)
{
    if (hbox->type)
    {
        padd(ascii("text:id"), sXML_CDATA,
            ascii(Int2Str(hbox->number, "edn%d", buf)));
        rstartEl(ascii("text:endnote"), rList);
        pList->clear();
        padd(ascii("text:label"), sXML_CDATA,
            ascii(Int2Str(hbox->number, "%d", buf)));
        rstartEl(ascii("text:endnote-citation"), rList);
        pList->clear();
        rchars(ascii(Int2Str(hbox->number, "%d", buf)));
        rendEl(ascii("text:endnote-citation"));
        rstartEl(ascii("text:endnote-body"), rList);
        parsePara(hbox->plist.first());
        rendEl(ascii("text:endnote-body"));
        rendEl(ascii("text:endnote"));
    }
    else
    {
        padd(ascii("text:id"), sXML_CDATA,
            ascii(Int2Str(hbox->number, "ftn%d", buf)));
        rstartEl(ascii("text:footnote"), rList);
        pList->clear();
        padd(ascii("text:label"), sXML_CDATA,
            ascii(Int2Str(hbox->number, "%d", buf)));
        rstartEl(ascii("text:footnote-citation"), rList);
        pList->clear();
        rchars(ascii(Int2Str(hbox->number, "%d", buf)));
        rendEl(ascii("text:footnote-citation"));
        rstartEl(ascii("text:footnote-body"), rList);
        parsePara(hbox->plist.first());
        rendEl(ascii("text:footnote-body"));
        rendEl(ascii("text:footnote"));
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
            rstartEl(ascii("text:page-number"), rList);
            rchars(ascii(Int2Str(hbox->number, "%d", buf)));
            rendEl(ascii("text:page-number"));
            break;
        case FNNUM_AUTO:
/*
padd(ascii("text:ref-name"), sXML_CDATA,
     ascii(Int2Str(hbox->number + 1, "ftn%d", buf)));
rstartEl(ascii("text:footnote-ref"), rList);
pList->clear();
//rchars(ascii(Int2Str(hbox->number + 1, "%d", buf)));
rchars(ascii(""));
rendEl(ascii("text:footnote-ref"));
*/
            break;
        case ENNUM_AUTO:
/*
padd(ascii("text:ref-name"), sXML_CDATA,
     ascii(Int2Str(hbox->number + 1, "edn%d", buf)));
rstartEl(ascii("text:endnote-ref"), rList);
pList->clear();
//rchars(ascii(Int2Str(hbox->number + 1, "%d", buf)));
rchars(ascii(""));
rendEl(ascii("text:endnote-ref"));
*/
            break;
        case EQUNUM_AUTO:
        case PICNUM_AUTO:
            padd(ascii("text:ref-name"),sXML_CDATA,
                ascii(Int2Str(hbox->number, "refIllustration%d", buf)));
            padd(ascii("text:name"),sXML_CDATA, ascii("Illustration"));
            padd(ascii("style:num-format"),sXML_CDATA, ascii("1"));
            rstartEl(ascii("text:sequence"), rList);
            rchars(ascii(Int2Str(hbox->number, "%d", buf)));
            rendEl(ascii("text:sequence"));
            break;
        case TBLNUM_AUTO:
            padd(ascii("text:ref-name"),sXML_CDATA,
                ascii(Int2Str(hbox->number, "refTable%d", buf)));
            padd(ascii("text:name"),sXML_CDATA, ascii("Table"));
            padd(ascii("style:num-format"),sXML_CDATA, ascii("1"));
            rstartEl(ascii("text:sequence"), rList);
            rchars(ascii(Int2Str(hbox->number, "%d", buf)));
            rendEl(ascii("text:sequence"));
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
    else                                          /* ?? ?????? ???????? ??????. */
    {
        if( d->nPnPos == 1 )
            nPos = 1;
        else if( d->nPnPos == 3 )
            nPos = 3;
    }

    padd(ascii("draw:style-name"), sXML_CDATA,
        ascii(Int2Str(nPos, "PNBox%d", buf)));
    padd(ascii("draw:name"), sXML_CDATA,
        ascii(Int2Str(nPos, "PageNumber%d", buf)));
    padd(ascii("text:anchor-type"), sXML_CDATA, ascii("paragraph"));
    padd(ascii("svg:y"), sXML_CDATA, ascii("0cm"));
    padd(ascii("svg:width"), sXML_CDATA, ascii("2.0cm"));
    padd(ascii("fo:min-height"), sXML_CDATA, ascii("0.5cm"));
    rstartEl(ascii("draw:text-box"), rList);
    pList->clear();

    padd(ascii("text:style-name"), sXML_CDATA,
        ascii(Int2Str(nPos, "PNPara%d", buf)));
    rstartEl(ascii("text:p"), rList);
    pList->clear();
    if( hbox->shape > 2 )
        rchars(ascii("- "));
    if( hbox->shape % 3 == 0 )
        padd(ascii("style:num-format"), sXML_CDATA, ascii("1"));
    else if( hbox->shape % 3 == 1 )
        padd(ascii("style:num-format"), sXML_CDATA, ascii("I"));
    else
        padd(ascii("style:num-format"), sXML_CDATA, ascii("i"));
    padd(ascii("text:select-page"), sXML_CDATA, ascii("current"));
    rstartEl(ascii("text:page-number"), rList);
    pList->clear();
    rchars(ascii("2"));
    rendEl(ascii("text:page-number"));
    if( hbox->shape > 2 )
        rchars(ascii(" -"));
    rendEl(ascii("text:p"));
    rendEl(ascii("draw:text-box"));
}


/**
 * mail merge operation using hwp adressbook and hwp data form.
 * not support operation in OO writer.
 */
void HwpReader::makeMailMerge(MailMerge * hbox)
{
    hbox->GetString(sbuf, 255);
    rchars((hconv(sbuf, gstr)));
}


/**
 * Make heading contents file using toc marks
 * not support operation.
 */
void HwpReader::makeTocMark(TocMark *  )          /*hbox */
{
}


/**
 * Make search character table in automatic
 * not support operation
 */
void HwpReader::makeIndexMark(IndexMark *  )      /*hbox */
{
}


void HwpReader::makeOutline(Outline * hbox)
{
    if( hbox->kind == 1 )
         rchars(OUString(hbox->GetUnicode(sbuf, 255)));
}


void HwpReader::parsePara(HWPPara * para, sal_Bool bParaStart)
{

    while (para)
    {
        if( para->nch == 1)
        {
            if( !bParaStart )
            {
                padd(ascii("text:style-name"), sXML_CDATA,
                    ascii(getPStyleName(para->GetParaShape()->index, buf)));
                rstartEl( ascii("text:p"),rList);
                pList->clear();
            }
            if( d->bFirstPara && d->bInBody )
            {
/* for HWP's Bookmark */
                strcpy(buf,"[?????? ????]");
                padd(ascii("text:name"), sXML_CDATA, OUString(buf, strlen(buf), RTL_TEXTENCODING_EUC_KR));
                rstartEl(ascii("text:bookmark"), rList);
                pList->clear();
                rendEl(ascii("text:bookmark"));
                d->bFirstPara = sal_False;
            }
            if( d->bInHeader )
            {
                makeShowPageNum();
                d->bInHeader = sal_False;
            }

            rendEl( ascii("text:p") );
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
