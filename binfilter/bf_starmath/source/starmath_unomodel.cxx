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

#ifdef _MSC_VER
#pragma hdrstop
#endif

#ifndef _VOS_MUTEX_HXX_
#include <vos/mutex.hxx>
#endif
#ifndef _OSL_MUTEX_HXX_
#include <osl/mutex.hxx>
#endif
#ifndef _SFX_PRINTER_HXX
#include <bf_sfx2/printer.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
#ifndef _CTRLTOOL_HXX
#include <bf_svtools/ctrltool.hxx>
#endif
#ifndef _SFX_ITEMPROP_HXX
#include <bf_svtools/itemprop.hxx>
#endif
#ifndef _UNOTOOLS_LOCALEDATAWRAPPER_HXX
#include <unotools/localedatawrapper.hxx>
#endif
#ifndef _UNOTOOLS_PROCESSFACTORY_HXX_
#include <unotools/processfactory.hxx>
#endif
#ifndef _SVX_PAPERINF_HXX
#include <bf_svx/paperinf.hxx>
#endif
#ifndef _SV_SETTINGS_HXX
#include <vcl/settings.hxx>
#endif
#ifndef _TOOLKIT_AWT_VCLXDEVICE_HXX_
#include <toolkit/awt/vclxdevice.hxx>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYSTATE_HPP_
#include <com/sun/star/beans/PropertyState.hpp>
#endif
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_FORMULA_SYMBOLDESCRIPTOR_HPP_
#include <com/sun/star/formula/SymbolDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_SIZE_HPP_
#include <com/sun/star/awt/Size.hpp>
#endif
#ifndef _COM_SUN_STAR_SCRIPT_XLIBRARYCONTAINER_HPP_
#include <com/sun/star/script/XLibraryContainer.hpp>
#endif

#ifndef _XMLOFF_XMLUCONV_HXX
#include <bf_xmloff/xmluconv.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif
#ifndef _COMPHELPER_PROPERTSETINFO_HXX_
#include <comphelper/propertysetinfo.hxx>
#endif

#ifndef UNOMODEL_HXX
#include <unomodel.hxx>
#endif
#ifndef DOCUMENT_HXX
#include <document.hxx>
#endif
#ifndef SYMBOL_HXX
#include <symbol.hxx>
#endif
#ifndef STARMATH_HRC
#include <starmath.hrc>
#endif
#ifndef CONFIG_HXX
#include <config.hxx>
#endif
#ifndef _LEGACYBINFILTERMGR_HXX
#include <legacysmgr/legacy_binfilters_smgr.hxx>	//STRIP002
#endif

#ifndef _RTL_UUID_H_
#include <rtl/uuid.h>
#endif

namespace binfilter {
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

#define TWIP_TO_MM100(TWIP) 	((TWIP) >= 0 ? (((TWIP)*127L+36L)/72L) : (((TWIP)*127L-36L)/72L))
#define MM100_TO_TWIP(MM100)	((MM100) >= 0 ? (((MM100)*72L+63L)/127L) : (((MM100)*72L-63L)/127L))

#define C2U(cChar) 	::rtl::OUString::createFromAscii(cChar)

////////////////////////////////////////
//
// class SmModel
//
#define PROPERTY_NONE 0
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
    HANDLE_BASIC_LIBRARIES      /* #93295# */
};

PropertySetInfo * lcl_createModelPropertyInfo ()
{
    static PropertyMapEntry aModelPropertyInfoMap[] =
    {
        { RTL_CONSTASCII_STRINGPARAM( "Alignment"                          ), HANDLE_ALIGNMENT                          , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM( "BaseFontHeight"                  ), HANDLE_BASE_FONT_HEIGHT                   , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM( "BasicLibraries"                  ), HANDLE_BASIC_LIBRARIES                   ,      &::getCppuType((const uno::Reference< script::XLibraryContainer > *)0),    PropertyAttribute::READONLY, 0},
        { RTL_CONSTASCII_STRINGPARAM( "BottomMargin"            		  ), HANDLE_BOTTOM_MARGIN              		   , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_BOTTOMSPACE			   },
        { RTL_CONSTASCII_STRINGPARAM( "CustomFontNameFixed"            ), HANDLE_CUSTOM_FONT_NAME_FIXED             , 		&::getCppuType((const OUString*)0), 	PROPERTY_NONE, FNT_FIXED		  },
        { RTL_CONSTASCII_STRINGPARAM( "CustomFontNameSans"              ), HANDLE_CUSTOM_FONT_NAME_SANS              , 		&::getCppuType((const OUString*)0), 	PROPERTY_NONE, FNT_SANS		   },
        { RTL_CONSTASCII_STRINGPARAM( "CustomFontNameSerif"             ), HANDLE_CUSTOM_FONT_NAME_SERIF             , 		&::getCppuType((const OUString*)0), 	PROPERTY_NONE, FNT_SERIF		  },
        { RTL_CONSTASCII_STRINGPARAM( "FontFixedIsBold"),	  HANDLE_CUSTOM_FONT_FIXED_WEIGHT    ,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_FIXED},
        { RTL_CONSTASCII_STRINGPARAM( "FontFixedIsItalic"), HANDLE_CUSTOM_FONT_FIXED_POSTURE   ,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_FIXED},
        { RTL_CONSTASCII_STRINGPARAM( "FontFunctionsIsBold"),	 HANDLE_FONT_FUNCTIONS_WEIGHT    ,  &::getBooleanCppuType(),  			PROPERTY_NONE, FNT_FUNCTION},
        { RTL_CONSTASCII_STRINGPARAM( "FontFunctionsIsItalic"),   HANDLE_FONT_FUNCTIONS_POSTURE   ,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_FUNCTION},
        { RTL_CONSTASCII_STRINGPARAM( "FontNameFunctions"                ), HANDLE_FONT_NAME_FUNCTIONS                , 		&::getCppuType((const OUString*)0), 	PROPERTY_NONE, FNT_FUNCTION	},
        { RTL_CONSTASCII_STRINGPARAM( "FontNameNumbers"                  ), HANDLE_FONT_NAME_NUMBERS                  , 		&::getCppuType((const OUString*)0), 	PROPERTY_NONE, FNT_NUMBER		 },
        { RTL_CONSTASCII_STRINGPARAM( "FontNameText"                     ), HANDLE_FONT_NAME_TEXT                     , 		&::getCppuType((const OUString*)0), 	PROPERTY_NONE, FNT_TEXT		   },
        { RTL_CONSTASCII_STRINGPARAM( "FontNameVariables"                ), HANDLE_FONT_NAME_VARIABLES                , 		&::getCppuType((const OUString*)0), 	PROPERTY_NONE, FNT_VARIABLE },
        { RTL_CONSTASCII_STRINGPARAM( "FontNumbersIsBold"),	 HANDLE_FONT_NUMBERS_WEIGHT    ,  &::getBooleanCppuType(),  			PROPERTY_NONE, FNT_NUMBER},
        { RTL_CONSTASCII_STRINGPARAM( "FontNumbersIsItalic"),   HANDLE_FONT_NUMBERS_POSTURE   ,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_NUMBER},
        { RTL_CONSTASCII_STRINGPARAM( "FontSansIsBold"),	 HANDLE_CUSTOM_FONT_SANS_WEIGHT    ,  &::getBooleanCppuType(),  			PROPERTY_NONE, FNT_SANS},
        { RTL_CONSTASCII_STRINGPARAM( "FontSansIsItalic"),   HANDLE_CUSTOM_FONT_SANS_POSTURE   ,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_SANS},
        { RTL_CONSTASCII_STRINGPARAM( "FontSerifIsBold"),	 HANDLE_CUSTOM_FONT_SERIF_WEIGHT    ,  &::getBooleanCppuType(),  			PROPERTY_NONE,  FNT_SERIF},
        { RTL_CONSTASCII_STRINGPARAM( "FontSerifIsItalic"),   HANDLE_CUSTOM_FONT_SERIF_POSTURE   ,  &::getBooleanCppuType(),  PROPERTY_NONE, FNT_SERIF},
        { RTL_CONSTASCII_STRINGPARAM( "FontTextIsBold"),	 HANDLE_FONT_TEXT_WEIGHT    ,  &::getBooleanCppuType(),  			PROPERTY_NONE, },
        { RTL_CONSTASCII_STRINGPARAM( "FontTextIsItalic"),   HANDLE_FONT_TEXT_POSTURE   ,  &::getBooleanCppuType(),  PROPERTY_NONE, },
        { RTL_CONSTASCII_STRINGPARAM( "FontVariablesIsBold"),	 HANDLE_FONT_VARIABLES_WEIGHT    ,  &::getBooleanCppuType(),  			PROPERTY_NONE, },
        { RTL_CONSTASCII_STRINGPARAM( "FontVariablesIsItalic"),   HANDLE_FONT_VARIABLES_POSTURE,  &::getBooleanCppuType(),  PROPERTY_NONE, },
        { RTL_CONSTASCII_STRINGPARAM( "Formula"							  ),	HANDLE_FORMULA							   , 		&::getCppuType((const OUString*)0), 	PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM( "IsScaleAllBrackets"              ), HANDLE_IS_SCALE_ALL_BRACKETS              , 		&::getBooleanCppuType(), 	PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM( "IsTextMode"                       ), HANDLE_IS_TEXT_MODE                       , 		&::getBooleanCppuType(), 	PROPERTY_NONE, 0},
        { RTL_CONSTASCII_STRINGPARAM( "LeftMargin"               		  ), HANDLE_LEFT_MARGIN        		           , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_LEFTSPACE			     },
        { RTL_CONSTASCII_STRINGPARAM( "PrinterName"                	   ), HANDLE_PRINTER_NAME               		 , 		&::getCppuType((const OUString*)0), 	PROPERTY_NONE, 0			      },
        { RTL_CONSTASCII_STRINGPARAM( "PrinterSetup"                	   ), HANDLE_PRINTER_SETUP               		 , 		&::getCppuType((const Sequence < sal_Int8 >*)0), 	PROPERTY_NONE, 0			      },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeBracketDistance"          ), HANDLE_RELATIVE_BRACKET_DISTANCE          , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_BRACKETSPACE	},
        { RTL_CONSTASCII_STRINGPARAM( "RelativeBracketExcessSize"       ), HANDLE_RELATIVE_BRACKET_EXCESS_SIZE       , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_BRACKETSIZE 	},
        { RTL_CONSTASCII_STRINGPARAM( "RelativeFontHeightFunctions"     ), HANDLE_RELATIVE_FONT_HEIGHT_FUNCTIONS     , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, SIZ_FUNCTION},
        { RTL_CONSTASCII_STRINGPARAM( "RelativeFontHeightIndices"       ), HANDLE_RELATIVE_FONT_HEIGHT_INDICES       , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, SIZ_INDEX	  },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeFontHeightLimits"        ), HANDLE_RELATIVE_FONT_HEIGHT_LIMITS        , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, SIZ_LIMITS	 },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeFontHeightOperators"     ), HANDLE_RELATIVE_FONT_HEIGHT_OPERATORS     , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, SIZ_OPERATOR},
        { RTL_CONSTASCII_STRINGPARAM( "RelativeFontHeightText"       	  ), HANDLE_RELATIVE_FONT_HEIGHT_TEXT          , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, SIZ_TEXT	  },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeFractionBarExcessLength"), HANDLE_RELATIVE_FRACTION_BAR_EXCESS_LENGTH, 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_FRACTION		   },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeFractionBarLineWeight"  ), HANDLE_RELATIVE_FRACTION_BAR_LINE_WEIGHT  , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_STROKEWIDTH 	},
        { RTL_CONSTASCII_STRINGPARAM( "RelativeFractionDenominatorDepth"), HANDLE_RELATIVE_FRACTION_DENOMINATOR_DEPTH, 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_DENOMINATOR 	},
        { RTL_CONSTASCII_STRINGPARAM( "RelativeFractionNumeratorHeight" ), HANDLE_RELATIVE_FRACTION_NUMERATOR_HEIGHT , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_NUMERATOR		  },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeIndexSubscript"           ), HANDLE_RELATIVE_INDEX_SUBSCRIPT           , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_SUBSCRIPT		  },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeIndexSuperscript"         ), HANDLE_RELATIVE_INDEX_SUPERSCRIPT         , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_SUPERSCRIPT 	},
        { RTL_CONSTASCII_STRINGPARAM( "RelativeLineSpacing"              ), HANDLE_RELATIVE_LINE_SPACING              , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_VERTICAL		   },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeLowerLimitDistance"      ), HANDLE_RELATIVE_LOWER_LIMIT_DISTANCE      , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_LOWERLIMIT		 },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeMatrixColumnSpacing"     ), HANDLE_RELATIVE_MATRIX_COLUMN_SPACING     , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_MATRIXCOL},
        { RTL_CONSTASCII_STRINGPARAM( "RelativeMatrixLineSpacing"       ), HANDLE_RELATIVE_MATRIX_LINE_SPACING       , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_MATRIXROW},
        { RTL_CONSTASCII_STRINGPARAM( "RelativeOperatorExcessSize"      ), HANDLE_RELATIVE_OPERATOR_EXCESS_SIZE      , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_OPERATORSIZE		   },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeOperatorSpacing"          ), HANDLE_RELATIVE_OPERATOR_SPACING          , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_OPERATORSPACE},
        { RTL_CONSTASCII_STRINGPARAM( "RelativeRootSpacing"              ), HANDLE_RELATIVE_ROOT_SPACING              , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_ROOT			      },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeScaleBracketExcessSize" ), HANDLE_RELATIVE_SCALE_BRACKET_EXCESS_SIZE , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_NORMALBRACKETSIZE},
        { RTL_CONSTASCII_STRINGPARAM( "RelativeSpacing"                   ), HANDLE_RELATIVE_SPACING                   , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_HORIZONTAL		 },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeSymbolMinimumHeight"     ), HANDLE_RELATIVE_SYMBOL_MINIMUM_HEIGHT     , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_ORNAMENTSPACE		 },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeSymbolPrimaryHeight"     ), HANDLE_RELATIVE_SYMBOL_PRIMARY_HEIGHT     , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_ORNAMENTSIZE		   },
        { RTL_CONSTASCII_STRINGPARAM( "RelativeUpperLimitDistance"      ), 	HANDLE_RELATIVE_UPPER_LIMIT_DISTANCE     , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_UPPERLIMIT		 },
        { RTL_CONSTASCII_STRINGPARAM( "RightMargin"              		  ), 	HANDLE_RIGHT_MARGIN              	 , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_RIGHTSPACE			    },
        { RTL_CONSTASCII_STRINGPARAM( "Symbols"              		  ), 		HANDLE_SYMBOLS              		 , 		&::getCppuType((const Sequence < SymbolDescriptor > *)0), 	PROPERTY_NONE, 0  },
        { RTL_CONSTASCII_STRINGPARAM( "TopMargin"                		  ), 	HANDLE_TOP_MARGIN                	 , 		&::getCppuType((const sal_Int16*)0), 	PROPERTY_NONE, DIS_TOPSPACE			      },
        { NULL, 0, 0, NULL, 0, 0 }
    };
    PropertySetInfo *pInfo = new PropertySetInfo ( aModelPropertyInfoMap );
    return pInfo;
}
//-----------------------------------------------------------------------
SmModel::SmModel( SfxObjectShell *pObjSh )
: SfxBaseModel(pObjSh)
, PropertySetHelper ( lcl_createModelPropertyInfo () )
{
}
//-----------------------------------------------------------------------
SmModel::~SmModel() throw ()
{
}
/*-- 28.03.00 14:18:17---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Any SAL_CALL SmModel::queryInterface( const uno::Type& rType ) throw(uno::RuntimeException)
{
    uno::Any aRet =  ::cppu::queryInterface ( rType,
                                    // OWeakObject interfaces
                                    reinterpret_cast< XInterface* > ( this ),
                                    static_cast< XWeak* > ( this ),
                                    // PropertySetHelper interfaces
                                    static_cast< XPropertySet* > ( this ),
                                    static_cast< XMultiPropertySet* > ( this ),
                                    //static_cast< XPropertyState* > ( this ),
                                    // my own interfaces
                                    static_cast< XUnoTunnel* > ( this ),
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
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    OWeakObject::acquire();
}
/*-- 28.03.00 14:18:18---------------------------------------------------

  -----------------------------------------------------------------------*/
void SAL_CALL SmModel::release() throw()
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    OWeakObject::release();
}
/*-- 28.03.00 14:18:19---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< uno::Type > SAL_CALL SmModel::getTypes(  ) throw(uno::RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    uno::Sequence< uno::Type > aTypes = SfxBaseModel::getTypes();
    sal_Int32 nLen = aTypes.getLength();
    aTypes.realloc(nLen + 5);
    uno::Type* pTypes = aTypes.getArray();
    pTypes[nLen++] = ::getCppuType((Reference<XServiceInfo>*)0);
    pTypes[nLen++] = ::getCppuType((Reference<XUnoTunnel>*)0);
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
        rtl_createUuid( (sal_uInt8*)aSeq.getArray(), 0,	sal_True );
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
            return (sal_Int64)this;
    }
    return 0;
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
    return C2U("SmModel");
}
/*-- 07.02.00 13:24:09---------------------------------------------------

  -----------------------------------------------------------------------*/
sal_Bool SmModel::supportsService(const OUString& rServiceName) throw( uno::RuntimeException )
{
    return (
            rServiceName == C2U("com.sun.star.document.OfficeDocument"  ) ||
            rServiceName == C2U("com.sun.star.formula.FormulaProperties")
           );

}
/*-- 07.02.00 13:24:09---------------------------------------------------

  -----------------------------------------------------------------------*/
uno::Sequence< OUString > SmModel::getSupportedServiceNames(void) throw( uno::RuntimeException )
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());

    uno::Sequence< OUString > aRet(2);
    OUString* pArray = aRet.getArray();
    pArray[0] = C2U("com.sun.star.document.OfficeDocument");
    pArray[1] = C2U("com.sun.star.formula.FormulaProperties");
    return aRet;
}

void SmModel::_setPropertyValues(const PropertyMapEntry** ppEntries, const Any* pValues)
    throw( UnknownPropertyException, PropertyVetoException, IllegalArgumentException, WrappedTargetException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());

    SmDocShell *pDocSh = static_cast < SmDocShell * > (GetObjectShell());

    if ( NULL == pDocSh )
        throw UnknownPropertyException();

    SmFormat & aFormat = pDocSh->GetFormat();

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
                BOOL bVal = *(sal_Bool*)(*pValues).getValue();
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
                BOOL bVal = *(sal_Bool*)(*pValues).getValue();
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
                for (USHORT  i = FNT_BEGIN;  i <= FNT_END;  i++)
                    aFormat.SetFontSize(i, aTmp);
            }
            break;
            case HANDLE_RELATIVE_FONT_HEIGHT_TEXT          :
            case HANDLE_RELATIVE_FONT_HEIGHT_INDICES       :
            case HANDLE_RELATIVE_FONT_HEIGHT_FUNCTIONS     :
            case HANDLE_RELATIVE_FONT_HEIGHT_OPERATORS     :
            case HANDLE_RELATIVE_FONT_HEIGHT_LIMITS        :
            {
                sal_Int16 nVal;
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

            case HANDLE_ALIGNMENT                          :
            {
                // SmHorAlign uses the same values as HorizontalAlignment
                sal_Int16 nVal;
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
                sal_Int16 nVal;
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
                SfxPrinter *pPrinter = pDocSh->GetPrinter ( );
                if (pPrinter)
                {
                    OUString sPrinterName;
                    if (*pValues >>= sPrinterName )
                    {
                        SfxPrinter *pNewPrinter = new SfxPrinter ( pPrinter->GetOptions().Clone(), sPrinterName );
                        if (pNewPrinter->IsKnown())
                            pDocSh->SetPrinter ( pNewPrinter );
                        else
                            delete pNewPrinter;
                    }
                    else
                        throw IllegalArgumentException();
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
                    SmModule *pp = SM_MOD1();
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
                    SmSymSetManager &rManager = pDocSh->GetSymSetManager();
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
                        aSymbol.SetDocSymbol( TRUE );
                        rManager.AddReplaceSymbol ( aSymbol );
                    }
                }
                else
                    throw IllegalArgumentException();
            }
            break;
        }
    }
}

void SmModel::_getPropertyValues( const PropertyMapEntry **ppEntries, Any *pValue )
    throw( UnknownPropertyException, WrappedTargetException )
{
    SmDocShell *pDocSh = static_cast < SmDocShell * > (GetObjectShell());

    if ( NULL == pDocSh )
        throw UnknownPropertyException();

    SmFormat & aFormat = pDocSh->GetFormat();

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
                BOOL bVal = (rFace.GetItalic() != ITALIC_NONE);
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
                BOOL bVal = (rFace.GetWeight() == WEIGHT_BOLD);
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
            case HANDLE_RELATIVE_FONT_HEIGHT_TEXT       	:
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
            {
                // this is get
                const SmSymSetManager &rManager = pDocSh->GetSymSetManager();
                vector < const SmSym * > aVector;

                USHORT nCount = 0;
                for (USHORT i = 0, nEnd = rManager.GetSymbolCount(); i < nEnd; i++)
                {
                    const SmSym * pSymbol = rManager.GetSymbolByPos( i );
                    if (pSymbol && !pSymbol->IsPredefined () )
                    {
                        aVector.push_back ( pSymbol );
                        nCount++;
                    }
                }
                Sequence < SymbolDescriptor > aSequence ( nCount );
                SymbolDescriptor * pDescriptor = aSequence.getArray();

                vector <const SmSym * >::const_iterator aIter = aVector.begin(), aEnd = aVector.end();
                for(; aIter != aEnd; pDescriptor++, aIter++)
                {
                    pDescriptor->sName = (*aIter)->GetName();
                    pDescriptor->sExportName = (*aIter)->GetExportName();
                    pDescriptor->sSymbolSet = (*aIter)->GetSetName();
                    pDescriptor->nCharacter = static_cast < sal_Int32 > ((*aIter)->GetCharacter());

                    Font rFont = (*aIter)->GetFace();
                    pDescriptor->sFontName = rFont.GetName();
                    pDescriptor->nCharSet  = rFont.GetCharSet();
                    pDescriptor->nFamily   = rFont.GetFamily();
                    pDescriptor->nPitch    = rFont.GetPitch();
                    pDescriptor->nWeight   = rFont.GetWeight();
                    pDescriptor->nItalic   = rFont.GetItalic();
                }
                *pValue <<= aSequence;
            }
            break;
            case HANDLE_BASIC_LIBRARIES:
                *pValue <<= pDocSh->GetBasicContainer();
            break;
        }
    }
}

//////////////////////////////////////////////////////////////////////

sal_Int32 SAL_CALL SmModel::getRendererCount(
        const uno::Any& rSelection,
        const uno::Sequence< beans::PropertyValue >& xOptions )
    throw (IllegalArgumentException, RuntimeException)
{
    ::vos::OGuard aGuard(Application::GetSolarMutex());
    return 1;
}


static Size lcl_GuessPaperSize()
{
    Size aRes;
    Reference< XMultiServiceFactory >  xMgr( ::legacy_binfilters::getLegacyProcessServiceFactory() );
    LocaleDataWrapper aLocWrp( xMgr, AllSettings().GetLocale() );
    if( MEASURE_METRIC == aLocWrp.getMeasurementSystemEnum() )
    {
        // in Twip
        aRes.Width()  = lA4Width;
        aRes.Height() = lA4Height;
    }
    else
    {
        // in Twip
        aRes.Width()  = lLetterWidth;
        aRes.Height() = lLetterHeight;
    }
    aRes = OutputDevice::LogicToLogic( aRes, MapMode(MAP_TWIP),
                                             MapMode(MAP_100TH_MM) );
    return aRes;
}


uno::Sequence< beans::PropertyValue > SAL_CALL SmModel::getRenderer(
        sal_Int32 nRenderer,
        const uno::Any& rSelection,
        const uno::Sequence< beans::PropertyValue >& xOptions )
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
    }
}

}
