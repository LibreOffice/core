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

#include <sal/config.h>
#include <sal/log.hxx>

#include <utility>

#include <o3tl/any.hxx>
#include <sfx2/printer.hxx>
#include <svl/itemprop.hxx>
#include <svl/itemset.hxx>
#include <vcl/svapp.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/settings.hxx>
#include <vcl/print.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/formula/SymbolDescriptor.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <comphelper/propertysetinfo.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <editeng/paperinf.hxx>
#include <unotools/moduleoptions.hxx>
#include <tools/mapunit.hxx>
#include <tools/stream.hxx>

#include <unomodel.hxx>
#include <document.hxx>
#include <view.hxx>
#include <symbol.hxx>
#include <starmath.hrc>
#include <strings.hrc>
#include <smmod.hxx>
#include <cfgitem.hxx>

using namespace ::cppu;
using namespace ::comphelper;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::formula;
using namespace ::com::sun::star::view;
using namespace ::com::sun::star::script;

SmPrintUIOptions::SmPrintUIOptions()
{
    SmModule *pp = SM_MOD();
    SmMathConfig *pConfig = pp->GetConfig();
    SAL_WARN_IF( !pConfig, "starmath", "SmConfig not found" );
    if (!pConfig)
        return;

    sal_Int32 nNumProps = 10, nIdx=0;

    // create sequence of print UI options
    // (Actually IsIgnoreSpacesRight is a parser option. Without it we need only 8 properties here.)
    m_aUIProperties.resize( nNumProps );

    // load the math PrinterOptions into the custom tab
    m_aUIProperties[nIdx].Name = "OptionsUIFile";
    m_aUIProperties[nIdx++].Value <<= u"modules/smath/ui/printeroptions.ui"_ustr;

    // create Section for formula (results in an extra tab page in dialog)
    SvtModuleOptions aOpt;
    OUString aAppGroupname(
        SmResId( RID_PRINTUIOPT_PRODNAME ).
            replaceFirst( "%s", aOpt.GetModuleName( SvtModuleOptions::EModule::MATH ) ) );
    m_aUIProperties[nIdx++].Value = setGroupControlOpt(u"tabcontrol-page2"_ustr, aAppGroupname, u".HelpID:vcl:PrintDialog:TabPage:AppPage"_ustr);

    // create subgroup for print options
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt(u"contents"_ustr, SmResId( RID_PRINTUIOPT_CONTENTS ), OUString());

    // create a bool option for title row (matches to SID_PRINTTITLE)
    m_aUIProperties[nIdx++].Value = setBoolControlOpt(u"title"_ustr, SmResId( RID_PRINTUIOPT_TITLE ),
                                                  u".HelpID:vcl:PrintDialog:TitleRow:CheckBox"_ustr,
                                                  PRTUIOPT_TITLE_ROW,
                                                  pConfig->IsPrintTitle());
    // create a bool option for formula text (matches to SID_PRINTTEXT)
    m_aUIProperties[nIdx++].Value = setBoolControlOpt(u"formulatext"_ustr, SmResId( RID_PRINTUIOPT_FRMLTXT ),
                                                  u".HelpID:vcl:PrintDialog:FormulaText:CheckBox"_ustr,
                                                  PRTUIOPT_FORMULA_TEXT,
                                                  pConfig->IsPrintFormulaText());
    // create a bool option for border (matches to SID_PRINTFRAME)
    m_aUIProperties[nIdx++].Value = setBoolControlOpt(u"borders"_ustr, SmResId( RID_PRINTUIOPT_BORDERS ),
                                                  u".HelpID:vcl:PrintDialog:Border:CheckBox"_ustr,
                                                  PRTUIOPT_BORDER,
                                                  pConfig->IsPrintFrame());

    // create subgroup for print format
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt(u"size"_ustr, SmResId( RID_PRINTUIOPT_SIZE ), OUString());

    // create a radio button group for print format (matches to SID_PRINTSIZE)
    Sequence< OUString > aChoices{
        SmResId( RID_PRINTUIOPT_ORIGSIZE ),
        SmResId( RID_PRINTUIOPT_FITTOPAGE ),
        SmResId( RID_PRINTUIOPT_SCALING )
    };
    Sequence< OUString > aHelpIds{
        u".HelpID:vcl:PrintDialog:PrintFormat:RadioButton:0"_ustr,
        u".HelpID:vcl:PrintDialog:PrintFormat:RadioButton:1"_ustr,
        u".HelpID:vcl:PrintDialog:PrintFormat:RadioButton:2"_ustr
    };
    Sequence< OUString > aWidgetIds{
        u"originalsize"_ustr,
        u"fittopage"_ustr,
        u"scaling"_ustr
    };
    OUString aPrintFormatProp( PRTUIOPT_PRINT_FORMAT );
    m_aUIProperties[nIdx++].Value = setChoiceRadiosControlOpt(aWidgetIds, OUString(),
                                                    aHelpIds,
                                                    aPrintFormatProp,
                                                    aChoices, static_cast< sal_Int32 >(pConfig->GetPrintSize())
                                                    );

    // create a numeric box for scale dependent on PrintFormat = "Scaling" (matches to SID_PRINTZOOM)
    vcl::PrinterOptionsHelper::UIControlOptions aRangeOpt( aPrintFormatProp, 2, true );
    m_aUIProperties[nIdx++].Value = setRangeControlOpt(u"scalingspin"_ustr, OUString(),
                                                     u".HelpID:vcl:PrintDialog:PrintScale:NumericField"_ustr,
                                                     PRTUIOPT_PRINT_SCALE,
                                                     pConfig->GetPrintZoomFactor(),    // initial value
                                                     10,     // min value
                                                     1000,   // max value
                                                     aRangeOpt);

    Sequence aHintNoLayoutPage{ comphelper::makePropertyValue(u"HintNoLayoutPage"_ustr, true) };
    m_aUIProperties[nIdx++].Value <<= aHintNoLayoutPage;

    assert(nIdx == nNumProps);
}



namespace {

enum SmModelPropertyHandles
{
    HANDLE_FORMULA,
    HANDLE_FONT_NAME_MATH,
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
    HANDLE_FONT_MATH_POSTURE,
    HANDLE_FONT_MATH_WEIGHT,
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
    HANDLE_IS_RIGHT_TO_LEFT,
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
    HANDLE_SAVE_THUMBNAIL,
    HANDLE_USED_SYMBOLS,
    HANDLE_BASIC_LIBRARIES,
    HANDLE_RUNTIME_UID,
    HANDLE_LOAD_READONLY,     // Security Options
    HANDLE_DIALOG_LIBRARIES,  // #i73329#
    HANDLE_BASELINE,
    HANDLE_INTEROP_GRAB_BAG,
    HANDLE_STARMATH_VERSION
};

}

static const rtl::Reference<PropertySetInfo> & lcl_createModelPropertyInfo ()
{
    static const PropertyMapEntry aModelPropertyInfoMap[] =
    {
        { u"Alignment"_ustr                        , HANDLE_ALIGNMENT                          ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  0                     },
        { u"BaseFontHeight"_ustr                   , HANDLE_BASE_FONT_HEIGHT                   ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  0                     },
        { u"BasicLibraries"_ustr                   , HANDLE_BASIC_LIBRARIES                    ,  cppu::UnoType<script::XLibraryContainer>::get(),  PropertyAttribute::READONLY,  0       },
        { u"BottomMargin"_ustr                     , HANDLE_BOTTOM_MARGIN                      ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_BOTTOMSPACE       },
        { u"CustomFontNameFixed"_ustr              , HANDLE_CUSTOM_FONT_NAME_FIXED             ,  ::cppu::UnoType<OUString>::get(),                                      PROPERTY_NONE,  FNT_FIXED             },
        { u"CustomFontNameSans"_ustr               , HANDLE_CUSTOM_FONT_NAME_SANS              ,  ::cppu::UnoType<OUString>::get(),                                      PROPERTY_NONE,  FNT_SANS              },
        { u"CustomFontNameSerif"_ustr              , HANDLE_CUSTOM_FONT_NAME_SERIF             ,  ::cppu::UnoType<OUString>::get(),                                      PROPERTY_NONE,  FNT_SERIF             },
        { u"DialogLibraries"_ustr                  , HANDLE_DIALOG_LIBRARIES                   ,  cppu::UnoType<script::XLibraryContainer>::get(),  PropertyAttribute::READONLY,  0       },
        { u"FontFixedIsBold"_ustr                  , HANDLE_CUSTOM_FONT_FIXED_WEIGHT           ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_FIXED             },
        { u"FontFixedIsItalic"_ustr                , HANDLE_CUSTOM_FONT_FIXED_POSTURE          ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_FIXED             },
        { u"FontFunctionsIsBold"_ustr              , HANDLE_FONT_FUNCTIONS_WEIGHT              ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_FUNCTION          },
        { u"FontFunctionsIsItalic"_ustr            , HANDLE_FONT_FUNCTIONS_POSTURE             ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_FUNCTION          },
        { u"FontMathIsBold"_ustr                   , HANDLE_FONT_MATH_WEIGHT                   ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_MATH              },
        { u"FontMathIsItalic"_ustr                 , HANDLE_FONT_MATH_POSTURE                  ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_MATH              },
        { u"FontNameFunctions"_ustr                , HANDLE_FONT_NAME_FUNCTIONS                ,  ::cppu::UnoType<OUString>::get(),                                      PROPERTY_NONE,  FNT_FUNCTION          },
        { u"FontNameMath"_ustr                     , HANDLE_FONT_NAME_MATH                     ,  ::cppu::UnoType<OUString>::get(),                                      PROPERTY_NONE,  FNT_MATH              },
        { u"FontNameNumbers"_ustr                  , HANDLE_FONT_NAME_NUMBERS                  ,  ::cppu::UnoType<OUString>::get(),                                      PROPERTY_NONE,  FNT_NUMBER            },
        { u"FontNameText"_ustr                     , HANDLE_FONT_NAME_TEXT                     ,  ::cppu::UnoType<OUString>::get(),                                      PROPERTY_NONE,  FNT_TEXT              },
        { u"FontNameVariables"_ustr                , HANDLE_FONT_NAME_VARIABLES                ,  ::cppu::UnoType<OUString>::get(),                                      PROPERTY_NONE,  FNT_VARIABLE          },
        { u"FontNumbersIsBold"_ustr                , HANDLE_FONT_NUMBERS_WEIGHT                ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_NUMBER            },
        { u"FontNumbersIsItalic"_ustr              , HANDLE_FONT_NUMBERS_POSTURE               ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_NUMBER            },
        { u"FontSansIsBold"_ustr                   , HANDLE_CUSTOM_FONT_SANS_WEIGHT            ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_SANS              },
        { u"FontSansIsItalic"_ustr                 , HANDLE_CUSTOM_FONT_SANS_POSTURE           ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_SANS              },
        { u"FontSerifIsBold"_ustr                  , HANDLE_CUSTOM_FONT_SERIF_WEIGHT           ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_SERIF             },
        { u"FontSerifIsItalic"_ustr                , HANDLE_CUSTOM_FONT_SERIF_POSTURE          ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_SERIF             },
        { u"FontTextIsBold"_ustr                   , HANDLE_FONT_TEXT_WEIGHT                   ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_TEXT              },
        { u"FontTextIsItalic"_ustr                 , HANDLE_FONT_TEXT_POSTURE                  ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_TEXT              },
        { u"FontVariablesIsBold"_ustr              , HANDLE_FONT_VARIABLES_WEIGHT              ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_VARIABLE          },
        { u"FontVariablesIsItalic"_ustr            , HANDLE_FONT_VARIABLES_POSTURE             ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_VARIABLE          },
        { u"Formula"_ustr                          , HANDLE_FORMULA                            ,  ::cppu::UnoType<OUString>::get(),                                      PROPERTY_NONE,  0                     },
        { u"IsScaleAllBrackets"_ustr               , HANDLE_IS_SCALE_ALL_BRACKETS              ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  0                     },
        { u"IsTextMode"_ustr                       , HANDLE_IS_TEXT_MODE                       ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  0                     },
        { u"IsRightToLeft"_ustr                    , HANDLE_IS_RIGHT_TO_LEFT                   ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  0                     },
        { u"GreekCharStyle"_ustr                   , HANDLE_GREEK_CHAR_STYLE                   ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  0                     },
        { u"LeftMargin"_ustr                       , HANDLE_LEFT_MARGIN                        ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_LEFTSPACE         },
        { u"PrinterName"_ustr                      , HANDLE_PRINTER_NAME                       ,  ::cppu::UnoType<OUString>::get(),                                      PROPERTY_NONE,  0                     },
        { u"PrinterSetup"_ustr                     , HANDLE_PRINTER_SETUP                      ,  cppu::UnoType<const Sequence < sal_Int8 >>::get(),                         PROPERTY_NONE,  0                     },
        { u"RelativeBracketDistance"_ustr          , HANDLE_RELATIVE_BRACKET_DISTANCE          ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_BRACKETSPACE      },
        { u"RelativeBracketExcessSize"_ustr        , HANDLE_RELATIVE_BRACKET_EXCESS_SIZE       ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_BRACKETSIZE       },
        { u"RelativeFontHeightFunctions"_ustr      , HANDLE_RELATIVE_FONT_HEIGHT_FUNCTIONS     ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  SIZ_FUNCTION          },
        { u"RelativeFontHeightIndices"_ustr        , HANDLE_RELATIVE_FONT_HEIGHT_INDICES       ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  SIZ_INDEX             },
        { u"RelativeFontHeightLimits"_ustr         , HANDLE_RELATIVE_FONT_HEIGHT_LIMITS        ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  SIZ_LIMITS            },
        { u"RelativeFontHeightOperators"_ustr      , HANDLE_RELATIVE_FONT_HEIGHT_OPERATORS     ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  SIZ_OPERATOR          },
        { u"RelativeFontHeightText"_ustr           , HANDLE_RELATIVE_FONT_HEIGHT_TEXT          ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  SIZ_TEXT              },
        { u"RelativeFractionBarExcessLength"_ustr  , HANDLE_RELATIVE_FRACTION_BAR_EXCESS_LENGTH,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_FRACTION          },
        { u"RelativeFractionBarLineWeight"_ustr    , HANDLE_RELATIVE_FRACTION_BAR_LINE_WEIGHT  ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_STROKEWIDTH       },
        { u"RelativeFractionDenominatorDepth"_ustr , HANDLE_RELATIVE_FRACTION_DENOMINATOR_DEPTH,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_DENOMINATOR       },
        { u"RelativeFractionNumeratorHeight"_ustr  , HANDLE_RELATIVE_FRACTION_NUMERATOR_HEIGHT ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_NUMERATOR         },
        { u"RelativeIndexSubscript"_ustr           , HANDLE_RELATIVE_INDEX_SUBSCRIPT           ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_SUBSCRIPT         },
        { u"RelativeIndexSuperscript"_ustr         , HANDLE_RELATIVE_INDEX_SUPERSCRIPT         ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_SUPERSCRIPT       },
        { u"RelativeLineSpacing"_ustr              , HANDLE_RELATIVE_LINE_SPACING              ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_VERTICAL          },
        { u"RelativeLowerLimitDistance"_ustr       , HANDLE_RELATIVE_LOWER_LIMIT_DISTANCE      ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_LOWERLIMIT        },
        { u"RelativeMatrixColumnSpacing"_ustr      , HANDLE_RELATIVE_MATRIX_COLUMN_SPACING     ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_MATRIXCOL         },
        { u"RelativeMatrixLineSpacing"_ustr        , HANDLE_RELATIVE_MATRIX_LINE_SPACING       ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_MATRIXROW         },
        { u"RelativeOperatorExcessSize"_ustr       , HANDLE_RELATIVE_OPERATOR_EXCESS_SIZE      ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_OPERATORSIZE      },
        { u"RelativeOperatorSpacing"_ustr          , HANDLE_RELATIVE_OPERATOR_SPACING          ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_OPERATORSPACE     },
        { u"RelativeRootSpacing"_ustr              , HANDLE_RELATIVE_ROOT_SPACING              ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_ROOT              },
        { u"RelativeScaleBracketExcessSize"_ustr   , HANDLE_RELATIVE_SCALE_BRACKET_EXCESS_SIZE ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_NORMALBRACKETSIZE },
        { u"RelativeSpacing"_ustr                  , HANDLE_RELATIVE_SPACING                   ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_HORIZONTAL        },
        { u"RelativeSymbolMinimumHeight"_ustr      , HANDLE_RELATIVE_SYMBOL_MINIMUM_HEIGHT     ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_ORNAMENTSPACE     },
        { u"RelativeSymbolPrimaryHeight"_ustr      , HANDLE_RELATIVE_SYMBOL_PRIMARY_HEIGHT     ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_ORNAMENTSIZE      },
        { u"RelativeUpperLimitDistance"_ustr       , HANDLE_RELATIVE_UPPER_LIMIT_DISTANCE      ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_UPPERLIMIT        },
        { u"RightMargin"_ustr                      , HANDLE_RIGHT_MARGIN                       ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_RIGHTSPACE        },
        { u"RuntimeUID"_ustr                       , HANDLE_RUNTIME_UID                        ,  cppu::UnoType<OUString>::get(),                                        PropertyAttribute::READONLY,  0       },
        { u"SaveThumbnail"_ustr                    , HANDLE_SAVE_THUMBNAIL                     ,  cppu::UnoType<bool>::get(),                                            PROPERTY_NONE,  0                     },
        { u"Symbols"_ustr                          , HANDLE_SYMBOLS                            ,  cppu::UnoType<Sequence < SymbolDescriptor >>::get(),                   PROPERTY_NONE,  0                     },
        { u"UserDefinedSymbolsInUse"_ustr          , HANDLE_USED_SYMBOLS                       ,  cppu::UnoType<Sequence < SymbolDescriptor >>::get(),                   PropertyAttribute::READONLY,  0       },
        { u"TopMargin"_ustr                        , HANDLE_TOP_MARGIN                         ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_TOPSPACE          },
        // #i33095# Security Options
        { u"LoadReadonly"_ustr                     , HANDLE_LOAD_READONLY                      ,  cppu::UnoType<bool>::get(),                                            PROPERTY_NONE,  0                     },
        // #i972#
        { u"BaseLine"_ustr                         , HANDLE_BASELINE                           ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  0                     },
        { u"InteropGrabBag"_ustr                   , HANDLE_INTEROP_GRAB_BAG                   ,  cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get(),           PROPERTY_NONE,  0                     },
        { u"SyntaxVersion"_ustr                    , HANDLE_STARMATH_VERSION                   ,  ::cppu::UnoType<sal_Int16>::get(),                             PROPERTY_NONE,  0                     },
    };
    static const rtl::Reference<PropertySetInfo> PROPS_INFO = new PropertySetInfo ( aModelPropertyInfoMap );
    return PROPS_INFO;
}

SmModel::SmModel(SmDocShell* pObjSh)
: SfxBaseModel(pObjSh)
, PropertySetHelper ( lcl_createModelPropertyInfo () )
{
}

SmModel::~SmModel() noexcept
{
}

SmDocShell* SmModel::GetSmDocShell() const { return static_cast<SmDocShell*>(GetObjectShell()); }

uno::Any SAL_CALL SmModel::queryInterface( const uno::Type& rType )
{
    uno::Any aRet =  ::cppu::queryInterface ( rType,
                                    // PropertySetHelper interfaces
                                    static_cast< XPropertySet* > ( this ),
                                    static_cast< XMultiPropertySet* > ( this ),
                                    // my own interfaces
                                    static_cast< XServiceInfo*  > ( this ),
                                    static_cast< XRenderable*  > ( this ) );
    if (!aRet.hasValue())
        aRet = SfxBaseModel::queryInterface ( rType );
    return aRet;
}

void SAL_CALL SmModel::acquire() noexcept
{
    OWeakObject::acquire();
}

void SAL_CALL SmModel::release() noexcept
{
    OWeakObject::release();
}

uno::Sequence< uno::Type > SAL_CALL SmModel::getTypes(  )
{
    return comphelper::concatSequences(SfxBaseModel::getTypes(),
        uno::Sequence  {
            cppu::UnoType<XServiceInfo>::get(),
            cppu::UnoType<XPropertySet>::get(),
            cppu::UnoType<XMultiPropertySet>::get(),
            cppu::UnoType<XRenderable>::get() });
}

const uno::Sequence< sal_Int8 > & SmModel::getUnoTunnelId()
{
    static const comphelper::UnoIdInit theSmModelUnoTunnelId;
    return theSmModelUnoTunnelId.getSeq();
}

sal_Int64 SAL_CALL SmModel::getSomething( const uno::Sequence< sal_Int8 >& rId )
{
    return comphelper::getSomethingImpl(rId, this,
                                        comphelper::FallbackToGetSomethingOf<SfxBaseModel>{});
}

static sal_Int16 lcl_AnyToINT16(const uno::Any& rAny)
{
    sal_Int16 nRet = 0;
    if( auto x = o3tl::tryAccess<double>(rAny) )
        nRet = static_cast<sal_Int16>(*x);
    else
        rAny >>= nRet;
    return nRet;
}

OUString SmModel::getImplementationName()
{
    return u"com.sun.star.comp.Math.FormulaDocument"_ustr;
}

sal_Bool SmModel::supportsService(const OUString& rServiceName)
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SmModel::getSupportedServiceNames()
{
    static constexpr OUString service1 = u"com.sun.star.document.OfficeDocument"_ustr;
    static constexpr OUString service2 = u"com.sun.star.formula.FormulaProperties"_ustr;
    return uno::Sequence<OUString>{ service1, service2 };
}

void SmModel::_setPropertyValues(const PropertyMapEntry** ppEntries, const Any* pValues)
{
    SolarMutexGuard aGuard;

    SmDocShell* pDocSh = GetSmDocShell();

    if ( nullptr == pDocSh )
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
            case HANDLE_FONT_NAME_MATH                     :
            case HANDLE_FONT_NAME_VARIABLES                :
            case HANDLE_FONT_NAME_FUNCTIONS                :
            case HANDLE_FONT_NAME_NUMBERS                  :
            case HANDLE_FONT_NAME_TEXT                     :
            case HANDLE_CUSTOM_FONT_NAME_SERIF             :
            case HANDLE_CUSTOM_FONT_NAME_SANS              :
            case HANDLE_CUSTOM_FONT_NAME_FIXED             :
            {
                OUString sFontName;
                *pValues >>= sFontName;
                if(sFontName.isEmpty())
                    throw IllegalArgumentException();
                maFonts[(*ppEntries)->mnMemberId].SetFamilyName(sFontName);
            }
            break;
            case HANDLE_CUSTOM_FONT_FIXED_POSTURE:
            case HANDLE_CUSTOM_FONT_SANS_POSTURE :
            case HANDLE_CUSTOM_FONT_SERIF_POSTURE:
            case HANDLE_FONT_MATH_POSTURE        :
            case HANDLE_FONT_VARIABLES_POSTURE   :
            case HANDLE_FONT_FUNCTIONS_POSTURE   :
            case HANDLE_FONT_NUMBERS_POSTURE     :
            case HANDLE_FONT_TEXT_POSTURE        :
            {
                std::optional<const bool> bVal = o3tl::tryAccess<bool>(*pValues);
                if(!bVal.has_value())
                    throw IllegalArgumentException();
                maFonts[(*ppEntries)->mnMemberId].SetItalic(*bVal ? ITALIC_NORMAL : ITALIC_NONE);
            }
            break;
            case HANDLE_CUSTOM_FONT_FIXED_WEIGHT :
            case HANDLE_CUSTOM_FONT_SANS_WEIGHT  :
            case HANDLE_CUSTOM_FONT_SERIF_WEIGHT :
            case HANDLE_FONT_MATH_WEIGHT         :
            case HANDLE_FONT_VARIABLES_WEIGHT    :
            case HANDLE_FONT_FUNCTIONS_WEIGHT    :
            case HANDLE_FONT_NUMBERS_WEIGHT      :
            case HANDLE_FONT_TEXT_WEIGHT         :
            {
                std::optional<const bool> bVal = o3tl::tryAccess<bool>(*pValues);
                if(!bVal.has_value())
                    throw IllegalArgumentException();
                maFonts[(*ppEntries)->mnMemberId].SetWeight(*bVal ? WEIGHT_BOLD : WEIGHT_NORMAL);
            }
            break;
            case HANDLE_BASE_FONT_HEIGHT                   :
            {
                // Point!
                sal_Int16 nVal = lcl_AnyToINT16(*pValues);
                if(nVal < 1)
                    throw IllegalArgumentException();
                Size aSize = aFormat.GetBaseSize();
                aSize.setHeight(o3tl::convert(nVal, o3tl::Length::pt, SmO3tlLengthUnit()));
                aFormat.SetBaseSize(aSize);

                // apply base size to fonts
                const Size aTmp( aFormat.GetBaseSize() );
                for (sal_uInt16  i = FNT_BEGIN;  i <= FNT_END;  i++)
                    maFonts[i].SetSize(aTmp);
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
                aFormat.SetTextmode(*o3tl::doAccess<bool>(*pValues));
            }
            break;

            case HANDLE_IS_RIGHT_TO_LEFT                   :
                aFormat.SetRightToLeft(*o3tl::doAccess<bool>(*pValues));
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
                aFormat.SetHorAlign(static_cast<SmHorAlign>(nVal));
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
                aFormat.SetScaleNormalBrackets(*o3tl::doAccess<bool>(*pValues));
            break;
            case HANDLE_PRINTER_NAME:
            {
                // embedded documents just ignore this property for now
                if ( pDocSh->GetCreateMode() != SfxObjectCreateMode::EMBEDDED )
                {
                    SfxPrinter *pPrinter = pDocSh->GetPrinter ( );
                    if (pPrinter)
                    {
                        OUString sPrinterName;
                        if ( !(*pValues >>= sPrinterName) )
                            throw IllegalArgumentException();

                        if ( !sPrinterName.isEmpty() )
                        {
                            VclPtrInstance<SfxPrinter> pNewPrinter( pPrinter->GetOptions().Clone(), sPrinterName );
                            if (pNewPrinter->IsKnown())
                                pDocSh->SetPrinter ( pNewPrinter );
                            else
                                pNewPrinter.disposeAndClear();
                        }
                    }
                }
            }
            break;
            case HANDLE_PRINTER_SETUP:
            {
                Sequence < sal_Int8 > aSequence;
                if ( !(*pValues >>= aSequence) )
                    throw IllegalArgumentException();

                sal_uInt32 nSize = aSequence.getLength();
                SvMemoryStream aStream ( aSequence.getArray(), nSize, StreamMode::READ );
                aStream.Seek ( STREAM_SEEK_TO_BEGIN );
                auto pItemSet = std::make_unique<SfxItemSetFixed<
                    SID_PRINTTITLE,      SID_PRINTTITLE,
                    SID_PRINTTEXT,       SID_PRINTTEXT,
                    SID_PRINTFRAME,      SID_PRINTFRAME,
                    SID_PRINTSIZE,       SID_PRINTSIZE,
                    SID_PRINTZOOM,       SID_PRINTZOOM,
                    SID_NO_RIGHT_SPACES, SID_NO_RIGHT_SPACES,
                    SID_SAVE_ONLY_USED_SYMBOLS, SID_SAVE_ONLY_USED_SYMBOLS,
                    SID_AUTO_CLOSE_BRACKETS,    SID_SMEDITWINDOWZOOM,
                    SID_INLINE_EDIT_ENABLE, SID_INLINE_EDIT_ENABLE>> ( SmDocShell::GetPool() );
                SmModule *pp = SM_MOD();
                pp->GetConfig()->ConfigToItemSet(*pItemSet);
                VclPtr<SfxPrinter> pPrinter = SfxPrinter::Create ( aStream, std::move(pItemSet) );

                pDocSh->SetPrinter( pPrinter );
            }
            break;
            case HANDLE_SYMBOLS:
            {
                // this is set
                Sequence < SymbolDescriptor > aSequence;
                if ( !(*pValues >>= aSequence) )
                    throw IllegalArgumentException();

                SmModule *pp = SM_MOD();
                SmSymbolManager &rManager = pp->GetSymbolManager();
                for (const SymbolDescriptor& rDescriptor : aSequence)
                {
                    vcl::Font aFont;
                    aFont.SetFamilyName ( rDescriptor.sFontName );
                    aFont.SetCharSet ( static_cast < rtl_TextEncoding > (rDescriptor.nCharSet) );
                    aFont.SetFamily ( static_cast < FontFamily > (rDescriptor.nFamily ) );
                    aFont.SetPitch  ( static_cast < FontPitch >  (rDescriptor.nPitch ) );
                    aFont.SetWeight ( static_cast < FontWeight > (rDescriptor.nWeight ) );
                    aFont.SetItalic ( static_cast < FontItalic > (rDescriptor.nItalic ) );
                    SmSym aSymbol ( rDescriptor.sName, aFont, static_cast < sal_Unicode > (rDescriptor.nCharacter),
                                    rDescriptor.sSymbolSet );
                    aSymbol.SetExportName ( rDescriptor.sExportName );
                    rManager.AddOrReplaceSymbol ( aSymbol );
                }
            }
            break;
            // #i33095# Security Options
            case HANDLE_LOAD_READONLY :
            {
                if ( (*pValues).getValueType() != cppu::UnoType<bool>::get() )
                    throw IllegalArgumentException();
                bool bReadonly = false;
                if ( *pValues >>= bReadonly )
                    pDocSh->SetLoadReadonly( bReadonly );
                break;
            }
            case HANDLE_INTEROP_GRAB_BAG:
                setGrabBagItem(*pValues);
            break;
            case HANDLE_SAVE_THUMBNAIL:
            {
                if ((*pValues).getValueType() != cppu::UnoType<bool>::get())
                    throw IllegalArgumentException();
                bool bThumbnail = false;
                if (*pValues >>= bThumbnail)
                    pDocSh->SetUseThumbnailSave(bThumbnail);
            }
            break;
            case HANDLE_STARMATH_VERSION:
                pDocSh->SetSmSyntaxVersion(pValues->get<sal_Int16>());
                break;
        }
    }

    // tdf#143213
    // Collect all font settings and apply them at the end, since the font name change can be seen
    // after italic or bold settings and would then override them.
    for (sal_uInt16 nFontDesc = FNT_BEGIN; nFontDesc <= FNT_END; ++nFontDesc)
    {
        const SmFace& rFont = maFonts[nFontDesc];
        if (rFont.GetFamilyName().isEmpty())
            continue;

        if (aFormat.GetFont(nFontDesc).GetFamilyName() != rFont.GetFamilyName())
        {
            const SmFace rOld = aFormat.GetFont(nFontDesc);

            SmFace aSet(rFont.GetFamilyName(), rOld.GetFontSize());
            aSet.SetBorderWidth(rOld.GetBorderWidth());
            aSet.SetAlignment(ALIGN_BASELINE);
            aFormat.SetFont(nFontDesc, aSet);
        }

        if (aFormat.GetFont(nFontDesc).GetItalic() != rFont.GetItalic())
        {
            vcl::Font aNewFont(aFormat.GetFont(nFontDesc));
            aNewFont.SetItalic(rFont.GetItalic());
            aFormat.SetFont(nFontDesc, aNewFont);
        }

        if (aFormat.GetFont(nFontDesc).GetWeight() != rFont.GetWeight())
        {
            vcl::Font aNewFont(aFormat.GetFont(nFontDesc));
            aNewFont.SetWeight(rFont.GetWeight());
            aFormat.SetFont(nFontDesc, aNewFont);
        }

        if (aFormat.GetFont(nFontDesc).GetFontSize() != rFont.GetFontSize())
        {
            aFormat.SetFontSize(nFontDesc, rFont.GetFontSize());
        }
    }

    pDocSh->SetFormat( aFormat );

    // #i67283# since about all of the above changes are likely to change
    // the formula size we have to recalculate the vis-area now
    pDocSh->SetVisArea( tools::Rectangle( Point(0, 0), pDocSh->GetSize() ) );
}

void SmModel::_getPropertyValues( const PropertyMapEntry **ppEntries, Any *pValue )
{
    SmDocShell* pDocSh = GetSmDocShell();

    if ( nullptr == pDocSh )
        throw UnknownPropertyException();

    const SmFormat & aFormat = pDocSh->GetFormat();

    for (; *ppEntries; ppEntries++, pValue++ )
    {
        switch ( (*ppEntries)->mnHandle )
        {
            case HANDLE_FORMULA:
                *pValue <<= pDocSh->GetText();
            break;
            case HANDLE_FONT_NAME_MATH                     :
            case HANDLE_FONT_NAME_VARIABLES                :
            case HANDLE_FONT_NAME_FUNCTIONS                :
            case HANDLE_FONT_NAME_NUMBERS                  :
            case HANDLE_FONT_NAME_TEXT                     :
            case HANDLE_CUSTOM_FONT_NAME_SERIF             :
            case HANDLE_CUSTOM_FONT_NAME_SANS              :
            case HANDLE_CUSTOM_FONT_NAME_FIXED             :
            {
                const SmFace &  rFace = aFormat.GetFont((*ppEntries)->mnMemberId);
                *pValue <<= rFace.GetFamilyName();
            }
            break;
            case HANDLE_CUSTOM_FONT_FIXED_POSTURE:
            case HANDLE_CUSTOM_FONT_SANS_POSTURE :
            case HANDLE_CUSTOM_FONT_SERIF_POSTURE:
            case HANDLE_FONT_MATH_POSTURE        :
            case HANDLE_FONT_VARIABLES_POSTURE   :
            case HANDLE_FONT_FUNCTIONS_POSTURE   :
            case HANDLE_FONT_NUMBERS_POSTURE     :
            case HANDLE_FONT_TEXT_POSTURE        :
            {
                const SmFace &  rFace = aFormat.GetFont((*ppEntries)->mnMemberId);
                *pValue <<= IsItalic( rFace );
            }
            break;
            case HANDLE_CUSTOM_FONT_FIXED_WEIGHT :
            case HANDLE_CUSTOM_FONT_SANS_WEIGHT  :
            case HANDLE_CUSTOM_FONT_SERIF_WEIGHT :
            case HANDLE_FONT_MATH_WEIGHT         :
            case HANDLE_FONT_VARIABLES_WEIGHT    :
            case HANDLE_FONT_FUNCTIONS_WEIGHT    :
            case HANDLE_FONT_NUMBERS_WEIGHT      :
            case HANDLE_FONT_TEXT_WEIGHT         :
            {
                const SmFace &  rFace = aFormat.GetFont((*ppEntries)->mnMemberId);
                *pValue <<= IsBold( rFace );
            }
            break;
            case HANDLE_BASE_FONT_HEIGHT                   :
            {
                // Point!
                *pValue <<= sal_Int16(o3tl::convert(aFormat.GetBaseSize().Height(),
                                                    SmO3tlLengthUnit(), o3tl::Length::pt));
            }
            break;
            case HANDLE_RELATIVE_FONT_HEIGHT_TEXT           :
            case HANDLE_RELATIVE_FONT_HEIGHT_INDICES       :
            case HANDLE_RELATIVE_FONT_HEIGHT_FUNCTIONS     :
            case HANDLE_RELATIVE_FONT_HEIGHT_OPERATORS     :
            case HANDLE_RELATIVE_FONT_HEIGHT_LIMITS        :
                *pValue <<= static_cast<sal_Int16>(aFormat.GetRelSize((*ppEntries)->mnMemberId));
            break;

            case HANDLE_IS_TEXT_MODE                       :
                *pValue <<= aFormat.IsTextmode();
            break;

            case HANDLE_IS_RIGHT_TO_LEFT                   :
                *pValue <<= aFormat.IsRightToLeft();
            break;

            case HANDLE_GREEK_CHAR_STYLE                    :
                *pValue <<= aFormat.GetGreekCharStyle();
            break;

            case HANDLE_ALIGNMENT                          :
                // SmHorAlign uses the same values as HorizontalAlignment
                *pValue <<= static_cast<sal_Int16>(aFormat.GetHorAlign());
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
                *pValue <<= static_cast<sal_Int16>(aFormat.GetDistance((*ppEntries)->mnMemberId));
            break;
            case HANDLE_IS_SCALE_ALL_BRACKETS              :
                *pValue <<= aFormat.IsScaleNormalBrackets();
            break;
            case HANDLE_PRINTER_NAME:
            {
                SfxPrinter *pPrinter = pDocSh->GetPrinter ( );
                *pValue <<= pPrinter ? pPrinter->GetName() : OUString();
            }
            break;
            case HANDLE_PRINTER_SETUP:
            {
                SfxPrinter *pPrinter = pDocSh->GetPrinter ();
                if (pPrinter)
                {
                    SvMemoryStream aStream;
                    pPrinter->Store( aStream );
                    sal_uInt32 nSize = aStream.TellEnd();
                    aStream.Seek ( STREAM_SEEK_TO_BEGIN );
                    Sequence < sal_Int8 > aSequence ( nSize );
                    aStream.ReadBytes(aSequence.getArray(), nSize);
                    *pValue <<= aSequence;
                }
            }
            break;
            case HANDLE_SYMBOLS:
            case HANDLE_USED_SYMBOLS:
            {
                const bool bUsedSymbolsOnly = (*ppEntries)->mnHandle == HANDLE_USED_SYMBOLS;
                const std::set< OUString > &rUsedSymbols = pDocSh->GetUsedSymbols();

                // this is get
                SmModule *pp = SM_MOD();
                const SmSymbolManager &rManager = pp->GetSymbolManager();
                std::vector < const SmSym * > aVector;

                const SymbolPtrVec_t aSymbols( rManager.GetSymbols() );
                for (const SmSym* pSymbol : aSymbols)
                {
                    if (pSymbol && !pSymbol->IsPredefined() &&
                        (!bUsedSymbolsOnly ||
                         rUsedSymbols.find( pSymbol->GetUiName() ) != rUsedSymbols.end()))
                        aVector.push_back ( pSymbol );
                }
                Sequence < SymbolDescriptor > aSequence ( aVector.size() );
                SymbolDescriptor * pDescriptor = aSequence.getArray();

                for (const SmSym* pSymbol : aVector)
                {
                    pDescriptor->sName = pSymbol->GetUiName();
                    pDescriptor->sExportName = pSymbol->GetExportName();
                    pDescriptor->sSymbolSet = pSymbol->GetSymbolSetName();
                    pDescriptor->nCharacter = static_cast < sal_Int32 > (pSymbol->GetCharacter());

                    vcl::Font rFont = pSymbol->GetFace();
                    pDescriptor->sFontName = rFont.GetFamilyName();
                    pDescriptor->nCharSet  = sal::static_int_cast< sal_Int16 >(rFont.GetCharSet());
                    pDescriptor->nFamily   = sal::static_int_cast< sal_Int16 >(rFont.GetFamilyType());
                    pDescriptor->nPitch    = sal::static_int_cast< sal_Int16 >(rFont.GetPitch());
                    pDescriptor->nWeight   = sal::static_int_cast< sal_Int16 >(rFont.GetWeight());
                    pDescriptor->nItalic   = sal::static_int_cast< sal_Int16 >(rFont.GetItalic());
                    pDescriptor++;
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
            // #i33095# Security Options
            case HANDLE_LOAD_READONLY :
            {
                *pValue <<= pDocSh->IsLoadReadonly();
                break;
            }
            // #i972#
            case HANDLE_BASELINE:
            {
                if ( !pDocSh->GetFormulaTree() )
                    pDocSh->Parse();
                if ( pDocSh->GetFormulaTree() )
                {
                    pDocSh->ArrangeFormula();

                    *pValue <<= static_cast<sal_Int32>(
                        o3tl::convert(pDocSh->GetFormulaTree()->GetFormulaBaseline(),
                                      SmO3tlLengthUnit(), o3tl::Length::mm100));
                }
                break;
            }
            case HANDLE_INTEROP_GRAB_BAG:
                getGrabBagItem(*pValue);
            break;
            case HANDLE_SAVE_THUMBNAIL:
            {
                *pValue <<= pDocSh->IsUseThumbnailSave();
            }
            break;
            case HANDLE_STARMATH_VERSION:
                *pValue <<= pDocSh->GetSmSyntaxVersion();
                break;
        }
    }
}


sal_Int32 SAL_CALL SmModel::getRendererCount(
        const uno::Any& /*rSelection*/,
        const uno::Sequence< beans::PropertyValue >& /*xOptions*/ )
{
    return 1;
}

uno::Sequence< beans::PropertyValue > SAL_CALL SmModel::getRenderer(
        sal_Int32 nRenderer,
        const uno::Any& /*rSelection*/,
        const uno::Sequence< beans::PropertyValue >& /*rxOptions*/ )
{
    SolarMutexGuard aGuard;

    if (0 != nRenderer)
        throw IllegalArgumentException();

    SmDocShell* pDocSh = GetSmDocShell();
    if (!pDocSh)
        throw RuntimeException();

    SmPrinterAccess aPrinterAccess( *pDocSh );
    Size aPrtPaperSize;
    if (Printer *pPrinter = aPrinterAccess.GetPrinter())
    {
        // tdf#157965: UNO methods are expected to return sizes in mm/100
        pPrinter->SetMapMode(MapMode(MapUnit::Map100thMM)); // reset in SmPrinterAccess dtor
        aPrtPaperSize = pPrinter->GetPaperSize();
    }

    // if paper size is 0 (usually if no 'real' printer is found),
    // guess the paper size
    if (aPrtPaperSize.IsEmpty())
        aPrtPaperSize = SvxPaperInfo::GetDefaultPaperSize(MapUnit::Map100thMM);
    awt::Size   aPageSize( aPrtPaperSize.Width(), aPrtPaperSize.Height() );

    uno::Sequence< beans::PropertyValue > aRenderer(1);
    PropertyValue  &rValue = aRenderer.getArray()[0];
    rValue.Name  = "PageSize";
    rValue.Value <<= aPageSize;

    if (!m_pPrintUIOptions)
        m_pPrintUIOptions.reset(new SmPrintUIOptions);
    m_pPrintUIOptions->appendPrintUIOptions( aRenderer );

    return aRenderer;
}

void SAL_CALL SmModel::render(
        sal_Int32 nRenderer,
        const uno::Any& rSelection,
        const uno::Sequence< beans::PropertyValue >& rxOptions )
{
    SolarMutexGuard aGuard;

    if (0 != nRenderer)
        throw IllegalArgumentException();

    SmDocShell* pDocSh = GetSmDocShell();
    if (!pDocSh)
        throw RuntimeException();

    // get device to be rendered in
    uno::Reference< awt::XDevice >  xRenderDevice;
    for (const auto& rxOption : rxOptions)
    {
        if( rxOption.Name == "RenderDevice" )
            rxOption.Value >>= xRenderDevice;
    }

    if (!xRenderDevice.is())
        return;

    VCLXDevice*   pDevice = dynamic_cast<VCLXDevice*>( xRenderDevice.get() );
    VclPtr< OutputDevice> pOut = pDevice ? pDevice->GetOutputDevice()
                                         : VclPtr< OutputDevice >();
    if (!pOut)
        throw RuntimeException();

    pOut->SetMapMode(MapMode(SmMapUnit()));

    uno::Reference< frame::XModel > xModel;
    rSelection >>= xModel;
    if (xModel != pDocSh->GetModel())
        return;

    SmPrinterAccess aPrinterAccess( *pDocSh );

    Size aPrtPaperSize;
    Size aOutputSize;
    Point aPrtPageOffset;
    if (Printer *pPrinter = aPrinterAccess.GetPrinter())
    {
        aPrtPaperSize = pPrinter->GetPaperSize();
        aOutputSize = pPrinter->GetOutputSize();
        aPrtPageOffset = pPrinter->GetPageOffset();
    }

    // no real printer ??
    if (aPrtPaperSize.IsEmpty())
    {
        aPrtPaperSize = SvxPaperInfo::GetDefaultPaperSize(SmMapUnit());
        // factors from Windows DIN A4
        aOutputSize    = Size( static_cast<tools::Long>(aPrtPaperSize.Width()  * 0.941),
                               static_cast<tools::Long>(aPrtPaperSize.Height() * 0.961));
        aPrtPageOffset = Point( static_cast<tools::Long>(aPrtPaperSize.Width()  * 0.0250),
                                static_cast<tools::Long>(aPrtPaperSize.Height() * 0.0214));
    }
    tools::Rectangle OutputRect( Point(), aOutputSize );


    // set minimum top and bottom border
    if (aPrtPageOffset.Y() < 2000)
        OutputRect.AdjustTop(2000 - aPrtPageOffset.Y() );
    if ((aPrtPaperSize.Height() - (aPrtPageOffset.Y() + OutputRect.Bottom())) < 2000)
        OutputRect.AdjustBottom( -(2000 - (aPrtPaperSize.Height() -
                                    (aPrtPageOffset.Y() + OutputRect.Bottom()))) );

    // set minimum left and right border
    if (aPrtPageOffset.X() < 2500)
        OutputRect.AdjustLeft(2500 - aPrtPageOffset.X() );
    if ((aPrtPaperSize.Width() - (aPrtPageOffset.X() + OutputRect.Right())) < 1500)
        OutputRect.AdjustRight( -(1500 - (aPrtPaperSize.Width() -
                                    (aPrtPageOffset.X() + OutputRect.Right()))) );

    if (!m_pPrintUIOptions)
        m_pPrintUIOptions.reset(new SmPrintUIOptions);
    m_pPrintUIOptions->processProperties( rxOptions );

    pDocSh->Impl_Print(*pOut, *m_pPrintUIOptions, OutputRect);

    // release SmPrintUIOptions when everything is done.
    // That way, when SmPrintUIOptions is needed again it will read the latest configuration settings in its c-tor.
    if (m_pPrintUIOptions->getBoolValue( "IsLastPage" ))
    {
        m_pPrintUIOptions.reset();
    }
}

void SAL_CALL SmModel::setParent( const uno::Reference< uno::XInterface >& xParent)
{
    SolarMutexGuard aGuard;
    SfxBaseModel::setParent( xParent );
    if (SfxObjectShell* pDoc = SfxObjectShell::GetShellFromComponent(xParent))
        GetObjectShell()->OnDocumentPrinterChanged(pDoc->GetDocumentPrinter());
}

void SmModel::writeFormulaOoxml(
        ::sax_fastparser::FSHelperPtr const pSerializer,
        oox::core::OoxmlVersion const version,
        oox::drawingml::DocumentType const documentType, sal_Int8 nAlign)
{
    GetSmDocShell()->writeFormulaOoxml(pSerializer, version, documentType, nAlign);
}

void SmModel::writeFormulaRtf(OStringBuffer& rBuffer, rtl_TextEncoding nEncoding)
{
    GetSmDocShell()->writeFormulaRtf(rBuffer, nEncoding);
}

void SmModel::readFormulaOoxml( oox::formulaimport::XmlStream& stream )
{
    GetSmDocShell()->readFormulaOoxml(stream);
}

Size SmModel::getFormulaSize() const
{
    return o3tl::convert(GetSmDocShell()->GetSize(), SmO3tlLengthUnit(), o3tl::Length::mm100);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
