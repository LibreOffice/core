/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
    NoSpaceEdit( Window* pParent, const ResId& rResId);
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
    TableNameEdit(Window* pWin, const ResId& rResId)
        : NoSpaceEdit(pWin, rResId)
    {
        SetForbiddenChars(rtl::OUString(" .<>"));
    }
    TableNameEdit(Window* pWin)
        : NoSpaceEdit(pWin)
    {
        SetForbiddenChars(rtl::OUString(" .<>"));
    }
};

/* --------------------------------------------------
    call a link when KEY_RETURN is pressed
 --------------------------------------------------*/
class SW_DLLPUBLIC ReturnActionEdit : public Edit
{
    Link    aReturnActionLink;
public:
    ReturnActionEdit( Window* pParent, const ResId& rResId)
        : Edit(pParent, rResId){}
    ~ReturnActionEdit();
    virtual void KeyInput( const KeyEvent& );

    void SetReturnActionLink(const Link& rLink)
            { aReturnActionLink = rLink;}
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
