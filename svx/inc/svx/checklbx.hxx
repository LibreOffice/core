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


#ifndef _SVX_CHECKLBX_HXX
#define _SVX_CHECKLBX_HXX

// include ---------------------------------------------------------------


#include <svtools/svtreebx.hxx>
#include <svtools/svlbitm.hxx>

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#include "svx/svxdllapi.h"

// class SvxCheckListBox -------------------------------------------------

class SVX_DLLPUBLIC SvxCheckListBox : public SvTreeListBox
{
    using Window::GetText;

private:
    SvLBoxButtonData*   pCheckButton;

#ifdef _SVX_CHECKLBX_CXX
    SVX_DLLPRIVATE void             Init_Impl();
#endif

    using SvTreeListBox::InsertEntry;
    // Avoid ambiguity with new InsertEntry:
    virtual SvLBoxEntry*    InsertEntry( const XubString& rText, SvLBoxEntry* pParent,
                                         sal_Bool bChildsOnDemand,
                                         sal_uIntPtr nPos, void* pUserData,
                                         SvLBoxButtonKind eButtonKind );

public:
    SvxCheckListBox( Window* pParent, WinBits nWinStyle = 0 );
    SvxCheckListBox( Window* pParent, const ResId& rResId );
    SvxCheckListBox( Window* pParent, const ResId& rResId,
                     const Image& rNormalStaticImage,
                     const Image& rHighContrastStaticImage );
    ~SvxCheckListBox();

    void            InsertEntry         ( const String& rStr,
                                          sal_uInt16 nPos = LISTBOX_APPEND,
                                          void* pUserData = NULL,
                                          SvLBoxButtonKind eButtonKind =
                                              SvLBoxButtonKind_enabledCheckbox );
    void            RemoveEntry         ( sal_uInt16 nPos );

    void            SelectEntryPos      ( sal_uInt16 nPos, sal_Bool bSelect = sal_True );
    sal_uInt16          GetSelectEntryPos   () const;

    String          GetText             ( sal_uInt16 nPos ) const;
    sal_uInt16          GetCheckedEntryCount() const;
    void            CheckEntryPos       ( sal_uInt16 nPos, sal_Bool bCheck = sal_True );
    sal_Bool            IsChecked           ( sal_uInt16 nPos ) const;
    void            ToggleCheckButton   ( SvLBoxEntry* pEntry );

    void*           SetEntryData        ( sal_uInt16 nPos, void* pNewData );
    void*           GetEntryData        ( sal_uInt16 nPos ) const;

    virtual void    MouseButtonDown     ( const MouseEvent& rMEvt );
    virtual void    KeyInput            ( const KeyEvent& rKEvt );
};


#endif

