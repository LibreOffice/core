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
#ifndef INCLUDED_CUI_SOURCE_INC_CONNECT_HXX
#define INCLUDED_CUI_SOURCE_INC_CONNECT_HXX

#include <svx/connctrl.hxx>
#include <sfx2/tabdlg.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/group.hxx>
#include <sfx2/basedlgs.hxx>

class SdrView;

/// Dialog for changing connectors.
class SvxConnectionPage : public SfxTabPage
{
private:
    ListBox*             m_pLbType;

    FixedText*           m_pFtLine1;
    MetricField*         m_pMtrFldLine1;
    FixedText*           m_pFtLine2;
    MetricField*         m_pMtrFldLine2;
    FixedText*           m_pFtLine3;
    MetricField*         m_pMtrFldLine3;

    MetricField*         m_pMtrFldHorz1;
    MetricField*         m_pMtrFldVert1;
    MetricField*         m_pMtrFldHorz2;
    MetricField*         m_pMtrFldVert2;

    SvxXConnectionPreview*   m_pCtlPreview;

    const SfxItemSet&   rOutAttrs;
    SfxItemSet          aAttrSet;
    const SdrView*      pView;
    SfxMapUnit          eUnit;

    void                FillTypeLB();

    DECL_LINK( ChangeAttrHdl_Impl, void * );

public:

    SvxConnectionPage( vcl::Window* pWindow, const SfxItemSet& rInAttrs );
    virtual ~SvxConnectionPage();

    static SfxTabPage*  Create( vcl::Window*, const SfxItemSet* );
    static  const sal_uInt16*     GetRanges();

    virtual bool        FillItemSet( SfxItemSet* ) SAL_OVERRIDE;
    virtual void        Reset( const SfxItemSet * ) SAL_OVERRIDE;

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }
    virtual void PageCreated(const SfxAllItemSet& aSet) SAL_OVERRIDE;
};

/* Derived from SfxSingleTabDialog, in order to be informed about
   virtual methods by the control. */
class SvxConnectionDialog : public SfxSingleTabDialog
{
public:
    SvxConnectionDialog( vcl::Window* pParent, const SfxItemSet& rAttr,
                       const SdrView* pView );
    virtual ~SvxConnectionDialog();
};

#endif // INCLUDED_CUI_SOURCE_INC_CONNECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
