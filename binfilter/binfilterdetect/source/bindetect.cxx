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

#include "bindetect.hxx"

#ifndef _COM_SUN_STAR_IO_XINPUTSTREAM_HPP_
#include <com/sun/star/io/XInputStream.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XCONTAINERQUERY_HPP_
#include <com/sun/star/container/XContainerQuery.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYVALUE_HPP_
#include <com/sun/star/beans/PropertyValue.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_NAMEDVALUE_HPP_
#include <com/sun/star/beans/NamedValue.hpp>
#endif

#ifndef _URLOBJ_HXX
#include <tools/urlobj.hxx>
#endif
#ifndef INCLUDED_SVTOOLS_PATHOPTIONS_HXX
#include <bf_svtools/pathoptions.hxx>
#endif
#include <rtl/ustring.h>
#include <vos/mutex.hxx>
#include <sot/storage.hxx>
#include <sot/exchange.hxx>
#include <comphelper/mediadescriptor.hxx>
#include <unotools/ucbstreamhelper.hxx>
#include <vcl/svapp.hxx>
#ifndef _OSL_MODULE_HXX_
#include <osl/module.hxx>
#endif
#ifndef _OSL_FILE_HXX_
#include <osl/file.hxx>
#endif

#if defined( WNT )

// Typen fuer die Funktion der W4W-DLL
#include <tools/svwin.h>
#endif

#ifdef UNX
#include <stdlib.h>
#endif

using namespace ::com::sun::star::registry;
using namespace ::com::sun::star::uno;

namespace binfilter {

struct TypesInfo
{
    const char* Type;
    sal_Int32   Length;
};

// --> OD 2005-08-04 #i47323#, #i51938# - list of file types
// the binfilter modul is responsible for
// - see /filter/source/config/fragments/filters/
static const sal_uInt8 nStreamFileTypeCount = 114;
static const sal_uInt8 nFileTypeCount = 29 + nStreamFileTypeCount;
static const sal_uInt8 nW4WStartOffset = 5;
static const TypesInfo aFileTypeList[ nFileTypeCount ] =
{
    RTL_CONSTASCII_STRINGPARAM("writer_StarWriter_10"),
    RTL_CONSTASCII_STRINGPARAM("writer_StarWriter_20"),
    RTL_CONSTASCII_STRINGPARAM("writer_StarWriter_DOS"),
    RTL_CONSTASCII_STRINGPARAM("writer_Lotus_1_2_3_10_DOS_StarWriter"),
    RTL_CONSTASCII_STRINGPARAM("writer_Lotus_1_2_3_10_WIN_StarWriter"),
    RTL_CONSTASCII_STRINGPARAM("writer_Ami_Pro_1x_31_W4W"),                 // 5 , 33
    RTL_CONSTASCII_STRINGPARAM("writer_CTOS_DEF_W4W"),                      // 6 , 36
    RTL_CONSTASCII_STRINGPARAM("writer_Claris_Works_W4W"),                  // 7 , 57
    RTL_CONSTASCII_STRINGPARAM("writer_DCA_Revisable_Form_Text_W4W"),       // 8 , 15
    RTL_CONSTASCII_STRINGPARAM("writer_DCA_with_Display_Write_5_W4W"),      // 9 , 15
    RTL_CONSTASCII_STRINGPARAM("writer_DCAFFT_Final_Form_Text_W4W"),        //10, 32
    RTL_CONSTASCII_STRINGPARAM("writer_DEC_DX_W4W"),                        //11, 30
    RTL_CONSTASCII_STRINGPARAM("writer_DEC_WPS_PLUS_W4W"),                  //12, 45
    RTL_CONSTASCII_STRINGPARAM("writer_DataGeneral_CEO_Write_W4W"),         //13, 104
    RTL_CONSTASCII_STRINGPARAM("writer_DisplayWrite_20_4x_W4W"),            //14, 15
    RTL_CONSTASCII_STRINGPARAM("writer_DisplayWrite_5x_W4W"),               //15, 15
    RTL_CONSTASCII_STRINGPARAM("writer_EBCDIC_W4W"),                        //16, 02
    RTL_CONSTASCII_STRINGPARAM("writer_Enable_W4W"),                        //17, 28
    RTL_CONSTASCII_STRINGPARAM("writer_Frame_Maker_MIF_30_W4W"),            //18, 42
    RTL_CONSTASCII_STRINGPARAM("writer_Frame_Maker_MIF_40_W4W"),            //19, 42
    RTL_CONSTASCII_STRINGPARAM("writer_Frame_Maker_MIF_50_W4W"),            //20, 42
    RTL_CONSTASCII_STRINGPARAM("writer_Frame_Work_III_W4W"),                //21, 29
    RTL_CONSTASCII_STRINGPARAM("writer_Frame_Work_IV_W4W"),                 //22, 29
    RTL_CONSTASCII_STRINGPARAM("writer_HP_AdvanceWrite_Plus_W4W"),          //23, 22
    RTL_CONSTASCII_STRINGPARAM("writer_ICL_Office_Power_6_W4W"),            //24, 102
    RTL_CONSTASCII_STRINGPARAM("writer_ICL_Office_Power_7_W4W"),            //25, 102
    RTL_CONSTASCII_STRINGPARAM("writer_Interleaf_W4W"),                     //26, 35
    RTL_CONSTASCII_STRINGPARAM("writer_Interleaf_5_6_W4W"),                 //27, 46
    RTL_CONSTASCII_STRINGPARAM("writer_Legacy_Winstar_onGO_W4W"),           //28, 37
    RTL_CONSTASCII_STRINGPARAM("writer_Lotus_Manuscript_W4W"),              //29, 24
    RTL_CONSTASCII_STRINGPARAM("writer_MASS_11_Rel_80_83_W4W"),             //30, 31
    RTL_CONSTASCII_STRINGPARAM("writer_MASS_11_Rel_85_90_W4W"),             //31, 31
    RTL_CONSTASCII_STRINGPARAM("writer_MS_MacWord_30_W4W"),                 //32, 54
    RTL_CONSTASCII_STRINGPARAM("writer_MS_MacWord_40_W4W"),                 //33, 54
    RTL_CONSTASCII_STRINGPARAM("writer_MS_MacWord_5x_W4W"),                 //34, 54
    RTL_CONSTASCII_STRINGPARAM("writer_MS_WinWord_1x_W4W"),                 //35, 44
    RTL_CONSTASCII_STRINGPARAM("writer_MS_WinWord_2x_W4W"),                 //36, 44
    RTL_CONSTASCII_STRINGPARAM("writer_MS_Word_3x_W4W"),                    //37, 05
    RTL_CONSTASCII_STRINGPARAM("writer_MS_Word_4x_W4W"),                    //38, 05
    RTL_CONSTASCII_STRINGPARAM("writer_MS_Word_5x_W4W"),                    //39, 05
    RTL_CONSTASCII_STRINGPARAM("writer_MS_Word_6x_W4W"),                    //40, 49
    RTL_CONSTASCII_STRINGPARAM("writer_MS_Works_20_DOS_W4W"),               //41, 39
    RTL_CONSTASCII_STRINGPARAM("writer_MS_Works_30_Win_W4W"),               //42, 39
    RTL_CONSTASCII_STRINGPARAM("writer_MS_Works_40_Mac_W4W"),               //43, 58
    RTL_CONSTASCII_STRINGPARAM("writer_Mac_Write_4x_50_W4W"),               //44, 51
    RTL_CONSTASCII_STRINGPARAM("writer_Mac_Write_II_W4W"),                  //45, 52
    RTL_CONSTASCII_STRINGPARAM("writer_Mac_Write_Pro_W4W"),                 //46, 56
    RTL_CONSTASCII_STRINGPARAM("writer_MultiMate_33_W4W"),                  //47, 10
    RTL_CONSTASCII_STRINGPARAM("writer_MultiMate_4_W4W"),                   //48, 10
    RTL_CONSTASCII_STRINGPARAM("writer_MultiMate_Adv_36_W4W"),              //49, 10
    RTL_CONSTASCII_STRINGPARAM("writer_MultiMate_Adv_II_37_W4W"),           //50, 10
    RTL_CONSTASCII_STRINGPARAM("writer_NAVY_DIF_W4W"),                      //51, 18
    RTL_CONSTASCII_STRINGPARAM("writer_OfficeWriter_40_W4W"),               //52, 16
    RTL_CONSTASCII_STRINGPARAM("writer_OfficeWriter_50_W4W"),               //53, 16
    RTL_CONSTASCII_STRINGPARAM("writer_OfficeWriter_6x_W4W"),               //54, 16
    RTL_CONSTASCII_STRINGPARAM("writer_PFS_First_Choice_10_W4W"),           //55, 08
    RTL_CONSTASCII_STRINGPARAM("writer_PFS_First_Choice_20_W4W"),           //56, 08
    RTL_CONSTASCII_STRINGPARAM("writer_PFS_First_Choice_30_W4W"),           //57, 08
    RTL_CONSTASCII_STRINGPARAM("writer_PFS_Write_W4W"),                     //58, 08
    RTL_CONSTASCII_STRINGPARAM("writer_Peach_Text_W4W"),                    //59, 27
    RTL_CONSTASCII_STRINGPARAM("writer_Professional_Write_10_W4W"),         //60, 33
    RTL_CONSTASCII_STRINGPARAM("writer_Professional_Write_2x_W4W"),         //61, 33
    RTL_CONSTASCII_STRINGPARAM("writer_Professional_Write_Plus_W4W"),       //62, 33
    RTL_CONSTASCII_STRINGPARAM("writer_QA_Write_10_30_W4W"),                //63, 23
    RTL_CONSTASCII_STRINGPARAM("writer_QA_Write_40_W4W"),                   //64, 23
    RTL_CONSTASCII_STRINGPARAM("writer_Rapid_File_10_W4W"),                 //65, 25
    RTL_CONSTASCII_STRINGPARAM("writer_Rapid_File_12_W4W"),                 //66, 25
    RTL_CONSTASCII_STRINGPARAM("writer_Samna_Word_IV_IV_Plus_W4W"),         //67, 22
    RTL_CONSTASCII_STRINGPARAM("writer_Total_Word_W4W"),                    //68, 14
    RTL_CONSTASCII_STRINGPARAM("writer_Uniplex_V7_V8_W4W"),                 //69, 101
    RTL_CONSTASCII_STRINGPARAM("writer_Uniplex_onGO_W4W"),                  //70, 101
    RTL_CONSTASCII_STRINGPARAM("writer_VolksWriter_3_and_4_W4W"),           //71, 14
    RTL_CONSTASCII_STRINGPARAM("writer_VolksWriter_Deluxe_W4W"),            //72, 11
    RTL_CONSTASCII_STRINGPARAM("writer_WITA_W4W"),                          //73, 34
    RTL_CONSTASCII_STRINGPARAM("writer_Wang_II_SWP_W4W"),                   //74, 88
    RTL_CONSTASCII_STRINGPARAM("writer_Wang_PC_W4W"),                       //75, 26
    RTL_CONSTASCII_STRINGPARAM("writer_Wang_WP_Plus_W4W"),                  //76, 89
    RTL_CONSTASCII_STRINGPARAM("writer_Win_Write_3x_W4W"),                  //77, 43
    RTL_CONSTASCII_STRINGPARAM("writer_WiziWord_30_W4W"),                   //78, 47
    RTL_CONSTASCII_STRINGPARAM("writer_WordPerfect_Win_51_52_W4W"),         //79, 07
    RTL_CONSTASCII_STRINGPARAM("writer_WordPerfect_Win_60_W4W"),            //80, 48
    RTL_CONSTASCII_STRINGPARAM("writer_WordPerfect_Win_61_W4W"),            //81, 48
    RTL_CONSTASCII_STRINGPARAM("writer_WordPerfect_Win_70_W4W"),            //82, 07
    RTL_CONSTASCII_STRINGPARAM("writer_WordPerfect_41_W4W"),                //83, 06
    RTL_CONSTASCII_STRINGPARAM("writer_WordPerfect_42_W4W"),                //84, 06
    RTL_CONSTASCII_STRINGPARAM("writer_WordPerfect_50_W4W"),                //85, 07
    RTL_CONSTASCII_STRINGPARAM("writer_WordPerfect_51_W4W"),                //86, 07
    RTL_CONSTASCII_STRINGPARAM("writer_WordPerfect_60_W4W"),                //87, 07
    RTL_CONSTASCII_STRINGPARAM("writer_WordPerfect_61_W4W"),                //88, 07
    RTL_CONSTASCII_STRINGPARAM("writer_WordPerfect_Mac_1_W4W"),             //89, 59
    RTL_CONSTASCII_STRINGPARAM("writer_WordPerfect_Mac_2_W4W"),             //90, 60
    RTL_CONSTASCII_STRINGPARAM("writer_WordPerfect_Mac_3_W4W"),             //91, 60
    RTL_CONSTASCII_STRINGPARAM("writer_WordStar_Win_1x_20_W4W"),            //92, 04
    RTL_CONSTASCII_STRINGPARAM("writer_WordStar_2000_Rel_30_W4W"),          //93, 09
    RTL_CONSTASCII_STRINGPARAM("writer_WordStar_2000_Rel_35_W4W"),          //94, 09
    RTL_CONSTASCII_STRINGPARAM("writer_WordStar_33x_W4W"),                  //95, 04
    RTL_CONSTASCII_STRINGPARAM("writer_WordStar_345_W4W"),                  //96, 04
    RTL_CONSTASCII_STRINGPARAM("writer_WordStar_40_W4W"),                   //97, 04
    RTL_CONSTASCII_STRINGPARAM("writer_WordStar_50_W4W"),                   //98, 04
    RTL_CONSTASCII_STRINGPARAM("writer_WordStar_55_W4W"),                   //99, 04
    RTL_CONSTASCII_STRINGPARAM("writer_WordStar_60_W4W"),                   //100, 04
    RTL_CONSTASCII_STRINGPARAM("writer_WordStar_70_W4W"),                   //101, 04
    RTL_CONSTASCII_STRINGPARAM("writer_WriteNow_30_Macintosh_W4W"),         //102, 62
    RTL_CONSTASCII_STRINGPARAM("writer_Writing_Assistant_W4W"),             //103, 13
    RTL_CONSTASCII_STRINGPARAM("writer_XEROX_XIF_50_Illustrator_W4W"),      //104, 103
    RTL_CONSTASCII_STRINGPARAM("writer_XEROX_XIF_50_W4W"),                  //105, 103
    RTL_CONSTASCII_STRINGPARAM("writer_XEROX_XIF_60_Color_Bitmap_W4W"),     //106, 103
    RTL_CONSTASCII_STRINGPARAM("writer_XEROX_XIF_60_Res_Graphic_W4W"),      //107, 103
    RTL_CONSTASCII_STRINGPARAM("writer_XyWrite_Win_10_W4W"),                //108, 17
    RTL_CONSTASCII_STRINGPARAM("writer_XyWrite_III_W4W"),                   //109, 17
    RTL_CONSTASCII_STRINGPARAM("writer_XyWrite_IIIP_W4W"),                  //110, 17
    RTL_CONSTASCII_STRINGPARAM("writer_XyWrite_IV_W4W"),                    //111, 17
    RTL_CONSTASCII_STRINGPARAM("writer_XyWrite_Sig_Win_W4W"),               //112, 17
    RTL_CONSTASCII_STRINGPARAM("writer_XyWrite_Signature_W4W"),             //113, 17
    RTL_CONSTASCII_STRINGPARAM("calc_StarCalc_30"),
    RTL_CONSTASCII_STRINGPARAM("calc_StarCalc_30_VorlageTemplate"),
    RTL_CONSTASCII_STRINGPARAM("calc_StarCalc_40"),
    RTL_CONSTASCII_STRINGPARAM("calc_StarCalc_40_VorlageTemplate"),
    RTL_CONSTASCII_STRINGPARAM("calc_StarCalc_50"),
    RTL_CONSTASCII_STRINGPARAM("calc_StarCalc_50_VorlageTemplate"),
    RTL_CONSTASCII_STRINGPARAM("chart_StarChart_30"),
    RTL_CONSTASCII_STRINGPARAM("chart_StarChart_40"),
    RTL_CONSTASCII_STRINGPARAM("chart_StarChart_50"),
    RTL_CONSTASCII_STRINGPARAM("draw_StarDraw_30"),
    RTL_CONSTASCII_STRINGPARAM("draw_StarDraw_30_Vorlage"),
    RTL_CONSTASCII_STRINGPARAM("draw_StarDraw_50"),
    RTL_CONSTASCII_STRINGPARAM("draw_StarDraw_50_Vorlage"),
    RTL_CONSTASCII_STRINGPARAM("impress_StarImpress_40"),
    RTL_CONSTASCII_STRINGPARAM("impress_StarImpress_40_Vorlage"),
    RTL_CONSTASCII_STRINGPARAM("impress_StarImpress_50"),
    RTL_CONSTASCII_STRINGPARAM("impress_StarImpress_50_Vorlage"),
    RTL_CONSTASCII_STRINGPARAM("impress_StarImpress_50_packed"),
    RTL_CONSTASCII_STRINGPARAM("math_StarMath_30"),
    RTL_CONSTASCII_STRINGPARAM("math_StarMath_40"),
    RTL_CONSTASCII_STRINGPARAM("math_StarMath_50"),
    RTL_CONSTASCII_STRINGPARAM("writer_StarWriter_30"),
    RTL_CONSTASCII_STRINGPARAM("writer_StarWriter_30_VorlageTemplate"),
    RTL_CONSTASCII_STRINGPARAM("writer_StarWriter_40"),
    RTL_CONSTASCII_STRINGPARAM("writer_StarWriter_40_VorlageTemplate"),
    RTL_CONSTASCII_STRINGPARAM("writer_globaldocument_StarWriter_40GlobalDocument"),
    RTL_CONSTASCII_STRINGPARAM("writer_StarWriter_50"),
    RTL_CONSTASCII_STRINGPARAM("writer_StarWriter_50_VorlageTemplate"),
    RTL_CONSTASCII_STRINGPARAM("writer_globaldocument_StarWriter_50GlobalDocument")
};
//list of Word4Word filters the identifiers are returned by AutoDetec
static const sal_uInt8 nW4WFilterCount = 109;
static const USHORT aW4WIdentifier[nW4WFilterCount] =
{
    33  ,//"writer_Ami_Pro_1x_31_W4W"
    36  ,//"writer_CTOS_DEF_W4W"
    57  ,//"writer_Claris_Works_W4W"
    15  ,//"writer_DCA_Revisable_Form_Text_W4W"
    15  ,//"writer_DCA_with_Display_Write_5_W4W"
    32  ,//"writer_DCAFFT_Final_Form_Text_W4W"
    30  ,//"writer_DEC_DX_W4W"
    45  ,//"writer_DEC_WPS_PLUS_W4W"
    104 ,//"writer_DataGeneral_CEO_Write_W4W"
    15  ,//"writer_DisplayWrite_20_4x_W4W"
    15  ,//"writer_DisplayWrite_5x_W4W"
    02  ,//"writer_EBCDIC_W4W"
    28  ,//"writer_Enable_W4W"
    42  ,//"writer_Frame_Maker_MIF_30_W4W"
    42  ,//"writer_Frame_Maker_MIF_40_W4W"
    42  ,//"writer_Frame_Maker_MIF_50_W4W"
    29  ,//"writer_Frame_Work_III_W4W"
    29  ,//"writer_Frame_Work_IV_W4W"
    22  ,//"writer_HP_AdvanceWrite_Plus_W4W"
    102 ,//"writer_ICL_Office_Power_6_W4W"
    102 ,//"writer_ICL_Office_Power_7_W4W"
    35  ,//"writer_Interleaf_W4W"
    46  ,//"writer_Interleaf_5_6_W4W"
    37  ,//"writer_Legacy_Winstar_onGO_W4W"
    24  ,//"writer_Lotus_Manuscript_W4W"
    31  ,//"writer_MASS_11_Rel_80_83_W4W"
    31  ,//"writer_MASS_11_Rel_85_90_W4W"
    54  ,//"writer_MS_MacWord_30_W4W"
    54  ,//"writer_MS_MacWord_40_W4W"
    54  ,//"writer_MS_MacWord_5x_W4W"
    44  ,//"writer_MS_WinWord_1x_W4W"
    44  ,//"writer_MS_WinWord_2x_W4W"
    05  ,//"writer_MS_Word_3x_W4W"
    05  ,//"writer_MS_Word_4x_W4W"
    05  ,//"writer_MS_Word_5x_W4W"
    49  ,//"writer_MS_Word_6x_W4W"
    39  ,//"writer_MS_Works_20_DOS_W4W"
    39  ,//"writer_MS_Works_30_Win_W4W"
    58  ,//"writer_MS_Works_40_Mac_W4W"
    51  ,//"writer_Mac_Write_4x_50_W4W"
    52  ,//"writer_Mac_Write_II_W4W"
    56  ,//"writer_Mac_Write_Pro_W4W"
    10  ,//"writer_MultiMate_33_W4W"
    10  ,//"writer_MultiMate_4_W4W"
    10  ,//"writer_MultiMate_Adv_36_W4W"
    10  ,//"writer_MultiMate_Adv_II_37_W4W"
    18  ,//"writer_NAVY_DIF_W4W"
    16  ,//"writer_OfficeWriter_40_W4W"
    16  ,//"writer_OfficeWriter_50_W4W"
    16  ,//"writer_OfficeWriter_6x_W4W"
    8  ,//"writer_PFS_First_Choice_10_W4W"
    8  ,//"writer_PFS_First_Choice_20_W4W"
    8  ,//"writer_PFS_First_Choice_30_W4W"
    8  ,//"writer_PFS_Write_W4W"
    27  ,//"writer_Peach_Text_W4W"
    33  ,//"writer_Professional_Write_10_W4W"
    33  ,//"writer_Professional_Write_2x_W4W"
    33  ,//"writer_Professional_Write_Plus_W4W"
    23  ,//"writer_QA_Write_10_30_W4W"
    23  ,//"writer_QA_Write_40_W4W"
    25  ,//"writer_Rapid_File_10_W4W"
    25  ,//"writer_Rapid_File_12_W4W"
    22  ,//"writer_Samna_Word_IV_IV_Plus_W4W"
    14  ,//"writer_Total_Word_W4W"
    101 ,//"writer_Uniplex_V7_V8_W4W"
    101 ,//"writer_Uniplex_onGO_W4W"
    14  ,//"writer_VolksWriter_3_and_4_W4W"
    11  ,//"writer_VolksWriter_Deluxe_W4W"
    34  ,//"writer_WITA_W4W"
    88  ,//"writer_Wang_II_SWP_W4W"
    26  ,//"writer_Wang_PC_W4W"
    89  ,//"writer_Wang_WP_Plus_W4W"
    43  ,//"writer_Win_Write_3x_W4W"
    47  ,//"writer_WiziWord_30_W4W"
    7  ,//"writer_WordPerfect_Win_51_52_W4W"
    48  ,//"writer_WordPerfect_Win_60_W4W"
    48  ,//"writer_WordPerfect_Win_61_W4W"
    7  ,//"writer_WordPerfect_Win_70_W4W"
    6  ,//"writer_WordPerfect_41_W4W"
    6  ,//"writer_WordPerfect_42_W4W"
    7  ,//"writer_WordPerfect_50_W4W"
    7  ,//"writer_WordPerfect_51_W4W"
    7  ,//"writer_WordPerfect_60_W4W"
    7  ,//"writer_WordPerfect_61_W4W"
    59  ,//"writer_WordPerfect_Mac_1_W4W"
    60  ,//"writer_WordPerfect_Mac_2_W4W"
    60  ,//"writer_WordPerfect_Mac_3_W4W"
    4  ,//"writer_WordStar_Win_1x_20_W4W"
    9  ,//"writer_WordStar_2000_Rel_30_W4W"
    9  ,//"writer_WordStar_2000_Rel_35_W4W"
    4  ,//"writer_WordStar_33x_W4W"
    4  ,//"writer_WordStar_345_W4W"
    4  ,//"writer_WordStar_40_W4W"
    4  ,//"writer_WordStar_50_W4W"
    4  ,//"writer_WordStar_55_W4W"
    4 ,//"writer_WordStar_60_W4W"
    4 ,//"writer_WordStar_70_W4W"
     62 ,//"writer_WriteNow_30_Macintosh_W4W"
     13 ,//"writer_Writing_Assistant_W4W"
     103,//"writer_XEROX_XIF_50_Illustrator_W4W"
     103,//"writer_XEROX_XIF_50_W4W"
     103,//"writer_XEROX_XIF_60_Color_Bitmap_W4W"
     103,//"writer_XEROX_XIF_60_Res_Graphic_W4W"
     17 ,//"writer_XyWrite_Win_10_W4W"
     17 ,//"writer_XyWrite_III_W4W"
     17 ,//"writer_XyWrite_IIIP_W4W"
     17 ,//"writer_XyWrite_IV_W4W"
     17 ,//"writer_XyWrite_Sig_Win_W4W"
     17 //"writer_XyWrite_Signature_W4W"
};

/*-- 06.03.2006 14:48:00---------------------------------------------------

  -----------------------------------------------------------------------*/
#ifndef WNT
//Same as osl::Module, except will search for its modules in the filter subdir
class filterModule : public osl::Module
{
public:
    sal_Bool filterLoad(String &rStr);
};

extern "C" { static void SAL_CALL thisModule() {} }

sal_Bool filterModule::filterLoad(String &rStr)
{
    sal_Bool bRet=sal_False;
    SvtPathOptions aOpt;
    if (aOpt.SearchFile(rStr, SvtPathOptions::PATH_FILTER))
    {
        rtl::OUString aPathURL;
        osl::FileBase::getFileURLFromSystemPath(rtl::OUString(rStr),aPathURL);
        bRet = loadRelative(&thisModule, aPathURL);
    }
    return bRet;
}

#endif
USHORT AutoDetec( const String& rFileName, USHORT & rVersion )
{
    ByteString aFileNm( rFileName, ::gsl_getSystemTextEncoding() );
    USHORT nFileType = 0;

#if defined( WNT )

    typedef int (FAR PASCAL *FNautorec)( LPSTR, int, LPINT );

    String aAutoRecDLL(RTL_CONSTASCII_STRINGPARAM("autorec.dll"));
    SvtPathOptions aOpt;
    if( !aOpt.SearchFile( aAutoRecDLL, SvtPathOptions::PATH_FILTER ))
        return 0;

    ByteString sAutoRecDllNm( aAutoRecDLL, ::gsl_getSystemTextEncoding() );
    HANDLE hDLL = LoadLibrary( (LPSTR)sAutoRecDllNm.GetBuffer() );
    if( hDLL >= (HANDLE)HINSTANCE_ERROR )
    {
        FNautorec fnAutorec = (FNautorec)GetProcAddress( (HINSTANCE)hDLL,
                                                    (LPSTR)"autorec" );

        long nVersion = 0;
        if( fnAutorec )
            nFileType = (*fnAutorec)( (LPSTR)aFileNm.GetBuffer(),
                                        0,
                                        (LPINT)&nVersion );
        rVersion = (USHORT)nVersion;
        FreeLibrary( (HINSTANCE)hDLL );
    }
#else
    //This is the nice modern code, the other stuff is a bit awful, but for
    //some reason the different platforms had different argument lists from
    //eachother, so we can't have super clean code :-(
    filterModule aAutoRec;
    String aTmpStr(RTL_CONSTASCII_STRINGPARAM((SVLIBRARY("autorec"))));
    if (aAutoRec.filterLoad(aTmpStr))
    {
        typedef int (*autorec_t)(const char*, int*);
        if (autorec_t autorec = (autorec_t)aAutoRec.getSymbol(String::CreateFromAscii("autorec")))
        {
            int nVersion;
            if ((nFileType = (USHORT)autorec(aFileNm.GetBuffer(), &nVersion)))
                rVersion = (USHORT)nVersion;
        }
    }
#endif

    if( nFileType == 31 )   // Autorec-Bug umpopeln: Eine ASCII-Datei, die nur
        nFileType = 1;      // aus sehr vielen CRs besteht,
                            // wird als VAX Mass-11 erkannt

    return nFileType == (USHORT)-1 ? 0 : nFileType;
}

bool checkFormat( SvStream* pStream, ::rtl::OUString& rTypeName, ::rtl::OUString& rURL, bool bCheckAll )
{
    sal_Char aBuffer[4098];
    if( pStream && !pStream->GetError() )
    {
        const ULONG nMaxRead = sizeof(aBuffer) - 2;
        pStream->Seek(STREAM_SEEK_TO_BEGIN);
        ULONG nBytesRead = pStream->Read(aBuffer, nMaxRead);
        pStream->Seek(STREAM_SEEK_TO_BEGIN);
        if (nBytesRead <= nMaxRead)
        {
            aBuffer[nBytesRead] = '\0';
            aBuffer[nBytesRead+1] = '\0';
            if (nBytesRead & 0x00000001)
                aBuffer[nBytesRead+2] = '\0';
        }
    }
    else
        return false;
    // clear type name in case nothing gets detected
    ::rtl::OUString aName( rTypeName );
    rTypeName = ::rtl::OUString();

    const sal_Char __FAR_DATA FILTER_SWG[]    = "SWG";
    if ( bCheckAll || aName.equalsAsciiL( aFileTypeList[0].Type, aFileTypeList[0].Length ) )
    {
        // StarWriter 1.0
        if ( 0 == strncmp( FILTER_SWG, aBuffer, 3 ) &&
                '1' == *(aBuffer + 3) )
        {
            rTypeName = ::rtl::OUString::createFromAscii( aFileTypeList[0].Type );
        }
    }
    if ( bCheckAll || aName.equalsAsciiL( aFileTypeList[1].Type, aFileTypeList[1].Length ) )
    {
        // StarWriter 2.0
        if( 0 == strncmp( FILTER_SWG, aBuffer, 3 ) &&
                '1' != *(aBuffer + 3) )
        {
            rTypeName = ::rtl::OUString::createFromAscii( aFileTypeList[1].Type );
        }
    }
    if ( bCheckAll || aName.equalsAsciiL( aFileTypeList[2].Type, aFileTypeList[2].Length ) )
    {
        // StarWriter DOS
        sal_Char __READONLY_DATA sSw6_FormatStt[] =         ".\\\\\\ WRITER ";
        sal_Char __READONLY_DATA sSw6_FormatEnd[] =         " \\\\\\";


        if ( 0 == strncmp( sSw6_FormatStt, aBuffer, 12 ) &&
               0 == strncmp( sSw6_FormatEnd, aBuffer + 12 + 1, 4 ))
        {
            rTypeName = ::rtl::OUString::createFromAscii( aFileTypeList[2].Type );
        }
    }
    if ( bCheckAll || aName.equalsAsciiL( aFileTypeList[3].Type, aFileTypeList[3].Length ) )
    {
        // writer_Lotus_1_2_3_10_DOS_StarWriter
        if ( 0 == aBuffer[0] && 0 == aBuffer[1] &&
                2 == aBuffer[2] && 0 == aBuffer[3] &&
                ( 4 == aBuffer[4] || 6 == aBuffer[4] ) && 4 == aBuffer[5])
        {
            rTypeName = ::rtl::OUString::createFromAscii( aFileTypeList[3].Type );
        }
    }
    if ( bCheckAll || aName.equalsAsciiL( aFileTypeList[4].Type, aFileTypeList[4].Length ) )
    {
        // writer_Lotus_1_2_3_10_WIN_StarWriter
        if ( 0 == aBuffer[0] && 0 == aBuffer[1] &&
                2 == aBuffer[2] && 0 == aBuffer[3] &&
                ( 4 == aBuffer[4] || 6 == aBuffer[4] ) && 4 == aBuffer[5])
        {
            rTypeName = ::rtl::OUString::createFromAscii( aFileTypeList[4].Type );
        }
    }
    //check for W4W at the end of the type name
    if( rURL.getLength() && (bCheckAll || aName.getLength() > 4 &&
                String(aName).Copy( xub_StrLen(aName.getLength()) - 4, 4 ).EqualsAscii( "_W4W" )))
//            aName.equalsAsciiL( aFileTypeList[5].Type, aFileTypeList[5].Length )) )
    {
        // W4W
        ::rtl::OUString sFileName = INetURLObject( rURL ).getFSysPath(INetURLObject::FSYS_DETECT);
        if(sFileName.getLength())
        {
            //Ami Pro: 0x021
            USHORT nVersion, nW4WId = AutoDetec( sFileName, nVersion );
            if(nW4WId > 1)
            {
                //find Id in aW4WIdentifier and compare to the appropriate filter name
                for(sal_Int8 nIdentifier = 0; nIdentifier < nW4WFilterCount; ++nIdentifier)
                {
                    if(aW4WIdentifier[nIdentifier] == nW4WId)
                    {
                        ::rtl::OUString sTemp = ::rtl::OUString::createFromAscii( aFileTypeList[nIdentifier + nW4WStartOffset].Type );
                        if(!aName.getLength() || aName == sTemp)
                        {
                            rTypeName = sTemp;
                            break;
                        }
                    }
                }
            }
        }
    }

    return (rTypeName.getLength() !=  0);
}

BinFilterDetect::BinFilterDetect( const Reference < ::com::sun::star::lang::XMultiServiceFactory >& xFactory )
    : m_xFact( xFactory )
{
}

BinFilterDetect::~BinFilterDetect()
{
}

::rtl::OUString SAL_CALL BinFilterDetect::detect( ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue >& lDescriptor ) throw( ::com::sun::star::uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );

    // create MediaDescriptor
    comphelper::MediaDescriptor aMedium( lDescriptor );

    // extract preselected type or filter
    ::rtl::OUString aTypeName, aPreselectedFilterName, aFilterName, aFileName;
    aTypeName   = aMedium.getUnpackedValueOrDefault(comphelper::MediaDescriptor::PROP_TYPENAME()  , ::rtl::OUString() );
    aFilterName = aMedium.getUnpackedValueOrDefault(comphelper::MediaDescriptor::PROP_FILTERNAME(), ::rtl::OUString() );
    aFileName   = aMedium.getUnpackedValueOrDefault(comphelper::MediaDescriptor::PROP_URL()       , ::rtl::OUString() );

    // open stream
    aMedium.addInputStream();
    Reference < com::sun::star::io::XInputStream > xInputStream = aMedium.getUnpackedValueOrDefault(comphelper::MediaDescriptor::PROP_INPUTSTREAM(), Reference < com::sun::star::io::XInputStream >() );

    // no detection without stream.
    // URL was used already to open stream ... so no further chance to get one.
    if ( ! xInputStream.is() )
        return ::rtl::OUString();

    try
    {
        // get type for preselected filter (will overwrite a possible TypeName)
        Reference< ::com::sun::star::container::XNameAccess> xFilters ( m_xFact->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.FilterFactory" ) ), UNO_QUERY_THROW );
        if ( aPreselectedFilterName.getLength() )
        {
            try
            {
                ::comphelper::SequenceAsHashMap( xFilters->getByName( aPreselectedFilterName ) )[DEFINE_CONST_UNICODE("Type")] >>= aTypeName;
            }
            catch ( com::sun::star::uno::Exception& )
            {
                // filter name might be wrong
                aPreselectedFilterName = ::rtl::OUString();
            }
        }

        // get ClipboardFormat from Storage
        SvStream* pStream = ::utl::UcbStreamHelper::CreateStream( xInputStream );
        SotStorageRef aStor = new SotStorage( pStream, FALSE );
        if ( !aStor->GetError() )
        {
            // check for the OLE-storage based formats
            sal_Int32 nClipId = aStor->GetFormat();
            ::rtl::OUString aFormatName = SotExchange::GetFormatName( nClipId );

            // check against preselected type
            ::rtl::OUString aSelectedFormatName;
            Reference< ::com::sun::star::container::XNameAccess > xTypes ( m_xFact->createInstance( DEFINE_CONST_UNICODE( "com.sun.star.document.TypeDetection" ) ), UNO_QUERY_THROW );
            if ( aTypeName.getLength() )
            {
                try
                {
                    ::comphelper::SequenceAsHashMap( xTypes->getByName( aTypeName ) )[DEFINE_CONST_UNICODE("ClipboardFormat")] >>= aSelectedFormatName;
                }
                catch ( com::sun::star::uno::Exception& )
                {
                    // type name might be wrong
                    aTypeName = ::rtl::OUString();
                }
            }

            // if preselected format doesn't fit check the other ones
            if ( aSelectedFormatName != aFormatName )
            {
                // no type detected so far because preselected one didn't match
                aTypeName = ::rtl::OUString();

                // check if template type is wanted (template and "normal" document share the MediaType)
                // a template type will be detected only if the file extension is "vor" (how else?!)
                INetURLObject aObj( aFileName );
                sal_Bool bIsTemplate = ( aObj.getExtension().equalsAscii("vor") );

                // get possible types
                Reference< ::com::sun::star::container::XContainerQuery > xTypeCFG ( xTypes, UNO_QUERY_THROW );
                Sequence < com::sun::star::beans::NamedValue > aSeq(1);
                aSeq[0].Name = ::rtl::OUString::createFromAscii("ClipboardFormat");
                aSeq[0].Value <<= aFormatName;
                Reference < com::sun::star::container::XEnumeration > xEnum = xTypeCFG->createSubSetEnumerationByProperties( aSeq );
                while ( xEnum->hasMoreElements() )
                {
                    Sequence < ::rtl::OUString > aExtensions;
                    ::comphelper::SequenceAsHashMap aHashedProps( xEnum->nextElement() );
                    aHashedProps[DEFINE_CONST_UNICODE("ClipboardFormat")] >>= aSelectedFormatName;
                    aHashedProps[DEFINE_CONST_UNICODE("Extensions")] >>= aExtensions;
                    if ( aSelectedFormatName == aFormatName )
                    {
                        // check wether this type specifies a template
                        sal_Bool bIsTemplateType = sal_False;
                        for ( sal_Int32 n=0; n<aExtensions.getLength(); n++ )
                            if ( aExtensions[n].equalsAscii("vor") )
                                bIsTemplateType = sal_True;

                        if ( bIsTemplateType != bIsTemplate )
                            // return it only if the file has the template extension (see comment above)
                            continue;

                        // get the type name from the properties
                        aHashedProps[DEFINE_CONST_UNICODE("Name")] >>= aTypeName;
                        break;
                    }
                }
            }
            else
                aFilterName = aPreselectedFilterName;
        }
        else
        {
            // check for the stream based formats
            if ( checkFormat( pStream, aTypeName, aFileName, FALSE ) )
                aFilterName = aPreselectedFilterName;
            else
                checkFormat( pStream, aTypeName, aFileName, TRUE );
        }

        DELETEZ( pStream );
    }
    catch ( com::sun::star::uno::Exception &)
    {
        // in case something goes wrong with type detection or filter configuration
       aTypeName = ::rtl::OUString();
    }

    /* #i71829# Dont search for any suitable filter during deep detection.
                A detection service is used to find a suitable type - not filter.
                Search for a filter is done outside ... recognizing preferred flags;
                user preselection and some other things too.
    */

    // only return a type name if it matches to the valid ones
    // Because stripped binfilter detect EVERY format, which was known before
    // code was changed ... it can happen that typename represent a format not realy
    // supported by binfilter component.
    sal_Bool bSupportedType = sal_False;
    if ( aTypeName.getLength() )
    {
        sal_Int32 i = 0;
        sal_Int32 c = nFileTypeCount;
        for (i=0; i<c; i++)
        {
            if ( aTypeName.equalsAsciiL( aFileTypeList[i].Type, aFileTypeList[i].Length ) )
            {
                bSupportedType = true;
                break;
            }
        }
    }

    if ( bSupportedType )
        aMedium[comphelper::MediaDescriptor::PROP_TYPENAME()] <<= aTypeName;
    else
        aTypeName = ::rtl::OUString();

    lDescriptor = aMedium.getAsConstPropertyValueList();
    
    return aTypeName;
}

SFX_IMPL_SINGLEFACTORY( BinFilterDetect )

/* XServiceInfo */
UNOOUSTRING SAL_CALL BinFilterDetect::getImplementationName() throw( UNORUNTIMEEXCEPTION )
{
    return impl_getStaticImplementationName();
}
                                                                                                                                \
/* XServiceInfo */
sal_Bool SAL_CALL BinFilterDetect::supportsService( const UNOOUSTRING& sServiceName ) throw( UNORUNTIMEEXCEPTION )
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
UNOSEQUENCE< UNOOUSTRING > SAL_CALL BinFilterDetect::getSupportedServiceNames() throw( UNORUNTIMEEXCEPTION )
{
    return impl_getStaticSupportedServiceNames();
}

/* Helper for XServiceInfo */
UNOSEQUENCE< UNOOUSTRING > BinFilterDetect::impl_getStaticSupportedServiceNames()
{
    UNOMUTEXGUARD aGuard( UNOMUTEX::getGlobalMutex() );
    UNOSEQUENCE< UNOOUSTRING > seqServiceNames( 1 );
    seqServiceNames.getArray() [0] = UNOOUSTRING::createFromAscii( "com.sun.star.frame.ExtendedTypeDetection"  );
    return seqServiceNames ;
}

/* Helper for XServiceInfo */
UNOOUSTRING BinFilterDetect::impl_getStaticImplementationName()
{
    return UNOOUSTRING::createFromAscii( "com.sun.star.comp.sfx2.BinaryFormatDetector" );
}

/* Helper for registry */
UNOREFERENCE< UNOXINTERFACE > SAL_CALL BinFilterDetect::impl_createInstance( const UNOREFERENCE< UNOXMULTISERVICEFACTORY >& xServiceManager ) throw( UNOEXCEPTION )
{
    return UNOREFERENCE< UNOXINTERFACE >( *new BinFilterDetect( xServiceManager ) );
}

}
extern "C" {

void SAL_CALL component_getImplementationEnvironment(	const	sal_Char**			ppEnvironmentTypeName	,
                                                                uno_Environment**	ppEnvironment			)
{
    *ppEnvironmentTypeName = CPPU_CURRENT_LANGUAGE_BINDING_NAME ;
}

sal_Bool SAL_CALL component_writeInfo(	void*	pServiceManager	,
                                        void*	pRegistryKey	)
{
    ::com::sun::star::uno::Reference< XRegistryKey >	 xKey( reinterpret_cast< XRegistryKey* >( pRegistryKey ) )	;

    // Eigentliche Implementierung und ihre Services registrieren
    ::rtl::OUString aImpl;
    ::rtl::OUString aTempStr;
    ::rtl::OUString aKeyStr;
    REFERENCE< XRegistryKey > xNewKey;
    REFERENCE< XRegistryKey > xLoaderKey;

    // frame loader
    aImpl = ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/"));
    aImpl += ::binfilter::BinFilterDetect::impl_getStaticImplementationName();

    aTempStr = aImpl;
    aTempStr += ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("/UNO/SERVICES"));
    xNewKey = xKey->createKey( aTempStr );
    Sequence < ::rtl::OUString > aServices = ::binfilter::BinFilterDetect::impl_getStaticSupportedServiceNames();
    sal_Int32 nCount = aServices.getLength();
    for ( sal_Int16 i=0; i<nCount; i++ )
        xNewKey->createKey( aServices.getConstArray()[i] );

    return sal_True;
}

void* SAL_CALL component_getFactory(	const	sal_Char*	pImplementationName	,
                                                void*		pServiceManager		,
                                                void*		pRegistryKey		)
{
    // Set default return value for this operation - if it failed.
    void* pReturn = NULL ;

    if	(
            ( pImplementationName	!=	NULL ) &&
            ( pServiceManager		!=	NULL )
        )
    {
        // Define variables which are used in following macros.
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XSingleServiceFactory >	xFactory																								;
        ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory >	xServiceManager( reinterpret_cast< ::com::sun::star::lang::XMultiServiceFactory* >( pServiceManager ) )	;

        //=============================================================================
        //  Add new macro line to handle new service.
        //
        //	!!! ATTENTION !!!
        //		Write no ";" at end of line and dont forget "else" ! (see macro)
        //=============================================================================
        IF_NAME_CREATECOMPONENTFACTORY( ::binfilter::BinFilterDetect )

        // Factory is valid - service was found.
        if ( xFactory.is() )
        {
            xFactory->acquire();
            pReturn = xFactory.get();
        }
    }
    // Return with result of this operation.
    return pReturn ;
}
} // extern "C"


