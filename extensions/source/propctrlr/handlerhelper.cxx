/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: handlerhelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2006-07-26 07:58:12 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_HANDLERHELPER_HXX
#include "handlerhelper.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_PROPRESID_HRC_
#include "propresid.hrc"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_STRINGREPRESENTATION_HXX
#include "stringrepresentation.hxx"
#endif
#ifndef _COMPHELPER_EXTRACT_HXX_
#include <comphelper/extract.hxx>
#endif
#ifndef _EXTENSIONS_PROPCTRLR_MODULEPRC_HXX_
#include "modulepcr.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_ENUMREPRESENTATION_HXX
#include "enumrepresentation.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_FORMMETADATA_HXX_
#include "formmetadata.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PCROMPONENTCONTEXT_HXX
#include "pcrcomponentcontext.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_BEANS_PROPERTYATTRIBUTE_HPP_
#include <com/sun/star/beans/PropertyAttribute.hpp>
#endif
#ifndef _COM_SUN_STAR_UNO_XCOMPONENTCONTEXT_HPP_
#include <com/sun/star/uno/XComponentContext.hpp>
#endif
#ifndef _COM_SUN_STAR_UTIL_XMODIFIABLE_HPP_
#include <com/sun/star/util/XModifiable.hpp>
#endif
#ifndef _COM_SUN_STAR_AWT_XWINDOW_HPP_
#include <com/sun/star/awt/XWindow.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_LINEDESCRIPTOR_HPP_
#include <com/sun/star/inspection/LineDescriptor.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_PROPERTYCONTROLTYPE_HPP_
#include <com/sun/star/inspection/PropertyControlType.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_XSTRINGLISTCONTROL_HPP_
#include <com/sun/star/inspection/XStringListControl.hpp>
#endif
#ifndef _COM_SUN_STAR_INSPECTION_XNUMERICCONTROL_HPP_
#include <com/sun/star/inspection/XNumericControl.hpp>
#endif
/** === end UNO includes === **/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _TOOLKIT_HELPER_VCLUNOHELPER_HXX_
#include <toolkit/helper/vclunohelper.hxx>
#endif

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

        // special handling for bolleans (this will become a list)
        if ( _rProperty.Type.getTypeClass() == TypeClass_BOOLEAN )
        {
            String aBoolOptions = String( PcrRes( RID_STR_BOOL ) );
            Sequence< ::rtl::OUString > aEntries(2);
            for ( xub_StrLen i=0; i<2; ++i )
                aEntries[i] = aBoolOptions.GetToken( i );
            _out_rDescriptor.Control = createListBoxControl( _rxControlFactory, aEntries, bReadOnlyControl );
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
            DBG_ERROR( "PropertyHandlerHelper::describePropertyLine: don't know how to represent this at the UI!" );
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
                sal_Bool _bTrueIfListBoxFalseIfComboBox
            )
        {
            Reference< XStringListControl > xListControl(
                _rxControlFactory->createPropertyControl(
                    _bTrueIfListBoxFalseIfComboBox ? PropertyControlType::ListBox : PropertyControlType::ComboBox, _bReadOnlyControl
                ),
                UNO_QUERY_THROW
            );

            for (   ::std::vector< ::rtl::OUString >::const_iterator loop = _rInitialListEntries.begin();
                    loop != _rInitialListEntries.end();
                    ++loop
                )
                xListControl->appendListEntry( *loop );
            return xListControl.get();
        }
    }

    //--------------------------------------------------------------------
    Reference< XPropertyControl > PropertyHandlerHelper::createListBoxControl( const Reference< XPropertyControlFactory >& _rxControlFactory,
                const Sequence< ::rtl::OUString >& _rInitialListEntries, sal_Bool _bReadOnlyControl )
    {
        ::std::vector< ::rtl::OUString > aAsVector( _rInitialListEntries.getConstArray(), _rInitialListEntries.getConstArray() + _rInitialListEntries.getLength() );
        return lcl_implCreateListLikeControl( _rxControlFactory, aAsVector, _bReadOnlyControl, sal_True );
    }

    //--------------------------------------------------------------------
    Reference< XPropertyControl > PropertyHandlerHelper::createListBoxControl( const Reference< XPropertyControlFactory >& _rxControlFactory,
                const ::std::vector< ::rtl::OUString >& _rInitialListEntries, sal_Bool _bReadOnlyControl )
    {
        return lcl_implCreateListLikeControl( _rxControlFactory, _rInitialListEntries, _bReadOnlyControl, sal_True );
    }

    //--------------------------------------------------------------------
    Reference< XPropertyControl > PropertyHandlerHelper::createComboBoxControl( const Reference< XPropertyControlFactory >& _rxControlFactory,
                const Sequence< ::rtl::OUString >& _rInitialListEntries, sal_Bool _bReadOnlyControl )
    {
        ::std::vector< ::rtl::OUString > aAsVector( _rInitialListEntries.getConstArray(), _rInitialListEntries.getConstArray() + _rInitialListEntries.getLength() );
        return lcl_implCreateListLikeControl( _rxControlFactory, aAsVector, _bReadOnlyControl, sal_False );
    }

    //--------------------------------------------------------------------
    Reference< XPropertyControl > PropertyHandlerHelper::createComboBoxControl( const Reference< XPropertyControlFactory >& _rxControlFactory,
                const ::std::vector< ::rtl::OUString >& _rInitialListEntries, sal_Bool _bReadOnlyControl )
    {
        return lcl_implCreateListLikeControl( _rxControlFactory, _rInitialListEntries, _bReadOnlyControl, sal_False );
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
    Any PropertyHandlerHelper::convertToPropertyValue( const Reference< XTypeConverter >& _rxTypeConverter,
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

            StringRepresentation aConversionHelper( _rxTypeConverter );
            aPropertyValue = aConversionHelper.convertStringRepresentationToPropertyValue( sControlValue, _rProperty.Type );
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
    Any PropertyHandlerHelper::convertToControlValue( const Reference< XTypeConverter >& _rxTypeConverter,
        const Any& _rPropertyValue, const Type& _rControlValueType )
    {
        Any aControlValue( _rPropertyValue );
        if ( !aControlValue.hasValue() )
            // NULL is converted to NULL
            return aControlValue;

        if ( _rControlValueType.getTypeClass() == TypeClass_STRING )
        {
            StringRepresentation aConversionHelper( _rxTypeConverter );
            aControlValue <<= aConversionHelper.convertPropertyValueToStringRepresentation( _rPropertyValue );
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
        catch( const Exception& e )
        {
        #if OSL_DEBUG_LEVEL > 0
            ::rtl::OString sMessage( "PropertyHandlerHelper::setContextDocumentModified: caught an exception!\n" );
            sMessage += "message:\n";
            sMessage += ::rtl::OString( e.Message.getStr(), e.Message.getLength(), osl_getThreadTextEncoding() );
            OSL_ENSURE( false, sMessage );
        #else
            e; // make compiler happy
        #endif
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
        catch( const Exception& e )
        {
        #if OSL_DEBUG_LEVEL > 0
            ::rtl::OString sMessage( "PropertyHandlerHelper::getDialogParentWindow: caught an exception!\n" );
            sMessage += "message:\n";
            sMessage += ::rtl::OString( e.Message.getStr(), e.Message.getLength(), osl_getThreadTextEncoding() );
            OSL_ENSURE( false, sMessage );
        #else
            e; // make compiler happy
        #endif
        }
        return pInspectorWindow;
    }

//........................................................................
} // namespace pcr
//........................................................................
