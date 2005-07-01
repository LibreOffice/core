/*************************************************************************
 *
 *  $RCSfile: eformshelper.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-07-01 11:49:11 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_EFORMSHELPER_HXX
#include "eformshelper.hxx"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_
#include "formstrings.hxx"
#endif
#ifndef _EXTENSIONS_FORMCTRLR_PROPRESID_HRC_
#include "formresid.hrc"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_MODULEPRC_HXX_
#include "modulepcr.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_LANG_XSERVICEINFO_HPP_
#include <com/sun/star/lang/XServiceInfo.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_FORMCOMPONENTTYPE_HPP_
#include <com/sun/star/form/FormComponentType.hpp>
#endif
#ifndef _COM_SUN_STAR_XFORMS_XFORMSUIHELPER1_HPP_
#include <com/sun/star/xforms/XFormsUIHelper1.hpp>
#endif
#ifndef _COM_SUN_STAR_XSD_DATATYPECLASS_HPP_
#include <com/sun/star/xsd/DataTypeClass.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_BINDING_XLISTENTRYSINK_HPP_
#include <com/sun/star/form/binding/XListEntrySink.hpp>
#endif
/** === end UNO includes === **/

#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

#include <functional>
#include <algorithm>
#include <set>

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::container;
    using namespace ::com::sun::star::form::binding;
    using namespace ::com::sun::star::xsd;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::form;

    //====================================================================
    //= file-local helpers
    //====================================================================
    namespace
    {
        //--------------------------------------------------------------------
        ::rtl::OUString composeModelElementUIName( const ::rtl::OUString& _rModelName, const ::rtl::OUString& _rElementName )
        {
            ::rtl::OUStringBuffer aBuffer;
            aBuffer.appendAscii( "[" );
            aBuffer.append( _rModelName );
            aBuffer.appendAscii( "] " );
            aBuffer.append( _rElementName );
            return aBuffer.makeStringAndClear();
        }
    }

    //====================================================================
    //= EFormsHelper
    //====================================================================
    //--------------------------------------------------------------------
    EFormsHelper::EFormsHelper( const Reference< XPropertySet >& _rxControlModel, const Reference< frame::XModel >& _rxContextDocument )
        :m_xControlModel( _rxControlModel )
    {
        OSL_ENSURE( _rxControlModel.is(), "EFormsHelper::EFormsHelper: invalid control model!" );
        m_xBindableControl = m_xBindableControl.query( _rxControlModel );

        m_xDocument = m_xDocument.query( _rxContextDocument );
        OSL_ENSURE( m_xDocument.is(), "EFormsHelper::EFormsHelper: invalid document!" );

    }

    //--------------------------------------------------------------------
    bool EFormsHelper::isEForm( const Reference< frame::XModel >& _rxContextDocument )
    {
        try
        {
            Reference< xforms::XFormsSupplier > xDocument( _rxContextDocument, UNO_QUERY );
            if ( !xDocument.is() )
                return false;

            return xDocument->getXForms().is();
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EFormsHelper::isEForm: caught an exception!" );
        }
        return false;
    }

    //--------------------------------------------------------------------
    bool EFormsHelper::canBindToDataType( sal_Int32 _nDataType ) const SAL_THROW(())
    {
        if ( !m_xBindableControl.is() )
            // cannot bind at all
            return false;

        // some types cannot be bound, independent from the control type
        if (  ( DataTypeClass::hexBinary == _nDataType )
           || ( DataTypeClass::base64Binary == _nDataType )
           || ( DataTypeClass::QName == _nDataType )
           || ( DataTypeClass::NOTATION == _nDataType )
           )
           return false;

        bool bCan = false;
        try
        {
            // classify the control model
            sal_Int16 nControlType = FormComponentType::CONTROL;
            OSL_VERIFY( m_xControlModel->getPropertyValue( PROPERTY_CLASSID ) >>= nControlType );

            // some lists
            sal_Int16 nNumericCompatibleTypes[] = { DataTypeClass::DECIMAL, DataTypeClass::FLOAT, DataTypeClass::DOUBLE, 0 };
            sal_Int16 nDateCompatibleTypes[] = { DataTypeClass::DATE, 0 };
            sal_Int16 nTimeCompatibleTypes[] = { DataTypeClass::TIME, 0 };
            sal_Int16 nCheckboxCompatibleTypes[] = { DataTypeClass::BOOLEAN, DataTypeClass::STRING, DataTypeClass::anyURI, 0 };
            sal_Int16 nRadiobuttonCompatibleTypes[] = { DataTypeClass::STRING, DataTypeClass::anyURI, 0 };
            sal_Int16 nFormattedCompatibleTypes[] = { DataTypeClass::DECIMAL, DataTypeClass::FLOAT, DataTypeClass::DOUBLE, DataTypeClass::DATETIME, DataTypeClass::DATE, DataTypeClass::TIME, 0 };

            sal_Int16* pCompatibleTypes = NULL;
            switch ( nControlType )
            {
            case FormComponentType::SPINBUTTON:
            case FormComponentType::NUMERICFIELD:
                pCompatibleTypes = nNumericCompatibleTypes;
                break;
            case FormComponentType::DATEFIELD:
                pCompatibleTypes = nDateCompatibleTypes;
                break;
            case FormComponentType::TIMEFIELD:
                pCompatibleTypes = nTimeCompatibleTypes;
                break;
            case FormComponentType::CHECKBOX:
                pCompatibleTypes = nCheckboxCompatibleTypes;
                break;
            case FormComponentType::RADIOBUTTON:
                pCompatibleTypes = nRadiobuttonCompatibleTypes;
                break;

            case FormComponentType::TEXTFIELD:
            {
                // both the normal text field, and the formatted field, claim to be a TEXTFIELD
                // need to distinguish by service name
                Reference< XServiceInfo > xSI( m_xControlModel, UNO_QUERY );
                OSL_ENSURE( xSI.is(), "EFormsHelper::canBindToDataType: a control model which has no service info?" );
                if ( xSI.is() )
                {
                    if ( xSI->supportsService( SERVICE_COMPONENT_FORMATTEDFIELD ) )
                    {
                        pCompatibleTypes = nFormattedCompatibleTypes;
                        break;
                    }
                }
                // NO break here!
            }
            case FormComponentType::LISTBOX:
            case FormComponentType::COMBOBOX:
                // edit fields and list/combo boxes can be bound to anything
                bCan = true;
            }

            if ( !bCan && pCompatibleTypes )
            {
                if ( _nDataType == -1 )
                {
                    // the control can be bound to at least one type, and exactly this is being asked for
                    bCan = true;
                }
                else
                {
                    while ( *pCompatibleTypes && !bCan )
                        bCan = ( *pCompatibleTypes++ == _nDataType );
                }
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EFormsHelper::canBindToDataType: caught an exception!" );
        }

        return bCan;
    }

    //--------------------------------------------------------------------
    bool EFormsHelper::isListEntrySink() const SAL_THROW(())
    {
        bool bIs = false;
        try
        {
            Reference< XListEntrySink > xAsSink( m_xControlModel, UNO_QUERY );
            bIs = xAsSink.is();
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EFormsHelper::isListEntrySink: caught an exception!" );
        }
        return bIs;
    }

    //--------------------------------------------------------------------
    void EFormsHelper::switchBindingListening( bool _bDoListening )
    {
        try
        {
            Reference< XPropertySet > xProps( m_xBindableControl, UNO_QUERY );
            if ( !xProps.is() || !m_xBindingListener.is() )
                return;

            if ( _bDoListening )
            {
                xProps->addPropertyChangeListener( ::rtl::OUString(), m_xBindingListener );
            }
            else
            {
                xProps->removePropertyChangeListener( ::rtl::OUString(), m_xBindingListener );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EFormsHelper::switchBindingListening: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void EFormsHelper::registerBindingListener( const Reference< XPropertyChangeListener >& _rxBindingListener )
    {
        switchBindingListening( false );
        m_xBindingListener = _rxBindingListener;
        switchBindingListening( true );
    }

    //--------------------------------------------------------------------
    void EFormsHelper::revokeBindingListener()
    {
        switchBindingListening( false );
        m_xBindingListener.clear();
    }

    //--------------------------------------------------------------------
    void EFormsHelper::getFormModelNames( ::std::vector< ::rtl::OUString >& /* [out] */ _rModelNames ) const SAL_THROW(())
    {
        if ( m_xDocument.is() )
        {
            try
            {
                _rModelNames.resize( 0 );

                Reference< XNameContainer > xForms( m_xDocument->getXForms() );
                OSL_ENSURE( xForms.is(), "EFormsHelper::getFormModelNames: invalid forms container!" );
                if ( xForms.is() )
                {
                    Sequence< ::rtl::OUString > aModelNames = xForms->getElementNames();
                    _rModelNames.resize( aModelNames.getLength() );
                    ::std::copy( aModelNames.getConstArray(), aModelNames.getConstArray() + aModelNames.getLength(),
                        _rModelNames.begin()
                    );
                }
            }
            catch( const Exception& )
            {
                OSL_ENSURE( sal_False, "EFormsHelper::getFormModelNames: caught an exception!" );
            }
        }
    }

    //--------------------------------------------------------------------
    void EFormsHelper::getBindingNames( const ::rtl::OUString& _rModelName, ::std::vector< ::rtl::OUString >& /* [out] */ _rBindingNames ) const SAL_THROW(())
    {
        _rBindingNames.resize( 0 );
        try
        {
            Reference< xforms::XModel > xModel( getFormModelByName( _rModelName ) );
            if ( xModel.is() )
            {
                Reference< XNameAccess > xBindings( xModel->getBindings(), UNO_QUERY );
                OSL_ENSURE( xBindings.is(), "EFormsHelper::getBindingNames: invalid bindings container obtained from the model!" );
                if ( xBindings.is() )
                {
                    Sequence< ::rtl::OUString > aNames = xBindings->getElementNames();
                    _rBindingNames.resize( aNames.getLength() );
                    ::std::copy( aNames.getConstArray(), aNames.getConstArray() + aNames.getLength(), _rBindingNames.begin() );
                }
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EFormsHelper::getBindingNames: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    Reference< xforms::XModel > EFormsHelper::getFormModelByName( const ::rtl::OUString& _rModelName ) const SAL_THROW(())
    {
        Reference< xforms::XModel > xReturn;
        try
        {
            Reference< XNameContainer > xForms( m_xDocument->getXForms() );
            OSL_ENSURE( xForms.is(), "EFormsHelper::getFormModelByName: invalid forms container!" );
            if ( xForms.is() )
                OSL_VERIFY( xForms->getByName( _rModelName ) >>= xReturn );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EFormsHelper::getFormModelByName: caught an exception!" );
        }
        return xReturn;
    }

    //--------------------------------------------------------------------
    Reference< xforms::XModel > EFormsHelper::getCurrentFormModel() const SAL_THROW(())
    {
        Reference< xforms::XModel > xModel;
        try
        {
            Reference< XPropertySet > xBinding( getCurrentBinding() );
            if ( xBinding.is() )
            {
                OSL_VERIFY( xBinding->getPropertyValue( PROPERTY_MODEL ) >>= xModel );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EFormsHelper::getCurrentFormModel: caught an exception!" );
        }
        return xModel;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString EFormsHelper::getCurrentFormModelName() const SAL_THROW(())
    {
        ::rtl::OUString sModelName;
        try
        {
            Reference< xforms::XModel > xFormsModel( getCurrentFormModel() );
            if ( xFormsModel.is() )
                sModelName = xFormsModel->getID();
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EFormsHelper::getCurrentFormModel: caught an exception!" );
        }
        return sModelName;
    }

    //--------------------------------------------------------------------
    Reference< XPropertySet > EFormsHelper::getCurrentBinding() const SAL_THROW(())
    {
        Reference< XPropertySet > xBinding;

        try
        {
            if ( m_xBindableControl.is() )
                xBinding = xBinding.query( m_xBindableControl->getValueBinding() );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EFormsHelper::getCurrentBinding: caught an exception!" );
        }

        return xBinding;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString EFormsHelper::getCurrentBindingName() const SAL_THROW(())
    {
        ::rtl::OUString sBindingName;
        try
        {
            Reference< XPropertySet > xBinding( getCurrentBinding() );
            if ( xBinding.is() )
                xBinding->getPropertyValue( PROPERTY_BINDING_ID ) >>= sBindingName;
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EFormsHelper::getCurrentBindingName: caught an exception!" );
        }
        return sBindingName;
    }

    //--------------------------------------------------------------------
    Reference< XListEntrySource > EFormsHelper::getCurrentListSourceBinding() const SAL_THROW(())
    {
        Reference< XListEntrySource > xReturn;
        try
        {
            Reference< XListEntrySink > xAsSink( m_xControlModel, UNO_QUERY );
            OSL_ENSURE( xAsSink.is(), "EFormsHelper::getCurrentListSourceBinding: you should have used isListEntrySink before!" );
            if ( xAsSink.is() )
                xReturn = xAsSink->getListEntrySource();
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EFormsHelper::getCurrentListSourceBinding: caught an exception!" );
        }
        return xReturn;
    }

    //--------------------------------------------------------------------
    void EFormsHelper::setListSourceBinding( const Reference< XListEntrySource >& _rxListSource ) SAL_THROW(())
    {
        try
        {
            Reference< XListEntrySink > xAsSink( m_xControlModel, UNO_QUERY );
            OSL_ENSURE( xAsSink.is(), "EFormsHelper::setListSourceBinding: you should have used isListEntrySink before!" );
            if ( xAsSink.is() )
                xAsSink->setListEntrySource( _rxListSource );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EFormsHelper::setListSourceBinding: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    void EFormsHelper::setBinding( const Reference< ::com::sun::star::beans::XPropertySet >& _rxBinding ) SAL_THROW(())
    {
        if ( !m_xBindableControl.is() )
            return;

        try
        {
            Reference< XPropertySet > xOldBinding( m_xBindableControl->getValueBinding(), UNO_QUERY );

            Reference< XValueBinding > xBinding( _rxBinding, UNO_QUERY );
            OSL_ENSURE( xBinding.is() || !_rxBinding.is(), "EFormsHelper::setBinding: invalid binding!" );

            m_xBindableControl->setValueBinding( xBinding );

            ::std::set< ::rtl::OUString > aSet;
            firePropertyChanges( xOldBinding, _rxBinding, aSet );
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EFormsHelper::setBinding: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    Reference< XPropertySet > EFormsHelper::createBindingForFormModel( const ::rtl::OUString& _rTargetModel ) const SAL_THROW(())
    {
        return implGetOrCreateBinding( _rTargetModel, ::rtl::OUString() );
    }

    //--------------------------------------------------------------------
    Reference< XPropertySet > EFormsHelper::getOrCreateBindingForModel( const ::rtl::OUString& _rTargetModel, const ::rtl::OUString& _rBindingName ) const SAL_THROW(())
    {
        OSL_ENSURE( _rBindingName.getLength(), "EFormsHelper::getOrCreateBindingForModel: invalid binding name!" );
        return implGetOrCreateBinding( _rTargetModel, _rBindingName );
    }

    //--------------------------------------------------------------------
    Reference< XPropertySet > EFormsHelper::implGetOrCreateBinding( const ::rtl::OUString& _rTargetModel, const ::rtl::OUString& _rBindingName ) const SAL_THROW(())
    {
        OSL_ENSURE( !( !_rTargetModel.getLength() && _rBindingName .getLength() ), "EFormsHelper::implGetOrCreateBinding: no model, but a binding name?" );

        Reference< XPropertySet > xBinding;
        try
        {
            ::rtl::OUString sTargetModel( _rTargetModel );
            // determine the model which the binding should belong to
            if ( !sTargetModel.getLength() )
            {
                ::std::vector< ::rtl::OUString > aModelNames;
                getFormModelNames( aModelNames );
                if ( !aModelNames.empty() )
                    sTargetModel = *aModelNames.begin();
                OSL_ENSURE( sTargetModel.getLength(), "EFormsHelper::implGetOrCreateBinding: unable to obtain a default model!" );
            }
            Reference< xforms::XModel > xModel( getFormModelByName( sTargetModel ) );
            Reference< XNameAccess > xBindingNames( xModel.is() ? xModel->getBindings() : Reference< XSet >(), UNO_QUERY );
            if ( xBindingNames.is() )
            {
                // get or create the binding instance
                if ( _rBindingName.getLength() )
                {
                    if ( xBindingNames->hasByName( _rBindingName ) )
                        OSL_VERIFY( xBindingNames->getByName( _rBindingName ) >>= xBinding );
                    else
                    {
                        xBinding = xModel->createBinding( );
                        if ( xBinding.is() )
                        {
                            xBinding->setPropertyValue( PROPERTY_BINDING_ID, makeAny( _rBindingName ) );
                            xModel->getBindings()->insert( makeAny( xBinding ) );
                        }
                    }
                }
                else
                {
                    xBinding = xModel->createBinding( );
                    if ( xBinding.is() )
                    {
                        // find a nice name for it
                        String sBaseName( ModuleRes( RID_STR_BINDING_UI_NAME ) );
                        sBaseName += String::CreateFromAscii( " " );
                        String sNewName;
                        sal_Int32 nNumber = 1;
                        do
                        {
                            sNewName = sBaseName + ::rtl::OUString::valueOf( nNumber++ );
                        }
                        while ( xBindingNames->hasByName( sNewName ) );
                        Reference< XNamed > xName( xBinding, UNO_QUERY_THROW );
                        xName->setName( sNewName );
                        // and insert into the model
                        xModel->getBindings()->insert( makeAny( xBinding ) );
                    }
                }
            }
        }
        catch( const Exception& e )
        {
#if OSL_DEBUG_LEVEL > 0
            ::rtl::OString sMessage( "EFormsHelper::createNewBinding: caught an exception:\n" );
            sMessage += ::rtl::OString( e.Message.getStr(), e.Message.getLength(), RTL_TEXTENCODING_ASCII_US );
            OSL_ENSURE( sal_False, sMessage.getStr() );
#endif
        }

        return xBinding;
    }

    //--------------------------------------------------------------------
    namespace
    {
        //................................................................
        struct PropertyLessByName : public ::std::binary_function< Property, Property, bool >
        {
            bool operator()( const Property& _rLHS, const Property& _rRHS )
            {
                return _rLHS.Name < _rRHS.Name;
            }
        };

        //................................................................
        typedef ::std::set< Property, PropertyLessByName > PropertyBag;

        //................................................................
        struct PropertyBagInserter : public ::std::unary_function< Property, void >
        {
        private:
            PropertyBag& m_rProperties;

        public:
            PropertyBagInserter( PropertyBag& rProperties ) : m_rProperties( rProperties ) { }

            void operator()( const Property& _rProp )
            {
                m_rProperties.insert( _rProp );
            }
        };

        //................................................................
        Reference< XPropertySetInfo > collectPropertiesGetInfo( const Reference< XPropertySet >& _rxProps, PropertyBag& _rBag )
        {
            Reference< XPropertySetInfo > xInfo;
            if ( _rxProps.is() )
                xInfo = _rxProps->getPropertySetInfo();
            if ( xInfo.is() )
            {
                Sequence< Property > aProperties = xInfo->getProperties();
                ::std::for_each( aProperties.getConstArray(), aProperties.getConstArray() + aProperties.getLength(),
                    PropertyBagInserter( _rBag )
                );
            }
            return xInfo;
        }
    }

    //--------------------------------------------------------------------
    ::rtl::OUString EFormsHelper::getModelElementUIName( const EFormsHelper::ModelElementType _eType, const Reference< XPropertySet >& _rxElement ) const SAL_THROW(())
    {
        ::rtl::OUString sUIName;
        try
        {
            // determine the model which the element belongs to
            Reference< xforms::XFormsUIHelper1 > xHelper;
            if ( _rxElement.is() )
                _rxElement->getPropertyValue( PROPERTY_MODEL ) >>= xHelper;
            OSL_ENSURE( xHelper.is(), "EFormsHelper::getModelElementUIName: invalid element or model!" );
            if ( xHelper.is() )
            {
                ::rtl::OUString sElementName = ( _eType == Submission ) ? xHelper->getSubmissionName( _rxElement, sal_True ) : xHelper->getBindingName( _rxElement, sal_True );
                Reference< xforms::XModel > xModel( xHelper, UNO_QUERY_THROW );
                sUIName = composeModelElementUIName( xModel->getID(), sElementName );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EFormsHelper::getModelElementUIName: caught an exception!" );
        }

        return sUIName;
    }

    //--------------------------------------------------------------------
    Reference< XPropertySet > EFormsHelper::getModelElementFromUIName( const EFormsHelper::ModelElementType _eType, const ::rtl::OUString& _rUIName ) const SAL_THROW(())
    {
        const MapStringToPropertySet& rMapUINameToElement( ( _eType == Submission ) ? m_aSubmissionUINames : m_aBindingUINames );
        MapStringToPropertySet::const_iterator pos = rMapUINameToElement.find( _rUIName );
        OSL_ENSURE( pos != rMapUINameToElement.end(), "EFormsHelper::getModelElementFromUIName: didn't find it!" );

        return ( pos != rMapUINameToElement.end() ) ? pos->second : Reference< XPropertySet >();
    }

    //--------------------------------------------------------------------
    void EFormsHelper::getAllElementUINames( const ModelElementType _eType, ::std::vector< ::rtl::OUString >& /* [out] */ _rElementNames, bool _bPrepentEmptyEntry )
    {
        MapStringToPropertySet& rMapUINameToElement( ( _eType == Submission ) ? m_aSubmissionUINames : m_aBindingUINames );
        rMapUINameToElement.clear();
        _rElementNames.resize( 0 );

        if ( _bPrepentEmptyEntry )
            rMapUINameToElement[ ::rtl::OUString() ] = Reference< XPropertySet >();

        try
        {
            // obtain the model names
            ::std::vector< ::rtl::OUString > aModels;
            getFormModelNames( aModels );
            _rElementNames.reserve( aModels.size() * 2 );    // heuristics

            // for every model, obtain the element
            for ( ::std::vector< ::rtl::OUString >::const_iterator pModelName = aModels.begin();
                  pModelName != aModels.end();
                  ++pModelName
                )
            {
                Reference< xforms::XModel > xModel = getFormModelByName( *pModelName );
                OSL_ENSURE( xModel.is(), "EFormsHelper::getAllElementUINames: inconsistency in the models!" );
                Reference< xforms::XFormsUIHelper1 > xHelper( xModel, UNO_QUERY );

                Reference< XIndexAccess > xElements;
                if ( xModel.is() )
                    xElements = xElements.query( ( _eType == Submission ) ? xModel->getSubmissions() : xModel->getBindings() );
                if ( !xElements.is() )
                    break;

                sal_Int32 nElementCount = xElements->getCount();
                for ( sal_Int32 i = 0; i < nElementCount; ++i )
                {
                    Reference< XPropertySet > xElement( xElements->getByIndex( i ), UNO_QUERY );
                    OSL_ENSURE( xElement.is(), "EFormsHelper::getAllElementUINames: empty element!" );
                    if ( !xElement.is() )
                        continue;
#if OSL_DEBUG_LEVEL > 0
                    {
                        Reference< xforms::XModel > xElementsModel;
                        xElement->getPropertyValue( PROPERTY_MODEL ) >>= xElementsModel;
                        OSL_ENSURE( xElementsModel == xModel, "EFormsHelper::getAllElementUINames: inconsistency in the model-element relationship!" );
                        if ( !( xElementsModel == xModel ) )
                            xElement->setPropertyValue( PROPERTY_MODEL, makeAny( xModel ) );
                    }
#endif
                    ::rtl::OUString sElementName = ( _eType == Submission ) ? xHelper->getSubmissionName( xElement, sal_True ) : xHelper->getBindingName( xElement, sal_True );
                    ::rtl::OUString sUIName = composeModelElementUIName( *pModelName, sElementName );

                    OSL_ENSURE( rMapUINameToElement.find( sUIName ) == rMapUINameToElement.end(), "EFormsHelper::getAllElementUINames: duplicate name!" );
                    rMapUINameToElement.insert( MapStringToPropertySet::value_type( sUIName, xElement ) );
                }
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EFormsHelper::getAllElementUINames: caught an exception!" );
        }

        _rElementNames.resize( rMapUINameToElement.size() );
        ::std::transform( rMapUINameToElement.begin(), rMapUINameToElement.end(), _rElementNames.begin(), ::std::select1st< MapStringToPropertySet::value_type >() );
    }

    //--------------------------------------------------------------------
    void EFormsHelper::firePropertyChanges( const Reference< XPropertySet >& _rxOldProps, const Reference< XPropertySet >& _rxNewProps, ::std::set< ::rtl::OUString >& _rFilter ) const
    {
        if ( !m_xBindingListener.is() )
            return;

        try
        {
            PropertyBag aProperties;
            Reference< XPropertySetInfo > xOldInfo = collectPropertiesGetInfo( _rxOldProps, aProperties );
            Reference< XPropertySetInfo > xNewInfo = collectPropertiesGetInfo( _rxNewProps, aProperties );

            PropertyChangeEvent aEvent;
            aEvent.Source = m_xBindableControl.get();

            for ( PropertyBag::const_iterator aProp = aProperties.begin();
                  aProp != aProperties.end();
                  ++aProp
                )
            {
                if ( _rFilter.find( aProp->Name ) != _rFilter.end() )
                    continue;

                Any aOldValue( NULL, aProp->Type );
                if ( xOldInfo.is() && xOldInfo->hasPropertyByName( aProp->Name ) )
                    aOldValue = _rxOldProps->getPropertyValue( aProp->Name );

                Any aNewValue( NULL, aProp->Type );
                if ( xNewInfo.is() && xNewInfo->hasPropertyByName( aProp->Name ) )
                    aNewValue = _rxNewProps->getPropertyValue( aProp->Name );

                if ( aOldValue != aNewValue )
                {
                    aEvent.PropertyName = aProp->Name;
                    aEvent.PropertyHandle = aProp->Handle;
                    aEvent.OldValue = aOldValue;
                    aEvent.NewValue = aNewValue;
                    m_xBindingListener->propertyChange( aEvent );
                }
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "EFormsHelper::firePropertyChanges: caught an exception!" );
        }
    }

//........................................................................
} // namespace pcr
//........................................................................

