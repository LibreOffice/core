/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: runnable.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 09:03:22 $
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

#ifndef _VOS_RUNNABLE_HXX_
#define _VOS_RUNNABLE_HXX_

#ifndef _VOS_TYPES_HXX_
#   include <vos/types.hxx>
#endif
#ifndef _VOS_REFERNCE_HXX_
#   include <vos/refernce.hxx>
#endif

#ifdef _USE_NAMESPACE
namespace vos
{
#endif


/** IRunnable

    The IRunnable-interface is to be understood as wrapper around
    your application-code, which allows it to be executed asynchronously.


    @author  Bernd Hofner
    @version 0.1
*/

class IRunnable
{
public:

    IRunnable() { }
    virtual ~IRunnable() { }


    /** overload to implement your functionality.
    */
    virtual void SAL_CALL run()= 0;

};


#ifdef _USE_NAMESPACE
}
#endif


#endif  // _VOS_RUNNABLE_HXX_

