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
#ifndef _SVX_LABDLG_HXX
#define _SVX_LABDLG_HXX

#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <svtools/valueset.hxx>
#include <sfx2/tabdlg.hxx>
class SdrView;

// class SvxCaptionTabPage -----------------------------------------------

const sal_uInt16 CAPTYPE_BITMAPS_COUNT = 3;

class SvxCaptionTabPage : public SfxTabPage
{
private:
    ValueSet        aCT_CAPTTYPE;
    FixedText       aFT_ABSTAND;
    MetricField     aMF_ABSTAND;
    FixedText       aFT_WINKEL;
    ListBox         aLB_WINKEL;
    FixedText       aFT_ANSATZ;
    ListBox         aLB_ANSATZ;
    FixedText       aFT_UM;
    MetricField     aMF_ANSATZ;
    FixedText       aFT_ANSATZ_REL;
    ListBox         aLB_ANSATZ_REL;
    FixedText       aFT_LAENGE;
    MetricField     aMF_LAENGE;
    CheckBox        aCB_LAENGE;

    Image*          mpBmpCapTypes[CAPTYPE_BITMAPS_COUNT];

    String          aStrHorzList;
    String          aStrVertList;

    short           nCaptionType;
    sal_Bool            bFixedAngle;
    sal_Int32           nFixedAngle;
    sal_Int32           nGap;
    short           nEscDir;
    sal_Bool            bEscRel;
    sal_Int32           nEscAbs;
    sal_Int32           nEscRel;
    sal_Int32           nLineLen;
    sal_Bool            bFitLineLen;

    sal_uInt16          nAnsatzRelPos;
    sal_uInt16          nAnsatzTypePos;
    sal_uInt16          nWinkelTypePos;

#ifdef _SVX_LABDLG_CXX
    void            SetupAnsatz_Impl( sal_uInt16 nType );
    void            SetupType_Impl( sal_uInt16 nType );
    DECL_LINK( AnsatzSelectHdl_Impl, ListBox * );
    DECL_LINK( AnsatzRelSelectHdl_Impl, ListBox * );
    DECL_LINK( LineOptHdl_Impl, Button * );
    DECL_LINK( SelectCaptTypeHdl_Impl, void * );
#endif

    const SfxItemSet&   rOutAttrs;
    const SdrView*      pView;

public:
    SvxCaptionTabPage( Window* pParent, const SfxItemSet& rInAttrs  );
    virtual ~SvxCaptionTabPage();

    static SfxTabPage*  Create( Window*, const SfxItemSet& );
    static sal_uInt16*      GetRanges();

    virtual sal_Bool        FillItemSet( SfxItemSet& );
    virtual void        Reset( const SfxItemSet & );
    void                Construct();
    void                SetView( const SdrView* pSdrView )
                            { pView = pSdrView; }

    virtual void DataChanged( const DataChangedEvent& rDCEvt );
    void FillValueSet();
};

// class SvxCaptionTabDialog ---------------------------------------------

class SvxCaptionTabDialog : public SfxTabDialog
{
private:
    const SdrView*      pView;
    sal_uInt16              nAnchorCtrls;

    Link                aValidateLink;

    virtual void        PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

public:

            SvxCaptionTabDialog(Window* pParent, const SdrView* pView,
                                    sal_uInt16 nAnchorTypes = 0 );

            ~SvxCaptionTabDialog();

            //link for the Writer to validate positions
            void SetValidateFramePosLink( const Link& rLink );
};


#endif //_SVX_LABDLG_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
