/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */

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

SfxItemPool* SwSvxUnoDrawPool::getModelPool( bool /*bReadOnly*/ ) throw()
{
    if(m_pDoc)
    {

        
        
        
        
        
        m_pDoc->GetOrCreateDrawModel();
        return &(m_pDoc->GetAttrPool());
    }
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
