/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
#include "precompiled_extensions.hxx"
#include "handlerhelper.hxx"
#include "propresid.hrc"
#include "formresid.hrc"
#include <comphelper/extract.hxx>
#include "modulepcr.hxx"
#include "enumrepresentation.hxx"
#include "formmetadata.hxx"
#include "pcrcomponentcontext.hxx"

/** === begin UNO includes === **/
#include "com/sun/star/inspection/StringRepresentation.hpp"
#include <com/sun/star/beans/PropertyAttribute.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/XModifiable.hpp>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/inspection/LineDescriptor.hpp>
#include <com/sun/star/inspection/PropertyControlType.hpp>
#include <com/sun/star/inspection/XStringListControl.hpp>
#include <com/sun/star/inspection/XNumericControl.hpp>
/** === end UNO includes === **/
#include <tools/debug.hxx>
#include <tools/diagnose_ex.h>
#include <tools/StringListResource.hxx>
#include <toolkit/helper/vclunohelper.hxx>

#include <algorithm>

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::awt;
    using namespace ::com::sun::star::util;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::inspection;

    //====================================================================
    //= PropertyHandlerHelper
    //====================================================================
    //--------------------------------------------------------------------
    void PropertyHandlerHelper::describePropertyLine( const Property& _rProperty,
        LineDescriptor& /* [out] */ _out_rDescriptor, const Reference< XPropertyControlFactory >& _rxControlFactory )
    {
        // display the pure property name - no L10N
        _out_rDescriptor.DisplayName = _rProperty.Name;

        OSL_PRECOND( _rxControlFactory.is(), "PropertyHandlerHelper::describePropertyLine: no factory -> no control!" );
        if ( !_rxControlFactory.is() )
            return;

        sal_Bool bReadOnlyControl = requiresReadOnlyControl( _rProperty.Attributes );

        // special handling for booleans (this will become a list)
        if ( _rProperty.Type.getTypeClass() == TypeClass_BOOLEAN )
        {
            ::std::vector< ::rtl::OUString > aListEntries;
            tools::StringListResource aRes(PcrRes(RID_RSC_ENUM_YESNO),aListEntries);
            _out_rDescriptor.Control = createListBoxControl( _rxControlFactory, aListEntries, bReadOnlyControl, sal_False );
            return;
        }

        sal_Int16 nControlType = PropertyControlType::TextField;
        switch ( _rProperty.Type.getTypeClass() )
        {
        case TypeClass_BYTE:
        case TypeClass_SHORT:
        case TypeClass_UNSIGNED_SHORT:
        case TypeClass_LONG:
        case TypeClass_UNSIGNED_LONG:
        case TypeClass_HYPER:
        case TypeClass_UNSIGNED_HYPER:
        case TypeClass_FLOAT:
        case TypeClass_DOUBLE:
            nControlType = PropertyControlType::NumericField;
            break;

        case TypeClass_SEQUENCE:
            nControlType = PropertyControlType::StringListField;
            break;

        default:
            OSL_FAIL( "PropertyHandlerHelper::describePropertyLine: don't know how to represent this at the UI!" );
            // NO break!

        case TypeClass_STRING:
            nControlType = PropertyControlType::TextField;
            break;
        }

        // create a control
        _out_rDescriptor.Control = _rxControlFactory->createPropertyControl( nControlType, bReadOnlyControl );
    }

    //--------------------------------------------------------------------
    namespace
    {
        Reference< XPropertyControl > lcl_implCreateListLikeControl(
                const Reference< XPropertyControlFactory >& _rxControlFactory,
                const ::std::vector< ::rtl::OUString >& _rInitialListEntries,
                sal_Bool _bReadOnlyControl,
                sal_Bool _bSorted,
                sal_Bool _bTrueIfListBoxFalseIfComboBox
            )
        {
            Reference< XStringListControl > xListControl(
                _rxControlFactory->createPropertyControl(
                    _bTrueIfListBoxFalseIfComboBox ? PropertyControlType::ListBox : PropertyControlType::ComboBox, _bReadOnlyControl
                ),
                UNO_QUERY_THROW
            );

            ::std::vector< ::rtl::OUString > aInitialEntries( _rInitialListEntries );
            if ( _bSorted )
                ::std::sort( aInitialEntries.begin(), aInitialEntries.end() );

            for (   ::std::vector< ::rtl::OUString >::const_iterator loop = aInitialEntries.begin();
                    loop != aInitialEntries.end();
                    ++loop
                )
                xListControl->appendListEntry( *loop );
            return xListControl.get();
        }
    }

    //--------------------------------------------------------------------
    Reference< XPropertyControl > PropertyHandlerHelper::createListBoxControl( const Reference< XPropertyControlFactory >& _rxControlFactory,
                const ::std::vector< ::rtl::OUString >& _rInitialListEntries, sal_Bool _bReadOnlyControl, sal_Bool _bSorted )
    {
        return lcl_implCreateListLikeControl( _rxControlFactory, _rInitialListEntries, _bReadOnlyControl, _bSorted, sal_True );
    }

    //--------------------------------------------------------------------
    Reference< XPropertyControl > PropertyHandlerHelper::createComboBoxControl( const Reference< XPropertyControlFactory >& _rxControlFactory,
                const ::std::vector< ::rtl::OUString >& _rInitialListEntries, sal_Bool _bReadOnlyControl, sal_Bool _bSorted )
    {
        return lcl_implCreateListLikeControl( _rxControlFactory, _rInitialListEntries, _bReadOnlyControl, _bSorted, sal_False );
    }

    //--------------------------------------------------------------------
    Reference< XPropertyControl > PropertyHandlerHelper::createNumericControl( const Reference< XPropertyControlFactory >& _rxControlFactory,
            sal_Int16 _nDigits, const Optional< double >& _rMinValue, const Optional< double >& _rMaxValue, sal_Bool _bReadOnlyControl )
    {
        Reference< XNumericControl > xNumericControl(
            _rxControlFactory->createPropertyControl( PropertyControlType::NumericField, _bReadOnlyControl ),
            UNO_QUERY_THROW
        );

        xNumericControl->setDecimalDigits( _nDigits );
        xNumericControl->setMinValue( _rMinValue );
        xNumericControl->setMaxValue( _rMaxValue );

        return xNumericControl.get();
    }

    //--------------------------------------------------------------------
    Any PropertyHandlerHelper::convertToPropertyValue( const Reference< XComponentContext >& _rxContext,const Reference< XTypeConverter >& _rxTypeConverter,
        const Property& _rProperty, const Any& _rControlValue )
    {
        Any aPropertyValue( _rControlValue );
        if ( !aPropertyValue.hasValue() )
            // NULL is converted to NULL
            return aPropertyValue;

        if ( aPropertyValue.getValueType().equals( _rProperty.Type ) )
            // nothing to do, type is already as desired
            return aPropertyValue;

        if ( _rControlValue.getValueType().getTypeClass() == TypeClass_STRING )
        {
            ::rtl::OUString sControlValue;
            _rControlValue >>= sControlValue;

            Reference< XStringRepresentation > xConversionHelper = StringRepresentation::create( _rxContext,_rxTypeConverter );
            aPropertyValue = xConversionHelper->convertToPropertyValue( sControlValue, _rProperty.Type );
        }
        else
        {
            try
            {
                if ( _rxTypeConverter.is() )
                    aPropertyValue = _rxTypeConverter->convertTo( _rControlValue, _rProperty.Type );
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "PropertyHandlerHelper::convertToPropertyValue: caught an exception while converting via TypeConverter!" );
            }
        }

        return aPropertyValue;
    }

    //--------------------------------------------------------------------
    Any PropertyHandlerHelper::convertToControlValue( const Reference< XComponentContext >& _rxContext,const Reference< XTypeConverter >& _rxTypeConverter,
        const Any& _rPropertyValue, const Type& _rControlValueType )
    {
        Any aControlValue( _rPropertyValue );
        if ( !aControlValue.hasValue() )
            // NULL is converted to NULL
            return aControlValue;

        if ( _rControlValueType.getTypeClass() == TypeClass_STRING )
        {
            Reference< XStringRepresentation > xConversionHelper = StringRepresentation::create( _rxContext,_rxTypeConverter );
            aControlValue <<= xConversionHelper->convertToControlValue( _rPropertyValue );
        }
        else
        {
            try
            {
                if ( _rxTypeConverter.is() )
                    aControlValue = _rxTypeConverter->convertTo( _rPropertyValue, _rControlValueType );
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "PropertyHandlerHelper::convertToControlValue: caught an exception while converting via TypeConverter!" );
            }
        }

        return aControlValue;
    }

    //--------------------------------------------------------------------
    void PropertyHandlerHelper::setContextDocumentModified( const ComponentContext& _rContext )
    {
        try
        {
            Reference< XModifiable > xDocumentModifiable( _rContext.getContextValueByAsciiName( "ContextDocument" ), UNO_QUERY_THROW );
            xDocumentModifiable->setModified( sal_True );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }

    //--------------------------------------------------------------------
    Window* PropertyHandlerHelper::getDialogParentWindow( const ComponentContext& _rContext )
    {
        Window* pInspectorWindow = NULL;
        try
        {
            Reference< XWindow > xInspectorWindow( _rContext.getContextValueByAsciiName( "DialogParentWindow" ), UNO_QUERY_THROW );
            pInspectorWindow = VCLUnoHelper::GetWindow( xInspectorWindow );
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
        return pInspectorWindow;
    }

//........................................................................
} // namespace pcr
//........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
