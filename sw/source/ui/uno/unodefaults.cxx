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


#include <unodefaults.hxx>
#include <svx/svdmodel.hxx>
#include <svx/unoprov.hxx>
#include <doc.hxx>

SwSvxUnoDrawPool::SwSvxUnoDrawPool( SwDoc* pDoc ) throw() :
    SvxUnoDrawPool(pDoc->GetDrawModel(), SVXUNO_SERVICEID_COM_SUN_STAR_DRAWING_DEFAULTS_WRITER),
    m_pDoc(pDoc)
{
}

SwSvxUnoDrawPool::~SwSvxUnoDrawPool() throw()
{
}

SfxItemPool* SwSvxUnoDrawPool::getModelPool( sal_Bool /*bReadOnly*/ ) throw()
{
    if(m_pDoc)
    {

        // DVO, OD 01.10.2003 #i18732# - return item pool of writer document;
        // it contains draw model item pool as secondary pool.
        //SdrModel* pModel = m_pDoc->MakeDrawModel();
        //return &pModel->GetItemPool();
        // --> OD 2005-08-08 #i52858# - method name changed
        m_pDoc->GetOrCreateDrawModel();
        // <--
        return &(m_pDoc->GetAttrPool());
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
