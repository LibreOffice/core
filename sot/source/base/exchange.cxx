/*************************************************************************
 *
 *  $RCSfile: exchange.cxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: ka $ $Date: 2001-03-06 17:54:11 $
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

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif
#ifndef _COM_SUN_STAR_DATATRANSFER_DATAFLAVOR_HPP_
#include <com/sun/star/datatransfer/DataFlavor.hpp>
#endif

using namespace::com::sun::star::uno;
using namespace::com::sun::star::datatransfer;

/*
    In diesen Tabellen stehen alle im Office verwendeten MimeTypes,
    Format-Bezeichner und Types.
    Die Tabelle ist nach den Formatstring-Ids sortiert und jede Id
    ist um genau 1 groesser als ihre Vorgaenger-Id, damit die Id als
    Tabellenindex benutzt werden kann.
*/
struct DataFlavorRepresentation
{
    const char*                         pMimeType;
    const char*                         pName;
    const ::com::sun::star::uno::Type*  pType;
};

// -----------------------------------------------------------------------------

static const DataFlavorRepresentation aFormatArray_Impl[] =
{
/*  0 SOT_FORMAT_SYSTEM_START*/                 { "", "", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*  1 SOT_FORMAT_STRING*/                       { "text/plain;charset=utf-16", "Text", &::getCppuType( (const ::rtl::OUString*) 0 ) },
/*  2 SOT_FORMAT_BITMAP*/                       { "image/x-cf-dib", "Bitmap", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*  3 SOT_FORMAT_GDIMETAFILE*/                  { "GDIMetaFile", "GDIMetaFile", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*  4 SOT_FORMAT_PRIVATE*/                      { "Private", "Private", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*  5 SOT_FORMAT_FILE*/                         { "FileName", "FileName", &::getCppuType( (const ::rtl::OUString*) 0 ) },
/*  6 SOT_FORMAT_FILE_LIST*/                    { "FileList", "FileList", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*  7 EMPTY*/                                   { "", "", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*  8 EMPTY*/                                   { "", "", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*  9 EMPTY*/                                   { "", "", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 10 SOT_FORMAT_RTF*/                          { "text/richtext", "Rich Text Format", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 11 SOT_FORMATSTR_ID_DRAWING*/                { "Drawing Format", "Drawing Format", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 12 SOT_FORMATSTR_ID_SVXB*/                   { "SVXB (StarView Bitmap/Animation)", "SVXB (StarView Bitmap/Animation)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 13 SOT_FORMATSTR_ID_SVIM*/                   { "SVIM (StarView ImageMap)", "SVIM (StarView ImageMap)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 14 SOT_FORMATSTR_ID_XFA*/                    { "XFA (XOutDev FillAttr)", "XFA (XOutDev FillAttr)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 15 SOT_FORMATSTR_ID_EDITENGINE*/             { "EditEngineFormat", "EditEngineFormat", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 16 SOT_FORMATSTR_ID_INTERNALLINK_STATE*/     { "StatusInfo vom SvxInternalLink", "StatusInfo vom SvxInternalLink", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 17 SOT_FORMATSTR_ID_SOLK*/                   { "SOLK (StarOffice Link)", "SOLK (StarOffice Link)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 18 SOT_FORMATSTR_ID_NETSCAPE_BOOKMARK*/      { "Netscape Bookmark", "Netscape Bookmark", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 19 SOT_FORMATSTR_ID_TREELISTBOX*/            { "SV_LBOX_DD_FORMAT", "SV_LBOX_DD_FORMAT", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 20 SOT_FORMATSTR_ID_NATIVE*/                 { "Native", "Native", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 21 SOT_FORMATSTR_ID_OWNERLINK*/              { "OwnerLink", "OwnerLink", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 22 SOT_FORMATSTR_ID_STARSERVER*/             { "StarServerFormat", "StarServerFormat", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 23 SOT_FORMATSTR_ID_STAROBJECT*/             { "StarObjectFormat", "StarObjectFormat", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 24 SOT_FORMATSTR_ID_APPLETOBJECT*/           { "Applet Object", "Applet Object", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 25 SOT_FORMATSTR_ID_PLUGIN_OBJECT*/          { "PlugIn Object", "PlugIn Object", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 26 SOT_FORMATSTR_ID_STARWRITER_30*/          { "StarWriter 3.0", "StarWriter 3.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 27 SOT_FORMATSTR_ID_STARWRITER_40*/          { "StarWriter 4.0", "StarWriter 4.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 28 SOT_FORMATSTR_ID_STARWRITER_50*/          { "StarWriter 5.0", "StarWriter 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 29 SOT_FORMATSTR_ID_STARWRITERWEB_40*/       { "StarWriter/Web 4.0", "StarWriter/Web 4.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 30 SOT_FORMATSTR_ID_STARWRITERWEB_50*/       { "StarWriter/Web 5.0", "StarWriter/Web 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 31 SOT_FORMATSTR_ID_STARWRITERGLOB_40*/      { "StarWriter/Global 4.0", "StarWriter/Global 4.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 32 SOT_FORMATSTR_ID_STARWRITERGLOB_50*/      { "StarWriter/Global 5.0", "StarWriter/Global 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 33 SOT_FORMATSTR_ID_STARDRAW*/               { "StarDrawDocument", "StarDrawDocument", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 34 SOT_FORMATSTR_ID_STARDRAW_40*/            { "StarDrawDocument 4.0", "StarDrawDocument 4.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 35 SOT_FORMATSTR_ID_STARIMPRESS_50*/         { "StarImpress 5.0", "StarImpress 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 36 SOT_FORMATSTR_ID_STARDRAW_50*/            { "StarDraw 5.0", "StarDraw 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 37 SOT_FORMATSTR_ID_STARCALC*/               { "StarCalcDocument", "StarCalcDocument", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 38 SOT_FORMATSTR_ID_STARCALC_40*/            { "StarCalc 4.0", "StarCalc 4.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 39 SOT_FORMATSTR_ID_STARCALC_50*/            { "StarCalc 5.0", "StarCalc 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 40 SOT_FORMATSTR_ID_STARCHART*/              { "StarChartDocument", "StarChartDocument", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 41 SOT_FORMATSTR_ID_STARCHART_40*/           { "StarChartDocument 4.0", "StarChartDocument 4.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 42 SOT_FORMATSTR_ID_STARCHART_50*/           { "StarChart 5.0", "StarChart 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 43 SOT_FORMATSTR_ID_STARIMAGE*/              { "StarImageDocument", "StarImageDocument", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 44 SOT_FORMATSTR_ID_STARIMAGE_40*/           { "StarImageDocument 4.0", "StarImageDocument 4.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 45 SOT_FORMATSTR_ID_STARIMAGE_50*/           { "StarImage 5.0", "StarImage 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 46 SOT_FORMATSTR_ID_STARMATH*/               { "StarMath", "StarMath", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 47 SOT_FORMATSTR_ID_STARMATH_40*/            { "StarMathDocument 4.0", "StarMathDocument 4.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 48 SOT_FORMATSTR_ID_STARMATH_50*/            { "StarMath 5.0", "StarMath 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 49 SOT_FORMATSTR_ID_STAROBJECT_PAINTDOC*/    { "StarObjectPaintDocument", "StarObjectPaintDocument", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 50 SOT_FORMATSTR_ID_FILLED_AREA*/            { "FilledArea", "FilledArea", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 51 SOT_FORMATSTR_ID_HTML*/                   { "HTML (HyperText Markup Language)", "HTML (HyperText Markup Language)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 52 SOT_FORMATSTR_ID_HTML_SIMPLE*/            { "text/html", "HTML Format", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 53 SOT_FORMATSTR_ID_CHAOS*/                  { "FORMAT_CHAOS", "FORMAT_CHAOS", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 54 SOT_FORMATSTR_ID_CNT_MSGATTACHFILE*/      { "CNT_MSGATTACHFILE_FORMAT", "CNT_MSGATTACHFILE_FORMAT", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 55 SOT_FORMATSTR_ID_BIFF_5*/                 { "Biff5", "Biff5", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 56 SOT_FORMATSTR_ID_BIFF__5*/                { "Biff 5", "Biff 5", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 57 SOT_FORMATSTR_ID_SYLK*/                   { "Sylk", "Sylk", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 58 SOT_FORMATSTR_ID_SYLK_BIGCAPS*/           { "SYLK", "SYLK", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 59 SOT_FORMATSTR_ID_LINK*/                   { "Link", "Link", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 60 SOT_FORMATSTR_ID_DIF*/                    { "DIF", "DIF", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 61 SOT_FORMATSTR_ID_STARDRAW_TABBAR*/        { "StarDraw TabBar", "StarDraw TabBar", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 62 SOT_FORMATSTR_ID_SONLK*/                  { "SONLK (StarOffice Navi Link)", "SONLK (StarOffice Navi Link)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 63 SOT_FORMATSTR_ID_MSWORD_DOC*/             { "MSWordDoc", "MSWordDoc", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 64 SOT_FORMATSTR_ID_STAR_FRAMESET_DOC*/      { "StarFrameSetDocument", "StarFrameSetDocument", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 65 SOT_FORMATSTR_ID_OFFICE_DOC*/             { "OfficeDocument", "OfficeDocument", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 66 SOT_FORMATSTR_ID_NOTES_DOCINFO*/          { "NotesDocInfo", "NotesDocInfo", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 67 SOT_FORMATSTR_ID_NOTES_HNOTE*/            { "NoteshNote", "NoteshNote", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 68 SOT_FORMATSTR_ID_NOTES_NATIVE*/           { "Native", "Native", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 69 SOT_FORMATSTR_ID_SFX_DOC*/                { "SfxDocument", "SfxDocument", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 70 SOT_FORMATSTR_ID_EVDF*/                   { "EVDF (Explorer View Dummy Format)", "EVDF (Explorer View Dummy Format)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 71 SOT_FORMATSTR_ID_ESDF*/                   { "ESDF (Explorer Search Dummy Format)", "ESDF (Explorer Search Dummy Format)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 72 SOT_FORMATSTR_ID_IDF*/                    { "IDF (Iconview Dummy Format)", "IDF (Iconview Dummy Format)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 73 SOT_FORMATSTR_ID_EFTP*/                   { "EFTP (Explorer Ftp File)", "EFTP (Explorer Ftp File)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 74 SOT_FORMATSTR_ID_EFD*/                    { "EFD (Explorer Ftp Dir)", "EFD (Explorer Ftp Dir)", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 75 SOT_FORMATSTR_ID_SVX_FORMFIELDEXCH*/      { "SvxFormFieldExch", "SvxFormFieldExch", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 76 SOT_FORMATSTR_ID_EXTENDED_TABBAR*/        { "ExtendedTabBar", "ExtendedTabBar", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 77 SOT_FORMATSTR_ID_SBA_DATAEXCHANGE*/       { "SBA-DATAFORMAT", "SBA-DATAFORMAT", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 78 SOT_FORMATSTR_ID_SBA_FIELDDATAEXCHANGE*/  { "SBA-FIELDFORMAT", "SBA-FIELDFORMAT", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 79 SOT_FORMATSTR_ID_SBA_PRIVATE_URL*/        { "SBA-PRIVATEURLFORMAT", "SBA-PRIVATEURLFORMAT", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 80 SOT_FORMATSTR_ID_SBA_TABED*/              { "Tabed", "Tabed", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 81 SOT_FORMATSTR_ID_SBA_TABID*/              { "Tabid", "Tabid", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 82 SOT_FORMATSTR_ID_SBA_JOIN*/               { "SBA-JOINFORMAT", "SBA-JOINFORMAT", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
#ifdef MAC
/* 83 SOT_FORMATSTR_ID_OBJECTDESCRIPTOR*/       { "Star OBJD", "Star OBJD", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 84 SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR*/      { "Star LKSD", "Star LKSD", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 85 SOT_FORMATSTR_ID_EMBED_SOURCE*/           { "Star EMBS", "Star EMBS", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 86 SOT_FORMATSTR_ID_LINK_SOURCE*/            { "Star LNKS", "Star LNKS", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 87 SOT_FORMATSTR_ID_EMBEDDED_OBJ*/           { "Star EMBO", "Star EMBO", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
#else                                   ;;
/* 83 SOT_FORMATSTR_ID_OBJECTDESCRIPTOR*/       { "Star Object Descriptor", "Star Object Descriptor", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 84 SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR*/      { "Star Link Source Descriptor", "Star Link Source Descriptor", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 85 SOT_FORMATSTR_ID_EMBED_SOURCE*/           { "Star Embed Source", "Star Embed Source", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 86 SOT_FORMATSTR_ID_LINK_SOURCE*/            { "Star Link Source", "Star Link Source", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 87 SOT_FORMATSTR_ID_EMBEDDED_OBJ*/           { "Star Embedded Object", "Star Embedded Object", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
#endif
#ifdef WNT
/* 88 SOT_FORMATSTR_ID_FILECONTENT*/            { CFSTR_FILECONTENTS, CFSTR_FILECONTENTS, &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 89 SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR*/      { CFSTR_FILEDESCRIPTOR, CFSTR_FILEDESCRIPTOR, &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 90 SOT_FORMATSTR_ID_FILENAME*/               { CFSTR_FILENAME, CFSTR_FILENAME, &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
#else
/* 88 SOT_FORMATSTR_ID_FILECONTENT*/            { "FileContents", "FileContents", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 89 SOT_FORMATSTR_ID_FILEGRPDESCRIPTOR*/      { "FileGroupDescriptor", "FileGroupDescriptor", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 90 SOT_FORMATSTR_ID_FILENAME*/               { "FileName", "FileName", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
#endif
/* 91 SOT_FORMATSTR_ID_SD_OLE*/                 { "SD-OLE", "SD-OLE", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 92 SOT_FORMATSTR_ID_EMBEDDED_OBJ_OLE*/       { "Embedded Object", "Embedded Object", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 93 SOT_FORMATSTR_ID_EMBED_SOURCE_OLE*/       { "Embed Source", "Embed Source", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 94 SOT_FORMATSTR_ID_OBJECTDESCRIPTOR_OLE*/   { "Object Descriptor", "Object Descriptor", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 95 SOT_FORMATSTR_ID_LINKSRCDESCRIPTOR_OLE*/  { "Link Source Descriptor", "Link Source Descriptor", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 96 SOT_FORMATSTR_ID_LINK_SOURCE_OLE*/        { "Link Source", "Link Source", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 97 SOT_FORMATSTR_ID_SBA_CTRLDATAEXCHANGE*/   { "SBA-CTRLFORMAT", "SBA-CTRLFORMAT", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 98 SOT_FORMATSTR_ID_OUTPLACE_OBJ*/           { "OutPlace Object", "OutPlace Object", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/* 99 SOT_FORMATSTR_ID_CNT_OWN_CLIP*/           { "CntOwnClipboard", "CntOwnClipboard", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*100 SOT_FORMATSTR_ID_INET_IMAGE*/             { "SO-INet-Image", "SO-INet-Image", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*101 SOT_FORMATSTR_ID_NETSCAPE_IMAGE*/         { "Netscape Image Format", "Netscape Image Format", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*102 SOT_FORMATSTR_ID_SBA_FORMEXCHANGE*/       { "SBA_FORMEXCHANGE", "SBA_FORMEXCHANGE", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*103 SOT_FORMATSTR_ID_SBA_REPORTEXCHANGE*/     { "SBA_REPORTEXCHANGE", "SBA_REPORTEXCHANGE", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*104 SOT_FORMATSTR_ID_UNIFORMRESOURCELOCATOR*/ { "UniformResourceLocator", "UniformResourceLocator", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*105 SOT_FORMATSTR_ID_STARCHARTDOCUMENT_50*/   { "StarChartDocument 5.0", "StarChartDocument 5.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*106 SOT_FORMATSTR_ID_GRAPHOBJ*/               { "Graphic Object", "Graphic Object", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*107 SOT_FORMATSTR_ID_STARWRITER_60*/          { "application/vnd.sun.xml.writer", "Writer 6.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*108 SOT_FORMATSTR_ID_STARWRITERWEB_60*/       { "application/vnd.sun.xml.writer.web", "Writer/Web 6.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*109 SOT_FORMATSTR_ID_STARWRITERGLOB_60*/      { "application/vnd.sun.xml.writer.global", "Writer/Global 6.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*110 SOT_FORMATSTR_ID_STARWDRAW_60*/           { "application/vnd.sun.xml.draw", "Draw 6.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*111 SOT_FORMATSTR_ID_STARIMPRESS_60*/         { "application/vnd.sun.xml.impress", "Impress 6.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*112 SOT_FORMATSTR_ID_STARCALC_60*/            { "application/vnd.sun.xml.calc", "Calc 6.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*113 SOT_FORMATSTR_ID_STARCHART_60*/           { "application/vnd.sun.xml.chart", "Chart 6.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*114 SOT_FORMATSTR_ID_STARMATH_60*/            { "application/vnd.sun.xml.math", "Math 6.0", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*115 SOT_FORMATSTR_ID_DUMMY3*/                 { "SO_DUMMYFORMAT_3", "SO_DUMMYFORMAT_3", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) },
/*116 SOT_FORMATSTR_ID_DUMMY4*/                 { "SO_DUMMYFORMAT_4", "SO_DUMMYFORMAT_4", &::getCppuType( (const Sequence< sal_Int8 >*) 0 ) }
};

//-----------------------------------------------------------------------

static List& InitFormats_Impl()
{
    SotData_Impl * pSotData = SOTDATA();
    if( !pSotData->pDataFlavorList )
        List* pL = pSotData->pDataFlavorList = new List();
    return *pSotData->pDataFlavorList;
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
        if( COMPARE_EQUAL == rName.CompareToAscii( aFormatArray_Impl[ i ].pName ) )
            return i;

    nMax = SOT_FORMATSTR_ID_USER_END;
    for( i = SOT_FORMAT_RTF; i <= nMax;  ++i )
        if( COMPARE_EQUAL == rName.CompareToAscii( aFormatArray_Impl[ i ].pName ) )
            return i;

    // dann in der dynamischen Liste
    List& rL = InitFormats_Impl();
    for( i = 0, nMax = rL.Count(); i < nMax; i++ )
    {
        DataFlavor* pFlavor = (DataFlavor*) rL.GetObject( i );
        if( pFlavor && rName == String( pFlavor->HumanPresentableName ) )
            return i + SOT_FORMATSTR_ID_USER_END + 1;
    }

    // nMax ist der neue Platz
    DataFlavor* pNewFlavor = new DataFlavor;

    pNewFlavor->MimeType = rName;
    pNewFlavor->HumanPresentableName = rName;
    pNewFlavor->DataType = ::getCppuType( (const ::rtl::OUString*) 0 );

    rL.Insert( pNewFlavor, LIST_APPEND );

    return nMax + SOT_FORMATSTR_ID_USER_END + 1;
}

/*************************************************************************
|*
|*    SotExchange::RegisterFormatName()
|*
|*    Beschreibung      CLIP.SDW
*************************************************************************/
ULONG SotExchange::RegisterFormat( const DataFlavor& rFlavor )
{
    ULONG nRet = GetFormat( rFlavor );

    if( !nRet )
    {
        List& rL = InitFormats_Impl();
        nRet = rL.Count() + SOT_FORMATSTR_ID_USER_END + 1;
        rL.Insert( new DataFlavor( rFlavor ), LIST_APPEND );
    }

    return nRet;
}

/*************************************************************************
|*
|*    SotExchange::GetFormatDataFlavor()
|*
*************************************************************************/

sal_Bool SotExchange::GetFormatDataFlavor( ULONG nFormat, DataFlavor& rFlavor )
{
    sal_Bool bRet;

    if( SOT_FORMATSTR_ID_USER_END >= nFormat )
    {
        const DataFlavorRepresentation& rData = *( aFormatArray_Impl + nFormat );

        rFlavor.MimeType = ::rtl::OUString::createFromAscii( rData.pMimeType );
        rFlavor.HumanPresentableName = ::rtl::OUString::createFromAscii( rData.pName );
        rFlavor.DataType = *rData.pType;

        bRet = sal_True;
    }
    else
    {
        List& rL = InitFormats_Impl();

        nFormat -= SOT_FORMATSTR_ID_USER_END + 1;

        if( rL.Count() > nFormat )
        {
            rFlavor = *(DataFlavor*) rL.GetObject( nFormat );
            bRet = sal_True;
        }
        else
        {
            rFlavor = DataFlavor();
            bRet = sal_False;
        }
    }

    DBG_ASSERT( bRet, "SotExchange::GetFormatDataFlavor(): DataFlavor not initialized" );

    return bRet;
}

/*************************************************************************
|*
|*    SotExchange::GetFormatName()
|*
|*    Beschreibung      CLIP.SDW
*************************************************************************/
ULONG SotExchange::GetFormat( const DataFlavor& rFlavor )
{
    // teste zuerst die Standard - Name
    const String    aMimeType( rFlavor.MimeType );
    ULONG           i, nMax = SOT_FORMAT_FILE_LIST;
    for( i = SOT_FORMAT_STRING; i <= nMax;  ++i )
        if( COMPARE_EQUAL == aMimeType.CompareToAscii( aFormatArray_Impl[ i ].pMimeType ) )
            return i;

    nMax = SOT_FORMATSTR_ID_USER_END;
    for( i = SOT_FORMAT_RTF; i <= nMax;  ++i )
        if( COMPARE_EQUAL == aMimeType.CompareToAscii( aFormatArray_Impl[ i ].pMimeType ) )
            return i;

    // dann in der dynamischen Liste
    List& rL = InitFormats_Impl();
    for( i = 0, nMax = rL.Count(); i < nMax; i++ )
    {
        DataFlavor* pFlavor = (DataFlavor*) rL.GetObject( i );
        if( pFlavor && rFlavor.MimeType == pFlavor->MimeType )
            return i + SOT_FORMATSTR_ID_USER_END + 1;
    }

    return 0;
}


/*************************************************************************
|*
|*    SotExchange::GetFormatName()
|*
|*    Beschreibung      CLIP.SDW
*************************************************************************/
String SotExchange::GetFormatName( ULONG nFormat )
{
    DataFlavor  aFlavor;
    String      aRet;

    if( GetFormatDataFlavor( nFormat, aFlavor ) )
        aRet = aFlavor.HumanPresentableName;

    return aRet;
}

/*************************************************************************
|*
|*    SotExchange::GetMaxFormat()
|*
*************************************************************************/
ULONG SotExchange::GetMaxFormat( void )
{
    return( SOT_FORMATSTR_ID_USER_END + InitFormats_Impl().Count() );
}
