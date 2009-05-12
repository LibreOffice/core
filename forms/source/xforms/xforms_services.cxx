/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: xforms_services.cxx,v $
 * $Revision: 1.4 $
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
