/*************************************************************************
 *
 *  $RCSfile: interaction.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: as $ $Date: 2001-11-08 11:57:56 $
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

#ifndef __FRAMEWORK_CLASSES_FILTERCACHE_HXX_
#include <classes/filtercache.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

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

#ifndef _COM_SUN_STAR_DOCUMENT_XINTERACTIONFILTERSELECT_HPP_
#include <com/sun/star/document/XInteractionFilterSelect.hpp>
#endif

#ifndef _COM_SUN_STAR_DOCUMENT_NOSUCHFILTERREQUEST_HPP_
#include <com/sun/star/document/NoSuchFilterRequest.hpp>
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
        virtual void SAL_CALL select() throw( css::uno::RuntimeException )
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

typedef ContinuationBase< css::task::XInteractionAbort > ContinuationAbort;

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

class ContinuationFilterSelect : public ContinuationBase< css::document::XInteractionFilterSelect >
{
    // c++ interface
    public:

        //---------------------------------------------------------------------------------------------------------
        // initialize continuation with right start values
        //---------------------------------------------------------------------------------------------------------
        ContinuationFilterSelect()
            : m_sFilter( ::rtl::OUString() )
        {
        }

    // uno interface
    public:

        //---------------------------------------------------------------------------------------------------------
        // handler should use it after selection to set user specified filter for transport
        //---------------------------------------------------------------------------------------------------------
        virtual void SAL_CALL setFilter( const ::rtl::OUString& sFilter ) throw( css::uno::RuntimeException )
        {
            m_sFilter = sFilter;
        }

        //---------------------------------------------------------------------------------------------------------
        // read access to transported filter
        //---------------------------------------------------------------------------------------------------------
        virtual ::rtl::OUString SAL_CALL getFilter() throw( css::uno::RuntimeException )
        {
            return m_sFilter;
        }

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

class RequestFilterSelect : public ::cppu::WeakImplHelper1< css::task::XInteractionRequest >
{
    // c++ interface
    public:

        //---------------------------------------------------------------------------------------------------------
        // initialize instance with all neccessary informations
        // We use it without any further checks on our member then ...!
        //---------------------------------------------------------------------------------------------------------
        RequestFilterSelect( const ::rtl::OUString& sURL )
        {
            css::document::NoSuchFilterRequest aFilterRequest( ::rtl::OUString(), css::uno::Reference< css::uno::XInterface >(), sURL );
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
        sal_Bool isAbort() const
        {
            return m_pAbort->isSelected();
        }

        //---------------------------------------------------------------------------------------------------------
        // return user selected filter
        // Return value valid for non aborted interaction only. Please check "isAbort()" before you call these ony!
        //---------------------------------------------------------------------------------------------------------
        ::rtl::OUString getFilter() const
        {
            return m_pFilter->getFilter();
        }

    // uno interface
    public:

        //---------------------------------------------------------------------------------------------------------
        // handler call it to get type of request
        // Is hard coded to "please select filter" here. see ctor for further informations.
        //---------------------------------------------------------------------------------------------------------
        virtual css::uno::Any SAL_CALL getRequest() throw( css::uno::RuntimeException )
        {
            return m_aRequest;
        }

        //---------------------------------------------------------------------------------------------------------
        // handler call it to get possible continuations
        // We support "abort/select_filter" only here.
        // After interaction we support read access on these continuations on our c++ interface to
        // return user decision.
        //---------------------------------------------------------------------------------------------------------
        virtual css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > > SAL_CALL getContinuations() throw( css::uno::RuntimeException )
        {
            return m_lContinuations;
        }

    // member
    private:

        css::uno::Any                                                                       m_aRequest      ;
        css::uno::Sequence< css::uno::Reference< css::task::XInteractionContinuation > >    m_lContinuations;
        ContinuationAbort*                                                                  m_pAbort        ;
        ContinuationFilterSelect*                                                           m_pFilter       ;

};  // class RequestFilterSelect

#endif  // #define __FRAMEWORK_DISPATCH_INTERACTION_HXX_

}       //  namespace framework
