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
