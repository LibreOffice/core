/*************************************************************************
 *
 *  $RCSfile: frmload.cxx,v $
 *
 *  $Revision: 1.14 $
 *
 *  last change: $Author: as $ $Date: 2000-11-28 14:37:37 $
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

#include "frmload.hxx"

#ifndef _COM_SUN_STAR_LANG_XMULTISERVICEFACTORY_HPP_
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_FRAME_XFRAME_HPP_
#include <com/sun/star/frame/XFrame.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_LANG_XUNOTUNNEL_HPP_
#include <com/sun/star/lang/XUnoTunnel.hpp>
#endif

#ifndef _TOOLKIT_UNOHLP_HXX
#include <toolkit/helper/vclunohelper.hxx>
#endif

#include <rtl/ustring.h>
#include <svtools/itemset.hxx>
#include <vcl/window.hxx>
#include <svtools/eitem.hxx>
#include <svtools/stritem.hxx>
#include <tools/urlobj.hxx>
#include <vos/mutex.hxx>
#include <svtools/sfxecode.hxx>
#include <svtools/ehdl.hxx>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::rtl;

#include "app.hxx"
#include "request.hxx"
#include "sfxsids.hrc"
#include "dispatch.hxx"
#include "sfxuno.hxx"
#include "viewfrm.hxx"
#include "topfrm.hxx"
#include "frame.hxx"
#include "docfac.hxx"
#include "fcontnr.hxx"
#include "loadenv.hxx"
#include "docfile.hxx"
#include "docfilt.hxx"

#ifdef TF_FILTER//MUSTFILTER
//*****************************************************************************************************************
// Fill hash to convert old to new filter names.
void SfxFrameLoader::impl_initFilterHashOld2New( TFilterNames& aHash )
{
    // key = old filter name, value = new name
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: HTML"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_HTML"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter/Web 5.0 Vorlage/Template"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriterWeb_50_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter/Web 4.0 Vorlage/Template"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriterWeb_40_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text (StarWriter/Web)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_StarWriterWeb"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text DOS (StarWriter/Web)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_DOS_StarWriterWeb"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text Mac (StarWriter/Web)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_Mac_StarWriterWeb"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text Unix (StarWriter/Web)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_Unix_StarWriterWeb"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter 5.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriter_50"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter 4.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriter_40"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter 3.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriter_30"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text (encoded)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_encoded"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 5.0/GlobalDocument"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_50GlobalDocument"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 4.0/GlobalDocument"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_40GlobalDocument"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 5.0"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_50"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 4.0"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_40"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 3.0"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_30"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: Text (encoded)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_Text_encoded"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 5.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_50"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 5.0 Vorlage/Template"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_50_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 4.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_40"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 4.0 Vorlage/Template"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_40_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 3.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_30"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 3.0 Vorlage/Template"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_30_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 97"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_97"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 97 Vorlage/Template"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_97_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 95"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_95"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 95 Vorlage/Template"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_95_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 5.0/95"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_5095"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 5.0/95 Vorlage/Template"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_5095_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 4.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_40"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 4.0 Vorlage/Template"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_40_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: Rich Text Format (StarCalc)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_Rich_Text_Format_StarCalc"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: SYLK"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_SYLK"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: DIF"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_DIF"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: HTML (StarCalc)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_HTML_StarCalc"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: dBase"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_dBase"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: Lotus"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_Lotus"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 1.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_10"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: Text - txt - csv (StarCalc)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_Text__ttxt__ccsv_StarCalc"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarOffice XML (Calc)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarOffice_XML_Calc"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 5.0"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_50"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 5.0 Vorlage"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_50_Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 4.0"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_40"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 4.0 Vorlage"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_40_Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 5.0 (StarImpress)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_50_StarImpress"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 5.0 Vorlage (StarImpress)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_50_Vorlage_StarImpress"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 3.0 (StarImpress)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_30_StarImpress"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 3.0 Vorlage (StarImpress)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_30_Vorlage_StarImpress"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: MS PowerPoint 97"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_MS_PowerPoint_97"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: MS PowerPoint 97 Vorlage"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_MS_PowerPoint_97_Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: CGM - Computer Graphics Metafile"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_CGM__CComputer_Graphics_Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 5.0 (packed)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_50_packed"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarOffice XML (Impress)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarOffice_XML_Impress"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 5.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_50"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 5.0 Vorlage"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_50_Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 5.0 (StarDraw)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_50_StarDraw"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 5.0 Vorlage (StarDraw)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_50_Vorlage_StarDraw"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: GIF - Graphics Interchange"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_GIF__GGraphics_Interchange"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 4.0 (StarDraw)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_40_StarDraw"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 4.0 Vorlage (StarDraw)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_40_Vorlage_StarDraw"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 3.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_30"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCD - Photo CD"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PCD__PPhoto_CD"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 3.0 Vorlage"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_30_Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCX - Zsoft Paintbrush"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PCX__ZZsoft_Paintbrush"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: DXF - AutoCAD Interchange"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_DXF__AAutoCAD_Interchange"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PSD - Adobe Photoshop"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PSD__AAdobe_Photoshop"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PNG - Portable Network Graphic"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PNG__PPortable_Network_Graphic"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: EPS - Encapsulated PostScript"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_EPS__EEncapsulated_PostScript"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: MET - OS/2 Metafile"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_MET__OOS2_Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PBM - Portable Bitmap"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PBM__PPortable_Bitmap"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCT - Mac Pict"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PCT__MMac_Pict"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PGM - Portable Graymap"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PGM__PPortable_Graymap"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SVM - StarView Metafile"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_SVM__SStarView_Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PPM - Portable Pixelmap"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PPM__PPortable_Pixelmap"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: EMF - MS Windows Metafile"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_EMF__MMS_Windows_Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: RAS - Sun Rasterfile"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_RAS__SSun_Rasterfile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: WMF - MS Windows Metafile"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_WMF__MMS_Windows_Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: TGA - Truevision TARGA"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_TGA__TTruevision_TARGA"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SGV - StarDraw 2.0"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_SGV__SStarDraw_20"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarOffice XML (Draw)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarOffice_XML_Draw"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: TIF - Tag Image File"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_TIF__TTag_Image_File"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SGF - StarOffice Writer SGF"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_SGF__SStarOffice_Writer_SGF"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: XPM"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_XPM"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImage 5.0"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImage_50"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImage 4.0"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImage_40"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImage 3.0"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImage_30"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: BMP - MS Windows"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_BMP__MMS_Windows"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: JPG - JPEG"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_JPG__JJPEG"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: XBM - X-Consortium"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("draw_XBM__XX_Consortium"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 5.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_50"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 4.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_40"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 3.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_30"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 2.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_20"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: MathType 3.x"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("math_MathType_3x"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("smath: MathML XML (Math)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("math_MathML_XML_Math"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarChart 5.0"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarChart_50"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarOffice XML (Chart)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarOffice_XML_Chart"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarChart 4.0"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarChart_40"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarChart 3.0"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarChart_30"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 5.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_50"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 5.0 Vorlage/Template"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_50_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 4.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_40"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 4.0 Vorlage/Template"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_40_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 3.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_30"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 3.0 Vorlage/Template"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_30_VorlageTemplate"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 2.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_20"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 1.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_10"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter DOS"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_DOS"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarOffice XML (Writer)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarOffice_XML_Writer"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: HTML (StarWriter)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_HTML_StarWriter"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text Unix"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_Unix"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text Mac"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_Mac"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text DOS"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_DOS"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Rich Text Format"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Rich_Text_Format"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 97"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_97"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 95"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_95"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 97 Vorlage"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_97_Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 95 Vorlage"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_95_Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 6.0"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_60"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 6.x (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_6x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 5"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_5"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 2.x (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_2x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 1.x (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_1x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 5.x (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_5x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 4.x (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_4x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 3.x (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_3x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS MacWord 5.x (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_MacWord_5x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS MacWord 4.0 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_MacWord_40_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS MacWord 3.0 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_MacWord_30_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect Mac 1 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Mac_1_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect Mac 2 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Mac_2_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect Mac 3 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Mac_3_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 5.1-5.2 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_51_52_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 6.0 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_60_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 6.1 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_61_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 7.0 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_70_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 4.1 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_41_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 4.2 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_42_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 5.0 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_50_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 5.1 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_51_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 6.0 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_60_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 6.1 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_61_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar (Win) 1.x-2.0 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_Win_1x_20_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 2000 Rel. 3.0 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_2000_Rel_30_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 2000 Rel. 3.5 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_2000_Rel_35_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 3.3x (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_33x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 3.45 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_345_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 4.0  (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_40_WW4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 5.0  (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_50_WW4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 5.5  (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_55_WW4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 6.0  (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_60_WW4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 7.0  (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_70_WW4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Ami Pro 1.1-1.2 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Ami_Pro_11_12_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Ami Pro 2.0-3.1 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Ami_Pro_20_31_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Excel 4.0 (StarWriter)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Excel_40_StarWriter"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Excel 5.0 (StarWriter)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Excel_50_StarWriter"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Excel 95 (StarWriter)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Excel_95_StarWriter"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Works 2.0 DOS (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Works_20_DOS_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Works 3.0 Win (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Works_30_Win_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Works 4.0 Mac (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Works_40_Mac_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Mac Write 4.x 5.0 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Mac_Write_4x_50_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Mac Write II (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Mac_Write_II_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Mac Write Pro (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Mac_Write_Pro_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Lotus 1-2-3 1.0 (DOS) (StarWriter)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Lotus_1_2_3_10_DOS_StarWriter"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Lotus 1-2-3 1.0 (WIN) (StarWriter)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Lotus_1_2_3_10_WIN_StarWriter"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Lotus Manuscript (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Lotus_Manuscript_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MASS 11 Rel. 8.0-8.3 (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MASS_11_Rel_80_83_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MASS 11 Rel. 8.5-9.0 (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MASS_11_Rel_85_90_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Claris Works (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Claris_Works_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: CTOS DEF (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_CTOS_DEF_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: OfficeWriter 4.0 (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_OfficeWriter_40_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: OfficeWriter 5.0 (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_OfficeWriter_50_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: OfficeWriter 6.x (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_OfficeWriter_6x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite III ( W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_III_WW4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite III+ ( W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_III+_WW4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite Signature (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_Signature_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite Sig. (Win) (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_Sig_Win_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite IV (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_IV_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite (Win) 1.0 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_Win_10_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 5.0 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_50_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 5.0 (Illustrator) (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_50_Illustrator_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 6.0 (Color Bitmap) (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_60_Color_Bitmap_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 6.0 (Res Graphic) (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_60_Res_Graphic_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WriteNow 3.0 (Macintosh) (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WriteNow_30_Macintosh_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Writing Assistant (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Writing_Assistant_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: VolksWriter Deluxe (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_VolksWriter_Deluxe_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: VolksWriter 3 and 4 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_VolksWriter_3_and_4_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate 3.3 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_33_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate Adv. 3.6 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_Adv_36_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate Adv. II 3.7 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_Adv_II_37_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate 4 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_4_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: NAVY DIF (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_NAVY_DIF_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS Write (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_Write_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS First Choice 1.0 (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_First_Choice_10_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS First Choice 2.0 (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_First_Choice_20_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS First Choice 3.0 (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_First_Choice_30_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Professional Write 1.0 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Professional_Write_10_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Professional Write 2.x (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Professional_Write_2x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Professional Write Plus (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Professional_Write_Plus_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Peach Text (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Peach_Text_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DCA Revisable Form Text (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DCA_Revisable_Form_Text_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DCA with Display Write 5 (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DCA_with_Display_Write_5_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DCA/FFT-Final Form Text (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DCAFFT_Final_Form_Text_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DEC DX (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DEC_DX_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DEC WPS-PLUS (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DEC_WPS_PLUS_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DisplayWrite 2.0-4.x (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DisplayWrite_20_4x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DisplayWrite 5.x (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DisplayWrite_5x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DataGeneral CEO Write (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DataGeneral_CEO_Write_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: EBCDIC (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_EBCDIC_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Enable (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Enable_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Maker MIF 3.0 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Maker_MIF_30_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Maker MIF 4.0 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Maker_MIF_40_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Maker MIF 5.0 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Maker_MIF_50_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Work III (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Work_III_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Work IV  (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Work_IV_WW4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: HP AdvanceWrite Plus (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_HP_AdvanceWrite_Plus_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: ICL Office Power 6 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_ICL_Office_Power_6_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: ICL Office Power 7 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_ICL_Office_Power_7_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Interleaf (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Interleaf_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Interleaf 5 - 6 (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Interleaf_5__66_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Legacy Winstar onGO (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Legacy_Winstar_onGO_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Q&A Write 1.0-3.0 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_QA_Write_10_30_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Q&A Write 4.0 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_QA_Write_40_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Rapid File 1.0 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Rapid_File_10_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Rapid File 1.2 (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Rapid_File_12_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Samna Word IV-IV Plus (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Samna_Word_IV_IV_Plus_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Total Word (W4W)"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Total_Word_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Uniplex onGO (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Uniplex_onGO_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Uniplex V7-V8 (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Uniplex_V7_V8_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Wang PC (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Wang_PC_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Wang II SWP (W4W)"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Wang_II_SWP_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Wang WP Plus (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Wang_WP_Plus_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Win Write 3.x (W4W)"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Win_Write_3x_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WITA (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WITA_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WiziWord 3.0 (W4W)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WiziWord_30_W4W"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text (encoded)"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_encoded"));
}

//*****************************************************************************************************************
// Fill hash to convert new to old filter names.
void SfxFrameLoader::impl_initFilterHashNew2Old( TFilterNames& aHash )
{
    // key = new filter name, value = old name
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_HTML"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: HTML"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriterWeb_50_VorlageTemplate"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter/Web 5.0 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriterWeb_40_VorlageTemplate"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter/Web 4.0 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_StarWriterWeb"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text (StarWriter/Web)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_DOS_StarWriterWeb"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text DOS (StarWriter/Web)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_Mac_StarWriterWeb"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text Mac (StarWriter/Web)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_Unix_StarWriterWeb"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text Unix (StarWriter/Web)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriter_50"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter 5.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriter_40"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter 4.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_StarWriter_30"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: StarWriter 3.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_web_Text_encoded"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/web: Text (encoded)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_50GlobalDocument"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 5.0/GlobalDocument"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_40GlobalDocument"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 4.0/GlobalDocument"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_50"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 5.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_40"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 4.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_StarWriter_30"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: StarWriter 3.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_globaldocument_Text_encoded"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter/GlobalDocument: Text (encoded)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_50"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 5.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_50_VorlageTemplate"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 5.0 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_40"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 4.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_40_VorlageTemplate"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 4.0 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_30"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 3.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_30_VorlageTemplate"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 3.0 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_97"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 97"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_97_VorlageTemplate"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 97 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_95"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 95"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_95_VorlageTemplate"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 95 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_5095"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 5.0/95"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_5095_VorlageTemplate"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 5.0/95 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_40"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 4.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_MS_Excel_40_VorlageTemplate"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: MS Excel 4.0 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_Rich_Text_Format_StarCalc"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: Rich Text Format (StarCalc)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_SYLK"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: SYLK"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_DIF"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: DIF"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_HTML_StarCalc"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: HTML (StarCalc)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_dBase"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: dBase"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_Lotus"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: Lotus"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarCalc_10"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarCalc 1.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_Text__ttxt__ccsv_StarCalc"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: Text - txt - csv (StarCalc)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("calc_StarOffice_XML_Calc"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("scalc: StarOffice XML (Calc)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_50"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 5.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_50_Vorlage"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 5.0 Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_40"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 4.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_40_Vorlage"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 4.0 Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_50_StarImpress"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 5.0 (StarImpress)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_50_Vorlage_StarImpress"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 5.0 Vorlage (StarImpress)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_30_StarImpress"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 3.0 (StarImpress)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarDraw_30_Vorlage_StarImpress"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarDraw 3.0 Vorlage (StarImpress)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_MS_PowerPoint_97"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: MS PowerPoint 97"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_MS_PowerPoint_97_Vorlage"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: MS PowerPoint 97 Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_CGM__CComputer_Graphics_Metafile"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: CGM - Computer Graphics Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarImpress_50_packed"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarImpress 5.0 (packed)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("impress_StarOffice_XML_Impress"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("simpress: StarOffice XML (Impress)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_50"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 5.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_50_Vorlage"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 5.0 Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_50_StarDraw"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 5.0 (StarDraw)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_50_Vorlage_StarDraw"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 5.0 Vorlage (StarDraw)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_GIF__GGraphics_Interchange"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: GIF - Graphics Interchange"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_40_StarDraw"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 4.0 (StarDraw)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImpress_40_Vorlage_StarDraw"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImpress 4.0 Vorlage (StarDraw)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_30"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 3.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PCD__PPhoto_CD"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCD - Photo CD"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarDraw_30_Vorlage"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarDraw 3.0 Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PCX__ZZsoft_Paintbrush"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCX - Zsoft Paintbrush"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_DXF__AAutoCAD_Interchange"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: DXF - AutoCAD Interchange"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PSD__AAdobe_Photoshop"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PSD - Adobe Photoshop"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PNG__PPortable_Network_Graphic"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PNG - Portable Network Graphic"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_EPS__EEncapsulated_PostScript"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: EPS - Encapsulated PostScript"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_MET__OOS2_Metafile"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: MET - OS/2 Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PBM__PPortable_Bitmap"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PBM - Portable Bitmap"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PCT__MMac_Pict"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PCT - Mac Pict"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PGM__PPortable_Graymap"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PGM - Portable Graymap"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_SVM__SStarView_Metafile"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SVM - StarView Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_PPM__PPortable_Pixelmap"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: PPM - Portable Pixelmap"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_EMF__MMS_Windows_Metafile"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: EMF - MS Windows Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_RAS__SSun_Rasterfile"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: RAS - Sun Rasterfile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_WMF__MMS_Windows_Metafile"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: WMF - MS Windows Metafile"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_TGA__TTruevision_TARGA"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: TGA - Truevision TARGA"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_SGV__SStarDraw_20"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SGV - StarDraw 2.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarOffice_XML_Draw"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarOffice XML (Draw)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_TIF__TTag_Image_File"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: TIF - Tag Image File"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_SGF__SStarOffice_Writer_SGF"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: SGF - StarOffice Writer SGF"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_XPM"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: XPM"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImage_50"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImage 5.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImage_40"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImage 4.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_StarImage_30"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: StarImage 3.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_BMP__MMS_Windows"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: BMP - MS Windows"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_JPG__JJPEG"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: JPG - JPEG"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("draw_XBM__XX_Consortium"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("sdraw: XBM - X-Consortium"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_50"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 5.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_40"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 4.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_30"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 3.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("math_StarMath_20"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("smath: StarMath 2.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("math_MathType_3x"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("smath: MathType 3.x"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("math_MathML_XML_Math"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("smath: MathML XML (Math)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarChart_50"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarChart 5.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarOffice_XML_Chart"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarOffice XML (Chart)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarChart_40"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarChart 4.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("chart_StarChart_30"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("schart: StarChart 3.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_50"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 5.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_50_VorlageTemplate"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 5.0 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_40"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 4.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_40_VorlageTemplate"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 4.0 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_30"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 3.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_30_VorlageTemplate"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 3.0 Vorlage/Template"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_20"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 2.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_10"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter 1.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarWriter_DOS"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarWriter DOS"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_StarOffice_XML_Writer"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: StarOffice XML (Writer)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_HTML_StarWriter"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: HTML (StarWriter)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_Unix"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text Unix"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_Mac"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text Mac"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_DOS"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text DOS"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Rich_Text_Format"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Rich Text Format"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_97"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 97"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_95"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 95"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_97_Vorlage"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 97 Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_95_Vorlage"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 95 Vorlage"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_60"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 6.0"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_6x_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 6.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_5"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 5"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_2x_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 2.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_WinWord_1x_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS WinWord 1.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_5x_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 5.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_4x_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 4.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Word_3x_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Word 3.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_MacWord_5x_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS MacWord 5.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_MacWord_40_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS MacWord 4.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_MacWord_30_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS MacWord 3.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Mac_1_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect Mac 1 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Mac_2_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect Mac 2 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Mac_3_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect Mac 3 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_51_52_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 5.1-5.2 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_60_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 6.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_61_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 6.1 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_Win_70_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect (Win) 7.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_41_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 4.1 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_42_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 4.2 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_50_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 5.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_51_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 5.1 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_60_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 6.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordPerfect_61_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordPerfect 6.1 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_Win_1x_20_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar (Win) 1.x-2.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_2000_Rel_30_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 2000 Rel. 3.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_2000_Rel_35_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 2000 Rel. 3.5 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_33x_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 3.3x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_345_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 3.45 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_40_WW4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 4.0  (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_50_WW4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 5.0  (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_55_WW4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 5.5  (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_60_WW4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 6.0  (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WordStar_70_WW4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WordStar 7.0  (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Ami_Pro_11_12_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Ami Pro 1.1-1.2 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Ami_Pro_20_31_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Ami Pro 2.0-3.1 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Excel_40_StarWriter"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Excel 4.0 (StarWriter)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Excel_50_StarWriter"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Excel 5.0 (StarWriter)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Excel_95_StarWriter"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Excel 95 (StarWriter)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Works_20_DOS_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Works 2.0 DOS (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Works_30_Win_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Works 3.0 Win (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MS_Works_40_Mac_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MS Works 4.0 Mac (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Mac_Write_4x_50_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Mac Write 4.x 5.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Mac_Write_II_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Mac Write II (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Mac_Write_Pro_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Mac Write Pro (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Lotus_1_2_3_10_DOS_StarWriter"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Lotus 1-2-3 1.0 (DOS) (StarWriter)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Lotus_1_2_3_10_WIN_StarWriter"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Lotus 1-2-3 1.0 (WIN) (StarWriter)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Lotus_Manuscript_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Lotus Manuscript (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MASS_11_Rel_80_83_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MASS 11 Rel. 8.0-8.3 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MASS_11_Rel_85_90_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MASS 11 Rel. 8.5-9.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Claris_Works_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Claris Works (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_CTOS_DEF_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: CTOS DEF (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_OfficeWriter_40_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: OfficeWriter 4.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_OfficeWriter_50_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: OfficeWriter 5.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_OfficeWriter_6x_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: OfficeWriter 6.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_III_WW4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite III ( W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_III+_WW4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite III+ ( W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_Signature_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite Signature (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_Sig_Win_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite Sig. (Win) (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_IV_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite IV (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XyWrite_Win_10_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XyWrite (Win) 1.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_50_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 5.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_50_Illustrator_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 5.0 (Illustrator) (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_60_Color_Bitmap_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 6.0 (Color Bitmap) (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_XEROX_XIF_60_Res_Graphic_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: XEROX XIF 6.0 (Res Graphic) (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WriteNow_30_Macintosh_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WriteNow 3.0 (Macintosh) (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Writing_Assistant_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Writing Assistant (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_VolksWriter_Deluxe_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: VolksWriter Deluxe (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_VolksWriter_3_and_4_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: VolksWriter 3 and 4 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_33_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate 3.3 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_Adv_36_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate Adv. 3.6 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_Adv_II_37_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate Adv. II 3.7 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_MultiMate_4_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: MultiMate 4 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_NAVY_DIF_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: NAVY DIF (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_Write_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS Write (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_First_Choice_10_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS First Choice 1.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_First_Choice_20_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS First Choice 2.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_PFS_First_Choice_30_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: PFS First Choice 3.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Professional_Write_10_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Professional Write 1.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Professional_Write_2x_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Professional Write 2.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Professional_Write_Plus_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Professional Write Plus (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Peach_Text_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Peach Text (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DCA_Revisable_Form_Text_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DCA Revisable Form Text (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DCA_with_Display_Write_5_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DCA with Display Write 5 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DCAFFT_Final_Form_Text_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DCA/FFT-Final Form Text (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DEC_DX_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DEC DX (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DEC_WPS_PLUS_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DEC WPS-PLUS (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DisplayWrite_20_4x_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DisplayWrite 2.0-4.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DisplayWrite_5x_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DisplayWrite 5.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_DataGeneral_CEO_Write_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: DataGeneral CEO Write (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_EBCDIC_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: EBCDIC (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Enable_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Enable (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Maker_MIF_30_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Maker MIF 3.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Maker_MIF_40_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Maker MIF 4.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Maker_MIF_50_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Maker MIF 5.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Work_III_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Work III (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Frame_Work_IV_WW4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Frame Work IV  (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_HP_AdvanceWrite_Plus_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: HP AdvanceWrite Plus (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_ICL_Office_Power_6_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: ICL Office Power 6 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_ICL_Office_Power_7_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: ICL Office Power 7 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Interleaf_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Interleaf (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Interleaf_5__66_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Interleaf 5 - 6 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Legacy_Winstar_onGO_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Legacy Winstar onGO (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_QA_Write_10_30_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Q&A Write 1.0-3.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_QA_Write_40_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Q&A Write 4.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Rapid_File_10_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Rapid File 1.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Rapid_File_12_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Rapid File 1.2 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Samna_Word_IV_IV_Plus_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Samna Word IV-IV Plus (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Total_Word_W4W"))]   =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Total Word (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Uniplex_onGO_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Uniplex onGO (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Uniplex_V7_V8_W4W"))]    =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Uniplex V7-V8 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Wang_PC_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Wang PC (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Wang_II_SWP_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Wang II SWP (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Wang_WP_Plus_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Wang WP Plus (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Win_Write_3x_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Win Write 3.x (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WITA_W4W"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WITA (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_WiziWord_30_W4W"))]  =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: WiziWord 3.0 (W4W)"));
    aHash[OUString(RTL_CONSTASCII_USTRINGPARAM("writer_Text_encoded"))] =   OUString(RTL_CONSTASCII_USTRINGPARAM("swriter: Text (encoded)"));
}

//*****************************************************************************************************************
::rtl::OUString SfxFrameLoader::impl_getNewFilterName( const ::rtl::OUString& sOldName )
{
    // Search for existing entry! Don't use index operato directly - he create a new entry if it not already exist automaticly!
    TConstConverterIterator pEntry = aConverterOld2New.find(sOldName);
    // Warn programmer if some filter names are not suported yet!
    DBG_ASSERT( !(pEntry==aConverterOld2New.end()), "SfxFrameLoader::impl_getNewFilterName()\nUnsupported filter name detected ... Convertion failed!\n" );
    ::rtl::OUString sNewName;
    if( pEntry!=aConverterOld2New.end() )
    {
        sNewName = aConverterOld2New[sOldName];
    }
    return sNewName;
}

//*****************************************************************************************************************
::rtl::OUString SfxFrameLoader::impl_getOldFilterName( const ::rtl::OUString& sNewName )
{
    // Search for existing entry! Don't use index operato directly - he create a new entry if it not already exist automaticly!
    TConstConverterIterator pEntry = aConverterNew2Old.find(sNewName);
    // Warn programmer if some filter names are not suported yet!
    DBG_ASSERT( !(pEntry==aConverterNew2Old.end()), "SfxFrameLoader::impl_getOldFilterName()\nUnsupported filter name detected ... Convertion failed!\n" );
    ::rtl::OUString sOldName;
    if( pEntry!=aConverterNew2Old.end() )
    {
        sOldName = aConverterNew2Old[sNewName];
    }
    return sOldName;
}

#endif//MUSTFILTER

Reference< XInterface > SAL_CALL SfxFrameLoaderFactory::createInstance(void) throw(Exception, RuntimeException)
{
    Reference < XFrameLoader > xLoader( pCreateFunction( xSMgr ), UNO_QUERY );
    SfxFrameLoader* pLoader = (SfxFrameLoader*) xLoader.get();
    pLoader->SetFilterName( aImplementationName );
    return xLoader;
}

Reference< XInterface > SAL_CALL SfxFrameLoaderFactory::createInstanceWithArguments(const Sequence<Any>& Arguments) throw(Exception, RuntimeException)
{
    return createInstance();
}

::rtl::OUString SAL_CALL SfxFrameLoaderFactory::getImplementationName() throw(RuntimeException)
{
    return aImplementationName;
}

sal_Bool SAL_CALL SfxFrameLoaderFactory::supportsService(const ::rtl::OUString& ServiceName) throw(RuntimeException)
{
    if ( ServiceName.compareToAscii("com.sun.star.frame.FrameLoader") == COMPARE_EQUAL )
        return sal_True;
    else
        return sal_False;
}

Sequence< ::rtl::OUString > SAL_CALL SfxFrameLoaderFactory::getSupportedServiceNames(void) throw(RuntimeException)
{
    Sequence< ::rtl::OUString > aRet(1);
    *aRet.getArray() = ::rtl::OUString::createFromAscii("com.sun.star.frame.FrameLoader");
    return aRet;
}

#ifndef TF_FILTER//MUSTFILTER
void SAL_CALL SfxFrameLoader::initialize( const Sequence< Any >& aArguments ) throw( Exception, RuntimeException )
{
    sal_Int32 nLen = aArguments.getLength();
    for ( sal_Int32 n=0; n<nLen; n++ )
    {
        PropertyValue aValue;
        if ( ( aArguments[n] >>= aValue ) && aValue.Name.compareToAscii("FilterName") == COMPARE_EQUAL )
        {
            ::rtl::OUString aTmp;
            aValue.Value >>= aTmp;
            aFilterName = aTmp;
        }
    }
}
#endif//MUSTFILTER

#ifdef TF_FILTER//MUSTFILTER
SfxFrameLoader::SfxFrameLoader( const REFERENCE < ::com::sun::star::lang::XMultiServiceFactory >& xFactory )
    : pMatcher( 0 )
    , pLoader( 0 )
    , bLoadDone( sal_False )
    , bLoadState( sal_False )
{
    impl_initFilterHashOld2New( aConverterOld2New );
    impl_initFilterHashNew2Old( aConverterNew2Old );
}
#else//MUSTFILTER
SfxFrameLoader::SfxFrameLoader( const REFERENCE < ::com::sun::star::lang::XMultiServiceFactory >& xFactory )
    : pMatcher( 0 )
    , pLoader( 0 )
{
}
#endif//MUSTFILTER

SfxFrameLoader::~SfxFrameLoader()
{
    if ( pLoader )
        pLoader->ReleaseRef();
    delete pMatcher;
}

#ifdef TF_FILTER//MUSTFILTER
//NEW
sal_Bool SAL_CALL SfxFrameLoader::load( const Sequence< PropertyValue >& rArgs, const Reference< XFrame >& rFrame ) throw( RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // Extract URL from given descriptor.
    String rURL;

    sal_uInt32 nPropertyCount = rArgs.getLength();
    for( sal_uInt32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        if( rArgs[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("FileName")) )
        {
            ::rtl::OUString sTemp;
            rArgs[nProperty].Value >>= sTemp;
            rURL = sTemp;
        }
        if( rArgs[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("FilterName")) )
        {
            ::rtl::OUString sTemp;
            rArgs[nProperty].Value >>= sTemp;
            // Convert new filtername to old format!
            aFilterName = impl_getOldFilterName(sTemp);
        }
    }

    xFrame = rFrame;

    // Achtung: beim Abräumen der Objekte kann die SfxApp destruiert werden, vorher noch Deinitialize_Impl rufen
    SfxApplication* pApp = SFX_APP();

    SfxAllItemSet aSet( pApp->GetPool() );
    TransformParameters( SID_OPENDOC, rArgs, aSet );

    SFX_ITEMSET_ARG( &aSet, pRefererItem, SfxStringItem, SID_REFERER, FALSE );
    if ( !pRefererItem )
        aSet.Put( SfxStringItem( SID_REFERER, String() ) );

    SfxFrame* pFrame=0;
    for ( pFrame = SfxFrame::GetFirst(); pFrame; pFrame = SfxFrame::GetNext( *pFrame ) )
    {
        if ( pFrame->GetFrameInterface() == xFrame )
            break;
    }

    if ( !pFrame )
        pFrame = SfxTopFrame::Create( rFrame );

    BOOL bFactoryURL = FALSE;
    const SfxObjectFactory* pFactory = 0;
    String aFact( rURL );
    String aPrefix = String::CreateFromAscii( "private:factory/" );
    String aParam;
    if ( aPrefix.Len() == aFact.Match( aPrefix ) )
    {
        bFactoryURL = TRUE;
        aFact.Erase( 0, aPrefix.Len() );
        USHORT nPos = aFact.Search( '?' );
        if ( nPos != STRING_NOTFOUND )
        {
            aParam = aFact.Copy( nPos, aFact.Len() );
            aFact.Erase( nPos, aFact.Len() );
            aParam.Erase(0,1);
        }
    }
    else
        aFact = aFilterName.GetToken( 0, ':' );

    WildCard aSearchedFac( aFact.EraseAllChars('4').ToUpperAscii() );
    for( USHORT n = SfxObjectFactory::GetObjectFactoryCount_Impl(); !pFactory && n--; )
    {
        pFactory = &SfxObjectFactory::GetObjectFactory_Impl( n );
        if( !aSearchedFac.Matches( String::CreateFromAscii( pFactory->GetShortName() ).ToUpperAscii() ) )
            pFactory = 0;
    }

    if ( bFactoryURL && pFactory )
    {
        INetURLObject aObj( rURL );
        if ( aParam.Len() )
        {
            sal_uInt16 nSlotId = aParam.ToInt32();
            SfxModule* pMod = pFactory->GetModule()->Load();
            SfxRequest aReq( nSlotId, SFX_CALLMODE_SYNCHRON, pMod->GetPool() );
            aReq.AppendItem( SfxStringItem ( SID_FILE_NAME, rURL ) );
            aReq.AppendItem( SfxFrameItem ( SID_DOCFRAME, pFrame ) );
            const SfxPoolItem* pRet = pMod->ExecuteSlot( aReq );
            if ( pRet )
                bLoadState = sal_True;
            else
            {
                if ( !pFrame->GetCurrentDocument() )
                {
                    pFrame->SetFrameInterface_Impl( Reference < XFrame >() );
                    pFrame->DoClose();
                }
                bLoadState = sal_False;
            }
            return bLoadState;
        }

        String aPathName( aObj.GetMainURL() );
        if( pFactory->GetStandardTemplate().Len() )
        {
            aSet.Put( SfxStringItem ( SID_FILE_NAME, pFactory->GetStandardTemplate() ) );
            aSet.Put( SfxBoolItem( SID_TEMPLATE, sal_True ) );
        }
        else
        {
            SfxViewShell *pView = pFrame->GetCurrentViewFrame() ? pFrame->GetCurrentViewFrame()->GetViewShell() : NULL;
            SfxRequest aReq( SID_NEWDOCDIRECT, SFX_CALLMODE_SYNCHRON, aSet );
            aReq.AppendItem( SfxFrameItem( SID_DOCFRAME, pFrame ) );
            aReq.AppendItem( SfxStringItem( SID_NEWDOCDIRECT, String::CreateFromAscii(pFactory->GetShortName()) ) );
            const SfxPoolItem* pRet = pApp->ExecuteSlot( aReq );
            if ( pFrame->GetCurrentViewFrame() && pView != pFrame->GetCurrentViewFrame()->GetViewShell() )
            {
                bLoadState = sal_True;
            }
            else if ( xListener.is() )
            {
                if ( !pFrame->GetCurrentDocument() )
                {
                    pFrame->SetFrameInterface_Impl( Reference < XFrame >() );
                    pFrame->DoClose();
                }
                bLoadState = sal_False;
            }

            xFrame = Reference < XFrame >();
            return bLoadState;
        }
    }
    else
    {
        aSet.Put( SfxStringItem ( SID_FILE_NAME, rURL ) );
    }

    aSet.Put( SfxFrameItem( SID_DOCFRAME, pFrame ) );
    aSet.Put( SfxStringItem( SID_FILTER_NAME, aFilterName ) );
    pLoader = LoadEnvironment_Impl::Create( aSet );
    pLoader->AddRef();
    pLoader->SetDoneLink( LINK( this, SfxFrameLoader, LoadDone_Impl ) );
    if ( pFactory )
        pMatcher = new SfxFilterMatcher( pFactory->GetFilterContainer() );
    pLoader->SetFilterMatcher( pMatcher );
    pLoader->Start();

    bLoadDone = sal_False;
    while( bLoadDone == sal_False )
    {
        Application::Yield();
    }

    return bLoadState;
}
#else//MUSTFILTER
//OLD
void SAL_CALL SfxFrameLoader::load( const Reference < XFrame >& rFrame, const OUString& rURL,
                const Sequence < PropertyValue >& rArgs,
                const Reference < XLoadEventListener > & rListener) throw ( RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    xFrame = rFrame;
    xListener = rListener;

    // Achtung: beim Abräumen der Objekte kann die SfxApp destruiert werden, vorher noch Deinitialize_Impl rufen
    SfxApplication* pApp = SFX_APP();

    SfxAllItemSet aSet( pApp->GetPool() );
    TransformParameters( SID_OPENDOC, rArgs, aSet );

    SFX_ITEMSET_ARG( &aSet, pRefererItem, SfxStringItem, SID_REFERER, FALSE );
    if ( !pRefererItem )
        aSet.Put( SfxStringItem( SID_REFERER, String() ) );

    SfxFrame* pFrame=0;
    for ( pFrame = SfxFrame::GetFirst(); pFrame; pFrame = SfxFrame::GetNext( *pFrame ) )
    {
        if ( pFrame->GetFrameInterface() == xFrame )
            break;
    }

    if ( !pFrame )
        pFrame = SfxTopFrame::Create( rFrame );

    BOOL bFactoryURL = FALSE;
    const SfxObjectFactory* pFactory = 0;
    String aFact( rURL );
    String aPrefix = String::CreateFromAscii( "private:factory/" );
    String aParam;
    if ( aPrefix.Len() == aFact.Match( aPrefix ) )
    {
        bFactoryURL = TRUE;
        aFact.Erase( 0, aPrefix.Len() );
        USHORT nPos = aFact.Search( '?' );
        if ( nPos != STRING_NOTFOUND )
        {
            aParam = aFact.Copy( nPos, aFact.Len() );
            aFact.Erase( nPos, aFact.Len() );
            aParam.Erase(0,1);
        }
    }
    else
        aFact = aFilterName.GetToken( 0, ':' );

    WildCard aSearchedFac( aFact.EraseAllChars('4').ToUpperAscii() );
    for( USHORT n = SfxObjectFactory::GetObjectFactoryCount_Impl(); !pFactory && n--; )
    {
        pFactory = &SfxObjectFactory::GetObjectFactory_Impl( n );
        if( !aSearchedFac.Matches( String::CreateFromAscii( pFactory->GetShortName() ).ToUpperAscii() ) )
            pFactory = 0;
    }

    if ( bFactoryURL && pFactory )
    {
        INetURLObject aObj( rURL );
        if ( aParam.Len() )
        {
            sal_uInt16 nSlotId = aParam.ToInt32();
            SfxModule* pMod = pFactory->GetModule()->Load();
            SfxRequest aReq( nSlotId, SFX_CALLMODE_SYNCHRON, pMod->GetPool() );
            aReq.AppendItem( SfxStringItem ( SID_FILE_NAME, rURL ) );
            aReq.AppendItem( SfxFrameItem ( SID_DOCFRAME, pFrame ) );
            const SfxPoolItem* pRet = pMod->ExecuteSlot( aReq );
            if ( xListener.is() )
            {
                if ( pRet )
                    xListener->loadFinished( this );
                else
                {
                    if ( !pFrame->GetCurrentDocument() )
                    {
                        pFrame->SetFrameInterface_Impl( Reference < XFrame >() );
                        pFrame->DoClose();
                    }
                    xListener->loadCancelled( this );
                }
            }

            return;
        }

        String aPathName( aObj.GetMainURL() );
        if( pFactory->GetStandardTemplate().Len() )
        {
            aSet.Put( SfxStringItem ( SID_FILE_NAME, pFactory->GetStandardTemplate() ) );
            aSet.Put( SfxBoolItem( SID_TEMPLATE, sal_True ) );
        }
        else
        {
            SfxViewShell *pView = pFrame->GetCurrentViewFrame() ? pFrame->GetCurrentViewFrame()->GetViewShell() : NULL;
            SfxRequest aReq( SID_NEWDOCDIRECT, SFX_CALLMODE_SYNCHRON, aSet );
            aReq.AppendItem( SfxFrameItem( SID_DOCFRAME, pFrame ) );
            aReq.AppendItem( SfxStringItem( SID_NEWDOCDIRECT, String::CreateFromAscii(pFactory->GetShortName()) ) );
            const SfxPoolItem* pRet = pApp->ExecuteSlot( aReq );
            if ( pFrame->GetCurrentViewFrame() && pView != pFrame->GetCurrentViewFrame()->GetViewShell() )
            {
                if ( xListener.is() )
                    xListener->loadFinished( this );
            }
            else if ( xListener.is() )
            {
                if ( !pFrame->GetCurrentDocument() )
                {
                    pFrame->SetFrameInterface_Impl( Reference < XFrame >() );
                    pFrame->DoClose();
                }
                xListener->loadCancelled( this );
            }

            xFrame = Reference < XFrame >();
            xListener = Reference < XLoadEventListener >();
            return;
        }
    }
    else
    {
        aSet.Put( SfxStringItem ( SID_FILE_NAME, rURL ) );
    }

    aSet.Put( SfxFrameItem( SID_DOCFRAME, pFrame ) );
    aSet.Put( SfxStringItem( SID_FILTER_NAME, aFilterName ) );
    pLoader = LoadEnvironment_Impl::Create( aSet );
    pLoader->AddRef();
    pLoader->SetDoneLink( LINK( this, SfxFrameLoader, LoadDone_Impl ) );
    if ( pFactory )
        pMatcher = new SfxFilterMatcher( pFactory->GetFilterContainer() );
    pLoader->SetFilterMatcher( pMatcher );
    pLoader->Start();
}
#endif//MUSTFILTER

void SfxFrameLoader::cancel() throw( RUNTIME_EXCEPTION )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    if ( pLoader )
        pLoader->CancelTransfers();
}

#ifdef TF_FILTER//MUSTFILTER
IMPL_LINK( SfxFrameLoader, LoadDone_Impl, void*, pVoid )
{
    DBG_ASSERT( pLoader, "No Loader created, but LoadDone ?!" );

    if ( pLoader->GetError() )
    {
        SfxFrame* pFrame = pLoader->GetFrame();
        if ( pFrame && !pFrame->GetCurrentDocument() )
        {
            ::vos::OGuard aGuard( Application::GetSolarMutex() );
            pFrame->SetFrameInterface_Impl( Reference < XFrame >() );
            pFrame->DoClose();
        }
        bLoadDone  = sal_True ;
        bLoadState = sal_False;
    }
    else
    {
        bLoadDone  = sal_True;
        bLoadState = sal_True;
    }

    xFrame = Reference < XFrame >();
    xListener = Reference < XLoadEventListener >();
    return NULL;
}
#else//MUSTFILTER
IMPL_LINK( SfxFrameLoader, LoadDone_Impl, void*, pVoid )
{
    DBG_ASSERT( pLoader, "No Loader created, but LoadDone ?!" );

    if ( pLoader->GetError() )
    {
        SfxFrame* pFrame = pLoader->GetFrame();
        if ( pFrame && !pFrame->GetCurrentDocument() )
        {
            ::vos::OGuard aGuard( Application::GetSolarMutex() );
            pFrame->SetFrameInterface_Impl( Reference < XFrame >() );
            pFrame->DoClose();
        }
        if ( xListener.is() )
            xListener->loadCancelled( this );
    }
    else
    {
        if ( xListener.is() )
            xListener->loadFinished( this );
    }

    xFrame = Reference < XFrame >();
    xListener = Reference < XLoadEventListener >();
    return NULL;
}
#endif//MUSTFILTER

SfxObjectFactory& SfxFrameLoader_Impl::GetFactory()
{
    SfxObjectFactory* pFactory = 0;
    String aFact = GetFilterName().GetToken( ':', 0 );
    WildCard aSearchedFac( aFact.EraseAllChars('4').ToUpperAscii() );
    for( USHORT n = SfxObjectFactory::GetObjectFactoryCount_Impl(); !pFactory && n--; )
    {
        pFactory = (SfxObjectFactory*) &SfxObjectFactory::GetObjectFactory_Impl( n );
        if( !aSearchedFac.Matches( String::CreateFromAscii( pFactory->GetShortName() ).ToUpperAscii() ) )
            pFactory = 0;
    }

    return *pFactory;
}

#ifdef TF_FILTER//MUSTFILTER
::rtl::OUString SAL_CALL SfxFrameLoader::detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lDescriptor ) throw( ::com::sun::star::uno::RuntimeException )
{
    // Extract URL from given descriptor.
    String aURL;
    ::rtl::OUString sTemp;

    sal_uInt32 nPropertyCount = lDescriptor.getLength();
    for( sal_uInt32 nProperty=0; nProperty<nPropertyCount; ++nProperty )
    {
        if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("FileName")) )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            aURL = sTemp;
        }
        if( lDescriptor[nProperty].Name == OUString(RTL_CONSTASCII_USTRINGPARAM("FilterName")) )
        {
            lDescriptor[nProperty].Value >>= sTemp;
            // Convert new filtername to old format!
            aFilterName = impl_getOldFilterName( sTemp );
        }
    }

    // If url protocol = "private:factopry/*" ... there is no document to detect!
    // That will create a new one. We can break detection.
    String aPrefix = String::CreateFromAscii( "private:factory/" );
    if( aURL.Match( aPrefix ) == aPrefix.Len() )
    {
        // Convert old to new filter name.
        return impl_getNewFilterName( aFilterName );
    }

    SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
    const SfxFilter* pFilter = rMatcher.GetFilter( aFilterName );

    if ( pFilter )
    {
        SfxErrorContext aCtx( ERRCTX_SFX_OPENDOC, aURL );
        const SfxFilter* pNew = NULL;

        SfxApplication* pApp = SFX_APP();
        SfxAllItemSet *pSet = new SfxAllItemSet( pApp->GetPool() );
        TransformParameters( SID_OPENDOC, lDescriptor, *pSet );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        SfxMedium aMedium( aURL, (STREAM_READ | STREAM_SHARE_DENYNONE), sal_False, NULL, pSet );
        if ( aMedium.IsStorage() )
            aMedium.GetStorage();
        else
            aMedium.GetInStream();

        // Access to Medium was successfull ?
        if ( aMedium.GetErrorCode() == ERRCODE_NONE )
        {
    /*      String aMime;
            aMedium.GetMIMEAndRedirect( aMime );
            if( aMime.Len() )
                pFilter = rMatcher.GetFilter4Mime( aMime );
    */
            const SfxFilter* pOldFilter = pFilter;
            SfxFilterFlags nMust = SFX_FILTER_IMPORT, nDont = SFX_FILTER_NOTINSTALLED;
            ErrCode nErr = ERRCODE_ABORT;
            if ( ( (pFilter)->GetFilterFlags() & nMust ) == nMust && ( (pFilter)->GetFilterFlags() & nDont ) == 0 )
                nErr = pFilter->GetFilterContainer()->GetFilter4Content( aMedium, &pFilter );
            else
                // filterflags not suitable
                pFilter = NULL;

            // No error while reading from medium ?
            if ( aMedium.GetErrorCode() == ERRCODE_NONE )
            {
                if ( !pFilter || pOldFilter == pFilter && nErr != ERRCODE_NONE )
                {
                    // try simplest file lookup: clipboard format in storage
                    pFilter = NULL;
                    SvStorageRef aStor = aMedium.GetStorage();
                    SfxFilterFlags nFlags = SFX_FILTER_IMPORT | SFX_FILTER_PREFERED;
                    if ( aStor.Is() )
                    {
                        pFilter = rMatcher.GetFilter4ClipBoardId( aStor->GetFormat(), nFlags );
                        if ( !pFilter || ( (pFilter)->GetFilterFlags() & nMust ) != nMust || ( (pFilter)->GetFilterFlags() & nDont ) != 0 )
                            pFilter = rMatcher.GetFilter4ClipBoardId( aStor->GetFormat() );
                    }
                    if ( pFilter )
                        nErr = pFilter->GetFilterContainer()->GetFilter4Content( aMedium, &pFilter );
                }

                // No error while reading from medium ?
                if ( aMedium.GetErrorCode() == ERRCODE_NONE )
                {
                    if ( !pFilter || pOldFilter == pFilter && nErr != ERRCODE_NONE )
                    {
                        pFilter = NULL;
                        nErr = rMatcher.GetFilter4Content( aMedium, &pFilter );
                    }
                }
            }
        }

        if ( aMedium.GetErrorCode() != ERRCODE_NONE )
        {
            // when access to medium gives an error, the filter can't be valid
            pFilter = NULL;
            ErrorHandler::HandleError( aMedium.GetError() );
        }
    }

    if ( !pFilter )
        return ::rtl::OUString();
    else
        return impl_getNewFilterName( pFilter->GetName() );
}
#else//MUSTFILTER
::rtl::OUString SAL_CALL SfxFrameLoader::detect( const ::rtl::OUString& sURL,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& aArgumentlist ) throw(::com::sun::star::uno::RuntimeException )
{
    String aFact( sURL );
    String aPrefix = String::CreateFromAscii( "private:factory/" );
    if ( aPrefix.Len() == aFact.Match( aPrefix ) )
        return aFilterName;

    SfxFilterMatcher& rMatcher = SFX_APP()->GetFilterMatcher();
    const SfxFilter* pFilter = rMatcher.GetFilter( aFilterName );

    if ( pFilter )
    {
        SfxErrorContext aCtx( ERRCTX_SFX_OPENDOC, sURL );
        const SfxFilter* pNew = NULL;

        SfxApplication* pApp = SFX_APP();
        SfxAllItemSet *pSet = new SfxAllItemSet( pApp->GetPool() );
        TransformParameters( SID_OPENDOC, aArgumentlist, *pSet );

        ::vos::OGuard aGuard( Application::GetSolarMutex() );
        SfxMedium aMedium( sURL, (STREAM_READ | STREAM_SHARE_DENYNONE), sal_False, NULL, pSet );
        if ( aMedium.IsStorage() )
            aMedium.GetStorage();
        else
            aMedium.GetInStream();

        // Access to Medium was successfull ?
        if ( aMedium.GetErrorCode() == ERRCODE_NONE )
        {
    /*      String aMime;
            aMedium.GetMIMEAndRedirect( aMime );
            if( aMime.Len() )
                pFilter = rMatcher.GetFilter4Mime( aMime );
    */
            const SfxFilter* pOldFilter = pFilter;
            SfxFilterFlags nMust = SFX_FILTER_IMPORT, nDont = SFX_FILTER_NOTINSTALLED;
            ErrCode nErr = ERRCODE_ABORT;
            if ( ( (pFilter)->GetFilterFlags() & nMust ) == nMust && ( (pFilter)->GetFilterFlags() & nDont ) == 0 )
                nErr = pFilter->GetFilterContainer()->GetFilter4Content( aMedium, &pFilter );
            else
                // filterflags not suitable
                pFilter = NULL;

            // No error while reading from medium ?
            if ( aMedium.GetErrorCode() == ERRCODE_NONE )
            {
                if ( !pFilter || pOldFilter == pFilter && nErr != ERRCODE_NONE )
                {
                    // try simplest file lookup: clipboard format in storage
                    pFilter = NULL;
                    SvStorageRef aStor = aMedium.GetStorage();
                    SfxFilterFlags nFlags = SFX_FILTER_IMPORT | SFX_FILTER_PREFERED;
                    if ( aStor.Is() )
                    {
                        pFilter = rMatcher.GetFilter4ClipBoardId( aStor->GetFormat(), nFlags );
                        if ( !pFilter || ( (pFilter)->GetFilterFlags() & nMust ) != nMust || ( (pFilter)->GetFilterFlags() & nDont ) != 0 )
                            pFilter = rMatcher.GetFilter4ClipBoardId( aStor->GetFormat() );
                    }
                    if ( pFilter )
                        nErr = pFilter->GetFilterContainer()->GetFilter4Content( aMedium, &pFilter );
                }

                // No error while reading from medium ?
                if ( aMedium.GetErrorCode() == ERRCODE_NONE )
                {
                    if ( !pFilter || pOldFilter == pFilter && nErr != ERRCODE_NONE )
                    {
                        pFilter = NULL;
                        nErr = rMatcher.GetFilter4Content( aMedium, &pFilter );
                    }
                }
            }
        }

        if ( aMedium.GetErrorCode() != ERRCODE_NONE )
        {
            // when access to medium gives an error, the filter can't be valid
            pFilter = NULL;
            ErrorHandler::HandleError( aMedium.GetError() );
        }
    }

    if ( !pFilter )
        return ::rtl::OUString();
    else
        return pFilter->GetName();
}
#endif//MUSTFILTER

SFX_IMPL_XINTERFACE_0( SfxFrameLoader_Impl, SfxFrameLoader )
SFX_IMPL_SINGLEFACTORY( SfxFrameLoader_Impl )

/* XServiceInfo */
UNOOUSTRING SAL_CALL SfxFrameLoader_Impl::getImplementationName() throw( UNORUNTIMEEXCEPTION )
{
    return impl_getStaticImplementationName();
}
                                                                                                                                \
/* XServiceInfo */
sal_Bool SAL_CALL SfxFrameLoader_Impl::supportsService( const UNOOUSTRING& sServiceName ) throw( UNORUNTIMEEXCEPTION )
{
    UNOSEQUENCE< UNOOUSTRING >  seqServiceNames =   getSupportedServiceNames();
    const UNOOUSTRING*          pArray          =   seqServiceNames.getConstArray();
    for ( sal_Int32 nCounter=0; nCounter<seqServiceNames.getLength(); nCounter++ )
    {
        if ( pArray[nCounter] == sServiceName )
        {
            return sal_True ;
        }
    }
    return sal_False ;
}

/* XServiceInfo */
UNOSEQUENCE< UNOOUSTRING > SAL_CALL SfxFrameLoader_Impl::getSupportedServiceNames() throw( UNORUNTIMEEXCEPTION )
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
UNOSEQUENCE< UNOOUSTRING > SfxFrameLoader_Impl::impl_getStaticSupportedServiceNames()
{
    UNOMUTEXGUARD aGuard( UNOMUTEX::getGlobalMutex() );
    UNOSEQUENCE< UNOOUSTRING > seqServiceNames( 2 );
    seqServiceNames.getArray() [0] = UNOOUSTRING::createFromAscii( "com.sun.star.frame.FrameLoader" );
    seqServiceNames.getArray() [1] = UNOOUSTRING::createFromAscii( "com.sun.star.frame.ExtendedFilterDetect" );
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
UNOOUSTRING SfxFrameLoader_Impl::impl_getStaticImplementationName()
{
    return UNOOUSTRING::createFromAscii( "com.sun.star.comp.office.FrameLoader" );
}

/* Helper for registry */
UNOREFERENCE< UNOXINTERFACE > SAL_CALL SfxFrameLoader_Impl::impl_createInstance( const UNOREFERENCE< UNOXMULTISERVICEFACTORY >& xServiceManager ) throw( UNOEXCEPTION )
{
    return UNOREFERENCE< UNOXINTERFACE >( *new SfxFrameLoader_Impl( xServiceManager ) );
}

SfxFrameLoader_Impl::SfxFrameLoader_Impl( const Reference < XMultiServiceFactory >& xFactory )
    : SfxFrameLoader( xFactory )
{
}


