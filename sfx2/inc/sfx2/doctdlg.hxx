/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: doctdlg.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 21:19:23 $
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
#ifndef _SFXDOCTDLG_HXX
#define _SFXDOCTDLG_HXX

#include <sfx2/doctempl.hxx>

#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_EDIT_HXX
#include <vcl/edit.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_DIALOG_HXX
#include <vcl/dialog.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

class SfxModalDefParentHelper;

class SfxDocumentTemplateDlg : public ModalDialog
{
private:

    FixedLine       aEditFL;
    Edit            aNameEd;
    FixedLine       aTemplateFL;
    FixedText       aRegionFt;
    ListBox         aRegionLb;
    FixedText       aTemplateFt;
    ListBox         aTemplateLb;

    OKButton        aOkBt;
    CancelButton    aCancelBt;
    HelpButton      aHelpBt;
    PushButton      aEditBt;
    PushButton      aOrganizeBt;

    SfxDocumentTemplates *pTemplates;

    SfxModalDefParentHelper* pHelper;

    void Init();

    DECL_LINK( OrganizeHdl, Button * );
    DECL_LINK( OkHdl, Control * );
    DECL_LINK( RegionSelect, ListBox * );
    DECL_LINK( TemplateSelect, ListBox * );
    DECL_LINK( NameModify, Edit * );
    DECL_LINK( EditHdl, Button * );

public:
    SfxDocumentTemplateDlg(Window * pParent, SfxDocumentTemplates* pTempl );
    ~SfxDocumentTemplateDlg();

    String GetTemplateName() const
    { return aNameEd.GetText().EraseLeadingChars(); }
    String GetTemplatePath();
    void NewTemplate(const String &rPath);
    USHORT GetRegion() const { return aRegionLb.GetSelectEntryPos(); }
    String GetRegionName() const { return aRegionLb.GetSelectEntry(); }
};

#endif

