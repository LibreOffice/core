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
#include "ParaBulletsPopup.hxx"
#include "ParaBulletsControl.hxx"
#include <boost/bind.hpp>
#include <unotools/viewoptions.hxx>

namespace svx { namespace sidebar {

ParaBulletsPopup::ParaBulletsPopup (
    Window* pParent,
    const ::boost::function<PopupControl*(PopupContainer*)>& rControlCreator)
    : Popup(
        pParent,
        rControlCreator,
        OUString( "Paragraph Bullets"))
{
}




ParaBulletsPopup::~ParaBulletsPopup (void)
{
}




void ParaBulletsPopup::UpdateValueSet ()
{
    ProvideContainerAndControl();

    ParaBulletsControl* pControl = dynamic_cast<ParaBulletsControl*>(mpControl.get());
    if (pControl != NULL)
        pControl->UpdateValueSet();
}



} } 



