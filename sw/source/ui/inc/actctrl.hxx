/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: actctrl.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 08:58:09 $
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
#ifndef _ACTCTRL_HXX
#define _ACTCTRL_HXX

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef INCLUDED_SWDLLAPI_H
#include "swdllapi.h"
#endif

/*--------------------------------------------------------------------
    Beschreibung:   numerische Eingabe
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


/* -----------------21.04.98 08:11-------------------
 * Edit, dass keine Spaces akzeptiert
 * --------------------------------------------------*/
class SW_DLLPUBLIC NoSpaceEdit : public Edit
{
    String sForbiddenChars;
protected:
    virtual void KeyInput( const KeyEvent& );
    virtual void        Modify();

public:
    NoSpaceEdit( Window* pParent, const ResId& rResId)
        : Edit(pParent, rResId),
        sForbiddenChars(String::CreateFromAscii(" "))
    {}
    void            SetForbiddenChars(const String& rSet){sForbiddenChars = rSet;}
    const String&   GetForbiddenChars(){return sForbiddenChars;}
};

/* -----------------21.04.98 08:33-------------------
 * Kein Space und kein Punkt
 * --------------------------------------------------*/
class TableNameEdit : public NoSpaceEdit
{
public:
    TableNameEdit(Window* pWin, const ResId& rResId) :
                            NoSpaceEdit(pWin, rResId)
                                {SetForbiddenChars(String::CreateFromAscii(" .<>"));}
};
/* -----------------25.06.2003 15:55-----------------
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
