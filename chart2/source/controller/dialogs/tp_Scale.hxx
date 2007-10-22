/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: tp_Scale.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 16:48:24 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _CHART2_TP_SCALES_HXX
#define _CHART2_TP_SCALES_HXX

// header for SfxTabPage
#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif
// header for class FormattedField
#ifndef _FMTFIELD_HXX_
#include <svtools/fmtfield.hxx>
#endif
// header for FixedText
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
// header for CheckBox
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
// header for MetricField
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
//class SvNumberFormatter;

//.............................................................................
namespace chart
{
//.............................................................................

class ScaleTabPage : public SfxTabPage
{
private:
    FixedLine           aFlScale;

    FixedText           aTxtMin;
    FormattedField      aFmtFldMin;
    CheckBox            aCbxAutoMin;

    FixedText           aTxtMax;
    FormattedField      aFmtFldMax;
    CheckBox            aCbxAutoMax;

    FixedText           aTxtMain;
    FormattedField      aFmtFldStepMain;
    CheckBox            aCbxAutoStepMain;

    FixedText           aTxtHelp;
    MetricField         aMtStepHelp;
    CheckBox            aCbxAutoStepHelp;

    FixedText           aTxtOrigin;
    FormattedField      aFmtFldOrigin;
    CheckBox            aCbxAutoOrigin;
    CheckBox            aCbxLogarithm;
    CheckBox            aCbxReverse;

    FixedLine aFlTicks;
    CheckBox aCbxTicksInner;
    CheckBox aCbxTicksOuter;

    FixedLine aFlHelpTicks;
    CheckBox aCbxHelpTicksInner;
    CheckBox aCbxHelpTicksOuter;

    double              fMin;
    double              fMax;
    double              fStepMain;
    sal_Int32           nStepHelp;
    double              fOrigin;
    int                 nAxisType;
    SvNumberFormatter*  pNumFormatter;

    void EnableControls();

    DECL_LINK( EnableValueHdl, CheckBox* );

    /** shows a warning window due to an invalid input.

        @param nResIdMessage
            The resource identifier that represents the localized warning text.
            If this is 0, no warning is shown and false is returned.

        @param pControl
            If non-NULL, contains a pointer to the control in which the
            errornous value was in.  This method gives this control the focus
            and selects its content.

        @return false, if nResIdMessage was 0, true otherwise
     */
    bool ShowWarning( USHORT nResIdMessage, Edit * pControl = NULL );

public:
    ScaleTabPage( Window* pParent, const SfxItemSet& rInAttrs );

    static SfxTabPage* Create( Window* pParent, const SfxItemSet& rInAttrs );
    virtual BOOL FillItemSet( SfxItemSet& rOutAttrs );
    virtual void Reset( const SfxItemSet& rInAttrs );
    using TabPage::DeactivatePage;
    virtual int DeactivatePage( SfxItemSet* pItemSet = NULL );

    void SetNumFormatter( SvNumberFormatter* pFormatter );
    void SetNumFormat();
};

//.............................................................................
} //namespace chart
//.............................................................................

#endif

