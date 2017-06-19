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
#ifndef INCLUDED_CUI_SOURCE_INC_CUICHARMAP_HXX
#define INCLUDED_CUI_SOURCE_INC_CUICHARMAP_HXX

#include <vcl/ctrl.hxx>
#include <vcl/metric.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <sfx2/basedlgs.hxx>
#include <svx/charmap.hxx>
#include <sfx2/charwin.hxx>

using namespace ::com::sun::star;
class SubsetMap;

#define CHARMAP_MAXLEN  32

namespace svx
{
    struct SvxShowCharSetItem;
}

class SvxShowText : public Control
{
public:
    SvxShowText(vcl::Window* pParent);

    void            SetFont( const vcl::Font& rFont );
    void            SetText( const OUString& rText ) override;
    void            SetCentered(bool bCenter) { mbCenter = bCenter; }

    virtual void    Resize() override;

    virtual Size    GetOptimalSize() const override;

protected:
    virtual void    Paint(vcl::RenderContext& rRenderContext, const ::tools::Rectangle&) override;

private:
    long            mnY;
    bool            mbCenter;
    vcl::Font       maFont;

};

/** The main purpose of this dialog is to enable the use of characters
    that are not easily accessible from the keyboard. */
class SvxCharacterMap : public SfxModalDialog
{
private:

    void            init();

    VclPtr<SvxShowCharSet> m_pShowSet;
    VclPtr<PushButton>     m_pOKBtn;
    VclPtr<FixedText>      m_pFontText;
    VclPtr<ListBox>        m_pFontLB;
    VclPtr<FixedText>      m_pSubsetText;
    VclPtr<ListBox>        m_pSubsetLB;
    VclPtr<SvxShowText>    m_pShowChar;
    VclPtr<Edit>           m_pHexCodeText;
    VclPtr<Edit>           m_pDecimalCodeText;
    VclPtr<SvxCharView>    m_pRecentCharView[16];
    vcl::Font       aFont;
    bool            bOne;
    const SubsetMap* pSubsetMap;

    std::deque<OUString> maRecentCharList;
    std::deque<OUString> maRecentCharFontList;

    uno::Reference< uno::XComponentContext > mxContext;

    enum class Radix : sal_Int16 {decimal = 10, hexadecimal=16};

    DECL_LINK(FontSelectHdl, ListBox&, void);
    DECL_LINK(SubsetSelectHdl, ListBox&, void);
    DECL_LINK(CharDoubleClickHdl, SvxShowCharSet*,void);
    DECL_LINK(CharSelectHdl, SvxShowCharSet*, void);
    DECL_LINK(CharHighlightHdl, SvxShowCharSet*, void);
    DECL_LINK(CharPreSelectHdl, SvxShowCharSet*, void);
    DECL_LINK(DecimalCodeChangeHdl, Edit&, void);
    DECL_LINK(HexCodeChangeHdl, Edit&, void);
    DECL_LINK(RecentClickHdl, SvxCharView*, void);
    DECL_LINK(InsertClickHdl, Button*, void);
    DECL_STATIC_LINK(SvxCharacterMap, LoseFocusHdl, Control&, void);

    static void fillAllSubsets(ListBox &rListBox);
    void selectCharByCode(Radix radix);

public:
                    SvxCharacterMap( vcl::Window* pParent, bool bOne=true, const SfxItemSet* pSet=nullptr );
    virtual         ~SvxCharacterMap() override;
    virtual short Execute() override;
    virtual void    dispose() override;

    void            DisableFontSelection();

    const vcl::Font&     GetCharFont() const { return aFont;}
    void            SetCharFont( const vcl::Font& rFont );

    void            SetChar( sal_UCS4 );
    sal_UCS4        GetChar() const;

    void            getRecentCharacterList(); //gets both recent char and recent char font list
    void            updateRecentCharacterList(const OUString& rChar, const OUString& rFont);

    void            updateRecentCharControl();
    void            insertCharToDoc(const OUString& sChar);
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
