/*************************************************************************
 *
 *  $RCSfile: interaction.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-12-19 13:18:32 $
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
    css::document::NoSuchFilterRequest aFilterRequest( ::rtl::OUString()                             ,
                                                       css::uno::Reference< css::uno::XInterface >() ,
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
    css::document::AmbigousFilterRequest aFilterRequest( ::rtl::OUString()                             ,
                                                         css::uno::Reference< css::uno::XInterface >() ,
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
