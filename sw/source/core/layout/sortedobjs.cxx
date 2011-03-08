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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"
#include <sortedobjs.hxx>
#include <sortedobjsimpl.hxx>

SwSortedObjs::SwSortedObjs()
    : mpImpl( new SwSortedObjsImpl )
{
}

SwSortedObjs::~SwSortedObjs()
{
    delete mpImpl;
}

sal_uInt32 SwSortedObjs::Count() const
{
    return mpImpl->Count();
}

SwAnchoredObject* SwSortedObjs::operator[]( sal_uInt32 _nIndex ) const
{
    return (*mpImpl)[ _nIndex ];
}

bool SwSortedObjs::Insert( SwAnchoredObject& _rAnchoredObj )
{
    return mpImpl->Insert( _rAnchoredObj );
}

bool SwSortedObjs::Remove( SwAnchoredObject& _rAnchoredObj )
{
    return mpImpl->Remove( _rAnchoredObj );
}

bool SwSortedObjs::Contains( const SwAnchoredObject& _rAnchoredObj ) const
{
    return mpImpl->Contains( _rAnchoredObj );
}

bool SwSortedObjs::Update( SwAnchoredObject& _rAnchoredObj )
{
    return mpImpl->Update( _rAnchoredObj );
}

sal_uInt32 SwSortedObjs::ListPosOf( const SwAnchoredObject& _rAnchoredObj ) const
{
    return mpImpl->ListPosOf( _rAnchoredObj );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
