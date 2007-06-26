/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: layeroptionsdlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2007-06-26 13:41:18 $
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


#ifndef _SD_LAYER_DLG_HXX_
#define _SD_LAYER_DLG_HXX_

#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/dialog.hxx>
#include <vcl/button.hxx>
#include <svtools/svmedit.hxx>
#include "sdresid.hxx"
#include "strings.hrc"

class SfxItemSet;

class SdInsertLayerDlg : public ModalDialog
{
private:
    FixedText           maFtName;
    Edit                maEdtName;
    FixedText           maFtTitle;
    Edit                maEdtTitle;
    FixedText           maFtDesc;
    MultiLineEdit       maEdtDesc;
    CheckBox            maCbxVisible;
    CheckBox            maCbxPrintable;
    CheckBox            maCbxLocked;
    FixedLine           maFixedLine;
    HelpButton          maBtnHelp;
    OKButton            maBtnOK;
    CancelButton        maBtnCancel;

    const SfxItemSet&   mrOutAttrs;

public:

    SdInsertLayerDlg( Window* pWindow, const SfxItemSet& rInAttrs, bool bDeletable, String aStr );
    void                GetAttr( SfxItemSet& rOutAttrs );
};

#endif // _SD_LAYER_DLG_HXX_
