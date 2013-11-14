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



// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"




#include <tools/list.hxx>
#include "actctrl.hxx"



void NumEditAction::Action()
{
    aActionLink.Call( this );
}


long NumEditAction::Notify( NotifyEvent& rNEvt )
{
    long nHandled = 0;

    if ( rNEvt.GetType() == EVENT_KEYINPUT )
    {
        const KeyEvent* pKEvt = rNEvt.GetKeyEvent();
        const KeyCode aKeyCode = pKEvt->GetKeyCode();
        const sal_uInt16 nModifier = aKeyCode.GetModifier();
        if( aKeyCode.GetCode() == KEY_RETURN &&
                !nModifier)
        {
            Action();
            nHandled = 1;
        }

    }
    if(!nHandled)
        NumericField::Notify( rNEvt );
    return nHandled;
}

/*------------------------------------------------------------------------
 Beschreibung:  KeyInput fuer ShortName - Edits ohne Spaces
------------------------------------------------------------------------*/

NoSpaceEdit::NoSpaceEdit( Window* pParent, const ResId& rResId)
    : Edit(pParent, rResId),
    sForbiddenChars(String::CreateFromAscii(" "))
{
}

NoSpaceEdit::~NoSpaceEdit()
{
}

void NoSpaceEdit::KeyInput(const KeyEvent& rEvt)
{
    sal_Bool bCallParent = sal_True;
    if(rEvt.GetCharCode())
    {
        String sKey = rEvt.GetCharCode();
        if( STRING_NOTFOUND != sForbiddenChars.Search(sKey))
            bCallParent = sal_False;
    }
    if(bCallParent)
        Edit::KeyInput(rEvt);
}
/* -----------------------------11.02.00 15:28--------------------------------

 ---------------------------------------------------------------------------*/
void NoSpaceEdit::Modify()
{
    Selection aSel = GetSelection();
    String sTemp = GetText();
    for(sal_uInt16 i = 0; i < sForbiddenChars.Len(); i++)
    {
        sTemp.EraseAllChars( sForbiddenChars.GetChar(i) );
    }
    sal_uInt16 nDiff = GetText().Len() - sTemp.Len();
    if(nDiff)
    {
        aSel.setMin(aSel.getMin() - nDiff);
        aSel.setMax(aSel.getMin());
        SetText(sTemp);
        SetSelection(aSel);
    }
//IAccessibility2 Impplementaton 2009-----
    Edit::Modify();
    /*
    if(GetModifyHdl().IsSet())
        GetModifyHdl().Call(this);
        */
//-----IAccessibility2 Impplementaton 2009
}
/* -----------------25.06.2003 15:57-----------------

 --------------------------------------------------*/
ReturnActionEdit::~ReturnActionEdit()
{
}
/* -----------------25.06.2003 15:58-----------------

 --------------------------------------------------*/
void ReturnActionEdit::KeyInput( const KeyEvent& rEvt)
{
    const KeyCode aKeyCode = rEvt.GetKeyCode();
    const sal_uInt16 nModifier = aKeyCode.GetModifier();
    if( aKeyCode.GetCode() == KEY_RETURN &&
            !nModifier)
    {
        if(aReturnActionLink.IsSet())
            aReturnActionLink.Call(this);
    }
    else
        Edit::KeyInput(rEvt);
}


