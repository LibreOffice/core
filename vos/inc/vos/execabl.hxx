/*************************************************************************
 *
 *  $RCSfile: execabl.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:12 $
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


#ifndef _VOS_EXECABL_HXX_
#define _VOS_EXECABL_HXX_

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


/** IExecutable

    The IExecutable-interface is to be understood as wrapper around
    your application-code, which allows it to be executed asynchronously.

    Wether you want real asynchronous behaviour or just pseudo-multitasking
    depends on which kind of execution-service you use to execute your executable.

    (E.g. Threadpool/Fiberpool)

    @author  Bernd Hofner
    @version 0.1
*/

class IExecutable : public NAMESPACE_VOS(IReference)
{
public:

    /** Overload to implement your functionality.
        Return True, if you want "execute()" to be called again.
    */
    virtual sal_Bool SAL_CALL execute()= 0;


    /** If you use blocking calls within your execute-function,
        you should provide here a means to unblock cleanly.
        @Return False if you are not able to unblock the
        thread.

    */
    virtual sal_Bool SAL_CALL unblock()= 0;

    /**
        STL demands this operators if you want to place
        IExecutables per value in collections.
    */
    virtual sal_Bool SAL_CALL operator<(const IExecutable&) const= 0;
    virtual sal_Bool SAL_CALL operator>(const IExecutable&) const= 0;
    virtual sal_Bool SAL_CALL operator==(const IExecutable&) const= 0;
    virtual sal_Bool SAL_CALL operator!=(const IExecutable&) const= 0;
};


/** OExecutable
    added default impl. of IReferenceCounter
*/
class OExecutable : public NAMESPACE_VOS(IExecutable),
                    public NAMESPACE_VOS(OReference)

{
public:

    virtual ~OExecutable()
    {
    }

    /*
        Since the dominance of the OReferenceCounter impl.
        of the IReferenceCounter is not granted, delegate
        the methods to this branch of the diamond-shaped
        inheritance tree.
    */

    virtual RefCount SAL_CALL acquire()
    {
        return OReference::acquire();
    }
    virtual RefCount SAL_CALL release()
    {
        return OReference::release();
    }
    virtual RefCount SAL_CALL referenced() const
    {
        return OReference::referenced();
    }


    /** Default implementation of unblock does nothing.
    */
    virtual sal_Bool SAL_CALL unblock() { return sal_True; }


    /**
        STL demands this operators if you want to place
        IExecutables per value in collections.
        Use a default implementation of the comparison-operators
        here without a correct semantic.
    */
    virtual sal_Bool SAL_CALL operator<(const IExecutable&) const
    {
        return sal_False;
    }

    virtual sal_Bool SAL_CALL operator>(const IExecutable&) const
    {
        return sal_False;
    }

    virtual sal_Bool SAL_CALL operator==(const IExecutable&) const
    {
        return sal_True;
    }

    virtual sal_Bool SAL_CALL operator!=(const IExecutable&) const
    {
        return sal_False;
    }
};


#ifdef _USE_NAMESPACE
}
#endif


#endif  // _VOS_EXECABL_HXX_

