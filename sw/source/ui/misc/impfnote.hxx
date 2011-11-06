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


#ifndef _IMPFNOTE_HXX
#define _IMPFNOTE_HXX

#include <sfx2/tabdlg.hxx>
#ifndef _BUTTON_HXX //autogen
#include <vcl/button.hxx>
#endif
#ifndef _LSTBOX_HXX //autogen
#include <vcl/lstbox.hxx>
#endif
#ifndef _FIXED_HXX //autogen
#include <vcl/fixed.hxx>
#endif
#ifndef _FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#include <numberingtypelistbox.hxx>

class SwWrtShell;

class SwEndNoteOptionPage : public SfxTabPage
{
    FixedLine       aNumFL;
    FixedText       aNumTypeFT;
    SwNumberingTypeListBox   aNumViewBox;
    FixedText       aOffsetLbl;
    NumericField    aOffsetFld;
    FixedText       aNumCountFT;
    ListBox         aNumCountBox;
    FixedText       aPrefixFT;
    Edit            aPrefixED;
    FixedText       aSuffixFT;
    Edit            aSuffixED;
    FixedText       aPosFT;
    RadioButton     aPosPageBox;
    RadioButton     aPosChapterBox;

    FixedLine        aTemplFL;
    FixedText       aParaTemplLbl;
    ListBox         aParaTemplBox;
    FixedText       aPageTemplLbl;
    ListBox         aPageTemplBox;

    FixedLine        aCharTemplFL;
    FixedText       aFtnCharAnchorTemplLbl;
    ListBox         aFtnCharAnchorTemplBox;
    FixedText       aFtnCharTextTemplLbl;
    ListBox         aFtnCharTextTemplBox;

    FixedLine aContFL;
    FixedText aContLbl;
    Edit aContEdit;
    FixedText aContFromLbl;
    Edit aContFromEdit;

    String aNumDoc;
    String aNumPage;
    String aNumChapter;
    SwWrtShell *pSh;
    sal_Bool    bPosDoc;
    sal_Bool    bEndNote;

    inline void SelectNumbering(int eNum);
    int GetNumbering() const;

    DECL_LINK( PosPageHdl, Button * );
    DECL_LINK( PosChapterHdl, Button * );
    DECL_LINK( NumCountHdl, ListBox * );


public:
    SwEndNoteOptionPage( Window *pParent, sal_Bool bEndNote,
                         const SfxItemSet &rSet );
    ~SwEndNoteOptionPage();

    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
    virtual sal_Bool FillItemSet(SfxItemSet &rSet);
    virtual void Reset( const SfxItemSet& );

    void SetShell( SwWrtShell &rShell );
};

class SwFootNoteOptionPage : public SwEndNoteOptionPage
{
    SwFootNoteOptionPage( Window *pParent, const SfxItemSet &rSet );
    ~SwFootNoteOptionPage();

public:
    static SfxTabPage *Create(Window *pParent, const SfxItemSet &rSet);
};



#endif
