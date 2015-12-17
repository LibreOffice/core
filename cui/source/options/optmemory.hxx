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
#ifndef INCLUDED_CUI_SOURCE_OPTIONS_OPTMEMORY_HXX
#define INCLUDED_CUI_SOURCE_OPTIONS_OPTMEMORY_HXX

#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <vcl/field.hxx>
#include <vcl/fixed.hxx>
#include <sfx2/tabdlg.hxx>
#include <svx/langbox.hxx>

// class OfaMiscTabPage --------------------------------------------------

class OfaMemoryOptionsPage : public SfxTabPage
{
    using SfxTabPage::DeactivatePage;
private:
    VclPtr<NumericField>       m_pNfGraphicCache;
    VclPtr<NumericField>       m_pNfGraphicObjectCache;
    VclPtr<TimeField>          m_pTfGraphicObjectTime;

    VclPtr<NumericField>       m_pNfOLECache;

    VclPtr<VclContainer>       m_pQuickStarterFrame;
    VclPtr<CheckBox>           m_pQuickLaunchCB;

    DECL_LINK_TYPED(GraphicCacheConfigHdl, SpinField&, void);
    DECL_LINK_TYPED(GraphicCacheLoseFocusHdl, Control&, void);

    sal_Int32 GetNfGraphicCacheVal() const; // returns # of Bytes
    inline void         SetNfGraphicCacheVal( long nSizeInBytes );

    long                GetNfGraphicObjectCacheVal() const;           // returns # of Bytes
    void                SetNfGraphicObjectCacheVal( long nSizeInBytes );
    inline void         SetNfGraphicObjectCacheMax( long nSizeInBytes );
    inline void         SetNfGraphicObjectCacheLast( long nSizeInBytes );
protected:
    virtual sfxpg       DeactivatePage( SfxItemSet* pSet = nullptr ) override;

public:
    OfaMemoryOptionsPage( vcl::Window* pParent, const SfxItemSet& rSet );
    virtual ~OfaMemoryOptionsPage();
    virtual void dispose() override;

    static VclPtr<SfxTabPage>  Create( vcl::Window* pParent, const SfxItemSet* rAttrSet );

    virtual bool        FillItemSet( SfxItemSet* rSet ) override;
    virtual void        Reset( const SfxItemSet* rSet ) override;
};


#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
