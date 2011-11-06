/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/


#ifndef _ACTCTRL_HXX
#define _ACTCTRL_HXX

#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#include "swdllapi.h"

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
    NoSpaceEdit( Window* pParent, const ResId& rResId);
    virtual ~NoSpaceEdit();
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
