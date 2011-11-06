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


#ifndef _BOOKMARK_HXX
#define _BOOKMARK_HXX

#include <svx/stddlg.hxx>
#include <vcl/fixed.hxx>

#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif

#include "swlbox.hxx"       // SwComboBox

class SwWrtShell;
class SfxRequest;

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

class BookmarkCombo : public SwComboBox
{
    sal_uInt16          GetFirstSelEntryPos() const;
    sal_uInt16          GetNextSelEntryPos(sal_uInt16 nPos) const;
    sal_uInt16          GetSelEntryPos(sal_uInt16 nPos) const;

    virtual long    PreNotify(NotifyEvent& rNEvt);
public:
    BookmarkCombo( Window* pWin, const ResId& rResId );

    sal_uInt16          GetSelectEntryCount() const;
    sal_uInt16          GetSelectEntryPos( sal_uInt16 nSelIndex = 0 ) const;

    static const String aForbiddenChars;
};

/*--------------------------------------------------------------------
    Beschreibung:
 --------------------------------------------------------------------*/

class SwInsertBookmarkDlg: public SvxStandardDialog
{
    FixedLine       aBookmarkFl;
    BookmarkCombo   aBookmarkBox;
    OKButton        aOkBtn;
    CancelButton    aCancelBtn;
    PushButton      aDeleteBtn;

    String          sRemoveWarning;
    SwWrtShell      &rSh;
    SfxRequest&     rReq;

    DECL_LINK( ModifyHdl, BookmarkCombo * );
    DECL_LINK( DeleteHdl, Button * );

    virtual void Apply();

public:
    SwInsertBookmarkDlg( Window *pParent, SwWrtShell &rSh, SfxRequest& rReq );
    ~SwInsertBookmarkDlg();
};

#endif
