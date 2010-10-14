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
#ifndef _SORTEDOBJSIMPL_HXX
#define _SORTEDOBJSIMPL_HXX

#include <vector>
#include <sal/types.h>
#include <fmtanchr.hxx>

class SwAnchoredObject;
class SwTxtFrm;

class SwSortedObjsImpl
{
    private:
        std::vector< SwAnchoredObject* > maSortedObjLst;

    public:
        SwSortedObjsImpl();
        ~SwSortedObjsImpl();

        sal_uInt32 Count() const;

        SwAnchoredObject* operator[]( sal_uInt32 _nIndex );

        bool Insert( SwAnchoredObject& _rAnchoredObj );

        bool Remove( SwAnchoredObject& _rAnchoredObj );

        bool Contains( const SwAnchoredObject& _rAnchoredObj ) const;

        bool Update( SwAnchoredObject& _rAnchoredObj );

        sal_uInt32 ListPosOf( const SwAnchoredObject& _rAnchoredObj ) const;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
