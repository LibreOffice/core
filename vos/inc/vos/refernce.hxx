/*************************************************************************
 *
 *  $RCSfile: refernce.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 15:18:13 $
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

#ifndef _VOS_REFERNCE_HXX_
#define _VOS_REFERNCE_HXX_

#ifndef _VOS_TYPES_HXX_
#   include <vos/types.hxx>
#endif
#ifndef _OSL_INTERLCK_H_
#   include <osl/interlck.h>
#endif
#ifndef _VOS_OBJECT_HXX_
#   include <vos/object.hxx>
#endif

#ifdef _USE_NAMESPACE
namespace vos
{
#endif

/** Interface for refernce-counting
*/
class IReference
{
public:

    IReference() { }
    virtual ~IReference() { }

    typedef oslInterlockedCount RefCount;

    virtual RefCount SAL_CALL acquire()=0;
    virtual RefCount SAL_CALL release()=0;

    virtual RefCount SAL_CALL referenced() const=0;
};

class ORefCount
{
public:
    typedef IReference::RefCount RefCount;

    ORefCount() { m_RefCount = 0; }
    ORefCount(RefCount n) { m_RefCount = n; }
    virtual ~ORefCount();

    RefCount SAL_CALL acquire() { return (osl_incrementInterlockedCount(&m_RefCount)); };
    RefCount SAL_CALL release() { return (osl_decrementInterlockedCount(&m_RefCount)); };

    RefCount SAL_CALL operator++()   { return acquire(); }
    // don't implement the postfix operator, it won't function this way!

    RefCount SAL_CALL operator--()   { return release(); }
    // don't implement the postfix operator, it won't function this way!

    RefCount SAL_CALL referenced() const
        { return (m_RefCount); }

protected:
    RefCount m_RefCount;

private:
    // disable copy/assignment
    ORefCount(const ORefCount&);
    ORefCount& SAL_CALL operator= (const ORefCount&);
};

class OReference : public NAMESPACE_VOS(IReference)
{
public:
    OReference();
    virtual ~OReference();

    virtual RefCount SAL_CALL acquire();
    virtual RefCount SAL_CALL release();

    virtual RefCount SAL_CALL referenced() const
        { return (m_RefCount.referenced()); }

protected:
    ORefCount m_RefCount;

private:
    // disable copy/assignment
    OReference(const OReference&);
    OReference& SAL_CALL operator= (const OReference&);
};

#ifdef _USE_NAMESPACE
}
#endif

#endif  // _VOS_REFERNCE_HXX_

