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


#ifndef _SVX_SRCHXTRA_HXX
#define _SVX_SRCHXTRA_HXX

// include ---------------------------------------------------------------

#include <vcl/field.hxx>
#include <svtools/ctrltool.hxx>
#include <sfx2/tabdlg.hxx>

#include <svx/checklbx.hxx>
#include <svx/srchdlg.hxx>

// class SvxSearchFormatDialog -------------------------------------------

class SvxSearchFormatDialog : public SfxTabDialog
{
public:
    SvxSearchFormatDialog( Window* pParent, const SfxItemSet& rSet );
    ~SvxSearchFormatDialog();

protected:
    virtual void    PageCreated( sal_uInt16 nId, SfxTabPage &rPage );

private:
    FontList*       pFontList;
};

// class SvxSearchFormatDialog -------------------------------------------

class SvxSearchAttributeDialog : public ModalDialog
{
public:
    SvxSearchAttributeDialog( Window* pParent, SearchAttrItemList& rLst,
                              const sal_uInt16* pWhRanges );

private:
    FixedText           aAttrFL;
    SvxCheckListBox     aAttrLB;

    OKButton            aOKBtn;
    CancelButton        aEscBtn;
    HelpButton          aHelpBtn;

    SearchAttrItemList& rList;

    DECL_LINK( OKHdl, Button * );
};

// class SvxSearchSimilarityDialog ---------------------------------------

class SvxSearchSimilarityDialog : public ModalDialog
{
private:
    FixedLine           aFixedLine;
    FixedText           aOtherTxt;
    NumericField        aOtherFld;
    FixedText           aLongerTxt;
    NumericField        aLongerFld;
    FixedText           aShorterTxt;
    NumericField        aShorterFld;
    CheckBox            aRelaxBox;

    OKButton            aOKBtn;
    CancelButton        aEscBtn;
    HelpButton          aHelpBtn;

public:
    SvxSearchSimilarityDialog(  Window* pParent,
                                sal_Bool bRelax,
                                sal_uInt16 nOther,
                                sal_uInt16 nShorter,
                                sal_uInt16 nLonger );

    sal_uInt16  GetOther()      { return (sal_uInt16)aOtherFld.GetValue(); }
    sal_uInt16  GetShorter()    { return (sal_uInt16)aShorterFld.GetValue(); }
    sal_uInt16  GetLonger()     { return (sal_uInt16)aLongerFld.GetValue(); }
    sal_Bool    IsRelaxed()     { return aRelaxBox.IsChecked(); }
};


#endif

