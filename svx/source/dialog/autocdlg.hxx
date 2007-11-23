/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: autocdlg.hxx,v $
 *
 *  $Revision: 1.10 $
 *
 *  last change: $Author: ihi $ $Date: 2007-11-23 16:37:17 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef _OFA_AUTOCDLG_HXX
#define _OFA_AUTOCDLG_HXX

#include <svtools/langtab.hxx>

#ifndef _SFXTABDLG_HXX
#include <sfx2/tabdlg.hxx>
#endif

#ifndef _TOOLS_TABLE_HXX //autogen
#include <tools/table.hxx>
#endif
#ifndef _SVX_CHECKLBX_HXX //autogen
#include <svx/checklbx.hxx>
#endif
#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif
#ifndef _SV_METRIC_HXX //autogen
#include <vcl/metric.hxx>
#endif
#ifndef _SVX_LANGBOX_HXX
#include <svx/langbox.hxx>
#endif

class SvxAutoCorrect;
class CharClass;
class CollatorWrapper;
class SmartTagMgr;

// class OfaAutoCorrDlg --------------------------------------------------

class OfaAutoCorrDlg : public SfxTabDialog
{
    FixedText       aLanguageFT;
    SvxLanguageBox  aLanguageLB;

    DECL_LINK(SelectLanguageHdl, ListBox*);
public:

    OfaAutoCorrDlg(Window* pParent, const SfxItemSet *pSet );

    void    EnableLanguage(BOOL bEnable)
            {   aLanguageFT.Enable(bEnable);
                aLanguageLB.Enable(bEnable);}

};

#ifdef _OFA_AUTOCDLG_CXX

#ifndef _SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SVTABBX_HXX
#include <svtools/svtabbx.hxx>
#endif
#ifndef _SVX_SIMPTABL_HXX //autogen
#include <svx/simptabl.hxx>
#endif

class SvStringsISortDtor;

// class OfaACorrCheckListBox ------------------------------------------

class OfaACorrCheckListBox : public SvxSimpleTable
{
    using SvxSimpleTable::SetTabs;
    using SvTreeListBox::GetCheckButtonState;
    using SvTreeListBox::SetCheckButtonState;

    protected:
        virtual void    SetTabs();
        virtual void    HBarClick();
        virtual void    KeyInput( const KeyEvent& rKEvt );

    public:
        OfaACorrCheckListBox(Window* pParent, const ResId& rResId ) :
            SvxSimpleTable( pParent, rResId ){}

        inline void *GetUserData(ULONG nPos) { return GetEntry(nPos)->GetUserData(); }
        inline void SetUserData(ULONG nPos, void *pData ) { GetEntry(nPos)->SetUserData(pData); }
        inline ULONG GetSelectEntryPos() { return GetModel()->GetAbsPos(FirstSelected()); }

        BOOL            IsChecked(ULONG nPos, USHORT nCol = 0);
        void            CheckEntryPos(ULONG nPos, USHORT nCol, BOOL bChecked);
        SvButtonState   GetCheckButtonState( SvLBoxEntry*, USHORT nCol ) const;
        void            SetCheckButtonState( SvLBoxEntry*, USHORT nCol, SvButtonState );
};

// class OfaAutocorrOptionsPage ------------------------------------------


class OfaAutocorrOptionsPage : public SfxTabPage
{
    using TabPage::ActivatePage;

private:
    SvxCheckListBox aCheckLB;

    String      sInput;
    String      sDoubleCaps;
    String      sStartCap;
    String      sBoldUnderline;
    String      sURL;
    String      sNoDblSpaces;
    String      sHalf;
    String      sDash;
    String      sFirst;

public:
                        OfaAutocorrOptionsPage( Window* pParent, const SfxItemSet& rSet );
                        ~OfaAutocorrOptionsPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        ActivatePage( const SfxItemSet& );

};

// class OfaSwAutoFmtOptionsPage ----------------------------------------------------

class OfaSwAutoFmtOptionsPage : public SfxTabPage
{
    using TabPage::ActivatePage;

    OfaACorrCheckListBox    aCheckLB;
    PushButton      aEditPB;
    FixedText       aHeader1Expl;
    FixedText       aHeader2Expl;

    String          sHeader1;
    String          sHeader2;

    String          sDeleteEmptyPara;
    String          sUseReplaceTbl;
    String          sCptlSttWord;
    String          sCptlSttSent;
    String          sTypo;
    String          sUserStyle;
    String          sBullet;
    String          sByInputBullet;
    String          sBoldUnder;
    String          sNoDblSpaces;
    String          sFraction;
    String          sDetectURL;
    String          sDash;
    String          sOrdinal;
    String          sRightMargin;
    String          sNum;
    String          sBorder;
    String          sTable;
    String          sReplaceTemplates;
    String          sDelSpaceAtSttEnd;
    String          sDelSpaceBetweenLines;

    String          sMargin;
    String          sBulletChar;
    String          sByInputBulletChar;

    Font            aBulletFont;
    Font            aByInputBulletFont;
    USHORT          nPercent;

    SvLBoxButtonData*   pCheckButtonData;

        DECL_LINK(SelectHdl, OfaACorrCheckListBox*);
        DECL_LINK(EditHdl, PushButton*);
        SvLBoxEntry* CreateEntry(String& rTxt, USHORT nCol);


        OfaSwAutoFmtOptionsPage( Window* pParent,
                            const SfxItemSet& rSet );
        ~OfaSwAutoFmtOptionsPage();

    public:
        static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);
        virtual BOOL        FillItemSet( SfxItemSet& rSet );
        virtual void        Reset( const SfxItemSet& rSet );
        virtual void        ActivatePage( const SfxItemSet& );
};

// class AutoCorrEdit ----------------------------------------------------

class AutoCorrEdit : public Edit
{
    Link    aActionLink;
    BOOL    bSpaces;

    public:
                    AutoCorrEdit(Window* pParent, const ResId& rResId) :
                        Edit(pParent, rResId), bSpaces(FALSE){}

    void            SetActionHdl( const Link& rLink )
                                { aActionLink = rLink;}

    void            SetSpaces(BOOL bSet)
                                {bSpaces = bSet;}

    virtual void    KeyInput( const KeyEvent& rKEvent );
};

// class OfaAutocorrReplacePage ------------------------------------------

class DoubleStringArray;
typedef DoubleStringArray* DoubleStringArrayPtr;
DECLARE_TABLE(DoubleStringTable, DoubleStringArrayPtr)

class OfaAutocorrReplacePage : public SfxTabPage
{
        using TabPage::ActivatePage;
        using TabPage::DeactivatePage;

private:
        CheckBox        aTextOnlyCB;
        FixedText       aShortFT;
        AutoCorrEdit    aShortED;
        FixedText       aReplaceFT;
        AutoCorrEdit    aReplaceED;
        SvTabListBox    aReplaceTLB;
        PushButton      aNewReplacePB;
        PushButton      aDeleteReplacePB;

        String          sModify;
        String          sNew;

        SvStringsISortDtor*     pFormatText;
        DoubleStringTable       aDoubleStringTable;
        CollatorWrapper*        pCompareClass;
        CollatorWrapper*        pCompareCaseClass;
        CharClass*              pCharClass;
        LanguageType            eLang;

        BOOL            bHasSelectionText;
        BOOL            bFirstSelect:1;
        BOOL            bReplaceEditChanged:1;
        BOOL            bSWriter:1;

        DECL_LINK(SelectHdl, SvTabListBox*);
        DECL_LINK(NewDelHdl, PushButton*);
        DECL_LINK(ModifyHdl, Edit*);

        void            RefillReplaceBox(BOOL bFromReset, //Box mit neuer Sprache fuellen
                                        LanguageType eOldLanguage,
                                        LanguageType eNewLanguage);

public:
                        OfaAutocorrReplacePage( Window* pParent, const SfxItemSet& rSet );
                        ~OfaAutocorrReplacePage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );

    void                SetLanguage(LanguageType eSet);
};

// class OfaAutocorrExceptPage ---------------------------------------------

struct StringsArrays;
typedef StringsArrays* StringsArraysPtr;
DECLARE_TABLE(StringsTable, StringsArraysPtr)

class OfaAutocorrExceptPage : public SfxTabPage
{
    using TabPage::ActivatePage;
    using TabPage::DeactivatePage;

private:
        FixedLine       aAbbrevFL;
        AutoCorrEdit    aAbbrevED;
        ListBox         aAbbrevLB;
        PushButton      aNewAbbrevPB;
        PushButton      aDelAbbrevPB;
        CheckBox        aAutoAbbrevCB;

        FixedLine       aDoubleCapsFL;
        AutoCorrEdit    aDoubleCapsED;
        ListBox         aDoubleCapsLB;
        PushButton      aNewDoublePB;
        PushButton      aDelDoublePB;
        CheckBox        aAutoCapsCB;

        StringsTable    aStringsTable;
        CollatorWrapper* pCompareClass;
        LanguageType    eLang;

    DECL_LINK(NewDelHdl, PushButton*);
    DECL_LINK(SelectHdl, ListBox*);
    DECL_LINK(ModifyHdl, Edit*);

    void            RefillReplaceBoxes(BOOL bFromReset, //Box mit neuer Sprache fuellen
                                        LanguageType eOldLanguage,
                                        LanguageType eNewLanguage);
public:
                        OfaAutocorrExceptPage( Window* pParent, const SfxItemSet& rSet );
                        ~OfaAutocorrExceptPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        ActivatePage( const SfxItemSet& );
    virtual int         DeactivatePage( SfxItemSet* pSet = 0 );
    void                SetLanguage(LanguageType eSet);

};

// class OfaQuoteTabPage -------------------------------------------------

class OfaQuoteTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;

private:

    FixedLine   aSingleFL;
    CheckBox    aSingleTypoCB;
    FixedText   aSglStartQuoteFT;
    PushButton  aSglStartQuotePB;
    FixedText   aSglStartExFT;
    FixedText   aSglEndQuoteFT;
    PushButton  aSglEndQuotePB;
    FixedText   aSglEndExFT;
    PushButton  aSglStandardPB;

    FixedLine   aDoubleFL;
    CheckBox    aTypoCB;
    FixedText   aStartQuoteFT;
    PushButton  aStartQuotePB;
    FixedText   aDblStartExFT;
    FixedText   aEndQuoteFT;
    PushButton  aEndQuotePB;
    FixedText   aDblEndExFT;
    PushButton  aDblStandardPB;

    String      sStartQuoteDlg;
    String      sEndQuoteDlg;

    String      sStandard;


    sal_UCS4    cSglStartQuote;
    sal_UCS4    cSglEndQuote;

    sal_UCS4    cStartQuote;
    sal_UCS4    cEndQuote;

    DECL_LINK( QuoteHdl, PushButton* );
    DECL_LINK( StdQuoteHdl, PushButton* );

    String              ChangeStringExt_Impl( sal_UCS4 );

                        OfaQuoteTabPage( Window* pParent, const SfxItemSet& rSet );
public:
                        ~OfaQuoteTabPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        ActivatePage( const SfxItemSet& );
};

// class OfaAutoCompleteTabPage ---------------------------------------------

class OfaAutoCompleteTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;

    class AutoCompleteMultiListBox : public MultiListBox
    {
        OfaAutoCompleteTabPage& rPage;
    public:
        AutoCompleteMultiListBox( OfaAutoCompleteTabPage& rPg,
                                    const ResId& rResId )
            : MultiListBox( &rPg, rResId ), rPage( rPg ) {}

        virtual long PreNotify( NotifyEvent& rNEvt );
    };

    CheckBox        aCBActiv; //Enable word completion
    CheckBox        aCBAppendSpace;//Append space
    CheckBox        aCBAsTip; //Show as tip

    CheckBox        aCBCollect;//Collect words
    CheckBox        aCBKeepList;//...save the list for later use...
    //--removed--CheckBox        aCBEndless;//

    FixedText       aFTExpandKey;
    ListBox         aDCBExpandKey;
    FixedText       aFTMinWordlen;
    NumericField    aNFMinWordlen;
    FixedText       aFTMaxEntries;
    NumericField    aNFMaxEntries;
    AutoCompleteMultiListBox    aLBEntries;
    PushButton                  aPBEntries;
    SvStringsISortDtor*         pAutoCmpltList;
    USHORT                      nAutoCmpltListCnt;

    DECL_LINK( CheckHdl, CheckBox* );

                        OfaAutoCompleteTabPage( Window* pParent,
                                                const SfxItemSet& rSet );
public:
                        virtual ~OfaAutoCompleteTabPage();

    static SfxTabPage*  Create( Window* pParent,
                                const SfxItemSet& rAttrSet);

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        ActivatePage( const SfxItemSet& );

    void CopyToClipboard() const;
    DECL_LINK( DeleteHdl, PushButton* );
};

// class OfaSmartTagOptionsTabPage ---------------------------------------------

/** Smart tag options tab page

    This tab page is used to enable/disable smart tag types
*/
class OfaSmartTagOptionsTabPage : public SfxTabPage
{
    using TabPage::ActivatePage;

private:

    // controls
    CheckBox                m_aMainCB;
    SvxCheckListBox         m_aSmartTagTypesLB;
    PushButton              m_aPropertiesPB;
    FixedText               m_aTitleFT;

    // construction via Create()
    OfaSmartTagOptionsTabPage( Window* pParent, const SfxItemSet& rSet );

    /** Inserts items into m_aSmartTagTypesLB

        Reads out the smart tag types supported by the SmartTagMgr and
        inserts the associated strings into the list box.
    */
    void FillListBox( const SmartTagMgr& rSmartTagMgr );

    /** Clears the m_aSmartTagTypesLB
    */
    void ClearListBox();

    /** Handler for the check box

        Enables/disables all controls in the tab page (except from the
        check box.
    */
    DECL_LINK( CheckHdl, CheckBox* );

    /** Handler for the push button

        Calls the displayPropertyPage function of the smart tag recognizer
        associated with the currently selected smart tag type.
    */
    DECL_LINK( ClickHdl, PushButton* );

    /** Handler for the list box

        Enables/disables the properties push button if selection in the
        smart tag types list box changes.
    */
    DECL_LINK( SelectHdl, SvxCheckListBox* );

public:

    virtual ~OfaSmartTagOptionsTabPage();

    static SfxTabPage*  Create( Window* pParent, const SfxItemSet& rAttrSet);

    virtual BOOL        FillItemSet( SfxItemSet& rSet );
    virtual void        Reset( const SfxItemSet& rSet );
    virtual void        ActivatePage( const SfxItemSet& );
};

#endif // _OFA_AUTOCDLG_CXX

#endif //

