/*************************************************************************
 *
 *  $RCSfile: interaction.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 18:19:39 $
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

#ifndef __FRAMEWORK_DISPATCH_INTERACTION_HXX_
#define __FRAMEWORK_DISPATCH_INTERACTION_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONCONTINUATION_HPP_
#include <com/sun/star/task/XInteractionContinuation.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONABORT_HPP_
#include <com/sun/star/task/XInteractionAbort.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONAPPROVE_HPP_
#include <com/sun/star/task/XInteractionApprove.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONDISAPPROVE_HPP_
#include <com/sun/star/task/XInteractionDisapprove.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONRETRY_HPP_
#include <com/sun/star/task/XInteractionRetry.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_XINTERACTIONFILTERSELECT_HPP_
#include <com/sun/star/document/XInteractionFilterSelect.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_NOSUCHFILTERREQUEST_HPP_
#include <com/sun/star/document/NoSuchFilterRequest.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_AMBIGOUSFILTERREQUEST_HPP_
#include <com/sun/star/document/AmbigousFilterRequest.hpp>
#endif

#ifndef _COM_SUN_STAR_UNO_RUNTIMEEXCEPTION_HPP_
#include <com/sun/star/uno/RuntimeException.hpp>
#endif

//_________________________________________________________________________________________________________________
//  includes of other projects
//_________________________________________________________________________________________________________________

#ifndef _RTL_USTRING_HXX_
#include <rtl/ustring.hxx>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_UNO_SEQUENCE_HXX_
#include <com/sun/star/uno/Sequence.hxx>
#endif

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

#endif  // #define __FRAMEWORK_DISPATCH_INTERACTION_HXX_

}       //  namespace framework
