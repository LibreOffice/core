/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: accresmgr.hxx,v $
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

#ifndef ACCESSIBILITY_HELPER_TKARESMGR_HXX
#define ACCESSIBILITY_HELPER_TKARESMGR_HXX

#ifndef _RTL_USTRING_HXX
#include <rtl/ustring.hxx>
#endif

class SimpleResMgr;

#define TK_RES_STRING(id) TkResMgr::loadString(id)

// -----------------------------------------------------------------------------
// TkResMgr
// -----------------------------------------------------------------------------

class TkResMgr
{
    static SimpleResMgr* m_pImpl;

private:
    // no instantiation allowed
    TkResMgr() { }
    ~TkResMgr() { }

    // we'll instantiate one static member of the following class,
    // which in it's dtor ensures that m_pImpl will be deleted
    class EnsureDelete
    {
    public:
        EnsureDelete() { }
        ~EnsureDelete();
    };
    friend class EnsureDelete;

protected:
    static void ensureImplExists();

public:
    // loads the string with the specified resource id
    static ::rtl::OUString loadString( sal_uInt16 nResId );
};


#endif // ACCESSIBILITY_HELPER_TKARESMGR_HXX

