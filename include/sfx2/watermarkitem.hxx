/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SFX2_WATERMARKITEM_HXX
#define INCLUDED_SFX2_WATERMARKITEM_HXX

#include <sfx2/dllapi.h>
#include <svl/poolitem.hxx>

class SFX2_DLLPUBLIC SfxWatermarkItem: public SfxPoolItem
{
public:
    static SfxPoolItem* CreateDefault();
    SfxWatermarkItem();
    virtual SfxPoolItem*    Clone( SfxItemPool *pPool = nullptr ) const override;
    virtual bool            operator==( const SfxPoolItem& ) const override;
    virtual bool            QueryValue( css::uno::Any& rVal, sal_uInt8 nMemberId = 0 ) const override;
    virtual bool            PutValue( const css::uno::Any& rVal, sal_uInt8 nMemberId ) override;

    const OUString          GetText() const { return m_aText; }
    void                    SetText(const OUString& aText) { m_aText = aText; }
    const OUString          GetFont() const { return m_aFont; }
    void                    SetFont(const OUString& aFont) { m_aFont = aFont; }
    sal_Int16               GetAngle() const { return m_nAngle; }
    void                    SetAngle(const sal_Int16 nAngle) { m_nAngle = nAngle; }
    sal_Int16               GetTransparency() const { return m_nTransparency; }
    void                    SetTransparency(const sal_Int16 nTransparency) { m_nTransparency = nTransparency; }
    sal_uInt32              GetColor() const { return m_nColor; }
    void                    SetColor(const sal_uInt32 nColor) { m_nColor = nColor; }

private:
    OUString                m_aText;
    OUString                m_aFont;
    sal_Int16               m_nAngle;
    sal_Int16               m_nTransparency;
    sal_uInt32              m_nColor;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
