/*************************************************************************
 *
 *  $RCSfile: enumhelper.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: fs $ $Date: 2000-09-29 11:28:15 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
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

#ifndef _COMPHELPER_ENUMHELPER_HXX_
#define _COMPHELPER_ENUMHELPER_HXX_

#ifndef _COM_SUN_STAR_CONTAINER_XNAMEACCESS_HPP_
#include <com/sun/star/container/XNameAccess.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XENUMERATION_HPP_
#include <com/sun/star/container/XEnumeration.hpp>
#endif
#ifndef _COM_SUN_STAR_CONTAINER_XINDEXACCESS_HPP_
#include <com/sun/star/container/XIndexAccess.hpp>
#endif

#ifndef _CPPUHELPER_IMPLBASE1_HXX_
#include <cppuhelper/implbase1.hxx>
#endif

//.........................................................................
namespace comphelper
{
//.........................................................................

    namespace starcontainer     = ::com::sun::star::container;
    namespace staruno           = ::com::sun::star::uno;
    namespace starlang          = ::com::sun::star::lang;

//==================================================================
//= OEnumerationByName
//==================================================================
/** provides an <type scope="com.sun.star.container">XEnumeration</type> access based
    on an object implementing the <type scope="com.sun.star.container">XNameAccess</type> interface
*/
class OEnumerationByName : public ::cppu::WeakImplHelper1<starcontainer::XEnumeration>
{
    staruno::Sequence< ::rtl::OUString>                 m_aNames;
    sal_Int32                                           m_nPos;
    staruno::Reference<starcontainer::XNameAccess>      m_xAccess;

public:
    OEnumerationByName(const staruno::Reference<starcontainer::XNameAccess>& _rxAccess)
        :m_aNames(_rxAccess->getElementNames())
        ,m_nPos(0)
        ,m_xAccess(_rxAccess)
    {
    }

    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw(staruno::RuntimeException);
    virtual staruno::Any SAL_CALL nextElement(  )
        throw(starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException);
};

//==================================================================
//= OEnumerationByIndex
//==================================================================
/** provides an <type scope="com.sun.star.container">XEnumeration</type> access based
    on an object implementing the <type scope="com.sun.star.container">XNameAccess</type> interface
*/
class OEnumerationByIndex : public ::cppu::WeakImplHelper1<starcontainer::XIndexAccess>
{
    sal_Int32                                       m_nPos;
    staruno::Reference<starcontainer::XIndexAccess> m_xAccess;

public:
    OEnumerationByIndex(const staruno::Reference<starcontainer::XIndexAccess>& _rxAccess)
        :m_xAccess(_rxAccess)
        ,m_nPos(0)
    {
    }

    virtual sal_Bool SAL_CALL hasMoreElements(  ) throw(staruno::RuntimeException);
    virtual staruno::Any SAL_CALL nextElement(  )
        throw(starcontainer::NoSuchElementException, starlang::WrappedTargetException, staruno::RuntimeException);
};

//.........................................................................
}
//... namespace comphelper .......................................................

#endif // _COMPHELPER_ENUMHELPER_HXX_


/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *
 *  Revision 1.0 29.09.00 08:49:14  fs
 ************************************************************************/

