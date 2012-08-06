/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
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
