/*************************************************************************
 *
 *  $RCSfile: standardcontrol.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: fs $ $Date: 2001-02-19 16:32:29 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _EXTENSIONS_PROPCTRLR_STANDARDCONTROL_HXX_
#include "standardcontrol.hxx"
#endif
#ifndef _EXTENSIONS_PROPCTRLR_BRWCONTROLLISTENER_HXX_
#include "brwcontrollistener.hxx"
#endif

#ifndef _TOOLS_SOLMATH_HXX
#include <tools/solmath.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif

//==================================================================
// ugly dependencies for the OColorControl
#ifndef _SVX_SVXIDS_HRC
#include <svx/svxids.hrc>
#endif
#define ITEMID_COLOR_TABLE      SID_COLOR_TABLE
#ifndef _SVX_DRAWITEM_HXX
#include <svx/drawitem.hxx>
#endif
#ifndef _XTABLE_HXX
#include <xtable.hxx>
#endif
//==================================================================
// ugly dependencies for the OMultilineEditControl
#ifndef _SV_FLOATWIN_HXX
#include <vcl/floatwin.hxx>
#endif
#ifndef _SVEDIT_HXX
#include <svtools/svmedit.hxx>
#endif
#ifndef _SV_BUTTON_HXX
#include <vcl/button.hxx>
#endif
#ifndef _SV_SVAPP_HXX
#include <vcl/svapp.hxx>
#endif
//==================================================================

#ifndef _OFF_APP_HXX
#include <offmgr/app.hxx>
#endif

//............................................................................
namespace pcr
{
//............................................................................

    //==================================================================
    //= OTimeControl
    //==================================================================
    //------------------------------------------------------------------
    OTimeControl::OTimeControl(Window* pParent, WinBits nWinStyle)
            :OCommonBehaviourControl(this)
            ,TimeField(pParent, nWinStyle)
    {
        SetStrictFormat(sal_True);
        SetFormat(TIMEF_SEC);

        SetModifyHdl(LINK(this, OCommonBehaviourControl, ModifiedHdl ));
        SetGetFocusHdl(LINK(this, OCommonBehaviourControl, GetFocusHdl));
        SetLoseFocusHdl(LINK(this, OCommonBehaviourControl, LoseFocusHdl));

        autoSizeWindow();

        EnableEmptyFieldValue(sal_True);
    }

    //------------------------------------------------------------------
    void OTimeControl::SetProperty(const ::rtl::OUString &rString,sal_Bool bIsUnknown)
    {
        if (bIsUnknown || (rString == m_sStandardString))
        {
            SetText(String());
            SetEmptyTime();
        }
        else
        {
            if (rString.getLength())
            {
                sal_uInt32 nTime = rString.toInt32();
                SetTime(Time(nTime));
            }
            else
            {
                SetText(String());
                SetEmptyTime();
            }
        }
    }

    //------------------------------------------------------------------
    ::rtl::OUString OTimeControl::GetProperty() const
    {
        if (GetText().Len()>0)
        {
            sal_Int32 nTime = GetTime().GetTime();
            return ::rtl::OUString::valueOf(nTime);
        }
        else
            return ::rtl::OUString();
    }

    //------------------------------------------------------------------
    long OTimeControl::PreNotify( NotifyEvent& rNEvt )
    {
        long nResult = OCommonBehaviourControl::handlePreNotify(rNEvt);
        if (nResult)    // handled
            return nResult;

        return TimeField::PreNotify(rNEvt);
    }

    //==================================================================
    //= ODateControl
    //==================================================================
    //------------------------------------------------------------------
    ODateControl::ODateControl(Window* pParent, WinBits nWinStyle)
            :OCommonBehaviourControl(this)
            ,DateField(pParent, nWinStyle)
    {
        SetStrictFormat(sal_True);

        SetModifyHdl(LINK(this, OCommonBehaviourControl, ModifiedHdl ));
        SetGetFocusHdl(LINK(this, OCommonBehaviourControl, GetFocusHdl));
        SetLoseFocusHdl(LINK(this, OCommonBehaviourControl, LoseFocusHdl));

        SetMin(Date(1,1,1600));
        SetFirst(Date(1,1,1600));
        SetLast(Date(1,1,9999));
        SetMax(Date(1,1,9999));

        autoSizeWindow();

        SetExtFormat(XTDATEF_SYSTEM_SHORT_YYYY);
        EnableEmptyFieldValue(sal_True);
    }

    //------------------------------------------------------------------
    void ODateControl::SetProperty(const ::rtl::OUString& rString, sal_Bool bIsUnknown)
    {
        if (bIsUnknown || (rString==m_sStandardString))
        {
            SetText(String());
            SetEmptyDate();
        }
        else
        {
            if (rString.getLength())
            {
                sal_uInt32 nDate = rString.toInt32();
                SetDate(Date(nDate));
            }
            else
            {
                SetText(String());
                SetEmptyDate();
            }
        }
    }

    //------------------------------------------------------------------
    ::rtl::OUString ODateControl::GetProperty() const
    {
        if (GetText().Len()>0)
        {
            sal_Int32 nDate = GetDate().GetDate();
            return ::rtl::OUString::valueOf(nDate);
        }
        else return ::rtl::OUString();
    }

    //------------------------------------------------------------------
    long ODateControl::PreNotify( NotifyEvent& rNEvt )
    {
        long nResult = OCommonBehaviourControl::handlePreNotify(rNEvt);
        if (nResult)    // handled
            return nResult;

        return DateField::PreNotify(rNEvt);
    }

    //==================================================================
    //= OEditControl
    //==================================================================
    //------------------------------------------------------------------
    OEditControl::OEditControl(Window* _pParent, sal_Bool _bPW, WinBits _nWinStyle)
            :OCommonBehaviourControl(this)
            ,Edit(_pParent, _nWinStyle)
    {
        m_bIsPassword = _bPW;

        SetModifyHdl(LINK(this, OCommonBehaviourControl, ModifiedHdl ));
        SetGetFocusHdl(LINK(this, OCommonBehaviourControl, GetFocusHdl));
        SetLoseFocusHdl(LINK(this, OCommonBehaviourControl, LoseFocusHdl));

        autoSizeWindow();

        if (m_bIsPassword)
           SetMaxTextLen(1);
    }

    //------------------------------------------------------------------
    void OEditControl::SetProperty(const ::rtl::OUString &rString,sal_Bool bIsUnknown)
    {
        String aStr;

        if (!bIsUnknown)
            aStr = rString.getStr();

        if (m_bIsPassword)
            if (aStr.Len())
            {
                sal_Unicode nCharacter = (sal_Unicode)aStr.ToInt32();
                if (nCharacter)
                    aStr = String(&nCharacter, 1);
                else
                    aStr = String();
            }

        SetText(aStr);
    }

    //------------------------------------------------------------------
    ::rtl::OUString OEditControl::GetProperty() const
    {
        String aStr = GetText();

        if (m_bIsPassword)
        {
            if (aStr.Len())
                aStr = String::CreateFromInt32((sal_Int32)aStr.GetBuffer()[0]);
            else
                aStr = String('0');
        }

        return aStr;
    }

    //------------------------------------------------------------------
    long OEditControl::PreNotify( NotifyEvent& rNEvt )
    {
        long nResult = OCommonBehaviourControl::handlePreNotify(rNEvt);
        if (nResult)    // handled
            return nResult;

        return Edit::PreNotify(rNEvt);
    }

    //------------------------------------------------------------------
    void OEditControl::modified(Window* _pSource)
    {
        OCommonBehaviourControl::modified(_pSource);

        // for pasword controls, we fire a commit for every single change
        if (m_bIsPassword)
            commitModified(_pSource);
    }

    //==================================================================
    // class OCurrencyControl
    //==================================================================
    //------------------------------------------------------------------
    OCurrencyControl::OCurrencyControl( Window* _pParent, sal_uInt16 _nDigits, WinBits _nWinStyle)
            :OCommonBehaviourControl(this)
            ,LongCurrencyField(_pParent, _nWinStyle)
    {
        SetModifyHdl(LINK( this, OCommonBehaviourControl, ModifiedHdl ));
        SetGetFocusHdl(LINK( this, OCommonBehaviourControl, GetFocusHdl));
        SetLoseFocusHdl(LINK( this, OCommonBehaviourControl, LoseFocusHdl));

        International aInter=GetInternational();
        aInter.SetCurrSymbol(String());
        SetInternational(aInter);

        double nMax = GetMax();
        nMax = -nMax;
        BigInt aBigInt(nMax);
        SetMin(aBigInt);

        SetStrictFormat(sal_True);
        SetDecimalDigits(_nDigits);
        EnableEmptyFieldValue(sal_True);

        autoSizeWindow();
    }

    //------------------------------------------------------------------
    static double ImplCalcLongValue( double nValue, sal_uInt16 nDigits )
    {
        double n = nValue;
        for ( sal_uInt16 d = 0; d < nDigits; d++ )
            n *= 10;
        return n;
    }

    //------------------------------------------------------------------
    static double ImplCalcDoubleValue( double nValue, sal_uInt16 nDigits )
    {
        double n = nValue;
        for ( sal_uInt16 d = 0; d < nDigits; d++ )
            n /= 10;
        return n;
    }

    //------------------------------------------------------------------
    void OCurrencyControl::SetProperty(const ::rtl::OUString &rString,sal_Bool bIsUnknown)
    {
        ::rtl::OUString aStr;

        if (bIsUnknown || (rString == m_sStandardString))
        {
            SetText(String());
            SetEmptyFieldValue();
        }
        else
        {
            if (rString.getLength())
            {
                int nErr = 0;
                double nValue = SolarMath::StringToDouble(rString, GetInternational(), nErr);
                BigInt aBInt = ImplCalcLongValue(nValue, GetDecimalDigits());
                SetValue(aBInt);
            }
            else
                SetEmptyFieldValue();
        }
    }


    //------------------------------------------------------------------
    ::rtl::OUString OCurrencyControl::GetProperty() const
    {
        if (GetText().Len()>0)
        {
            String aBuffer;
            BigInt aBInt=GetValue();
            double nValue = ImplCalcDoubleValue(aBInt,GetDecimalDigits());
            SolarMath::DoubleToString(aBuffer,nValue,'F',GetDecimalDigits());

            return aBuffer;
        }
        else
            return ::rtl::OUString();

    }

    //------------------------------------------------------------------
    long OCurrencyControl::PreNotify( NotifyEvent& rNEvt )
    {
        long nResult = OCommonBehaviourControl::handlePreNotify(rNEvt);
        if (nResult)    // handled
            return nResult;

        return LongCurrencyField::PreNotify(rNEvt);
    }

    //==================================================================
    //= ONumericControl
    //==================================================================
    //------------------------------------------------------------------
    ONumericControl::ONumericControl( Window* _pParent, sal_uInt16 _nDigits, WinBits _nWinStyle)
            :OCommonBehaviourControl(this)
            ,NumericField(_pParent, _nWinStyle)
    {
        SetModifyHdl(LINK( this, OCommonBehaviourControl, ModifiedHdl ));
        SetGetFocusHdl(LINK( this, OCommonBehaviourControl, GetFocusHdl));
        SetLoseFocusHdl(LINK( this, OCommonBehaviourControl, LoseFocusHdl));

        SetDecimalDigits(_nDigits);
        EnableEmptyFieldValue(sal_True);
        SetMin(-GetMax());
        SetStrictFormat(sal_True);

        autoSizeWindow();
    }

    //------------------------------------------------------------------
    void ONumericControl::SetProperty(const ::rtl::OUString &rString,sal_Bool bIsUnknown)
    {
        if (bIsUnknown || (rString == m_sStandardString))
        {
            SetText(String());
            SetEmptyFieldValue();
        }
        else
        {
            if (rString.getLength()>0)
                SetValue(rString.toInt32());
            else
                SetEmptyFieldValue();
        }
    }

    //------------------------------------------------------------------
    ::rtl::OUString ONumericControl::GetProperty() const
    {
        if (GetText().Len() != 0)
            return ::rtl::OUString::valueOf((sal_Int32)GetValue());
        else
            return ::rtl::OUString();

    }

    //------------------------------------------------------------------
    long ONumericControl::PreNotify( NotifyEvent& rNEvt )
    {
        long nResult = OCommonBehaviourControl::handlePreNotify(rNEvt);
        if (nResult)    // handled
            return nResult;

        return NumericField::PreNotify(rNEvt);
    }

    //==================================================================
    //= OColorControl
    //==================================================================
    #define LB_DEFAULT_COUNT 9
    //------------------------------------------------------------------
    String MakeHexStr(sal_uInt32 nVal, sal_uInt32 nLength)
    {
        String aStr;
        while (nVal>0)
        {
            sal_uInt16 nDigit=sal_uInt16(nVal & 0x000F);
            char c=char(nVal & 0x000F);
            nVal>>=4;
            if (c<=9) c+='0';
            else c+='A'-10;
            aStr.Insert(c,0);
        }
        while (aStr.Len() < nLength) aStr.Insert('0',0);
        return aStr;
    }

    //------------------------------------------------------------------
    OColorControl::OColorControl(Window* pParent, WinBits nWinStyle)
            :OCommonBehaviourControl(this)
            ,ColorListBox(pParent, nWinStyle)
    {
        SetSelectHdl(LINK( this, OCommonBehaviourControl, ModifiedHdl ));
        SetGetFocusHdl(LINK( this, OCommonBehaviourControl, GetFocusHdl));
        SetLoseFocusHdl(LINK( this, OCommonBehaviourControl, LoseFocusHdl));


        // initialize the color listbox
        SfxObjectShell* pDocSh = SfxObjectShell::Current();
        DBG_ASSERT( pDocSh, "OColorControl::OColorControl: no doc shell!");

        if (pDocSh)
        {
            XColorTable* pColorTbl;
            const SfxPoolItem* pColorItem = pDocSh->GetItem( SID_COLOR_TABLE );
            if (pColorItem)
            {
                DBG_ASSERT(pColorItem->ISA(SvxColorTableItem), "OColorControl::OColorControl: invalid color item!");
                SvxColorTableItem aItem( *static_cast< const SvxColorTableItem*>( pColorItem ) );
                pColorTbl = aItem.GetColorTable();
            }
            else    // BasicDocShell has no color item
            {
                pColorTbl = OFF_APP()->GetStdColorTable();
            }

            DBG_ASSERT(pColorTbl, "OColorControl::OColorControl: no color table!");

            if (pColorTbl)
            {
                for (sal_uInt16 i = 0; i < pColorTbl->Count(); ++i)
                {
                    XColorEntry* pEntry = pColorTbl->Get( i );
                    InsertEntry( pEntry->GetColor(), pEntry->GetName() );
                }
            }
        }

        SetUpdateMode( sal_False);

        if (m_sStandardString.getLength()>0)
            InsertEntry(m_sStandardString, 0);

        SetUpdateMode( sal_True );
        SelectEntryPos(0);
    }

    //------------------------------------------------------------------
    void OColorControl::SetCtrSize(const Size& _rSize)
    {
        OCommonBehaviourControl::SetCtrSize(_rSize);

        sal_uInt16 nCount = GetEntryCount();
        if (nCount>LB_DEFAULT_COUNT)
            nCount=LB_DEFAULT_COUNT;
        SetDropDownLineCount(nCount);
    }

    //------------------------------------------------------------------
    void OColorControl::SetProperty(const ::rtl::OUString &rString,sal_Bool bIsUnknown)
    {
        if (!bIsUnknown)
        {
            if ((rString.getLength() != 0) && (rString != m_sStandardString))
            {
                sal_uInt32 nColor = rString.toInt32();
                Color aRgbCol((ColorData)nColor);

                if (!IsEntrySelected(aRgbCol))
                {
                    SelectEntry(aRgbCol);
                    if (!IsEntrySelected(aRgbCol))
                    {   // the given color is not part of the list -> insert a new entry with the hex code of the color
                        String aStr = String::CreateFromAscii("0x");
                        aStr += MakeHexStr(nColor,8);
                        InsertEntry(aRgbCol, aStr);
                        SelectEntry(aRgbCol);
                    }
                }
            }
            else
                SelectEntryPos(0);
        }
        else
            SetNoSelection();
    }

    //------------------------------------------------------------------
    ::rtl::OUString OColorControl::GetProperty()const
    {
        ::rtl::OUString aResult;
        if (GetSelectEntryCount()>0)
        {
            aResult=GetSelectEntry();
            if (aResult != m_sStandardString)
            {
                Color aRgbCol = GetSelectEntryColor();
                sal_Int32 nColor = aRgbCol.GetColor();
                aResult = ::rtl::OUString::valueOf(nColor);
            }
        }
        return aResult;
    }

    //------------------------------------------------------------------
    sal_Bool OColorControl::HasList()
    {
        return sal_True;
    }

    //------------------------------------------------------------------
    void OColorControl::ClearList()
    {
        Clear();
    }

    //------------------------------------------------------------------
    void OColorControl::InsertCtrEntry( const ::rtl::OUString& _rString, sal_uInt16 _nPos)
    {
        InsertEntry(_rString, _nPos);
    }

    //------------------------------------------------------------------
    long OColorControl::PreNotify( NotifyEvent& rNEvt )
    {
        long nResult = OCommonBehaviourControl::handlePreNotify(rNEvt);
        if (nResult)    // handled
            return nResult;

        return ColorListBox::PreNotify(rNEvt);
    }

    //------------------------------------------------------------------
    void OColorControl::modified(Window* _pSource)
    {
        OCommonBehaviourControl::modified(_pSource);

        if (!IsTravelSelect())
            // fire a commit
            commitModified(_pSource);
    }

    //==================================================================
    //= OListboxControl
    //==================================================================
    //------------------------------------------------------------------
    OListboxControl::OListboxControl( Window* pParent, WinBits nWinStyle)
            :OCommonBehaviourControl(this)
            ,ListBox(pParent, nWinStyle)
    {
        SetDropDownLineCount(9);

        SetSelectHdl(LINK( this, OCommonBehaviourControl, ModifiedHdl ));
        SetGetFocusHdl(LINK( this, OCommonBehaviourControl, GetFocusHdl));
        SetLoseFocusHdl(LINK( this, OCommonBehaviourControl, LoseFocusHdl));
    }

    //------------------------------------------------------------------
    void OListboxControl::SetCtrSize(const Size& rSize)
    {
        OCommonBehaviourControl::SetCtrSize(rSize);

        sal_uInt16 nCount = GetEntryCount();
        if (nCount>LB_DEFAULT_COUNT)
            nCount=LB_DEFAULT_COUNT;
        SetDropDownLineCount(nCount);
    }

    //------------------------------------------------------------------
    ::rtl::OUString OListboxControl::GetProperty()const
    {
        if (GetSelectEntryCount()>0)
            return GetSelectEntry();
        return String();
    }

    //------------------------------------------------------------------
    void OListboxControl::SetProperty(const ::rtl::OUString &rString,sal_Bool bIsUnknown)
    {
        if (!bIsUnknown)
        {
            String aTmp = rString;
            if (aTmp != GetSelectEntry())
                SelectEntry(aTmp);

            if (!IsEntrySelected(aTmp))
            {
                if (!rString.getLength())
                {
                    SelectEntry(m_sStandardString);
                }
                else
                {
                    InsertEntry(aTmp, 0);
                    SelectEntry(aTmp);
                }
            }
        }
        else
        {
            SetNoSelection();
        }
    }

    //------------------------------------------------------------------
    sal_Bool OListboxControl::HasList()
    {
        return sal_True;
    }

    //------------------------------------------------------------------
    void OListboxControl::ClearList()
    {
        Clear();
    }

    //------------------------------------------------------------------
    void OListboxControl::InsertCtrEntry( const ::rtl::OUString& rString,sal_uInt16 nPos)
    {
        InsertEntry(rString,nPos);
    }

    //------------------------------------------------------------------
    long OListboxControl::PreNotify( NotifyEvent& rNEvt )
    {
        long nResult = OCommonBehaviourControl::handlePreNotify(rNEvt);
        if (nResult)    // handled
            return nResult;

        return ListBox::PreNotify(rNEvt);
    }

    //------------------------------------------------------------------
    void OListboxControl::modified(Window* _pSource)
    {
        OCommonBehaviourControl::modified(_pSource);

        if (!IsTravelSelect())
            // fire a commit
            commitModified(_pSource);
    }

    //==================================================================
    //= OComboboxControl
    //==================================================================
    //------------------------------------------------------------------
    OComboboxControl::OComboboxControl( Window* pParent, WinBits nWinStyle)
            :OCommonBehaviourControl(this)
            ,ComboBox(pParent,nWinStyle)
    {
        SetModifyHdl(LINK( this, OCommonBehaviourControl, ModifiedHdl ));
        SetGetFocusHdl(LINK( this, OCommonBehaviourControl, GetFocusHdl));
        SetLoseFocusHdl(LINK( this, OCommonBehaviourControl, LoseFocusHdl));

        SetDropDownLineCount(9);
    }

    //------------------------------------------------------------------
    void OComboboxControl::SetCtrSize(const Size& rSize)
    {
        OCommonBehaviourControl::SetCtrSize(rSize);

        sal_uInt16 nCount = GetEntryCount();
        if (nCount > LB_DEFAULT_COUNT)
            nCount = LB_DEFAULT_COUNT;
        SetDropDownLineCount(nCount);
    }

    //------------------------------------------------------------------
    void OComboboxControl::SetProperty(const ::rtl::OUString &rString,sal_Bool bIsUnknown)
    {
        ::rtl::OUString aStr;
        if (!bIsUnknown)
            aStr = rString;
        SetText(aStr);
    }

    //------------------------------------------------------------------
    ::rtl::OUString OComboboxControl::GetProperty() const
    {
        return GetText();
    }

    //------------------------------------------------------------------
    sal_Bool OComboboxControl::HasList()
    {
        return sal_True;
    }

    //------------------------------------------------------------------
    void OComboboxControl::ClearList()
    {
        Clear();
    }

    //------------------------------------------------------------------
    void OComboboxControl::InsertCtrEntry( const ::rtl::OUString& rString,sal_uInt16 nPos)
    {
        InsertEntry(rString,nPos);
    }

    //------------------------------------------------------------------
    long OComboboxControl::PreNotify( NotifyEvent& rNEvt )
    {
        long nResult = OCommonBehaviourControl::handlePreNotify(rNEvt);
        if (nResult)    // handled
            return nResult;

        return ComboBox::PreNotify(rNEvt);
    }

    //==================================================================
    //= OMultilineEditControl
    //==================================================================
    class OMultilineFloatingEdit : public FloatingWindow
    {
    private:
        MultiLineEdit   m_aImplEdit;

    protected:
        virtual void    Resize();

    public:
                        OMultilineFloatingEdit(Window* _pParen);
        MultiLineEdit*  getEdit() { return &m_aImplEdit; }

    protected:
        virtual long    PreNotify(NotifyEvent& _rNEvt);
    };

    //------------------------------------------------------------------
    OMultilineFloatingEdit::OMultilineFloatingEdit(Window* _pParent)
        :FloatingWindow(_pParent, WB_BORDER)
        ,m_aImplEdit(this, WB_VSCROLL|WB_IGNORETAB|WB_NOBORDER)
    {
        m_aImplEdit.Show();
    }

    //------------------------------------------------------------------
    void OMultilineFloatingEdit::Resize()
    {
        m_aImplEdit.SetSizePixel(GetOutputSizePixel());
    }

    //------------------------------------------------------------------
    long OMultilineFloatingEdit::PreNotify(NotifyEvent& _rNEvt)
    {
        long nResult = sal_True;

        sal_uInt16 nSwitch = _rNEvt.GetType();
        if (EVENT_KEYINPUT == nSwitch)
        {
            const KeyCode& aKeyCode = _rNEvt.GetKeyEvent()->GetKeyCode();
            sal_uInt16 nKey = aKeyCode.GetCode();

            if  (   (   (KEY_RETURN == nKey)
                    && !aKeyCode.IsShift()
                    )
                ||  (   (KEY_UP == nKey)
                    &&  aKeyCode.IsMod2()
                    )
                )
            {
                EndPopupMode();
            }
            else
                nResult=FloatingWindow::PreNotify(_rNEvt);
        }
        else
            nResult=FloatingWindow::PreNotify(_rNEvt);

        return nResult;
    }

    //==================================================================
    //= OMultilineEditControl
    //==================================================================
    #define STD_HEIGHT  100
    //------------------------------------------------------------------
    OMultilineEditControl::OMultilineEditControl( Window* pParent,sal_Bool bEd, WinBits nWinStyle)
            :OCommonBehaviourControl(this)
            ,Control(pParent,((nWinStyle | WB_DIALOGCONTROL)& (~WB_READONLY| ~WB_DROPDOWN)))
            ,m_pFloatingEdit(NULL)
            ,m_pDropdownButton(NULL)
            ,m_pImplEdit(NULL)
            ,m_bDropdown(sal_False)
            ,m_bEdit(bEd)
    {
        m_pImplEdit = new MultiLineEdit(this,WB_TABSTOP|WB_IGNORETAB| WB_NOBORDER| nWinStyle& WB_READONLY);
        m_pImplEdit->Show();

        if (nWinStyle & WB_DROPDOWN)
        {
            m_pDropdownButton = new PushButton( this, WB_NOLIGHTBORDER | WB_RECTSTYLE | WB_NOTABSTOP);
            m_pDropdownButton->SetSymbol(SYMBOL_SPIN_DOWN);
            m_pDropdownButton->SetClickHdl( LINK( this, OMultilineEditControl, DropDownHdl ) );
            m_pDropdownButton->Show();
        }
        SetBackground(m_pImplEdit->GetBackground());

        m_pFloatingEdit = new OMultilineFloatingEdit(this); //FloatingWindow

        m_pFloatingEdit->SetPopupModeEndHdl(LINK( this, OMultilineEditControl, ReturnHdl ));

        m_pFloatingEdit->getEdit()->SetModifyHdl(LINK( this, OCommonBehaviourControl, ModifiedHdl));
        m_pImplEdit->SetGetFocusHdl(LINK( this, OCommonBehaviourControl, GetFocusHdl));
        m_pImplEdit->SetModifyHdl(LINK( this, OCommonBehaviourControl, ModifiedHdl));
        m_pImplEdit->SetLoseFocusHdl(LINK( this, OCommonBehaviourControl, LoseFocusHdl));

        autoSizeWindow();
    }

    //------------------------------------------------------------------
    OMultilineEditControl::~OMultilineEditControl()
    {
        delete m_pFloatingEdit;
        delete m_pImplEdit;
        delete m_pDropdownButton;
    }

    //------------------------------------------------------------------
    sal_Bool OMultilineEditControl::ShowDropDown( sal_Bool bShow )
    {
        if (bShow)
        {
            Point aMePos= GetPosPixel();
            aMePos = GetParent()->OutputToScreenPixel( aMePos );
            Size aSize=GetSizePixel();
            Rectangle aRect(aMePos,aSize);
            aSize.Height() = STD_HEIGHT;
            m_pFloatingEdit->SetOutputSizePixel(aSize);
            m_pFloatingEdit->StartPopupMode( aRect, FLOATWIN_POPUPMODE_DOWN );

            m_pFloatingEdit->Show();
            m_pFloatingEdit->getEdit()->GrabFocus();
            m_pFloatingEdit->getEdit()->SetSelection(Selection(m_pFloatingEdit->getEdit()->GetText().Len()));
            m_bDropdown=sal_True;
            if (m_bEdit)
                m_pFloatingEdit->getEdit()->SetText(m_pImplEdit->GetText());
            m_pImplEdit->SetText(String());
        }
        else
        {
            m_pFloatingEdit->Hide();
            m_pFloatingEdit->Invalidate();
            m_pFloatingEdit->Update();

            String aOutput;
            String aStr=m_pFloatingEdit->getEdit()->GetText();

            if (aStr.Len()>0)
            {
                sal_Int32 nCount = aStr.GetTokenCount('\n');

                String aInput=aStr.GetToken(0,'\n' );

                if (m_bEdit)
                {
                    aOutput=aStr;
                }
                else
                {
                    if (aInput.Len()>0)
                    {
                        aOutput+='\"';
                        aOutput+=aInput;
                        aOutput+='\"';
                    }

                    for (sal_Int32 i=1; i<nCount; ++i)
                    {
                        aInput = aStr.GetToken((sal_uInt16)i, '\n');
                        if (aInput.Len()>0)
                        {
                            aOutput += ';';
                            aOutput += '\"';
                            aOutput += aInput;
                            aOutput += '\"';
                        }
                    }
                }
            }

            m_pImplEdit->SetText(aOutput);
            GetParent()->Invalidate(INVALIDATE_CHILDREN);
            m_bDropdown=sal_False;
            m_pImplEdit->GrabFocus();
        }
        return m_bDropdown;

    }


    //------------------------------------------------------------------
    void OMultilineEditControl::Resize()
    {
        Size aOutSz = GetOutputSizePixel();

        if (m_pDropdownButton!=NULL)
        {
            long nSBWidth = GetSettings().GetStyleSettings().GetScrollBarSize();
            nSBWidth = CalcZoom( nSBWidth );
            m_pImplEdit->SetPosSizePixel( 0, 1, aOutSz.Width() - nSBWidth, aOutSz.Height()-2 );
            m_pDropdownButton->SetPosSizePixel( aOutSz.Width() - nSBWidth, 0, nSBWidth, aOutSz.Height() );
        }
        else
            m_pImplEdit->SetPosSizePixel( 0, 1, aOutSz.Width(), aOutSz.Height()-2 );
    }

    //------------------------------------------------------------------
    void OMultilineEditControl::SetLocked(sal_Bool bFlag)
    {
        m_bLocked = bFlag;
        Font aFont = GetFont();

        m_pFloatingEdit->getEdit()->SetReadOnly(bFlag);
        if (m_bLocked)
        {
            aFont.SetColor(Color(COL_GRAY));
        }
        else
        {
            aFont = GetParent()->GetFont();
        }
        SetFont(aFont);
    }

    //------------------------------------------------------------------
    void OMultilineEditControl::SetProperty(const ::rtl::OUString &rString,sal_Bool bIsUnknown)
    {
        String aStr;

        if (!bIsUnknown)
            aStr=rString.getStr();

        m_pFloatingEdit->getEdit()->SetText(aStr);

        if (m_bEdit)
        {
            m_pImplEdit->SetText(aStr);
        }
        else
        {
            String aOutput;

            if (aStr.Len()>0)
            {
                if (aStr.Len()>0)
                {
                    sal_Int32 nCount = aStr.GetTokenCount('\n');

                    String aInput=aStr.GetToken(0,'\n' );
                    if (aInput.Len()>0)
                    {
                        aOutput+='\"';
                        aOutput+=aInput;
                        aOutput+='\"';
                    }

                    for(sal_Int32 i=1;i<nCount;i++)
                    {
                        aInput=aStr.GetToken((sal_uInt16)i,'\n' );
                        if (aInput.Len()>0)
                        {
                            aOutput += ';';
                            aOutput += '\"';
                            aOutput += aInput;
                            aOutput += '\"';
                        }
                    }
                }
            }
            m_pImplEdit->SetText(aOutput);
        }
    }

    //------------------------------------------------------------------
    ::rtl::OUString OMultilineEditControl::GetProperty() const
    {
        if (m_bEdit)
            return m_pImplEdit->GetText();
        else
            return m_pFloatingEdit->getEdit()->GetText();
    }

    //------------------------------------------------------------------
    void OMultilineEditControl::GetFocus()
    {
        m_pImplEdit->GetFocus();
    }

    //------------------------------------------------------------------
    long OMultilineEditControl::PreNotify( NotifyEvent& rNEvt )
    {
        long nResult=sal_True;

        sal_uInt16 nSwitch=rNEvt.GetType();
        if (nSwitch==EVENT_KEYINPUT)
        {
            const KeyCode& aKeyCode=rNEvt.GetKeyEvent()->GetKeyCode();
            sal_uInt16 nKey=aKeyCode.GetCode();

            if (nKey==KEY_RETURN &&!aKeyCode.IsShift())
            {
                LoseFocusHdl(m_pImplEdit);
                m_bDir = sal_True;
                if (getListener())
                    getListener()->TravelLine(this);
            }
            else if (nKey==KEY_DOWN && aKeyCode.IsMod2())
            {
                Invalidate();
                ShowDropDown(sal_True);
            }
            else if (KEYGROUP_CURSOR==aKeyCode.GetGroup()|| nKey==KEY_HELP
                    || aKeyCode.GetGroup()==KEYGROUP_FKEYS  || m_bEdit)
            {
                nResult=Control::PreNotify(rNEvt);
            }
            else if (!m_bEdit)
            {
                Selection aSel=m_pImplEdit->GetSelection();
                if (aSel.Min()!=aSel.Max())
                {
                    aSel.Min()=FindPos(aSel.Min());
                    aSel.Max()=FindPos(aSel.Max());
                }
                else
                {
                    aSel.Min()=FindPos(aSel.Min());
                    aSel.Max()=aSel.Min();
                }
                Invalidate();
                ShowDropDown(sal_True);
                m_pFloatingEdit->getEdit()->GrabFocus();
                m_pFloatingEdit->getEdit()->SetSelection(aSel);
                Window* pFocusWin = Application::GetFocusWindow();
                pFocusWin->KeyInput(*rNEvt.GetKeyEvent());
            }
        }
        else
            nResult=Control::PreNotify(rNEvt);

        return nResult;
    }

    //------------------------------------------------------------------
    long OMultilineEditControl::FindPos(long nSinglePos)
    {
        sal_uInt16 nPos=0;
        sal_uInt16 nDiff=0;
        String aOutput;
        String aStr=m_pFloatingEdit->getEdit()->GetText();
        String aStr1=m_pImplEdit->GetText();

        if ((nSinglePos == 0) || (nSinglePos == aStr1.Len()))
        {
            return nSinglePos;
        }

        if (aStr.Len()>0)
        {
            sal_Int32 nCount = aStr.GetTokenCount('\n');

            String aInput = aStr.GetToken(0,'\n' );

            if (aInput.Len()>0)
            {
                aOutput+='\"';
                nDiff++;
                aOutput+=aInput;
                aOutput+='\"';
            }

            if (nSinglePos <= aOutput.Len())
            {
                nPos=nSinglePos-nDiff;
            }
            else
            {
                for (sal_Int32 i=1; i<nCount; ++i)
                {
                    aInput=aStr.GetToken((sal_uInt16)i, '\n');
                    if (aInput.Len()>0)
                    {
                        aOutput += ';';
                        aOutput += '\"';
                        nDiff += 2;
                        aOutput += aInput;
                        aOutput += '\"';

                        if (nSinglePos <= aOutput.Len())
                        {
                            nPos=nSinglePos-nDiff;
                            break;
                        }
                    }
                }
            }
        }
        return nPos;
    }

    //------------------------------------------------------------------
    IMPL_LINK( OMultilineEditControl, ReturnHdl, OMultilineFloatingEdit*, pMEd)
    {

        String aStr = m_pFloatingEdit->getEdit()->GetText();
        String aStr2 = m_pImplEdit->GetText();
        ShowDropDown(sal_False);

        if (aStr!=aStr2 || !m_bEdit)
        {
            if (m_bModified)
                modified(m_pImplEdit);
            commitModified(m_pImplEdit);
        }

        return 0;
    }

    //------------------------------------------------------------------
    IMPL_LINK( OMultilineEditControl, DropDownHdl, PushButton*, pPb )
    {
        ShowDropDown(!m_bDropdown);
        return 0;
    }

    //------------------------------------------------------------------
    void OMultilineEditControl::modified(Window* _pSource)
    {
        if (_pSource == m_pFloatingEdit->getEdit())
        {   // no listener notification in this case, just the set flag
            m_bModified = sal_True;
        }
        else
            OCommonBehaviourControl::modified(_pSource);
    }

//............................................................................
} // namespace pcr
//............................................................................

/*************************************************************************
 * history:
 *  $Log: not supported by cvs2svn $
 *  Revision 1.3  2001/02/05 12:24:09  tbe
 *  get standard color table if SID_COLOR_TABLE item not available
 *
 *  Revision 1.2  2001/01/24 14:12:53  fs
 *  recognize and tolerate a missing SID_COLOR_TABLE item
 *
 *  Revision 1.1  2001/01/12 11:33:20  fs
 *  initial checkin - outsourced the form property browser
 *
 *
 *  Revision 1.0 09.01.01 10:28:46  fs
 ************************************************************************/

