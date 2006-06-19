/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: stillinteraction.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: hr $ $Date: 2006-06-19 10:55:51 $
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

#ifndef __FRAMEWORK_INTERACTION_STILLINTERACTION_HXX_
#define __FRAMEWORK_INTERACTION_STILLINTERACTION_HXX_

//_________________________________________________________________________________________________________________
//  my own includes
//_________________________________________________________________________________________________________________

#ifndef __FRAMEWORK_THREADHELP_THREADHELPBASE_HXX_
#include <threadhelp/threadhelpbase.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XINTERFACE_HXX_
#include <macros/xinterface.hxx>
#endif

#ifndef __FRAMEWORK_MACROS_XTYPEPROVIDER_HXX_
#include <macros/xtypeprovider.hxx>
#endif

#ifndef __FRAMEWORK_GENERAL_H_
#include <general.h>
#endif

//_________________________________________________________________________________________________________________
//  interface includes
//_________________________________________________________________________________________________________________

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONHANDLER_HPP_
#include <com/sun/star/task/XInteractionHandler.hpp>
#endif

#ifndef _COM_SUN_STAR_TASK_XINTERACTIONREQUEST_HPP_
#include <com/sun/star/task/XInteractionRequest.hpp>
#endif

//_________________________________________________________________________________________________________________
//  other includes
//_________________________________________________________________________________________________________________

#ifndef _CPPUHELPER_WEAK_HXX_
#include <cppuhelper/weak.hxx>
#endif

//_________________________________________________________________________________________________________________
//  namespace
//_________________________________________________________________________________________________________________

namespace framework{

//_________________________________________________________________________________________________________________
//  exported const
//_________________________________________________________________________________________________________________

//_________________________________________________________________________________________________________________
//  exported definitions
//_________________________________________________________________________________________________________________

/**
    @short      handle interactions non visible
    @descr      Sometimes it's neccessary to use a non visible interaction handler.
                He can't do anything, which a visible one can handle.
                But it can be used to intercept problems e.g. during loading of documents.

                In current implementation we solve conflicts for following situations only:
                    - AmbigousFilterRequest
                    - InteractiveIOException
                    - InteractiveAugmentedIOException
                All other requests will be aborted.

    @modified   12.07.2002 14:06
    @by         Andreas Schlüns
 */
class StillInteraction : public  css::lang::XTypeProvider
                       , public  css::task::XInteractionHandler
                       , private ThreadHelpBase
                       , public  ::cppu::OWeakObject
{
    //_____________________________________
    // member
    private:

        /// in case an unknown interaction was aborted - we save it for our external user!
        css::uno::Any m_aRequest;

    //_____________________________________
    // uno interface
    public:

        // XInterface, XTypeProvider
        FWK_DECLARE_XINTERFACE
        FWK_DECLARE_XTYPEPROVIDER

        //_________________________________
        /**
            @interface  XInteractionHandler
            @short      called from outside to handle a problem
            @descr      The only interaction we can handle here is to
                        decide which of two ambigous filters should be realy used.
                        We use the user selected one every time.
                        All other request will be aborted and can break the code,
                        which use this interaction handler.

                        But you can use another method of this class to check for
                        some special interactions too: IO Exceptions
                        May a ComponentLoader needs that to throw suitable exception
                        on his own interface.

            @threadsafe yes
        */
        virtual void SAL_CALL handle( const css::uno::Reference< css::task::XInteractionRequest >& xRequest ) throw( css::uno::RuntimeException );

    //_____________________________________
    // c++ interface
    public:

        //_________________________________
        /**
            @short      ctor to guarantee right initialized instances of this class
            @descr      -

            @threadsafe not neccessary
        */
        StillInteraction();

        //_________________________________
        /**
            @short      return the handled interaction request
            @descr      We saved any getted interaction request internaly.
                        May the outside user of this class is interessted
                        on that. Especialy we gotted an unknown interaction
                        and aborted it hard.

            @return     [com.sun.star.uno.Any]
                            the packed interaction request
                            Can be empty if no interaction was used!

            @threadsafe yes
        */
        css::uno::Any getRequest() const;

        //_________________________________
        /**
            @short      returns information if interaction was used
            @descr      It can be usefully to know the reason for a failed operation.

            @return     [boolean]
                            <TRUE/> for used interaction
                            <FALSE/> otherwhise

            @threadsafe yes
        */
        sal_Bool wasUsed() const;
};

} // namespace framework

#endif // #ifndef __FRAMEWORK_INTERACTION_STILLINTERACTION_HXX_
