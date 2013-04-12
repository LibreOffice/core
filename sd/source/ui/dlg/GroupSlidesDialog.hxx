/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
* This file is part of the LibreOffice project.
*
* This Source Code Form is subject to the terms of the Mozilla Public
* License, v. 2.0. If a copy of the MPL was not distributed with this
* file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _SD_GROUPSLIDESDIALOG_HXX
#define _SD_GROUPSLIDESDIALOG_HXX

#include "tools/link.hxx"
#include "sdpage.hxx"
#include "pres.hxx"
#include "drawdoc.hxx"

#include <vcl/lstbox.hxx>
#include <vcl/fixed.hxx>
#include <vcl/button.hxx>
#include <vcl/dialog.hxx>
#include <vcl/field.hxx>
#include <vcl/graphicfilter.hxx>
#include <svx/svdotext.hxx>

#include <slidehack.hxx>

class SdrTextObj;
class SdDrawDocument;
class SdPage;

namespace SlideHack {
    class Group;
    typedef boost::shared_ptr< class Group > GroupPtr;
}

namespace sd
{

class SdGroupSlidesDialog : public ModalDialog
{
public:
    SdGroupSlidesDialog(Window* pWindow, SdDrawDocument* pActDoc,
                        const std::vector< SdPage * > &rPages );
    ~SdGroupSlidesDialog();

private:
    SdDrawDocument* mpDoc;
    std::vector< SdPage * > maPages;

    CancelButton  *mpCancelBtn;
    PushButton    *mpAddBtn;

//    ComboBox*      mpGroupCombo;
    Edit          *mpGroupEdit;
    std::vector< SlideHack::GroupPtr > maGroups;
    void addGroupsToCombo( ComboBox *pBox, SdDrawDocument *pDoc );

    Edit          *mpTitle;
    Edit          *mpKeywords;

    DECL_LINK( AddHdl, void* );
    DECL_LINK( CancelHdl, void* );
#if 0
    DECL_LINK( GroupSelectHdl, void* );
    DECL_LINK( GroupDoubleClickHdl, void* );
#endif

    int  endDialog( bool bSuccessSoSave );
    void populateEdits( SlideHack::GroupPtr pGroup );
};

}

#endif // _SD_GROUPSLIDESDIALOG_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
