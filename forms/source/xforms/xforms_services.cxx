/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/




// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_forms.hxx"


#include "services.hxx"

#include "binding.hxx"
#include "model.hxx"
#include "NameContainer.hxx"

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/container/XNameContainer.hpp>

using com::sun::star::uno::Reference;
using com::sun::star::uno::XInterface;
using com::sun::star::lang::XMultiServiceFactory;
using com::sun::star::uno::RuntimeException;
using com::sun::star::form::binding::XValueBinding;
using com::sun::star::beans::XPropertySet;
using com::sun::star::container::XNameContainer;
using rtl::OUString;


namespace frm
{

Reference<XInterface> Model_CreateInstance(
    const Reference<XMultiServiceFactory>& )
    throw( RuntimeException )
{
    return static_cast<XPropertySet*>( new xforms::Model );
}

Reference<XInterface> XForms_CreateInstance(
    const Reference<XMultiServiceFactory>& )
    throw( RuntimeException )
{
    return static_cast<XNameContainer*>( new NameContainer<Reference<XPropertySet> >() );
}

}
