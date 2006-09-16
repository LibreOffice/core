/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: interaction.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2006-09-16 13:54:21 $
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
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_DISPATCH_INTERACTION_HXX_
#include <dispatch/interaction.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  non exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  non exported definitions
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  declarations
//_________________________________________________________________________________________________________________

//---------------------------------------------------------------------------------------------------------
// initialize continuation with right start values
//---------------------------------------------------------------------------------------------------------
ContinuationFilterSelect::ContinuationFilterSelect()
    : m_sFilter( ::rtl::OUString() )
{
}

//---------------------------------------------------------------------------------------------------------
// handler should use it after selection to set user specified filter for transport
//---------------------------------------------------------------------------------------------------------
void SAL_CALL ContinuationFilterSelect::setFilter( const ::rtl::OUString& sFilter ) throw( css::uno::RuntimeException )
{
    m_sFilter = sFilter;
}

//---------------------------------------------------------------------------------------------------------
// read access to transported filter
//---------------------------------------------------------------------------------------------------------
::rtl::OUString SAL_CALL ContinuationFilterSelect::getFilter() throw( css::uno::RuntimeException )
{
    return m_sFilter;
}

//---------------------------------------------------------------------------------------------------------
// initialize instance with all neccessary informations
// We use it without any further checks on our member then ...!
//---------------------------------------------------------------------------------------------------------
RequestFilterSelect::RequestFilterSelect( const ::rtl::OUString& sURL )
{
    ::rtl::OUString temp;
    css::uno::Reference< css::uno::XInterface > temp2;
    css::document::NoSuchFilterRequest aFilterRequest( temp                             ,
                                                       temp2                            ,
                                                       sURL                                          );
    m_aRequest <<= aFilterRequest;

    m_pAbort  = new ContinuationAbort       ;
    m_pFilter = new ContinuationFilterSelect;

    m_lContinuations.realloc( 2 );
    m_lContinuations[0] = css::uno::Reference< css::task::XInteractionContinuation >( m_pAbort  );
    m_lContinuations[1] = css::uno::Reference< css::task::XInteractionContinuation >( m_pFilter );
}

//---------------------------------------------------------------------------------------------------------
// return abort state of interaction
// If it is true, return value of method "getFilter()" will be unspecified then!
//---------------------------------------------------------------------------------------------------------
sal_Bool RequestFilterSelect::isAbort() const
{
    return m_pAbort->isSelected();
}

//---------------------------------------------------------------------------------------------------------
// return user selected filter
// Return value valid for non aborted interaction only. Please check "isAbort()" before you call these ony!
//---------------------------------------------------------------------------------------------------------
::rtl::OUString RequestFilterSelect::getFilter() const
{
    return m_pFilter->getFilter();
}

//---------------------------------------------------------------------------------------------------------
// handler call it to get type of request
// Is hard coded to "please select filter" here. see ctor for further informations.
//---------------------------------------------------------------------------------------------------------
css::uno::Any SAL_CALL RequestFilterSelect::getRequest() throw( css::uno::RuntimeException )
{
    return m_aRequest;
}

//---------------------------------------------------------------------------------------------------------
// handler call it to get possible continuations
// We support "abort/select_filter" only here.
// After interaction we support read access on these continuations on our c++ interface to
// return user decision.
//---------------------------------------------------------------------------------------------------------
css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > SAL_CALL RequestFilterSelect::getContinuations() throw( css::uno::RuntimeException )
{
    return m_lContinuations;
}

//---------------------------------------------------------------------------------------------------------
// initialize instance with all neccessary informations
// We use it without any further checks on our member then ...!
//---------------------------------------------------------------------------------------------------------
RequestAmbigousFilter::RequestAmbigousFilter( const ::rtl::OUString& sURL            ,
                                              const ::rtl::OUString& sSelectedFilter ,
                                              const ::rtl::OUString& sDetectedFilter )
{
    ::rtl::OUString temp;
    css::uno::Reference< css::uno::XInterface > temp2;
    css::document::AmbigousFilterRequest aFilterRequest( temp                             ,
                                                         temp2 ,
                                                         sURL                                          ,
                                                         sSelectedFilter                               ,
                                                         sDetectedFilter                               );
    m_aRequest <<= aFilterRequest;

    m_pAbort  = new ContinuationAbort       ;
    m_pFilter = new ContinuationFilterSelect;

    m_lContinuations.realloc( 2 );
    m_lContinuations[0] = css::uno::Reference< css::task::XInteractionContinuation >( m_pAbort  );
    m_lContinuations[1] = css::uno::Reference< css::task::XInteractionContinuation >( m_pFilter );
}

//---------------------------------------------------------------------------------------------------------
// return abort state of interaction
// If it is true, return value of method "getFilter()" will be unspecified then!
//---------------------------------------------------------------------------------------------------------
sal_Bool RequestAmbigousFilter::isAbort() const
{
    return m_pAbort->isSelected();
}

//---------------------------------------------------------------------------------------------------------
// return user selected filter
// Return value valid for non aborted interaction only. Please check "isAbort()" before you call these ony!
//---------------------------------------------------------------------------------------------------------
::rtl::OUString RequestAmbigousFilter::getFilter() const
{
    return m_pFilter->getFilter();
}

//---------------------------------------------------------------------------------------------------------
// handler call it to get type of request
// Is hard coded to "please select filter" here. see ctor for further informations.
//---------------------------------------------------------------------------------------------------------
css::uno::Any SAL_CALL RequestAmbigousFilter::getRequest() throw( css::uno::RuntimeException )
{
    return m_aRequest;
}

//---------------------------------------------------------------------------------------------------------
// handler call it to get possible continuations
// We support "abort/select_filter" only here.
// After interaction we support read access on these continuations on our c++ interface to
// return user decision.
//---------------------------------------------------------------------------------------------------------
css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > SAL_CALL RequestAmbigousFilter::getContinuations() throw( css::uno::RuntimeException )
{
    return m_lContinuations;
}

}       //  namespace framework
