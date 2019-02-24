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

#ifndef INCLUDED_CUI_SOURCE_INC_HLMARKWN_HXX
#define INCLUDED_CUI_SOURCE_INC_HLMARKWN_HXX

#include <com/sun/star/container/XNameAccess.hpp>
#include <vcl/button.hxx>
#include <vcl/floatwin.hxx>
#include <vcl/treelistbox.hxx>

#include "hlmarkwn_def.hxx"
class SvxHyperlinkTabPageBase;


//#                                                                      #
//# Tree-Window                                                          #
//#                                                                      #


class SvxHlinkDlgMarkWnd;

class SvxHlmarkTreeLBox : public SvTreeListBox
{
private:
    VclPtr<SvxHlinkDlgMarkWnd> mpParentWnd;

public:
    SvxHlmarkTreeLBox(vcl::Window* pParent, WinBits nStyle);
    virtual ~SvxHlmarkTreeLBox() override;
    virtual void dispose() override;

    void SetParentWnd(SvxHlinkDlgMarkWnd* pParent)
    {
        mpParentWnd = pParent;
    }

    virtual void Paint( vcl::RenderContext& rRenderContext, const ::tools::Rectangle& rRect ) override;
    virtual Size GetOptimalSize() const override;
};


//#                                                                      #
//# Window-Class                                                         #
//#                                                                      #
class SvxHlinkDlgMarkWnd : public FloatingWindow //FloatingWindow
{
private:
    friend class SvxHlmarkTreeLBox;

    VclPtr<PushButton>       mpBtApply;
    VclPtr<PushButton>       mpBtClose;
    VclPtr<SvxHlmarkTreeLBox>  mpLbTree;

    bool            mbUserMoved;

    VclPtr<SvxHyperlinkTabPageBase> mpParent;

    sal_uInt16          mnError;

protected:
    bool RefreshFromDoc( const OUString& aURL );
    void RestoreLastSelection();

    SvTreeListEntry* FindEntry(const OUString& aStrName);
    void ClearTree();
    int FillTree( const css::uno::Reference< css::container::XNameAccess >& xLinks, SvTreeListEntry* pParentEntry =nullptr );

    virtual void Move () override;

    DECL_LINK( ClickApplyHdl_Impl, Button*, void );
    DECL_LINK( DoubleClickApplyHdl_Impl, SvTreeListBox*, bool );
    DECL_LINK( ClickCloseHdl_Impl, Button*, void );

public:
    SvxHlinkDlgMarkWnd (SvxHyperlinkTabPageBase *pParent);
    virtual ~SvxHlinkDlgMarkWnd() override;
    virtual void dispose() override;

    bool MoveTo ( Point aNewPos );
    void RefreshTree(const OUString& aStrURL);
    bool SelectEntry(const OUString& aStrMark);

    bool ConnectToDialog();

    sal_uInt16 SetError( sal_uInt16 nError);
};


#endif // INCLUDED_CUI_SOURCE_INC_HLMARKWN_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
