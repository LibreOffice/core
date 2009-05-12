/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: CppObject.hxx,v $
 * $Revision: 1.3 $
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
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
