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

#ifndef INCLUDED_VCL_FIXED_HXX
#define INCLUDED_VCL_FIXED_HXX

#include <vcl/dllapi.h>
#include <vcl/ctrl.hxx>
#include <vcl/image.hxx>

class VCL_DLLPUBLIC FixedText : public Control
{
private:
    sal_Int32 m_nMaxWidthChars;
    sal_Int32 m_nMinWidthChars;
    VclPtr<vcl::Window> m_pMnemonicWindow;

    using Control::ImplInitSettings;
    using Window::ImplInit;
    SAL_DLLPRIVATE void    ImplInit( vcl::Window* pParent, WinBits nStyle );
    SAL_DLLPRIVATE static WinBits ImplInitStyle( WinBits nStyle );
    SAL_DLLPRIVATE void    ImplDraw( OutputDevice* pDev, DrawFlags nDrawFlags,
                              const Point& rPos, const Size& rSize, bool bFillLayout = false ) const;
public:
    SAL_DLLPRIVATE static DrawTextFlags ImplGetTextStyle( WinBits nWinBits );
protected:
    virtual void    FillLayoutData() const override;
    virtual const vcl::Font&
                    GetCanonicalFont( const StyleSettings& _rStyle ) const override;
    virtual const Color&
                    GetCanonicalTextColor( const StyleSettings& _rStyle ) const override;

    virtual vcl::Window* getAccessibleRelationLabelFor() const override;

public:
    explicit        FixedText( vcl::Window* pParent, WinBits nStyle = 0 );
    virtual         ~FixedText() override;
    virtual void    dispose() override;

    virtual void    ApplySettings(vcl::RenderContext& rRenderContext) override;

    virtual void    Paint( vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect ) override;
    virtual void    Draw( OutputDevice* pDev, const Point& rPos, DrawFlags nFlags ) override;
    virtual void    Resize() override;
    virtual void    StateChanged( StateChangedType nType ) override;
    virtual void    DataChanged( const DataChangedEvent& rDCEvt ) override;

    void            setMaxWidthChars(sal_Int32 nWidth);
    void            setMinWidthChars(sal_Int32 nWidth);
    static Size     CalcMinimumTextSize(Control const* pControl, tools::Long nMaxWidth = 0x7fffffff);
    static Size     getTextDimensions(Control const *pControl, const OUString &rTxt, tools::Long nMaxWidth);
    Size            CalcMinimumSize(tools::Long nMaxWidth = 0x7fffffff) const;
    virtual Size    GetOptimalSize() const override;
    virtual bool    set_property(const OString &rKey, const OUString &rValue) override;
    void            set_mnemonic_widget(vcl::Window *pWindow);
    vcl::Window*    get_mnemonic_widget() const { return m_pMnemonicWindow; }
};

#endif // INCLUDED_VCL_FIXED_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
