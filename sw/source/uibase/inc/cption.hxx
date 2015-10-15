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
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_CPTION_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_CPTION_HXX

#include <svx/stddlg.hxx>

#include <vcl/fixed.hxx>

#include <vcl/lstbox.hxx>

#include <vcl/edit.hxx>

#include <vcl/group.hxx>

#include <vcl/button.hxx>
#include <actctrl.hxx>

#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XHierarchicalNameAccess.hpp>
#include <com/sun/star/container/XNameAccess.hpp>
#include <com/sun/star/container/XContentEnumerationAccess.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XElementAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/container/XNamed.hpp>

class SwFieldMgr;
class SwView;

#include <wrtsh.hxx>
#include "optload.hxx"
#include "swlbox.hxx"

class TextFilterAutoConvert : public TextFilter
{
private:
    OUString m_sLastGoodText;
    OUString m_sNone;
public:
    TextFilterAutoConvert(const OUString &rNone)
        : m_sNone(rNone)
    {
    }
    virtual OUString filter(const OUString &rText) override;
};

class SwCaptionDialog : public SvxStandardDialog
{
    VclPtr<Edit>        m_pTextEdit;
    VclPtr<ComboBox>    m_pCategoryBox;
    OUString     m_sNone;
    TextFilterAutoConvert m_aTextFilter;
    VclPtr<FixedText>   m_pFormatText;
    VclPtr<ListBox>     m_pFormatBox;
    //#i61007# order of captions
    VclPtr<FixedText>   m_pNumberingSeparatorFT;
    VclPtr<Edit>        m_pNumberingSeparatorED;
    VclPtr<FixedText>   m_pSepText;
    VclPtr<Edit>        m_pSepEdit;
    VclPtr<FixedText>   m_pPosText;
    VclPtr<ListBox>     m_pPosBox;
    VclPtr<OKButton>    m_pOKButton;
    VclPtr<PushButton>  m_pAutoCaptionButton;
    VclPtr<PushButton>  m_pOptionButton;

    VclPtr<SwCaptionPreview> m_pPreview;

    SwView       &rView; // search per active, avoid View
    SwFieldMgr     *pMgr;      // pointer to save the include
    SelectionType eType;

    OUString     sCharacterStyle;
    OUString     sObjectName;
    bool         bCopyAttributes;
    bool        bOrderNumberingFirst; //#i61007# order of captions

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    xNameAccess;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed >         xNamed;

    DECL_LINK_TYPED(SelectHdl, ComboBox&, void);
    DECL_LINK_TYPED(SelectListBoxHdl, ListBox&, void);
    DECL_LINK_TYPED(ModifyHdl, Edit&, void);
    DECL_LINK_TYPED(OptionHdl, Button *, void);
    DECL_LINK_TYPED(CaptionHdl, Button *, void);

    virtual void Apply() override;

    void    DrawSample();
    void    ApplyCaptionOrder(); //#i61007# order of captions

    static OUString our_aSepTextSave; // Save caption separator text
public:
     SwCaptionDialog( vcl::Window *pParent, SwView &rV );
    virtual ~SwCaptionDialog();
    virtual void dispose() override;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
