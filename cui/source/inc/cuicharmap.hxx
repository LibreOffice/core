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

class SubsetMap;

#define CHARMAP_MAXLEN  32

namespace svx
{
    struct SvxShowCharSetItem;
}

class SvxShowText : public Control
{
public:
                    SvxShowText( Window* pParent,
                                 bool bCenter = false );
                    virtual ~SvxShowText();

    void            SetFont( const Font& rFont );
    void            SetText( const OUString& rText ) SAL_OVERRIDE;
    void            SetCentered(bool bCenter) { mbCenter = bCenter; }

    virtual void    Resize() SAL_OVERRIDE;

    virtual Size    GetOptimalSize() const SAL_OVERRIDE;

protected:
    virtual void    Paint( const Rectangle& ) SAL_OVERRIDE;

private:
    long            mnY;
    bool            mbCenter;

};

/** The main purpose of this dialog is to enable the use of characters
    that are not easily accessible from the keyboard. */
class SvxCharacterMap : public SfxModalDialog
{
private:

    void            init();

    SvxShowCharSet* m_pShowSet;
    SvxShowText*    m_pShowText;
    OKButton*       m_pOKBtn;
    PushButton*     m_pDeleteLastBtn;
    PushButton*     m_pDeleteBtn;
    FixedText*      m_pFontText;
    ListBox*        m_pFontLB;
    FixedText*      m_pSubsetText;
    ListBox*        m_pSubsetLB;
    FixedText*      m_pSymbolText;
    SvxShowText*    m_pShowChar;
    FixedText*      m_pCharCodeText;
    Font            aFont;
    bool        bOne;
    const SubsetMap* pSubsetMap;

    DECL_LINK(OKHdl, void *);
    DECL_LINK(FontSelectHdl, void *);
    DECL_LINK(SubsetSelectHdl, void *);
    DECL_LINK(CharDoubleClickHdl, void *);
    DECL_LINK(CharSelectHdl, void *);
    DECL_LINK(CharHighlightHdl, void *);
    DECL_LINK(CharPreSelectHdl, void *);
    DECL_LINK(DeleteLastHdl, void *);
    DECL_LINK(DeleteHdl, void *);

    void fillAllSubsets(ListBox &rListBox);

public:
                    SvxCharacterMap( Window* pParent, bool bOne=true, const SfxItemSet* pSet=0 );
                    virtual ~SvxCharacterMap();

    void            DisableFontSelection();

    const Font&     GetCharFont() const { return aFont;}
    void            SetCharFont( const Font& rFont );

    void            SetChar( sal_UCS4 );
    sal_UCS4        GetChar() const;

    OUString        GetCharacters() const;

    virtual short   Execute() SAL_OVERRIDE;
};

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
