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

#include "FormsCollection.hxx"

#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif


#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif
#ifndef _RTL_LOGFILE_HXX_ 
#include <rtl/logfile.hxx>
#endif
namespace binfilter {

//.........................................................................
namespace frm
{
//.........................................................................
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::form;
using namespace ::com::sun::star::container;
//------------------------------------------------------------------
DBG_NAME(OFormsCollection)
//------------------------------------------------------------------
InterfaceRef SAL_CALL OFormsCollection_CreateInstance(const Reference<XMultiServiceFactory>& _rxFactory)
{
    return *(new OFormsCollection(_rxFactory));
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFormsCollection::getServiceName() throw(RuntimeException)
{
    return FRM_SUN_FORMS_COLLECTION;
}

//------------------------------------------------------------------------------
Sequence< sal_Int8 > SAL_CALL OFormsCollection::getImplementationId(  ) throw(RuntimeException)
{
    return OImplementationIds::getImplementationId(getTypes());
}

//------------------------------------------------------------------------------
Sequence<Type> SAL_CALL OFormsCollection::getTypes() throw(RuntimeException)
{
    return concatSequences(OInterfaceContainer::getTypes(), FormsCollectionComponentBase::getTypes(), OFormsCollection_BASE::getTypes());
}

//------------------------------------------------------------------
OFormsCollection::OFormsCollection(const Reference<XMultiServiceFactory>& _rxFactory)
         :FormsCollectionComponentBase(m_aMutex)
         ,OInterfaceContainer(_rxFactory, m_aMutex, ::getCppuType(static_cast<Reference<XForm>*>(NULL)))
{
    DBG_CTOR(OFormsCollection, NULL);
}

//------------------------------------------------------------------------------
OFormsCollection::~OFormsCollection()
{
    DBG_DTOR(OFormsCollection, NULL);
    if (!FormsCollectionComponentBase::rBHelper.bDisposed)
    {
        acquire();
        dispose();
    }
}

//------------------------------------------------------------------------------
Any SAL_CALL OFormsCollection::queryAggregation(const Type& _rType) throw(RuntimeException)
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

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFormsCollection::getImplementationName() throw(RuntimeException)
{
    return ::rtl::OUString::createFromAscii("com.sun.star.comp.forms.OFormsCollection");
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OFormsCollection::supportsService( const ::rtl::OUString& _rServiceName ) throw(RuntimeException)
{
    Sequence<rtl::OUString> aSupported = getSupportedServiceNames();
    const rtl::OUString* pSupported = aSupported.getConstArray();
    for (sal_Int32 i=0; i<aSupported.getLength(); ++i, ++pSupported)
        if (pSupported->equals(_rServiceName))
            return sal_True;
    return sal_False;
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL OFormsCollection::getSupportedServiceNames() throw(RuntimeException)
{
    StringSequence aReturn(2);

    aReturn.getArray()[0] = FRM_SUN_FORMS_COLLECTION;
    aReturn.getArray()[1] = ::rtl::OUString::createFromAscii("com.sun.star.form.FormComponents");

    return aReturn;
}

// OComponentHelper
//------------------------------------------------------------------------------
void OFormsCollection::disposing()
{
    {
        RTL_LOGFILE_CONTEXT( aLogger, "forms::OFormsCollection::disposing" );
        OInterfaceContainer::disposing();
    }
    FormsCollectionComponentBase::disposing();
    m_xParent = NULL;
}

//XChild
//------------------------------------------------------------------------------
void OFormsCollection::setParent(const InterfaceRef& Parent) throw( NoSupportException, RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_xParent = Parent;
}

//------------------------------------------------------------------------------
InterfaceRef  OFormsCollection::getParent() throw( RuntimeException )
{
    return m_xParent;
}

//.........................................................................
}	// namespace frm
//.........................................................................

}
