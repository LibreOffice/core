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
#include "EditBase.hxx"
#include "services.hxx"
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <cppuhelper/queryinterface.hxx>
#include <connectivity/dbtools.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

using namespace frm;
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

OFormattedFieldWrapper::OFormattedFieldWrapper(const Reference<XComponentContext>& _rxFactory)
    :m_xContext(_rxFactory)
{
}

css::uno::Reference<css::uno::XInterface> OFormattedFieldWrapper::createFormattedFieldWrapper(const css::uno::Reference< css::uno::XComponentContext>& _rxFactory, bool bActAsFormatted)
{
    OFormattedFieldWrapper *pRef = new OFormattedFieldWrapper(_rxFactory);

    if (bActAsFormatted)
    {
        // instantiate an FormattedModel
        // (instantiate it directly ..., as the OFormattedModel isn't
        // registered for any service names anymore)
        OFormattedModel* pModel = new OFormattedModel(pRef->m_xContext);
        css::uno::Reference<css::uno::XInterface> xFormattedModel(
            static_cast<XWeak*>(pModel), css::uno::UNO_QUERY);

        pRef->m_xAggregate.set(xFormattedModel, UNO_QUERY);
        OSL_ENSURE(pRef->m_xAggregate.is(), "the OFormattedModel didn't have an XAggregation interface !");

        // _before_ setting the delegator, give it to the member references
        pRef->m_xFormattedPart.set(xFormattedModel, css::uno::UNO_QUERY);
        pRef->m_pEditPart.set(new OEditModel(pRef->m_xContext));
    }

    osl_atomic_increment(&pRef->m_refCount);

    if (pRef->m_xAggregate.is())
    {   // has to be in it's own block because of the temporary variable created by *this
        pRef->m_xAggregate->setDelegator(static_cast<XWeak*>(pRef));
    }

    css::uno::Reference<css::uno::XInterface> xRef(*pRef);
    osl_atomic_decrement(&pRef->m_refCount);

    return xRef;
}

Reference< XCloneable > SAL_CALL OFormattedFieldWrapper::createClone() throw (RuntimeException, std::exception)
{
    ensureAggregate();

    rtl::Reference< OFormattedFieldWrapper > xRef(new OFormattedFieldWrapper(m_xContext));

    Reference< XCloneable > xCloneAccess;
    query_aggregation( m_xAggregate, xCloneAccess );

    // clone the aggregate
    if ( xCloneAccess.is() )
    {
        Reference< XCloneable > xClone = xCloneAccess->createClone();
        xRef->m_xAggregate.set(xClone, UNO_QUERY);
        OSL_ENSURE(xRef->m_xAggregate.is(), "invalid aggregate cloned !");

        xRef->m_xFormattedPart.set(
            Reference< XInterface >(xClone.get()), css::uno::UNO_QUERY);

        if ( m_pEditPart.is() )
        {
            xRef->m_pEditPart.set( new OEditModel(m_pEditPart.get(), m_xContext) );
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
        m_xAggregate->setDelegator(css::uno::Reference<css::uno::XInterface> ());

}

Any SAL_CALL OFormattedFieldWrapper::queryAggregation(const Type& _rType) throw (RuntimeException, std::exception)
{
    Any aReturn;

    if (_rType.equals( cppu::UnoType<XTypeProvider>::get() ) )
    {   // a XTypeProvider interface needs a working aggregate - we don't want to give the type provider
        // of our base class (OFormattedFieldWrapper_Base) to the caller as it supplies nearly nothing
        ensureAggregate();
        if (m_xAggregate.is())
            aReturn = m_xAggregate->queryAggregation(_rType);
    }

    if (!aReturn.hasValue())
    {
        aReturn = OFormattedFieldWrapper_Base::queryAggregation(_rType);

        if ((_rType.equals( cppu::UnoType<XServiceInfo>::get() ) ) && aReturn.hasValue())
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

OUString SAL_CALL OFormattedFieldWrapper::getServiceName() throw(RuntimeException, std::exception)
{
    // return the old compatibility name for an EditModel
    return OUString(FRM_COMPONENT_EDIT);
}

OUString SAL_CALL OFormattedFieldWrapper::getImplementationName(  ) throw (RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.forms.OFormattedFieldWrapper_ForcedFormatted");
}

sal_Bool SAL_CALL OFormattedFieldWrapper::supportsService( const OUString& _rServiceName ) throw (RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > SAL_CALL OFormattedFieldWrapper::getSupportedServiceNames(  ) throw (RuntimeException, std::exception)
{
    DBG_ASSERT(m_xAggregate.is(), "OFormattedFieldWrapper::getSupportedServiceNames: should never have made it 'til here without an aggregate!");
    Reference< XServiceInfo > xSI;
    m_xAggregate->queryAggregation(cppu::UnoType<XServiceInfo>::get()) >>= xSI;
    return xSI->getSupportedServiceNames();
}

void SAL_CALL OFormattedFieldWrapper::write(const Reference<XObjectOutputStream>& _rxOutStream) throw( IOException, RuntimeException, std::exception )
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
        throw RuntimeException( OUString(), *this );

    // for this we transfer the current props of the formatted part to the edit part
    Reference<XPropertySet>  xFormatProps(m_xFormattedPart, UNO_QUERY);
    Reference<XPropertySet> xEditProps(
        static_cast<XWeak*>(m_pEditPart.get()), css::uno::UNO_QUERY);

    Locale aAppLanguage = Application::GetSettings().GetUILanguageTag().getLocale();
    dbtools::TransferFormComponentProperties(xFormatProps, xEditProps, aAppLanguage);

    // then write the edit part, after switching to "fake mode"
    m_pEditPart->enableFormattedWriteFake();
    m_pEditPart->write(_rxOutStream);
    m_pEditPart->disableFormattedWriteFake();

    // and finally write the formatted part we're really interested in
    m_xFormattedPart->write(_rxOutStream);
}

void SAL_CALL OFormattedFieldWrapper::read(const Reference<XObjectInputStream>& _rxInStream) throw( IOException, RuntimeException, std::exception )
{
    SolarMutexGuard g;
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
        rtl::Reference< OEditModel > pBasicReader(new OEditModel(m_xContext));
        pBasicReader->read(_rxInStream);

        // was it really an edit model ?
        if (!pBasicReader->lastReadWasFormattedFake())
        {
            // yes -> all fine
            m_xAggregate.set( pBasicReader.get() );
        }
        else
        {   // no -> substitute it with a formatted model
            // let the formmatted model do the reading
            m_xFormattedPart.set(new OFormattedModel(m_xContext));
            m_xFormattedPart->read(_rxInStream);
            m_pEditPart = pBasicReader;
            m_xAggregate.set( m_xFormattedPart, UNO_QUERY );
        }
    }

    // do the aggregation
    osl_atomic_increment(&m_refCount);
    if (m_xAggregate.is())
    {   // has to be in it's own block because of the temporary variable created by *this
        m_xAggregate->setDelegator(static_cast<XWeak*>(this));
    }
    osl_atomic_decrement(&m_refCount);
}

void OFormattedFieldWrapper::ensureAggregate()
{
    if (m_xAggregate.is())
        return;

    {
        // instantiate an EditModel (the only place where we are allowed to decide that we're an FormattedModel
        // is in ::read)
        css::uno::Reference<css::uno::XInterface>  xEditModel = m_xContext->getServiceManager()->createInstanceWithContext(FRM_SUN_COMPONENT_TEXTFIELD, m_xContext);
        if (!xEditModel.is())
        {
            // arghhh... instantiate it directly... it's dirty, but we really need this aggregate
            OEditModel* pModel = new OEditModel(m_xContext);
            xEditModel.set(static_cast<XWeak*>(pModel), css::uno::UNO_QUERY);
        }

        m_xAggregate.set(xEditModel, UNO_QUERY);
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

    osl_atomic_increment(&m_refCount);
    if (m_xAggregate.is())
    {   // has to be in it's own block because of the temporary variable created by *this
        m_xAggregate->setDelegator(static_cast<XWeak*>(this));
    }
    osl_atomic_decrement(&m_refCount);
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_form_OFormattedFieldWrapper_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    css::uno::Reference<css::uno::XInterface> inst(
        OFormattedFieldWrapper::createFormattedFieldWrapper(component, false));
    inst->acquire();
    return inst.get();
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface* SAL_CALL
com_sun_star_comp_forms_OFormattedFieldWrapper_ForcedFormatted_get_implementation(css::uno::XComponentContext* component,
        css::uno::Sequence<css::uno::Any> const &)
{
    css::uno::Reference<css::uno::XInterface> inst(
        OFormattedFieldWrapper::createFormattedFieldWrapper(component, true));
    inst->acquire();
    return inst.get();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
