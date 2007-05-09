/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: CppObject.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-09 13:43:58 $
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

#ifndef INCLUDED_CppObject_hxx
#define INCLUDED_CppObject_hxx


#include "com/sun/star/uno/XComponentContext.hpp"


#include "callee.hxx"

namespace cssu = ::com::sun::star::uno;


class CppObject: public cssu::XInterface
{
public:
    static cssu::XInterface * s_create (Callee * pCallee);
    static void               s_release(cssu::XInterface       * pXInterface);
    static void      s_call   (cssu::XInterface       * pXInterface);

    virtual void SAL_CALL acquire() throw ();
    virtual void SAL_CALL release() throw ();

    virtual cssu::Any SAL_CALL queryInterface(cssu::Type const & rType ) throw (cssu::RuntimeException);

protected:
    oslInterlockedCount    m_nRef;

    explicit CppObject(Callee * pCallee) SAL_THROW((cssu::RuntimeException));
    virtual ~CppObject(void)          SAL_THROW((cssu::RuntimeException));

    Callee * m_pCallee;
};


#endif
