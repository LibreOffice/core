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


#ifndef _INSFNOTE_HXX
#define _INSFNOTE_HXX

#include <svx/stddlg.hxx>

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#ifndef _EDIT_HXX //autogen
#include <vcl/edit.hxx>
#endif
#include <vcl/fixed.hxx>

class SwWrtShell;

class SwInsFootNoteDlg: public SvxStandardDialog
{
    SwWrtShell     &rSh;

    //Alles fuer das/die Zeichen
    String          aFontName;
    CharSet         eCharSet;
    char            cExtChar;
    sal_Bool        bExtCharAvailable;
    sal_Bool        bEdit;
    FixedLine       aNumberFL;
    RadioButton     aNumberAutoBtn;
    RadioButton     aNumberCharBtn;
    Edit            aNumberCharEdit;
    PushButton      aNumberExtChar;

    //Alles fuer die Auswahl Fussnote/Endnote
    FixedLine        aTypeFL;
    RadioButton     aFtnBtn;
    RadioButton     aEndNoteBtn;

    OKButton        aOkBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;
    ImageButton     aPrevBT;
    ImageButton     aNextBT;

    DECL_LINK( NumberCharHdl, Button * );
    DECL_LINK( NumberEditHdl, void * );
    DECL_LINK( NumberAutoBtnHdl, Button *);
    DECL_LINK( NumberExtCharHdl, Button *);
    DECL_LINK( NextPrevHdl, Button * );

    virtual void    Apply();

    void            Init();

public:
    SwInsFootNoteDlg(Window * pParent, SwWrtShell &rSh, sal_Bool bEd = sal_False);
    ~SwInsFootNoteDlg();

    CharSet         GetCharSet() { return eCharSet; }
    sal_Bool            IsExtCharAvailable() { return bExtCharAvailable; }
    String          GetFontName() { return aFontName; }
    sal_Bool            IsEndNote() { return aEndNoteBtn.IsChecked(); }
    String          GetStr()
                    {
                        if ( aNumberCharBtn.IsChecked() )
                            return aNumberCharEdit.GetText();
                        else
                            return String();
                    }
};

#endif
