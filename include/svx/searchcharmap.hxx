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

#include <map>
#include <memory>

#include <sal/types.h>
#include <rtl/ref.hxx>
#include <svx/svxdllapi.h>
#include <svx/charmap.hxx>
#include <tools/gen.hxx>
#include <tools/link.hxx>
#include <vcl/ctrl.hxx>
#include <vcl/event.hxx>
#include <vcl/outdev.hxx>
#include <svx/ucsubset.hxx>
#include <vcl/metric.hxx>
#include <vcl/vclptr.hxx>
#include <vcl/window.hxx>
#include <vector>
#include <unordered_map>

namespace com { namespace sun { namespace star {
    namespace accessibility { class XAccessible; }
} } }

namespace vcl { class Font; }

#define COLUMN_COUNT    16
#define ROW_COUNT        8

class CommandEvent;
class ScrollBar;

class SVX_DLLPUBLIC SvxSearchCharSet : public SvxShowCharSet
{
public:
    SvxSearchCharSet(std::unique_ptr<weld::ScrolledWindow> pScrolledWindow, const VclPtr<VirtualDevice> &rDevice);
    virtual ~SvxSearchCharSet() override;

    virtual void                        RecalculateFont(vcl::RenderContext& rRenderContext) override;

    void                                SelectCharacter( const Subset* sub);
    virtual sal_UCS4                    GetSelectCharacter() const override;

    virtual svx::SvxShowCharSetItem*    ImplGetItem( int _nPos ) override;
    virtual int                         LastInView() const override;
    virtual void                        SelectIndex( int index, bool bFocus = false ) override;
    void                                AppendCharToList(sal_UCS4 cChar);
    void                                ClearPreviousData();

    virtual sal_Int32                   getMaxCharCount() const override;

private:
    sal_Int32     nCount;

    //index to char code mapping for the search
    //to uniquely identify each appended element
    std::unordered_map<sal_Int32, sal_UCS4> m_aItemList;
private:
    virtual void DrawChars_Impl(vcl::RenderContext& rRenderContext, int n1, int n2) override;
    virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    virtual bool KeyInput(const KeyEvent& rKEvt) override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
