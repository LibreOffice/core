/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http:
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http:
 */
#include "ParaLineSpacingPopup.hxx"
#include "ParaLineSpacingControl.hxx"
#include <boost/bind.hpp>
#include <unotools/viewoptions.hxx>

namespace svx { namespace sidebar {

ParaLineSpacingPopup::ParaLineSpacingPopup (
    Window* pParent,
    const ::boost::function<PopupControl*(PopupContainer*)>& rControlCreator)
    : Popup(
        pParent,
        rControlCreator,
        OUString( "Paragraph Line Spacing"))
{
    SetPopupModeEndHandler(::boost::bind(&ParaLineSpacingPopup::PopupModeEndCallback, this));
}




ParaLineSpacingPopup::~ParaLineSpacingPopup (void)
{
}




void ParaLineSpacingPopup::Rearrange (SfxItemState currSPState,FieldUnit currMetricUnit,SvxLineSpacingItem* currSPItem,const ::sfx2::sidebar::EnumContext currentContext)
{
    ProvideContainerAndControl();

    ParaLineSpacingControl* pControl = dynamic_cast<ParaLineSpacingControl*>(mpControl.get());
    if (pControl != NULL)
        pControl->Rearrange(currSPState,currMetricUnit, currSPItem,currentContext);
}

void ParaLineSpacingPopup::PopupModeEndCallback (void)
{
    ProvideContainerAndControl();
    ParaLineSpacingControl* pControl = dynamic_cast<ParaLineSpacingControl*>(mpControl.get());
    if (pControl == NULL)
        return;

    pControl->PopupModeEndCallback();

    /*if( pControl->GetLastCustomState() == SPACING_CLOSE_BY_CUS_EDIT)
    {
        SvtViewOptions aWinOpt( E_WINDOW, SIDEBAR_SPACING_GLOBAL_VALUE );
        ::com::sun::star::uno::Sequence < ::com::sun::star::beans::NamedValue > aSeq(1);
        aSeq[0].Name = "Spacing";
        aSeq[0].Value <<= ::rtl::OUString( String::CreateFromInt32( pControl->GetLastCustomValue() ));
        aWinOpt.SetUserData( aSeq );

    }*/
}


} } 


