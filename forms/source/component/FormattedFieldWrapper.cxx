/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

#include "FormattedFieldWrapper.hxx"
#include "Edit.hxx"
#include "FormattedField.hxx"
#include "EditBase.hxx"
#include "services.hxx"
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <connectivity/dbtools.hxx>
#include <tools/debug.hxx>
#include <vcl/svapp.hxx>
#include <vcl/settings.hxx>

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


Reference<XInterface> SAL_CALL OFormattedFieldWrapper_CreateInstance_ForceFormatted(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return OFormattedFieldWrapper::createFormattedFieldWrapper( comphelper::getComponentContext(_rxFactory), true);
}

InterfaceRef SAL_CALL OFormattedFieldWrapper_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return OFormattedFieldWrapper::createFormattedFieldWrapper( comphelper::getComponentContext(_rxFactory), false);
}

OFormattedFieldWrapper::OFormattedFieldWrapper(const Reference<XComponentContext>& _rxFactory)
    :m_xContext(_rxFactory)
{
}

InterfaceRef OFormattedFieldWrapper::createFormattedFieldWrapper(const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext>& _rxFactory, bool bActAsFormatted)
{
    OFormattedFieldWrapper *pRef = new OFormattedFieldWrapper(_rxFactory);

    if (bActAsFormatted)
    {
        
        InterfaceRef  xFormattedModel;
        
        
        OFormattedModel* pModel = new OFormattedModel(pRef->m_xContext);
        query_interface(static_cast<XWeak*>(pModel), xFormattedModel);

        pRef->m_xAggregate = Reference<XAggregation> (xFormattedModel, UNO_QUERY);
        OSL_ENSURE(pRef->m_xAggregate.is(), "the OFormattedModel didn't have an XAggregation interface !");

        
        query_interface(xFormattedModel, pRef->m_xFormattedPart);
        pRef->m_pEditPart = rtl::Reference< OEditModel >(new OEditModel(pRef->m_xContext));
    }

    increment(pRef->m_refCount);

    if (pRef->m_xAggregate.is())
    {   
        pRef->m_xAggregate->setDelegator(static_cast<XWeak*>(pRef));
    }

    InterfaceRef xRef(*pRef);
    decrement(pRef->m_refCount);

    return xRef;
}

Reference< XCloneable > SAL_CALL OFormattedFieldWrapper::createClone() throw (RuntimeException)
{
    ensureAggregate();

    rtl::Reference< OFormattedFieldWrapper > xRef(new OFormattedFieldWrapper(m_xContext));

    Reference< XCloneable > xCloneAccess;
    query_aggregation( m_xAggregate, xCloneAccess );

    
    if ( xCloneAccess.is() )
    {
        Reference< XCloneable > xClone = xCloneAccess->createClone();
        xRef->m_xAggregate = Reference< XAggregation >(xClone, UNO_QUERY);
        OSL_ENSURE(xRef->m_xAggregate.is(), "invalid aggregate cloned !");

        query_interface( Reference< XInterface >(xClone.get() ), xRef->m_xFormattedPart);

        if ( m_pEditPart.is() )
        {
            xRef->m_pEditPart = rtl::Reference< OEditModel >( new OEditModel(m_pEditPart.get(), m_xContext));
        }
    }
    else
    {   
    }

    if ( xRef->m_xAggregate.is() )
    {   
        xRef->m_xAggregate->setDelegator(static_cast< XWeak* >(xRef.get()));
    }

    return xRef.get();
}

OFormattedFieldWrapper::~OFormattedFieldWrapper()
{
    
    if (m_xAggregate.is())
        m_xAggregate->setDelegator(InterfaceRef ());

}

Any SAL_CALL OFormattedFieldWrapper::queryAggregation(const Type& _rType) throw (RuntimeException)
{
    Any aReturn;

    if (_rType.equals( ::getCppuType( static_cast< Reference< XTypeProvider >* >(NULL) ) ) )
    {   
        
        ensureAggregate();
        if (m_xAggregate.is())
            aReturn = m_xAggregate->queryAggregation(_rType);
    }

    if (!aReturn.hasValue())
    {
        aReturn = OFormattedFieldWrapper_Base::queryAggregation(_rType);

        if ((_rType.equals( ::getCppuType( static_cast< Reference< XServiceInfo >* >(NULL) ) ) ) && aReturn.hasValue())
        {   
            
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
                
                
                
                ensureAggregate();
                if (m_xAggregate.is())
                    aReturn = m_xAggregate->queryAggregation(_rType);
            }
        }
    }

    return aReturn;
}

OUString SAL_CALL OFormattedFieldWrapper::getServiceName() throw(RuntimeException)
{
    
    return OUString(FRM_COMPONENT_EDIT);
}

OUString SAL_CALL OFormattedFieldWrapper::getImplementationName(  ) throw (RuntimeException)
{
    return OUString("com.sun.star.comp.forms.OFormattedFieldWrapper");
}

sal_Bool SAL_CALL OFormattedFieldWrapper::supportsService( const OUString& _rServiceName ) throw (RuntimeException)
{
    return cppu::supportsService(this, _rServiceName);
}

Sequence< OUString > SAL_CALL OFormattedFieldWrapper::getSupportedServiceNames(  ) throw (RuntimeException)
{
    DBG_ASSERT(m_xAggregate.is(), "OFormattedFieldWrapper::getSupportedServiceNames: should never have made it 'til here without an aggregate!");
    Reference< XServiceInfo > xSI;
    m_xAggregate->queryAggregation(::getCppuType(static_cast< Reference< XServiceInfo >* >(NULL))) >>= xSI;
    return xSI->getSupportedServiceNames();
}

void SAL_CALL OFormattedFieldWrapper::write(const Reference<XObjectOutputStream>& _rxOutStream) throw( IOException, RuntimeException )
{
    
    ensureAggregate();

    
    if (!m_xFormattedPart.is())
    {
        Reference<XPersistObject>  xAggregatePersistence;
        query_aggregation(m_xAggregate, xAggregatePersistence);
        DBG_ASSERT(xAggregatePersistence.is(), "OFormattedFieldWrapper::write : don't know how to handle this : can't write !");
            
        if (xAggregatePersistence.is())
            xAggregatePersistence->write(_rxOutStream);
        return;
    }

    
    OSL_ENSURE(m_pEditPart.is(), "OFormattedFieldWrapper::write : formatted part without edit part ?");
    if ( !m_pEditPart.is() )
        throw RuntimeException( OUString(), *this );

    
    Reference<XPropertySet>  xFormatProps(m_xFormattedPart, UNO_QUERY);
    Reference<XPropertySet>  xEditProps;
    query_interface(static_cast<XWeak*>(m_pEditPart.get()), xEditProps);

    Locale aAppLanguage = Application::GetSettings().GetUILanguageTag().getLocale();
    dbtools::TransferFormComponentProperties(xFormatProps, xEditProps, aAppLanguage);

    
    m_pEditPart->enableFormattedWriteFake();
    m_pEditPart->write(_rxOutStream);
    m_pEditPart->disableFormattedWriteFake();

    
    m_xFormattedPart->write(_rxOutStream);
}

void SAL_CALL OFormattedFieldWrapper::read(const Reference<XObjectInputStream>& _rxInStream) throw( IOException, RuntimeException )
{
    SolarMutexGuard g;
    if (m_xAggregate.is())
    {   

        
        if (m_xFormattedPart.is())
        {
            
            
            
            
            
            
            Reference<XMarkableStream>  xInMarkable(_rxInStream, UNO_QUERY);
            DBG_ASSERT(xInMarkable.is(), "OFormattedFieldWrapper::read : can only work with markable streams !");
            sal_Int32 nBeforeEditPart = xInMarkable->createMark();

            m_pEditPart->read(_rxInStream);
            
            
            if (!m_pEditPart->lastReadWasFormattedFake())
            {   
                xInMarkable->jumpToMark(nBeforeEditPart);
            }
            xInMarkable->deleteMark(nBeforeEditPart);
        }

        Reference<XPersistObject>  xAggregatePersistence;
        query_aggregation(m_xAggregate, xAggregatePersistence);
        DBG_ASSERT(xAggregatePersistence.is(), "OFormattedFieldWrapper::read : don't know how to handle this : can't read !");
            

        if (xAggregatePersistence.is())
            xAggregatePersistence->read(_rxInStream);
        return;
    }

    
    

    {
        
        rtl::Reference< OEditModel > pBasicReader(new OEditModel(m_xContext));
        pBasicReader->read(_rxInStream);

        
        if (!pBasicReader->lastReadWasFormattedFake())
        {
            
            m_xAggregate = Reference< XAggregation >( pBasicReader.get() );
        }
        else
        {   
            
            m_xFormattedPart = Reference< XPersistObject >(new OFormattedModel(m_xContext));
            m_xFormattedPart->read(_rxInStream);
            m_pEditPart = pBasicReader;
            m_xAggregate = Reference< XAggregation >( m_xFormattedPart, UNO_QUERY );
        }
    }

    
    increment(m_refCount);
    if (m_xAggregate.is())
    {   
        m_xAggregate->setDelegator(static_cast<XWeak*>(this));
    }
    decrement(m_refCount);
}

void OFormattedFieldWrapper::ensureAggregate()
{
    if (m_xAggregate.is())
        return;

    {
        
        
        InterfaceRef  xEditModel = m_xContext->getServiceManager()->createInstanceWithContext(FRM_SUN_COMPONENT_TEXTFIELD, m_xContext);
        if (!xEditModel.is())
        {
            
            OEditModel* pModel = new OEditModel(m_xContext);
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
    {   
        m_xAggregate->setDelegator(static_cast<XWeak*>(this));
    }
    decrement(m_refCount);
}

}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
