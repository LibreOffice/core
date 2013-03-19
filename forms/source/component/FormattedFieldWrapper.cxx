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

#include "FormattedFieldWrapper.hxx"
#include "Edit.hxx"
#include "FormattedField.hxx"
#include <tools/debug.hxx>
#include "EditBase.hxx"
#include "services.hxx"
#include <connectivity/dbtools.hxx>
#include <vcl/svapp.hxx>

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

//==================================================================
// OFormattedFieldWrapper
//==================================================================
DBG_NAME(OFormattedFieldWrapper)

InterfaceRef SAL_CALL OFormattedFieldWrapper_CreateInstance_ForceFormatted(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return OFormattedFieldWrapper::createFormattedFieldWrapper(_rxFactory, true);
}

InterfaceRef SAL_CALL OFormattedFieldWrapper_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return OFormattedFieldWrapper::createFormattedFieldWrapper(_rxFactory, false);
}

OFormattedFieldWrapper::OFormattedFieldWrapper(const Reference<XMultiServiceFactory>& _rxFactory)
    :m_xServiceFactory(_rxFactory)
{
    DBG_CTOR(OFormattedFieldWrapper, NULL);
}

InterfaceRef OFormattedFieldWrapper::createFormattedFieldWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::lang::XMultiServiceFactory>& _rxFactory, bool bActAsFormatted)
{
    OFormattedFieldWrapper *pRef = new OFormattedFieldWrapper(_rxFactory);

    if (bActAsFormatted)
    {
        // instantiate an FormattedModel
        InterfaceRef  xFormattedModel;
        // (instantiate it directly ..., as the OFormattedModel isn't
        // registered for any service names anymore)
        OFormattedModel* pModel = new OFormattedModel(pRef->m_xServiceFactory);
        query_interface(static_cast<XWeak*>(pModel), xFormattedModel);

        pRef->m_xAggregate = Reference<XAggregation> (xFormattedModel, UNO_QUERY);
        OSL_ENSURE(pRef->m_xAggregate.is(), "the OFormattedModel didn't have an XAggregation interface !");

        // _before_ setting the delegator, give it to the member references
        query_interface(xFormattedModel, pRef->m_xFormattedPart);
        pRef->m_pEditPart = rtl::Reference< OEditModel >(new OEditModel(pRef->m_xServiceFactory));
    }

    increment(pRef->m_refCount);

    if (pRef->m_xAggregate.is())
    {   // has to be in it's own block because of the temporary variable created by *this
        pRef->m_xAggregate->setDelegator(static_cast<XWeak*>(pRef));
    }

    InterfaceRef xRef(*pRef);
    decrement(pRef->m_refCount);

    return xRef;
}

Reference< XCloneable > SAL_CALL OFormattedFieldWrapper::createClone() throw (RuntimeException)
{
    ensureAggregate();

    rtl::Reference< OFormattedFieldWrapper > xRef(new OFormattedFieldWrapper(m_xServiceFactory));

    Reference< XCloneable > xCloneAccess;
    query_aggregation( m_xAggregate, xCloneAccess );

    // clone the aggregate
    if ( xCloneAccess.is() )
    {
        Reference< XCloneable > xClone = xCloneAccess->createClone();
        xRef->m_xAggregate = Reference< XAggregation >(xClone, UNO_QUERY);
        OSL_ENSURE(xRef->m_xAggregate.is(), "invalid aggregate cloned !");

        query_interface( Reference< XInterface >(xClone.get() ), xRef->m_xFormattedPart);

        if ( m_pEditPart.is() )
        {
            xRef->m_pEditPart = rtl::Reference< OEditModel >( new OEditModel(m_pEditPart.get(), m_xServiceFactory));
        }
    }
    else
    {   // the clone source does not yet have an aggregate -> we don't yet need one, too
    }

    if ( xRef->m_xAggregate.is() )
    {   // has to be in it's own block because of the temporary variable created by *this
        xRef->m_xAggregate->setDelegator(static_cast< XWeak* >(xRef.get()));
    }

    return xRef.get();
}

OFormattedFieldWrapper::~OFormattedFieldWrapper()
{
    // release the aggregated object (if any)
    if (m_xAggregate.is())
        m_xAggregate->setDelegator(InterfaceRef ());

    DBG_DTOR(OFormattedFieldWrapper, NULL);
}

//------------------------------------------------------------------
Any SAL_CALL OFormattedFieldWrapper::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    Any aReturn;

    if (_rType.equals( ::getCppuType( static_cast< Reference< XTypeProvider >* >(NULL) ) ) )
    {   // a XTypeProvider interface needs a working aggregate - we don't want to give the type provider
        // of our base class (OFormattedFieldWrapper_Base) to the caller as it supplies nearly nothing
        ensureAggregate();
        if (m_xAggregate.is())
            aReturn = m_xAggregate->queryAggregation(_rType);
    }

    if (!aReturn.hasValue())
    {
        aReturn = OFormattedFieldWrapper_Base::queryAggregation(_rType);

        if ((_rType.equals( ::getCppuType( static_cast< Reference< XServiceInfo >* >(NULL) ) ) ) && aReturn.hasValue())
        {   // somebody requested an XServiceInfo interface and our base class provided it
            // check our aggregate if it has one, too
            ensureAggregate();
        }

        if (!aReturn.hasValue())
        {
            aReturn = ::cppu::queryInterface( _rType,
                static_cast< XPersistObject* >( this ),
                static_cast< XCloneable* >( this )
            );

            if (!aReturn.hasValue())
            {
                // somebody requests an interface other than the basics (XInterface) and other than
                // the two we can supply without an aggregate. So ensure
                // the aggregate exists.
                ensureAggregate();
                if (m_xAggregate.is())
                    aReturn = m_xAggregate->queryAggregation(_rType);
            }
        }
    }

    return aReturn;
}

//------------------------------------------------------------------
::rtl::OUString SAL_CALL OFormattedFieldWrapper::getServiceName() throw(RuntimeException)
{
    // return the old compatibility name for an EditModel
    return FRM_COMPONENT_EDIT;
}

//------------------------------------------------------------------
::rtl::OUString SAL_CALL OFormattedFieldWrapper::getImplementationName(  ) throw (RuntimeException)
{
    return ::rtl::OUString("com.sun.star.comp.forms.OFormattedFieldWrapper");
}

//------------------------------------------------------------------
sal_Bool SAL_CALL OFormattedFieldWrapper::supportsService( const ::rtl::OUString& _rServiceName ) throw (RuntimeException)
{
    DBG_ASSERT(m_xAggregate.is(), "OFormattedFieldWrapper::supportsService: should never have made it 'til here without an aggregate!");
    Reference< XServiceInfo > xSI;
    m_xAggregate->queryAggregation(::getCppuType(static_cast< Reference< XServiceInfo >* >(NULL))) >>= xSI;
    return xSI->supportsService(_rServiceName);
}

//------------------------------------------------------------------
Sequence< ::rtl::OUString > SAL_CALL OFormattedFieldWrapper::getSupportedServiceNames(  ) throw (RuntimeException)
{
    DBG_ASSERT(m_xAggregate.is(), "OFormattedFieldWrapper::getSupportedServiceNames: should never have made it 'til here without an aggregate!");
    Reference< XServiceInfo > xSI;
    m_xAggregate->queryAggregation(::getCppuType(static_cast< Reference< XServiceInfo >* >(NULL))) >>= xSI;
    return xSI->getSupportedServiceNames();
}

//------------------------------------------------------------------
void SAL_CALL OFormattedFieldWrapper::write(const Reference<XObjectOutputStream>& _rxOutStream) throw( IOException, RuntimeException )
{
    // can't write myself
    ensureAggregate();

    // if we act as real edit field, we can simple forward this write request
    if (!m_xFormattedPart.is())
    {
        Reference<XPersistObject>  xAggregatePersistence;
        query_aggregation(m_xAggregate, xAggregatePersistence);
        DBG_ASSERT(xAggregatePersistence.is(), "OFormattedFieldWrapper::write : don't know how to handle this : can't write !");
            // oops ... We gave an XPersistObject interface to the caller but now we aren't an XPersistObject ...
        if (xAggregatePersistence.is())
            xAggregatePersistence->write(_rxOutStream);
        return;
    }

    // else we have to write an edit part first
    OSL_ENSURE(m_pEditPart.is(), "OFormattedFieldWrapper::write : formatted part without edit part ?");
    if ( !m_pEditPart.is() )
        throw RuntimeException( ::rtl::OUString(), *this );

    // for this we transfer the current props of the formatted part to the edit part
    Reference<XPropertySet>  xFormatProps(m_xFormattedPart, UNO_QUERY);
    Reference<XPropertySet>  xEditProps;
    query_interface(static_cast<XWeak*>(m_pEditPart.get()), xEditProps);

    Locale aAppLanguage = Application::GetSettings().GetUILanguageTag().getLocale();
    dbtools::TransferFormComponentProperties(xFormatProps, xEditProps, aAppLanguage);

    // then write the edit part, after switching to "fake mode"
    m_pEditPart->enableFormattedWriteFake();
    m_pEditPart->write(_rxOutStream);
    m_pEditPart->disableFormattedWriteFake();

    // and finally write the formatted part we're really interested in
    m_xFormattedPart->write(_rxOutStream);
}

//------------------------------------------------------------------
void SAL_CALL OFormattedFieldWrapper::read(const Reference<XObjectInputStream>& _rxInStream) throw( IOException, RuntimeException )
{
    if (m_xAggregate.is())
    {   //  we alread did a decision if we're an EditModel or a FormattedModel

        // if we act as formatted, we have to read the edit part first
        if (m_xFormattedPart.is())
        {
            // two possible cases:
            // a) the stuff was written by a version which didn't work with an Edit header (all intermediate
            //      versions >5.1 && <=568)
            // b) it was written by a version using edit headers
            // as we can distinguish a) from b) only after we have read the edit part, we need to remember the
            // position
            Reference<XMarkableStream>  xInMarkable(_rxInStream, UNO_QUERY);
            DBG_ASSERT(xInMarkable.is(), "OFormattedFieldWrapper::read : can only work with markable streams !");
            sal_Int32 nBeforeEditPart = xInMarkable->createMark();

            m_pEditPart->read(_rxInStream);
            // this only works because an edit model can read the stuff written by a formatted model
            // (maybe with some assertions) , but not vice versa
            if (!m_pEditPart->lastReadWasFormattedFake())
            {   // case a), written with a version without the edit part fake, so seek to the start position, again
                xInMarkable->jumpToMark(nBeforeEditPart);
            }
            xInMarkable->deleteMark(nBeforeEditPart);
        }

        Reference<XPersistObject>  xAggregatePersistence;
        query_aggregation(m_xAggregate, xAggregatePersistence);
        DBG_ASSERT(xAggregatePersistence.is(), "OFormattedFieldWrapper::read : don't know how to handle this : can't read !");
            // oops ... We gave an XPersistObject interface to the caller but now we aren't an XPersistObject ...

        if (xAggregatePersistence.is())
            xAggregatePersistence->read(_rxInStream);
        return;
    }

    // we have to decide from the data within the stream whether we should
    // be an EditModel or a FormattedModel

    {
        // let an OEditModel do the reading
        rtl::Reference< OEditModel > pBasicReader(new OEditModel(m_xServiceFactory));
        pBasicReader->read(_rxInStream);

        // was it really an edit model ?
        if (!pBasicReader->lastReadWasFormattedFake())
        {
            // yes -> all fine
            m_xAggregate = Reference< XAggregation >( pBasicReader.get() );
        }
        else
        {   // no -> substitute it with a formatted model
            // let the formmatted model do the reading
            m_xFormattedPart = Reference< XPersistObject >(new OFormattedModel(m_xServiceFactory));
            m_xFormattedPart->read(_rxInStream);
            m_pEditPart = pBasicReader;
            m_xAggregate = Reference< XAggregation >( m_xFormattedPart, UNO_QUERY );
        }
    }

    // do the aggregation
    increment(m_refCount);
    if (m_xAggregate.is())
    {   // has to be in it's own block because of the temporary variable created by *this
        m_xAggregate->setDelegator(static_cast<XWeak*>(this));
    }
    decrement(m_refCount);
}

//------------------------------------------------------------------
void OFormattedFieldWrapper::ensureAggregate()
{
    if (m_xAggregate.is())
        return;

    {
        // instantiate an EditModel (the only place where we are allowed to decide that we're an FormattedModel
        // is in ::read)
        InterfaceRef  xEditModel = m_xServiceFactory->createInstance(FRM_SUN_COMPONENT_TEXTFIELD);
        if (!xEditModel.is())
        {
            // arghhh ... instantiate it directly ... it's dirty, but we really need this aggregate
            OEditModel* pModel = new OEditModel(m_xServiceFactory);
            query_interface(static_cast<XWeak*>(pModel), xEditModel);
        }

        m_xAggregate = Reference<XAggregation> (xEditModel, UNO_QUERY);
        DBG_ASSERT(m_xAggregate.is(), "OFormattedFieldWrapper::ensureAggregate : the OEditModel didn't have an XAggregation interface !");

        {
            Reference< XServiceInfo > xSI(m_xAggregate, UNO_QUERY);
            if (!xSI.is())
            {
                OSL_FAIL("OFormattedFieldWrapper::ensureAggregate: the aggregate has no XServiceInfo!");
                m_xAggregate.clear();
            }
        }
    }

    increment(m_refCount);
    if (m_xAggregate.is())
    {   // has to be in it's own block because of the temporary variable created by *this
        m_xAggregate->setDelegator(static_cast<XWeak*>(this));
    }
    decrement(m_refCount);
}

//.........................................................................
}
//.........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
