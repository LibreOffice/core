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
    FixedText           aFtType;
    ListBox             aLbType;

    FixedLine           aFlDelta;
    FixedText           aFtLine1;
    MetricField         aMtrFldLine1;
    FixedText           aFtLine2;
    MetricField         aMtrFldLine2;
    FixedText           aFtLine3;
    MetricField         aMtrFldLine3;

    FixedLine           aFlDistance;
    FixedText           aFtHorz1;
    MetricField         aMtrFldHorz1;
    FixedText           aFtVert1;
    MetricField         aMtrFldVert1;
    FixedText           aFtHorz2;
    MetricField         aMtrFldHorz2;
    FixedText           aFtVert2;
    MetricField         aMtrFldVert2;

    SvxXConnectionPreview   aCtlPreview;

    const SfxItemSet&   rOutAttrs;
    SfxItemSet          aAttrSet;
    const SdrView*      pView;
    SfxMapUnit          eUnit;

#ifdef _SVX_CONNECT_CXX
    void                FillTypeLB();

                        DECL_LINK( ChangeAttrHdl_Impl, void * );
#endif

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

/* Derived from SfxSingleTabDialog, in order to be informed about
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
