/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AsynchronousCall.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2007-04-03 16:14:24 $
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

#ifndef SD_ASYNCHRONOUS_CALL_HXX
#define SD_ASYNCHRONOUS_CALL_HXX

#include <vcl/timer.hxx>
#include <memory>
#include <boost/function.hpp>

namespace sd { namespace tools {


/** Store a function object and execute it asynchronous.

    The features of this class are:
    a) It provides a wrapper around a VCL Timer so that generic function
    objects can be used.
    b) When more than one function objects are posted to be executed later
    then the pending ones are erased and only the last one will actually be
    executed.

    Use this class like this:
    aInstanceOfAsynchronousCall.Post(
        ::boost::bind(
            ::std::mem_fun(&DrawViewShell::SwitchPage),
            pDrawViewShell,
            11));
*/
class AsynchronousCall
{
public:
    /** Create a new asynchronous call.  Each object of this class processes
        one (semantical) type of call.
    */
    AsynchronousCall (void);

    ~AsynchronousCall (void);

    /** Post a function object that is to be executed asynchronously.  When
        this method is called while the current function object has not bee
        executed then the later is destroyed and only the given function
        object will be executed.
        @param rFunction
            The function object that may be called asynchronously in the
            near future.
        @param nTimeoutInMilliseconds
            The timeout in milliseconds until the function object is
            executed.
    */
    typedef ::boost::function0<void> AsynchronousFunction;
    void Post (
        const AsynchronousFunction& rFunction,
        sal_uInt32 nTimeoutInMilliseconds=10);

private:
    Timer maTimer;
    /** The function object that will be executed when the TimerCallback
        function is called the next time.  This pointer may be NULL.
    */
    ::std::auto_ptr<AsynchronousFunction> mpFunction;
    DECL_LINK(TimerCallback,Timer*);
};


} } // end of namespace ::sd::tools

#endif
