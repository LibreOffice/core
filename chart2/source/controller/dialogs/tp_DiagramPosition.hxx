/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: tp_DiagramPosition.hxx,v $
 * $Revision: 1.7 $
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
#ifndef _CHART2_TP_DIAGRAMPOSITION_HXX
#define _CHART2_TP_DIAGRAMPOSITION_HXX

// header for class FormattedField
#include <svtools/fmtfield.hxx>
// header for FixedText
#include <vcl/fixed.hxx>
// header for CheckBox
#include <vcl/button.hxx>
// header for MetricField
#include <vcl/field.hxx>
// header for SvxTabPage and SvxRectCtl
#include <svx/dlgctrl.hxx>

//.............................................................................
namespace chart
{
//.............................................................................
class DiagramPositionTabPage : public SvxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

public:
    DiagramPositionTabPage( Window* pParent, const SfxItemSet& rInAttrs  );

    static SfxTabPage* Create( Window*, const SfxItemSet& );

    virtual BOOL FillItemSet( SfxItemSet& );
    virtual void Reset( const SfxItemSet & );

    virtual void ActivatePage( const SfxItemSet& rSet );
    virtual int  DeactivatePage( SfxItemSet* pSet );

    //SvxTabPage communication interface with SvxRectCtl
    virtual void PointChanged( Window* pWindow, RECT_POINT eRP );

    void         Construct();

    virtual void FillUserData();

private:
    //methods

    DECL_LINK( ChangeModeHdl, RadioButton * );
    DECL_LINK( ChangePosXHdl, void * );
    DECL_LINK( ChangePosYHdl, void * );
    DECL_LINK( ChangeWidthHdl, void * );
    DECL_LINK( ChangeHeightHdl, void * );
    DECL_LINK( ClickScaleHdl, void * );

    void SetRectIn100thmm( const Rectangle& rRect );
    Rectangle GetRectIn100thmm();
    long get1oothMMPosValue( MetricField& rField );
    long get1oothMMSizeValue( MetricField& rField );
    void set1oothMMPosValue( MetricField& rField, long n100thMM );
    void set1oothMMSizeValue( MetricField& rField, long n100thMM );
    sal_Int64 convert1oothMMValueToFieldUnit( MetricField& rField, long n100thMM );
    void ReleaseBorders();
    void SetMinMaxPosition();
    void UpdateControlStates();

private:

    //positioning mode
    FixedLine           m_aFlPosMode;

    RadioButton         m_aRbPosMode_Auto;
    RadioButton         m_aRbPosMode_Including;
    RadioButton         m_aRbPosMode_Excluding;

    // position
    FixedLine           maFlPosition;
    FixedText           maFtPosX;
    MetricField         maMtrPosX;
    FixedText           maFtPosY;
    MetricField         maMtrPosY;
    FixedText           maFtPosReference;
    SvxRectCtl          maCtlPos;

    // size
    FixedLine           maFlSize;
    FixedText           maFtWidth;
    MetricField         maMtrWidth;
    FixedText           maFtHeight;
    MetricField         maMtrHeight;
    CheckBox            maCbxScale;
    FixedText           maFtSizeReference;
    SvxRectCtl          maCtlSize;

private:
    Rectangle           m_aExcludingRect;
    Rectangle           m_aIncludingRect;
    Rectangle           m_aMaxRect;
    double              m_fUIScale;
    Rectangle           m_aCurrentRect;
    bool                m_bRectChangedByUser;

    SfxMapUnit          mePoolUnit;
    FieldUnit           meDlgUnit;

    double              mfScaleSizeX;
    double              mfScaleSizeY;
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif

