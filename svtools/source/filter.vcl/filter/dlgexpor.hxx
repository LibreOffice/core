/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: dlgexpor.hxx,v $
 *
 *  $Revision: 1.6 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-27 21:35:09 $
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


#ifndef _DLGEXPOR_HXX_
#define _DLGEXPOR_HXX_

#include <svtools/fltcall.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/field.hxx>
#include <vcl/lstbox.hxx>

/*************************************************************************
|*
|* Dialog zum Einstellen von Filteroptionen bei Pixelformaten
|*
\************************************************************************/

class FilterConfigItem;
class DlgExportPix : public ModalDialog
{
private:

    FltCallDialogParameter& rFltCallPara;

    OKButton            aBtnOK;
    CancelButton        aBtnCancel;
    HelpButton          aBtnHelp;

    ListBox             aLbColors;
    CheckBox            aCbxRLE;
    FixedLine           aGrpColors;

    RadioButton         aRbOriginal;
    RadioButton         aRbRes;
    RadioButton         aRbSize;
    FixedText           aFtSizeX;
    MetricField         aMtfSizeX;
    FixedText           aFtSizeY;
    MetricField         aMtfSizeY;
    FixedLine           aGrpMode;
    ComboBox            aCbbRes;

    FilterConfigItem*   pConfigItem;
    ResMgr*             pMgr;

    String              aExt;

                        DECL_LINK( OK, void* p );
                        DECL_LINK( ClickRbOriginal,void* p );
                        DECL_LINK( ClickRbRes,void* p );
                        DECL_LINK( ClickRbSize,void* p );
                        DECL_LINK( SelectLbColors, void* p );

public:
                        DlgExportPix( FltCallDialogParameter& rPara );
                        ~DlgExportPix();
};


/*************************************************************************
|*
|* Dialog zum Einstellen von Filteroptionen bei Vektorformaten
|*
\************************************************************************/
class DlgExportVec : public ModalDialog
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

    String              aExt;

    DECL_LINK( OK, void* p );
    DECL_LINK( ClickRbOriginal,void* p );
    DECL_LINK( ClickRbSize,void* p );

public:
            DlgExportVec( FltCallDialogParameter& rPara );
            ~DlgExportVec();
};

#endif // _DLGEXPOR_HXX_

