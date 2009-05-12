/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: interaction.hxx,v $
 * $Revision: 1.7 $
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

#ifndef __FRAMEWORK_DISPATCH_INTERACTION_HXX_
#define __FRAMEWORK_DISPATCH_INTERACTION_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#include <com/sun/star/task/XInteractionRequest.hpp>
#include <com/sun/star/task/XInteractionContinuation.hpp>
#include <com/sun/star/task/XInteractionAbort.hpp>
#include <com/sun/star/task/XInteractionApprove.hpp>
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#include <com/sun/star/task/XInteractionRetry.hpp>
#include <com/sun/star/document/XInteractionFilterSelect.hpp>
#include <com/sun/star/document/NoSuchFilterRequest.hpp>
#include <com/sun/star/document/AmbigousFilterRequest.hpp>
#include <com/sun/star/uno/RuntimeException.hpp>

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________
#include <rtl/ustring.hxx>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

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

/*-************************************************************************************************************//**
    @short          base for continuation classes
    @descr          An interaction continuation could be used on XInteractionHandler/XInteractionRequest
                    to abort or react for it.
                    Base functionality is everytime the same - handler mark right continuation by calling
                    interface method "select()". User of interaction can detect it by testing c++ method "isSelected()"!
                    Superclasses can add additional interfaces or methods to support additional features ...
                    but selection of it is supported here!

    @implements     XInterface
                    XTypeProvider (supported by WeakImplHelper!)
                    XInteractionContinuation

    @base           WeakImplHelper1

    @devstatus      ready to use
    @threadsafe     no (used on once position only!)
*//*-*************************************************************************************************************/
template< class TContinuationType >
class ContinuationBase : public ::cppu::WeakImplHelper1< TContinuationType >
{
    // c++ interface
    public:

        //---------------------------------------------------------------------------------------------------------
        // initialize continuation with right start values
        //---------------------------------------------------------------------------------------------------------
        ContinuationBase()
            :   m_bSelected( sal_False )
        {
        }

        //---------------------------------------------------------------------------------------------------------
        // was continuation selected by handler?
        //---------------------------------------------------------------------------------------------------------
        sal_Bool isSelected() const
        {
            return m_bSelected;
        }

        //---------------------------------------------------------------------------------------------------------
        // make using more then once possible
        //---------------------------------------------------------------------------------------------------------
        void reset()
        {
            m_bSelected = sal_False;
        }

    // uno interface
    public:

        //---------------------------------------------------------------------------------------------------------
        // called by handler to mark continuation as the only possible solution for started interaction
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL select() throw( ::com::sun::star::uno::RuntimeException )
        {
            m_bSelected = sal_True;
        }

    // member
    private:

        sal_Bool m_bSelected;

};  // class ContinuationBase

/*-************************************************************************************************************//**
    @short          declaration of some simple continuations
    @descr          These derived classes implements some simple continuations, which doesnt need and additional
                    interfaces or methods. Her selected state is the only neccessary feature. User of it can
                    distinguish by type between different functionality!

    @implements     -

    @base           ContinuationBase

    @devstatus      ready to use
    @threadsafe     no (used on once position only!)
*//*-*************************************************************************************************************/
typedef ContinuationBase< ::com::sun::star::task::XInteractionAbort > ContinuationAbort;
typedef ContinuationBase< ::com::sun::star::task::XInteractionApprove > ContinuationApprove;
typedef ContinuationBase< ::com::sun::star::task::XInteractionDisapprove > ContinuationDisapprove;
typedef ContinuationBase< ::com::sun::star::task::XInteractionRetry > ContinuationRetry;

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
class ContinuationFilterSelect : public ContinuationBase< ::com::sun::star::document::XInteractionFilterSelect >
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

/*-************************************************************************************************************//**
    @short          special request for interaction to ask user for right filter
    @descr          These helper can be used to ask user for right filter, if filter detection failed.
                    It capsulate communication with any interaction handler and supports an easy
                    access on interaction results for user of these class.
                    Use it and forget complex mechanism of interaction ...

    @example        RequestFilterSelect*             pRequest = new RequestFilterSelect;
                    Reference< XInteractionRequest > xRequest ( pRequest );
                    xInteractionHandler->handle( xRequest );
                    if( ! pRequest.isAbort() )
                    {
                        OUString sFilter = pRequest->getFilter();
                    }

    @implements     XInteractionRequest

    @base           WeakImplHelper1

    @devstatus      ready to use
    @threadsafe     no (used on once position only!)
*//*-*************************************************************************************************************/
class RequestFilterSelect : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionRequest >
{
    // c++ interface
    public:
        RequestFilterSelect( const ::rtl::OUString& sURL );
        sal_Bool        isAbort  () const;
        ::rtl::OUString getFilter() const;

    // uno interface
    public:
        virtual ::com::sun::star::uno::Any                                                                                              SAL_CALL getRequest      () throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > > SAL_CALL getContinuations() throw( ::com::sun::star::uno::RuntimeException );

    // member
    private:
        ::com::sun::star::uno::Any                                                                                                 m_aRequest      ;
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > >    m_lContinuations;
        ContinuationAbort*                                                                                                         m_pAbort        ;
        ContinuationFilterSelect*                                                                                                  m_pFilter       ;

};  // class RequestFilterSelect

/*-************************************************************************************************************//**
    @short          special request for interaction
    @descr          User must decide between a preselected and another detected filter.
                    It capsulate communication with any interaction handler and supports an easy
                    access on interaction results for user of these class.

    @implements     XInteractionRequest

    @base           WeakImplHelper1

    @devstatus      ready to use
    @threadsafe     no (used on once position only!)
*//*-*************************************************************************************************************/
class RequestAmbigousFilter : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionRequest >
{
    // c++ interface
    public:
        RequestAmbigousFilter( const ::rtl::OUString& sURL            ,
                               const ::rtl::OUString& sSelectedFilter ,
                               const ::rtl::OUString& sDetectedFilter );
        sal_Bool        isAbort  () const;
        ::rtl::OUString getFilter() const;

    // uno interface
    public:
        virtual ::com::sun::star::uno::Any                                                                                              SAL_CALL getRequest      () throw( ::com::sun::star::uno::RuntimeException );
        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > > SAL_CALL getContinuations() throw( ::com::sun::star::uno::RuntimeException );

    // member
    private:
        ::com::sun::star::uno::Any                                                                                                 m_aRequest      ;
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > >    m_lContinuations;
        ContinuationAbort*                                                                                                         m_pAbort        ;
        ContinuationFilterSelect*                                                                                                  m_pFilter       ;

};  // class RequestFilterSelect

/*-************************************************************************************************************//**
    @short          special request for interaction
    @descr          User must decide between a preselected and another detected filter.
                    It capsulate communication with any interaction handler and supports an easy
                    access on interaction results for user of these class.

    @implements     XInteractionRequest

    @base           WeakImplHelper1

    @devstatus      ready to use
    @threadsafe     no (used on once position only!)
*//*-*************************************************************************************************************/
class InteractionRequest : public ::cppu::WeakImplHelper1< ::com::sun::star::task::XInteractionRequest >
{
    // c++ interface
    public:
        InteractionRequest( const ::com::sun::star::uno::Any&                                                                                             aRequest       ,
                            const ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > > lContinuations )
        {
            m_aRequest       = aRequest      ;
            m_lContinuations = lContinuations;
        }

    // uno interface
    public:
        virtual ::com::sun::star::uno::Any SAL_CALL getRequest()
            throw( ::com::sun::star::uno::RuntimeException )
            {
                return m_aRequest;
            }

        virtual ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > > SAL_CALL getContinuations()
            throw( ::com::sun::star::uno::RuntimeException )
            {
                return m_lContinuations;
            }

    // member
    private:
        ::com::sun::star::uno::Any                                                                                                 m_aRequest      ;
        ::com::sun::star::uno::Sequence< ::com::sun::star::uno::Reference< ::com::sun::star::task::XInteractionContinuation > >    m_lContinuations;

};  // class RequestFilterSelect

}       //  namespace framework

#endif  // #define __FRAMEWORK_DISPATCH_INTERACTION_HXX_
