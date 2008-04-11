/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: model_helper.hxx,v $
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
#ifndef _MODEL_HELPER_HXX
#define _MODEL_HELPER_HXX

//
// some helper definitions that must be available for model.cxx and
// model_ui.cxx
//

#include "namedcollection.hxx"
#include "binding.hxx"
#include "submission.hxx"
#include "unohelper.hxx"

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/beans/PropertyValue.hpp>

namespace xforms
{
    class Model;
}

//
// BindingCollection
//

namespace xforms
{

class BindingCollection : public NamedCollection<com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> >
{
    Model* mpModel;

public:
    BindingCollection( Model* pModel ) : mpModel( pModel ) {}
    virtual ~BindingCollection() {}

    virtual bool isValid( const T& t ) const
    {
        return Binding::getBinding( t ) != NULL;
    }

protected:
    virtual void _insert( const T& t )
    {
        OSL_ENSURE( Binding::getBinding( t ) != NULL, "invalid item?" );
        Binding::getBinding( t )->_setModel( Binding::Model_t( mpModel ) );
    }

    virtual void _remove( const T& t )
    {
        OSL_ENSURE( Binding::getBinding( t ) != NULL, "invalid item?" );
        Binding::getBinding( t )->_setModel( Binding::Model_t() );
    }
};



//
// SubmissionCollection
//

class SubmissionCollection : public NamedCollection<com::sun::star::uno::Reference<com::sun::star::beans::XPropertySet> >
{
    Model* mpModel;

public:
    SubmissionCollection( Model* pModel ) : mpModel( pModel ) {}
    virtual ~SubmissionCollection() {}

public:
    virtual bool isValid( const T& t ) const
    {
        return Submission::getSubmission( t ) != NULL;
    }

protected:
    virtual void _insert( const T& t )
    {
        OSL_ENSURE( Submission::getSubmission( t ) != NULL, "invalid item?" );
        Submission::getSubmission( t )->setModel( com::sun::star::uno::Reference<com::sun::star::xforms::XModel>( mpModel ) );
    }

    virtual void _remove( const T& t )
    {
        OSL_ENSURE( Submission::getSubmission( t ) != NULL, "invalid item?" );
        Submission::getSubmission( t )->setModel( com::sun::star::uno::Reference<com::sun::star::xforms::XModel>( ) );
    }
};


//
// InstanceCollection
//

class InstanceCollection : public Collection<com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue> >
{
public:
    virtual bool isValid( const T& t ) const
    {
        const com::sun::star::beans::PropertyValue* pValues = t.getConstArray();
        rtl::OUString sInstance( OUSTRING("Instance") );
        sal_Bool bFound = sal_False;
        for( sal_Int32 i = 0; ( ! bFound ) && ( i < t.getLength() ); i++ )
        {
            bFound |= ( pValues[i].Name == sInstance );
        }
        return bFound ? true : false;
    }
};


//
// helper functions
//

sal_Int32 lcl_findInstance( const InstanceCollection*,
                            const rtl::OUString& );


// get values from Sequence<PropertyValue> describing an Instance
void getInstanceData(
    const com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>&,
    rtl::OUString* pID,
    com::sun::star::uno::Reference<com::sun::star::xml::dom::XDocument>*,
    rtl::OUString* pURL,
    bool* pURLOnce );

// set values on Sequence<PropertyValue> for an Instance
void setInstanceData(
    com::sun::star::uno::Sequence<com::sun::star::beans::PropertyValue>&,
    const rtl::OUString* pID,
    const com::sun::star::uno::Reference<com::sun::star::xml::dom::XDocument>*,
    const rtl::OUString* pURL,
    const bool* pURLOnce );

} // namespace xforms

#endif
