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
#ifndef _ACTCTRL_HXX
#define _ACTCTRL_HXX

#include <vcl/field.hxx>
#include "swdllapi.h"

/*--------------------------------------------------------------------
    Description:    numerical input
 --------------------------------------------------------------------*/
class SW_DLLPUBLIC NumEditAction: public NumericField
{
    Link aActionLink;

protected:
    virtual void Action();
    virtual long    Notify( NotifyEvent& rNEvt );
public:
    NumEditAction( Window* pParent, const ResId& rResId ) :
                                NumericField(pParent, rResId) {}

    void        SetActionHdl( const Link& rLink )   { aActionLink = rLink;}
    const Link& GetActionHdl() const                { return aActionLink; }
};


/* --------------------------------------------------
 * Edit that doesn't accept spaces
 * --------------------------------------------------*/
class SW_DLLPUBLIC NoSpaceEdit : public Edit
{
    String sForbiddenChars;
protected:
    virtual void KeyInput( const KeyEvent& );
    virtual void        Modify();

public:
    NoSpaceEdit( Window* pParent );
    virtual ~NoSpaceEdit();
    void            SetForbiddenChars(const String& rSet){sForbiddenChars = rSet;}
    const String&   GetForbiddenChars(){return sForbiddenChars;}
};

/* --------------------------------------------------
 * No space and no full stop
 * --------------------------------------------------*/
class TableNameEdit : public NoSpaceEdit
{
public:
    TableNameEdit(Window* pWin)
        : NoSpaceEdit(pWin)
    {
        SetForbiddenChars(OUString(" .<>"));
    }
};

/* --------------------------------------------------
    call a link when KEY_RETURN is pressed
 --------------------------------------------------*/
class SW_DLLPUBLIC ReturnActionEdit : public Edit
{
    Link    aReturnActionLink;
public:
    ReturnActionEdit(Window* pParent, const ResId& rResId);
    ReturnActionEdit(Window* pParent, WinBits nStyle)
        : Edit(pParent, nStyle){}
    ~ReturnActionEdit();
    virtual void KeyInput( const KeyEvent& );

    void SetReturnActionLink(const Link& rLink)
            { aReturnActionLink = rLink;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
