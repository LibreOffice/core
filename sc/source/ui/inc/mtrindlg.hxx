/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: mtrindlg.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-08 21:37:49 $
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

#ifndef SC_MTRINDLG_HXX
#define SC_MTRINDLG_HXX

#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/imagebtn.hxx>
#endif

#ifndef _SV_FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif


//------------------------------------------------------------------------

class ScMetricInputDlg : public ModalDialog
{
public:
            ScMetricInputDlg( Window*       pParent,
                              USHORT        nResId,     // Ableitung fuer jeden Dialog!
                              long          nCurrent,
                              long          nDefault,
                              FieldUnit     eFUnit    = FUNIT_MM,
                              USHORT        nDecimals = 2,
                              long          nMaximum  = 1000,
                              long          nMinimum  = 0,
                              long          nFirst    = 1,
                              long          nLast     = 100 );
            ~ScMetricInputDlg();

    long GetInputValue( FieldUnit eUnit = FUNIT_TWIP ) const;

private:
    FixedText       aFtEditTitle;
    MetricField     aEdValue;
    CheckBox        aBtnDefVal;
    OKButton        aBtnOk;
    CancelButton    aBtnCancel;
    HelpButton      aBtnHelp;
    long            nDefaultValue;
    long            nCurrentValue;

    void CalcPositions();
    DECL_LINK( SetDefValHdl, CheckBox * );
    DECL_LINK( ModifyHdl, MetricField * );
};

#endif // SC_MTRINDLG_HXX





