/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: AllMasterPagesSelector.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 06:37:31 $
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

#include "AllMasterPagesSelector.hxx"
#include "PreviewValueSet.hxx"

#include "MasterPageContainer.hxx"
#include "DrawDocShell.hxx"
#include <svtools/languageoptions.hxx>
#include "drawdoc.hxx"
#include <vcl/image.hxx>
#include <sfx2/app.hxx>
#include <svtools/itemset.hxx>
#include <svtools/eitem.hxx>
#include <svtools/colorcfg.hxx>


namespace sd { namespace toolpanel { namespace controls {

AllMasterPagesSelector::AllMasterPagesSelector (
    TreeNode* pParent,
    SdDrawDocument& rDocument,
    ViewShellBase& rBase,
    DrawViewShell& rViewShell)
    : MasterPagesContainerSelector (pParent, rDocument, rBase),
      mrViewShell(rViewShell)
{
    SetName (String(RTL_CONSTASCII_USTRINGPARAM("AllMasterPagesSelector")));
    Fill ();
}




AllMasterPagesSelector::~AllMasterPagesSelector (void)
{
}




void AllMasterPagesSelector::Fill (void)
{
    Clear();

    MasterPageContainer& rContainer (MasterPageContainer::Instance());

    int nTokenCount = rContainer.GetTokenCount();
    for (int i=0; i<nTokenCount; i++)
    {
        MasterPageContainer::Token aToken (rContainer.GetTokenForIndex(i));
        AddItemForToken (aToken, true);
    }
    mpPageSet->Rearrange ();
}


} } } // end of namespace ::sd::toolpanel::controls
