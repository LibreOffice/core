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
#include "precompiled_svx.hxx"
#include <svx/svdobj.hxx>
// header for class OGuard
#include <osl/mutex.hxx>
// header for class Application
#include <vcl/svapp.hxx>

#include "unomlstr.hxx"

using namespace ::com::sun::star;

SvxUnoShapeModifyListener::SvxUnoShapeModifyListener( SdrObject* pObj ) throw()
{
    mpObj = pObj;
}

SvxUnoShapeModifyListener::~SvxUnoShapeModifyListener() throw()
{
}

// ::com::sun::star::util::XModifyListener
void SAL_CALL SvxUnoShapeModifyListener::modified(const lang::EventObject& ) throw( uno::RuntimeException )
{
    SolarMutexGuard aGuard;
    if( mpObj )
    {
        mpObj->SetChanged();
        mpObj->BroadcastObjectChange();
    }
}

// ::com::sun::star::lang::XEventListener
void SvxUnoShapeModifyListener::disposing(const lang::EventObject& ) throw( uno::RuntimeException )
{
    invalidate();
}

// internal
void SvxUnoShapeModifyListener::invalidate() throw()
{
    mpObj = NULL;
}


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
