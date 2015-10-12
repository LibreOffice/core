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
#ifndef INCLUDED_FORMS_SOURCE_XFORMS_MODEL_HELPER_HXX
#define INCLUDED_FORMS_SOURCE_XFORMS_MODEL_HELPER_HXX


// some helper definitions that must be available for model.cxx and
// model_ui.cxx


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


// BindingCollection


namespace xforms
{

class BindingCollection : public NamedCollection<css::uno::Reference<css::beans::XPropertySet> >
{
    Model* mpModel;

public:
    explicit BindingCollection( Model* pModel ) : mpModel( pModel ) {}
    virtual ~BindingCollection() {}

    virtual bool isValid( const T& t ) const override
    {
        return Binding::getBinding( t ) != NULL;
    }

protected:
    virtual void _insert( const T& t ) override
    {
        OSL_ENSURE( Binding::getBinding( t ) != NULL, "invalid item?" );
        Binding::getBinding( t )->_setModel( css::uno::Reference<css::xforms::XModel>( mpModel ) );
    }

    virtual void _remove( const T& t ) override
    {
        OSL_ENSURE( Binding::getBinding( t ) != NULL, "invalid item?" );
        Binding::getBinding( t )->_setModel( css::uno::Reference<css::xforms::XModel>() );
    }
};

class SubmissionCollection : public NamedCollection<css::uno::Reference<css::beans::XPropertySet> >
{
    Model* mpModel;

public:
    explicit SubmissionCollection( Model* pModel ) : mpModel( pModel ) {}
    virtual ~SubmissionCollection() {}

public:
    virtual bool isValid( const T& t ) const override
    {
        return Submission::getSubmission( t ) != NULL;
    }

protected:
    virtual void _insert( const T& t ) override
    {
        OSL_ENSURE( Submission::getSubmission( t ) != NULL, "invalid item?" );
        Submission::getSubmission( t )->setModel( css::uno::Reference<css::xforms::XModel>( mpModel ) );
    }

    virtual void _remove( const T& t ) override
    {
        OSL_ENSURE( Submission::getSubmission( t ) != NULL, "invalid item?" );
        Submission::getSubmission( t )->setModel( css::uno::Reference<css::xforms::XModel>( ) );
    }
};

class InstanceCollection : public Collection<css::uno::Sequence<css::beans::PropertyValue> >
{
public:
    virtual bool isValid( const T& t ) const override
    {
        const css::beans::PropertyValue* pValues = t.getConstArray();
        OUString sInstance( "Instance" );
        bool bFound = false;
        for( sal_Int32 i = 0; ( ! bFound ) && ( i < t.getLength() ); i++ )
        {
            bFound |= ( pValues[i].Name == sInstance );
        }
        return bFound;
    }
};



// helper functions


sal_Int32 lcl_findInstance( const InstanceCollection*,
                            const OUString& );


// get values from Sequence<PropertyValue> describing an Instance
void getInstanceData(
    const css::uno::Sequence<css::beans::PropertyValue>&,
    OUString* pID,
    css::uno::Reference<css::xml::dom::XDocument>*,
    OUString* pURL,
    bool* pURLOnce );

// set values on Sequence<PropertyValue> for an Instance
void setInstanceData(
    css::uno::Sequence<css::beans::PropertyValue>&,
    const OUString* pID,
    const css::uno::Reference<css::xml::dom::XDocument>*,
    const OUString* pURL,
    const bool* pURLOnce );

} // namespace xforms

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
