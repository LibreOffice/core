/*************************************************************************
 *
 *  $RCSfile: submissionhandler.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 12:13:18 $
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

#ifndef EXTENSIONS_SOURCE_PROPCTRLR_SUBMISSIONHANDLER_HXX
#include "submissionhandler.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_FORMMETADATA_HXX_
#include "formmetadata.hxx"
#endif
#ifndef _EXTENSIONS_FORMSCTRLR_FORMSTRINGS_HXX_
#include "formstrings.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_LINEDESCRIPTOR_HXX_
#include "linedescriptor.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_STRINGREPRESENTATION_HXX
#include "stringrepresentation.hxx"
#endif
#ifndef EXTENSIONS_SOURCE_PROPCTRLR_PROPBROWSERUI_HXX
#include "propbrowserui.hxx"
#endif

/** === begin UNO includes === **/
#ifndef _COM_SUN_STAR_FORM_FORMBUTTONTYPE_HPP_
#include <com/sun/star/form/FormButtonType.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XNAMED_HPP_
#include <com/sun/star/container/XNamed.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_FORM_SUBMISSION_XSUBMISSIONSUPPLIER_HPP_
#include <com/sun/star/form/submission/XSubmissionSupplier.hpp>
#endif
/** === end UNO includes === **/

#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif
#ifndef _RTL_USTRBUF_HXX_
#include <rtl/ustrbuf.hxx>
#endif

//........................................................................
namespace pcr
{
//........................................................................

    using namespace ::comphelper;
    using namespace ::com::sun::star;
    using namespace ::com::sun::star::uno;
    using namespace ::com::sun::star::lang;
    using namespace ::com::sun::star::beans;
    using namespace ::com::sun::star::script;
    using namespace ::com::sun::star::form;
    using namespace ::com::sun::star::xforms;
    using namespace ::com::sun::star::container;

    //====================================================================
    //= SubmissionHelper
    //====================================================================
    //--------------------------------------------------------------------
    SubmissionHelper::SubmissionHelper( const Reference< XPropertySet >& _rxIntrospectee, const Reference< frame::XModel >& _rxContextDocument )
        :EFormsHelper( _rxIntrospectee, _rxContextDocument )
    {
        OSL_ENSURE( canTriggerSubmissions( _rxIntrospectee, _rxContextDocument ),
            "SubmissionHelper::SubmissionPropertyHandler: you should not have instantiated me!" );
    }

    //--------------------------------------------------------------------
    bool SubmissionHelper::canTriggerSubmissions( const Reference< XPropertySet >& _rxControlModel,
        const Reference< frame::XModel >& _rxContextDocument ) SAL_THROW(())
    {
        if ( !EFormsHelper::isEForm( _rxContextDocument ) )
            return false;

        try
        {
            Reference< submission::XSubmissionSupplier > xSubmissionSupp( _rxControlModel, UNO_QUERY );
            if ( xSubmissionSupp.is() )
                return true;
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "SubmissionHelper::canTriggerSubmissions: caught an exception!" );
        }
        return false;
    }

    //====================================================================
    //= SubmissionPropertyHandler
    //====================================================================
    //--------------------------------------------------------------------
    SubmissionPropertyHandler::SubmissionPropertyHandler( const Reference< XPropertySet >& _rxIntrospectee,
        const Reference< frame::XModel >& _rxContextDocument, const Reference< XTypeConverter >& _rxTypeConverter  )
        :PropertyHandler       ( _rxIntrospectee, _rxTypeConverter )
        ,OPropertyChangeListener( m_aMutex                          )
    {
        if ( SubmissionHelper::canTriggerSubmissions( m_xIntrospectee, _rxContextDocument ) )
        {
            m_pHelper.reset( new SubmissionHelper( m_xIntrospectee, _rxContextDocument ) );

            OPropertyChangeMultiplexer* pMultiplexer = new OPropertyChangeMultiplexer( this, _rxIntrospectee );
            pMultiplexer->addProperty( PROPERTY_BUTTONTYPE );
                // this does not delete the object, since our base class holds it alive
        }
    }

    //--------------------------------------------------------------------
    SubmissionPropertyHandler::~SubmissionPropertyHandler( )
    {
        disposeAdapter();
    }

    //--------------------------------------------------------------------
    Any SAL_CALL SubmissionPropertyHandler::getPropertyValue( PropertyId _nPropId, bool _bLazy ) const
    {
        Any aReturn;

        OSL_ENSURE( m_pHelper.get(), "SubmissionPropertyHandler::getPropertyValue: we have no SupportedProperties!" );
        if ( !m_pHelper.get() )
            return aReturn;

        try
        {
            switch ( _nPropId )
            {
            case PROPERTY_ID_SUBMISSION_ID:
            {
                Reference< submission::XSubmissionSupplier > xSubmissionSupp( m_xIntrospectee, UNO_QUERY );
                OSL_ENSURE( xSubmissionSupp.is(), "SubmissionPropertyHandler::getPropertyValue: this should never happen ..." );
                    // this handler is not intended for components which are no XSubmissionSupplier
                Reference< submission::XSubmission > xSubmission;
                if ( xSubmissionSupp.is() )
                    xSubmission = xSubmissionSupp->getSubmission( );
                aReturn <<= xSubmission;
            }
            break;

            case PROPERTY_ID_XFORMS_BUTTONTYPE:
            {
                FormButtonType eType = FormButtonType_PUSH;
                OSL_VERIFY( m_xIntrospectee->getPropertyValue( PROPERTY_BUTTONTYPE ) >>= eType );
                if ( ( eType != FormButtonType_PUSH ) && ( eType != FormButtonType_SUBMIT ) )
                    eType = FormButtonType_PUSH;
                aReturn <<= eType;
            }
            break;

            default:
                DBG_ERROR( "SubmissionPropertyHandler::getPropertyValue: cannot handle this property!" );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "SubmissionPropertyHandler::getPropertyValue: caught an exception!" );
        }

        return aReturn;
    }

    //--------------------------------------------------------------------
    void SAL_CALL SubmissionPropertyHandler::setPropertyValue( PropertyId _nPropId, const Any& _rValue )
    {
        OSL_ENSURE( m_pHelper.get(), "SubmissionPropertyHandler::setPropertyValue: we have no SupportedProperties!" );
        if ( !m_pHelper.get() )
            return;

        try
        {
            switch ( _nPropId )
            {
            case PROPERTY_ID_SUBMISSION_ID:
            {
                Reference< submission::XSubmission > xSubmission;
                OSL_VERIFY( _rValue >>= xSubmission );

                Reference< submission::XSubmissionSupplier > xSubmissionSupp( m_xIntrospectee, UNO_QUERY );
                OSL_ENSURE( xSubmissionSupp.is(), "SubmissionPropertyHandler::setPropertyValue: this should never happen ..." );
                    // this handler is not intended for components which are no XSubmissionSupplier
                if ( xSubmissionSupp.is() )
                    xSubmissionSupp->setSubmission( xSubmission );
            }
            break;

            case PROPERTY_ID_XFORMS_BUTTONTYPE:
                m_xIntrospectee->setPropertyValue( PROPERTY_BUTTONTYPE, _rValue );
                break;

            default:
                OSL_ENSURE( sal_False, "SubmissionPropertyHandler::setPropertyValue: cannot handle this id!" );
            }
        }
        catch( const Exception& )
        {
            OSL_ENSURE( sal_False, "SubmissionPropertyHandler::setPropertyValue: caught an exception!" );
        }
    }

    //--------------------------------------------------------------------
    ::std::vector< ::rtl::OUString > SAL_CALL SubmissionPropertyHandler::getActuatingProperties( ) const
    {
        if ( !m_pHelper.get() )
            return ::std::vector< ::rtl::OUString >();

        ::std::vector< ::rtl::OUString > aReturn( 1 );
        aReturn[ 0 ] = PROPERTY_XFORMS_BUTTONTYPE;
        return aReturn;
    }

    //--------------------------------------------------------------------
    ::std::vector< ::rtl::OUString > SAL_CALL SubmissionPropertyHandler::getSupersededProperties( ) const
    {
        if ( !m_pHelper.get() )
            return ::std::vector< ::rtl::OUString >();

        ::std::vector< ::rtl::OUString > aReturn( 3 );
        aReturn[ 0 ] = PROPERTY_TARGET_URL;
        aReturn[ 1 ] = PROPERTY_TARGET_FRAME;
        aReturn[ 2 ] = PROPERTY_BUTTONTYPE;
        return aReturn;
    }

    //--------------------------------------------------------------------
    ::std::vector< Property > SAL_CALL SubmissionPropertyHandler::implDescribeSupportedProperties() const
    {
        ::std::vector< Property > aProperties;
        if ( m_pHelper.get() )
        {
            implAddPropertyDescription( aProperties, PROPERTY_SUBMISSION_ID, ::getCppuType( static_cast< Reference< submission::XSubmission > * >( NULL ) ) );
            implAddPropertyDescription( aProperties, PROPERTY_XFORMS_BUTTONTYPE, ::getCppuType( static_cast< FormButtonType* >( NULL ) ) );
        }
        return aProperties;
    }

    //--------------------------------------------------------------------
    bool SAL_CALL SubmissionPropertyHandler::supportsUIDescriptor( PropertyId _nPropId ) const
    {
        return true;
    }

    //--------------------------------------------------------------------
    void SAL_CALL SubmissionPropertyHandler::describePropertyUI( PropertyId _nPropId, PropertyUIDescriptor& /* [out] */ _rDescriptor ) const
    {
        OSL_ENSURE( m_pHelper.get(), "SubmissionPropertyHandler::describePropertyUI: we have no SupportedProperties!" );
        if ( !m_pHelper.get() )
            return;

        switch ( _nPropId )
        {
        case PROPERTY_ID_SUBMISSION_ID:
            _rDescriptor.eControlType = BCT_LISTBOX;
            const_cast< SubmissionHelper* >( m_pHelper.get() )->getAllElementUINames( EFormsHelper::Submission, _rDescriptor.aListValues, false );
            break;

        case PROPERTY_ID_XFORMS_BUTTONTYPE:
        {
            _rDescriptor.eControlType = BCT_LISTBOX;
            // available options are nearly the same as for the "normal" button type, but only the
            // first two options
            ::std::vector< String > aAllValues( m_pInfoService->getPropertyEnumRepresentations( PROPERTY_ID_BUTTONTYPE ) );
            _rDescriptor.aListValues.resize( 2 );
            _rDescriptor.aListValues[ 0 ] = aAllValues[ 0 ];
            _rDescriptor.aListValues[ 1 ] = aAllValues[ 1 ];
        }
        break;

        default:
            OSL_ENSURE( sal_False, "SubmissionPropertyHandler::describePropertyUI: cannot handle this id!" );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL SubmissionPropertyHandler::updateDependentProperties( PropertyId _nActuatingPropId, const Any& _rNewValue, const Any& _rOldValue, IPropertyBrowserUI* _pUpdater )
    {
        OSL_ENSURE( m_pHelper.get(), "SubmissionPropertyHandler::updateDependentProperties: we have no ActuatingProperties!" );
        if ( !m_pHelper.get() )
            return;

        switch ( _nActuatingPropId )
        {
        case PROPERTY_ID_XFORMS_BUTTONTYPE:
        {
            FormButtonType eType = FormButtonType_PUSH;
            OSL_VERIFY( _rNewValue >>= eType );
            _pUpdater->enablePropertyUI( PROPERTY_SUBMISSION_ID, eType == FormButtonType_SUBMIT );
        }
        break;

        default:
            OSL_ENSURE( sal_False, "SubmissionPropertyHandler::updateDependentProperties: cannot handle this id!" );
        }
    }

    //--------------------------------------------------------------------
    void SAL_CALL SubmissionPropertyHandler::initializePropertyUI( PropertyId _nPropId, IPropertyBrowserUI* _pUpdater )
    {
        OSL_ENSURE( m_pHelper.get(), "SubmissionPropertyHandler::initializePropertyUI: we have no SupportedProperties!" );
        // nothing to do
    }

    //--------------------------------------------------------------------
    Any SAL_CALL SubmissionPropertyHandler::getPropertyValueFromStringRep( PropertyId _nPropId, const ::rtl::OUString& _rStringRep ) const
    {
        Any aReturn;

        OSL_ENSURE( m_pHelper.get(), "SubmissionPropertyHandler::getPropertyValueFromStringRep: we have no SupportedProperties!" );
        if ( !m_pHelper.get() )
            return aReturn;

        switch ( _nPropId )
        {
        case PROPERTY_ID_SUBMISSION_ID:
        {
            Reference< XSubmission > xSubmission( m_pHelper->getModelElementFromUIName( EFormsHelper::Submission, _rStringRep ), UNO_QUERY );
            aReturn <<= xSubmission;
        }
        break;

        case PROPERTY_ID_XFORMS_BUTTONTYPE:
        {
            StringRepresentation aConversionHelper( m_xTypeConverter );
            aReturn = aConversionHelper.getPropertyValueFromStringRep( _rStringRep, ::getCppuType( static_cast< FormButtonType* >( NULL ) ), PROPERTY_ID_BUTTONTYPE, m_pInfoService.get() );
        }
        break;

        default:
            OSL_ENSURE( sal_False, "SubmissionPropertyHandler::getPropertyValueFromStringRep: cannot handle this id!" );
        }

        return aReturn;
    }

    //--------------------------------------------------------------------
    ::rtl::OUString SAL_CALL SubmissionPropertyHandler::getStringRepFromPropertyValue( PropertyId _nPropId, const Any& _rValue ) const
    {
        ::rtl::OUString sReturn;

        OSL_ENSURE( m_pHelper.get(), "SubmissionPropertyHandler::getStringRepFromPropertyValue: we have no SupportedProperties!" );
        if ( !m_pHelper.get() )
            return sReturn;

        switch ( _nPropId )
        {
        case PROPERTY_ID_SUBMISSION_ID:
        {
            Reference< XPropertySet > xSubmission( _rValue, UNO_QUERY );
            if ( xSubmission.is() )
                sReturn = m_pHelper->getModelElementUIName( EFormsHelper::Submission, xSubmission );
        }
        break;

        case PROPERTY_ID_XFORMS_BUTTONTYPE:
        {
            StringRepresentation aConversionHelper( m_xTypeConverter );
            sReturn = aConversionHelper.getStringRepFromPropertyValue( _rValue, PROPERTY_ID_BUTTONTYPE, m_pInfoService.get() );
        }
        break;

        default:
            OSL_ENSURE( sal_False, "SubmissionPropertyHandler::getStringRepFromPropertyValue: cannot handle this id!" );
        }

        return sReturn;
    }

    //--------------------------------------------------------------------
    void SubmissionPropertyHandler::_propertyChanged( const PropertyChangeEvent& _rEvent ) throw(RuntimeException)
    {
        if ( _rEvent.PropertyName == PROPERTY_BUTTONTYPE )
            firePropertyChange( PROPERTY_XFORMS_BUTTONTYPE, PROPERTY_ID_XFORMS_BUTTONTYPE, _rEvent.OldValue, _rEvent.NewValue );
    }

//........................................................................
} // namespace pcr
//........................................................................

