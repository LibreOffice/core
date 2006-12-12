/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inspagob.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 17:44:08 $
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

#ifndef _SD_INSPAGOB_HXX
#define _SD_INSPAGOB_HXX

#ifndef _SV_BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _SV_DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif

#ifndef _SDTREELB_HXX
#include "sdtreelb.hxx"
#endif

class SdDrawDocument;

//------------------------------------------------------------------------

class SdInsertPagesObjsDlg : public ModalDialog
{
private:
    SdPageObjsTLB           aLbTree;
    CheckBox                aCbxLink;
    CheckBox                aCbxMasters;
    OKButton                aBtnOk;
    CancelButton            aBtnCancel;
    HelpButton              aBtnHelp;

    SfxMedium*              pMedium;
    const SdDrawDocument*   mpDoc;
    const String&           rName;

    void                    Reset();
    DECL_LINK( SelectObjectHdl, void * );

public:
                SdInsertPagesObjsDlg( Window* pParent,
                                const SdDrawDocument* pDoc,
                                SfxMedium* pSfxMedium,
                                const String& rFileName );
                ~SdInsertPagesObjsDlg();

    List*       GetList( USHORT nType );
    BOOL        IsLink();
    BOOL        IsRemoveUnnessesaryMasterPages() const;
};


#endif // _SD_INSPAGOB_HXX
