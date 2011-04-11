/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
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

#ifndef _TOOLKIT_HELPER_TKRESMGR_HXX_
#define _TOOLKIT_HELPER_TKRESMGR_HXX_

#include <rtl/ustring.hxx>
#include <vcl/image.hxx>

class SimpleResMgr;
class ResMgr;

#define TK_RES_STRING(id)   TkResMgr::loadString(id)
#define TK_RES_IMAGE(id)    TkResMgr::loadImage(id)

// -----------------------------------------------------------------------------
// TkResMgr
// -----------------------------------------------------------------------------

class TkResMgr
{
    static SimpleResMgr* m_pSimpleResMgr;
    static ResMgr* m_pResMgr;

private:
    // no instantiation allowed
    TkResMgr() { }
    ~TkResMgr() { }

    // we'll instantiate one static member of the following class,
    // which in it's dtor ensures that m_pSimpleResMgr will be deleted
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

    // loads the image with the specified resource id
    static Image loadImage( sal_uInt16 nResId );
    static Image getImageFromURL( const ::rtl::OUString& i_rImageURL );
};


#endif // _TOOLKIT_HELPER_TKRESMGR_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
