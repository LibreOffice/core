/*************************************************************************
 *
 *  $RCSfile: model_helper.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 10:54:22 $
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
