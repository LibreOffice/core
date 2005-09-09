/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: masterlayoutdlg.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 05:45:15 $
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
#ifndef _SD_MASTERLAYOUT_DIALOG_HXX
#define _SD_MASTERLAYOUT_DIALOG_HXX

#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#include "sdpage.hxx"

class SdDrawDocument;

namespace sd
{

class MasterLayoutDialog : public ModalDialog
{
private:
    SdDrawDocument* mpDoc;
    SdPage*         mpCurrentPage;

    FixedLine       maFLPlaceholders;
    CheckBox        maCBDate;
    CheckBox        maCBPageNumber;
    CheckBox        maCBHeader;
    CheckBox        maCBFooter;

    OKButton        maPBOK;
    CancelButton    maPBCancel;

    BOOL            mbOldHeader;
    BOOL            mbOldFooter;
    BOOL            mbOldDate;
    BOOL            mbOldPageNumber;

    void applyChanges();
    void remove( PresObjKind eKind );
    void create( PresObjKind eKind );

public:
    MasterLayoutDialog( Window* pParent, SdDrawDocument* pDoc, SdPage* pCurrentPage );
    ~MasterLayoutDialog();

    virtual short Execute();
};

}

#endif // _SD_MASTERLAYOUT_DIALOG_HXX

