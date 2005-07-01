/*************************************************************************
 *
 *  $RCSfile: eformspropertyhandler.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-07-01 11:49:44 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_EFORMSPROPERTYHANDLER_HXX
#include "eformspropertyhandler.hxx"
#endif

#ifndef _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_
#include "formstrings.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_FORMMETADATA_HXX_
#include "formmetadata.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_STRINGREPRESENTATION_HXX
#include "stringrepresentation.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_
#include "linedescriptor.hxx"
#endif
#ifndef EXTENSIONS_INC_EXTENSIO_HRC
#include "extensio.hrc"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMBROWSERTOOLS_HXX_
#include "formbrowsertools.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPBROWSERUI_HXX
#include "propbrowserui.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_EFORMSHELPER_HXX
#include "eformshelper.hxx"
#endif

/** === begin UNO includes === **/
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
/** === end UNO includes === **/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#include <functional>

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::xforms;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::ui::dialogs;
    using namespace ::com::sun::star::form::binding;

    //====================================================================
    //= EFormsPropertyHandler
    //====================================================================
    //--------------------------------------------------------------------
    EFormsPropertyHandler::EFormsPropertyHandler( const Reference< XMultiServiceFactory >& _rxORB,
        const Reference< XPropertySet >& _rxIntrospectee, const Reference< frame::XModel >& _rxContextDocument,
        const Reference< XTypeConverter >& _rxTypeConverter  )
        :PropertyHandler( _rxIntrospectee, _rxTypeConverter )
        ,m_xORB( _rxORB )
    {
        if ( EFormsHelper::isEForm( _rxContextDocument ) )
            m_pHelper.reset( new EFormsHelper( m_xIntrospectee, _rxContextDocument ) );

        OSL_ENSURE( m_xORB.is(), "EFormsPropertyHandler::EFormsPropertyHandler: invalid service factory!" );
    }

    //--------------------------------------------------------------------
    EFormsPropertyHandler::~EFormsPropertyHandler( )
    {
    }

    //--------------------------------------------------------------------
    bool SAL_CALL EFormsPropertyHandler::supportsUIDescriptor( PropertyId _nPropId ) const
    {
        return true;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString EFormsPropertyHandler::getModelNamePropertyValue() const
    {
        ::rtl::OUString sModelName = m_pHelper->getCurrentFormModelName();
        if ( !sModelName.getLength() )
            sModelName = m_sBindingLessModelName;
        return sModelName;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL EFormsPropertyHandler::getPropertyValue( PropertyId _nPropId, bool _bLazy ) const
    {
        Any aReturn;

        OSL_ENSURE( m_pHelper.get(), "EFormsPropertyHandler::getPropertyValue: we don't have any SupportedProperties!" );
        if ( !m_pHelper.get() )
            return aReturn;

        try
        {
            switch ( _nPropId )
            {
            case PROPERTY_ID_LIST_BINDING:
                aReturn <<= m_pHelper->getCurrentListSourceBinding();
                break;

            case PROPERTY_ID_XML_DATA_MODEL:
                aReturn <<= getModelNamePropertyValue();
                break;

            case PROPERTY_ID_BINDING_NAME:
                aReturn <<= m_pHelper->getCurrentBindingName();
                break;

            case PROPERTY_ID_BIND_EXPRESSION:
            case PROPERTY_ID_XSD_CONSTRAINT:
            case PROPERTY_ID_XSD_CALCULATION:
            case PROPERTY_ID_XSD_REQUIRED:
            case PROPERTY_ID_XSD_RELEVANT:
            case PROPERTY_ID_XSD_READONLY:
            {
                Reference< XPropertySet > xBindingProps( m_pHelper->getCurrentBinding() );
                if ( xBindingProps.is() )
                {
                    aReturn = xBindingProps->getPropertyValue( getPropertyNameFromId( _nPropId  ) );
                    DBG_ASSERT( aReturn.getValueType().equals( ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ) ),
                        "EFormsPropertyHandler::getPropertyValue: invalid BindingExpression value type!" );
                }
                else
                    aReturn <<= ::rtl::OUString();
            }
            break;

            default:
                DBG_ERROR( "EFormsPropertyHandler::getPropertyValue: cannot handle this property!" );
            }
        }
        catch( const Exception& )
        {
#if OSL_DEBUG_LEVEL > 0
            ::rtl::OUString sPropertyName( getPropertyNameFromId( _nPropId ) );
            ::rtl::OString sMessage( "EFormsPropertyHandler::getPropertyValue: caught an exception!" );
            sMessage += "\n(have been asked for the \"";
            sMessage += ::rtl::OString( sPropertyName.getStr(), sPropertyName.getLength(), RTL_TEXTENCODING_ASCII_US );
            sMessage += "\" property.)";
            OSL_ENSURE( sal_False, sMessage.getStr() );
#endif
        }
        return aReturn;
    }

    //--------------------------------------------------------------------
    void SAL_CALL EFormsPropertyHandler::setPropertyValue( PropertyId _nPropId, const Any& _rValue )
    {
        OSL_ENSURE( m_pHelper.get(), "EFormsPropertyHandler::setPropertyValue: we don't have any SupportedProperties!" );
        if ( !m_pHelper.get() )
            return;

        try
        {
            switch ( _nPropId )
            {
            case PROPERTY_ID_LIST_BINDING:
            {
                Reference< XListEntrySource > xSource;
                OSL_VERIFY( _rValue >>= xSource );
                m_pHelper->setListSourceBinding( xSource );
            }
            break;

            case PROPERTY_ID_XML_DATA_MODEL:
            {
                OSL_VERIFY( _rValue >>= m_sBindingLessModelName );

                // if the model changed, reset the binding to NULL
                if ( m_pHelper->getCurrentFormModelName() != m_sBindingLessModelName )
                {
                    ::rtl::OUString sOldBindingName = m_pHelper->getCurrentBindingName();
                    m_pHelper->setBinding( NULL );
                    firePropertyChange( PROPERTY_BINDING_NAME, PROPERTY_ID_BINDING_NAME,
                        makeAny( sOldBindingName ), makeAny( ::rtl::OUString() ) );
                }
            }
            break;

            case PROPERTY_ID_BINDING_NAME:
            {
                ::rtl::OUString sNewBindingName;
                OSL_VERIFY( _rValue >>= sNewBindingName );

                bool bPreviouslyEmptyModel = !m_pHelper->getCurrentFormModel().is();

                Reference< XPropertySet > xNewBinding;
                if ( sNewBindingName.getLength() )
                    // obtain the binding with this name, for the current model
                    xNewBinding = m_pHelper->getOrCreateBindingForModel( getModelNamePropertyValue(), sNewBindingName );

                m_pHelper->setBinding( xNewBinding );

                if ( bPreviouslyEmptyModel )
                {   // simulate a property change for the model property
                    // This is because we "simulate" the Model property by remembering the
                    // value ourself. Other instances might, however, not know this value,
                    // but prefer retrieve it somewhere else - e.g. from the EFormsHelper
                    //
                    // The really correct solution would be if *all* property handlers
                    // obtain a "current property value" for *all* properties from a central
                    // instance. Then, handler A could ask it for the value of property
                    // X, and this request would be re-routed to handler B, which ultimately
                    // knows the current value.
                    // However, there's no such mechanism in place currently.
                    firePropertyChange( PROPERTY_XML_DATA_MODEL, PROPERTY_ID_XML_DATA_MODEL,
                        makeAny( ::rtl::OUString() ), makeAny( getModelNamePropertyValue() ) );
                }
            }
            break;

            case PROPERTY_ID_BIND_EXPRESSION:
            {
                Reference< XPropertySet > xBinding( m_pHelper->getCurrentBinding() );
                OSL_ENSURE( xBinding.is(), "You should not reach this without an active binding!" );
                if ( xBinding.is() )
                    xBinding->setPropertyValue( PROPERTY_BIND_EXPRESSION, _rValue );
            }
            break;

            case PROPERTY_ID_XSD_REQUIRED:
            case PROPERTY_ID_XSD_RELEVANT:
            case PROPERTY_ID_XSD_READONLY:
            case PROPERTY_ID_XSD_CONSTRAINT:
            case PROPERTY_ID_XSD_CALCULATION:
            {
                Reference< XPropertySet > xBindingProps( m_pHelper->getCurrentBinding() );
                DBG_ASSERT( xBindingProps.is(), "EFormsPropertyHandler::setPropertyValue: how can I set a property if there's no binding?" );
                if ( xBindingProps.is() )
                {
                    DBG_ASSERT( _rValue.getValueType().equals( ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ) ),
                        "EFormsPropertyHandler::setPropertyValue: invalid value type!" );
                    xBindingProps->setPropertyValue( getPropertyNameFromId( _nPropId ), _rValue );
                }
            }
            break;

            default:
                DBG_ERROR( "EFormsPropertyHandler::setPropertyValue: cannot handle this property!" );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EFormsPropertyHandler::setPropertyValue: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    ::std::vector< Property > SAL_CALL EFormsPropertyHandler::implDescribeSupportedProperties() const
    {
        ::std::vector< Property > aProperties;

        if ( m_pHelper.get() )
        {
            if ( m_pHelper->canBindToAnyDataType() )
            {
                aProperties.reserve( 7 );
                addStringPropertyDescription( aProperties, PROPERTY_XML_DATA_MODEL );
                addStringPropertyDescription( aProperties, PROPERTY_BINDING_NAME );
                addStringPropertyDescription( aProperties, PROPERTY_BIND_EXPRESSION );
                addStringPropertyDescription( aProperties, PROPERTY_XSD_REQUIRED );
                addStringPropertyDescription( aProperties, PROPERTY_XSD_RELEVANT );
                addStringPropertyDescription( aProperties, PROPERTY_XSD_READONLY );
                addStringPropertyDescription( aProperties, PROPERTY_XSD_CONSTRAINT );
                addStringPropertyDescription( aProperties, PROPERTY_XSD_CALCULATION );
            }
            if ( m_pHelper->isListEntrySink() )
            {
                implAddPropertyDescription( aProperties, PROPERTY_LIST_BINDING,
                    ::getCppuType( static_cast< Reference< XListEntrySource > * >( NULL ) ) );
            }
        }

        return aProperties;
    }

    //--------------------------------------------------------------------
    Any SAL_CALL EFormsPropertyHandler::getPropertyValueFromStringRep( PropertyId _nPropId, const ::rtl::OUString& _rStringRep ) const
    {
        Any aReturn;

        OSL_ENSURE( m_pHelper.get(), "EFormsPropertyHandler::getPropertyValueFromStringRep: we have no SupportedProperties!" );
        if ( !m_pHelper.get() )
            return aReturn;

        switch ( _nPropId )
        {
        case PROPERTY_ID_LIST_BINDING:
        {
            Reference< XListEntrySource > xListSource( m_pHelper->getModelElementFromUIName( EFormsHelper::Binding, _rStringRep ), UNO_QUERY );
            OSL_ENSURE( xListSource.is() || !m_pHelper->getModelElementFromUIName( EFormsHelper::Binding, _rStringRep ).is(),
                "EFormsPropertyHandler::getPropertyValueFromStringRep: there's a binding which is no ListEntrySource!" );
            aReturn <<= xListSource;
        }
        break;

        default:
            aReturn = PropertyHandler::getPropertyValueFromStringRep( _nPropId, _rStringRep );
            break;
        }

        return aReturn;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL EFormsPropertyHandler::getStringRepFromPropertyValue( PropertyId _nPropId, const Any& _rValue ) const
    {
        ::rtl::OUString sReturn;

        OSL_ENSURE( m_pHelper.get(), "EFormsPropertyHandler::getStringRepFromPropertyValue: we have no SupportedProperties!" );
        if ( !m_pHelper.get() )
            return sReturn;

        switch ( _nPropId )
        {
        case PROPERTY_ID_LIST_BINDING:
        {
            Reference< XPropertySet > xListSourceBinding( _rValue, UNO_QUERY );
            if ( xListSourceBinding.is() )
                sReturn = m_pHelper->getModelElementUIName( EFormsHelper::Binding, xListSourceBinding );
        }
        break;

        default:
            sReturn = PropertyHandler::getStringRepFromPropertyValue( _nPropId, _rValue );
            break;
        }

        return sReturn;
    }

    //--------------------------------------------------------------------
    ::std::vector< ::rtl::OUString > SAL_CALL EFormsPropertyHandler::getActuatingProperties( ) const
    {
        if ( !m_pHelper.get() )
            return ::std::vector< ::rtl::OUString >();

        ::std::vector< ::rtl::OUString > aInterestedInActuations( 2 );
        aInterestedInActuations[ 0 ] = PROPERTY_XML_DATA_MODEL;
        aInterestedInActuations[ 1 ] = PROPERTY_BINDING_NAME;
        return aInterestedInActuations;
    }

    //--------------------------------------------------------------------
    void SAL_CALL EFormsPropertyHandler::initializePropertyUI( PropertyId _nPropId, IPropertyBrowserUI* _pUpdater )
    {
        // nothing to do
        OSL_ENSURE( m_pHelper.get(), "EFormsPropertyHandler::initializePropertyUI: we do not have any SupportedProperties!" );
    }

    //--------------------------------------------------------------------
    void SAL_CALL EFormsPropertyHandler::describePropertyUI( PropertyId _nPropId, PropertyUIDescriptor& /* [out] */ _rDescriptor ) const
    {
        OSL_ENSURE( m_pHelper.get(), "EFormsPropertyHandler::describePropertyUI: we do not have any SupportedProperties!" );
        if ( !m_pHelper.get() )
            return;

        _rDescriptor.clear();
        _rDescriptor.eControlType = BCT_EDIT;

        switch ( _nPropId )
        {
        case PROPERTY_ID_LIST_BINDING:
            _rDescriptor.eControlType = BCT_LISTBOX;
            const_cast< EFormsHelper* >( m_pHelper.get() )->getAllElementUINames( EFormsHelper::Binding, _rDescriptor.aListValues, true );
            break;

        case PROPERTY_ID_XML_DATA_MODEL:
            _rDescriptor.eControlType = BCT_LISTBOX;
            m_pHelper->getFormModelNames( _rDescriptor.aListValues );
            break;

        case PROPERTY_ID_BINDING_NAME:
        {
            _rDescriptor.eControlType = BCT_COMBOBOX;
            ::rtl::OUString sCurrentModel( getModelNamePropertyValue() );
            if ( sCurrentModel.getLength() )
                m_pHelper->getBindingNames( sCurrentModel, _rDescriptor.aListValues );
        }
        break;

        case PROPERTY_ID_BIND_EXPRESSION:   _rDescriptor.nButtonHelpId = UID_PROP_DLG_BIND_EXPRESSION; break;
        case PROPERTY_ID_XSD_REQUIRED:      _rDescriptor.nButtonHelpId = UID_PROP_DLG_XSD_REQUIRED;    break;
        case PROPERTY_ID_XSD_RELEVANT:      _rDescriptor.nButtonHelpId = UID_PROP_DLG_XSD_RELEVANT;    break;
        case PROPERTY_ID_XSD_READONLY:      _rDescriptor.nButtonHelpId = UID_PROP_DLG_XSD_READONLY;    break;
        case PROPERTY_ID_XSD_CONSTRAINT:    _rDescriptor.nButtonHelpId = UID_PROP_DLG_XSD_CONSTRAINT;  break;
        case PROPERTY_ID_XSD_CALCULATION:   _rDescriptor.nButtonHelpId = UID_PROP_DLG_XSD_CALCULATION; break;

        default:
            DBG_ERROR( "EFormsPropertyHandler::describePropertyUI: cannot handle this property!" );
        }
    }

    //--------------------------------------------------------------------
    bool SAL_CALL EFormsPropertyHandler::requestUserInputOnButtonClick( PropertyId _nPropId, bool _bPrimary, Any& _rData )
    {
        OSL_ENSURE( m_pHelper.get(), "EFormsPropertyHandler::requestUserInputOnButtonClick: we do not have any SupportedProperties!" );
        if ( !m_pHelper.get() )
            return false;

        OSL_ENSURE( ( PROPERTY_ID_BINDING_NAME == _nPropId )
                 || ( PROPERTY_ID_BIND_EXPRESSION == _nPropId )
                 || ( PROPERTY_ID_XSD_REQUIRED == _nPropId )
                 || ( PROPERTY_ID_XSD_RELEVANT == _nPropId )
                 || ( PROPERTY_ID_XSD_READONLY == _nPropId )
                 || ( PROPERTY_ID_XSD_CONSTRAINT == _nPropId )
                 || ( PROPERTY_ID_XSD_CALCULATION == _nPropId ), "EFormsPropertyHandler::requestUserInputOnButtonClick: unexpected!" );

        try
        {
            Reference< XExecutableDialog > xDialog;
            if ( m_xORB.is() )
                xDialog = xDialog.query( m_xORB->createInstance( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "com.sun.star.xforms.ui.dialogs.AddCondition" ) ) ) );

            Reference< XPropertySet > xDialogProps( xDialog, UNO_QUERY );
            OSL_ENSURE( xDialogProps.is(), "EFormsPropertyHandler::requestUserInputOnButtonClick: invalid (or unavailable) dialog!" );
            if ( !xDialogProps.is() )
                return false;

            // the model for the dialog to work with
            Reference< xforms::XModel > xModel( m_pHelper->getCurrentFormModel() );
            // the binding for the dialog to work with
            Reference< XPropertySet > xBinding( m_pHelper->getCurrentBinding() );
            // the aspect of the binding which the dialog should modify
            ::rtl::OUString sFacetName( getPropertyNameFromId( _nPropId ) );

            OSL_ENSURE( xModel.is() && xBinding.is() && sFacetName.getLength(),
                "EFormsPropertyHandler::requestUserInputOnButtonClick: something is missing for the dialog initialization!" );
            if ( !( xModel.is() && xBinding.is() && sFacetName.getLength() ) )
                return false;

            xDialogProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FormModel" ) ), makeAny( xModel ) );
            xDialogProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "Binding" ) ), makeAny( xBinding ) );
            xDialogProps->setPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "FacetName" ) ), makeAny( sFacetName ) );

            if ( !xDialog->execute() )
                // cancelled
                return false;

            _rData = xDialogProps->getPropertyValue( ::rtl::OUString( RTL_CONSTASCII_USTRINGPARAM( "ConditionValue" ) ) );
            return true;
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EFormsPropertyHandler::requestUserInputOnButtonClick: caught an exception!" );
        }

        // something went wrong here ...(but has been asserted already)
        return false;
    }

    //--------------------------------------------------------------------
    void SAL_CALL EFormsPropertyHandler::executeButtonClick( PropertyId _nPropId, bool _bPrimary, const Any& _rData, IPropertyBrowserUI* _pUpdater )
    {
        try
        {
            Any aOldValue = getPropertyValue( _nPropId );
            setPropertyValue( _nPropId, _rData );
            firePropertyChange( getPropertyNameFromId( _nPropId ), _nPropId, aOldValue, _rData );
                // TODO: shouldn't this be integral part of setPropertyValue?
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EFormsPropertyHandler::executeButtonClick: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL EFormsPropertyHandler::startAllPropertyChangeListening( const Reference< XPropertyChangeListener >& _rxListener )
    {
        OSL_ENSURE( m_pHelper.get(), "EFormsPropertyHandler::startAllPropertyChangeListening: we do not have any SupportedProperties!" );
        if ( !m_pHelper.get() )
            return;

        PropertyHandler::startAllPropertyChangeListening( _rxListener );
        m_pHelper->registerBindingListener( _rxListener );
    }

    //--------------------------------------------------------------------
    void SAL_CALL EFormsPropertyHandler::stopAllPropertyChangeListening( )
    {
        OSL_ENSURE( m_pHelper.get(), "EFormsPropertyHandler::stopAllPropertyChangeListening: we do not have any SupportedProperties!" );
        if ( !m_pHelper.get() )
            return;

        m_pHelper->revokeBindingListener();
        PropertyHandler::stopAllPropertyChangeListening();
    }

    //--------------------------------------------------------------------
    void SAL_CALL EFormsPropertyHandler::actuatingPropertyChanged( PropertyId _nActuatingPropId, const Any& _rNewValue, const Any& _rOldValue, IPropertyBrowserUI* _pUpdater, bool )
    {
        OSL_ENSURE( m_pHelper.get(), "EFormsPropertyHandler::actuatingPropertyChanged: we do not have any ActuatingProperties!" );
        if ( !m_pHelper.get() )
            return;

        DBG_ASSERT( _pUpdater, "EFormsPropertyHandler::actuatingPropertyChanged: invalid callback!" );
        if ( !_pUpdater )
            return;

        switch ( _nActuatingPropId )
        {
        case PROPERTY_ID_XML_DATA_MODEL:
        {
            ::rtl::OUString sDataModelName;
            OSL_VERIFY( _rNewValue >>= sDataModelName );
            bool bBoundToSomeModel = 0 != sDataModelName.getLength();
            _pUpdater->rebuildPropertyUI( PROPERTY_BINDING_NAME );
            _pUpdater->enablePropertyUI( PROPERTY_BINDING_NAME, bBoundToSomeModel );
        }
        // NO break

        case PROPERTY_ID_BINDING_NAME:
        {
            bool bHaveABinding = ( m_pHelper->getCurrentBindingName().getLength() > 0 );
            _pUpdater->enablePropertyUI( PROPERTY_BIND_EXPRESSION, bHaveABinding );
            _pUpdater->enablePropertyUI( PROPERTY_XSD_REQUIRED, bHaveABinding );
            _pUpdater->enablePropertyUI( PROPERTY_XSD_RELEVANT, bHaveABinding );
            _pUpdater->enablePropertyUI( PROPERTY_XSD_READONLY, bHaveABinding );
            _pUpdater->enablePropertyUI( PROPERTY_XSD_CONSTRAINT, bHaveABinding );
            _pUpdater->enablePropertyUI( PROPERTY_XSD_CALCULATION, bHaveABinding );
            _pUpdater->enablePropertyUI( PROPERTY_XSD_DATA_TYPE, bHaveABinding );
        }
        break;

        default:
            DBG_ERROR( "EFormsPropertyHandler::actuatingPropertyChanged: cannot handle this property!" );
        }
    }

//........................................................................
} // namespace pcr
//........................................................................

