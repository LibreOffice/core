/*************************************************************************
 *
 *  $RCSfile: FormsCollection.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:29:04 $
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

#include "FormsCollection.hxx"

#ifndef _FRM_SERVICES_HXX_
#include "services.hxx"
#endif

#ifndef _UTL_SEQUENCE_HXX_
#include <unotools/sequence.hxx>
#endif
#ifndef _TOOLS_DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _COM_SUN_STAR_FORM_XFORM_HPP_
#include <com/sun/star/form/XForm.hpp>
#endif

//.........................................................................
namespace frm
{
//.........................................................................

//------------------------------------------------------------------
DBG_NAME(OFormsCollection)
//------------------------------------------------------------------
InterfaceRef SAL_CALL OFormsCollection_CreateInstance(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
{
    return *(new OFormsCollection(_rxFactory));
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFormsCollection::getServiceName() throw(staruno::RuntimeException)
{
    return FRM_SUN_FORMS_COLLECTION;
}

//------------------------------------------------------------------------------
staruno::Sequence< sal_Int8 > SAL_CALL OFormsCollection::getImplementationId(  ) throw(staruno::RuntimeException)
{
    return OImplementationIds::getImplementationId(getTypes());
}

//------------------------------------------------------------------------------
staruno::Sequence<staruno::Type> SAL_CALL OFormsCollection::getTypes() throw(staruno::RuntimeException)
{
    staruno::Sequence<staruno::Type> aBaseTypes = OInterfaceContainer::getTypes();
    staruno::Sequence<staruno::Type> aComponentTypes = FormsCollectionComponentBase::getTypes();

    staruno::Sequence<staruno::Type> aOwnTypes(1);
    aOwnTypes.getArray()[0] = ::getCppuType(static_cast<staruno::Reference<starcontainer::XChild>*>(NULL));

    return concatSequences(aBaseTypes, aComponentTypes, aOwnTypes);
}

//------------------------------------------------------------------
OFormsCollection::OFormsCollection(const staruno::Reference<starlang::XMultiServiceFactory>& _rxFactory)
         :FormsCollectionComponentBase(m_aMutex)
         ,OInterfaceContainer(_rxFactory, m_aMutex, ::getCppuType(static_cast<staruno::Reference<starform::XForm>*>(NULL)))
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
staruno::Any SAL_CALL OFormsCollection::queryAggregation(const staruno::Type& _rType) throw(staruno::RuntimeException)
{
    staruno::Any aReturn = ::cppu::queryInterface(
        _rType,
        static_cast<starcontainer::XChild*>(this),
        static_cast<starlang::XServiceInfo*>(this)
    );

    if (!aReturn.hasValue())
        aReturn = OInterfaceContainer::queryInterface(_rType);

    if (!aReturn.hasValue())
        aReturn = FormsCollectionComponentBase::queryAggregation(_rType);

    return aReturn;
}

//------------------------------------------------------------------------------
::rtl::OUString SAL_CALL OFormsCollection::getImplementationName() throw(staruno::RuntimeException)
{
    return ::rtl::OUString::createFromAscii("com.sun.star.form.OFormsCollection");
}

//------------------------------------------------------------------------------
sal_Bool SAL_CALL OFormsCollection::supportsService( const ::rtl::OUString& _rServiceName ) throw(staruno::RuntimeException)
{
    staruno::Sequence<rtl::OUString> aSupported = getSupportedServiceNames();
    const rtl::OUString* pSupported = aSupported.getConstArray();
    for (sal_Int32 i=0; i<aSupported.getLength(); ++i, ++pSupported)
        if (pSupported->equals(_rServiceName))
            return sal_True;
    return sal_False;
}

//------------------------------------------------------------------------------
StringSequence SAL_CALL OFormsCollection::getSupportedServiceNames() throw(staruno::RuntimeException)
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
    OInterfaceContainer::disposing();
    FormsCollectionComponentBase::disposing();
    m_xParent = NULL;
}

//XChild
//------------------------------------------------------------------------------
void OFormsCollection::setParent(const InterfaceRef& Parent) throw( starlang::NoSupportException, staruno::RuntimeException )
{
    ::osl::MutexGuard aGuard( m_aMutex );
    m_xParent = Parent;
}

//------------------------------------------------------------------------------
InterfaceRef  OFormsCollection::getParent() throw( staruno::RuntimeException )
{
    return m_xParent;
}

//.........................................................................
}   // namespace frm
//.........................................................................

