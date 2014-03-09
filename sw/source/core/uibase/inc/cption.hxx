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
#ifndef INCLUDED_SW_SOURCE_UI_INC_CPTION_HXX
#define INCLUDED_SW_SOURCE_UI_INC_CPTION_HXX

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

class SwFldMgr;
class SwView;

#include <wrtsh.hxx>
#include "optload.hxx"
#include "swlbox.hxx"

class CategoryBox : public ComboBox
{
public:
    CategoryBox( Window* pParent, WinBits nStyle = 0 )
        : ComboBox( pParent, nStyle )
    {}

    virtual bool PreNotify( NotifyEvent& rNEvt );
};

class SwCaptionDialog : public SvxStandardDialog
{
    Edit*        m_pTextEdit;
    CategoryBox* m_pCategoryBox;
    FixedText*   m_pFormatText;
    ListBox*     m_pFormatBox;
    //#i61007# order of captions
    FixedText*   m_pNumberingSeparatorFT;
    Edit*        m_pNumberingSeparatorED;
    FixedText*   m_pSepText;
    Edit*        m_pSepEdit;
    FixedText*   m_pPosText;
    ListBox*     m_pPosBox;
    OKButton*    m_pOKButton;
    PushButton*  m_pAutoCaptionButton;
    PushButton*  m_pOptionButton;

    SwCaptionPreview* m_pPreview;

    OUString     m_sNone;

    SwView       &rView; // search per active, avoid ::com::sun::star::sdbcx::View
    SwFldMgr     *pMgr;      // pointer to save the include
    SelectionType eType;

    OUString     sCharacterStyle;
    OUString     sObjectName;
    bool         bCopyAttributes;
    bool        bOrderNumberingFirst; //#i61007# order of captions

    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNameAccess >    xNameAccess;
    ::com::sun::star::uno::Reference< ::com::sun::star::container::XNamed >         xNamed;

    DECL_LINK(SelectHdl, void *);
    DECL_LINK(ModifyHdl, void *);
    DECL_LINK(OptionHdl, Button *);
    DECL_LINK(CaptionHdl, void *);

    virtual void Apply();

    void    DrawSample();
    void    CheckButtonWidth();
    void    ApplyCaptionOrder(); //#i61007# order of captions

    static OUString our_aSepTextSave; // Save caption separator text
public:
     SwCaptionDialog( Window *pParent, SwView &rV );
    ~SwCaptionDialog();
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
