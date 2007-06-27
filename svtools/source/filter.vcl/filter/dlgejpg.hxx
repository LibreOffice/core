/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgejpg.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 21:34:02 $
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


#ifndef _DLGEJPG_HXX_
#define _DLGEJPG_HXX_

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/msgbox.hxx>
#include <svtools/stdctrl.hxx>
#include <svtools/fltcall.hxx>

/*************************************************************************
|*
|* Dialog zum Einstellen von Filteroptionen
|*
\************************************************************************/
class FilterConfigItem;
class DlgExportEJPG : public ModalDialog
{
private:

    FltCallDialogParameter& rFltCallPara;

    FixedInfo           aFiDescr;
    NumericField        aNumFldQuality;
    FixedLine           aGrpQuality;
    RadioButton         aRbGray;
    RadioButton         aRbRGB;
    FixedLine           aGrpColors;
    OKButton            aBtnOK;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;
    FilterConfigItem*   pConfigItem;

    DECL_LINK( OK, void * );

public:
            DlgExportEJPG( FltCallDialogParameter& rDlgPara );
            ~DlgExportEJPG();
};

#endif // _DLGEJPG_HXX_

