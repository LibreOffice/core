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
#ifndef _SVX_CONNECT_HXX
#define _SVX_CONNECT_HXX

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

    SvxConnectionPage( Window* pWindow, const SfxItemSet& rInAttrs );
    ~SvxConnectionPage();

    static SfxTabPage*  Create( Window*, const SfxItemSet& );
    static  sal_uInt16*     GetRanges();

    virtual sal_Bool        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet & );

    void         Construct();
    void         SetView( const SdrView* pSdrView ) { pView = pSdrView; }
    virtual void PageCreated (SfxAllItemSet aSet);
};

/* Derived from SfxNoLayoutSingleTabDialog, in order to be informed about
   virtual methods by the control. */
class SvxConnectionDialog : public SfxSingleTabDialog
{
public:
    SvxConnectionDialog( Window* pParent, const SfxItemSet& rAttr,
                       const SdrView* pView );
    ~SvxConnectionDialog();
};

#endif // _SVX_CONNECT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
