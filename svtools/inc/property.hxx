/*************************************************************************
 *
 *  $RCSfile: property.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:58:52 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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
#ifndef _SV_COMBOBOX_HXX //autogen
#include <vcl/combobox.hxx>
#endif
#ifndef _SV_FIELD_HXX //autogen
#include <vcl/field.hxx>
#endif

#ifndef _SVARRAY_HXX //autogen
#include <svarray.hxx>
#endif

#define _SVSTDARR_USHORTS
#include <svstdarr.hxx>

#ifndef _SV_VIRDEV_HXX //autogen
#include <vcl/virdev.hxx>
#endif

#ifndef _SV_SCRBAR_HXX //autogen
#include <vcl/scrbar.hxx>
#endif

#ifndef _SVARRAY_HXX //autogen
#include <svarray.hxx>
#endif

#ifndef _SVSTDARR_STRINGS
#define _SVSTDARR_STRINGS
#include <svstdarr.hxx>
#endif

#ifndef _SV_TABPAGE_HXX //autogen
#include <vcl/tabpage.hxx>
#endif

#ifndef _SV_TABCTRL_HXX //autogen
#include <vcl/tabctrl.hxx>
#endif

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
        virtual BOOL                    HasList()=0;
        virtual void                    ClearList()=0;
        virtual void                    InsertEntry( const String& rString,
                                                USHORT nPos = LISTBOX_APPEND )=0;

        virtual void                    SetCtrSize(const Size& rSize)=0;
        virtual void                    SetLocked(BOOL bLocked=TRUE)=0;

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

        virtual BOOL                    HasList();
        virtual void                    ClearList();
        virtual void                    InsertEntry( const String& rString,
                                                USHORT nPos = LISTBOX_APPEND );

        virtual void                    SetCtrSize(const Size& rSize);
        virtual void                    SetLocked(BOOL bLocked=TRUE);

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

        virtual BOOL                    HasList();
        virtual void                    ClearList();
        virtual void                    InsertEntry( const String& rString,
                                                USHORT nPos = LISTBOX_APPEND );

        virtual void                    SetCtrSize(const Size& rSize);
        virtual void                    SetLocked(BOOL bLocked=TRUE);

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

        virtual BOOL                    HasList();
        virtual void                    ClearList();
        virtual void                    InsertEntry( const String& rString,
                                                USHORT nPos = LISTBOX_APPEND );

        virtual void                    SetCtrSize(const Size& rSize);
        virtual void                    SetLocked(BOOL bLocked=TRUE);

        virtual void                    SetMyName(const String &rString);
        virtual String                  GetMyName()const;

        virtual void                    SetMyData(void*);
        virtual void*                   GetMyData();
};



class SvPropertyLine : public Control
{
private:
            FixedText           aName;
            USHORT              nNameWidth;
            BOOL                bNeedsRepaint;
            SvXPropertyControl* pSvXPropertyControl;

            PushButton          aXButton;
            BOOL                bIsLocked;
            BOOL                bHasXButton;
            BOOL                bIsHyperlink;
            eKindOfControl      eKindOfCtr;

protected:
            virtual void        Resize();

public:
            SvPropertyLine( Window* pParent,
                            WinBits nWinStyle = 0 );
            SvPropertyLine( Window* pParent,
                            const ResId& rResId );

            BOOL                NeedsRepaint();
            void                SetNeedsRepaint(BOOL bFlag);
            void                SetSvXPropertyControl(SvXPropertyControl*);
            SvXPropertyControl* GetSvXPropertyControl();

            void                SetKindOfControl(eKindOfControl);
            eKindOfControl      GetKindOfControl();

            void                SetName(const String& rString );
            String              GetName() const;
            void                SetNameWidth(USHORT);

            void                ShowXButton();
            void                HideXButton();
            BOOL                IsVisibleXButton();
            void                ShowAsHyperLink(BOOL nFlag=TRUE);
            BOOL                IsShownAsHyperlink();

            void                Locked(BOOL nFlag=TRUE);
            BOOL                IsLocked();

            void                SetClickHdl(const Link&);

};


class SvPropertyData
{
public:
        eKindOfControl          eKind;
        String                  aName;
        String                  aValue;
        SvStrings               theValues; // ???

        BOOL                    bHasVisibleXButton;
        BOOL                    bIsHyperLink;
        BOOL                    bIsLocked;
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

SV_DECL_PTRARR(SvPropLineArray,SvPropertyLinePtr,1,1);

class SvListBoxForProperties: public Control
{
private:

            SvXPropEvListener       aListener;
            Window                  aPlayGround;
            ScrollBar               aVScroll;
            SvPropLineArray         PLineArray;
            SvPropertyDataControl*  pPropDataControl;
            USHORT                  nRowHeight;
            BOOL                    bUpdate;
            USHORT                  nTheNameSize;
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

            virtual USHORT          CalcVisibleLines();
            virtual void            EnableUpdate();
            virtual void            DisableUpdate();

            virtual void            SetController(SvPropertyDataControl *);

            virtual void            Clear();

            virtual USHORT          InsertEntry( const SvPropertyData&, USHORT nPos = LISTBOX_APPEND );

            virtual void            ChangeEntry( const SvPropertyData&, USHORT nPos);

            virtual USHORT          AppendEntry( const SvPropertyData&);

            virtual void            SetPropertyValue( const String & rEntryName, const String & rValue );

            virtual void            SetFirstVisibleEntry(USHORT nPos);
            virtual USHORT          GetFirstVisibleEntry();

            virtual void            SetSelectedEntry(USHORT nPos);
            virtual USHORT          GetSelectedEntry();
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

        BOOL                            bCorrectness;
        String                          aEntryName;
        String                          aEntryProperty;
        String                          aCorrectProperty;
        void*                           pTheData;
        Link                            aModifyLink;
        Link                            aClickedLink;
        Link                            aCommitLink;
        Link                            aSelectLink;

public:

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

        virtual void                    SetIsCorrect(BOOL nFlag);

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

            virtual USHORT              CalcVisibleLines();
            virtual void                EnableUpdate();     // auch IDL?
            virtual void                DisableUpdate();    // auch IDL?

            // AB: Hier beginnt das 'offizielle' Interface, das in IDL uebernommen werden soll
            virtual void                SetController(SvPropertyDataControl *);

            virtual USHORT              AppendPage( const String & r );
            virtual void                SetPage( USHORT );
            virtual USHORT              GetCurPage();
            virtual void                ClearAll();
            virtual void                ClearTable();

            virtual void                SetPropertyValue( const String & rEntryName, const String & rValue );

            virtual USHORT              InsertEntry( const SvPropertyData&, USHORT nPos = LISTBOX_APPEND );
            virtual void                ChangeEntry( const SvPropertyData&, USHORT nPos);
            virtual USHORT              AppendEntry( const SvPropertyData&);

            virtual void                SetFirstVisibleEntry(USHORT nPos);
            virtual USHORT              GetFirstVisibleEntry();

            virtual void                SetSelectedEntry(USHORT nPos);
            virtual USHORT              GetSelectedEntry();
};



/*
class ScPropertyDlg : public ModalDialog
{
private:
            SvBasicPropertyDataControl  aBaProDatCtr;
            OKButton                    anOk;
            CancelButton                aCancel;
            USHORT                      nCount;
            USHORT                      nClickCount;

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


