/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: Edit.cxx,v $
 * $Revision: 1.40 $
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
#include "precompiled_forms.hxx"

#include "Edit.hxx"

#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/form/XSubmit.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>

#include <vcl/svapp.hxx>
#include <vcl/wintypes.hxx>

#include <connectivity/dbtools.hxx>
#include <connectivity/formattedcolumnvalue.hxx>
#include <connectivity/dbconversion.hxx>

#include <tools/diagnose_ex.h>
#include <tools/debug.hxx>

#include <comphelper/container.hxx>
#include <comphelper/numbers.hxx>

using namespace dbtools;

//.........................................................................
namespace frm
{
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::sdb;
using namespace ::com::sun::star::sdbc;
using namespace ::com::sun::star::sdbcx;
using namespace ::com::sun::star::beans;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::awt;
using namespace ::com::sun::star::io;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::util;
using namespace ::com::sun::star::form::binding;

//------------------------------------------------------------------
InterfaceRef SAL_CALL OEditControl_CreateInstance(const Reference< XMultiServiceFactory > & _rxFactory)
{
    return *(new OEditControl(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OEditControl::_getTypes()
{
    static Sequence<Type> aTypes;
    if (!aTypes.getLength())
    {
        // my two base classes
        aTypes = concatSequences(OBoundControl::_getTypes(), OEditControl_BASE::getTypes());
    }
    return aTypes;
}

//------------------------------------------------------------------------------
Any SAL_CALL OEditControl::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    Any aReturn = OBoundControl::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = OEditControl_BASE::queryInterface(_rType);

    return aReturn;
}

DBG_NAME(OEditControl);
//------------------------------------------------------------------------------
OEditControl::OEditControl(const Reference<XMultiServiceFactory>& _rxFactory)
               :OBoundControl( _rxFactory, FRM_SUN_CONTROL_RICHTEXTCONTROL )
               ,m_aChangeListeners(m_aMutex)
               ,m_nKeyEvent( 0 )
{
    DBG_CTOR(OEditControl,NULL);

    increment(m_refCount);
    {
        Reference<XWindow>  xComp;
        if (query_aggregation(m_xAggregate, xComp))
        {
            xComp->addFocusListener(this);
            xComp->addKeyListener(this);
        }
    }
    decrement(m_refCount);
}

//------------------------------------------------------------------------------
OEditControl::~OEditControl()
{
    if( m_nKeyEvent )
        Application::RemoveUserEvent( m_nKeyEvent );

    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

    DBG_DTOR(OEditControl,NULL);
}

// XChangeBroadcaster
//------------------------------------------------------------------------------
void OEditControl::addChangeListener(const Reference<XChangeListener>& l) throw ( ::com::sun::star::uno::RuntimeException)
{
    m_aChangeListeners.addInterface( l );
}

//------------------------------------------------------------------------------
void OEditControl::removeChangeListener(const Reference<XChangeListener>& l) throw ( ::com::sun::star::uno::RuntimeException)
{
    m_aChangeListeners.removeInterface( l );
}

// OComponentHelper
//------------------------------------------------------------------------------
void OEditControl::disposing()
{
    OBoundControl::disposing();

    EventObject aEvt(static_cast<XWeak*>(this));
    m_aChangeListeners.disposeAndClear(aEvt);
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence  OEditControl::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 1);

    ::rtl::OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-1] = FRM_SUN_CONTROL_TEXTFIELD;
    return aSupported;
}

// XEventListener
//------------------------------------------------------------------------------
void OEditControl::disposing(const EventObject& Source) throw( RuntimeException )
{
    OBoundControl::disposing(Source);
}

// XFocusListener
//------------------------------------------------------------------------------
void OEditControl::focusGained( const FocusEvent& /*e*/ ) throw ( ::com::sun::star::uno::RuntimeException)
{
    Reference<XPropertySet>  xSet(getModel(), UNO_QUERY);
    if (xSet.is())
        xSet->getPropertyValue( PROPERTY_TEXT ) >>= m_aHtmlChangeValue;
}

//------------------------------------------------------------------------------
void OEditControl::focusLost( const FocusEvent& /*e*/ ) throw ( ::com::sun::star::uno::RuntimeException)
{
    Reference<XPropertySet>  xSet(getModel(), UNO_QUERY);
    if (xSet.is())
    {
        ::rtl::OUString sNewHtmlChangeValue;
        xSet->getPropertyValue( PROPERTY_TEXT ) >>= sNewHtmlChangeValue;
        if( sNewHtmlChangeValue != m_aHtmlChangeValue )
        {
            EventObject aEvt( *this );
            m_aChangeListeners.notifyEach( &XChangeListener::changed, aEvt );
        }
    }
}

// XKeyListener
//------------------------------------------------------------------------------
void OEditControl::keyPressed(const ::com::sun::star::awt::KeyEvent& e) throw ( ::com::sun::star::uno::RuntimeException)
{
    if( e.KeyCode != KEY_RETURN || e.Modifiers != 0 )
        return;

    // Steht das Control in einem Formular mit einer Submit-URL?
    Reference<XPropertySet>  xSet(getModel(), UNO_QUERY);
    if( !xSet.is() )
        return;

    // nicht fuer multiline edits
    Any aTmp( xSet->getPropertyValue(PROPERTY_MULTILINE));
    if ((aTmp.getValueType().equals(::getBooleanCppuType())) && getBOOL(aTmp))
        return;

    Reference<XFormComponent>  xFComp(xSet, UNO_QUERY);
    InterfaceRef  xParent = xFComp->getParent();
    if( !xParent.is() )
        return;

    Reference<XPropertySet>  xFormSet(xParent, UNO_QUERY);
    if( !xFormSet.is() )
        return;

    aTmp = xFormSet->getPropertyValue( PROPERTY_TARGET_URL );
    if (!aTmp.getValueType().equals(::getCppuType((const ::rtl::OUString*)NULL)) ||
        !getString(aTmp).getLength() )
        return;

    Reference<XIndexAccess>  xElements(xParent, UNO_QUERY);
    sal_Int32 nCount = xElements->getCount();
    if( nCount > 1 )
    {
        Reference<XPropertySet>  xFCSet;
        for( sal_Int32 nIndex=0; nIndex < nCount; nIndex++ )
        {
            //  Any aElement(xElements->getByIndex(nIndex));
            xElements->getByIndex(nIndex) >>= xFCSet;
            OSL_ENSURE(xFCSet.is(),"OEditControl::keyPressed: No XPropertySet!");

            if (hasProperty(PROPERTY_CLASSID, xFCSet) &&
                getINT16(xFCSet->getPropertyValue(PROPERTY_CLASSID)) == FormComponentType::TEXTFIELD)
            {
                // Noch ein weiteres Edit gefunden ==> dann nicht submitten
                if (xFCSet != xSet)
                    return;
            }
        }
    }

    // Da wir noch im Haender stehen, submit asynchron ausloesen
    if( m_nKeyEvent )
        Application::RemoveUserEvent( m_nKeyEvent );
    m_nKeyEvent = Application::PostUserEvent( LINK(this, OEditControl,OnKeyPressed) );
}

//------------------------------------------------------------------------------
void OEditControl::keyReleased(const ::com::sun::star::awt::KeyEvent& /*e*/) throw ( ::com::sun::star::uno::RuntimeException)
{
}

//------------------------------------------------------------------------------
IMPL_LINK(OEditControl, OnKeyPressed, void*, /*EMPTYARG*/)
{
    m_nKeyEvent = 0;

    Reference<XFormComponent>  xFComp(getModel(), UNO_QUERY);
    InterfaceRef  xParent = xFComp->getParent();
    Reference<XSubmit>  xSubmit(xParent, UNO_QUERY);
    if (xSubmit.is())
        xSubmit->submit( Reference<XControl>(), ::com::sun::star::awt::MouseEvent() );
    return 0L;
}

//------------------------------------------------------------------
void SAL_CALL OEditControl::createPeer( const Reference< XToolkit>& _rxToolkit, const Reference< XWindowPeer>& _rxParent ) throw ( RuntimeException )
{
    OBoundControl::createPeer(_rxToolkit, _rxParent);

    Reference< XVclWindowPeer >  xVclWindowPeer( getPeer(), UNO_QUERY );
    if ( xVclWindowPeer.is() )
        xVclWindowPeer->setProperty( ::rtl::OUString::createFromAscii("JavaCompatibleTextNotifications"), ::cppu::bool2any( sal_False ) );
}

/*************************************************************************/
//------------------------------------------------------------------
InterfaceRef SAL_CALL OEditModel_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OEditModel(_rxFactory));
}

//------------------------------------------------------------------------------
Sequence<Type> OEditModel::_getTypes()
{
    return OEditBaseModel::_getTypes();
}


DBG_NAME(OEditModel);
//------------------------------------------------------------------
OEditModel::OEditModel(const Reference<XMultiServiceFactory>& _rxFactory)
    :OEditBaseModel( _rxFactory, FRM_SUN_COMPONENT_RICHTEXTCONTROL, FRM_SUN_CONTROL_TEXTFIELD, sal_True, sal_True )
    ,m_bMaxTextLenModified(sal_False)
    ,m_bWritingFormattedFake(sal_False)
{
    DBG_CTOR(OEditModel,NULL);

    m_nClassId = FormComponentType::TEXTFIELD;
    initValueProperty( PROPERTY_TEXT, PROPERTY_ID_TEXT );
}

//------------------------------------------------------------------
OEditModel::OEditModel( const OEditModel* _pOriginal, const Reference<XMultiServiceFactory>& _rxFactory )
    :OEditBaseModel( _pOriginal, _rxFactory )
    ,m_bMaxTextLenModified(sal_False)
    ,m_bWritingFormattedFake(sal_False)
{
    DBG_CTOR( OEditModel, NULL );

    // Note that most of the properties are not clone from the original object:
    // Things as the format key, it's type, and such, depend on the field being part of a loaded form
    // (they're initialized in onConnectedDbColumn). Even if the original object _is_ part of such a form, we ourself
    // certainly aren't, so these members are defaulted. If we're inserted into a form which is already loaded,
    // they will be set to new values, anyway ....
}

//------------------------------------------------------------------
OEditModel::~OEditModel()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

    DBG_DTOR(OEditModel,NULL);
}

//------------------------------------------------------------------------------
IMPLEMENT_DEFAULT_CLONING( OEditModel )

//------------------------------------------------------------------------------
void OEditModel::disposing()
{
    OEditBaseModel::disposing();
    m_pValueFormatter.reset();
}

// XPersistObject
//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OEditModel::getServiceName() throw ( ::com::sun::star::uno::RuntimeException)
{
    return FRM_COMPONENT_EDIT;  // old (non-sun) name for compatibility !
}

// XServiceInfo
//------------------------------------------------------------------------------
StringSequence SAL_CALL OEditModel::getSupportedServiceNames() throw()
{
    StringSequence aSupported = OBoundControlModel::getSupportedServiceNames();

    sal_Int32 nOldLen = aSupported.getLength();
    aSupported.realloc( nOldLen + 8 );
    ::rtl::OUString* pStoreTo = aSupported.getArray() + nOldLen;

    *pStoreTo++ = BINDABLE_CONTROL_MODEL;
    *pStoreTo++ = DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_CONTROL_MODEL;

    *pStoreTo++ = BINDABLE_DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_BINDABLE_CONTROL_MODEL;

    *pStoreTo++ = FRM_SUN_COMPONENT_TEXTFIELD;
    *pStoreTo++ = FRM_SUN_COMPONENT_DATABASE_TEXTFIELD;
    *pStoreTo++ = BINDABLE_DATABASE_TEXT_FIELD;

    return aSupported;
}

// XPropertySet
void SAL_CALL OEditModel::getFastPropertyValue(Any& rValue, sal_Int32 nHandle ) const
{
    if ( PROPERTY_ID_PERSISTENCE_MAXTEXTLENGTH == nHandle )
    {
        if ( m_bMaxTextLenModified )
            rValue <<= sal_Int16(0);
        else if ( m_xAggregateSet.is() )
            rValue = m_xAggregateSet->getPropertyValue(PROPERTY_MAXTEXTLEN);
    }
    else
    {
        OEditBaseModel::getFastPropertyValue(rValue, nHandle );
    }
}

//------------------------------------------------------------------------------
void OEditModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 5, OEditBaseModel )
        DECL_PROP2(PERSISTENCE_MAXTEXTLENGTH,sal_Int16,         READONLY, TRANSIENT);
        DECL_PROP2(DEFAULT_TEXT,        ::rtl::OUString,        BOUND, MAYBEDEFAULT);
        DECL_BOOL_PROP1(EMPTY_IS_NULL,                          BOUND);
        DECL_PROP1(TABINDEX,            sal_Int16,              BOUND);
        DECL_BOOL_PROP2(FILTERPROPOSAL,                         BOUND, MAYBEDEFAULT);
    END_DESCRIBE_PROPERTIES();
}

//------------------------------------------------------------------------------
void OEditModel::describeAggregateProperties( Sequence< Property >& _rAggregateProps ) const
{
    OEditBaseModel::describeAggregateProperties( _rAggregateProps );

    // our aggregate is a rich text model, which also derives from OControlModel, as
    // do we, so we need to remove some duplicate properties
    RemoveProperty( _rAggregateProps, PROPERTY_TABINDEX );
    RemoveProperty( _rAggregateProps, PROPERTY_CLASSID );
    RemoveProperty( _rAggregateProps, PROPERTY_NAME );
    RemoveProperty( _rAggregateProps, PROPERTY_TAG );
    RemoveProperty( _rAggregateProps, PROPERTY_NATIVE_LOOK );

}

//------------------------------------------------------------------------------
bool OEditModel::implActsAsRichText( ) const
{
    sal_Bool bActAsRichText = sal_False;
    if ( m_xAggregateSet.is() )
    {
        OSL_VERIFY( m_xAggregateSet->getPropertyValue( PROPERTY_RICH_TEXT ) >>= bActAsRichText );
    }
    return bActAsRichText;
}

//------------------------------------------------------------------------------
void SAL_CALL OEditModel::reset(  ) throw(RuntimeException)
{
    // no reset if we currently act as rich text control
    if ( implActsAsRichText() )
        return;

    OEditBaseModel::reset();
}

//------------------------------------------------------------------------------
namespace
{
    void lcl_transferProperties( const Reference< XPropertySet >& _rxSource, const Reference< XPropertySet >& _rxDest )
    {
        try
        {
            Reference< XPropertySetInfo > xSourceInfo;
            if ( _rxSource.is() )
                xSourceInfo = _rxSource->getPropertySetInfo();

            Reference< XPropertySetInfo > xDestInfo;
            if ( _rxDest.is() )
                xDestInfo = _rxDest->getPropertySetInfo();

            if ( !xSourceInfo.is() || !xDestInfo.is() )
            {
                OSL_ENSURE( sal_False, "lcl_transferProperties: invalid property set(s)!" );
                return;
            }

            Sequence< Property > aSourceProps( xSourceInfo->getProperties() );
            const Property* pSourceProps = aSourceProps.getConstArray();
            const Property* pSourcePropsEnd = aSourceProps.getConstArray() + aSourceProps.getLength();
            while ( pSourceProps != pSourcePropsEnd )
            {
                if ( !xDestInfo->hasPropertyByName( pSourceProps->Name ) )
                {
                    ++pSourceProps;
                    continue;
                }

                Property aDestProp( xDestInfo->getPropertyByName( pSourceProps->Name ) );
                if ( 0 != ( aDestProp.Attributes & PropertyAttribute::READONLY ) )
                {
                    ++pSourceProps;
                    continue;
                }

                try
                {
                    _rxDest->setPropertyValue( pSourceProps->Name, _rxSource->getPropertyValue( pSourceProps->Name ) );
                }
                catch( IllegalArgumentException e )
                {
#if OSL_DEBUG_LEVEL > 0
                    ::rtl::OString sMessage( "could not transfer the property named '" );
                    sMessage += ::rtl::OString( pSourceProps->Name.getStr(), pSourceProps->Name.getLength(), RTL_TEXTENCODING_ASCII_US );
                    sMessage += ::rtl::OString( "'." );
                    if ( e.Message.getLength() )
                    {
                        sMessage += ::rtl::OString( "\n\nMessage:\n" );
                        sMessage += ::rtl::OString( e.Message.getStr(), e.Message.getLength(), RTL_TEXTENCODING_ASCII_US );
                    }
                    OSL_ENSURE( sal_False, sMessage.getStr() );
#endif
                }

                ++pSourceProps;
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION();
        }
    }
}

//------------------------------------------------------------------------------
void OEditModel::writeAggregate( const Reference< XObjectOutputStream >& _rxOutStream ) const
{
    // we need to fake the writing of our aggregate. Since #i24387#, we have another aggregate,
    // but for compatibility, we need to use an "old" aggregate for writing and reading

    Reference< XPropertySet > xFakedAggregate(
        getORB()->createInstance( VCL_CONTROLMODEL_EDIT ), UNO_QUERY
    );
    OSL_ENSURE( xFakedAggregate.is(), "OEditModel::writeAggregate: could not create an old EditControlModel!" );
    if ( !xFakedAggregate.is() )
        return;

    lcl_transferProperties( m_xAggregateSet, xFakedAggregate );

    Reference< XPersistObject > xFakedPersist( xFakedAggregate, UNO_QUERY );
    OSL_ENSURE( xFakedPersist.is(), "OEditModel::writeAggregate: no XPersistObject!" );
    if ( xFakedPersist.is() )
        xFakedPersist->write( _rxOutStream );
}

//------------------------------------------------------------------------------
void OEditModel::readAggregate( const Reference< XObjectInputStream >& _rxInStream )
{
    // we need to fake the reading of our aggregate. Since #i24387#, we have another aggregate,
    // but for compatibility, we need to use an "old" aggregate for writing and reading

    Reference< XPropertySet > xFakedAggregate(
        getORB()->createInstance( VCL_CONTROLMODEL_EDIT ), UNO_QUERY
    );
    Reference< XPersistObject > xFakedPersist( xFakedAggregate, UNO_QUERY );
    OSL_ENSURE( xFakedPersist.is(), "OEditModel::readAggregate: no XPersistObject, or no faked aggregate at all!" );
    if ( xFakedPersist.is() )
    {
        xFakedPersist->read( _rxInStream );
        lcl_transferProperties( xFakedAggregate, m_xAggregateSet );
    }
}

//------------------------------------------------------------------------------
void OEditModel::write(const Reference<XObjectOutputStream>& _rxOutStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    Any aCurrentText;
    sal_Int16 nOldTextLen = 0;
    // bin ich gerade loaded und habe dazu zeitweilig die MaxTextLen umgesetzt ?
    if ( m_bMaxTextLenModified )
    {   // -> fuer die Dauer des Speicherns meinem aggregierten Model die alte TextLen einreden

        // before doing this we have to save the current text value of the aggregate, as this may be affected by resetting the text len
        // FS - 08.12.99 - 70606
        aCurrentText = m_xAggregateSet->getPropertyValue(PROPERTY_TEXT);

        m_xAggregateSet->getPropertyValue(PROPERTY_MAXTEXTLEN) >>= nOldTextLen;
        m_xAggregateSet->setPropertyValue(PROPERTY_MAXTEXTLEN, makeAny((sal_Int16)0));
    }

    OEditBaseModel::write(_rxOutStream);

    if ( m_bMaxTextLenModified )
    {   // wieder zuruecksetzen
        m_xAggregateSet->setPropertyValue(PROPERTY_MAXTEXTLEN, makeAny(nOldTextLen));
        // and reset the text
        // First we set it to an empty string : Without this the second setPropertyValue would not do anything as it thinks
        // we aren't changing the prop (it didn't notify the - implicite - change of the text prop while setting the max text len)
        // This seems to be a bug with in toolkit's EditControl-implementation.
        m_xAggregateSet->setPropertyValue(PROPERTY_TEXT, makeAny(::rtl::OUString()));
        m_xAggregateSet->setPropertyValue(PROPERTY_TEXT, aCurrentText);
    }
}

//------------------------------------------------------------------------------
void OEditModel::read(const Reference<XObjectInputStream>& _rxInStream) throw ( ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException)
{
    OEditBaseModel::read(_rxInStream);

    // Some versions (5.1 'til about 552) wrote a wrong DefaultControl-property value which is unknown
    // to older versions (5.0).
    // correct this ...
    if (m_xAggregateSet.is())
    {
        Any aDefaultControl = m_xAggregateSet->getPropertyValue(PROPERTY_DEFAULTCONTROL);
        if  (   (aDefaultControl.getValueType().getTypeClass() == TypeClass_STRING)
            &&  (getString(aDefaultControl).compareTo(STARDIV_ONE_FORM_CONTROL_TEXTFIELD) == COMPARE_EQUAL)
            )
        {
            m_xAggregateSet->setPropertyValue( PROPERTY_DEFAULTCONTROL, makeAny( (::rtl::OUString)STARDIV_ONE_FORM_CONTROL_EDIT ) );
            // Older as well as current versions should understand this : the former knew only the STARDIV_ONE_FORM_CONTROL_EDIT,
            // the latter are registered for both STARDIV_ONE_FORM_CONTROL_EDIT and STARDIV_ONE_FORM_CONTROL_TEXTFIELD.
        }
    }
}

//------------------------------------------------------------------------------
sal_uInt16 OEditModel::getPersistenceFlags() const
{
    sal_uInt16 nFlags = OEditBaseModel::getPersistenceFlags();

    if (m_bWritingFormattedFake)
        nFlags |= PF_FAKE_FORMATTED_FIELD;

    return nFlags;
}

//------------------------------------------------------------------------------
void OEditModel::onConnectedDbColumn( const Reference< XInterface >& _rxForm )
{
    Reference< XPropertySet > xField = getField();
    if ( xField.is() )
    {
        m_pValueFormatter.reset( new ::dbtools::FormattedColumnValue( getContext(), Reference< XRowSet >( _rxForm, UNO_QUERY ), xField ) );

        if ( m_pValueFormatter->getKeyType() != NumberFormat::SCIENTIFIC )
        {
            m_bMaxTextLenModified = getINT16(m_xAggregateSet->getPropertyValue(PROPERTY_MAXTEXTLEN)) != 0;
            if ( !m_bMaxTextLenModified )
            {
                sal_Int32 nFieldLen = 0;
                xField->getPropertyValue(::rtl::OUString::createFromAscii("Precision")) >>= nFieldLen;

                if (nFieldLen && nFieldLen <= USHRT_MAX)
                {
                    Any aVal;
                    aVal <<= (sal_Int16)nFieldLen;
                    m_xAggregateSet->setPropertyValue(PROPERTY_MAXTEXTLEN, aVal);

                    m_bMaxTextLenModified = sal_True;
                }
            }
            else
                m_bMaxTextLenModified = sal_False; // to get sure that the text len won't be set in unloaded
        }
    }
}

//------------------------------------------------------------------------------
void OEditModel::onDisconnectedDbColumn()
{
    OEditBaseModel::onDisconnectedDbColumn();

    m_pValueFormatter.reset();

    if ( hasField() && m_bMaxTextLenModified )
    {
        Any aVal;
        aVal <<= (sal_Int16)0;  // nur wenn es 0 war, habe ich es in onConnectedDbColumn umgesetzt
        m_xAggregateSet->setPropertyValue(PROPERTY_MAXTEXTLEN, aVal);
        m_bMaxTextLenModified = sal_False;
    }
}

//------------------------------------------------------------------------------
sal_Bool OEditModel::approveDbColumnType( sal_Int32 _nColumnType )
{
    // if we act as rich text curently, we do not allow binding to a database column
    if ( implActsAsRichText() )
        return sal_False;

    return OEditBaseModel::approveDbColumnType( _nColumnType );
}

//------------------------------------------------------------------------------
sal_Bool OEditModel::approveValueBinding( const Reference< XValueBinding >& _rxBinding )
{
    OSL_PRECOND( _rxBinding.is(), "OEditModel::approveValueBinding: invalid binding!" );

    // only strings are accepted for simplicity
    return  _rxBinding.is()
        &&  _rxBinding->supportsType( ::getCppuType( static_cast< ::rtl::OUString* >( NULL ) ) );
}

//------------------------------------------------------------------------------
sal_Bool OEditModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
{
    ::rtl::OUString sNewValue;
    m_xAggregateFastSet->getFastPropertyValue( getValuePropertyAggHandle() ) >>= sNewValue;
    if (sNewValue != m_aSaveValue)
    {
        if (!sNewValue.getLength() && !isRequired() && m_bEmptyIsNull)
            m_xColumnUpdate->updateNull();
        else
        {
            OSL_PRECOND( m_pValueFormatter.get(), "OEditModel::commitControlValueToDbColumn: no value formatter!" );
            try
            {
                if ( m_pValueFormatter.get() )
                {
                    if ( !m_pValueFormatter->setFormattedValue( sNewValue ) )
                        return sal_False;
                }
                else
                    m_xColumnUpdate->updateString( sNewValue );
            }
            catch ( const Exception& )
            {
                return sal_False;
            }
        }
        m_aSaveValue = sNewValue;
    }
    return sal_True;
}

//------------------------------------------------------------------------------
Any OEditModel::translateDbColumnToControlValue()
{
    OSL_PRECOND( m_pValueFormatter.get(), "OEditModel::translateDbColumnToControlValue: no value formatter!" );
    if ( m_pValueFormatter.get() )
        m_aSaveValue = m_pValueFormatter->getFormattedValue();
    else
        m_aSaveValue = ::rtl::OUString();

    // #i2817# OJ
    sal_uInt16 nMaxTextLen = getINT16( m_xAggregateSet->getPropertyValue( PROPERTY_MAXTEXTLEN ) );
    if ( nMaxTextLen && m_aSaveValue.getLength() > nMaxTextLen )
    {
        sal_Int32 nDiff = m_aSaveValue.getLength() - nMaxTextLen;
        m_aSaveValue = m_aSaveValue.replaceAt( nMaxTextLen, nDiff, ::rtl::OUString() );
    }

    return makeAny( m_aSaveValue );
}

//------------------------------------------------------------------------------
Any OEditModel::getDefaultForReset() const
{
    return makeAny( m_aDefaultText );
}

//.........................................................................
}
//.........................................................................

