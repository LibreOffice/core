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

#include <osl/mutex.hxx>
#include <sfx2/printer.hxx>
#include <vcl/svapp.hxx>
#include <svtools/ctrltool.hxx>
#include <svl/itemprop.hxx>
#include <unotools/localedatawrapper.hxx>
#include <editeng/paperinf.hxx>
#include <vcl/settings.hxx>
#include <vcl/print.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/formula/SymbolDescriptor.hpp>
#include <com/sun/star/awt/Size.hpp>
#include <com/sun/star/script/XLibraryContainer.hpp>
#include <xmloff/xmluconv.hxx>
#include <rtl/ustrbuf.hxx>
#include <comphelper/propertysetinfo.hxx>
#include <comphelper/servicehelper.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <unotools/moduleoptions.hxx>
#include <tools/mapunit.hxx>

#include <unomodel.hxx>
#include <document.hxx>
#include <view.hxx>
#include <symbol.hxx>
#include <starmath.hrc>
#include <smdll.hxx>
#include "cfgitem.hxx"

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

SmPrintUIOptions::SmPrintUIOptions()
{
    ResStringArray      aLocalizedStrings( SmResId( RID_PRINTUIOPTIONS ) );
    SAL_WARN_IF( aLocalizedStrings.Count() < 18, "starmath", "resource incomplete" );
    if( aLocalizedStrings.Count() < 9 ) // bad resource ?
        return;

    SmModule *pp = SM_MOD();
    SmMathConfig *pConfig = pp->GetConfig();
    SAL_WARN_IF( !pConfig, "starmath", "SmConfig not found" );
    if (!pConfig)
        return;

    sal_Int32 nNumProps = 10, nIdx=0;

    // create sequence of print UI options
    // (Actually IsIgnoreSpacesRight is a parser option. Without it we need only 8 properties here.)
    m_aUIProperties.realloc( nNumProps );

    // load the math PrinterOptions into the custom tab
    m_aUIProperties[nIdx].Name = "OptionsUIFile";
    m_aUIProperties[nIdx++].Value <<= OUString("modules/smath/ui/printeroptions.ui");

    // create Section for formula (results in an extra tab page in dialog)
    SvtModuleOptions aOpt;
    OUString aAppGroupname(
        aLocalizedStrings.GetString( 0 ).
            replaceFirst( "%s", aOpt.GetModuleName( SvtModuleOptions::EModule::MATH ) ) );
    m_aUIProperties[nIdx++].Value = setGroupControlOpt("tabcontrol-page2", aAppGroupname, ".HelpID:vcl:PrintDialog:TabPage:AppPage");

    // create subgroup for print options
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("contents", aLocalizedStrings.GetString(1), OUString());

    // create a bool option for title row (matches to SID_PRINTTITLE)
    m_aUIProperties[nIdx++].Value = setBoolControlOpt("title", aLocalizedStrings.GetString( 2 ),
                                                  ".HelpID:vcl:PrintDialog:TitleRow:CheckBox",
                                                  PRTUIOPT_TITLE_ROW,
                                                  pConfig->IsPrintTitle());
    // create a bool option for formula text (matches to SID_PRINTTEXT)
    m_aUIProperties[nIdx++].Value = setBoolControlOpt("formulatext", aLocalizedStrings.GetString( 3 ),
                                                  ".HelpID:vcl:PrintDialog:FormulaText:CheckBox",
                                                  PRTUIOPT_FORMULA_TEXT,
                                                  pConfig->IsPrintFormulaText());
    // create a bool option for border (matches to SID_PRINTFRAME)
    m_aUIProperties[nIdx++].Value = setBoolControlOpt("borders", aLocalizedStrings.GetString( 4 ),
                                                  ".HelpID:vcl:PrintDialog:Border:CheckBox",
                                                  PRTUIOPT_BORDER,
                                                  pConfig->IsPrintFrame());

    // create subgroup for print format
    m_aUIProperties[nIdx++].Value = setSubgroupControlOpt("size", aLocalizedStrings.GetString(5), OUString());

    // create a radio button group for print format (matches to SID_PRINTSIZE)
    Sequence< OUString > aChoices{
        aLocalizedStrings.GetString( 6 ),
        aLocalizedStrings.GetString( 7 ),
        aLocalizedStrings.GetString( 8 )
    };
    Sequence< OUString > aHelpIds{
        ".HelpID:vcl:PrintDialog:PrintFormat:RadioButton:0",
        ".HelpID:vcl:PrintDialog:PrintFormat:RadioButton:1",
        ".HelpID:vcl:PrintDialog:PrintFormat:RadioButton:2"
    };
    Sequence< OUString > aWidgetIds{
        "originalsize",
        "fittopage",
        "scaling"
    };
    OUString aPrintFormatProp( PRTUIOPT_PRINT_FORMAT );
    m_aUIProperties[nIdx++].Value = setChoiceRadiosControlOpt(aWidgetIds, OUString(),
                                                    aHelpIds,
                                                    aPrintFormatProp,
                                                    aChoices, static_cast< sal_Int32 >(pConfig->GetPrintSize())
                                                    );

    // create a numeric box for scale dependent on PrintFormat = "Scaling" (matches to SID_PRINTZOOM)
    vcl::PrinterOptionsHelper::UIControlOptions aRangeOpt( aPrintFormatProp, 2, true );
    m_aUIProperties[nIdx++].Value = setRangeControlOpt("scalingspin", OUString(),
                                                     ".HelpID:vcl:PrintDialog:PrintScale:NumericField",
                                                     PRTUIOPT_PRINT_SCALE,
                                                     pConfig->GetPrintZoomFactor(),    // initial value
                                                     10,     // min value
                                                     1000,   // max value
                                                     aRangeOpt);

    Sequence< PropertyValue > aHintNoLayoutPage( 1 );
    aHintNoLayoutPage[0].Name = "HintNoLayoutPage";
    aHintNoLayoutPage[0].Value = makeAny( sal_True );
    m_aUIProperties[nIdx++].Value <<= aHintNoLayoutPage;

    assert(nIdx == nNumProps);
}


// class SmModel


// values from com/sun/star/beans/PropertyAttribute
#define PROPERTY_NONE        0

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
    HANDLE_BASIC_LIBRARIES,
    HANDLE_RUNTIME_UID,
    HANDLE_LOAD_READONLY,     // Security Options
    HANDLE_DIALOG_LIBRARIES,  // #i73329#
    HANDLE_BASELINE,
    HANDLE_INTEROP_GRAB_BAG,
};

static PropertySetInfo * lcl_createModelPropertyInfo ()
{
    static PropertyMapEntry aModelPropertyInfoMap[] =
    {
        { OUString("Alignment")                        , HANDLE_ALIGNMENT                          ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  0                     },
        { OUString("BaseFontHeight")                   , HANDLE_BASE_FONT_HEIGHT                   ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  0                     },
        { OUString("BasicLibraries")                   , HANDLE_BASIC_LIBRARIES                    ,  cppu::UnoType<script::XLibraryContainer>::get(),  PropertyAttribute::READONLY,  0       },
        { OUString("BottomMargin")                     , HANDLE_BOTTOM_MARGIN                      ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_BOTTOMSPACE       },
        { OUString("CustomFontNameFixed")              , HANDLE_CUSTOM_FONT_NAME_FIXED             ,  ::cppu::UnoType<OUString>::get(),                                      PROPERTY_NONE,  FNT_FIXED             },
        { OUString("CustomFontNameSans")               , HANDLE_CUSTOM_FONT_NAME_SANS              ,  ::cppu::UnoType<OUString>::get(),                                      PROPERTY_NONE,  FNT_SANS              },
        { OUString("CustomFontNameSerif")              , HANDLE_CUSTOM_FONT_NAME_SERIF             ,  ::cppu::UnoType<OUString>::get(),                                      PROPERTY_NONE,  FNT_SERIF             },
        { OUString("DialogLibraries")                  , HANDLE_DIALOG_LIBRARIES                   ,  cppu::UnoType<script::XLibraryContainer>::get(),  PropertyAttribute::READONLY,  0       },
        { OUString("FontFixedIsBold")                  , HANDLE_CUSTOM_FONT_FIXED_WEIGHT           ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_FIXED             },
        { OUString("FontFixedIsItalic")                , HANDLE_CUSTOM_FONT_FIXED_POSTURE          ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_FIXED             },
        { OUString("FontFunctionsIsBold")              , HANDLE_FONT_FUNCTIONS_WEIGHT              ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_FUNCTION          },
        { OUString("FontFunctionsIsItalic")            , HANDLE_FONT_FUNCTIONS_POSTURE             ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_FUNCTION          },
        { OUString("FontNameFunctions")                , HANDLE_FONT_NAME_FUNCTIONS                ,  ::cppu::UnoType<OUString>::get(),                                      PROPERTY_NONE,  FNT_FUNCTION          },
        { OUString("FontNameNumbers")                  , HANDLE_FONT_NAME_NUMBERS                  ,  ::cppu::UnoType<OUString>::get(),                                      PROPERTY_NONE,  FNT_NUMBER            },
        { OUString("FontNameText")                     , HANDLE_FONT_NAME_TEXT                     ,  ::cppu::UnoType<OUString>::get(),                                      PROPERTY_NONE,  FNT_TEXT              },
        { OUString("FontNameVariables")                , HANDLE_FONT_NAME_VARIABLES                ,  ::cppu::UnoType<OUString>::get(),                                      PROPERTY_NONE,  FNT_VARIABLE          },
        { OUString("FontNumbersIsBold")                , HANDLE_FONT_NUMBERS_WEIGHT                ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_NUMBER            },
        { OUString("FontNumbersIsItalic")              , HANDLE_FONT_NUMBERS_POSTURE               ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_NUMBER            },
        { OUString("FontSansIsBold")                   , HANDLE_CUSTOM_FONT_SANS_WEIGHT            ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_SANS              },
        { OUString("FontSansIsItalic")                 , HANDLE_CUSTOM_FONT_SANS_POSTURE           ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_SANS              },
        { OUString("FontSerifIsBold")                  , HANDLE_CUSTOM_FONT_SERIF_WEIGHT           ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_SERIF             },
        { OUString("FontSerifIsItalic")                , HANDLE_CUSTOM_FONT_SERIF_POSTURE          ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_SERIF             },
        { OUString("FontTextIsBold")                   , HANDLE_FONT_TEXT_WEIGHT                   ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_TEXT              },
        { OUString("FontTextIsItalic")                 , HANDLE_FONT_TEXT_POSTURE                  ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_TEXT              },
        { OUString("FontVariablesIsBold")              , HANDLE_FONT_VARIABLES_WEIGHT              ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_VARIABLE          },
        { OUString("FontVariablesIsItalic")            , HANDLE_FONT_VARIABLES_POSTURE             ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  FNT_VARIABLE          },
        { OUString("Formula")                          , HANDLE_FORMULA                            ,  ::cppu::UnoType<OUString>::get(),                                      PROPERTY_NONE,  0                     },
        { OUString("IsScaleAllBrackets")               , HANDLE_IS_SCALE_ALL_BRACKETS              ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  0                     },
        { OUString("IsTextMode")                       , HANDLE_IS_TEXT_MODE                       ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  0                     },
        { OUString("GreekCharStyle")                   , HANDLE_GREEK_CHAR_STYLE                   ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  0                     },
        { OUString("LeftMargin")                       , HANDLE_LEFT_MARGIN                        ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_LEFTSPACE         },
        { OUString("PrinterName")                      , HANDLE_PRINTER_NAME                       ,  ::cppu::UnoType<OUString>::get(),                                      PROPERTY_NONE,  0                     },
        { OUString("PrinterSetup")                     , HANDLE_PRINTER_SETUP                      ,  cppu::UnoType<const Sequence < sal_Int8 >>::get(),                         PROPERTY_NONE,  0                     },
        { OUString("RelativeBracketDistance")          , HANDLE_RELATIVE_BRACKET_DISTANCE          ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_BRACKETSPACE      },
        { OUString("RelativeBracketExcessSize")        , HANDLE_RELATIVE_BRACKET_EXCESS_SIZE       ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_BRACKETSIZE       },
        { OUString("RelativeFontHeightFunctions")      , HANDLE_RELATIVE_FONT_HEIGHT_FUNCTIONS     ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  SIZ_FUNCTION          },
        { OUString("RelativeFontHeightIndices")        , HANDLE_RELATIVE_FONT_HEIGHT_INDICES       ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  SIZ_INDEX             },
        { OUString("RelativeFontHeightLimits")         , HANDLE_RELATIVE_FONT_HEIGHT_LIMITS        ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  SIZ_LIMITS            },
        { OUString("RelativeFontHeightOperators")      , HANDLE_RELATIVE_FONT_HEIGHT_OPERATORS     ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  SIZ_OPERATOR          },
        { OUString("RelativeFontHeightText")           , HANDLE_RELATIVE_FONT_HEIGHT_TEXT          ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  SIZ_TEXT              },
        { OUString("RelativeFractionBarExcessLength")  , HANDLE_RELATIVE_FRACTION_BAR_EXCESS_LENGTH,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_FRACTION          },
        { OUString("RelativeFractionBarLineWeight")    , HANDLE_RELATIVE_FRACTION_BAR_LINE_WEIGHT  ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_STROKEWIDTH       },
        { OUString("RelativeFractionDenominatorDepth") , HANDLE_RELATIVE_FRACTION_DENOMINATOR_DEPTH,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_DENOMINATOR       },
        { OUString("RelativeFractionNumeratorHeight")  , HANDLE_RELATIVE_FRACTION_NUMERATOR_HEIGHT ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_NUMERATOR         },
        { OUString("RelativeIndexSubscript")           , HANDLE_RELATIVE_INDEX_SUBSCRIPT           ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_SUBSCRIPT         },
        { OUString("RelativeIndexSuperscript")         , HANDLE_RELATIVE_INDEX_SUPERSCRIPT         ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_SUPERSCRIPT       },
        { OUString("RelativeLineSpacing")              , HANDLE_RELATIVE_LINE_SPACING              ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_VERTICAL          },
        { OUString("RelativeLowerLimitDistance")       , HANDLE_RELATIVE_LOWER_LIMIT_DISTANCE      ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_LOWERLIMIT        },
        { OUString("RelativeMatrixColumnSpacing")      , HANDLE_RELATIVE_MATRIX_COLUMN_SPACING     ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_MATRIXCOL         },
        { OUString("RelativeMatrixLineSpacing")        , HANDLE_RELATIVE_MATRIX_LINE_SPACING       ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_MATRIXROW         },
        { OUString("RelativeOperatorExcessSize")       , HANDLE_RELATIVE_OPERATOR_EXCESS_SIZE      ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_OPERATORSIZE      },
        { OUString("RelativeOperatorSpacing")          , HANDLE_RELATIVE_OPERATOR_SPACING          ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_OPERATORSPACE     },
        { OUString("RelativeRootSpacing")              , HANDLE_RELATIVE_ROOT_SPACING              ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_ROOT              },
        { OUString("RelativeScaleBracketExcessSize")   , HANDLE_RELATIVE_SCALE_BRACKET_EXCESS_SIZE ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_NORMALBRACKETSIZE },
        { OUString("RelativeSpacing")                  , HANDLE_RELATIVE_SPACING                   ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_HORIZONTAL        },
        { OUString("RelativeSymbolMinimumHeight")      , HANDLE_RELATIVE_SYMBOL_MINIMUM_HEIGHT     ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_ORNAMENTSPACE     },
        { OUString("RelativeSymbolPrimaryHeight")      , HANDLE_RELATIVE_SYMBOL_PRIMARY_HEIGHT     ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_ORNAMENTSIZE      },
        { OUString("RelativeUpperLimitDistance")       , HANDLE_RELATIVE_UPPER_LIMIT_DISTANCE      ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_UPPERLIMIT        },
        { OUString("RightMargin")                      , HANDLE_RIGHT_MARGIN                       ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_RIGHTSPACE        },
        { OUString("RuntimeUID")                       , HANDLE_RUNTIME_UID                        ,  cppu::UnoType<OUString>::get(),                      PropertyAttribute::READONLY,  0       },
        { OUString("Symbols")                          , HANDLE_SYMBOLS                            ,  cppu::UnoType<Sequence < SymbolDescriptor >>::get(),                PROPERTY_NONE,  0                     },
        { OUString("UserDefinedSymbolsInUse")          , HANDLE_USED_SYMBOLS                       ,  cppu::UnoType<Sequence < SymbolDescriptor >>::get(),                PropertyAttribute::READONLY,  0       },
        { OUString("TopMargin")                        , HANDLE_TOP_MARGIN                         ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  DIS_TOPSPACE          },
        // #i33095# Security Options
        { OUString("LoadReadonly")                     , HANDLE_LOAD_READONLY                      ,  cppu::UnoType<bool>::get(),                                                 PROPERTY_NONE,  0                     },
        // #i972#
        { OUString("BaseLine")                         , HANDLE_BASELINE                           ,  ::cppu::UnoType<sal_Int16>::get(),                                     PROPERTY_NONE,  0                     },
        { OUString("InteropGrabBag")                   , HANDLE_INTEROP_GRAB_BAG                   ,  cppu::UnoType<uno::Sequence< beans::PropertyValue >>::get(),       PROPERTY_NONE,  0                     },
        { OUString(), 0, css::uno::Type(), 0, 0 }
    };
    PropertySetInfo *pInfo = new PropertySetInfo ( aModelPropertyInfoMap );
    return pInfo;
}

SmModel::SmModel( SfxObjectShell *pObjSh )
: SfxBaseModel(pObjSh)
, PropertySetHelper ( lcl_createModelPropertyInfo () )
{
}

SmModel::~SmModel() throw ()
{
}

uno::Any SAL_CALL SmModel::queryInterface( const uno::Type& rType ) throw(uno::RuntimeException, std::exception)
{
    uno::Any aRet =  ::cppu::queryInterface ( rType,
                                    // OWeakObject interfaces
                                    dynamic_cast< XInterface* > ( static_cast< XUnoTunnel* > ( this )),
                                    static_cast< XWeak* > ( this ),
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

void SAL_CALL SmModel::acquire() throw()
{
    OWeakObject::acquire();
}

void SAL_CALL SmModel::release() throw()
{
    OWeakObject::release();
}

uno::Sequence< uno::Type > SAL_CALL SmModel::getTypes(  ) throw(uno::RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    uno::Sequence< uno::Type > aTypes = SfxBaseModel::getTypes();
    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 4);
    uno::Type* pTypes = aTypes.getArray();
    pTypes[nLen++] = cppu::UnoType<XServiceInfo>::get();
    pTypes[nLen++] = cppu::UnoType<XPropertySet>::get();
    pTypes[nLen++] = cppu::UnoType<XMultiPropertySet>::get();
    pTypes[nLen++] = cppu::UnoType<XRenderable>::get();

    return aTypes;
}

namespace
{
    class theSmModelUnoTunnelId : public rtl::Static< UnoTunnelIdInit, theSmModelUnoTunnelId> {};
}

const uno::Sequence< sal_Int8 > & SmModel::getUnoTunnelId()
{
    return theSmModelUnoTunnelId::get().getSeq();
}

sal_Int64 SAL_CALL SmModel::getSomething( const uno::Sequence< sal_Int8 >& rId )
    throw(uno::RuntimeException, std::exception)
{
    if( rId.getLength() == 16
        && 0 == memcmp( getUnoTunnelId().getConstArray(),
                                        rId.getConstArray(), 16 ) )
    {
            return sal::static_int_cast< sal_Int64 >(reinterpret_cast< sal_uIntPtr >(this));
    }

    return SfxBaseModel::getSomething( rId );
}

static sal_Int16 lcl_AnyToINT16(const uno::Any& rAny)
{
    uno::TypeClass eType = rAny.getValueType().getTypeClass();

    sal_Int16 nRet = 0;
    if( eType == uno::TypeClass_DOUBLE )
        nRet = (sal_Int16)*static_cast<double const *>(rAny.getValue());
    else if( eType == uno::TypeClass_FLOAT )
        nRet = (sal_Int16)*static_cast<float const *>(rAny.getValue());
    else
        rAny >>= nRet;
    return nRet;
}

OUString SmModel::getImplementationName() throw( uno::RuntimeException, std::exception )
{
    return OUString("com.sun.star.comp.Math.FormulaDocument");
}

sal_Bool SmModel::supportsService(const OUString& rServiceName) throw( uno::RuntimeException, std::exception )
{
    return cppu::supportsService(this, rServiceName);
}

uno::Sequence< OUString > SmModel::getSupportedServiceNames() throw( uno::RuntimeException, std::exception )
{
    return uno::Sequence<OUString>{
        "com.sun.star.document.OfficeDocument",
        "com.sun.star.formula.FormulaProperties"
    };
}

void SmModel::_setPropertyValues(const PropertyMapEntry** ppEntries, const Any* pValues)
    throw (RuntimeException, UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException, std::exception)
{
    SolarMutexGuard aGuard;

    SmDocShell *pDocSh = static_cast < SmDocShell * > (GetObjectShell());

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

                if(OUString(aFormat.GetFont((*ppEntries)->mnMemberId).GetFamilyName()) != sFontName)
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
                if((*pValues).getValueType() != cppu::UnoType<bool>::get())
                    throw IllegalArgumentException();
                bool bVal = *static_cast<sal_Bool const *>((*pValues).getValue());
                vcl::Font aNewFont(aFormat.GetFont((*ppEntries)->mnMemberId));
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
                if((*pValues).getValueType() != cppu::UnoType<bool>::get())
                    throw IllegalArgumentException();
                bool bVal = *static_cast<sal_Bool const *>((*pValues).getValue());
                vcl::Font aNewFont(aFormat.GetFont((*ppEntries)->mnMemberId));
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
                nVal = static_cast < sal_Int16 > ( convertTwipToMm100(nVal) );
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
                aFormat.SetTextmode(*static_cast<sal_Bool const *>((*pValues).getValue()));
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
                aFormat.SetScaleNormalBrackets(*static_cast<sal_Bool const *>((*pValues).getValue()));
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
                        if (*pValues >>= sPrinterName )
                        {
                            if ( !sPrinterName.isEmpty() )
                            {
                                VclPtrInstance<SfxPrinter> pNewPrinter( pPrinter->GetOptions().Clone(), sPrinterName );
                                if (pNewPrinter->IsKnown())
                                    pDocSh->SetPrinter ( pNewPrinter );
                                else
                                    pNewPrinter.disposeAndClear();
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
                    SvMemoryStream aStream ( aSequence.getArray(), nSize, StreamMode::READ );
                    aStream.Seek ( STREAM_SEEK_TO_BEGIN );
                    static sal_uInt16 const nRange[] =
                    {
                        SID_PRINTSIZE,       SID_PRINTSIZE,
                        SID_PRINTZOOM,       SID_PRINTZOOM,
                        SID_PRINTTITLE,      SID_PRINTTITLE,
                        SID_PRINTTEXT,       SID_PRINTTEXT,
                        SID_PRINTFRAME,      SID_PRINTFRAME,
                        SID_NO_RIGHT_SPACES, SID_NO_RIGHT_SPACES,
                        SID_SAVE_ONLY_USED_SYMBOLS, SID_SAVE_ONLY_USED_SYMBOLS,
                        SID_AUTO_CLOSE_BRACKETS,    SID_AUTO_CLOSE_BRACKETS,
                        0
                    };
                    SfxItemSet *pItemSet = new SfxItemSet( SmDocShell::GetPool(), nRange );
                    SmModule *pp = SM_MOD();
                    pp->GetConfig()->ConfigToItemSet(*pItemSet);
                    VclPtr<SfxPrinter> pPrinter = SfxPrinter::Create ( aStream, pItemSet );

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
                        vcl::Font aFont;
                        aFont.SetName ( pDescriptor->sFontName );
                        aFont.SetCharSet ( static_cast < rtl_TextEncoding > (pDescriptor->nCharSet) );
                        aFont.SetFamily ( static_cast < FontFamily > (pDescriptor->nFamily ) );
                        aFont.SetPitch  ( static_cast < FontPitch >  (pDescriptor->nPitch ) );
                        aFont.SetWeight ( static_cast < FontWeight > (pDescriptor->nWeight ) );
                        aFont.SetItalic ( static_cast < FontItalic > (pDescriptor->nItalic ) );
                        SmSym aSymbol ( pDescriptor->sName, aFont, static_cast < sal_Unicode > (pDescriptor->nCharacter),
                                        pDescriptor->sSymbolSet );
                        aSymbol.SetExportName ( pDescriptor->sExportName );
                        aSymbol.SetDocSymbol( true );
                        rManager.AddOrReplaceSymbol ( aSymbol );
                    }
                }
                else
                    throw IllegalArgumentException();
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
        }
    }

    pDocSh->SetFormat( aFormat );

    // #i67283# since about all of the above changes are likely to change
    // the formula size we have to recalculate the vis-area now
    pDocSh->SetVisArea( Rectangle( Point(0, 0), pDocSh->GetSize() ) );
}

void SmModel::_getPropertyValues( const PropertyMapEntry **ppEntries, Any *pValue )
    throw (RuntimeException, UnknownPropertyException, WrappedTargetException, std::exception)
{
    SmDocShell *pDocSh = static_cast < SmDocShell * > (GetObjectShell());

    if ( nullptr == pDocSh )
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
                *pValue <<= OUString(rFace.GetFamilyName());
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
                bool bVal = IsItalic( rFace );
                (*pValue).setValue(&bVal, (*ppEntries)->maType);
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
                bool bVal = IsBold( rFace ); // bold?
                (*pValue).setValue(&bVal, (*ppEntries)->maType);
            }
            break;
            case HANDLE_BASE_FONT_HEIGHT                   :
            {
                // Point!
                sal_Int16 nVal = static_cast < sal_Int16 > (aFormat.GetBaseSize().Height());
                nVal = static_cast < sal_Int16 > (convertMm100ToTwip(nVal));
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
                (*pValue).setValue(&bVal, cppu::UnoType<bool>::get());
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
                (*pValue).setValue(&bVal, cppu::UnoType<bool>::get());
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
                const std::set< OUString > &rUsedSymbols = pDocSh->GetUsedSymbols();

                // this is get
                SmModule *pp = SM_MOD();
                const SmSymbolManager &rManager = pp->GetSymbolManager();
                vector < const SmSym * > aVector;

                const SymbolPtrVec_t aSymbols( rManager.GetSymbols() );
                size_t nCount = 0;
                for (size_t i = 0; i < aSymbols.size(); ++i)
                {
                    const SmSym * pSymbol = aSymbols[ i ];
                    if (pSymbol && !pSymbol->IsPredefined() &&
                        (!bUsedSymbolsOnly ||
                         rUsedSymbols.find( pSymbol->GetName() ) != rUsedSymbols.end()))
                    {
                        aVector.push_back ( pSymbol );
                        nCount++;
                    }
                }
                Sequence < SymbolDescriptor > aSequence ( nCount );
                SymbolDescriptor * pDescriptor = aSequence.getArray();

                vector < const SmSym * >::const_iterator aIter = aVector.begin(), aEnd = aVector.end();
                for(; aIter != aEnd; pDescriptor++, ++aIter)
                {
                    pDescriptor->sName = (*aIter)->GetName();
                    pDescriptor->sExportName = (*aIter)->GetExportName();
                    pDescriptor->sSymbolSet = (*aIter)->GetSymbolSetName();
                    pDescriptor->nCharacter = static_cast < sal_Int32 > ((*aIter)->GetCharacter());

                    vcl::Font rFont = (*aIter)->GetFace();
                    pDescriptor->sFontName = rFont.GetFamilyName();
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
            // #i33095# Security Options
            case HANDLE_LOAD_READONLY :
            {
                 *pValue <<= pDocSh->IsLoadReadonly();
                break;
            }
            // #i972#
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
                break;
            }
            case HANDLE_INTEROP_GRAB_BAG:
                getGrabBagItem(*pValue);
            break;
        }
    }
}


sal_Int32 SAL_CALL SmModel::getRendererCount(
        const uno::Any& /*rSelection*/,
        const uno::Sequence< beans::PropertyValue >& /*xOptions*/ )
    throw (IllegalArgumentException, RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;
    return 1;
}


static Size lcl_GuessPaperSize()
{
    Size aRes;
    const LocaleDataWrapper& rLocWrp( AllSettings().GetLocaleDataWrapper() );
    if( MEASURE_METRIC == rLocWrp.getMeasurementSystemEnum() )
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
    throw (IllegalArgumentException, RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if (0 != nRenderer)
        throw IllegalArgumentException();

    SmDocShell *pDocSh = static_cast < SmDocShell * >( GetObjectShell() );
    if (!pDocSh)
        throw RuntimeException();

    SmPrinterAccess aPrinterAccess( *pDocSh );
    Printer *pPrinter = aPrinterAccess.GetPrinter();
    Size    aPrtPaperSize ( pPrinter->GetPaperSize() );

    // if paper size is 0 (usually if no 'real' printer is found),
    // guess the paper size
    if (aPrtPaperSize.Height() == 0 || aPrtPaperSize.Width() == 0)
        aPrtPaperSize = lcl_GuessPaperSize();
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
    throw (IllegalArgumentException, RuntimeException, std::exception)
{
    SolarMutexGuard aGuard;

    if (0 != nRenderer)
        throw IllegalArgumentException();

    SmDocShell *pDocSh = static_cast < SmDocShell * >( GetObjectShell() );
    if (!pDocSh)
        throw RuntimeException();

    // get device to be rendered in
    uno::Reference< awt::XDevice >  xRenderDevice;
    for (sal_Int32 i = 0, nCount = rxOptions.getLength();  i < nCount;  ++i)
    {
        if( rxOptions[i].Name == "RenderDevice" )
            rxOptions[i].Value >>= xRenderDevice;
    }

    if (xRenderDevice.is())
    {
        VCLXDevice*   pDevice = VCLXDevice::GetImplementation( xRenderDevice );
        VclPtr< OutputDevice> pOut = pDevice ? pDevice->GetOutputDevice()
                                             : VclPtr< OutputDevice >();
        if (!pOut)
            throw RuntimeException();

        pOut->SetMapMode( MAP_100TH_MM );

        uno::Reference< frame::XModel > xModel;
        rSelection >>= xModel;
        if (xModel == pDocSh->GetModel())
        {
            //!! when called via API we may not have an active view
            //!! thus we go and look for a view that can be used.
            SfxViewShell* pViewSh = SfxViewShell::GetFirst( false /* search non-visible views as well*/, checkSfxViewShell<SmViewShell> );
            while (pViewSh && pViewSh->GetObjectShell() != pDocSh)
                pViewSh = SfxViewShell::GetNext( *pViewSh, false /* search non-visible views as well*/, checkSfxViewShell<SmViewShell> );
            SmViewShell *pView = dynamic_cast< SmViewShell *>( pViewSh );
            SAL_WARN_IF( !pView, "starmath", "SmModel::render : no SmViewShell found" );

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
                    m_pPrintUIOptions.reset(new SmPrintUIOptions);
                m_pPrintUIOptions->processProperties( rxOptions );

                pView->Impl_Print( *pOut, *m_pPrintUIOptions, Rectangle( OutputRect ), Point() );

                // release SmPrintUIOptions when everything is done.
                // That way, when SmPrintUIOptions is needed again it will read the latest configuration settings in its c-tor.
                if (m_pPrintUIOptions->getBoolValue( "IsLastPage" ))
                {
                    m_pPrintUIOptions.reset();
                }
            }
        }
    }
}

void SAL_CALL SmModel::setParent( const uno::Reference< uno::XInterface >& xParent)
        throw( lang::NoSupportException, uno::RuntimeException, std::exception )
{
    SolarMutexGuard aGuard;
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

void SmModel::writeFormulaOoxml(
        ::sax_fastparser::FSHelperPtr const pSerializer,
        oox::core::OoxmlVersion const version,
        oox::drawingml::DocumentType const documentType)
{
    static_cast<SmDocShell*>(GetObjectShell())->writeFormulaOoxml(pSerializer, version, documentType);
}

void SmModel::writeFormulaRtf(OStringBuffer& rBuffer, rtl_TextEncoding nEncoding)
{
    static_cast<SmDocShell*>(GetObjectShell())->writeFormulaRtf(rBuffer, nEncoding);
}

void SmModel::readFormulaOoxml( oox::formulaimport::XmlStream& stream )
{
    static_cast< SmDocShell* >( GetObjectShell())->readFormulaOoxml( stream );
}

Size SmModel::getFormulaSize() const
{
    return static_cast< SmDocShell* >( GetObjectShell())->GetSize();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
