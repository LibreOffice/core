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


#include "formcontrolfactory.hxx"
#include "fmcontrollayout.hxx"
#include "fmprop.hrc"
#include "svx/fmresids.hrc"
#include "fmservs.hxx"
#include "svx/dialmgr.hxx"
#include "svx/svdouno.hxx"

#include <com/sun/star/form/XFormComponent.hpp>
#include <com/sun/star/form/FormComponentType.hpp>
#include <com/sun/star/awt/ScrollBarOrientation.hpp>
#include <com/sun/star/awt/MouseWheelBehavior.hpp>
#include <com/sun/star/form/XGridColumnFactory.hpp>
#include <com/sun/star/style/VerticalAlignment.hpp>
#include <com/sun/star/awt/LineEndFormat.hpp>
#include <com/sun/star/awt/ImageScaleMode.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/util/XNumberFormatTypes.hpp>
#include <com/sun/star/sdbc/ColumnValue.hpp>
#include <com/sun/star/text/WritingMode2.hpp>

#include <comphelper/numbers.hxx>
#include <comphelper/processfactory.hxx>
#include <unotools/syslocale.hxx>
#include <tools/gen.hxx>
#include <tools/diagnose_ex.h>
#include <connectivity/dbtools.hxx>

#include <set>

using namespace ::dbtools;

namespace svxform
{


    using ::com::sun::star::uno::Reference;
    using ::com::sun::star::uno::UNO_QUERY;
    using ::com::sun::star::uno::UNO_QUERY_THROW;
    using ::com::sun::star::uno::UNO_SET_THROW;
    using ::com::sun::star::uno::Exception;
    using ::com::sun::star::uno::Any;
    using ::com::sun::star::uno::makeAny;
    using ::com::sun::star::uno::Sequence;
    using ::com::sun::star::uno::Type;
    using ::com::sun::star::uno::XComponentContext;
    using ::com::sun::star::beans::XPropertySet;
    using ::com::sun::star::form::XFormComponent;
    using ::com::sun::star::container::XIndexAccess;
    using ::com::sun::star::beans::XPropertySetInfo;
    using ::com::sun::star::beans::PropertyValue;
    using ::com::sun::star::container::XChild;
    using ::com::sun::star::form::XGridColumnFactory;
    using ::com::sun::star::style::VerticalAlignment_MIDDLE;
    using ::com::sun::star::beans::Property;
    using ::com::sun::star::uno::TypeClass_DOUBLE;
    using ::com::sun::star::uno::TypeClass_LONG;
    using ::com::sun::star::util::XNumberFormats;
    using ::com::sun::star::util::XNumberFormatTypes;
    using ::com::sun::star::lang::XServiceInfo;
    using ::com::sun::star::container::XNameAccess;

    namespace FormComponentType = ::com::sun::star::form::FormComponentType;
    namespace ScrollBarOrientation = ::com::sun::star::awt::ScrollBarOrientation;
    namespace MouseWheelBehavior = ::com::sun::star::awt::MouseWheelBehavior;
    namespace LineEndFormat = ::com::sun::star::awt::LineEndFormat;
    namespace ImageScaleMode = ::com::sun::star::awt::ImageScaleMode;
    namespace DataType = ::com::sun::star::sdbc::DataType;
    namespace ColumnValue = ::com::sun::star::sdbc::ColumnValue;
    namespace WritingMode2 = ::com::sun::star::text::WritingMode2;

    struct FormControlFactory_Data
    {
        Reference<XComponentContext>  m_xContext;

        explicit FormControlFactory_Data( const Reference<XComponentContext>& _rContext )
            :m_xContext( _rContext )
        {
        }
    };

    FormControlFactory::FormControlFactory( const Reference<XComponentContext>& _rContext )
        :m_pData( new FormControlFactory_Data( _rContext ) )
    {
    }

    FormControlFactory::FormControlFactory( )
        :m_pData( new FormControlFactory_Data( comphelper::getProcessComponentContext() ) )
    {
    }


    FormControlFactory::~FormControlFactory()
    {
    }


    sal_Int16 FormControlFactory::initializeControlModel( const DocumentType _eDocType, const SdrUnoObj& _rObject )
    {
        return initializeControlModel(
            _eDocType,
            Reference< XPropertySet >( _rObject.GetUnoControlModel(), UNO_QUERY ),
            _rObject.GetCurrentBoundRect()
        );
    }


    void FormControlFactory::initializeControlModel( const DocumentType _eDocType, const Reference< XPropertySet >& _rxControlModel )
    {
        initializeControlModel(
            _eDocType, _rxControlModel, Rectangle()
        );
    }


    namespace
    {

        OUString lcl_getUniqueLabel_nothrow( const Reference< XPropertySet >& _rxControlModel, const OUString& _rBaseLabel )
        {
            OUString sLabel( _rBaseLabel );
            try
            {
                typedef ::std::set< OUString > StringBag;
                StringBag aUsedLabels;

                Reference< XFormComponent > xFormComponent( _rxControlModel, UNO_QUERY_THROW );
                Reference< XIndexAccess > xContainer( xFormComponent->getParent(), UNO_QUERY_THROW );
                // loop through all siblings of the control model, and collect their labels
                for ( sal_Int32 index=xContainer->getCount(); index>0; )
                {
                    Reference< XPropertySet > xElement( xContainer->getByIndex( --index ), UNO_QUERY_THROW );
                    if ( xElement == _rxControlModel )
                        continue;

                    Reference< XPropertySetInfo > xPSI( xElement->getPropertySetInfo(), UNO_SET_THROW );
                    if ( !xPSI->hasPropertyByName( FM_PROP_LABEL ) )
                        continue;

                    OUString sElementLabel;
                    OSL_VERIFY( xElement->getPropertyValue( FM_PROP_LABEL ) >>= sElementLabel );
                    aUsedLabels.insert( sElementLabel );
                }

                // now find a free label
                sal_Int32 i=2;
                while ( aUsedLabels.find( sLabel ) != aUsedLabels.end() )
                {
                    OUStringBuffer aBuffer( _rBaseLabel );
                    aBuffer.append( " " );
                    aBuffer.append( (sal_Int32)i++ );
                    sLabel = aBuffer.makeStringAndClear();
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
            return sLabel;
        }


        Sequence< PropertyValue > lcl_getDataSourceIndirectProperties( const Reference< XPropertySet >& _rxControlModel,
            const Reference<XComponentContext>& _rContext )
        {
            OSL_PRECOND( _rxControlModel.is(), "lcl_getDataSourceIndirectProperties: invalid model!" );

            Sequence< PropertyValue > aInfo;
            try
            {
                Reference< XChild > xChild( _rxControlModel, UNO_QUERY );
                Reference< XPropertySet > xForm;
                if ( xChild.is() )
                    xForm.set(xChild->getParent(), css::uno::UNO_QUERY);

                if ( Reference< XGridColumnFactory >( xForm, UNO_QUERY ).is() )
                {   // hmm. the model is a grid column, in real
                    xChild.set(xForm, css::uno::UNO_QUERY);
                    xForm.set(xChild->getParent(), css::uno::UNO_QUERY);
                }

                OSL_ENSURE( xForm.is(), "lcl_getDataSourceIndirectProperties: could not determine the form!" );
                if ( !xForm.is() )
                    return aInfo;
                OUString sDataSourceName;
                xForm->getPropertyValue( FM_PROP_DATASOURCE ) >>= sDataSourceName;

                Reference< XPropertySet > xDsProperties;
                if ( !sDataSourceName.isEmpty() )
                    xDsProperties.set(getDataSource( sDataSourceName, _rContext ), css::uno::UNO_QUERY);
                if ( xDsProperties.is() )
                    xDsProperties->getPropertyValue("Info") >>= aInfo;
            }
            catch( const Exception& )
            {
                OSL_FAIL( "lcl_getDataSourceIndirectProperties: caught an exception!" );
            }
            return aInfo;
        }


        static const sal_Char* aCharacterAndParagraphProperties[] =
        {
            "CharFontName",
            "CharFontStyleName",
            "CharFontFamily",
            "CharFontCharSet",
            "CharFontPitch",
            "CharColor",
            "CharEscapement",
            "CharHeight",
            "CharUnderline",
            "CharWeight",
            "CharPosture",
            "CharAutoKerning",
            "CharBackColor",
            "CharBackTransparent",
            "CharCaseMap",
            "CharCrossedOut",
            "CharFlash",
            "CharStrikeout",
            "CharWordMode",
            "CharKerning",
            "CharLocale",
            "CharKeepTogether",
            "CharNoLineBreak",
            "CharShadowed",
            "CharFontType",
            "CharStyleName",
            "CharContoured",
            "CharCombineIsOn",
            "CharCombinePrefix",
            "CharCombineSuffix",
            "CharEmphasize",
            "CharRelief",
            "RubyText",
            "RubyAdjust",
            "RubyCharStyleName",
            "RubyIsAbove",
            "CharRotation",
            "CharRotationIsFitToLine",
            "CharScaleWidth",
            "HyperLinkURL",
            "HyperLinkTarget",
            "HyperLinkName",
            "VisitedCharStyleName",
            "UnvisitedCharStyleName",
            "CharEscapementHeight",
            "CharNoHyphenation",
            "CharUnderlineColor",
            "CharUnderlineHasColor",
            "CharStyleNames",
            "CharHeightAsian",
            "CharWeightAsian",
            "CharFontNameAsian",
            "CharFontStyleNameAsian",
            "CharFontFamilyAsian",
            "CharFontCharSetAsian",
            "CharFontPitchAsian",
            "CharPostureAsian",
            "CharLocaleAsian",
            "ParaIsCharacterDistance",
            "ParaIsForbiddenRules",
            "ParaIsHangingPunctuation",
            "CharHeightComplex",
            "CharWeightComplex",
            "CharFontNameComplex",
            "CharFontStyleNameComplex",
            "CharFontFamilyComplex",
            "CharFontCharSetComplex",
            "CharFontPitchComplex",
            "CharPostureComplex",
            "CharLocaleComplex",
            "ParaAdjust",
            "ParaLineSpacing",
            "ParaBackColor",
            "ParaBackTransparent",
            "ParaBackGraphicURL",
            "ParaBackGraphicFilter",
            "ParaBackGraphicLocation",
            "ParaLastLineAdjust",
            "ParaExpandSingleWord",
            "ParaLeftMargin",
            "ParaRightMargin",
            "ParaTopMargin",
            "ParaBottomMargin",
            "ParaLineNumberCount",
            "ParaLineNumberStartValue",
            "PageDescName",
            "PageNumberOffset",
            "ParaRegisterModeActive",
            "ParaTabStops",
            "ParaStyleName",
            "DropCapFormat",
            "DropCapWholeWord",
            "ParaKeepTogether",
            "Setting",
            "ParaSplit",
            "Setting",
            "NumberingLevel",
            "NumberingRules",
            "NumberingStartValue",
            "ParaIsNumberingRestart",
            "NumberingStyleName",
            "ParaOrphans",
            "ParaWidows",
            "ParaShadowFormat",
            "LeftBorder",
            "RightBorder",
            "TopBorder",
            "BottomBorder",
            "BorderDistance",
            "LeftBorderDistance",
            "RightBorderDistance",
            "TopBorderDistance",
            "BottomBorderDistance",
            "BreakType",
            "DropCapCharStyleName",
            "ParaFirstLineIndent",
            "ParaIsAutoFirstLineIndent",
            "ParaIsHyphenation",
            "ParaHyphenationMaxHyphens",
            "ParaHyphenationMaxLeadingChars",
            "ParaHyphenationMaxTrailingChars",
            "ParaVertAlignment",
            "ParaUserDefinedAttributes",
            "NumberingIsNumber",
            "ParaIsConnectBorder",
            nullptr
        };


        void lcl_initializeCharacterAttributes( const Reference< XPropertySet >& _rxModel )
        {
            try
            {
                Reference< XPropertySet > xStyle( ControlLayouter::getDefaultDocumentTextStyle( _rxModel ), UNO_SET_THROW );

                // transfer all properties which are described by the style
                Reference< XPropertySetInfo > xSourcePropInfo( xStyle->getPropertySetInfo(), UNO_SET_THROW );
                Reference< XPropertySetInfo > xDestPropInfo( _rxModel->getPropertySetInfo(), UNO_SET_THROW );

                OUString sPropertyName;
                const sal_Char** pCharacterProperty = aCharacterAndParagraphProperties;
                while ( *pCharacterProperty )
                {
                    sPropertyName = OUString::createFromAscii( *pCharacterProperty );

                    if ( xSourcePropInfo->hasPropertyByName( sPropertyName ) && xDestPropInfo->hasPropertyByName( sPropertyName ) )
                        _rxModel->setPropertyValue( sPropertyName, xStyle->getPropertyValue( sPropertyName ) );

                    ++pCharacterProperty;
                }
            }
            catch( const Exception& )
            {
                DBG_UNHANDLED_EXCEPTION();
            }
        }
    }


    sal_Int16 FormControlFactory::initializeControlModel( const DocumentType _eDocType, const Reference< XPropertySet >& _rxControlModel,
        const Rectangle& _rControlBoundRect )
    {
        sal_Int16 nClassId = FormComponentType::CONTROL;

        OSL_ENSURE( _rxControlModel.is(), "FormControlFactory::initializeControlModel: invalid model!" );
        if ( !_rxControlModel.is() )
            return nClassId;

        try
        {
            ControlLayouter::initializeControlLayout( _rxControlModel, _eDocType );

            _rxControlModel->getPropertyValue( FM_PROP_CLASSID ) >>= nClassId;
            Reference< XPropertySetInfo > xPSI( _rxControlModel->getPropertySetInfo(), UNO_SET_THROW );
            switch ( nClassId )
            {
                case FormComponentType::SCROLLBAR:
                    _rxControlModel->setPropertyValue("LiveScroll", makeAny( true ) );
                    SAL_FALLTHROUGH;
                case FormComponentType::SPINBUTTON:
                {
                    sal_Int32 eOrientation = ScrollBarOrientation::HORIZONTAL;
                    if ( !_rControlBoundRect.IsEmpty() && ( _rControlBoundRect.GetWidth() < _rControlBoundRect.GetHeight() ) )
                        eOrientation = ScrollBarOrientation::VERTICAL;
                    _rxControlModel->setPropertyValue( FM_PROP_ORIENTATION, makeAny( eOrientation ) );
                }
                break;

                case FormComponentType::LISTBOX:
                case FormComponentType::COMBOBOX:
                {
                    bool bDropDown = !_rControlBoundRect.IsEmpty() && ( _rControlBoundRect.GetWidth() >= 3 * _rControlBoundRect.GetHeight() );
                    if ( xPSI->hasPropertyByName( FM_PROP_DROPDOWN ) )
                        _rxControlModel->setPropertyValue( FM_PROP_DROPDOWN, makeAny( bDropDown ) );
                    _rxControlModel->setPropertyValue( FM_PROP_LINECOUNT, makeAny( sal_Int16( 20 ) ) );
                }
                break;

                case FormComponentType::TEXTFIELD:
                {
                    initializeTextFieldLineEnds( _rxControlModel );
                    lcl_initializeCharacterAttributes( _rxControlModel );

                    if  (   !_rControlBoundRect.IsEmpty()
                        &&  !( _rControlBoundRect.GetWidth() > 4 * _rControlBoundRect.GetHeight() )
                        )
                    {
                        if ( xPSI->hasPropertyByName( FM_PROP_MULTILINE ) )
                            _rxControlModel->setPropertyValue( FM_PROP_MULTILINE, makeAny( true ) );
                    }
                }
                break;

                case FormComponentType::RADIOBUTTON:
                case FormComponentType::CHECKBOX:
                case FormComponentType::FIXEDTEXT:
                {
                    OUString sVertAlignPropertyName( "VerticalAlign" );
                    if ( xPSI->hasPropertyByName( sVertAlignPropertyName ) )
                        _rxControlModel->setPropertyValue( sVertAlignPropertyName, makeAny( VerticalAlignment_MIDDLE ) );
                }
                break;

                case FormComponentType::IMAGEBUTTON:
                case FormComponentType::IMAGECONTROL:
                {
                    const OUString sScaleModeProperty( "ScaleMode" );
                    if ( xPSI->hasPropertyByName( sScaleModeProperty ) )
                        _rxControlModel->setPropertyValue( sScaleModeProperty, makeAny( ImageScaleMode::ISOTROPIC ) );
                }
                break;
            }

            // initial default label for the control
            if ( xPSI->hasPropertyByName( FM_PROP_LABEL ) )
            {
                OUString sExistingLabel;
                OSL_VERIFY( _rxControlModel->getPropertyValue( FM_PROP_LABEL ) >>= sExistingLabel );
                if ( sExistingLabel.isEmpty() )
                {
                    OUString sInitialLabel;
                    OSL_VERIFY( _rxControlModel->getPropertyValue( FM_PROP_NAME ) >>= sInitialLabel );

                    sal_uInt16 nTitleResId = 0;
                    switch ( nClassId )
                    {
                        case FormComponentType::COMMANDBUTTON:  nTitleResId = RID_STR_PROPTITLE_PUSHBUTTON;      break;
                        case FormComponentType::RADIOBUTTON:    nTitleResId = RID_STR_PROPTITLE_RADIOBUTTON;     break;
                        case FormComponentType::CHECKBOX:       nTitleResId = RID_STR_PROPTITLE_CHECKBOX;        break;
                        case FormComponentType::GROUPBOX:       nTitleResId = RID_STR_PROPTITLE_GROUPBOX;        break;
                        case FormComponentType::FIXEDTEXT:      nTitleResId = RID_STR_PROPTITLE_FIXEDTEXT;       break;
                    }

                    if ( nTitleResId )
                        sInitialLabel = SVX_RESSTR(nTitleResId);

                    _rxControlModel->setPropertyValue(
                        FM_PROP_LABEL,
                        makeAny( lcl_getUniqueLabel_nothrow( _rxControlModel, sInitialLabel ) )
                    );
                }
            }

            // strict format = yes is the default (i93467)
            if ( xPSI->hasPropertyByName( FM_PROP_STRICTFORMAT ) )
            {
                _rxControlModel->setPropertyValue( FM_PROP_STRICTFORMAT, makeAny( true ) );
            }

            // mouse wheel: don't use it for scrolling by default (i110036)
            if ( xPSI->hasPropertyByName( FM_PROP_MOUSE_WHEEL_BEHAVIOR ) )
            {
                _rxControlModel->setPropertyValue( FM_PROP_MOUSE_WHEEL_BEHAVIOR, makeAny( MouseWheelBehavior::SCROLL_DISABLED ) );
            }

            if ( xPSI->hasPropertyByName( FM_PROP_WRITING_MODE ) )
                _rxControlModel->setPropertyValue( FM_PROP_WRITING_MODE, makeAny( WritingMode2::CONTEXT ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return nClassId;
    }


    void FormControlFactory::initializeTextFieldLineEnds( const Reference< XPropertySet >& _rxModel )
    {
        OSL_PRECOND( _rxModel.is(), "initializeTextFieldLineEnds: invalid model!" );
        if ( !_rxModel.is() )
            return;

        try
        {
            Reference< XPropertySetInfo > xInfo = _rxModel->getPropertySetInfo();
            if ( !xInfo.is() || !xInfo->hasPropertyByName( FM_PROP_LINEENDFORMAT ) )
                return;

            // let's see if the data source which the form belongs to (if any)
            // has a setting for the preferred line end format
            bool bDosLineEnds = false;
            Sequence< PropertyValue > aInfo = lcl_getDataSourceIndirectProperties( _rxModel, m_pData->m_xContext );
            const PropertyValue* pInfo = aInfo.getConstArray();
            const PropertyValue* pInfoEnd = pInfo + aInfo.getLength();
            for ( ; pInfo != pInfoEnd; ++pInfo )
            {
                if ( pInfo->Name == "PreferDosLikeLineEnds" )
                {
                    pInfo->Value >>= bDosLineEnds;
                    break;
                }
            }

            sal_Int16 nLineEndFormat = bDosLineEnds ? LineEndFormat::CARRIAGE_RETURN_LINE_FEED : LineEndFormat::LINE_FEED;
            _rxModel->setPropertyValue( FM_PROP_LINEENDFORMAT, makeAny( nLineEndFormat ) );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    void FormControlFactory::initializeFieldDependentProperties( const Reference< XPropertySet >& _rxDatabaseField,
        const Reference< XPropertySet >& _rxControlModel, const Reference< XNumberFormats >& _rxNumberFormats )
    {
        OSL_PRECOND( _rxDatabaseField.is() && _rxControlModel.is(),
            "FormControlFactory::initializeFieldDependentProperties: illegal params!" );
        if ( !_rxDatabaseField.is() || !_rxControlModel.is() )
            return;

        try
        {

            // if the field has a numeric format, and the model has a "Scale" property, sync it
            Reference< XPropertySetInfo > xFieldPSI( _rxDatabaseField->getPropertySetInfo(), UNO_SET_THROW );
            Reference< XPropertySetInfo > xModelPSI( _rxControlModel->getPropertySetInfo(), UNO_SET_THROW );

            if ( xModelPSI->hasPropertyByName( FM_PROP_DECIMAL_ACCURACY ) )
            {
                sal_Int32 nFormatKey = 0;
                if ( xFieldPSI->hasPropertyByName( FM_PROP_FORMATKEY ) )
                {
                    _rxDatabaseField->getPropertyValue( FM_PROP_FORMATKEY ) >>= nFormatKey;
                }
                else
                {
                    nFormatKey = getDefaultNumberFormat(
                        _rxDatabaseField,
                        Reference< XNumberFormatTypes >( _rxNumberFormats, UNO_QUERY ),
                        SvtSysLocale().GetLanguageTag().getLocale()
                    );
                }

                Any aScaleVal( ::comphelper::getNumberFormatDecimals( _rxNumberFormats, nFormatKey ) );
                _rxControlModel->setPropertyValue( FM_PROP_DECIMAL_ACCURACY, aScaleVal );
            }


            // minimum and maximum of the control according to the type of the database field
            sal_Int32 nDataType = DataType::OTHER;
            OSL_VERIFY( _rxDatabaseField->getPropertyValue( FM_PROP_FIELDTYPE ) >>= nDataType );

            if  (   xModelPSI->hasPropertyByName( FM_PROP_VALUEMIN )
                &&  xModelPSI->hasPropertyByName( FM_PROP_VALUEMAX )
                )
            {
                sal_Int32 nMinValue = -1000000000, nMaxValue = 1000000000;
                switch ( nDataType )
                {
                    case DataType::TINYINT  : nMinValue = 0; nMaxValue = 255; break;
                    case DataType::SMALLINT : nMinValue = -32768; nMaxValue = 32767; break;
                    case DataType::INTEGER  : nMinValue = 0x80000000; nMaxValue = 0x7FFFFFFF; break;
                        // double and singles are ignored
                }

                Any aValue;

                // both the minimum and the maximum value properties can be either Long or Double
                Property aProperty = xModelPSI->getPropertyByName( FM_PROP_VALUEMIN );
                if ( aProperty.Type.getTypeClass() == TypeClass_DOUBLE )
                    aValue <<= (double)nMinValue;
                else if ( aProperty.Type.getTypeClass() == TypeClass_LONG )
                    aValue <<= (sal_Int32)nMinValue;
                else
                {
                    OSL_FAIL( "FormControlFactory::initializeFieldDependentProperties: unexpected property type (MinValue)!" );
                }
                _rxControlModel->setPropertyValue( FM_PROP_VALUEMIN, aValue );

                // both the minimum and the maximum value properties can be either Long or Double
                aProperty = xModelPSI->getPropertyByName( FM_PROP_VALUEMAX );
                if ( aProperty.Type.getTypeClass() == TypeClass_DOUBLE )
                    aValue <<= (double)nMaxValue;
                else if ( aProperty.Type.getTypeClass() == TypeClass_LONG )
                    aValue <<= (sal_Int32)nMaxValue;
                else
                {
                    OSL_FAIL( "FormControlFactory::initializeFieldDependentProperties: unexpected property type (MaxValue)!" );
                }
                _rxControlModel->setPropertyValue( FM_PROP_VALUEMAX, aValue );
            }


            // a check box can be tristate if and only if the column it is bound to is nullable
            sal_Int16 nClassId = FormComponentType::CONTROL;
            OSL_VERIFY( _rxControlModel->getPropertyValue( FM_PROP_CLASSID ) >>= nClassId );
            if ( nClassId == FormComponentType::CHECKBOX )
            {
                sal_Int32 nNullable = ColumnValue::NULLABLE_UNKNOWN;
                OSL_VERIFY( _rxDatabaseField->getPropertyValue( FM_PROP_ISNULLABLE ) >>= nNullable );
                _rxControlModel->setPropertyValue( FM_PROP_TRISTATE, makeAny( ColumnValue::NO_NULLS != nNullable ) );
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }


    OUString FormControlFactory::getDefaultName( sal_Int16 _nClassId, const Reference< XServiceInfo >& _rxObject )
    {
        sal_uInt16 nResId(0);

        switch ( _nClassId )
        {
            case FormComponentType::COMMANDBUTTON:  nResId = RID_STR_PROPTITLE_PUSHBUTTON;  break;
            case FormComponentType::RADIOBUTTON:    nResId = RID_STR_PROPTITLE_RADIOBUTTON; break;
            case FormComponentType::CHECKBOX:       nResId = RID_STR_PROPTITLE_CHECKBOX;    break;
            case FormComponentType::LISTBOX:        nResId = RID_STR_PROPTITLE_LISTBOX;     break;
            case FormComponentType::COMBOBOX:       nResId = RID_STR_PROPTITLE_COMBOBOX;    break;
            case FormComponentType::GROUPBOX:       nResId = RID_STR_PROPTITLE_GROUPBOX;    break;
            case FormComponentType::IMAGEBUTTON:    nResId = RID_STR_PROPTITLE_IMAGEBUTTON; break;
            case FormComponentType::FIXEDTEXT:      nResId = RID_STR_PROPTITLE_FIXEDTEXT;   break;
            case FormComponentType::GRIDCONTROL:    nResId = RID_STR_PROPTITLE_DBGRID;      break;
            case FormComponentType::FILECONTROL:    nResId = RID_STR_PROPTITLE_FILECONTROL; break;
            case FormComponentType::DATEFIELD:      nResId = RID_STR_PROPTITLE_DATEFIELD;   break;
            case FormComponentType::TIMEFIELD:      nResId = RID_STR_PROPTITLE_TIMEFIELD;   break;
            case FormComponentType::NUMERICFIELD:   nResId = RID_STR_PROPTITLE_NUMERICFIELD;    break;
            case FormComponentType::CURRENCYFIELD:  nResId = RID_STR_PROPTITLE_CURRENCYFIELD;   break;
            case FormComponentType::PATTERNFIELD:   nResId = RID_STR_PROPTITLE_PATTERNFIELD;    break;
            case FormComponentType::IMAGECONTROL:   nResId = RID_STR_PROPTITLE_IMAGECONTROL;    break;
            case FormComponentType::HIDDENCONTROL:  nResId = RID_STR_PROPTITLE_HIDDEN;      break;
            case FormComponentType::SCROLLBAR:      nResId = RID_STR_PROPTITLE_SCROLLBAR;   break;
            case FormComponentType::SPINBUTTON:     nResId = RID_STR_PROPTITLE_SPINBUTTON;  break;
            case FormComponentType::NAVIGATIONBAR:  nResId = RID_STR_PROPTITLE_NAVBAR;      break;

            case FormComponentType::TEXTFIELD:
                nResId = RID_STR_PROPTITLE_EDIT;
                if ( _rxObject.is() && _rxObject->supportsService( FM_SUN_COMPONENT_FORMATTEDFIELD ) )
                    nResId = RID_STR_PROPTITLE_FORMATTED;
                break;

            default:
                nResId = RID_STR_CONTROL;     break;
        }

        return SVX_RESSTR(nResId);
    }


    OUString FormControlFactory::getDefaultUniqueName_ByComponentType( const Reference< XNameAccess >& _rxContainer,
        const Reference< XPropertySet >& _rxObject )
    {
        sal_Int16 nClassId = FormComponentType::CONTROL;
        OSL_VERIFY( _rxObject->getPropertyValue( FM_PROP_CLASSID ) >>= nClassId );
        OUString sBaseName = getDefaultName( nClassId, Reference< XServiceInfo >( _rxObject, UNO_QUERY ) );

        return getUniqueName( _rxContainer, sBaseName );
    }


    OUString FormControlFactory::getUniqueName( const Reference< XNameAccess >& _rxContainer, const OUString& _rBaseName )
    {
        sal_Int32 n = 0;
        OUString sName;
        do
        {
            OUStringBuffer aBuf( _rBaseName );
            aBuf.append( " " );
            aBuf.append( ++n );
            sName = aBuf.makeStringAndClear();
        }
        while ( _rxContainer->hasByName( sName ) );

        return sName;
    }


}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
