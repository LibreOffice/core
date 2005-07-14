/*************************************************************************
 *
 *  $RCSfile: RecentMasterPagesSelector.cxx,v $
 *
 *  $Revision: 1.5 $
 *
 *  last change: $Author: kz $ $Date: 2005-07-14 10:26:37 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#include "RecentMasterPagesSelector.hxx"
#include "PreviewValueSet.hxx"

#include "RecentlyUsedMasterPages.hxx"
#include "MasterPageObserver.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"
#include <vcl/bitmap.hxx>
#include <tools/color.hxx>

namespace sd { namespace toolpanel { namespace controls {


RecentMasterPagesSelector::RecentMasterPagesSelector (
    TreeNode* pParent,
    SdDrawDocument& rDocument,
    ViewShellBase& rBase)
    : MasterPagesContainerSelector (pParent, rDocument, rBase)
{
    SetName (String(RTL_CONSTASCII_USTRINGPARAM("RecentMasterPagesSelector")));
}




RecentMasterPagesSelector::~RecentMasterPagesSelector (void)
{
    RecentlyUsedMasterPages::Instance().RemoveEventListener (
        LINK(this,RecentMasterPagesSelector,MasterPageListListener));
}




void RecentMasterPagesSelector::LateInit (void)
{
    MasterPagesSelector::LateInit();

    Fill ();
    RecentlyUsedMasterPages::Instance().AddEventListener (
        LINK(this,RecentMasterPagesSelector,MasterPageListListener));
}




IMPL_LINK(RecentMasterPagesSelector,MasterPageListListener, void*, pUnused)
{
    Fill ();
    return 0;
}




void RecentMasterPagesSelector::Fill (void)
{
    Clear ();

    // Create a set of names of the master pages used by the given document.
    MasterPageObserver::MasterPageNameSet aCurrentNames;
    USHORT nMasterPageCount = mrDocument.GetMasterSdPageCount(PK_STANDARD);
    USHORT nIndex;
    for (nIndex=0; nIndex<nMasterPageCount; nIndex++)
    {
        SdPage* pMasterPage = mrDocument.GetMasterSdPage (nIndex, PK_STANDARD);
        if (pMasterPage != NULL)
            aCurrentNames.insert (pMasterPage->GetName());
    }
    MasterPageObserver::MasterPageNameSet::iterator aI;

    // Insert the recently used master pages that are not currently used.
    int nPageCount = RecentlyUsedMasterPages::Instance().GetMasterPageCount();
    for (nIndex=0; nIndex<nPageCount; nIndex++)
    {
        // Add the page when a) the style name is empty, i.e. it has not yet
        // been loaded (and thus can not be in use) or otherwise b) the
        // style name is not currently in use.
        String sStyleName (RecentlyUsedMasterPages::Instance().GetMasterPageStyleName(nIndex));
        String sPageName (RecentlyUsedMasterPages::Instance().GetMasterPageName(nIndex));
        if (sStyleName.Len()==0
            || aCurrentNames.find(sStyleName) == aCurrentNames.end())
        {
            AddItemForPage (
                String(),
                sPageName,
                NULL,
                Image());
            aCurrentNames.insert (sPageName);
        }
    }
    mpPageSet->Rearrange();
}




void RecentMasterPagesSelector::AssignMasterPageToPageList (
    SdPage* pMasterPage,
    const ::std::vector<SdPage*>& rPageList)
{
    USHORT nSelectedItemId = mpPageSet->GetSelectItemId();

    MasterPagesContainerSelector::AssignMasterPageToPageList(pMasterPage, rPageList);

    // Restore the selection.
    if (mpPageSet->GetItemCount() > 0)
    {
        if (mpPageSet->GetItemCount() >= nSelectedItemId)
            mpPageSet->SelectItem(nSelectedItemId);
        else
            mpPageSet->SelectItem(mpPageSet->GetItemCount());
    }
}

} } } // end of namespace ::sd::toolpanel::controls
