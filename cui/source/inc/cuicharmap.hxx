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


#ifndef _CUI_CHARMAP_HXX
#define _CUI_CHARMAP_HXX

// include ---------------------------------------------------------------

#include <vcl/ctrl.hxx>
#include <vcl/metric.hxx>
#include <vcl/button.hxx>
#include <vcl/fixed.hxx>
#include <vcl/lstbox.hxx>
#include <sfx2/basedlgs.hxx>
#include <svx/charmap.hxx>

class SubsetMap;
class SvxCharMapData;

// define ----------------------------------------------------------------

#define CHARMAP_MAXLEN  32

namespace svx
{
    struct SvxShowCharSetItem;
    class SvxShowCharSetVirtualAcc;
}
// class SvxCharacterMap -------------------------------------------------
// the main purpose of this dialog is to enable the use of characters
// that are not easily accesible from the keyboard

// class SvxShowText =====================================================

class SvxShowText : public Control
{
public:
                    SvxShowText( Window* pParent,
                                 const ResId& rResId,
                                 sal_Bool bCenter = sal_False );
                    ~SvxShowText();

    void            SetFont( const Font& rFont );
    void            SetText( const String& rText );

protected:
    virtual void    Paint( const Rectangle& );

private:
    long            mnY;
    sal_Bool            mbCenter;

};

class SvxCharMapData
{
public:
                    SvxCharMapData( class SfxModalDialog* pDialog, sal_Bool bOne_, ResMgr* pResContext );

    void            SetCharFont( const Font& rFont );

private:
friend class SvxCharacterMap;
    SfxModalDialog* mpDialog;

    FixedText       aFontText;
    ListBox         aFontLB;
    FixedText       aSubsetText;
    ListBox         aSubsetLB;
    SvxShowCharSet  aShowSet;
    FixedText       aSymbolText;
    SvxShowText     aShowText;
//    SvxShowText     aShowShortcut;
    OKButton        aOKBtn;
    CancelButton    aCancelBtn;
    HelpButton      aHelpBtn;
    PushButton      aDeleteBtn;
//    PushButton        aAssignBtn;
    SvxShowText     aShowChar;
    FixedText       aCharCodeText;
//  FixedText       aAssignText;
    Font            aFont;
    sal_Bool            bOne;
    const SubsetMap* pSubsetMap;

    DECL_LINK( OKHdl, OKButton* );
    DECL_LINK( FontSelectHdl, ListBox* );
    DECL_LINK( SubsetSelectHdl, ListBox* );
    DECL_LINK( CharDoubleClickHdl, Control* pControl );
    DECL_LINK( CharSelectHdl, Control* pControl );
    DECL_LINK( CharHighlightHdl, Control* pControl );
    DECL_LINK( CharPreSelectHdl, Control* pControl );
    DECL_LINK( DeleteHdl, PushButton* pBtn );
    DECL_LINK( AssignHdl, PushButton* pBtn );
};

class SvxCharacterMap : public SfxModalDialog
{
private:
    SvxCharMapData* mpCharMapData;

public:
                    SvxCharacterMap( Window* pParent, sal_Bool bOne=sal_True, const SfxItemSet* pSet=0 );
                    ~SvxCharacterMap();

    void            DisableFontSelection();

    const Font&     GetCharFont() const;
    void            SetCharFont( const Font& rFont );

    void            SetChar( sal_UCS4 );
    sal_UCS4        GetChar() const;

    String          GetCharacters() const;

    virtual short   Execute();
};

#endif

