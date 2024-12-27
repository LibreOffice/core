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
#ifndef INCLUDED_SVX_SEARCHCHARMAP_HXX
#define INCLUDED_SVX_SEARCHCHARMAP_HXX

#include <memory>

#include <sal/types.h>
#include <svx/svxdllapi.h>
#include <svx/charmap.hxx>
#include <vcl/outdev.hxx>
#include <vcl/vclptr.hxx>
#include <unordered_map>

namespace weld { class ScrolledWindow; }

class Subset;

class SVX_DLLPUBLIC SvxSearchCharSet final : public SvxShowCharSet
{
public:
    SvxSearchCharSet(std::unique_ptr<weld::ScrolledWindow> pScrolledWindow, const VclPtr<VirtualDevice> &rDevice);
    virtual ~SvxSearchCharSet() override;

    virtual void                        RecalculateFont(vcl::RenderContext& rRenderContext) override;

    void                                SelectCharacter( const Subset* sub);
    virtual sal_UCS4                    GetSelectCharacter() const override;
    virtual sal_UCS4                    GetCharFromIndex(int index) const override;

    virtual svx::SvxShowCharSetItem*    ImplGetItem( int _nPos ) override;
    virtual int                         LastInView() const override;
    virtual void                        SelectIndex( int index, bool bFocus = false ) override;
    void                                AppendCharToList(sal_UCS4 cChar);
    void                                ClearPreviousData();
    void                                UpdateScrollRange();

    virtual sal_Int32                   getMaxCharCount() const override;

private:
    //index to char code mapping for the search
    //to uniquely identify each appended element
    std::unordered_map<sal_Int32, sal_UCS4> m_aItemList;
private:
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual bool KeyInput(const KeyEvent& rKEvt) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
