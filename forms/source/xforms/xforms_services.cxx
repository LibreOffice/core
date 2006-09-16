/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: xforms_services.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-17 00:06:08 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
