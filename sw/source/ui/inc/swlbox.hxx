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


#ifndef _SWLBOX_HXX
#define _SWLBOX_HXX

#include <svl/svarray.hxx>

#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif

#ifndef _COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#include "swdllapi.h"

class SwBoxEntry;
class Window;

SV_DECL_PTRARR_DEL(SwEntryLst, SwBoxEntry*, 10, 10)

/*--------------------------------------------------------------------
     Beschreibung: SwBoxEntry
 --------------------------------------------------------------------*/

class SW_DLLPUBLIC SwBoxEntry
{
    friend class SwComboBox;

    sal_Bool    bModified : 1;
    sal_Bool    bNew : 1;

    String  aName;
    sal_uInt16  nId;

public:
    SwBoxEntry(const String& aName, sal_uInt16 nId=0);
    SwBoxEntry(const SwBoxEntry& rOrg);
    SwBoxEntry();

    const String& GetName() const { return aName;}
};

/*--------------------------------------------------------------------
     Beschreibung: fuer ComboBoxen
 --------------------------------------------------------------------*/

typedef sal_uInt16 SwComboBoxStyle;
namespace nsSwComboBoxStyle
{
    const SwComboBoxStyle CBS_UPPER         = 0x01;
    const SwComboBoxStyle CBS_LOWER         = 0x02;
    const SwComboBoxStyle CBS_ALL           = 0x04;
    const SwComboBoxStyle CBS_FILENAME      = 0x08;
    const SwComboBoxStyle CBS_SW_FILENAME   = CBS_FILENAME;
}

class SW_DLLPUBLIC SwComboBox : public ComboBox
{
    SwEntryLst              aEntryLst;
    SwEntryLst              aDelEntryLst;
    SwBoxEntry              aDefault;
    sal_uInt16                  nStyle;

    SW_DLLPRIVATE void                  InitComboBox();
    SW_DLLPRIVATE void                  InsertSorted(SwBoxEntry* pEntry);

    using ComboBox::InsertEntry;
    using ComboBox::RemoveEntry;
    using Window::SetStyle;

public:

    using ComboBox::GetEntryPos;

    SwComboBox(Window* pParent, const ResId& rId,
               sal_uInt16 nStyleBits = nsSwComboBoxStyle::CBS_ALL);
    ~SwComboBox();

    virtual void            KeyInput( const KeyEvent& rKEvt );

    void                    InsertEntry(const SwBoxEntry&);
    sal_uInt16                  InsertEntry( const XubString& rStr, sal_uInt16 = 0)
                            {        InsertEntry( SwBoxEntry( rStr ) ); return 0;    }

    void                    RemoveEntry(sal_uInt16 nPos);

    sal_uInt16                  GetEntryPos(const SwBoxEntry& rEntry) const;
    const SwBoxEntry&       GetEntry(sal_uInt16) const;

    sal_uInt16                  GetRemovedCount() const;
    const SwBoxEntry&       GetRemovedEntry(sal_uInt16 nPos) const;

    sal_uInt16                  GetStyle() const            { return nStyle;    }
    void                    SetStyle(const sal_uInt16 nSt)  { nStyle = nSt;     }

    String                  GetText() const;
//IAccessibility2 Impplementaton 2009-----
    void                    SetText( const XubString& rStr );
    void                    SetText( const XubString& rStr, const Selection& rNewSelection ) { ComboBox::SetText( rStr, rNewSelection ); };
};

#endif /* _SWLBOX_HXX */
