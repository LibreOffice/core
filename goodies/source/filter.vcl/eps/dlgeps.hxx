/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgeps.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:44:20 $
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

#ifndef _DLGEPS_HXX_
#define _DLGEPS_HXX_
#include <svtools/fltcall.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>
#include <svtools/stdctrl.hxx>


/*************************************************************************
|*
|* Dialog zum Einstellen von Filteroptionen
|*
\************************************************************************/

class FilterConfigItem;
class ResMgr;

class DlgExportEPS : public ModalDialog
{
private:

    FltCallDialogParameter& rFltCallPara;

    FixedLine           aGrpPreview;
    CheckBox            aCBPreviewTiff;
    CheckBox            aCBPreviewEPSI;
    FixedLine           aGrpVersion;
    RadioButton         aRBLevel1;
    RadioButton         aRBLevel2;
    FixedLine           aGrpColor;
    RadioButton         aRBColor;
    RadioButton         aRBGrayscale;
    FixedLine           aGrpCompression;
    RadioButton         aRBCompressionLZW;
    RadioButton         aRBCompressionNone;
    OKButton            aBtnOK;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;

    FilterConfigItem*   pConfigItem;
    ResMgr*             pMgr;

    DECL_LINK( OK, void * );
    DECL_LINK( LEVEL1, void* );
    DECL_LINK( LEVEL2, void* );

public:
            DlgExportEPS( FltCallDialogParameter& rPara );
            ~DlgExportEPS();
};

#endif // _DLGEPS_HXX_
