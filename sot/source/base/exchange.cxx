/*************************************************************************
 *
 *  $RCSfile: exchange.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: shane $ $Date: 2000-10-02 19:06:48 $
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
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#define _SOT_EXCHANGE_CXX
#define SOT_STRING_LIST
#define _SOT_FORMATS_INCLUDE_SYSTEMFORMATS

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _SOLAR_H
#include <tools/solar.h>
#endif

#include <tools/string.hxx>
#include <sotdata.hxx>
#include <exchange.hxx>
#include <formats.hxx>

#pragma hdrstop

/*
    In dieser Tabelle stehen alle im Office verwendeten Format-Bezeichner.
    Die Tabelle ist nach den Formatstring-Ids sortiert und jede Id
    ist um genau 1 groesser als ihre Vorgaenger-Id, damit die Id als
    Tabellenindex benutzt werden kann.
*/

static const sal_Char* aFormatArray_Impl[] =
{
/*  0 SOT_FORMAT_SYSTEM_START*/             "",
/*  1 SOT_FORMAT_STRING*/                   "Text",
/*  2 SOT_FORMAT_BITMAP*/                   "Bitmap",
/*  3 SOT_FORMAT_GDIMETAFILE*/              "GDIMetaFile",
/*  4 SOT_FORMAT_PRIVATE*/                  "Private",
/*  5 SOT_FORMAT_FILE*/                     "FileName",
/*  6 SOT_FORMAT_FILE_LIST*/                "FileList",
/*  7 EMPTY*/                               "",
/*  8 EMPTY*/                               "",
/*  9 EMPTY*/                               "",
/* 10 SOT_FORMAT_RTF*/                      "Rich Text Format",

/* 11 SOT_FORMATSTR_ID_DRAWING*/            "StarOffice Drawing Format",
/* 12 SOT_FORMATSTR_ID_SVXB*/               "SVXB (StarView Bitmap/Animation)",
/* 13 SOT_FORMATSTR_ID_SVIM*/               "SVIM (StarView ImageMap)",
/* 14 SOT_FORMATSTR_ID_XFA*/                "XFA (XOutDev FillAttr)",
/* 15 SOT_FORMATSTR_ID_EDITENGINE*/         "EditEngineFormat",
/* 16 SOT_FORMATSTR_ID_INTERNALLINK_STATE*/ "StatusInfo vom SvxInternalLink",
/* 17 SOT_FORMATSTR_ID_SOLK*/               "SOLK (StarOffice Link)",
/* 18 SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK*/  "Netscape Bookmark",
/* 19 SOT_FORMATSTR_ID_TREELISTBOX*/        "SV_LBOX_DD_FORMAT",
/* 20 SOT_FORMATSTR_ID_NATIVE*/             "Native",
/* 21 SOT_FORMATSTR_ID_OWNERLINK*/          "OwnerLink",
/* 22 SOT_FORMATSTR_ID_STARSERVER*/         "StarServerFormat",
/* 23 SOT_FORMATSTR_ID_STAROBJECT*/         "StarObjectFormat",
/* 24 SOT_FORMATSTR_ID_APPLETOBJECT*/       "Applet Object",
/* 25 SOT_FORMATSTR_ID_PLUGIN_OBJECT*/      "PlugIn Object",
/* 26 SOT_FORMATSTR_ID_STARWRITER_30*/      "StarWriter 3.0",
/* 27 SOT_FORMATSTR_ID_STARWRITER_40*/      "StarWriter 4.0",
/* 28 SOT_FORMATSTR_ID_STARWRITER_50*/      "StarWriter 5.0",
/* 29 SOT_FORMATSTR_ID_STARWRITERWEB_40*/   "StarWriter/Web 4.0",
/* 30 SOT_FORMATSTR_ID_STARWRITERWEB_50*/   "StarWriter/Web 5.0",
/* 31 SOT_FORMATSTR_ID_STARWRITERGLOB_40*/  "StarWriter/Global 4.0",
/* 32 SOT_FORMATSTR_ID_STARWRITERGLOB_50*/  "StarWriter/Global 5.0",
/* 33 SOT_FORMATSTR_ID_STARDRAW*/           "StarDrawDocument",
/* 34 SOT_FORMATSTR_ID_STARDRAW_40*/        "StarDrawDocument 4.0",
/* 35 SOT_FORMATSTR_ID_STARIMPRESS_50*/     "StarImpress 5.0",
/* 36 SOT_FORMATSTR_ID_STARDRAW_50*/        "StarDraw 5.0",
/* 37 SOT_FORMATSTR_ID_STARCALC*/           "StarCalcDocument",
/* 38 SOT_FORMATSTR_ID_STARCALC_40*/        "StarCalc 4.0",
/* 39 SOT_FORMATSTR_ID_STARCALC_50*/        "StarCalc 5.0",
/* 40 SOT_FORMATSTR_ID_STARCHART*/          "StarChartDocument",
/* 41 SOT_FORMATSTR_ID_STARCHART_40*/       "StarChartDocument 4.0",
/* 42 SOT_FORMATSTR_ID_STARCHART_50*/       "StarChart 5.0",
/* 43 SOT_FORMATSTR_ID_STARIMAGE*/          "StarImageDocument",
/* 44 SOT_FORMATSTR_ID_STARIMAGE_40*/       "StarImageDocument 4.0",
/* 45 SOT_FORMATSTR_ID_STARIMAGE_50*/       "StarImage 5.0",
/* 46 SOT_FORMATSTR_ID_STARMATH*/           "StarMath",
/* 47 SOT_FORMATSTR_ID_STARMATH_40*/        "StarMathDocument 4.0",
/* 48 SOT_FORMATSTR_ID_STARMATH_50*/        "StarMath 5.0",
/* 49 SOT_FORMATSTR_ID_STAROBJECT_PAINTDOC*/    "StarObjectPaintDocument",
/* 50 SOT_FORMATSTR_ID_FILLED_AREA*/        "FilledArea",
/* 51 SOT_FORMATSTR_ID_HTML*/               "HTML (HyperText Markup Language)",
/* 52 SOT_FORMATSTR_ID_HTML_SIMPLE*/        "HTML Format",
/* 53 SOT_FORMATSTR_ID_CHAOS*/              "FORMAT_CHAOS",
/* 54 SOT_FORMATSTR_ID_CNT_MSGATTACHFILE*/  "CNT_MSGATTACHFILE_FORMAT",
/* 55 SOT_FORMATSTR_ID_BIFF_5*/             "Biff5",
/* 56 SOT_FORMATSTR_ID_BIFF__5*/            "Biff 5",
/* 57 SOT_FORMATSTR_ID_SYLK*/               "Sylk",
/* 58 SOT_FORMATSTR_ID_SYLK_BIGCAPS*/       "SYLK",
/* 59 SOT_FORMATSTR_ID_LINK*/               "Link",
/* 60 SOT_FORMATSTR_ID_DIF*/                "DIF",
/* 61 SOT_FORMATSTR_ID_STARDRAW_TABBAR*/    "StarDraw TabBar",
/* 62 SOT_FORMATSTR_ID_SONLK*/              "SONLK (StarOffice Navi Link)",
/* 63 SOT_FORMATSTR_ID_MSWORD_DOC*/         "MSWordDoc",
/* 64 SOT_FORMATSTR_ID_STAR_FRAMESET_DOC*/  "StarFrameSetDocument",
/* 65 SOT_FORMATSTR_ID_OFFICE_DOC*/         "OfficeDocument",
/* 66 SOT_FORMATSTR_ID_NOTES_DOCINFO*/      "NotesDocInfo",
/* 67 SOT_FORMATSTR_ID_NOTES_HNOTE*/        "NoteshNote",
/* 68 SOT_FORMATSTR_ID_NOTES_NATIVE*/       "Native",
/* 69 SOT_FORMATSTR_ID_SFX_DOC*/            "SfxDocument",
/* 70 SOT_FORMATSTR_ID_EVDF*/               "EVDF (Explorer View Dummy Format)",
/* 71 SOT_FORMATSTR_ID_ESDF*/               "ESDF (Explorer Search Dummy Format)",
/* 72 SOT_FORMATSTR_ID_IDF*/                "IDF (Iconview Dummy Format)",
/* 73 SOT_FORMATSTR_ID_EFTP*/               "EFTP (Explorer Ftp File)",
/* 74 SOT_FORMATSTR_ID_EFD*/                "EFD (Explorer Ftp Dir)",
/* 75 SOT_FORMATSTR_ID_SVX_FORMFIELDEXCH*/  "SvxFormFieldExch",
/* 76 SOT_FORMATSTR_ID_EXTENDED_TABBAR*/    "ExtendedTabBar",
/* 77 SOT_FORMATSTR_ID_SBA_DATAEXCHANGE*/   "SBA-DATAFORMAT",
/* 78 SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE*/  "SBA-FIELDFORMAT",
/* 79 SOT_FORMATSTR_ID_SBA_PRIVATE_URL*/    "SBA-PRIVATEURLFORMAT",
/* 80 SOT_FORMATSTR_ID_SBA_TABED*/          "Tabed",
/* 81 SOT_FORMATSTR_ID_SBA_TABID*/          "Tabid",
/* 82 SOT_FORMATSTR_ID_SBA_JOIN*/           "SBA-JOINFORMAT",

#ifdef MAC
/* 83 SOT_FORMATSTR_ID_OBJECTDESCRIPTOR*/   "Star OBJD",
/* 84 SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR*/  "Star LKSD",
/* 85 SOT_FORMATSTR_ID_EMBED_SOURCE*/       "Star EMBS",
/* 86 SOT_FORMATSTR_ID_LINK_SOURCE*/        "Star LNKS",
/* 87 SOT_FORMATSTR_ID_EMBEDDED_OBJ*/       "Star EMBO",
#else
/* 83 SOT_FORMATSTR_ID_OBJECTDESCRIPTOR*/   "Star Object Descriptor",
/* 84 SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR*/  "Star Link Source Descriptor",
/* 85 SOT_FORMATSTR_ID_EMBED_SOURCE*/       "Star Embed Source",
/* 86 SOT_FORMATSTR_ID_LINK_SOURCE*/        "Star Link Source",
/* 87 SOT_FORMATSTR_ID_EMBEDDED_OBJ*/       "Star Embedded Object",
#endif

#ifdef WNT
/* 88 SOT_FORMATSTR_ID_FILECONTENT*/        CFSTR_FILECONTENTS,
/* 89 SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR*/  CFSTR_FILEDESCRIPTOR,
/* 90 SOT_FORMATSTR_ID_FILENAME*/           CFSTR_FILENAME,
#else
/* 88 SOT_FORMATSTR_ID_FILECONTENT*/        "FileContents",
/* 89 SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR*/  "FileGroupDescriptor",
/* 90 SOT_FORMATSTR_ID_FILENAME*/           "FileName",
#endif

/* 91 SOT_FORMATSTR_ID_SD_OLE*/             "SD-OLE",
/* 92 SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE*/   "Embedded Object",
/* 93 SOT_FORMATSTR_ID_EMBED_SOURCE_OLE*/   "Embed Source",
/* 94 SOT_FORMATSTR_ID_OBJECTDESCRIPTOR_OLE*/   "Object Descriptor",
/* 95 SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR_OLE*/  "Link Source Descriptor",
/* 96 SOT_FORMATSTR_ID_LINK_SOURCE_OLE*/    "Link Source",

/* 97 SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE*/   "SBA-CTRLFORMAT",
/* 98 SOT_FORMATSTR_ID_OUTPLACE_OBJ*/       "OutPlace Object",
/* 99 SOT_FORMATSTR_ID_CNT_OWN_CLIP*/       "CntOwnClipboard",
/*100 SOT_FORMATSTR_ID_INET_IMAGE*/         "SO-INet-Image",
/*101 SOT_FORMATSTR_ID_NETSCAPE_IMAGE*/     "Netscape Image Format",
/*102 SOT_FORMATSTR_ID_SBA_FORMEXCHANGE*/   "SBA_FORMEXCHANGE",
/*103 SOT_FORMATSTR_ID_SBA_REPORTEXCHANGE*/ "SBA_REPORTEXCHANGE",
/*104 SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR*/ "UniformResourceLocator",

/*105 SOT_FORMATSTR_ID_STARCHARTDOCUMENT_50*/"StarChartDocument 5.0",
/*106 SOT_FORMATSTR_ID_GRAPHOBJ*/           "Graphic Object",
/*107 SOT_FORMATSTR_ID_DUMMY3*/             "SO_DUMMYFORMAT_3",
/*108 SOT_FORMATSTR_ID_DUMMY4*/             "SO_DUMMYFORMAT_4"
};


//-----------------------------------------------------------------------
//-----------------------------------------------------------------------
static StringList & Init_Impl()
{
    SotData_Impl * pSotData = SOTDATA();
    if( !pSotData->pAtomList )
        StringList * pSL = pSotData->pAtomList = new StringList();
    return *pSotData->pAtomList;
}

/*************************************************************************
|*
|*    SotExchange::RegisterFormatName()
|*
|*    Beschreibung      CLIP.SDW
*************************************************************************/
ULONG SotExchange::RegisterFormatName( const String& rName )
{
    // teste zuerst die Standard - Name
    ULONG i, nMax = SOT_FORMAT_FILE_LIST;
    for( i = SOT_FORMAT_STRING; i <= nMax;  ++i )
        if( COMPARE_EQUAL == rName.CompareToAscii( *(aFormatArray_Impl + i ) ) )
            return i;

    nMax = SOT_FORMATSTR_ID_USER_END;
    for( i = SOT_FORMAT_RTF; i <= nMax;  ++i )
        if( COMPARE_EQUAL == rName.CompareToAscii( *(aFormatArray_Impl + i ) ) )
            return i;

    // dann in der dynamischen Liste
    StringList & rSL = Init_Impl();
    nMax = rSL.Count();
    for( i = 0; i < nMax; i++ )
    {
        String * pStr = rSL.GetObject( i );
        if( pStr && *pStr == rName )
            return i + SOT_FORMATSTR_ID_USER_END + 1;
    }
    // nMax ist der neue Platz
    rSL.Insert( new String( rName ), LIST_APPEND );
    return nMax + SOT_FORMATSTR_ID_USER_END + 1;
}

/*************************************************************************
|*
|*    SotExchange::GetFormatName()
|*
|*    Beschreibung      CLIP.SDW
*************************************************************************/
String SotExchange::GetFormatName( ULONG nFormat )
{
    String sRet;
    if( SOT_FORMATSTR_ID_USER_END >= nFormat )
        sRet.AppendAscii( *( aFormatArray_Impl + nFormat ) );
    else
    {
        nFormat -= SOT_FORMATSTR_ID_USER_END + 1;
        StringList & rSL = Init_Impl();
        String* pStr;
        if( rSL.Count() > nFormat && (pStr = rSL.GetObject( nFormat )) )
            sRet = *pStr;
    }
    return sRet;
}

/*************************************************************************
|*
|*    SotExchange::GetMaxFormat()
|*
*************************************************************************/
ULONG SotExchange::GetMaxFormat( void )
{
    StringList & rSL = Init_Impl();
    return SOT_FORMATSTR_ID_USER_END + rSL.Count();
}


