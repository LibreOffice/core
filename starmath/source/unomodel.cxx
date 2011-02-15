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
#include "precompiled_starmath.hxx"


#include <vos/mutex.hxx>
#include <osl/mutex.hxx>
#include <sfx2/printer.hxx>
#include <vcl/svapp.hxx>
#include <svtools/ctrltool.hxx>
#include <svl/itemprop.hxx>
#include <unotools/localedatawrapper.hxx>
#include <unotools/processfactory.hxx>
#include <editeng/paperinf.hxx>
#include <vcl/settings.hxx>
#include <vcl/print.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <com/sun/star/beans/PropertyState.hpp>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/formula/SymbolDescriptor.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <unotools/moduleoptions.hxx>

#include <unomodel.hxx>
#include <document.hxx>
#include <view.hxx>
#include <symbol.hxx>
#include <starmath.hrc>
#include <config.hxx>
#include <smdll.hxx>

using namespace ::vos;
using namespace ::rtl;
using namespace ::cppu;
using namespace ::std;
using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::formula;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::script;


#define TWIP_TO_MM100(TWIP)     ((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)    ((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))

////////////////////////////////////////////////////////////

SmPrintUIOptions::SmPrintUIOptions()
{
    ResStringArray      aLocalizedStrings( SmResId( RID_PRINTUIOPTIONS ) );
    DBG_ASSERT( aLocalizedStrings.Count() >= 9, "resource incomplete" );
    if( aLocalizedStrings.Count() < 9 ) // bad resource ?
        return;

    SmModule *pp = SM_MOD();
    SmConfig *pConfig = pp->GetConfig();
    DBG_ASSERT( pConfig, "SmConfig not found" );
    if (!pConfig)
        return;

    // create sequence of print UI options
    // (Actually IsIgnoreSpacesRight is a parser option. Without it we need only 8 properties here.)
    m_aUIProperties.realloc( 9 );

    // create Section for formula (results in an extra tab page in dialog)
    SvtModuleOptions aOpt;
    String aAppGroupname( aLocalizedStrings.GetString( 0 ) );
    aAppGroupname.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s" ) ),
                                    aOpt.GetModuleName( SvtModuleOptions::E_SMATH ) );
    m_aUIProperties[0].Value = getGroupControlOpt( aAppGroupname, rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:TabPage:AppPage" ) ) );

    // create subgroup for print options
    m_aUIProperties[1].Value = getSubgroupControlOpt( aLocalizedStrings.GetString( 1 ), rtl::OUString() );

    // create a bool option for title row (matches to SID_PRINTTITLE)
    m_aUIProperties[2].Value = getBoolControlOpt( aLocalizedStrings.GetString( 2 ),
                                                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:TitleRow:CheckBox" ) ),
                                                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( PRTUIOPT_TITLE_ROW ) ),
                                                  pConfig->IsPrintTitle() );
    // create a bool option for formula text (matches to SID_PRINTTEXT)
    m_aUIProperties[3].Value = getBoolControlOpt( aLocalizedStrings.GetString( 3 ),
                                                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:FormulaText:CheckBox" ) ),
                                                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( PRTUIOPT_FORMULA_TEXT ) ),
                                                  pConfig->IsPrintFormulaText() );
    // create a bool option for border (matches to SID_PRINTFRAME)
    m_aUIProperties[4].Value = getBoolControlOpt( aLocalizedStrings.GetString( 4 ),
                                                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:Border:CheckBox" ) ),
                                                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( PRTUIOPT_BORDER ) ),
                                                  pConfig->IsPrintFrame() );

    // create subgroup for print format
    m_aUIProperties[5].Value = getSubgroupControlOpt( aLocalizedStrings.GetString( 5 ), rtl::OUString() );

    // create a radio button group for print format (matches to SID_PRINTSIZE)
    Sequence< rtl::OUString > aChoices( 3 );
    aChoices[0] = aLocalizedStrings.GetString( 6 );
    aChoices[1] = aLocalizedStrings.GetString( 7 );
    aChoices[2] = aLocalizedStrings.GetString( 8 );
    Sequence< rtl::OUString > aHelpIds( 3 );
    aHelpIds[0] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintFormat:RadioButton:0" ) );
    aHelpIds[1] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintFormat:RadioButton:1" ) );
    aHelpIds[2] = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintFormat:RadioButton:2" ) );
    OUString aPrintFormatProp( RTL_CONSTASCII_USTRINGPARAM( PRTUIOPT_PRINT_FORMAT ) );
    m_aUIProperties[6].Value = getChoiceControlOpt( rtl::OUString(),
                                                    aHelpIds,
                                                    aPrintFormatProp,
                                                    aChoices, static_cast< sal_Int32 >(pConfig->GetPrintSize())
                                                    );

    // create a numeric box for scale dependent on PrintFormat = "Scaling" (matches to SID_PRINTZOOM)
    vcl::PrinterOptionsHelper::UIControlOptions aRangeOpt( aPrintFormatProp, 2, sal_True );
    m_aUIProperties[ 7 ].Value = getRangeControlOpt( rtl::OUString(),
                                                     rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( ".HelpID:vcl:PrintDialog:PrintScale:NumericField" ) ),
                                                     rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( PRTUIOPT_PRINT_SCALE ) ),
                                                     pConfig->GetPrintZoomFactor(),    // initial value
                                                     10,     // min value
                                                     1000,   // max value
                                                     aRangeOpt );

    Sequence< PropertyValue > aHintNoLayoutPage( 1 );
    aHintNoLayoutPage[0].Name = rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "HintNoLayoutPage" ) );
    aHintNoLayoutPage[0].Value = makeAny( sal_True );
    m_aUIProperties[8].Value <<= aHintNoLayoutPage;

// IsIgnoreSpacesRight is a parser option! Thus we don't add it to the printer UI.
//
//    // create subgroup for misc options
//    m_aUIProperties[8].Value = getSubgroupControlOpt( aLocalizedStrings.GetString( 9 ) );
//
//    // create a bool option for ignore spacing (matches to SID_NO_RIGHT_SPACES)
//    m_aUIProperties[9].Value = getBoolControlOpt( aLocalizedStrings.GetString( 10 ),
//                                                  rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( PRTUIOPT_NO_RIGHT_SPACE ) ),
//                                                  pConfig->IsIgnoreSpacesRight() );
}


////////////////////////////////////////////////////////////
//
// class SmModel
//

// values from com/sun/star/beans/PropertyAttribute
#define PROPERTY_NONE        0
#define PROPERTY_READONLY   16

enum SmModelPropertyHandles
{
    HANDLE_FORMULA,
    HANDLE_FONT_NAME_VARIABLES,
    HANDLE_FONT_NAME_FUNCTIONS,
    HANDLE_FONT_NAME_NUMBERS,
    HANDLE_FONT_NAME_TEXT,
    HANDLE_CUSTOM_FONT_NAME_SERIF,
    HANDLE_CUSTOM_FONT_NAME_SANS,
    HANDLE_CUSTOM_FONT_NAME_FIXED,
    HANDLE_CUSTOM_FONT_FIXED_POSTURE,
    HANDLE_CUSTOM_FONT_FIXED_WEIGHT,
    HANDLE_CUSTOM_FONT_SANS_POSTURE,
    HANDLE_CUSTOM_FONT_SANS_WEIGHT,
    HANDLE_CUSTOM_FONT_SERIF_POSTURE,
    HANDLE_CUSTOM_FONT_SERIF_WEIGHT,
    HANDLE_FONT_VARIABLES_POSTURE,
    HANDLE_FONT_VARIABLES_WEIGHT,
    HANDLE_FONT_FUNCTIONS_POSTURE,
    HANDLE_FONT_FUNCTIONS_WEIGHT,
    HANDLE_FONT_NUMBERS_POSTURE,
    HANDLE_FONT_NUMBERS_WEIGHT,
    HANDLE_FONT_TEXT_POSTURE,
    HANDLE_FONT_TEXT_WEIGHT,
    HANDLE_BASE_FONT_HEIGHT,
    HANDLE_RELATIVE_FONT_HEIGHT_TEXT,
    HANDLE_RELATIVE_FONT_HEIGHT_INDICES,
    HANDLE_RELATIVE_FONT_HEIGHT_FUNCTIONS,
    HANDLE_RELATIVE_FONT_HEIGHT_OPERATORS,
    HANDLE_RELATIVE_FONT_HEIGHT_LIMITS,
    HANDLE_IS_TEXT_MODE,
    HANDLE_GREEK_CHAR_STYLE,
    HANDLE_ALIGNMENT,
    HANDLE_RELATIVE_SPACING,
    HANDLE_RELATIVE_LINE_SPACING,
    HANDLE_RELATIVE_ROOT_SPACING,
    HANDLE_RELATIVE_INDEX_SUPERSCRIPT,
    HANDLE_RELATIVE_INDEX_SUBSCRIPT,
    HANDLE_RELATIVE_FRACTION_NUMERATOR_HEIGHT,
    HANDLE_RELATIVE_FRACTION_DENOMINATOR_DEPTH,
    HANDLE_RELATIVE_FRACTION_BAR_EXCESS_LENGTH,
    HANDLE_RELATIVE_FRACTION_BAR_LINE_WEIGHT,
    HANDLE_RELATIVE_UPPER_LIMIT_DISTANCE,
    HANDLE_RELATIVE_LOWER_LIMIT_DISTANCE,
    HANDLE_RELATIVE_BRACKET_EXCESS_SIZE,
    HANDLE_RELATIVE_BRACKET_DISTANCE,
    HANDLE_IS_SCALE_ALL_BRACKETS,
    HANDLE_RELATIVE_SCALE_BRACKET_EXCESS_SIZE,
    HANDLE_RELATIVE_MATRIX_LINE_SPACING,
    HANDLE_RELATIVE_MATRIX_COLUMN_SPACING,
    HANDLE_RELATIVE_SYMBOL_PRIMARY_HEIGHT,
    HANDLE_RELATIVE_SYMBOL_MINIMUM_HEIGHT,
    HANDLE_RELATIVE_OPERATOR_EXCESS_SIZE,
    HANDLE_RELATIVE_OPERATOR_SPACING,
    HANDLE_LEFT_MARGIN,
    HANDLE_RIGHT_MARGIN,
    HANDLE_TOP_MARGIN,
    HANDLE_BOTTOM_MARGIN,
    HANDLE_PRINTER_NAME,
    HANDLE_PRINTER_SETUP,
    HANDLE_SYMBOLS,
    HANDLE_USED_SYMBOLS,
    HANDLE_BASIC_LIBRARIES,     /* #93295# */
    HANDLE_RUNTIME_UID,
    // --> PB 2004-08-25 #i33095# Security Options
    HANDLE_LOAD_READONLY,
    // <--
    HANDLE_DIALOG_LIBRARIES,     // #i73329#
    HANDLE_BASELINE // 3.7.2010 #i972#
};

PropertySetInfo * lcl_createModelPropertyInfo ()
{
    static PropertyMapEntry aModelPropertyInfoMap[] =
    {
        { RTL_CONSTASCII_STRINGPARAM( "Alignment"                          ), HANDLE_ALIGNMENT                          ,       &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM( "BaseFontHeight"                  ), HANDLE_BASE_FONT_HEIGHT                   ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM( "BasicLibraries"                  ), HANDLE_BASIC_LIBRARIES                   ,      &::getCppuType((const uno::Reference< script::XLibraryContainer > *)0),    PropertyAttribute::READONLY, 0},
        { RTL_CONSTASCII_STRINGPARAM( "BottomMargin"                      ), HANDLE_BOTTOM_MARGIN                      ,        &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_BOTTOMSPACE             },
        { RTL_CONSTASCII_STRINGPARAM( "CustomFontNameFixed"            ), HANDLE_CUSTOM_FONT_NAME_FIXED             ,       &::getCppuType((const OUString*)0),     PROPERTY_NONE, FNT_FIXED          },
        { RTL_CONSTASCII_STRINGPARAM( "CustomFontNameSans"              ), HANDLE_CUSTOM_FONT_NAME_SANS              ,      &::getCppuType((const OUString*)0),     PROPERTY_NONE, FNT_SANS        },
        { RTL_CONSTASCII_STRINGPARAM( "CustomFontNameSerif"             ), HANDLE_CUSTOM_FONT_NAME_SERIF             ,      &::getCppuType((const OUString*)0),     PROPERTY_NONE, FNT_SERIF          },
        { RTL_CONSTASCII_STRINGPARAM( "DialogLibraries"                 ), HANDLE_DIALOG_LIBRARIES                   ,      &::getCppuType((const uno::Reference< script::XLibraryContainer > *)0),    PropertyAttribute::READONLY, 0},
        { RTL_CONSTASCII_STRINGPARAM( "FontFixedIsBold"),     HANDLE_CUSTOM_FONT_FIXED_WEIGHT    ,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_FIXED},
        { RTL_CONSTASCII_STRINGPARAM( "FontFixedIsItalic"), HANDLE_CUSTOM_FONT_FIXED_POSTURE   ,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_FIXED},
        { RTL_CONSTASCII_STRINGPARAM( "FontFunctionsIsBold"),    HANDLE_FONT_FUNCTIONS_WEIGHT    ,  &::getBooleanCppuType(),            PROPERTY_NONE, FNT_FUNCTION},
        { RTL_CONSTASCII_STRINGPARAM( "FontFunctionsIsItalic"),   HANDLE_FONT_FUNCTIONS_POSTURE   ,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_FUNCTION},
        { RTL_CONSTASCII_STRINGPARAM( "FontNameFunctions"                ), HANDLE_FONT_NAME_FUNCTIONS                ,         &::getCppuType((const OUString*)0),     PROPERTY_NONE, FNT_FUNCTION },
        { RTL_CONSTASCII_STRINGPARAM( "FontNameNumbers"                  ), HANDLE_FONT_NAME_NUMBERS                  ,         &::getCppuType((const OUString*)0),     PROPERTY_NONE, FNT_NUMBER        },
        { RTL_CONSTASCII_STRINGPARAM( "FontNameText"                     ), HANDLE_FONT_NAME_TEXT                     ,         &::getCppuType((const OUString*)0),     PROPERTY_NONE, FNT_TEXT        },
        { RTL_CONSTASCII_STRINGPARAM( "FontNameVariables"                ), HANDLE_FONT_NAME_VARIABLES                ,         &::getCppuType((const OUString*)0),     PROPERTY_NONE, FNT_VARIABLE },
        { RTL_CONSTASCII_STRINGPARAM( "FontNumbersIsBold"),  HANDLE_FONT_NUMBERS_WEIGHT    ,  &::getBooleanCppuType(),              PROPERTY_NONE, FNT_NUMBER},
        { RTL_CONSTASCII_STRINGPARAM( "FontNumbersIsItalic"),   HANDLE_FONT_NUMBERS_POSTURE   ,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_NUMBER},
        { RTL_CONSTASCII_STRINGPARAM( "FontSansIsBold"),     HANDLE_CUSTOM_FONT_SANS_WEIGHT    ,  &::getBooleanCppuType(),              PROPERTY_NONE, FNT_SANS},
        { RTL_CONSTASCII_STRINGPARAM( "FontSansIsItalic"),   HANDLE_CUSTOM_FONT_SANS_POSTURE   ,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_SANS},
        { RTL_CONSTASCII_STRINGPARAM( "FontSerifIsBold"),    HANDLE_CUSTOM_FONT_SERIF_WEIGHT    ,  &::getBooleanCppuType(),             PROPERTY_NONE,  FNT_SERIF},
        { RTL_CONSTASCII_STRINGPARAM( "FontSerifIsItalic"),   HANDLE_CUSTOM_FONT_SERIF_POSTURE   ,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_SERIF},
        { RTL_CONSTASCII_STRINGPARAM( "FontTextIsBold"),     HANDLE_FONT_TEXT_WEIGHT    ,  &::getBooleanCppuType(),             PROPERTY_NONE, FNT_TEXT},
        { RTL_CONSTASCII_STRINGPARAM( "FontTextIsItalic"),   HANDLE_FONT_TEXT_POSTURE   ,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_TEXT},
        { RTL_CONSTASCII_STRINGPARAM( "FontVariablesIsBold"),    HANDLE_FONT_VARIABLES_WEIGHT    ,  &::getBooleanCppuType(),            PROPERTY_NONE, FNT_VARIABLE},
        { RTL_CONSTASCII_STRINGPARAM( "FontVariablesIsItalic"),   HANDLE_FONT_VARIABLES_POSTURE,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_VARIABLE},
        { RTL_CONSTASCII_STRINGPARAM( "Formula"                           ),    HANDLE_FORMULA                             ,        &::getCppuType((const OUString*)0),     PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM( "IsScaleAllBrackets"              ), HANDLE_IS_SCALE_ALL_BRACKETS              ,      &::getBooleanCppuType(),    PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM( "IsTextMode"                       ), HANDLE_IS_TEXT_MODE                       ,         &::getBooleanCppuType(),    PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM( "GreekCharStyle" ),                   HANDLE_GREEK_CHAR_STYLE,    &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM( "LeftMargin"                        ), HANDLE_LEFT_MARGIN                        ,        &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_LEFTSPACE                 },
        { RTL_CONSTASCII_STRINGPARAM( "PrinterName"                    ), HANDLE_PRINTER_NAME                        ,      &::getCppuType((const OUString*)0),     PROPERTY_NONE, 0                  },
        { RTL_CONSTASCII_STRINGPARAM( "PrinterSetup"                       ), HANDLE_PRINTER_SETUP                       ,      &::getCppuType((const Sequence < sal_Int8 >*)0),    PROPERTY_NONE, 0                  },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeBracketDistance"          ), HANDLE_RELATIVE_BRACKET_DISTANCE          ,         &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_BRACKETSPACE },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeBracketExcessSize"       ), HANDLE_RELATIVE_BRACKET_EXCESS_SIZE       ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_BRACKETSIZE  },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeFontHeightFunctions"     ), HANDLE_RELATIVE_FONT_HEIGHT_FUNCTIONS     ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, SIZ_FUNCTION},
        { RTL_CONSTASCII_STRINGPARAM( "RelativeFontHeightIndices"       ), HANDLE_RELATIVE_FONT_HEIGHT_INDICES       ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, SIZ_INDEX      },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeFontHeightLimits"        ), HANDLE_RELATIVE_FONT_HEIGHT_LIMITS        ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, SIZ_LIMITS    },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeFontHeightOperators"     ), HANDLE_RELATIVE_FONT_HEIGHT_OPERATORS     ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, SIZ_OPERATOR},
        { RTL_CONSTASCII_STRINGPARAM( "RelativeFontHeightText"            ), HANDLE_RELATIVE_FONT_HEIGHT_TEXT          ,        &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, SIZ_TEXT   },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeFractionBarExcessLength"), HANDLE_RELATIVE_FRACTION_BAR_EXCESS_LENGTH,       &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_FRACTION        },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeFractionBarLineWeight"  ), HANDLE_RELATIVE_FRACTION_BAR_LINE_WEIGHT  ,       &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_STROKEWIDTH  },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeFractionDenominatorDepth"), HANDLE_RELATIVE_FRACTION_DENOMINATOR_DEPTH,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_DENOMINATOR  },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeFractionNumeratorHeight" ), HANDLE_RELATIVE_FRACTION_NUMERATOR_HEIGHT ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_NUMERATOR          },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeIndexSubscript"           ), HANDLE_RELATIVE_INDEX_SUBSCRIPT           ,         &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_SUBSCRIPT          },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeIndexSuperscript"         ), HANDLE_RELATIVE_INDEX_SUPERSCRIPT         ,         &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_SUPERSCRIPT  },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeLineSpacing"              ), HANDLE_RELATIVE_LINE_SPACING              ,         &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_VERTICAL        },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeLowerLimitDistance"      ), HANDLE_RELATIVE_LOWER_LIMIT_DISTANCE      ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_LOWERLIMIT        },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeMatrixColumnSpacing"     ), HANDLE_RELATIVE_MATRIX_COLUMN_SPACING     ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_MATRIXCOL},
        { RTL_CONSTASCII_STRINGPARAM( "RelativeMatrixLineSpacing"       ), HANDLE_RELATIVE_MATRIX_LINE_SPACING       ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_MATRIXROW},
        { RTL_CONSTASCII_STRINGPARAM( "RelativeOperatorExcessSize"      ), HANDLE_RELATIVE_OPERATOR_EXCESS_SIZE      ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_OPERATORSIZE        },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeOperatorSpacing"          ), HANDLE_RELATIVE_OPERATOR_SPACING          ,         &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_OPERATORSPACE},
        { RTL_CONSTASCII_STRINGPARAM( "RelativeRootSpacing"              ), HANDLE_RELATIVE_ROOT_SPACING              ,         &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_ROOT               },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeScaleBracketExcessSize" ), HANDLE_RELATIVE_SCALE_BRACKET_EXCESS_SIZE ,       &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_NORMALBRACKETSIZE},
        { RTL_CONSTASCII_STRINGPARAM( "RelativeSpacing"                   ), HANDLE_RELATIVE_SPACING                   ,        &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_HORIZONTAL        },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeSymbolMinimumHeight"     ), HANDLE_RELATIVE_SYMBOL_MINIMUM_HEIGHT     ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_ORNAMENTSPACE         },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeSymbolPrimaryHeight"     ), HANDLE_RELATIVE_SYMBOL_PRIMARY_HEIGHT     ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_ORNAMENTSIZE        },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeUpperLimitDistance"      ),  HANDLE_RELATIVE_UPPER_LIMIT_DISTANCE     ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_UPPERLIMIT        },
        { RTL_CONSTASCII_STRINGPARAM( "RightMargin"                       ),    HANDLE_RIGHT_MARGIN                  ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_RIGHTSPACE               },
        { RTL_CONSTASCII_STRINGPARAM( "RuntimeUID"                      ), HANDLE_RUNTIME_UID                        ,      &::getCppuType(static_cast< const rtl::OUString * >(0)),    PropertyAttribute::READONLY, 0 },
        { RTL_CONSTASCII_STRINGPARAM( "Symbols"                       ),        HANDLE_SYMBOLS                       ,      &::getCppuType((const Sequence < SymbolDescriptor > *)0),   PROPERTY_NONE, 0  },
        { RTL_CONSTASCII_STRINGPARAM( "UserDefinedSymbolsInUse"       ),        HANDLE_USED_SYMBOLS                  ,      &::getCppuType((const Sequence < SymbolDescriptor > *)0),   PropertyAttribute::READONLY, 0  },
        { RTL_CONSTASCII_STRINGPARAM( "TopMargin"                         ),    HANDLE_TOP_MARGIN                    ,      &::getCppuType((const sal_Int16*)0),    PROPERTY_NONE, DIS_TOPSPACE               },
        // --> PB 2004-08-25 #i33095# Security Options
        { RTL_CONSTASCII_STRINGPARAM( "LoadReadonly" ), HANDLE_LOAD_READONLY, &::getBooleanCppuType(), PROPERTY_NONE, 0 },
        // <--
        // --> 3.7.2010 #i972#
        { RTL_CONSTASCII_STRINGPARAM( "BaseLine"), HANDLE_BASELINE, &::getCppuType((const sal_Int16*)0), PROPERTY_NONE, 0},
        // <--
        { NULL, 0, 0, NULL, 0, 0 }
    };
    PropertySetInfo *pInfo = new PropertySetInfo ( aModelPropertyInfoMap );
    return pInfo;
}
//-----------------------------------------------------------------------
SmModel::SmModel( SfxObjectShell *pObjSh )
: SfxBaseModel(pObjSh)
, PropertySetHelper ( lcl_createModelPropertyInfo () )
, m_pPrintUIOptions( NULL )

{
}
//-----------------------------------------------------------------------
SmModel::~SmModel() throw ()
{
    delete m_pPrintUIOptions;
}
/*-- 28.03.00 14:18:17---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SAL_CALL SmModel::queryInterface( const uno::Type& rType ) throw(uno::RuntimeException)
{
    uno::Any aRet =  ::cppu::queryInterface ( rType,
                                    // OWeakObject interfaces
                                    dynamic_cast< XInterface* > ( static_cast< XUnoTunnel* > ( this )),
                                    static_cast< XWeak* > ( this ),
                                    // PropertySetHelper interfaces
                                    static_cast< XPropertySet* > ( this ),
                                    static_cast< XMultiPropertySet* > ( this ),
                                    //static_cast< XPropertyState* > ( this ),
                                    // my own interfaces
                                    static_cast< XServiceInfo*  > ( this ),
                                    static_cast< XRenderable*  > ( this ) );
    if (!aRet.hasValue())
        aRet = SfxBaseModel::queryInterface ( rType );
    return aRet;
}
/*-- 28.03.00 14:18:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SmModel::acquire() throw()
{
    OWeakObject::acquire();
}
/*-- 28.03.00 14:18:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SmModel::release() throw()
{
    OWeakObject::release();
}
/*-- 28.03.00 14:18:19---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< uno::Type > SAL_CALL SmModel::getTypes(  ) throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Sequence< uno::Type > aTypes = SfxBaseModel::getTypes();
    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 4);
    uno::Type* pTypes = aTypes.getArray();
    pTypes[nLen++] = ::getCppuType((Reference<XServiceInfo>*)0);
    pTypes[nLen++] = ::getCppuType((Reference<XPropertySet>*)0);
    pTypes[nLen++] = ::getCppuType((Reference<XMultiPropertySet>*)0);
    pTypes[nLen++] = ::getCppuType((Reference<XRenderable>*)0);

    // XPropertyState not supported?? (respective virtual functions from
    // PropertySetHelper not overloaded)
    //pTypes[nLen++] = ::getCppuType((Reference<XPropertyState>*)0);

    return aTypes;
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
} /* -----------------------------28.03.00 14:23--------------------------------

 ---------------------------------------------------------------------------*/
sal_Int64 SAL_CALL SmModel::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException)
{
    if( rId.getLength() == 16
        && 0 == rtl_compareMemory( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return sal::static_int_cast< sal_Int64 >(reinterpret_cast< sal_uIntPtr >(this));
    }

    return SfxBaseModel::getSomething( rId );
}
/*-- 07.01.00 16:32:59---------------------------------------------------

  -----------------------------------------------------------------------*/
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

OUString SmModel::getImplementationName(void) throw( uno::RuntimeException )
{
    return getImplementationName_Static();
}


::rtl::OUString SmModel::getImplementationName_Static()
{
    return rtl::OUString::createFromAscii("com.sun.star.comp.math.FormulaDocument");
}

/*-- 20.01.04 11:21:00---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SmModel::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return (
            rServiceName == A2OU("com.sun.star.document.OfficeDocument"  ) ||
            rServiceName == A2OU("com.sun.star.formula.FormulaProperties")
           );
}
/*-- 20.01.04 11:21:00---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > SmModel::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    return getSupportedServiceNames_Static();
}

uno::Sequence< OUString > SmModel::getSupportedServiceNames_Static(void)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());

    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = A2OU("com.sun.star.document.OfficeDocument");
    pArray[1] = A2OU("com.sun.star.formula.FormulaProperties");
    return aRet;
}

void SmModel::_setPropertyValues(const PropertyMapEntry** ppEntries, const Any* pValues)
    throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());

    SmDocShell *pDocSh = static_cast < SmDocShell * > (GetObjectShell());

    if ( NULL == pDocSh )
        throw UnknownPropertyException();

    SmFormat aFormat = pDocSh->GetFormat();

    for (; *ppEntries; ppEntries++, pValues++ )
    {
        if ((*ppEntries)->mnAttributes & PropertyAttribute::READONLY)
            throw PropertyVetoException();

        switch ( (*ppEntries)->mnHandle )
        {
            case HANDLE_FORMULA:
            {
                OUString aText;
                *pValues >>= aText;
                pDocSh->SetText(aText);
            }
            break;
            case HANDLE_FONT_NAME_VARIABLES                :
            case HANDLE_FONT_NAME_FUNCTIONS                :
            case HANDLE_FONT_NAME_NUMBERS                  :
            case HANDLE_FONT_NAME_TEXT                     :
            case HANDLE_CUSTOM_FONT_NAME_SERIF             :
            case HANDLE_CUSTOM_FONT_NAME_SANS              :
            case HANDLE_CUSTOM_FONT_NAME_FIXED             :
            {
                OUString aText;
                *pValues >>= aText;
                String sFontName = aText;
                if(!sFontName.Len())
                    throw IllegalArgumentException();

                if(aFormat.GetFont((*ppEntries)->mnMemberId).GetName() != sFontName)
                {
                    const SmFace rOld = aFormat.GetFont((*ppEntries)->mnMemberId);

                    SmFace aSet( sFontName, rOld.GetSize() );
                    aSet.SetBorderWidth( rOld.GetBorderWidth() );
                    aSet.SetAlign( ALIGN_BASELINE );
                    aFormat.SetFont( (*ppEntries)->mnMemberId, aSet );
                }
            }
            break;
            case HANDLE_CUSTOM_FONT_FIXED_POSTURE:
            case HANDLE_CUSTOM_FONT_SANS_POSTURE :
            case HANDLE_CUSTOM_FONT_SERIF_POSTURE:
            case HANDLE_FONT_VARIABLES_POSTURE   :
            case HANDLE_FONT_FUNCTIONS_POSTURE   :
            case HANDLE_FONT_NUMBERS_POSTURE     :
            case HANDLE_FONT_TEXT_POSTURE        :
            {
                if((*pValues).getValueType() != ::getBooleanCppuType())
                    throw IllegalArgumentException();
                sal_Bool bVal = *(sal_Bool*)(*pValues).getValue();
                Font aNewFont(aFormat.GetFont((*ppEntries)->mnMemberId));
                aNewFont.SetItalic((bVal) ? ITALIC_NORMAL : ITALIC_NONE);
                aFormat.SetFont((*ppEntries)->mnMemberId, aNewFont);
            }
            break;
            case HANDLE_CUSTOM_FONT_FIXED_WEIGHT :
            case HANDLE_CUSTOM_FONT_SANS_WEIGHT  :
            case HANDLE_CUSTOM_FONT_SERIF_WEIGHT :
            case HANDLE_FONT_VARIABLES_WEIGHT    :
            case HANDLE_FONT_FUNCTIONS_WEIGHT    :
            case HANDLE_FONT_NUMBERS_WEIGHT      :
            case HANDLE_FONT_TEXT_WEIGHT         :
            {
                if((*pValues).getValueType() != ::getBooleanCppuType())
                    throw IllegalArgumentException();
                sal_Bool bVal = *(sal_Bool*)(*pValues).getValue();
                Font aNewFont(aFormat.GetFont((*ppEntries)->mnMemberId));
                aNewFont.SetWeight((bVal) ? WEIGHT_BOLD : WEIGHT_NORMAL);
                aFormat.SetFont((*ppEntries)->mnMemberId, aNewFont);
            }
            break;
            case HANDLE_BASE_FONT_HEIGHT                   :
            {
                // Point!
                sal_Int16 nVal = lcl_AnyToINT16(*pValues);
                if(nVal < 1)
                    throw IllegalArgumentException();
                Size aSize = aFormat.GetBaseSize();
                nVal *= 20;
                nVal = static_cast < sal_Int16 > ( TWIP_TO_MM100(nVal) );
                aSize.Height() = nVal;
                aFormat.SetBaseSize(aSize);

                // apply base size to fonts
                const Size aTmp( aFormat.GetBaseSize() );
                for (sal_uInt16  i = FNT_BEGIN;  i <= FNT_END;  i++)
                    aFormat.SetFontSize(i, aTmp);
            }
            break;
            case HANDLE_RELATIVE_FONT_HEIGHT_TEXT          :
            case HANDLE_RELATIVE_FONT_HEIGHT_INDICES       :
            case HANDLE_RELATIVE_FONT_HEIGHT_FUNCTIONS     :
            case HANDLE_RELATIVE_FONT_HEIGHT_OPERATORS     :
            case HANDLE_RELATIVE_FONT_HEIGHT_LIMITS        :
            {
                sal_Int16 nVal = 0;
                *pValues >>= nVal;
                if(nVal < 1)
                    throw IllegalArgumentException();
                aFormat.SetRelSize((*ppEntries)->mnMemberId, nVal);
            }
            break;

            case HANDLE_IS_TEXT_MODE                       :
            {
                aFormat.SetTextmode(*(sal_Bool*)(*pValues).getValue());
            }
            break;

            case HANDLE_GREEK_CHAR_STYLE                    :
            {
                sal_Int16 nVal = 0;
                *pValues >>= nVal;
                if (nVal < 0 || nVal > 2)
                    throw IllegalArgumentException();
                aFormat.SetGreekCharStyle( nVal );
            }
            break;

            case HANDLE_ALIGNMENT                          :
            {
                // SmHorAlign uses the same values as HorizontalAlignment
                sal_Int16 nVal = 0;
                *pValues >>= nVal;
                if(nVal < 0 || nVal > 2)
                    throw IllegalArgumentException();
                aFormat.SetHorAlign((SmHorAlign)nVal);
            }
            break;

            case HANDLE_RELATIVE_SPACING                   :
            case HANDLE_RELATIVE_LINE_SPACING              :
            case HANDLE_RELATIVE_ROOT_SPACING              :
            case HANDLE_RELATIVE_INDEX_SUPERSCRIPT         :
            case HANDLE_RELATIVE_INDEX_SUBSCRIPT           :
            case HANDLE_RELATIVE_FRACTION_NUMERATOR_HEIGHT :
            case HANDLE_RELATIVE_FRACTION_DENOMINATOR_DEPTH:
            case HANDLE_RELATIVE_FRACTION_BAR_EXCESS_LENGTH:
            case HANDLE_RELATIVE_FRACTION_BAR_LINE_WEIGHT  :
            case HANDLE_RELATIVE_UPPER_LIMIT_DISTANCE      :
            case HANDLE_RELATIVE_LOWER_LIMIT_DISTANCE      :
            case HANDLE_RELATIVE_BRACKET_EXCESS_SIZE       :
            case HANDLE_RELATIVE_BRACKET_DISTANCE          :
            case HANDLE_RELATIVE_SCALE_BRACKET_EXCESS_SIZE :
            case HANDLE_RELATIVE_MATRIX_LINE_SPACING       :
            case HANDLE_RELATIVE_MATRIX_COLUMN_SPACING     :
            case HANDLE_RELATIVE_SYMBOL_PRIMARY_HEIGHT     :
            case HANDLE_RELATIVE_SYMBOL_MINIMUM_HEIGHT     :
            case HANDLE_RELATIVE_OPERATOR_EXCESS_SIZE      :
            case HANDLE_RELATIVE_OPERATOR_SPACING          :
            case HANDLE_LEFT_MARGIN               :
            case HANDLE_RIGHT_MARGIN              :
            case HANDLE_TOP_MARGIN                :
            case HANDLE_BOTTOM_MARGIN             :
            {
                sal_Int16 nVal = 0;
                *pValues >>= nVal;
                if(nVal < 0)
                    throw IllegalArgumentException();
                aFormat.SetDistance((*ppEntries)->mnMemberId, nVal);
            }
            break;
            case HANDLE_IS_SCALE_ALL_BRACKETS              :
                aFormat.SetScaleNormalBrackets(*(sal_Bool*)(*pValues).getValue());
            break;
            case HANDLE_PRINTER_NAME:
            {
                // embedded documents just ignore this property for now
                if ( pDocSh->GetCreateMode() != SFX_CREATE_MODE_EMBEDDED )
                {
                    SfxPrinter *pPrinter = pDocSh->GetPrinter ( );
                    if (pPrinter)
                    {
                        OUString sPrinterName;
                        if (*pValues >>= sPrinterName )
                        {
                            if ( sPrinterName.getLength() )
                            {
                                SfxPrinter *pNewPrinter = new SfxPrinter ( pPrinter->GetOptions().Clone(), sPrinterName );
                                if (pNewPrinter->IsKnown())
                                    pDocSh->SetPrinter ( pNewPrinter );
                                else
                                    delete pNewPrinter;
                            }
                        }
                        else
                            throw IllegalArgumentException();
                    }
                }
            }
            break;
            case HANDLE_PRINTER_SETUP:
            {
                Sequence < sal_Int8 > aSequence;
                if ( *pValues >>= aSequence )
                {
                    sal_uInt32 nSize = aSequence.getLength();
                    SvMemoryStream aStream ( aSequence.getArray(), nSize, STREAM_READ );
                    aStream.Seek ( STREAM_SEEK_TO_BEGIN );
                    static sal_uInt16 __READONLY_DATA nRange[] =
                    {
                        SID_PRINTSIZE,       SID_PRINTSIZE,
                        SID_PRINTZOOM,       SID_PRINTZOOM,
                        SID_PRINTTITLE,      SID_PRINTTITLE,
                        SID_PRINTTEXT,       SID_PRINTTEXT,
                        SID_PRINTFRAME,      SID_PRINTFRAME,
                        SID_NO_RIGHT_SPACES, SID_NO_RIGHT_SPACES,
                        0
                    };
                    SfxItemSet *pItemSet = new SfxItemSet( pDocSh->GetPool(), nRange );
                    SmModule *pp = SM_MOD();
                    pp->GetConfig()->ConfigToItemSet(*pItemSet);
                    SfxPrinter *pPrinter = SfxPrinter::Create ( aStream, pItemSet );

                    pDocSh->SetPrinter( pPrinter );
                }
                else
                    throw IllegalArgumentException();
            }
            break;
            case HANDLE_SYMBOLS:
            {
                // this is set
                Sequence < SymbolDescriptor > aSequence;
                if ( *pValues >>= aSequence )
                {
                    sal_uInt32 nSize = aSequence.getLength();
                    SmModule *pp = SM_MOD();
                    SmSymbolManager &rManager = pp->GetSymbolManager();
                    SymbolDescriptor *pDescriptor = aSequence.getArray();
                    for (sal_uInt32 i = 0; i < nSize ; i++, pDescriptor++)
                    {
                        Font aFont;
                        aFont.SetName ( pDescriptor->sFontName );
                        aFont.SetCharSet ( static_cast < rtl_TextEncoding > (pDescriptor->nCharSet) );
                        aFont.SetFamily ( static_cast < FontFamily > (pDescriptor->nFamily ) );
                        aFont.SetPitch  ( static_cast < FontPitch >  (pDescriptor->nPitch ) );
                        aFont.SetWeight ( static_cast < FontWeight > (pDescriptor->nWeight ) );
                        aFont.SetItalic ( static_cast < FontItalic > (pDescriptor->nItalic ) );
                        SmSym aSymbol ( pDescriptor->sName, aFont, static_cast < sal_Unicode > (pDescriptor->nCharacter),
                                        pDescriptor->sSymbolSet );
                        aSymbol.SetExportName ( pDescriptor->sExportName );
                        aSymbol.SetDocSymbol( sal_True );
                        rManager.AddOrReplaceSymbol ( aSymbol );
                    }
                }
                else
                    throw IllegalArgumentException();
            }
            break;
            // --> PB 2004-08-25 #i33095# Security Options
            case HANDLE_LOAD_READONLY :
            {
                if ( (*pValues).getValueType() != ::getBooleanCppuType() )
                    throw IllegalArgumentException();
                sal_Bool bReadonly = sal_False;
                if ( *pValues >>= bReadonly )
                    pDocSh->SetLoadReadonly( bReadonly );
                break;
            }
            // <--
        }
    }

    pDocSh->SetFormat( aFormat );

    // #i67283# since about all of the above changes are likely to change
    // the formula size we have to recalculate the vis-area now
    pDocSh->SetVisArea( Rectangle( Point(0, 0), pDocSh->GetSize() ) );
}

void SmModel::_getPropertyValues( const PropertyMapEntry **ppEntries, Any *pValue )
    throw( UnknownPropertyException, WrappedTargetException )
{
    SmDocShell *pDocSh = static_cast < SmDocShell * > (GetObjectShell());

    if ( NULL == pDocSh )
        throw UnknownPropertyException();

    const SmFormat & aFormat = pDocSh->GetFormat();

    for (; *ppEntries; ppEntries++, pValue++ )
    {
        switch ( (*ppEntries)->mnHandle )
        {
            case HANDLE_FORMULA:
                *pValue <<= OUString(pDocSh->GetText());
            break;
            case HANDLE_FONT_NAME_VARIABLES                :
            case HANDLE_FONT_NAME_FUNCTIONS                :
            case HANDLE_FONT_NAME_NUMBERS                  :
            case HANDLE_FONT_NAME_TEXT                     :
            case HANDLE_CUSTOM_FONT_NAME_SERIF             :
            case HANDLE_CUSTOM_FONT_NAME_SANS              :
            case HANDLE_CUSTOM_FONT_NAME_FIXED             :
            {
                const SmFace &  rFace = aFormat.GetFont((*ppEntries)->mnMemberId);
                *pValue <<= OUString(rFace.GetName());
            }
            break;
            case HANDLE_CUSTOM_FONT_FIXED_POSTURE:
            case HANDLE_CUSTOM_FONT_SANS_POSTURE :
            case HANDLE_CUSTOM_FONT_SERIF_POSTURE:
            case HANDLE_FONT_VARIABLES_POSTURE   :
            case HANDLE_FONT_FUNCTIONS_POSTURE   :
            case HANDLE_FONT_NUMBERS_POSTURE     :
            case HANDLE_FONT_TEXT_POSTURE        :
            {
                const SmFace &  rFace = aFormat.GetFont((*ppEntries)->mnMemberId);
                sal_Bool bVal = IsItalic( rFace );
                (*pValue).setValue(&bVal, *(*ppEntries)->mpType);
            }
            break;
            case HANDLE_CUSTOM_FONT_FIXED_WEIGHT :
            case HANDLE_CUSTOM_FONT_SANS_WEIGHT  :
            case HANDLE_CUSTOM_FONT_SERIF_WEIGHT :
            case HANDLE_FONT_VARIABLES_WEIGHT    :
            case HANDLE_FONT_FUNCTIONS_WEIGHT    :
            case HANDLE_FONT_NUMBERS_WEIGHT      :
            case HANDLE_FONT_TEXT_WEIGHT         :
            {
                const SmFace &  rFace = aFormat.GetFont((*ppEntries)->mnMemberId);
                sal_Bool bVal = IsBold( rFace ); // bold?
                (*pValue).setValue(&bVal, *(*ppEntries)->mpType);
            }
            break;
            case HANDLE_BASE_FONT_HEIGHT                   :
            {
                // Point!
                sal_Int16 nVal = static_cast < sal_Int16 > (aFormat.GetBaseSize().Height());
                nVal = static_cast < sal_Int16 > (MM100_TO_TWIP(nVal));
                nVal = (nVal + 10) / 20;
                *pValue <<= nVal;
            }
            break;
            case HANDLE_RELATIVE_FONT_HEIGHT_TEXT           :
            case HANDLE_RELATIVE_FONT_HEIGHT_INDICES       :
            case HANDLE_RELATIVE_FONT_HEIGHT_FUNCTIONS     :
            case HANDLE_RELATIVE_FONT_HEIGHT_OPERATORS     :
            case HANDLE_RELATIVE_FONT_HEIGHT_LIMITS        :
                *pValue <<= (sal_Int16) aFormat.GetRelSize((*ppEntries)->mnMemberId);
            break;

            case HANDLE_IS_TEXT_MODE                       :
            {
                sal_Bool bVal = aFormat.IsTextmode();
                (*pValue).setValue(&bVal, ::getBooleanCppuType());
            }
            break;

            case HANDLE_GREEK_CHAR_STYLE                    :
                *pValue <<= (sal_Int16)aFormat.GetGreekCharStyle();
            break;

            case HANDLE_ALIGNMENT                          :
                // SmHorAlign uses the same values as HorizontalAlignment
                *pValue <<= (sal_Int16)aFormat.GetHorAlign();
            break;

            case HANDLE_RELATIVE_SPACING                   :
            case HANDLE_RELATIVE_LINE_SPACING              :
            case HANDLE_RELATIVE_ROOT_SPACING              :
            case HANDLE_RELATIVE_INDEX_SUPERSCRIPT         :
            case HANDLE_RELATIVE_INDEX_SUBSCRIPT           :
            case HANDLE_RELATIVE_FRACTION_NUMERATOR_HEIGHT :
            case HANDLE_RELATIVE_FRACTION_DENOMINATOR_DEPTH:
            case HANDLE_RELATIVE_FRACTION_BAR_EXCESS_LENGTH:
            case HANDLE_RELATIVE_FRACTION_BAR_LINE_WEIGHT  :
            case HANDLE_RELATIVE_UPPER_LIMIT_DISTANCE      :
            case HANDLE_RELATIVE_LOWER_LIMIT_DISTANCE      :
            case HANDLE_RELATIVE_BRACKET_EXCESS_SIZE       :
            case HANDLE_RELATIVE_BRACKET_DISTANCE          :
            case HANDLE_RELATIVE_SCALE_BRACKET_EXCESS_SIZE :
            case HANDLE_RELATIVE_MATRIX_LINE_SPACING       :
            case HANDLE_RELATIVE_MATRIX_COLUMN_SPACING     :
            case HANDLE_RELATIVE_SYMBOL_PRIMARY_HEIGHT     :
            case HANDLE_RELATIVE_SYMBOL_MINIMUM_HEIGHT     :
            case HANDLE_RELATIVE_OPERATOR_EXCESS_SIZE      :
            case HANDLE_RELATIVE_OPERATOR_SPACING          :
            case HANDLE_LEFT_MARGIN               :
            case HANDLE_RIGHT_MARGIN              :
            case HANDLE_TOP_MARGIN                :
            case HANDLE_BOTTOM_MARGIN             :
                *pValue <<= (sal_Int16)aFormat.GetDistance((*ppEntries)->mnMemberId);
            break;
            case HANDLE_IS_SCALE_ALL_BRACKETS              :
            {
                sal_Bool bVal = aFormat.IsScaleNormalBrackets();
                (*pValue).setValue(&bVal, ::getBooleanCppuType());
            }
            break;
            case HANDLE_PRINTER_NAME:
            {
                SfxPrinter *pPrinter = pDocSh->GetPrinter ( );
                *pValue <<= pPrinter ? OUString ( pPrinter->GetName()) : OUString();
            }
            break;
            case HANDLE_PRINTER_SETUP:
            {
                SfxPrinter *pPrinter = pDocSh->GetPrinter ();
                if (pPrinter)
                {
                    SvMemoryStream aStream;
                    pPrinter->Store( aStream );
                    aStream.Seek ( STREAM_SEEK_TO_END );
                    sal_uInt32 nSize = aStream.Tell();
                    aStream.Seek ( STREAM_SEEK_TO_BEGIN );
                    Sequence < sal_Int8 > aSequence ( nSize );
                    aStream.Read ( aSequence.getArray(), nSize );
                    *pValue <<= aSequence;
                }
            }
            break;
            case HANDLE_SYMBOLS:
            case HANDLE_USED_SYMBOLS:
            {
                const bool bUsedSymbolsOnly = (*ppEntries)->mnHandle == HANDLE_USED_SYMBOLS;
                const std::set< rtl::OUString > &rUsedSymbols = pDocSh->GetUsedSymbols();

                // this is get
                SmModule *pp = SM_MOD();
                const SmSymbolManager &rManager = pp->GetSymbolManager();
                vector < const SmSym * > aVector;

                const SymbolPtrVec_t aSymbols( rManager.GetSymbols() );
                size_t nCount = 0;
                for (size_t i = 0; i < aSymbols.size(); ++i)
                {
                    const SmSym * pSymbol = aSymbols[ i ];
                    const bool bIsUsedSymbol = rUsedSymbols.find( pSymbol->GetName() ) != rUsedSymbols.end();
                    if (pSymbol && !pSymbol->IsPredefined() &&
                        (!bUsedSymbolsOnly || bIsUsedSymbol))
                    {
                        aVector.push_back ( pSymbol );
                        nCount++;
                    }
                }
                Sequence < SymbolDescriptor > aSequence ( nCount );
                SymbolDescriptor * pDescriptor = aSequence.getArray();

                vector < const SmSym * >::const_iterator aIter = aVector.begin(), aEnd = aVector.end();
                for(; aIter != aEnd; pDescriptor++, aIter++)
                {
                    pDescriptor->sName = (*aIter)->GetName();
                    pDescriptor->sExportName = (*aIter)->GetExportName();
                    pDescriptor->sSymbolSet = (*aIter)->GetSymbolSetName();
                    pDescriptor->nCharacter = static_cast < sal_Int32 > ((*aIter)->GetCharacter());

                    Font rFont = (*aIter)->GetFace();
                    pDescriptor->sFontName = rFont.GetName();
                    pDescriptor->nCharSet  = sal::static_int_cast< sal_Int16 >(rFont.GetCharSet());
                    pDescriptor->nFamily   = sal::static_int_cast< sal_Int16 >(rFont.GetFamily());
                    pDescriptor->nPitch    = sal::static_int_cast< sal_Int16 >(rFont.GetPitch());
                    pDescriptor->nWeight   = sal::static_int_cast< sal_Int16 >(rFont.GetWeight());
                    pDescriptor->nItalic   = sal::static_int_cast< sal_Int16 >(rFont.GetItalic());
                }
                *pValue <<= aSequence;
            }
            break;
            case HANDLE_BASIC_LIBRARIES:
                *pValue <<= pDocSh->GetBasicContainer();
            break;
            case HANDLE_DIALOG_LIBRARIES:
                *pValue <<= pDocSh->GetDialogContainer();
            break;
            case HANDLE_RUNTIME_UID:
                *pValue <<= getRuntimeUID();
            break;
            // --> PB 2004-08-25 #i33095# Security Options
            case HANDLE_LOAD_READONLY :
            {
                 *pValue <<= pDocSh->IsLoadReadonly();
                break;
            }
            // <--
            // --> 3.7.2010 #i972#
            case HANDLE_BASELINE:
            {
                if ( !pDocSh->pTree )
                    pDocSh->Parse();
                if ( pDocSh->pTree )
                {
                    if ( !pDocSh->IsFormulaArranged() )
                        pDocSh->ArrangeFormula();

                    *pValue <<= static_cast<sal_Int32>( pDocSh->pTree->GetFormulaBaseline() );
                }
            }
            break;
            // <--
        }
    }
}

//////////////////////////////////////////////////////////////////////

sal_Int32 SAL_CALL SmModel::getRendererCount(
        const uno::Any& /*rSelection*/,
        const uno::Sequence< beans::PropertyValue >& /*xOptions*/ )
    throw (IllegalArgumentException, RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    return 1;
}


static Size lcl_GuessPaperSize()
{
    Size aRes;
    Reference< XMultiServiceFactory >  xMgr( getProcessServiceFactory() );
    LocaleDataWrapper aLocWrp( xMgr, AllSettings().GetLocale() );
    if( MEASURE_METRIC == aLocWrp.getMeasurementSystemEnum() )
    {
        // in 100th mm
        PaperInfo aInfo( PAPER_A4 );
        aRes.Width()  = aInfo.getWidth();
        aRes.Height() = aInfo.getHeight();
    }
    else
    {
        // in 100th mm
        PaperInfo aInfo( PAPER_LETTER );
        aRes.Width()  = aInfo.getWidth();
        aRes.Height() = aInfo.getHeight();
    }
    return aRes;
}

uno::Sequence< beans::PropertyValue > SAL_CALL SmModel::getRenderer(
        sal_Int32 nRenderer,
        const uno::Any& /*rSelection*/,
        const uno::Sequence< beans::PropertyValue >& /*rxOptions*/ )
    throw (IllegalArgumentException, RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());

    if (0 != nRenderer)
        throw IllegalArgumentException();

    SmDocShell *pDocSh = static_cast < SmDocShell * >( GetObjectShell() );
    if (!pDocSh)
        throw RuntimeException();

    SmPrinterAccess aPrinterAccess( *pDocSh );
    Printer *pPrinter = aPrinterAccess.GetPrinter();
    //Point   aPrtPageOffset( pPrinter->GetPageOffset() );
    Size    aPrtPaperSize ( pPrinter->GetPaperSize() );

    // if paper size is 0 (usually if no 'real' printer is found),
    // guess the paper size
    if (aPrtPaperSize.Height() == 0 || aPrtPaperSize.Width() == 0)
        aPrtPaperSize = lcl_GuessPaperSize();
    awt::Size   aPageSize( aPrtPaperSize.Width(), aPrtPaperSize.Height() );

    uno::Sequence< beans::PropertyValue > aRenderer(1);
    PropertyValue  &rValue = aRenderer.getArray()[0];
    rValue.Name  = OUString( RTL_CONSTASCII_USTRINGPARAM( "PageSize" ) );
    rValue.Value <<= aPageSize;

    if (!m_pPrintUIOptions)
        m_pPrintUIOptions = new SmPrintUIOptions();
    m_pPrintUIOptions->appendPrintUIOptions( aRenderer );

    return aRenderer;
}

void SAL_CALL SmModel::render(
        sal_Int32 nRenderer,
        const uno::Any& rSelection,
        const uno::Sequence< beans::PropertyValue >& rxOptions )
    throw (IllegalArgumentException, RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());

    if (0 != nRenderer)
        throw IllegalArgumentException();

    SmDocShell *pDocSh = static_cast < SmDocShell * >( GetObjectShell() );
    if (!pDocSh)
        throw RuntimeException();

    // get device to be rendered in
    uno::Reference< awt::XDevice >  xRenderDevice;
    for (sal_Int32 i = 0, nCount = rxOptions.getLength();  i < nCount;  ++i)
    {
        if( rxOptions[i].Name == OUString( RTL_CONSTASCII_USTRINGPARAM( "RenderDevice" ) ) )
            rxOptions[i].Value >>= xRenderDevice;
    }

    if (xRenderDevice.is())
    {
        VCLXDevice*   pDevice = VCLXDevice::GetImplementation( xRenderDevice );
        OutputDevice* pOut = pDevice ? pDevice->GetOutputDevice() : NULL;

        if (!pOut)
            throw RuntimeException();

        pOut->SetMapMode( MAP_100TH_MM );

        uno::Reference< frame::XModel > xModel;
        rSelection >>= xModel;
        if (xModel == pDocSh->GetModel())
        {
            //!! when called via API we may not have an active view
            //!! thus we go and look for a view that can be used.
            const TypeId aTypeId = TYPE( SmViewShell );
            SfxViewShell* pViewSh = SfxViewShell::GetFirst( &aTypeId, sal_False /* search non-visible views as well*/ );
            while (pViewSh && pViewSh->GetObjectShell() != pDocSh)
                pViewSh = SfxViewShell::GetNext( *pViewSh, &aTypeId, sal_False /* search non-visible views as well*/ );
            SmViewShell *pView = PTR_CAST( SmViewShell, pViewSh );
            DBG_ASSERT( pView, "SmModel::render : no SmViewShell found" );

            if (pView)
            {
                SmPrinterAccess aPrinterAccess( *pDocSh );
                Printer *pPrinter = aPrinterAccess.GetPrinter();

                Size    aPrtPaperSize ( pPrinter->GetPaperSize() );
                Size    aOutputSize   ( pPrinter->GetOutputSize() );
                Point   aPrtPageOffset( pPrinter->GetPageOffset() );

                // no real printer ??
                if (aPrtPaperSize.Height() == 0 || aPrtPaperSize.Width() == 0)
                {
                    aPrtPaperSize = lcl_GuessPaperSize();
                    // factors from Windows DIN A4
                    aOutputSize    = Size( (long)(aPrtPaperSize.Width()  * 0.941),
                                           (long)(aPrtPaperSize.Height() * 0.961));
                    aPrtPageOffset = Point( (long)(aPrtPaperSize.Width()  * 0.0250),
                                            (long)(aPrtPaperSize.Height() * 0.0214));
                }
                Point   aZeroPoint;
                Rectangle OutputRect( aZeroPoint, aOutputSize );


                // set minimum top and bottom border
                if (aPrtPageOffset.Y() < 2000)
                    OutputRect.Top() += 2000 - aPrtPageOffset.Y();
                if ((aPrtPaperSize.Height() - (aPrtPageOffset.Y() + OutputRect.Bottom())) < 2000)
                    OutputRect.Bottom() -= 2000 - (aPrtPaperSize.Height() -
                                                (aPrtPageOffset.Y() + OutputRect.Bottom()));

                // set minimum left and right border
                if (aPrtPageOffset.X() < 2500)
                    OutputRect.Left() += 2500 - aPrtPageOffset.X();
                if ((aPrtPaperSize.Width() - (aPrtPageOffset.X() + OutputRect.Right())) < 1500)
                    OutputRect.Right() -= 1500 - (aPrtPaperSize.Width() -
                                                (aPrtPageOffset.X() + OutputRect.Right()));

                if (!m_pPrintUIOptions)
                    m_pPrintUIOptions = new SmPrintUIOptions();
                m_pPrintUIOptions->processProperties( rxOptions );

                pView->Impl_Print( *pOut, *m_pPrintUIOptions, Rectangle( OutputRect ), Point() );

                // release SmPrintUIOptions when everything is done.
                // That way, when SmPrintUIOptions is needed again it will read the latest configuration settings in its c-tor.
                if (m_pPrintUIOptions->getBoolValue( "IsLastPage", sal_False ))
                {
                    delete m_pPrintUIOptions;   m_pPrintUIOptions = 0;
                }
            }
        }
    }
}

void SAL_CALL SmModel::setParent( const uno::Reference< uno::XInterface >& xParent)
        throw( lang::NoSupportException, uno::RuntimeException )
{
    ::vos::OGuard aGuard( Application::GetSolarMutex() );
    SfxBaseModel::setParent( xParent );
    uno::Reference< lang::XUnoTunnel > xParentTunnel( xParent, uno::UNO_QUERY );
    if ( xParentTunnel.is() )
    {
        SvGlobalName aSfxIdent( SFX_GLOBAL_CLASSID );
        SfxObjectShell* pDoc = reinterpret_cast<SfxObjectShell *>(xParentTunnel->getSomething(
                                        uno::Sequence< sal_Int8 >( aSfxIdent.GetByteSequence() ) ) );
        if ( pDoc )
            GetObjectShell()->OnDocumentPrinterChanged( pDoc->GetDocumentPrinter() );
    }
}

