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

#include "FormattedFieldWrapper.hxx"
#include "Edit.hxx"
#include "FormattedField.hxx"
#include <tools/debug.hxx>
#include "EditBase.hxx"
#include "services.hxx"
#include <connectivity/dbtools.hxx>
#include <vcl/svapp.hxx>
#include "boost/noncopyable.hpp"

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

namespace {

class RefCountGuard: private boost::noncopyable {
public:
    explicit RefCountGuard(oslInterlockedCount & count): count_(count)
    { osl_atomic_increment(&count_); }

    ~RefCountGuard() { osl_atomic_decrement(&count_); }

private:
    oslInterlockedCount & count_;
};

}

//==================================================================
// OFormattedFieldWrapper
//==================================================================
DBG_NAME(OFormattedFieldWrapper)
//------------------------------------------------------------------
InterfaceRef SAL_CALL OFormattedFieldWrapper_CreateInstance_ForceFormatted(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OFormattedFieldWrapper(_rxFactory, sal_True));
}

//------------------------------------------------------------------
InterfaceRef SAL_CALL OFormattedFieldWrapper_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OFormattedFieldWrapper(_rxFactory, sal_False));
}

//------------------------------------------------------------------
OFormattedFieldWrapper::OFormattedFieldWrapper(const Reference<XMultiServiceFactory>& _rxFactory, sal_Bool _bActAsFormatted)
    :m_xServiceFactory(_rxFactory)
{
    DBG_CTOR(OFormattedFieldWrapper, NULL);

    if (_bActAsFormatted)
    {
        RefCountGuard g(m_refCount);
        {
            // instantiate an FormattedModel (instantiate it directly, as the
            // OFormattedModel isn't registered for any service names anymore)
            m_xFormattedPart.set(new OFormattedModel(m_xServiceFactory));
            m_editPart.set(new OEditModel(m_xServiceFactory));
            m_xAggregate.set(m_xFormattedPart, UNO_QUERY_THROW);
        }
        m_xAggregate->setDelegator(static_cast< OWeakObject * >(this));
    }
}

//------------------------------------------------------------------
OFormattedFieldWrapper::OFormattedFieldWrapper( const OFormattedFieldWrapper* _pCloneSource )
    :m_xServiceFactory( _pCloneSource->m_xServiceFactory )
{
    Reference< XCloneable > xCloneAccess;
    query_aggregation( _pCloneSource->m_xAggregate, xCloneAccess );

    // clone the aggregate
    if ( xCloneAccess.is() )
    {
        RefCountGuard g(m_refCount);
        {
            m_xFormattedPart.set(xCloneAccess->createClone(), UNO_QUERY_THROW);
            if (_pCloneSource->m_editPart.is())
            {
                m_editPart.set(
                    new OEditModel(
                        _pCloneSource->m_editPart.get(),
                        _pCloneSource->m_xServiceFactory));
            }
            m_xAggregate.set(m_xFormattedPart, UNO_QUERY_THROW);
        }
        m_xAggregate->setDelegator(static_cast< OWeakObject * >(this));
    }
    else
    {   // the clone source does not yet have an aggregate -> we don't yet need one, too
    }
}

//------------------------------------------------------------------
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
    return ::rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("com.sun.star.comp.forms.OFormattedFieldWrapper") );
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
    DBG_ASSERT(
        m_editPart.is(),
        "OFormattedFieldWrapper::write: formatted part without edit part?");
    if (!m_editPart.is())
        throw RuntimeException( ::rtl::OUString(), *this );

    // for this we transfer the current props of the formatted part to the edit part
    Reference<XPropertySet>  xFormatProps(m_xFormattedPart, UNO_QUERY);
	Reference<XPropertySet>  xEditProps(
        static_cast< OWeakObject * >(m_editPart.get()), UNO_QUERY);

    Locale aAppLanguage = Application::GetSettings().GetUILocale();
    dbtools::TransferFormComponentProperties(xFormatProps, xEditProps, aAppLanguage);

    // then write the edit part, after switching to "fake mode"
	m_editPart->enableFormattedWriteFake();
	m_editPart->write(_rxOutStream);
	m_editPart->disableFormattedWriteFake();

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

			m_editPart->read(_rxInStream);
                // this only works because an edit model can read the stuff written by a formatted model (maybe with
                // some assertions) , but not vice versa
			if (!m_editPart->lastReadWasFormattedFake())
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

    // we have to decide from the data within the stream whether we should be an EditModel or a FormattedModel
    {
        // let an OEditModel do the reading
        rtl::Reference< OEditModel > basicReader(
            new OEditModel(m_xServiceFactory));
        basicReader->read(_rxInStream);

        // was it really an edit model ?
        if (!basicReader->lastReadWasFormattedFake())
            // yes -> all fine
            m_xAggregate.set(
                static_cast< OWeakObject * >(basicReader.get()),
                UNO_QUERY_THROW);
        else
        { // no -> substitute it with a formatted model

            // let the formmatted model do the reading
            m_xFormattedPart.set(new OFormattedModel(m_xServiceFactory));
            m_xFormattedPart->read(_rxInStream);

            m_editPart = basicReader;

            m_xAggregate.set(m_xFormattedPart, UNO_QUERY_THROW);
        }
    }
    RefCountGuard g(m_refCount);
    m_xAggregate->setDelegator(static_cast< OWeakObject * >(this));
}

//------------------------------------------------------------------
Reference< XCloneable > SAL_CALL OFormattedFieldWrapper::createClone(  ) throw (RuntimeException)
{
    ensureAggregate();

    return new OFormattedFieldWrapper( this );
}

//------------------------------------------------------------------
void OFormattedFieldWrapper::ensureAggregate()
{
    if (m_xAggregate.is())
        return;

    RefCountGuard g(m_refCount);
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

        m_xAggregate = Reference<XAggregation> (xEditModel, UNO_QUERY_THROW);

        Reference< XServiceInfo > xSI(m_xAggregate, UNO_QUERY);
        if (!xSI.is())
        {
            OSL_FAIL("OFormattedFieldWrapper::ensureAggregate: the aggregate has no XServiceInfo!");
            m_xAggregate.clear();
        }
    }
    if (m_xAggregate.is())
    {
        m_xAggregate->setDelegator(static_cast<XWeak*>(this));
    }
}

//.........................................................................
}
//.........................................................................


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
