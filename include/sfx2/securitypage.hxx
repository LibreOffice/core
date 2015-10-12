/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SFX2_SECURITYPAGE_HXX
#define INCLUDED_SFX2_SECURITYPAGE_HXX

#include <sfx2/tabdlg.hxx>
#include <sfx2/htmlmode.hxx>

#include <memory>



struct SfxSecurityPage_Impl;

class SfxSecurityPage : public SfxTabPage
{
    std::unique_ptr< SfxSecurityPage_Impl > m_pImpl;

protected:
    virtual bool            FillItemSet( SfxItemSet* ) override;
    virtual void            Reset( const SfxItemSet* ) override;

public:
    SfxSecurityPage( vcl::Window* pParent, const SfxItemSet& );
    static VclPtr<SfxTabPage> Create( vcl::Window* pParent, const SfxItemSet* );
};


#endif // INCLUDED_SFX2_SECURITYPAGE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
