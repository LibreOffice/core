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



#ifndef _SVX_THESDLG_IMPL_HXX
#define _SVX_THESDLG_IMPL_HXX

#include "thesdlg.hxx"

#include <svtools/ehdl.hxx>
#include <svx/checklbx.hxx>
#include <vcl/button.hxx>
#include <vcl/combobox.hxx>
#include <vcl/edit.hxx>
#include <vcl/fixed.hxx>
#include <vcl/image.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/menubtn.hxx>
#include <vcl/msgbox.hxx>
#include <vcl/svapp.hxx>
#include <vcl/timer.hxx>
#include <vcl/wrkwin.hxx>

#include <com/sun/star/linguistic2/XThesaurus.hpp>
#include <com/sun/star/linguistic2/XMeaning.hpp>

#include <stack>
#include <map>
#include <algorithm>

using namespace ::com::sun::star;
using ::rtl::OUString;

class SvLBoxEntry;
class ThesaurusAlternativesCtrl_Impl;


// class LookUpComboBox_Impl --------------------------------------------------

class LookUpComboBox_Impl : public ComboBox
{
    Timer                       m_aModifyTimer;
    Selection                   m_aSelection;
    Button *                    m_pBtn;
    SvxThesaurusDialog_Impl &   m_rDialogImpl;

    // disable copy c-tor and assignment operator
    LookUpComboBox_Impl( const LookUpComboBox_Impl & );
    LookUpComboBox_Impl & operator = ( const LookUpComboBox_Impl & );

public:
    LookUpComboBox_Impl( Window *pParent, const ResId &rResId, SvxThesaurusDialog_Impl &rImpl );
    virtual ~LookUpComboBox_Impl();

    DECL_LINK( ModifyTimer_Hdl, Timer * );

    void SetButton( Button *pBtn )  { m_pBtn = pBtn; }

    // ComboBox
    virtual void        Modify();
};

// class ReplaceEdit_Impl --------------------------------------------------

class ReplaceEdit_Impl : public Edit
{
    Button *                    m_pBtn;

    // disable copy c-tor and assignment operator
    ReplaceEdit_Impl( const ReplaceEdit_Impl & );
    ReplaceEdit_Impl & operator = ( const ReplaceEdit_Impl & );

public:
    ReplaceEdit_Impl( Window *pParent, const ResId &rResId );
    virtual ~ReplaceEdit_Impl();

    void SetButton( Button *pBtn )  { m_pBtn = pBtn; }

    // Edit
    virtual void        Modify();
    virtual void        SetText( const XubString& rStr );
    virtual void        SetText( const XubString& rStr, const Selection& rNewSelection );
};

// class ThesaurusAlternativesCtrl_Impl ----------------------------------

class AlternativesExtraData
{
    String  sText;
    bool    bHeader;

public:
    AlternativesExtraData() : bHeader( false ) {}
    AlternativesExtraData( const String &rText, bool bIsHeader ) :
        sText(rText),
        bHeader(bIsHeader)
        {
        }

    bool  IsHeader() const          { return bHeader; }
    const String& GetText() const   { return sText; }
};


class AlternativesString_Impl : public SvLBoxString
{
    ThesaurusAlternativesCtrl_Impl &    m_rControlImpl;
public:

    AlternativesString_Impl( ThesaurusAlternativesCtrl_Impl &rControl,
        SvLBoxEntry* pEntry, sal_uInt16 nFlags, const String& rStr );

    virtual void Paint( const Point& rPos, SvLBox& rDev, sal_uInt16 nFlags, SvLBoxEntry* pEntry);
};


class ThesaurusAlternativesCtrl_Impl :
    public SvxCheckListBox
{
    SvxThesaurusDialog_Impl &   m_rDialogImpl;

    typedef std::map< const SvLBoxEntry *, AlternativesExtraData >  UserDataMap_t;
    UserDataMap_t           m_aUserData;

    // disable copy c-tor and assignment operator
    ThesaurusAlternativesCtrl_Impl( const ThesaurusAlternativesCtrl_Impl & );
    ThesaurusAlternativesCtrl_Impl & operator = ( const ThesaurusAlternativesCtrl_Impl & );

public:
    ThesaurusAlternativesCtrl_Impl( Window* pParent, SvxThesaurusDialog_Impl &rImpl );
    virtual ~ThesaurusAlternativesCtrl_Impl();


    SvLBoxEntry *   AddEntry( sal_Int32 nVal, const String &rText, bool bIsHeader );

    void            ClearExtraData();
    void            SetExtraData( const SvLBoxEntry *pEntry, const AlternativesExtraData &rData );
    AlternativesExtraData * GetExtraData( const SvLBoxEntry *pEntry );

    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    Paint( const Rectangle& rRect );
};


// struct SvxThesaurusDialog_Impl ----------------------------------------

struct SvxThesaurusDialog_Impl
{
    SvxThesaurusDialog *    m_pDialog;

    FixedImage              aVendorImageFI;
    ImageButton             aLeftBtn;
    FixedText               aWordText;
    LookUpComboBox_Impl     aWordCB;
    FixedText               m_aAlternativesText;
    boost::shared_ptr< ThesaurusAlternativesCtrl_Impl > m_pAlternativesCT;
    FixedText               aReplaceText;
    ReplaceEdit_Impl        aReplaceEdit;
    FixedLine               aFL;
    HelpButton              aHelpBtn;
    MenuButton              aLangMBtn;
    OKButton                aReplaceBtn;
    CancelButton            aCancelBtn;

    String          aErrStr;
    Image           aVendorDefaultImage;
    Image           aVendorDefaultImageHC;

    uno::Reference< linguistic2::XThesaurus >   xThesaurus;
    OUString                aLookUpText;
    LanguageType            nLookUpLanguage;
    std::stack< OUString >  aLookUpHistory;
    bool                    m_bWordFound;


    // Handler
    DECL_LINK( LeftBtnHdl_Impl, Button * );
    DECL_LINK( LanguageHdl_Impl, MenuButton * );
    DECL_LINK( LookUpHdl_Impl, Button * );
    DECL_LINK( WordSelectHdl_Impl, ComboBox * );
    DECL_LINK( AlternativesSelectHdl_Impl, SvxCheckListBox * );
    DECL_LINK( AlternativesDoubleClickHdl_Impl, SvxCheckListBox * );

    DECL_STATIC_LINK( SvxThesaurusDialog_Impl, SelectFirstHdl_Impl, SvxCheckListBox * );
    DECL_STATIC_LINK( SvxThesaurusDialog_Impl, VendorImageInitHdl, SvxThesaurusDialog_Impl * );


    SvxThesaurusDialog_Impl( SvxThesaurusDialog * pDialog );
    ~SvxThesaurusDialog_Impl();

    uno::Sequence< uno::Reference< linguistic2::XMeaning > > SAL_CALL
            queryMeanings_Impl( ::rtl::OUString& rTerm, const lang::Locale& rLocale, const beans::PropertyValues& rProperties ) throw(lang::IllegalArgumentException, uno::RuntimeException);

    bool    UpdateAlternativesBox_Impl();
    void    UpdateVendorImage();
    void    LookUp( const String &rText );
    void    LookUp_Impl();
};

#endif

