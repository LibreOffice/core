/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _SFXDOCTDLG_HXX
#define _SFXDOCTDLG_HXX

#include <sfx2/doctempl.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#include <vcl/dialog.hxx>
#include <vcl/fixed.hxx>

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
    sal_uInt16 GetRegion() const { return aRegionLb.GetSelectEntryPos(); }
    String GetRegionName() const { return aRegionLb.GetSelectEntry(); }
};

#endif

