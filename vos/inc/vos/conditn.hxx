/*************************************************************************
 *
 *  $RCSfile: conditn.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:11 $
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


#ifndef _VOS_CONDITN_HXX_
#define _VOS_CONDITN_HXX_

#ifndef _OSL_CONDITN_H_
#   include <osl/conditn.h>
#endif
#ifndef _VOS_OBJECT_HXX_
#   include <vos/object.hxx>
#endif

#ifdef _USE_NAMESPACE
namespace vos
{
#endif


/** ICondition

    Interface for a thread-spanning condition. If a condition-object
    is created, its initial condition is False. You can check the
    condition nonblocking with "check()" or wait for it to become set
    with "wait()". The methods "set()" and "reset()" are used to change
    the conditions state.

    @author  Bernd Hofner
    @version 1.0
*/
class ICondition
{
public:

    ICondition() { }
    virtual ~ICondition() { }



    enum TResult
    {
        result_ok          = osl_cond_result_ok,
        result_error       = osl_cond_result_error,
        result_timeout     = osl_cond_result_timeout
    };

    /** set condition to True =>
        wait() will not block, check() returns True
    */
    virtual void SAL_CALL set()= 0;

    /** set condition to False =>
        wait() will block, check() returns False
    */
    virtual void SAL_CALL reset()= 0;

    /** Blocks if condition is not set<BR>
        If condition has been destroyed prematurely, wait() will
        return with False.
    */
    virtual TResult SAL_CALL wait(const TimeValue* pTimeout = 0)= 0;

    /** True: condition is set <BR>
        False: condition is not set <BR>
        does not block
    */
    virtual sal_Bool SAL_CALL check()= 0;
};


/** OCondition

    Implements the ICondition interface.

    @author  Bernd Hofner
    @version 1.0

*/
class OCondition : public OObject, public ICondition
{

    VOS_DECLARE_CLASSINFO(VOS_NAMESPACE(OCondition, vos));

public:

    /// initial state of condition is not set
    OCondition();
    virtual ~OCondition();

    /// set condition to True => wait() will not block, check() returns True
    virtual void SAL_CALL set();

    /// set condition to False => wait() will block, check() returns False
    virtual void SAL_CALL reset();

    /** Blocks if condition is not set<BR>
        If condition has been destroyed prematurely, wait() will
        return with False.
    */
    TResult SAL_CALL wait(const TimeValue* pTimeout = 0);

    /** True: condition is set <BR>
        False: condition is not set <BR>
        does not block
    */
    virtual sal_Bool SAL_CALL check();

protected:

    oslCondition    m_Condition;

};

#ifdef _USE_NAMESPACE
}
#endif

#endif  // _VOS_CONDITN_HXX_

