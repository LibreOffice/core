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


#include "Edit.hxx"

#include <com/sun/star/uno/Type.hxx>
#include <com/sun/star/awt/XWindow.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/form/XSubmit.hpp>
#include <com/sun/star/util/NumberFormat.hpp>
#include <com/sun/star/sdbc/DataType.hpp>
#include <com/sun/star/awt/XVclWindowPeer.hpp>

#include <vcl/svapp.hxx>
#include <vcl/keycodes.hxx>
#include <tools/wintypes.hxx>

#include <connectivity/dbtools.hxx>
#include <connectivity/formattedcolumnvalue.hxx>
#include <connectivity/dbconversion.hxx>

#include <comphelper/property.hxx>
#include <tools/diagnose_ex.h>

using namespace dbtools;

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


Any SAL_CALL OEditControl::queryAggregation(const Type& _rType)
{
    Any aReturn = OBoundControl::queryAggregation(_rType);
    if (!aReturn.hasValue())
        aReturn = OEditControl_BASE::queryInterface(_rType);

    return aReturn;
}


OEditControl::OEditControl(const Reference<XComponentContext>& _rxFactory)
               :OBoundControl( _rxFactory, FRM_SUN_CONTROL_RICHTEXTCONTROL )
               ,m_aChangeListeners(m_aMutex)
               ,m_nKeyEvent( nullptr )
{

    osl_atomic_increment(&m_refCount);
    {
        Reference<XWindow>  xComp;
        if (query_aggregation(m_xAggregate, xComp))
        {
            xComp->addFocusListener(this);
            xComp->addKeyListener(this);
        }
    }
    osl_atomic_decrement(&m_refCount);
}


OEditControl::~OEditControl()
{
    if( m_nKeyEvent )
        Application::RemoveUserEvent( m_nKeyEvent );

    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

}

// XChangeBroadcaster

void OEditControl::addChangeListener(const Reference<XChangeListener>& l)
{
    m_aChangeListeners.addInterface( l );
}


void OEditControl::removeChangeListener(const Reference<XChangeListener>& l)
{
    m_aChangeListeners.removeInterface( l );
}

// OComponentHelper

void OEditControl::disposing()
{
    OBoundControl::disposing();

    EventObject aEvt(static_cast<XWeak*>(this));
    m_aChangeListeners.disposeAndClear(aEvt);
}

// XServiceInfo

css::uno::Sequence<OUString>  OEditControl::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aSupported = OBoundControl::getSupportedServiceNames();
    aSupported.realloc(aSupported.getLength() + 3);

    OUString*pArray = aSupported.getArray();
    pArray[aSupported.getLength()-3] = FRM_SUN_CONTROL_TEXTFIELD;
    pArray[aSupported.getLength()-2] = STARDIV_ONE_FORM_CONTROL_EDIT;
    pArray[aSupported.getLength()-1] = STARDIV_ONE_FORM_CONTROL_TEXTFIELD;
    return aSupported;
}

// XEventListener

void OEditControl::disposing(const EventObject& Source)
{
    OBoundControl::disposing(Source);
}

// XFocusListener

void OEditControl::focusGained( const FocusEvent& /*e*/ )
{
    Reference<XPropertySet>  xSet(getModel(), UNO_QUERY);
    if (xSet.is())
        xSet->getPropertyValue( PROPERTY_TEXT ) >>= m_aHtmlChangeValue;
}


void OEditControl::focusLost( const FocusEvent& /*e*/ )
{
    Reference<XPropertySet>  xSet(getModel(), UNO_QUERY);
    if (xSet.is())
    {
        OUString sNewHtmlChangeValue;
        xSet->getPropertyValue( PROPERTY_TEXT ) >>= sNewHtmlChangeValue;
        if( sNewHtmlChangeValue != m_aHtmlChangeValue )
        {
            EventObject aEvt( *this );
            m_aChangeListeners.notifyEach( &XChangeListener::changed, aEvt );
        }
    }
}

// XKeyListener

void OEditControl::keyPressed(const css::awt::KeyEvent& e)
{
    if( e.KeyCode != KEY_RETURN || e.Modifiers != 0 )
        return;

    // Is the Control in a form with a submit URL?
    Reference<XPropertySet>  xSet(getModel(), UNO_QUERY);
    if( !xSet.is() )
        return;

    // Not for multiline edits
    Any aTmp( xSet->getPropertyValue(PROPERTY_MULTILINE));
    if ((aTmp.getValueType().equals(cppu::UnoType<bool>::get())) && getBOOL(aTmp))
        return;

    Reference<XFormComponent>  xFComp(xSet, UNO_QUERY);
    css::uno::Reference<css::uno::XInterface>  xParent = xFComp->getParent();
    if( !xParent.is() )
        return;

    Reference<XPropertySet>  xFormSet(xParent, UNO_QUERY);
    if( !xFormSet.is() )
        return;

    aTmp = xFormSet->getPropertyValue( PROPERTY_TARGET_URL );
    if (!aTmp.getValueType().equals(cppu::UnoType<OUString>::get()) ||
        getString(aTmp).isEmpty() )
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
                // Found another Edit -> then do not submit!
                if (xFCSet != xSet)
                    return;
            }
        }
    }

    // Because we're still in the header, trigger submit asynchronously
    if( m_nKeyEvent )
        Application::RemoveUserEvent( m_nKeyEvent );
    m_nKeyEvent = Application::PostUserEvent( LINK(this, OEditControl, OnKeyPressed) );
}


void OEditControl::keyReleased(const css::awt::KeyEvent& /*e*/)
{
}


IMPL_LINK_NOARG(OEditControl, OnKeyPressed, void*, void)
{
    m_nKeyEvent = nullptr;

    Reference<XFormComponent>  xFComp(getModel(), UNO_QUERY);
    css::uno::Reference<css::uno::XInterface>  xParent = xFComp->getParent();
    Reference<XSubmit>  xSubmit(xParent, UNO_QUERY);
    if (xSubmit.is())
        xSubmit->submit( Reference<XControl>(), css::awt::MouseEvent() );
}



OEditModel::OEditModel(const Reference<XComponentContext>& _rxFactory)
    :OEditBaseModel( _rxFactory, FRM_SUN_COMPONENT_RICHTEXTCONTROL, FRM_SUN_CONTROL_TEXTFIELD, true, true )
    ,m_bMaxTextLenModified(false)
    ,m_bWritingFormattedFake(false)
{

    m_nClassId = FormComponentType::TEXTFIELD;
    initValueProperty( PROPERTY_TEXT, PROPERTY_ID_TEXT );
}


OEditModel::OEditModel( const OEditModel* _pOriginal, const Reference<XComponentContext>& _rxFactory )
    :OEditBaseModel( _pOriginal, _rxFactory )
    ,m_bMaxTextLenModified(false)
    ,m_bWritingFormattedFake(false)
{

    // Note that most of the properties are not clone from the original object:
    // Things as the format key, it's type, and such, depend on the field being part of a loaded form
    // (they're initialized in onConnectedDbColumn). Even if the original object _is_ part of such a form, we ourself
    // certainly aren't, so these members are defaulted. If we're inserted into a form which is already loaded,
    // they will be set to new values, anyway ....
}


OEditModel::~OEditModel()
{
    if (!OComponentHelper::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }

}


IMPLEMENT_DEFAULT_CLONING( OEditModel )


void OEditModel::disposing()
{
    OEditBaseModel::disposing();
    m_pValueFormatter.reset();
}

// XPersistObject

OUString SAL_CALL OEditModel::getServiceName()
{
    return OUString(FRM_COMPONENT_EDIT);  // old (non-sun) name for compatibility !
}

// XServiceInfo

css::uno::Sequence<OUString> SAL_CALL OEditModel::getSupportedServiceNames()
{
    css::uno::Sequence<OUString> aSupported = OBoundControlModel::getSupportedServiceNames();

    sal_Int32 nOldLen = aSupported.getLength();
    aSupported.realloc( nOldLen + 9 );
    OUString* pStoreTo = aSupported.getArray() + nOldLen;

    *pStoreTo++ = BINDABLE_CONTROL_MODEL;
    *pStoreTo++ = DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_CONTROL_MODEL;

    *pStoreTo++ = BINDABLE_DATA_AWARE_CONTROL_MODEL;
    *pStoreTo++ = VALIDATABLE_BINDABLE_CONTROL_MODEL;

    *pStoreTo++ = FRM_SUN_COMPONENT_TEXTFIELD;
    *pStoreTo++ = FRM_SUN_COMPONENT_DATABASE_TEXTFIELD;
    *pStoreTo++ = BINDABLE_DATABASE_TEXT_FIELD;

    *pStoreTo++ = FRM_COMPONENT_TEXTFIELD;

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


void OEditModel::describeFixedProperties( Sequence< Property >& _rProps ) const
{
    BEGIN_DESCRIBE_PROPERTIES( 5, OEditBaseModel )
        DECL_PROP2(PERSISTENCE_MAXTEXTLENGTH,sal_Int16,         READONLY, TRANSIENT);
        DECL_PROP2(DEFAULT_TEXT,        OUString,        BOUND, MAYBEDEFAULT);
        DECL_BOOL_PROP1(EMPTY_IS_NULL,                          BOUND);
        DECL_PROP1(TABINDEX,            sal_Int16,              BOUND);
        DECL_BOOL_PROP2(FILTERPROPOSAL,                         BOUND, MAYBEDEFAULT);
    END_DESCRIBE_PROPERTIES();
}


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


bool OEditModel::implActsAsRichText( ) const
{
    bool bActAsRichText = false;
    if ( m_xAggregateSet.is() )
    {
        OSL_VERIFY( m_xAggregateSet->getPropertyValue( PROPERTY_RICH_TEXT ) >>= bActAsRichText );
    }
    return bActAsRichText;
}


void SAL_CALL OEditModel::reset(  )
{
    // no reset if we currently act as rich text control
    if ( implActsAsRichText() )
        return;

    OEditBaseModel::reset();
}


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
                OSL_FAIL( "lcl_transferProperties: invalid property set(s)!" );
                return;
            }

            Sequence< Property > aSourceProps( xSourceInfo->getProperties() );
            for ( auto const & sourceprop : aSourceProps )
            {
                if ( !xDestInfo->hasPropertyByName( sourceprop.Name ) )
                {
                    continue;
                }

                Property aDestProp( xDestInfo->getPropertyByName( sourceprop.Name ) );
                if ( 0 != ( aDestProp.Attributes & PropertyAttribute::READONLY ) )
                {
                    continue;
                }

                try
                {
                    _rxDest->setPropertyValue( sourceprop.Name, _rxSource->getPropertyValue( sourceprop.Name ) );
                }
                catch(const IllegalArgumentException& e)
                {
                    SAL_WARN( "forms.component", "could not transfer the property named '"
                                << sourceprop.Name
                                << "'. " << e );
                }
            }
        }
        catch( const Exception& )
        {
            DBG_UNHANDLED_EXCEPTION("forms.component");
        }
    }
}


void OEditModel::writeAggregate( const Reference< XObjectOutputStream >& _rxOutStream ) const
{
    // we need to fake the writing of our aggregate. Since #i24387#, we have another aggregate,
    // but for compatibility, we need to use an "old" aggregate for writing and reading

    Reference< XPropertySet > xFakedAggregate(
        getContext()->getServiceManager()->createInstanceWithContext( VCL_CONTROLMODEL_EDIT, getContext() ),
        UNO_QUERY
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


void OEditModel::readAggregate( const Reference< XObjectInputStream >& _rxInStream )
{
    // we need to fake the reading of our aggregate. Since #i24387#, we have another aggregate,
    // but for compatibility, we need to use an "old" aggregate for writing and reading

    Reference< XPropertySet > xFakedAggregate(
        getContext()->getServiceManager()->createInstanceWithContext( VCL_CONTROLMODEL_EDIT, getContext() ),
        UNO_QUERY
    );
    Reference< XPersistObject > xFakedPersist( xFakedAggregate, UNO_QUERY );
    OSL_ENSURE( xFakedPersist.is(), "OEditModel::readAggregate: no XPersistObject, or no faked aggregate at all!" );
    if ( xFakedPersist.is() )
    {
        xFakedPersist->read( _rxInStream );
        lcl_transferProperties( xFakedAggregate, m_xAggregateSet );
    }
}


void OEditModel::write(const Reference<XObjectOutputStream>& _rxOutStream)
{
    Any aCurrentText;
    sal_Int16 nOldTextLen = 0;
    // Am I loaded at the moment and did I switch MaxTextLen temporarily?
    if ( m_bMaxTextLenModified )
    {   // -> for the duration of saving, make my aggregated model believe the old TextLen

        // before doing this we have to save the current text value of the aggregate, as this may be affected by resetting the text len
        aCurrentText = m_xAggregateSet->getPropertyValue(PROPERTY_TEXT);

        m_xAggregateSet->getPropertyValue(PROPERTY_MAXTEXTLEN) >>= nOldTextLen;
        m_xAggregateSet->setPropertyValue(PROPERTY_MAXTEXTLEN, makeAny(sal_Int16(0)));
    }

    OEditBaseModel::write(_rxOutStream);

    if ( m_bMaxTextLenModified )
    {   // Reset again
        m_xAggregateSet->setPropertyValue(PROPERTY_MAXTEXTLEN, makeAny(nOldTextLen));
        // and reset the text
        // First we set it to an empty string : Without this the second setPropertyValue would not do anything as it thinks
        // we aren't changing the prop (it didn't notify the - implicit - change of the text prop while setting the max text len)
        // This seems to be a bug with in toolkit's EditControl-implementation.
        m_xAggregateSet->setPropertyValue(PROPERTY_TEXT, makeAny(OUString()));
        m_xAggregateSet->setPropertyValue(PROPERTY_TEXT, aCurrentText);
    }
}


void OEditModel::read(const Reference<XObjectInputStream>& _rxInStream)
{
    OEditBaseModel::read(_rxInStream);

    // Some versions (5.1 'til about 552) wrote a wrong DefaultControl-property value which is unknown
    // to older versions (5.0).
    // correct this ...
    if (m_xAggregateSet.is())
    {
        Any aDefaultControl = m_xAggregateSet->getPropertyValue(PROPERTY_DEFAULTCONTROL);
        if  (   (aDefaultControl.getValueType().getTypeClass() == TypeClass_STRING)
            &&  (getString(aDefaultControl) == STARDIV_ONE_FORM_CONTROL_TEXTFIELD )
            )
        {
            m_xAggregateSet->setPropertyValue( PROPERTY_DEFAULTCONTROL, makeAny( OUString(STARDIV_ONE_FORM_CONTROL_EDIT) ) );
            // Older as well as current versions should understand this : the former knew only the STARDIV_ONE_FORM_CONTROL_EDIT,
            // the latter are registered for both STARDIV_ONE_FORM_CONTROL_EDIT and STARDIV_ONE_FORM_CONTROL_TEXTFIELD.
        }
    }
}


sal_uInt16 OEditModel::getPersistenceFlags() const
{
    sal_uInt16 nFlags = OEditBaseModel::getPersistenceFlags();

    if (m_bWritingFormattedFake)
        nFlags |= PF_FAKE_FORMATTED_FIELD;

    return nFlags;
}


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
                xField->getPropertyValue("Precision") >>= nFieldLen;

                if (nFieldLen > 0 && nFieldLen <= SAL_MAX_INT16)
                {
                    Any aVal;
                    aVal <<= static_cast<sal_Int16>(nFieldLen);
                    m_xAggregateSet->setPropertyValue(PROPERTY_MAXTEXTLEN, aVal);

                    m_bMaxTextLenModified = true;
                }
            }
            else
                m_bMaxTextLenModified = false; // to get sure that the text len won't be set in unloaded
        }
    }
}


void OEditModel::onDisconnectedDbColumn()
{
    OEditBaseModel::onDisconnectedDbColumn();

    m_pValueFormatter.reset();

    if ( hasField() && m_bMaxTextLenModified )
    {
        Any aVal;
        aVal <<= sal_Int16(0);  // Only if it was 0, I switched it in onConnectedDbColumn
        m_xAggregateSet->setPropertyValue(PROPERTY_MAXTEXTLEN, aVal);
        m_bMaxTextLenModified = false;
    }
}


bool OEditModel::approveDbColumnType( sal_Int32 _nColumnType )
{
    // if we act as rich text currently, we do not allow binding to a database column
    if ( implActsAsRichText() )
        return false;

    return OEditBaseModel::approveDbColumnType( _nColumnType );
}


bool OEditModel::commitControlValueToDbColumn( bool /*_bPostReset*/ )
{
    Any aNewValue( m_xAggregateFastSet->getFastPropertyValue( getValuePropertyAggHandle() ) );

    OUString sNewValue;
    aNewValue >>= sNewValue;

    if  (   !aNewValue.hasValue()
        ||  (   sNewValue.isEmpty()         // an empty string
            &&  m_bEmptyIsNull              // which should be interpreted as NULL
            )
        )
    {
        m_xColumnUpdate->updateNull();
    }
    else
    {
        OSL_PRECOND( m_pValueFormatter.get(), "OEditModel::commitControlValueToDbColumn: no value formatter!" );
        try
        {
            if ( m_pValueFormatter.get() )
            {
                if ( !m_pValueFormatter->setFormattedValue( sNewValue ) )
                    return false;
            }
            else
                m_xColumnUpdate->updateString( sNewValue );
        }
        catch ( const Exception& )
        {
            return false;
        }
    }

    return true;
}


Any OEditModel::translateDbColumnToControlValue()
{
    OSL_PRECOND( m_pValueFormatter.get(), "OEditModel::translateDbColumnToControlValue: no value formatter!" );
    Any aRet;
    if ( m_pValueFormatter.get() )
    {
        OUString sValue( m_pValueFormatter->getFormattedValue() );
        if  (   sValue.isEmpty()
            &&  m_pValueFormatter->getColumn().is()
            &&  m_pValueFormatter->getColumn()->wasNull()
            )
        {
        }
        else
        {
            // #i2817# OJ
            sal_uInt16 nMaxTextLen = getINT16( m_xAggregateSet->getPropertyValue( PROPERTY_MAXTEXTLEN ) );
            if ( nMaxTextLen && sValue.getLength() > nMaxTextLen )
            {
                sal_Int32 nDiff = sValue.getLength() - nMaxTextLen;
                sValue = sValue.replaceAt( nMaxTextLen, nDiff, OUString() );
            }

            aRet <<= sValue;
        }
    }

    return aRet.hasValue() ? aRet : makeAny( OUString() );
}


Any OEditModel::getDefaultForReset() const
{
    return makeAny( m_aDefaultText );
}

}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_OEditModel_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OEditModel(component));
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_form_OEditControl_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    return cppu::acquire(new frm::OEditControl(component));
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
