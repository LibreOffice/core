/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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
#include "precompiled_framework.hxx"

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________
#include <comphelper/interaction.hxx>
#include <framework/interaction.hxx>
#include <general.h>

using namespace ::com::sun::star;

namespace framework{

/*-************************************************************************************************************//**
    @short          declaration of special continuation for filter selection
    @descr          Sometimes filter detection during loading document failed. Then we need a possibility
                    to ask user for his decision. These continuation transport selected filter by user to
                    code user of interaction.

    @attention      This implementation could be used one times only. We don't support a resetable continuation yet!
                    Why? Normaly interaction should show a filter selection dialog and ask user for his decision.
                    He can select any filter - then instances of these class will be called by handler ... or user
                    close dialog without any selection. Then another continuation should be slected by handler to
                    abort continuations ... Retrying isn't very usefull here ... I think.

    @implements     XInteractionFilterSelect

    @base           ImplInheritanceHelper1
                    ContinuationBase

    @devstatus      ready to use
    @threadsafe     no (used on once position only!)
*//*-*************************************************************************************************************/
class ContinuationFilterSelect : public comphelper::OInteraction< ::com::sun::star::document::XInteractionFilterSelect >
{
    // c++ interface
    public:
        ContinuationFilterSelect();

    // uno interface
    public:
        virtual void            SAL_CALL setFilter( const ::rtl::OUString& sFilter ) throw( ::com::sun::star::uno::RuntimeException );
        virtual ::rtl::OUString SAL_CALL getFilter(                                ) throw( ::com::sun::star::uno::RuntimeException );

    // member
    private:
        ::rtl::OUString m_sFilter;

};  // class ContinuationFilterSelect


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

class RequestFilterSelect_Impl : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionRequest >
{
public:
    RequestFilterSelect_Impl( const ::rtl::OUString& sURL );
    sal_Bool        isAbort  () const;
    ::rtl::OUString getFilter() const;

public:
    virtual ::com::sun::star::uno::Any SAL_CALL getRequest() throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > > SAL_CALL getContinuations() throw( ::com::sun::star::uno::RuntimeException );

private:
    ::com::sun::star::uno::Any                                                                                                 m_aRequest      ;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > >    m_lContinuations;
    comphelper::OInteractionAbort* m_pAbort;
    ContinuationFilterSelect* m_pFilter;
};

//---------------------------------------------------------------------------------------------------------
// initialize instance with all neccessary informations
// We use it without any further checks on our member then ...!
//---------------------------------------------------------------------------------------------------------
RequestFilterSelect_Impl::RequestFilterSelect_Impl( const ::rtl::OUString& sURL )
{
    ::rtl::OUString temp;
    css::uno::Reference< css::uno::XInterface > temp2;
    css::document::NoSuchFilterRequest aFilterRequest( temp                             ,
                                                       temp2                            ,
                                                       sURL                                          );
    m_aRequest <<= aFilterRequest;

    m_pAbort  = new comphelper::OInteractionAbort;
    m_pFilter = new ContinuationFilterSelect;

    m_lContinuations.realloc( 2 );
    m_lContinuations[0] = css::uno::Reference< css::task::XInteractionContinuation >( m_pAbort  );
    m_lContinuations[1] = css::uno::Reference< css::task::XInteractionContinuation >( m_pFilter );
}

//---------------------------------------------------------------------------------------------------------
// return abort state of interaction
// If it is true, return value of method "getFilter()" will be unspecified then!
//---------------------------------------------------------------------------------------------------------
sal_Bool RequestFilterSelect_Impl::isAbort() const
{
    return m_pAbort->wasSelected();
}

//---------------------------------------------------------------------------------------------------------
// return user selected filter
// Return value valid for non aborted interaction only. Please check "isAbort()" before you call these ony!
//---------------------------------------------------------------------------------------------------------
::rtl::OUString RequestFilterSelect_Impl::getFilter() const
{
    return m_pFilter->getFilter();
}

//---------------------------------------------------------------------------------------------------------
// handler call it to get type of request
// Is hard coded to "please select filter" here. see ctor for further informations.
//---------------------------------------------------------------------------------------------------------
css::uno::Any SAL_CALL RequestFilterSelect_Impl::getRequest() throw( css::uno::RuntimeException )
{
    return m_aRequest;
}

//---------------------------------------------------------------------------------------------------------
// handler call it to get possible continuations
// We support "abort/select_filter" only here.
// After interaction we support read access on these continuations on our c++ interface to
// return user decision.
//---------------------------------------------------------------------------------------------------------
css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > SAL_CALL RequestFilterSelect_Impl::getContinuations() throw( css::uno::RuntimeException )
{
    return m_lContinuations;
}


RequestFilterSelect::RequestFilterSelect( const ::rtl::OUString& sURL )
{
    pImp = new RequestFilterSelect_Impl( sURL );
    pImp->acquire();
}

RequestFilterSelect::~RequestFilterSelect()
{
    pImp->release();
}


//---------------------------------------------------------------------------------------------------------
// return abort state of interaction
// If it is true, return value of method "getFilter()" will be unspecified then!
//---------------------------------------------------------------------------------------------------------
sal_Bool RequestFilterSelect::isAbort() const
{
    return pImp->isAbort();
}

//---------------------------------------------------------------------------------------------------------
// return user selected filter
// Return value valid for non aborted interaction only. Please check "isAbort()" before you call these ony!
//---------------------------------------------------------------------------------------------------------
::rtl::OUString RequestFilterSelect::getFilter() const
{
    return pImp->getFilter();
}

uno::Reference < task::XInteractionRequest > RequestFilterSelect::GetRequest()
{
    return uno::Reference < task::XInteractionRequest > (pImp);
}

/*
class RequestAmbigousFilter_Impl : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionRequest >
{
public:
    RequestAmbigousFilter_Impl( const ::rtl::OUString& sURL            ,
                            const ::rtl::OUString& sSelectedFilter ,
                            const ::rtl::OUString& sDetectedFilter );
    sal_Bool        isAbort  () const;
    ::rtl::OUString getFilter() const;

    virtual ::com::sun::star::uno::Any SAL_CALL getRequest () throw( ::com::sun::star::uno::RuntimeException );
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > > SAL_CALL getContinuations() throw( ::com::sun::star::uno::RuntimeException );

    ::com::sun::star::uno::Any                                                                                                 m_aRequest      ;
    ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > >    m_lContinuations;
    ContinuationAbort*                                                                                                         m_pAbort        ;
    ContinuationFilterSelect*                                                                                                  m_pFilter       ;
};

RequestAmbigousFilter::RequestAmbigousFilter( const ::rtl::OUString& sURL, const ::rtl::OUString& sSelectedFilter,
    const ::rtl::OUString& sDetectedFilter )
{
    pImp = new RequestAmbigousFilter_Impl( sURL, sSelectedFilter, sDetectedFilter );
    pImp->acquire();
}

RequestAmbigousFilter::~RequestAmbigousFilter()
{
    pImp->release();
}

sal_Bool RequestAmbigousFilter::isAbort() const
{
    return pImp->isAbort();
}

//---------------------------------------------------------------------------------------------------------
// return user selected filter
// Return value valid for non aborted interaction only. Please check "isAbort()" before you call these ony!
//---------------------------------------------------------------------------------------------------------
::rtl::OUString RequestAmbigousFilter::getFilter() const
{
    return pImp->getFilter();
}

uno::Reference < task::XInteractionRequest > RequestAmbigousFilter::GetRequest()
{
    return uno::Reference < task::XInteractionRequest > (pImp);
}

//---------------------------------------------------------------------------------------------------------
// initialize instance with all neccessary informations
// We use it without any further checks on our member then ...!
//---------------------------------------------------------------------------------------------------------
RequestAmbigousFilter_Impl::RequestAmbigousFilter_Impl( const ::rtl::OUString& sURL            ,
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
sal_Bool RequestAmbigousFilter_Impl::isAbort() const
{
    return m_pAbort->isSelected();
}

//---------------------------------------------------------------------------------------------------------
// return user selected filter
// Return value valid for non aborted interaction only. Please check "isAbort()" before you call these ony!
//---------------------------------------------------------------------------------------------------------
::rtl::OUString RequestAmbigousFilter_Impl::getFilter() const
{
    return m_pFilter->getFilter();
}

//---------------------------------------------------------------------------------------------------------
// handler call it to get type of request
// Is hard coded to "please select filter" here. see ctor for further informations.
//---------------------------------------------------------------------------------------------------------
css::uno::Any SAL_CALL RequestAmbigousFilter_Impl::getRequest() throw( css::uno::RuntimeException )
{
    return m_aRequest;
}

//---------------------------------------------------------------------------------------------------------
// handler call it to get possible continuations
// We support "abort/select_filter" only here.
// After interaction we support read access on these continuations on our c++ interface to
// return user decision.
//---------------------------------------------------------------------------------------------------------
css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > SAL_CALL RequestAmbigousFilter_Impl::getContinuations() throw( css::uno::RuntimeException )
{
    return m_lContinuations;
}
*/

class InteractionRequest_Impl : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionRequest >
{
    uno::Any m_aRequest;
    uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > > m_lContinuations;

public:
    InteractionRequest_Impl( const ::com::sun::star::uno::Any& aRequest,
        const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > > lContinuations )
    {
        m_aRequest = aRequest;
        m_lContinuations = lContinuations;
    }

    virtual uno::Any SAL_CALL getRequest() throw( uno::RuntimeException );
    virtual uno::Sequence< uno::Reference< task::XInteractionContinuation > > SAL_CALL getContinuations()
            throw( uno::RuntimeException );
};

uno::Any SAL_CALL InteractionRequest_Impl::getRequest() throw( uno::RuntimeException )
{
    return m_aRequest;
}

uno::Sequence< uno::Reference< task::XInteractionContinuation > > SAL_CALL InteractionRequest_Impl::getContinuations()
    throw( uno::RuntimeException )
{
    return m_lContinuations;
}

uno::Reference < task::XInteractionRequest > InteractionRequest::CreateRequest(
    const uno::Any& aRequest, const uno::Sequence< uno::Reference< task::XInteractionContinuation > > lContinuations )
{
    return new InteractionRequest_Impl( aRequest, lContinuations );
}

}       //  namespace framework
