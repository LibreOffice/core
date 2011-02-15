/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/

#ifndef SV_PROPERTY_HXX
#define SV_PROPERTY_HXX

#ifndef _VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif
#ifndef SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif
#ifndef SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef SV_GROUP_HXX
#include <vcl/group.hxx>
#endif
#ifndef SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef SV_MOREBTN_HXX
#include <vcl/morebtn.hxx>
#endif
#ifndef _DIALOG_HXX //autogen
#include <vcl/dialog.hxx>
#endif
#include <vcl/combobox.hxx>
#include <vcl/field.hxx>
#include <svl/svarray.hxx>

#define _SVSTDARR_USHORTS
#include <svl/svstdarr.hxx>
#include <vcl/virdev.hxx>
#include <vcl/scrbar.hxx>
#include <svl/svarray.hxx>

#ifndef _SVSTDARR_STRINGS
#define _SVSTDARR_STRINGS
#include <svl/svstdarr.hxx>
#endif
#include <vcl/tabpage.hxx>
#include <vcl/tabctrl.hxx>

//------------------------------------------------------------------------

//========================================================================
enum eKindOfControl { KOC_UNDEFINED=0,KOC_LISTBOX=1, KOC_COMBOBOX=2,
                        KOC_EDIT=3,KOC_USERDEFINED=5};

class SvXPropertyCtrListener;

class SvXPropertyControl : public Control
{
public:
                                        SvXPropertyControl( Window* pParent, WinBits nWinStyle = 0 );
                                        SvXPropertyControl( Window* pParent, const ResId& rResId );

        virtual void                    SetSvXPropertyCtrListener(SvXPropertyCtrListener*)=0;

        virtual SvXPropertyCtrListener* GetSvXPropertyCtrListener()=0;
        virtual void                    SetProperty(const String &rString)=0;
        virtual String                  GetProperty()const=0;
        virtual sal_Bool                    HasList()=0;
        virtual void                    ClearList()=0;
        virtual void                    InsertEntry( const String& rString,
                                                sal_uInt16 nPos = LISTBOX_APPEND )=0;

        virtual void                    SetCtrSize(const Size& rSize)=0;
        virtual void                    SetLocked(sal_Bool bLocked=sal_True)=0;

        virtual void                    SetMyName(const String &rString)=0;
        virtual String                  GetMyName()const=0;

        virtual void                    SetMyData(void*)=0;
        virtual void*                   GetMyData()=0;
};

class SvXPropertyCtrListener
{

public:
        virtual void                    Modified (SvXPropertyControl *pSvXPCtr)=0;
        virtual void                    GetFocus (SvXPropertyControl *pSvXPCtr)=0;
        virtual void                    LoseFocus(SvXPropertyControl *pSvXPCtr)=0;
        virtual void                    KeyInput (SvXPropertyControl *pSvXPCtr ,const KeyCode&)=0;
};


class SvXPropertyEdit : public SvXPropertyControl
{
private:

        String                          aName;
        SvXPropertyCtrListener*         pListener;
        Edit                            aEdit;
        void*                           pData;

        DECL_LINK(ModifiedHdl,Edit*);
        DECL_LINK(GetFocusHdl,Edit*);
        DECL_LINK(LoseFocusHdl,Edit*);

public:
                                        SvXPropertyEdit( Window* pParent, WinBits nWinStyle = 0 );
                                        SvXPropertyEdit( Window* pParent, const ResId& rResId );

        virtual void                    SetSvXPropertyCtrListener(SvXPropertyCtrListener*);

        virtual SvXPropertyCtrListener* GetSvXPropertyCtrListener();

        virtual void                    SetProperty(const String &rString);
        virtual String                  GetProperty()const;

        virtual sal_Bool                    HasList();
        virtual void                    ClearList();
        virtual void                    InsertEntry( const String& rString,
                                                sal_uInt16 nPos = LISTBOX_APPEND );

        virtual void                    SetCtrSize(const Size& rSize);
        virtual void                    SetLocked(sal_Bool bLocked=sal_True);

        virtual void                    SetMyName(const String &rString);
        virtual String                  GetMyName()const;

        virtual void                    SetMyData(void*);
        virtual void*                   GetMyData();
};


class SvXPropertyListBox : public SvXPropertyControl
{
private:

        String                          aName;
        SvXPropertyCtrListener*         pListener;
        ListBox                         aListBox;
        void*                           pData;

        DECL_LINK(ModifiedHdl,ListBox*);
        DECL_LINK(GetFocusHdl,ListBox*);
        DECL_LINK(LoseFocusHdl,ListBox*);


public:
                                        SvXPropertyListBox( Window* pParent, WinBits nWinStyle = 0 );
                                        SvXPropertyListBox( Window* pParent, const ResId& rResId );

        virtual void                    SetSvXPropertyCtrListener(SvXPropertyCtrListener*);

        virtual SvXPropertyCtrListener* GetSvXPropertyCtrListener();

        ListBox*                        GetListBox();

        virtual void                    SetProperty(const String &rString);
        virtual String                  GetProperty()const;

        virtual sal_Bool                    HasList();
        virtual void                    ClearList();
        virtual void                    InsertEntry( const String& rString,
                                                sal_uInt16 nPos = LISTBOX_APPEND );

        virtual void                    SetCtrSize(const Size& rSize);
        virtual void                    SetLocked(sal_Bool bLocked=sal_True);

        virtual void                    SetMyName(const String &rString);
        virtual String                  GetMyName()const;

        virtual void                    SetMyData(void*);
        virtual void*                   GetMyData();
};

class SvXPropertyComboBox : public SvXPropertyControl
{
private:

        String                          aName;
        SvXPropertyCtrListener*         pListener;
        ComboBox                        aComboBox;
        void*                           pData;

        DECL_LINK(ModifiedHdl,ComboBox*);
        DECL_LINK(GetFocusHdl,ComboBox*);
        DECL_LINK(LoseFocusHdl,ComboBox*);


public:
                                        SvXPropertyComboBox( Window* pParent, WinBits nWinStyle = 0 );
                                        SvXPropertyComboBox( Window* pParent, const ResId& rResId );

        virtual void                    SetSvXPropertyCtrListener(SvXPropertyCtrListener*);

        virtual SvXPropertyCtrListener* GetSvXPropertyCtrListener();

        ComboBox*                       GetComboBox();

        virtual void                    SetProperty(const String &rString);
        virtual String                  GetProperty()const;

        virtual sal_Bool                    HasList();
        virtual void                    ClearList();
        virtual void                    InsertEntry( const String& rString,
                                                sal_uInt16 nPos = LISTBOX_APPEND );

        virtual void                    SetCtrSize(const Size& rSize);
        virtual void                    SetLocked(sal_Bool bLocked=sal_True);

        virtual void                    SetMyName(const String &rString);
        virtual String                  GetMyName()const;

        virtual void                    SetMyData(void*);
        virtual void*                   GetMyData();
};



class SvPropertyLine : public Control
{
private:
            FixedText           aName;
            sal_uInt16              nNameWidth;
            sal_Bool                bNeedsRepaint;
            SvXPropertyControl* pSvXPropertyControl;

            PushButton          aXButton;
            sal_Bool                bIsLocked;
            sal_Bool                bHasXButton;
            sal_Bool                bIsHyperlink;
            eKindOfControl      eKindOfCtr;

protected:
            virtual void        Resize();

public:
            SvPropertyLine( Window* pParent,
                            WinBits nWinStyle = 0 );
            SvPropertyLine( Window* pParent,
                            const ResId& rResId );

            sal_Bool                NeedsRepaint();
            void                SetNeedsRepaint(sal_Bool bFlag);
            void                SetSvXPropertyControl(SvXPropertyControl*);
            SvXPropertyControl* GetSvXPropertyControl();

            void                SetKindOfControl(eKindOfControl);
            eKindOfControl      GetKindOfControl();

            void                SetName(const String& rString );
            String              GetName() const;
            void                SetNameWidth(sal_uInt16);

            void                ShowXButton();
            void                HideXButton();
            sal_Bool                IsVisibleXButton();
            void                ShowAsHyperLink(sal_Bool nFlag=sal_True);
            sal_Bool                IsShownAsHyperlink();

            void                Locked(sal_Bool nFlag=sal_True);
            sal_Bool                IsLineLocked();

            void                SetClickHdl(const Link&);

};


class SvPropertyData
{
public:
        eKindOfControl          eKind;
        String                  aName;
        String                  aValue;
        SvStrings               theValues; // ???

        sal_Bool                    bHasVisibleXButton;
        sal_Bool                    bIsHyperLink;
        sal_Bool                    bIsLocked;
        void*                   pDataPtr;
        SvXPropertyControl*     pControl;
};

class SvPropertyDataObjectControl
{
};

class SvPropertyDataControl
{
public:
        virtual void                    Modified(   const String& aName,
                                                    const String& aVal,
                                                    void* pData)=0;

        virtual void                    Clicked(    const String& aName,
                                                    const String& aVal,
                                                    void* pData)=0;

        virtual void                    Commit(     const String& aName,
                                                    const String& aVal,
                                                    void* pData)=0;

        virtual void                    Select(     const String& aName,
                                                    void* pData)=0;

        virtual void                    LinkClicked(const String& aName,
                                                    void* pData)=0;
};

class SvXPropEvListener: public SvXPropertyCtrListener
{
        Link                            aModifyLink;
        Link                            aGetFocusLink;
        Link                            aLoseFocusLink;
        Link                            aKeyInputLink;
        String                          aModifiedResult;

        SvXPropertyControl *            pTheActiveControl;
        KeyCode                         aKeyCode;

public:
                                        SvXPropEvListener();
        virtual                        ~SvXPropEvListener();

        virtual void                    Modified (SvXPropertyControl *pSvXPCtr);
        virtual void                    GetFocus (SvXPropertyControl *pSvXPCtr);
        virtual void                    LoseFocus(SvXPropertyControl *pSvXPCtr);
        virtual void                    KeyInput(SvXPropertyControl *pSvXPCtr ,const KeyCode&);

        SvXPropertyControl *                GetPropertyControl();
        KeyCode                         GetKeyCode() const;

        void                            SetModifyHdl( const Link& rLink ) { aModifyLink = rLink; }
        const Link&                     GetModifyHdl() const { return aModifyLink; }

        void                            SetGetFocusHdl( const Link& rLink ) { aGetFocusLink = rLink; }
        const Link&                     GetGetFocusHdl() const { return aGetFocusLink; }

        void                            SetLoseFocusHdl( const Link& rLink ) { aLoseFocusLink = rLink; }
        const Link&                     GetLoseFocusHdl() const { return aLoseFocusLink; }

        void                            SetKeyInputHdl( const Link& rLink ) { aKeyInputLink = rLink; }
        const Link&                     GetKeyInputHdl() const { return aKeyInputLink; }


};

typedef SvPropertyLine * SvPropertyLinePtr;

SV_DECL_PTRARR(SvPropLineArray,SvPropertyLinePtr,1,1)

class SvListBoxForProperties: public Control
{
private:

            SvXPropEvListener       aListener;
            Window                  aPlayGround;
            ScrollBar               aVScroll;
            SvPropLineArray         PLineArray;
            SvPropertyDataControl*  pPropDataControl;
            sal_uInt16                  nRowHeight;
            sal_Bool                    bUpdate;
            sal_uInt16                  nTheNameSize;
            long                    nYOffset;

            DECL_LINK(   ScrollHdl,ScrollBar*);
            DECL_LINK(   ClickHdl ,PushButton*);

            DECL_LINK(   ModifyHdl,SvXPropEvListener*);
            DECL_LINK( GetFocusHdl,SvXPropEvListener*);
            DECL_LINK(LoseFocusHdl,SvXPropEvListener*);
            DECL_LINK( KeyInputHdl,SvXPropEvListener*);

protected:
            void                    UpdateAll();
            void                    UpdatePosNSize();
            void                    UpdatePlayGround();
            void                    UpdateVScroll();

            void                    Resize();

public:
                                    SvListBoxForProperties( Window* pParent, WinBits nWinStyle = 0 );
                                    SvListBoxForProperties( Window* pParent, const ResId& rResId );

                                    ~SvListBoxForProperties();

            virtual sal_uInt16          CalcVisibleLines();
            virtual void            EnableUpdate();
            virtual void            DisableUpdate();

            virtual void            SetController(SvPropertyDataControl *);

            virtual void            Clear();

            virtual sal_uInt16          InsertEntry( const SvPropertyData&, sal_uInt16 nPos = LISTBOX_APPEND );

            virtual void            ChangeEntry( const SvPropertyData&, sal_uInt16 nPos);

            virtual sal_uInt16          AppendEntry( const SvPropertyData&);

            virtual void            SetPropertyValue( const String & rEntryName, const String & rValue );

            virtual void            SetFirstVisibleEntry(sal_uInt16 nPos);
            virtual sal_uInt16          GetFirstVisibleEntry();

            virtual void            SetSelectedEntry(sal_uInt16 nPos);
            virtual sal_uInt16          GetSelectedEntry();
};

class SvTabPageForProperties: public TabPage
{

private:

            SvListBoxForProperties  aLbProp;

protected:

            virtual void            Resize();

public:
            SvTabPageForProperties(Window* pParent,WinBits nWinStyle = 0 );

            SvListBoxForProperties*   GetTheListBox();
};


class SvBasicPropertyDataControl: public SvPropertyDataControl
{
private:

        sal_Bool                            bCorrectness;
        String                          aEntryName;
        String                          aEntryProperty;
        String                          aCorrectProperty;
        void*                           pTheData;
        Link                            aModifyLink;
        Link                            aClickedLink;
        Link                            aCommitLink;
        Link                            aSelectLink;

public:
        virtual ~SvBasicPropertyDataControl();

        virtual void                    Modified(   const String& aName,
                                                    const String& aVal,
                                                    void* pData);       //User has modified Property

        virtual void                    Clicked(    const String& aName,
                                                    const String& aVal,
                                                    void* pData);       //Xtension-Button pressed

        virtual void                    Commit(     const String& aName,
                                                    const String& aVal,
                                                    void* pData);       //User accept changes

        virtual void                    Select(     const String& aName,
                                                    void* pData);       //User select new Row

        virtual void                    LinkClicked(const String& aName,
                                                    void* pData);

        virtual void                    SetIsCorrect(sal_Bool nFlag);

        //virtual String                    GetTheCorrectProperty()const;
        virtual void                    SetTheCorrectProperty(const String& aName);

        String                          GetName() const; //Tell's the name of the Property
        String                          GetProperty() const; //Tell's the content of the Property
        void*                           GetData(); //Tell's the storage


        void                            SetModifyHdl( const Link& rLink ) { aModifyLink = rLink; }
        const Link&                     GetModifyHdl() const { return aModifyLink; }

        void                            SetClickedHdl( const Link& rLink ) { aClickedLink = rLink; }
        const Link&                     GetClickedHdl() const { return aClickedLink; }

        void                            SetCommitHdl( const Link& rLink ) { aCommitLink = rLink; }
        const Link&                     GetCommitHdl() const { return aCommitLink; }

        void                            SetSelectHdl( const Link& rLink ) { aSelectLink = rLink; }
        const Link&                     GetSelectHdl() const { return aSelectLink; }

};


class SvPropertyBox: public Control
{
private:
            SvPropertyDataControl*      pThePropDataCtr;
            TabControl                  aTabControl;

protected:
            virtual void                Resize();

public:
                                        SvPropertyBox   ( Window* pParent, WinBits nWinStyle = 0 );
                                        SvPropertyBox   ( Window* pParent, const ResId& rResId );

                                        ~SvPropertyBox();

            virtual sal_uInt16              CalcVisibleLines();
            virtual void                EnableUpdate();     // auch IDL?
            virtual void                DisableUpdate();    // auch IDL?

            // AB: Hier beginnt das 'offizielle' Interface, das in IDL uebernommen werden soll
            virtual void                SetController(SvPropertyDataControl *);

            virtual sal_uInt16              AppendPage( const String & r );
            virtual void                SetPage( sal_uInt16 );
            virtual sal_uInt16              GetCurPage();
            virtual void                ClearAll();
            virtual void                ClearTable();

            virtual void                SetPropertyValue( const String & rEntryName, const String & rValue );

            virtual sal_uInt16              InsertEntry( const SvPropertyData&, sal_uInt16 nPos = LISTBOX_APPEND );
            virtual void                ChangeEntry( const SvPropertyData&, sal_uInt16 nPos);
            virtual sal_uInt16              AppendEntry( const SvPropertyData&);

            virtual void                SetFirstVisibleEntry(sal_uInt16 nPos);
            virtual sal_uInt16              GetFirstVisibleEntry();

            virtual void                SetSelectedEntry(sal_uInt16 nPos);
            virtual sal_uInt16              GetSelectedEntry();
};



/*
class ScPropertyDlg : public ModalDialog
{
private:
            SvBasicPropertyDataControl  aBaProDatCtr;
            OKButton                    anOk;
            CancelButton                aCancel;
            sal_uInt16                      nCount;
            sal_uInt16                      nClickCount;

            SvPropertyData              aProperty;
            SvPropertyBox               aPropListBox;

            ListBox                     aKindOfListBox;
            FixedText                   aModAnswer;
            FixedText                   aClickAnswer;
            FixedText                   aCommitAnswer;
            FixedText                   aSelectAnswer;

            DECL_LINK( ModifiedHdl, ListBox*);

            DECL_LINK( RowModifiedHdl, SvBasicPropertyDataControl*);
            DECL_LINK( ClickHdl      , SvBasicPropertyDataControl*);
            DECL_LINK( SelectHdl     , SvBasicPropertyDataControl*);
            DECL_LINK( CommitHdl     , SvBasicPropertyDataControl*);

public:
            ScPropertyDlg( Window*  pParent);
            ~ScPropertyDlg();
};
*/
#endif // SC_AUTOFMT_HXX


