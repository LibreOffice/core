/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */
#ifndef INCLUDED_SW_SOURCE_UIBASE_INC_ACTCTRL_HXX
#define INCLUDED_SW_SOURCE_UIBASE_INC_ACTCTRL_HXX

#include <vcl/field.hxx>
#include "swdllapi.h"

// numerical input
class SW_DLLPUBLIC NumEditAction: public NumericField
{
    Link<NumEditAction&,void> aActionLink;

protected:
    void Action();
    virtual bool Notify( NotifyEvent& rNEvt ) override;
public:
    NumEditAction( vcl::Window* pParent, const ResId& rResId ) :
                                NumericField(pParent, rResId) {}

    void        SetActionHdl( const Link<NumEditAction&,void>& rLink ) { aActionLink = rLink;}
};

// call a link when KEY_RETURN is pressed
class SW_DLLPUBLIC ReturnActionEdit : public Edit
{
    Link<ReturnActionEdit&,void>  aReturnActionLink;
public:
    ReturnActionEdit(vcl::Window* pParent, WinBits nStyle)
        : Edit(pParent, nStyle)
    {
    }
    virtual void KeyInput( const KeyEvent& ) override;

    void SetReturnActionLink(const Link<ReturnActionEdit&,void>& rLink)
            { aReturnActionLink = rLink;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
