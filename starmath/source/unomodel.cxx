/*************************************************************************
 *
 *  $RCSfile: unomodel.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:57:27 $
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

#pragma hdrstop

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _SFX_PRINTER_HXX
#include <sfx2/printer.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _CTRLTOOL_HXX
#include <svtools/ctrltool.hxx>
#endif
#ifndef _SFX_ITEMPROP_HXX
#include <svtools/itemprop.hxx>
#endif
#ifndef UNOMODEL_HXX
#include "unomodel.hxx"
#endif
#ifndef DOCUMENT_HXX
#include "document.hxx"
#endif

using namespace vos;
using namespace rtl;
using namespace ::com::sun::star;

#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)    ((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))

#define C2U(cChar)  rtl::OUString::createFromAscii(cChar)

////////////////////////////////////////
//
// class SmModel
//
#define PROPERTY_NONE 0

#define FID_FORMULA                                 0
#define FID_FONT_NAME_VARIABLES                     1
#define FID_FONT_NAME_FUNCTIONS                     2
#define FID_FONT_NAME_NUMBERS                       3
#define FID_FONT_NAME_TEXT                          4
#define FID_CUSTOM_FONT_NAME_SERIF                  5
#define FID_CUSTOM_FONT_NAME_SANS                   6
#define FID_CUSTOM_FONT_NAME_FIXED                  7
#define FID_CUSTOM_FONT_FIXED_POSTURE               8
#define FID_CUSTOM_FONT_FIXED_WEIGHT                9
#define FID_CUSTOM_FONT_SANS_POSTURE                10
#define FID_CUSTOM_FONT_SANS_WEIGHT                 11
#define FID_CUSTOM_FONT_SERIF_POSTURE               12
#define FID_CUSTOM_FONT_SERIF_WEIGHT                13
#define FID_FONT_VARIABLES_POSTURE                  14
#define FID_FONT_VARIABLES_WEIGHT                   15
#define FID_FONT_FUNCTIONS_POSTURE                  16
#define FID_FONT_FUNCTIONS_WEIGHT                   17
#define FID_FONT_NUMBERS_POSTURE                    18
#define FID_FONT_NUMBERS_WEIGHT                     19
#define FID_FONT_TEXT_POSTURE                       20
#define FID_FONT_TEXT_WEIGHT                        21
#define FID_BASE_FONT_HEIGHT                        22
#define FID_RELATIVE_FONT_HEIGHT_TEXT               23
#define FID_RELATIVE_FONT_HEIGHT_INDICES            24
#define FID_RELATIVE_FONT_HEIGHT_FUNCTIONS          25
#define FID_RELATIVE_FONT_HEIGHT_OPERATORS          26
#define FID_RELATIVE_FONT_HEIGHT_LIMITS             27
#define FID_IS_TEXT_MODE                            28
#define FID_ALIGNMENT                               29
#define FID_RELATIVE_SPACING                        30
#define FID_RELATIVE_LINE_SPACING                   31
#define FID_RELATIVE_ROOT_SPACING                   32
#define FID_RELATIVE_INDEX_SUPERSCRIPT              33
#define FID_RELATIVE_INDEX_SUBSCRIPT                34
#define FID_RELATIVE_FRACTION_NUMERATOR_HEIGHT      35
#define FID_RELATIVE_FRACTION_DENOMINATOR_DEPTH     36
#define FID_RELATIVE_FRACTION_BAR_EXCESS_LENGTH     37
#define FID_RELATIVE_FRACTION_BAR_LINE_WEIGHT       38
#define FID_RELATIVE_UPPER_LIMIT_DISTANCE           39
#define FID_RELATIVE_LOWER_LIMIT_DISTANCE           40
#define FID_RELATIVE_BRACKET_EXCESS_SIZE            41
#define FID_RELATIVE_BRACKET_DISTANCE               42
#define FID_IS_SCALE_ALL_BRACKETS                   43
#define FID_RELATIVE_SCALE_BRACKET_EXCESS_SIZE      44
#define FID_RELATIVE_MATRIX_LINE_SPACING            45
#define FID_RELATIVE_MATRIX_COLUMN_SPACING          46
#define FID_RELATIVE_SYMBOL_PRIMARY_HEIGHT          47
#define FID_RELATIVE_SYMBOL_MINIMUM_HEIGHT          48
#define FID_RELATIVE_OPERATOR_EXCESS_SIZE           49
#define FID_RELATIVE_OPERATOR_SPACING               50
#define FID_LEFT_MARGIN                             51
#define FID_RIGHT_MARGIN                            52
#define FID_TOP_MARGIN                              53
#define FID_BOTTOM_MARGIN                           54

#define UNO_NAME_FORMULA                                "Formula"
#define UNO_NAME_FONT_NAME_VARIABLES                    "FontNameVariables"
#define UNO_NAME_CUSTOM_FONT_VARIABLES_IS_ITALIC        "FontVariablesIsItalic"
#define UNO_NAME_CUSTOM_FONT_VARIABLES_IS_BOLD          "FontVariablesIsBold"
#define UNO_NAME_FONT_NAME_FUNCTIONS                    "FontNameFunctions"
#define UNO_NAME_CUSTOM_FONT_FUNCTIONS_IS_ITALIC        "FontFunctionsIsItalic"
#define UNO_NAME_CUSTOM_FONT_FUNCTIONS_IS_BOLD          "FontFunctionsIsBold"
#define UNO_NAME_FONT_NAME_NUMBERS                      "FontNameNumbers"
#define UNO_NAME_CUSTOM_FONT_NUMBERS_IS_ITALIC          "FontNumbersIsItalic"
#define UNO_NAME_CUSTOM_FONT_NUMBERS_IS_BOLD            "FontNumbersIsBold"
#define UNO_NAME_FONT_NAME_TEXT                         "FontNameText"
#define UNO_NAME_CUSTOM_FONT_TEXT_IS_ITALIC             "FontTextIsItalic"
#define UNO_NAME_CUSTOM_FONT_TEXT_IS_BOLD               "FontTextIsBold"
#define UNO_NAME_CUSTOM_FONT_NAME_SERIF                 "CustomFontNameSerif"
#define UNO_NAME_CUSTOM_FONT_SERIF_IS_ITALIC            "FontSerifIsItalic"
#define UNO_NAME_CUSTOM_FONT_SERIF_IS_BOLD              "FontSerifIsBold"
#define UNO_NAME_CUSTOM_FONT_NAME_SANS                  "CustomFontNameSans"
#define UNO_NAME_CUSTOM_FONT_SANS_IS_ITALIC             "FontSansIsItalic"
#define UNO_NAME_CUSTOM_FONT_SANS_IS_BOLD               "FontSansIsBold"
#define UNO_NAME_CUSTOM_FONT_NAME_FIXED                 "CustomFontNameFixed"
#define UNO_NAME_CUSTOM_FONT_FIXED_IS_ITALIC            "FontFixedIsItalic"
#define UNO_NAME_CUSTOM_FONT_FIXED_IS_BOLD              "FontFixedIsBold"
#define UNO_NAME_BASE_FONT_HEIGHT                       "BaseFontHeight"
#define UNO_NAME_RELATIVE_FONT_HEIGHT_TEXT              "RelativeFontHeightText"
#define UNO_NAME_RELATIVE_FONT_HEIGHT_INDICES           "RelativeFontHeightIndices"
#define UNO_NAME_RELATIVE_FONT_HEIGHT_FUNCTIONS         "RelativeFontHeightFunctions"
#define UNO_NAME_RELATIVE_FONT_HEIGHT_OPERATORS         "RelativeFontHeightOperators"
#define UNO_NAME_RELATIVE_FONT_HEIGHT_LIMITS            "RelativeFontHeightLimits"
#define UNO_NAME_IS_TEXT_MODE                           "IsTextMode"
#define UNO_NAME_ALIGNMENT                              "Alignment"
#define UNO_NAME_RELATIVE_SPACING                       "RelativeSpacing"
#define UNO_NAME_RELATIVE_LINE_SPACING                  "RelativeLineSpacing"
#define UNO_NAME_RELATIVE_ROOT_SPACING                  "RelativeRootSpacing"
#define UNO_NAME_RELATIVE_INDEX_SUPERSCRIPT             "RelativeIndexSuperscript"
#define UNO_NAME_RELATIVE_INDEX_SUBSCRIPT               "RelativeIndexSubscript"
#define UNO_NAME_RELATIVE_FRACTION_NUMERATOR_HEIGHT     "RelativeFractionNumeratorHeight"
#define UNO_NAME_RELATIVE_FRACTION_DENOMINATOR_DEPTH    "RelativeFractionDenominatorDepth"
#define UNO_NAME_RELATIVE_FRACTION_BAR_EXCESS_LENGTH    "RelativeFractionBarExcessLength"
#define UNO_NAME_RELATIVE_FRACTION_BAR_LINE_WEIGHT      "RelativeFractionBarLineWeight"
#define UNO_NAME_RELATIVE_UPPER_LIMIT_DISTANCE          "RelativeUpperLimitDistance"
#define UNO_NAME_RELATIVE_LOWER_LIMIT_DISTANCE          "RelativeLowerLimitDistance"
#define UNO_NAME_RELATIVE_BRACKET_EXCESS_SIZE           "RelativeBracketExcessSize"
#define UNO_NAME_RELATIVE_BRACKET_DISTANCE              "RelativeBracketDistance"
#define UNO_NAME_IS_SCALE_ALL_BRACKETS                  "IsScaleAllBrackets"
#define UNO_NAME_RELATIVE_SCALE_BRACKET_EXCESS_SIZE     "RelativeScaleBracketExcessSize"
#define UNO_NAME_RELATIVE_MATRIX_LINE_SPACING           "RelativeMatrixLineSpacing"
#define UNO_NAME_RELATIVE_MATRIX_COLUMN_SPACING         "RelativeMatrixColumnSpacing"
#define UNO_NAME_RELATIVE_SYMBOL_PRIMARY_HEIGHT         "RelativeSymbolPrimaryHeight"
#define UNO_NAME_RELATIVE_SYMBOL_MINIMUM_HEIGHT         "RelativeSymbolMinimumHeight"
#define UNO_NAME_RELATIVE_OPERATOR_EXCESS_SIZE          "RelativeOperatorExcessSize"
#define UNO_NAME_RELATIVE_OPERATOR_SPACING              "RelativeOperatorSpacing"
#define UNO_NAME_LEFT_MARGIN                            "LeftMargin"
#define UNO_NAME_RIGHT_MARGIN                           "RightMargin"
#define UNO_NAME_TOP_MARGIN                             "TopMargin"
#define UNO_NAME_BOTTOM_MARGIN                          "BottomMargin"


//must be sorted by names!
SfxItemPropertyMap aMathModelMap_Impl[] =
{
    { MAP_CHAR_LEN(UNO_NAME_ALIGNMENT                          ), FID_ALIGNMENT                          ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, 0},
    { MAP_CHAR_LEN(UNO_NAME_BASE_FONT_HEIGHT                   ), FID_BASE_FONT_HEIGHT                   ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, 0},
    { MAP_CHAR_LEN(UNO_NAME_BOTTOM_MARGIN                     ), FID_BOTTOM_MARGIN                         ,        &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_BOTTOMSPACE             },

    { MAP_CHAR_LEN(UNO_NAME_CUSTOM_FONT_NAME_FIXED             ), FID_CUSTOM_FONT_NAME_FIXED             ,      &::getCppuType((const OUString*)0),     PROPERTY_NONE, FNT_FIXED          },
    { MAP_CHAR_LEN(UNO_NAME_CUSTOM_FONT_FIXED_IS_ITALIC), FID_CUSTOM_FONT_FIXED_POSTURE   ,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_FIXED},
    { MAP_CHAR_LEN(UNO_NAME_CUSTOM_FONT_FIXED_IS_BOLD),   FID_CUSTOM_FONT_FIXED_WEIGHT    ,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_FIXED},

    { MAP_CHAR_LEN(UNO_NAME_CUSTOM_FONT_NAME_SANS              ), FID_CUSTOM_FONT_NAME_SANS              ,      &::getCppuType((const OUString*)0),     PROPERTY_NONE, FNT_SANS        },
    { MAP_CHAR_LEN(UNO_NAME_CUSTOM_FONT_SANS_IS_ITALIC),   FID_CUSTOM_FONT_SANS_POSTURE   ,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_SANS},
    { MAP_CHAR_LEN(UNO_NAME_CUSTOM_FONT_SANS_IS_BOLD),   FID_CUSTOM_FONT_SANS_WEIGHT    ,  &::getBooleanCppuType(),             PROPERTY_NONE, FNT_SANS},

    { MAP_CHAR_LEN(UNO_NAME_CUSTOM_FONT_NAME_SERIF             ), FID_CUSTOM_FONT_NAME_SERIF             ,      &::getCppuType((const OUString*)0),     PROPERTY_NONE, FNT_SERIF          },
    { MAP_CHAR_LEN(UNO_NAME_CUSTOM_FONT_SERIF_IS_ITALIC),   FID_CUSTOM_FONT_SERIF_POSTURE   ,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_SERIF},
    { MAP_CHAR_LEN(UNO_NAME_CUSTOM_FONT_SERIF_IS_BOLD),  FID_CUSTOM_FONT_SERIF_WEIGHT    ,  &::getBooleanCppuType(),            PROPERTY_NONE,  FNT_SERIF},

    { MAP_CHAR_LEN(UNO_NAME_FONT_NAME_FUNCTIONS                ), FID_FONT_NAME_FUNCTIONS                ,      &::getCppuType((const OUString*)0),     PROPERTY_NONE, FNT_FUNCTION },
    { MAP_CHAR_LEN(UNO_NAME_CUSTOM_FONT_FUNCTIONS_IS_ITALIC),   FID_FONT_FUNCTIONS_POSTURE   ,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_FUNCTION},
    { MAP_CHAR_LEN(UNO_NAME_CUSTOM_FONT_FUNCTIONS_IS_BOLD),  FID_FONT_FUNCTIONS_WEIGHT    ,  &::getBooleanCppuType(),           PROPERTY_NONE, FNT_FUNCTION},

    { MAP_CHAR_LEN(UNO_NAME_FONT_NAME_NUMBERS                  ), FID_FONT_NAME_NUMBERS                  ,      &::getCppuType((const OUString*)0),     PROPERTY_NONE, FNT_NUMBER        },
    { MAP_CHAR_LEN(UNO_NAME_CUSTOM_FONT_NUMBERS_IS_ITALIC),   FID_FONT_NUMBERS_POSTURE   ,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_NUMBER},
    { MAP_CHAR_LEN(UNO_NAME_CUSTOM_FONT_NUMBERS_IS_BOLD),    FID_FONT_NUMBERS_WEIGHT    ,  &::getBooleanCppuType(),             PROPERTY_NONE, FNT_NUMBER},

    { MAP_CHAR_LEN(UNO_NAME_FONT_NAME_TEXT                     ), FID_FONT_NAME_TEXT                     ,      &::getCppuType((const OUString*)0),     PROPERTY_NONE, FNT_TEXT        },
    { MAP_CHAR_LEN(UNO_NAME_CUSTOM_FONT_TEXT_IS_ITALIC),   FID_FONT_TEXT_POSTURE   ,  &::getBooleanCppuType(),  PROPERTY_NONE, },
    { MAP_CHAR_LEN(UNO_NAME_CUSTOM_FONT_TEXT_IS_BOLD),   FID_FONT_TEXT_WEIGHT    ,  &::getBooleanCppuType(),            PROPERTY_NONE, },

    { MAP_CHAR_LEN(UNO_NAME_FONT_NAME_VARIABLES                ), FID_FONT_NAME_VARIABLES                ,      &::getCppuType((const OUString*)0),     PROPERTY_NONE, FNT_VARIABLE },
    { MAP_CHAR_LEN(UNO_NAME_CUSTOM_FONT_VARIABLES_IS_ITALIC),   FID_FONT_VARIABLES_POSTURE,  &::getBooleanCppuType(),  PROPERTY_NONE, },
    { MAP_CHAR_LEN(UNO_NAME_CUSTOM_FONT_VARIABLES_IS_BOLD),  FID_FONT_VARIABLES_WEIGHT    ,  &::getBooleanCppuType(),           PROPERTY_NONE, },

    { MAP_CHAR_LEN(UNO_NAME_FORMULA                           ),    FID_FORMULA                            ,        &::getCppuType((const OUString*)0),     PROPERTY_NONE, 0},
    { MAP_CHAR_LEN(UNO_NAME_IS_SCALE_ALL_BRACKETS              ), FID_IS_SCALE_ALL_BRACKETS              ,      &::getBooleanCppuType(),    PROPERTY_NONE, 0},
    { MAP_CHAR_LEN(UNO_NAME_IS_TEXT_MODE                       ), FID_IS_TEXT_MODE                       ,      &::getBooleanCppuType(),    PROPERTY_NONE, 0},
    { MAP_CHAR_LEN(UNO_NAME_LEFT_MARGIN                       ), FID_LEFT_MARGIN                           ,        &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_LEFTSPACE                 },
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_BRACKET_DISTANCE          ), FID_RELATIVE_BRACKET_DISTANCE          ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_BRACKETSPACE },
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_BRACKET_EXCESS_SIZE       ), FID_RELATIVE_BRACKET_EXCESS_SIZE       ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_BRACKETSIZE  },
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_FONT_HEIGHT_FUNCTIONS     ), FID_RELATIVE_FONT_HEIGHT_FUNCTIONS     ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, SIZ_FUNCTION},
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_FONT_HEIGHT_INDICES       ), FID_RELATIVE_FONT_HEIGHT_INDICES       ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, SIZ_INDEX      },
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_FONT_HEIGHT_LIMITS        ), FID_RELATIVE_FONT_HEIGHT_LIMITS        ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, SIZ_LIMITS    },
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_FONT_HEIGHT_OPERATORS     ), FID_RELATIVE_FONT_HEIGHT_OPERATORS     ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, SIZ_OPERATOR},
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_FONT_HEIGHT_TEXT             ), FID_RELATIVE_FONT_HEIGHT_TEXT          ,       &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, SIZ_TEXT   },
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_FRACTION_BAR_EXCESS_LENGTH), FID_RELATIVE_FRACTION_BAR_EXCESS_LENGTH,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_FRACTION        },
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_FRACTION_BAR_LINE_WEIGHT  ), FID_RELATIVE_FRACTION_BAR_LINE_WEIGHT  ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_STROKEWIDTH  },
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_FRACTION_DENOMINATOR_DEPTH), FID_RELATIVE_FRACTION_DENOMINATOR_DEPTH,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_DENOMINATOR  },
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_FRACTION_NUMERATOR_HEIGHT ), FID_RELATIVE_FRACTION_NUMERATOR_HEIGHT ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_NUMERATOR          },
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_INDEX_SUBSCRIPT           ), FID_RELATIVE_INDEX_SUBSCRIPT           ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_SUBSCRIPT          },
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_INDEX_SUPERSCRIPT         ), FID_RELATIVE_INDEX_SUPERSCRIPT         ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_SUPERSCRIPT  },
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_LINE_SPACING              ), FID_RELATIVE_LINE_SPACING              ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_VERTICAL        },
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_LOWER_LIMIT_DISTANCE      ), FID_RELATIVE_LOWER_LIMIT_DISTANCE      ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_LOWERLIMIT        },
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_MATRIX_COLUMN_SPACING     ), FID_RELATIVE_MATRIX_COLUMN_SPACING     ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_MATRIXCOL},
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_MATRIX_LINE_SPACING       ), FID_RELATIVE_MATRIX_LINE_SPACING       ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_MATRIXROW},
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_OPERATOR_EXCESS_SIZE      ), FID_RELATIVE_OPERATOR_EXCESS_SIZE      ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_OPERATORSIZE        },
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_OPERATOR_SPACING          ), FID_RELATIVE_OPERATOR_SPACING          ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_OPERATORSPACE},
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_ROOT_SPACING              ), FID_RELATIVE_ROOT_SPACING              ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_ROOT               },
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_SCALE_BRACKET_EXCESS_SIZE ), FID_RELATIVE_SCALE_BRACKET_EXCESS_SIZE ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_NORMALBRACKETSIZE},
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_SPACING                   ), FID_RELATIVE_SPACING                   ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_HORIZONTAL        },
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_SYMBOL_MINIMUM_HEIGHT     ), FID_RELATIVE_SYMBOL_MINIMUM_HEIGHT     ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_ORNAMENTSPACE         },
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_SYMBOL_PRIMARY_HEIGHT     ), FID_RELATIVE_SYMBOL_PRIMARY_HEIGHT     ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_ORNAMENTSIZE        },
    { MAP_CHAR_LEN(UNO_NAME_RELATIVE_UPPER_LIMIT_DISTANCE      ), FID_RELATIVE_UPPER_LIMIT_DISTANCE      ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_UPPERLIMIT        },
    { MAP_CHAR_LEN(UNO_NAME_RIGHT_MARGIN                      ), FID_RIGHT_MARGIN                      ,        &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_RIGHTSPACE               },
    { MAP_CHAR_LEN(UNO_NAME_TOP_MARGIN                        ), FID_TOP_MARGIN                        ,        &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_TOPSPACE               },
    {0,0,0,0}
};
//-----------------------------------------------------------------------
SmModel::SmModel( SfxObjectShell *pObjSh ) :
    SfxBaseModel(pObjSh),
    _pMap(aMathModelMap_Impl)
{
}
//-----------------------------------------------------------------------
SmModel::~SmModel()
{
}
/*-- 28.03.00 14:18:17---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SAL_CALL SmModel::queryInterface( const uno::Type& rType ) throw(uno::RuntimeException)
{
    uno::Any aRet = SmModelBaseClass::queryInterface(rType);
    if(!aRet.hasValue())
    {
        aRet = SfxBaseModel::queryInterface(rType);
    }
    return aRet;
}
/*-- 28.03.00 14:18:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SmModel::acquire() throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    SmModelBaseClass::acquire();
}
/*-- 28.03.00 14:18:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SmModel::release() throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    SmModelBaseClass::release();
}
/*-- 28.03.00 14:18:19---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< uno::Type > SAL_CALL SmModel::getTypes(  ) throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());

    uno::Sequence< uno::Type > aBaseTypes = SmModelBaseClass::getTypes();
    uno::Sequence< uno::Type > aSfxTypes = SfxBaseModel::getTypes();
    long nIndex = aBaseTypes.getLength();
    aBaseTypes.realloc(aBaseTypes.getLength() + aSfxTypes.getLength());
    uno::Type* pBaseTypes = aBaseTypes.getArray();
    const uno::Type* pSfxTypes = aSfxTypes.getConstArray();
    for(long nSfx = 0; nSfx < aSfxTypes.getLength(); nSfx++)
        pBaseTypes[nIndex++] = pSfxTypes[nSfx];
    return aBaseTypes;
}
/* -----------------------------28.03.00 14:23--------------------------------

 ---------------------------------------------------------------------------*/
const uno::Sequence< sal_Int8 > & SmModel::getUnoTunnelId()
{
    static osl::Mutex aCreateMutex;
    osl::Guard<osl::Mutex> aGuard( aCreateMutex );

    static uno::Sequence< sal_Int8 > aSeq;
    if(!aSeq.getLength())
    {
        aSeq.realloc( 16 );
        rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0, sal_True );
    }
    return aSeq;
}
/* -----------------------------28.03.00 14:23--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SmModel::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return (sal_Int64)this;
    }
    return 0;
}

/*-- 07.01.00 16:32:59---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Reference< beans::XPropertySetInfo >  SAL_CALL SmModel::getPropertySetInfo(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());

    static uno::Reference< beans::XPropertySetInfo >  xRet = new SfxItemPropertySetInfo( _pMap );
    return xRet;
}
/*-- 07.01.00 16:33:00---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Int16 lcl_AnyToINT16(const uno::Any& rAny)
{
    uno::TypeClass eType = rAny.getValueType().getTypeClass();

    sal_Int16 nRet = 0;
    if( eType == uno::TypeClass_DOUBLE )
        nRet = (sal_Int16)*(double*)rAny.getValue();
    else if( eType == uno::TypeClass_FLOAT )
        nRet = (sal_Int16)*(float*)rAny.getValue();
    else
        rAny >>= nRet;
    return nRet;
}
//-----------------------------------------------------------------------------
void SmModel::setPropertyValue(const OUString& rPropertyName, const uno::Any& aValue)
    throw( beans::UnknownPropertyException, beans::PropertyVetoException,
         lang::IllegalArgumentException, lang::WrappedTargetException, uno::RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());

    SfxObjectShell* pObjShell = GetObjectShell();
    if(!pObjShell)
        throw uno::RuntimeException();
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                        _pMap, rPropertyName);
    if(pMap)
    {
        SmFormat& aFormat = ((SmDocShell*)pObjShell)->GetFormat();
        switch(pMap->nWID)
        {
            case  FID_FORMULA:
            {
                OUString aText;
                aValue >>= aText;
                ((SmDocShell*)pObjShell)->SetText(aText);
            }
            break;
            case FID_FONT_NAME_VARIABLES                :
            case FID_FONT_NAME_FUNCTIONS                :
            case FID_FONT_NAME_NUMBERS                  :
            case FID_FONT_NAME_TEXT                     :
            case FID_CUSTOM_FONT_NAME_SERIF             :
            case FID_CUSTOM_FONT_NAME_SANS              :
            case FID_CUSTOM_FONT_NAME_FIXED             :
            {
                OUString aText;
                aValue >>= aText;
                String sFontName = aText;
                if(!sFontName.Len())
                    throw lang::IllegalArgumentException();
                if(aFormat.GetFont(pMap->nMemberId).GetName() != sFontName)
                {
                    OutputDevice *pDev = ((SmDocShell*)pObjShell)->GetPrinter();
                    if (!pDev || pDev->GetDevFontCount() == 0)
                        pDev = (OutputDevice *) GetpApp()->GetDefaultDevice();

                    FontList aFontList(pDev);

                    sal_uInt16  nCount = aFontList.GetFontNameCount();
                    sal_Bool bSet = sal_False;
                    for (sal_uInt16 i = 0;  i < nCount;  i++)
                    {
                        const FontInfo& rInfo = aFontList.GetFontName( i );
                        if(rInfo.GetName() == sFontName)
                        {
                            SmFace aSet(rInfo);
                            const SmFace rOld = aFormat.GetFont(pMap->nMemberId);
                            aSet.SetBorderWidth(rOld.GetBorderWidth());
                            aSet.SetSize(rOld.GetSize());
                            aSet.SetAlign(ALIGN_BASELINE);
                            aFormat.SetFont(pMap->nMemberId, aSet);
                            bSet = sal_True;
                            break;
                        }
                    }
                    if(!bSet)
                        throw lang::IllegalArgumentException();
                }
            }
            break;
            case FID_CUSTOM_FONT_FIXED_POSTURE:
            case FID_CUSTOM_FONT_SANS_POSTURE :
            case FID_CUSTOM_FONT_SERIF_POSTURE:
            case FID_FONT_VARIABLES_POSTURE   :
            case FID_FONT_FUNCTIONS_POSTURE   :
            case FID_FONT_NUMBERS_POSTURE     :
            case FID_FONT_TEXT_POSTURE        :
            {
                if(aValue.getValueType() != ::getBooleanCppuType())
                    throw lang::IllegalArgumentException();
                BOOL bVal = *(sal_Bool*)aValue.getValue();
                Font aNewFont(aFormat.GetFont(pMap->nMemberId));
                aNewFont.SetItalic((bVal) ? ITALIC_NORMAL : ITALIC_NONE);
                aFormat.SetFont(pMap->nMemberId, aNewFont);
            }
            break;
            case FID_CUSTOM_FONT_FIXED_WEIGHT :
            case FID_CUSTOM_FONT_SANS_WEIGHT  :
            case FID_CUSTOM_FONT_SERIF_WEIGHT :
            case FID_FONT_VARIABLES_WEIGHT    :
            case FID_FONT_FUNCTIONS_WEIGHT    :
            case FID_FONT_NUMBERS_WEIGHT      :
            case FID_FONT_TEXT_WEIGHT         :
            {
                if(aValue.getValueType() != ::getBooleanCppuType())
                    throw lang::IllegalArgumentException();
                BOOL bVal = *(sal_Bool*)aValue.getValue();
                Font aNewFont(aFormat.GetFont(pMap->nMemberId));
                aNewFont.SetWeight((bVal) ? WEIGHT_BOLD : WEIGHT_NORMAL);
                aFormat.SetFont(pMap->nMemberId, aNewFont);
            }
            break;
            case FID_BASE_FONT_HEIGHT                   :
            {
                // Point!
                sal_Int16 nVal = lcl_AnyToINT16(aValue);
                if(nVal < 1)
                    throw lang::IllegalArgumentException();
                Size aSize = aFormat.GetBaseSize();
                nVal *= 20;
                nVal = TWIP_TO_MM100(nVal);
                aSize.Height() = nVal;
                aFormat.SetBaseSize(aSize);
            }
            break;
            case FID_RELATIVE_FONT_HEIGHT_TEXT          :
            case FID_RELATIVE_FONT_HEIGHT_INDICES       :
            case FID_RELATIVE_FONT_HEIGHT_FUNCTIONS     :
            case FID_RELATIVE_FONT_HEIGHT_OPERATORS     :
            case FID_RELATIVE_FONT_HEIGHT_LIMITS        :
            {
                sal_Int16 nVal;
                aValue >>= nVal;
                if(nVal < 1)
                    throw lang::IllegalArgumentException();
                aFormat.SetRelSize(pMap->nMemberId, nVal);
            }
            break;

            case FID_IS_TEXT_MODE                       :
            {
                aFormat.SetTextmode(*(sal_Bool*)aValue.getValue());
            }
            break;

            case FID_ALIGNMENT                          :
            {
                // SmHorAlign uses the same values as HorizontalAlignment
                sal_Int16 nVal;
                aValue >>= nVal;
                if(nVal < 0 || nVal > 2)
                    throw lang::IllegalArgumentException();
                aFormat.SetHorAlign((SmHorAlign)nVal);
            }
            break;

            case FID_RELATIVE_SPACING                   :
            case FID_RELATIVE_LINE_SPACING              :
            case FID_RELATIVE_ROOT_SPACING              :
            case FID_RELATIVE_INDEX_SUPERSCRIPT         :
            case FID_RELATIVE_INDEX_SUBSCRIPT           :
            case FID_RELATIVE_FRACTION_NUMERATOR_HEIGHT :
            case FID_RELATIVE_FRACTION_DENOMINATOR_DEPTH:
            case FID_RELATIVE_FRACTION_BAR_EXCESS_LENGTH:
            case FID_RELATIVE_FRACTION_BAR_LINE_WEIGHT  :
            case FID_RELATIVE_UPPER_LIMIT_DISTANCE      :
            case FID_RELATIVE_LOWER_LIMIT_DISTANCE      :
            case FID_RELATIVE_BRACKET_EXCESS_SIZE       :
            case FID_RELATIVE_BRACKET_DISTANCE          :
            case FID_RELATIVE_SCALE_BRACKET_EXCESS_SIZE :
            case FID_RELATIVE_MATRIX_LINE_SPACING       :
            case FID_RELATIVE_MATRIX_COLUMN_SPACING     :
            case FID_RELATIVE_SYMBOL_PRIMARY_HEIGHT     :
            case FID_RELATIVE_SYMBOL_MINIMUM_HEIGHT     :
            case FID_RELATIVE_OPERATOR_EXCESS_SIZE      :
            case FID_RELATIVE_OPERATOR_SPACING          :
            case FID_LEFT_MARGIN               :
            case FID_RIGHT_MARGIN              :
            case FID_TOP_MARGIN                :
            case FID_BOTTOM_MARGIN             :
            {
                sal_Int16 nVal;
                aValue >>= nVal;
                if(nVal < 0)
                    throw lang::IllegalArgumentException();
                aFormat.SetDistance(pMap->nMemberId, nVal);
            }
            break;
            case FID_IS_SCALE_ALL_BRACKETS              :
                aFormat.SetScaleNormalBrackets(*(sal_Bool*)aValue.getValue());
            break;
        }
        if(pMap->nWID)
        {
            ((SmDocShell*)pObjShell)->SetFormat(aFormat);
            aFormat.RequestApplyChanges();
        }
    }
    else
        throw beans::UnknownPropertyException();
}
/*-- 07.01.00 16:33:00---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SmModel::getPropertyValue(const OUString& rPropertyName)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());

    uno::Any aRet;
    SfxObjectShell* pObjShell = GetObjectShell();
    if(!pObjShell)
        throw uno::RuntimeException();
    const SfxItemPropertyMap*   pMap = SfxItemPropertyMap::GetByName(
                                                        _pMap, rPropertyName);
    if(pMap)
    {
        const SmFormat& rFormat = ((SmDocShell*)pObjShell)->GetFormat();
        switch(pMap->nWID)
        {
            case  FID_FORMULA:
                aRet <<= OUString(((SmDocShell*)pObjShell)->GetText());
            break;
            case FID_FONT_NAME_VARIABLES                :
            case FID_FONT_NAME_FUNCTIONS                :
            case FID_FONT_NAME_NUMBERS                  :
            case FID_FONT_NAME_TEXT                     :
            case FID_CUSTOM_FONT_NAME_SERIF             :
            case FID_CUSTOM_FONT_NAME_SANS              :
            case FID_CUSTOM_FONT_NAME_FIXED             :
            {
                const SmFace &  rFace = rFormat.GetFont(pMap->nMemberId);
                aRet <<= OUString(rFace.GetName());
            }
            break;
            case FID_CUSTOM_FONT_FIXED_POSTURE:
            case FID_CUSTOM_FONT_SANS_POSTURE :
            case FID_CUSTOM_FONT_SERIF_POSTURE:
            case FID_FONT_VARIABLES_POSTURE   :
            case FID_FONT_FUNCTIONS_POSTURE   :
            case FID_FONT_NUMBERS_POSTURE     :
            case FID_FONT_TEXT_POSTURE        :
            {
                const SmFace &  rFace = rFormat.GetFont(pMap->nMemberId);
                BOOL bVal = (rFace.GetItalic() != ITALIC_NONE);
                aRet.setValue(&bVal, *pMap->pType);
            }
            break;
            case FID_CUSTOM_FONT_FIXED_WEIGHT :
            case FID_CUSTOM_FONT_SANS_WEIGHT  :
            case FID_CUSTOM_FONT_SERIF_WEIGHT :
            case FID_FONT_VARIABLES_WEIGHT    :
            case FID_FONT_FUNCTIONS_WEIGHT    :
            case FID_FONT_NUMBERS_WEIGHT      :
            case FID_FONT_TEXT_WEIGHT         :
            {
                const SmFace &  rFace = rFormat.GetFont(pMap->nMemberId);
                BOOL bVal = (rFace.GetWeight() == WEIGHT_BOLD);
                aRet.setValue(&bVal, *pMap->pType);
            }
            break;
            case FID_BASE_FONT_HEIGHT                   :
            {
                // Point!
                sal_Int16 nVal = rFormat.GetBaseSize().Height();
                nVal = MM100_TO_TWIP(nVal);
                nVal = (nVal + 10) / 20;
                aRet <<= nVal;
            }
            break;
            case FID_RELATIVE_FONT_HEIGHT_TEXT          :
            case FID_RELATIVE_FONT_HEIGHT_INDICES       :
            case FID_RELATIVE_FONT_HEIGHT_FUNCTIONS     :
            case FID_RELATIVE_FONT_HEIGHT_OPERATORS     :
            case FID_RELATIVE_FONT_HEIGHT_LIMITS        :
                aRet <<= (sal_Int16) rFormat.GetRelSize(pMap->nMemberId);
            break;

            case FID_IS_TEXT_MODE                       :
            {
                sal_Bool bVal = rFormat.IsTextmode();
                aRet.setValue(&bVal, ::getBooleanCppuType());
            }
            break;

            case FID_ALIGNMENT                          :
                // SmHorAlign uses the same values as HorizontalAlignment
                aRet <<= (sal_Int16)rFormat.GetHorAlign();
            break;

            case FID_RELATIVE_SPACING                   :
            case FID_RELATIVE_LINE_SPACING              :
            case FID_RELATIVE_ROOT_SPACING              :
            case FID_RELATIVE_INDEX_SUPERSCRIPT         :
            case FID_RELATIVE_INDEX_SUBSCRIPT           :
            case FID_RELATIVE_FRACTION_NUMERATOR_HEIGHT :
            case FID_RELATIVE_FRACTION_DENOMINATOR_DEPTH:
            case FID_RELATIVE_FRACTION_BAR_EXCESS_LENGTH:
            case FID_RELATIVE_FRACTION_BAR_LINE_WEIGHT  :
            case FID_RELATIVE_UPPER_LIMIT_DISTANCE      :
            case FID_RELATIVE_LOWER_LIMIT_DISTANCE      :
            case FID_RELATIVE_BRACKET_EXCESS_SIZE       :
            case FID_RELATIVE_BRACKET_DISTANCE          :
            case FID_RELATIVE_SCALE_BRACKET_EXCESS_SIZE :
            case FID_RELATIVE_MATRIX_LINE_SPACING       :
            case FID_RELATIVE_MATRIX_COLUMN_SPACING     :
            case FID_RELATIVE_SYMBOL_PRIMARY_HEIGHT     :
            case FID_RELATIVE_SYMBOL_MINIMUM_HEIGHT     :
            case FID_RELATIVE_OPERATOR_EXCESS_SIZE      :
            case FID_RELATIVE_OPERATOR_SPACING          :
            case FID_LEFT_MARGIN               :
            case FID_RIGHT_MARGIN              :
            case FID_TOP_MARGIN                :
            case FID_BOTTOM_MARGIN             :
                aRet <<= (sal_Int16)rFormat.GetDistance(pMap->nMemberId);
            break;
            case FID_IS_SCALE_ALL_BRACKETS              :
                sal_Bool bVal = rFormat.IsScaleNormalBrackets();
                aRet.setValue(&bVal, ::getBooleanCppuType());
            break;
        }
    }
    else
        throw beans::UnknownPropertyException();
    return aRet;
}
/*-- 07.01.00 16:33:00---------------------------------------------------

  -----------------------------------------------------------------------*/
void SmModel::addPropertyChangeListener(const OUString& PropertyName,
    const uno::Reference< beans::XPropertyChangeListener > & aListener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
}
/*-- 07.01.00 16:33:01---------------------------------------------------

  -----------------------------------------------------------------------*/
void SmModel::removePropertyChangeListener(const OUString& PropertyName,
    const uno::Reference< beans::XPropertyChangeListener > & aListener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
}
/*-- 07.01.00 16:33:01---------------------------------------------------

  -----------------------------------------------------------------------*/
void SmModel::addVetoableChangeListener(const OUString& PropertyName,
    const uno::Reference< beans::XVetoableChangeListener > & aListener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
}
/*-- 07.01.00 16:33:01---------------------------------------------------

  -----------------------------------------------------------------------*/
void SmModel::removeVetoableChangeListener(const OUString& PropertyName,
    const uno::Reference< beans::XVetoableChangeListener > & aListener)
    throw( beans::UnknownPropertyException, lang::WrappedTargetException, uno::RuntimeException )
{
}
/*-- 07.02.00 13:24:08---------------------------------------------------

  -----------------------------------------------------------------------*/
OUString SmModel::getImplementationName(void) throw( uno::RuntimeException )
{
    return C2U("SmModel");
}
/*-- 07.02.00 13:24:09---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SmModel::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return rServiceName == C2U("com.sun.star.formula.FormulaProperties");
}
/*-- 07.02.00 13:24:09---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > SmModel::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());

    uno::Sequence< OUString > aRet(1);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.formula.FormulaProperties");
    return aRet;
}
/* -----------------------------20.06.00 10:47--------------------------------

 ---------------------------------------------------------------------------*/
void * SAL_CALL SmModel::operator new( size_t t ) throw()
{
    return SmModelBaseClass::operator new( t );
}
/* -----------------------------20.06.00 10:47--------------------------------

 ---------------------------------------------------------------------------*/
void SAL_CALL SmModel::operator delete( void * p ) throw()
{
    SmModelBaseClass::operator delete( p );
}

