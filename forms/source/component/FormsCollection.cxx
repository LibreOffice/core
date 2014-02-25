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


#include "FormsCollection.hxx"
#include "services.hxx"
#include <comphelper/sequence.hxx>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <tools/debug.hxx>
#include <com/sun/star/form/XForm.hpp>

//.........................................................................
namespace frm
{
//.........................................................................
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::container;
using namespace ::com::sun::star::util;



InterfaceRef SAL_CALL OFormsCollection_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OFormsCollection( comphelper::getComponentContext(_rxFactory) ));
}


OUString SAL_CALL OFormsCollection::getServiceName() throw(RuntimeException, std::exception)
{
    return OUString("com.sun.star.form.Forms");
}


Sequence< sal_Int8 > SAL_CALL OFormsCollection::getImplementationId(  ) throw(RuntimeException, std::exception)
{
    return OImplementationIds::getImplementationId(getTypes());
}


Sequence<Type> SAL_CALL OFormsCollection::getTypes() throw(RuntimeException, std::exception)
{
    return concatSequences(OInterfaceContainer::getTypes(), FormsCollectionComponentBase::getTypes(), OFormsCollection_BASE::getTypes());
}


OFormsCollection::OFormsCollection(const Reference<XComponentContext>& _rxFactory)
    :FormsCollectionComponentBase( m_aMutex )
    ,OInterfaceContainer( _rxFactory, m_aMutex, cppu::UnoType<XForm>::get() )
    ,OFormsCollection_BASE()
{
}


OFormsCollection::OFormsCollection( const OFormsCollection& _cloneSource )
    :FormsCollectionComponentBase( m_aMutex )
    ,OInterfaceContainer( m_aMutex, _cloneSource )
    ,OFormsCollection_BASE()
{
}


OFormsCollection::~OFormsCollection()
{
    if (!FormsCollectionComponentBase::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }
}


Any SAL_CALL OFormsCollection::queryAggregation(const Type& _rType) throw(RuntimeException, std::exception)
{
    Any aReturn = OFormsCollection_BASE::queryInterface(_rType);
    if (!aReturn.hasValue())
    {
        aReturn = OInterfaceContainer::queryInterface(_rType);

        if (!aReturn.hasValue())
            aReturn = FormsCollectionComponentBase::queryAggregation(_rType);
    }

    return aReturn;
}

OUString SAL_CALL OFormsCollection::getImplementationName() throw(RuntimeException, std::exception)
{
    return OUString("com.sun.star.comp.forms.OFormsCollection");
}

sal_Bool SAL_CALL OFormsCollection::supportsService( const OUString& _rServiceName ) throw(RuntimeException, std::exception)
{
    return cppu::supportsService(this, _rServiceName);
}

StringSequence SAL_CALL OFormsCollection::getSupportedServiceNames() throw(RuntimeException, std::exception)
{
    StringSequence aReturn(2);

    aReturn[0] = "com.sun.star.form.Forms";
    aReturn[1] = "com.sun.star.form.FormComponents";

    return aReturn;
}

// XCloneable
Reference< XCloneable > SAL_CALL OFormsCollection::createClone(  ) throw (RuntimeException, std::exception)
{
    OFormsCollection* pClone = new OFormsCollection( *this );
    osl_atomic_increment( &pClone->m_refCount );
    pClone->clonedFrom( *this );
    osl_atomic_decrement( &pClone->m_refCount );
    return static_cast<OInterfaceContainer*>(pClone);
}

// OComponentHelper

void OFormsCollection::disposing()
{
    {
        SAL_INFO( "forms.component", "forms::OFormsCollection::disposing" );
        OInterfaceContainer::disposing();
    }
    FormsCollectionComponentBase::disposing();
    m_xParent = NULL;
}

//XChild

void OFormsCollection::setParent(const InterfaceRef& Parent) throw( NoSupportException, RuntimeException, std::exception )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_xParent = Parent;
}


InterfaceRef  OFormsCollection::getParent() throw( RuntimeException, std::exception )
{
    return m_xParent;
}

//.........................................................................
}   // namespace frm
//.........................................................................

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
