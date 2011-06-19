/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
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

#ifndef SVT_REFBASE_HXX
#define SVT_REFBASE_HXX

#include "svtools/svtdllapi.h"

#include <rtl/ref.hxx>

//........................................................................
namespace svt
{
//........................................................................

    //====================================================================
    //= RefBase
    //====================================================================
    class SVT_DLLPUBLIC RefBase : public ::rtl::IReference
    {
    protected:
        RefBase()
            :m_refCount( 0 )
        {
        }

        virtual ~RefBase()
        {
        }

        virtual oslInterlockedCount SAL_CALL acquire();
        virtual oslInterlockedCount SAL_CALL release();

    private:
        oslInterlockedCount m_refCount;
    };

#define DECLARE_IREFERENCE()    \
    virtual oslInterlockedCount SAL_CALL acquire(); \
    virtual oslInterlockedCount SAL_CALL release();


#define IMPLEMENT_IREFERENCE( classname )   \
    oslInterlockedCount classname::acquire()    \
    {   \
        return RefBase::acquire();  \
    }   \
    oslInterlockedCount classname::release()    \
    {   \
        return RefBase::release();  \
    }

//........................................................................
} // namespace svt
//........................................................................

#endif // SVT_REFBASE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
