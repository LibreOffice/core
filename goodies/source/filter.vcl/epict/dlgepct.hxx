/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgepct.hxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:39:50 $
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


#ifndef _DLGEPCT_HXX_
#define _DLGEPCT_HXX_

#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <svtools/fltcall.hxx>

/*************************************************************************
|*
|* Dialog zum Einstellen von Filteroptionen bei Vektorformaten
|*
\************************************************************************/

class FilterConfigItem;
class ResMgr;

class DlgExportEPCT : public ModalDialog
{
private:

    FltCallDialogParameter& rFltCallPara;

    OKButton            aBtnOK;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;

    RadioButton         aRbOriginal;
    RadioButton         aRbSize;
    FixedLine           aGrpMode;

    FixedText           aFtSizeX;
    MetricField         aMtfSizeX;
    FixedText           aFtSizeY;
    MetricField         aMtfSizeY;
    FixedLine           aGrpSize;

    FilterConfigItem*   pConfigItem;
    ResMgr*             pMgr;

    DECL_LINK( OK, void* p );
    DECL_LINK( ClickRbOriginal,void* p );
    DECL_LINK( ClickRbSize,void* p );

public:
            DlgExportEPCT( FltCallDialogParameter& rPara );
            ~DlgExportEPCT();
};

#endif // _DLGEPCT_HXX_

