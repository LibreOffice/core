/*************************************************************************
 *
 *  $RCSfile: standardcontrol.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: hr $ $Date: 2003-03-25 16:03:59 $
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
#define _EXTENSIONS_PROPCTRLR_STANDARDCONTROL_HXX_

#ifndef _EXTENSIONS_PROPCTRLR_COMMONCONTROL_HXX_
#include "commoncontrol.hxx"
#endif
#ifndef _SV_FIELD_HXX
#include <vcl/field.hxx>
#endif
#ifndef _LONGCURR_HXX
#include <vcl/longcurr.hxx>
#endif
#ifndef _CTRLBOX_HXX
#include <svtools/ctrlbox.hxx>
#endif
#ifndef _SV_LSTBOX_HXX
#include <vcl/lstbox.hxx>
#endif
#ifndef _SV_COMBOBOX_HXX
#include <vcl/combobox.hxx>
#endif

class PushButton;
class MultiLineEdit;
//............................................................................
namespace pcr
{
//............................................................................

    //========================================================================
    //= OTimeControl
    //========================================================================
    class OTimeControl : public OCommonBehaviourControl, TimeField
    {
    protected:
            virtual long                    PreNotify( NotifyEvent& rNEvt );

    public:
                                            OTimeControl( Window* pParent, WinBits nWinStyle = WB_TABSTOP);

            virtual void                    SetProperty(const ::rtl::OUString &rString,sal_Bool bIsUnknown=sal_False);
            virtual ::rtl::OUString         GetProperty()const;
    };

    //========================================================================
    //= ODateControl
    //========================================================================
    class ODateControl : public OCommonBehaviourControl, DateField
    {
    protected:
            virtual long                    PreNotify( NotifyEvent& rNEvt );

    public:
                                            ODateControl( Window* pParent, WinBits nWinStyle = WB_TABSTOP);

            virtual void                    SetProperty(const ::rtl::OUString &rString,sal_Bool bIsUnknown=sal_False);
            virtual ::rtl::OUString         GetProperty()const;
    };

    //========================================================================
    //= OEditControl
    //========================================================================
    class OEditControl : public OCommonBehaviourControl, Edit
    {
    protected:
        sal_Bool                            m_bIsPassword : 1;

    protected:
            virtual long                    PreNotify( NotifyEvent& rNEvt );

    public:
                                            OEditControl(Window* _pParent, sal_Bool _bPassWord, WinBits nWinStyle = WB_TABSTOP);

            virtual void                    SetProperty(const ::rtl::OUString &rString,sal_Bool bIsUnknown=sal_False);
            virtual ::rtl::OUString         GetProperty()const;

    protected:
            virtual void modified(Window* _pSource);
    };

    //========================================================================
    //= OCurrencyControl
    //========================================================================
    class OCurrencyControl : public OCommonBehaviourControl, LongCurrencyField
    {
    protected:
            virtual long                    PreNotify( NotifyEvent& rNEvt );

    public:
                                            OCurrencyControl( Window* pParent,sal_uInt16 nDigits,
                                                        WinBits nWinStyle = WB_TABSTOP);

            virtual void                    SetProperty(const ::rtl::OUString &rString,sal_Bool bIsUnknown=sal_False);
            virtual ::rtl::OUString         GetProperty()const;
    };

    //========================================================================
    //= ONumericControl
    //========================================================================
    class ONumericControl : public OCommonBehaviourControl, NumericField
    {
    protected:
            virtual long                    PreNotify( NotifyEvent& rNEvt );

    public:
                                            ONumericControl( Window* pParent,sal_uInt16 nDigits, WinBits nWinStyle = WB_TABSTOP);

            virtual void                    SetProperty(const ::rtl::OUString &rString,sal_Bool bIsUnknown=sal_False);
            virtual ::rtl::OUString         GetProperty()const;

            void                            SetMin(sal_Int32 _nMin) { NumericField::SetMin(_nMin); }
            void                            SetMax(sal_Int32 _nMax) { NumericField::SetMax(_nMax); }
    };

    //========================================================================
    //= OColorControl
    //========================================================================
    class OColorControl : public OCommonBehaviourControl, ColorListBox
    {
    protected:
            virtual long                    PreNotify( NotifyEvent& rNEvt );

    public:
                                            OColorControl( Window* pParent, WinBits nWinStyle = WB_TABSTOP |WB_DROPDOWN|WB_AUTOSIZE);

            // property value handling
            virtual void                    SetProperty(const ::rtl::OUString &rString,sal_Bool bIsUnknown=sal_False);
            virtual ::rtl::OUString         GetProperty()const;

            // list handling
            virtual sal_Bool                HasList();
            virtual void                    ClearList();
            virtual void                    InsertCtrEntry( const ::rtl::OUString& rString, sal_uInt16 nPos = LISTBOX_APPEND );

            virtual void                    SetCtrSize(const Size& rSize);

    protected:
            virtual void modified(Window* _pSource);
    };

    //========================================================================
    //= OListboxControl
    //========================================================================
    class OListboxControl : public OCommonBehaviourControl, ListBox
    {
    protected:
            virtual long                    PreNotify( NotifyEvent& rNEvt );

    public:
                                            OListboxControl( Window* pParent, WinBits nWinStyle = WB_TABSTOP|WB_SORT|WB_DROPDOWN|WB_AUTOSIZE);

            // property value handling
            virtual void                    SetProperty(const ::rtl::OUString &rString,sal_Bool bIsUnknown=sal_False);
            virtual ::rtl::OUString         GetProperty()const;

            // list handling
            virtual sal_Bool                HasList();
            virtual void                    ClearList();
            virtual void                    InsertCtrEntry( const ::rtl::OUString& rString, sal_uInt16 nPos = LISTBOX_APPEND );

            virtual void                    SetCtrSize(const Size& rSize);

    protected:
            virtual void modified(Window* _pSource);
    };

    //========================================================================
    //= OComboboxControl
    //========================================================================
    class OComboboxControl : public OCommonBehaviourControl, ComboBox
    {
    protected:
            virtual long                    PreNotify( NotifyEvent& rNEvt );


    public:
                                            OComboboxControl( Window* pParent,
                                                    WinBits nWinStyle = WB_TABSTOP | WB_SORT | WB_DROPDOWN | WB_AUTOSIZE);

            // property value handling
            virtual void                    SetProperty(const ::rtl::OUString &rString,sal_Bool bIsUnknown=sal_False);
            virtual ::rtl::OUString         GetProperty()const;

            // list handling
            virtual sal_Bool                HasList();
            virtual void                    ClearList();
            virtual void                    InsertCtrEntry( const ::rtl::OUString& rString, sal_uInt16 nPos = LISTBOX_APPEND );

            virtual void                    SetCtrSize(const Size& rSize);

    };

    //========================================================================
    //= OMultilineEditControl
    //========================================================================
    class OMultilineFloatingEdit;
    class OMultilineEditControl : public OCommonBehaviourControl, Control
    {
            OMultilineFloatingEdit*             m_pFloatingEdit;
            MultiLineEdit*                      m_pImplEdit;
            PushButton*                         m_pDropdownButton;
            sal_Bool                            m_bDropdown :1;
            sal_Bool                            m_bEdit     :1;

            DECL_LINK(ReturnHdl,OMultilineFloatingEdit*);
            DECL_LINK(DropDownHdl, PushButton*);

    protected:

            virtual long                    PreNotify( NotifyEvent& rNEvt );
            void                            Resize();

            long                            FindPos(long nSinglePos);

    public:
                                            OMultilineEditControl( Window* pParent,sal_Bool bEdit,
                                                        WinBits nWinStyle = WB_TABSTOP | WB_DROPDOWN);

                                            ~OMultilineEditControl();

            virtual void                    GetFocus();
            virtual sal_Bool                ShowDropDown( sal_Bool bShow );

            virtual void                    SetProperty(const ::rtl::OUString &rString,sal_Bool bIsUnknown=sal_False);
            virtual ::rtl::OUString         GetProperty()const;

            virtual void                    SetLocked(sal_Bool bLocked=sal_True);
    protected:
            virtual void modified(Window* _pSource);
    };
//............................................................................
} // namespace pcr
//............................................................................

#endif // _EXTENSIONS_PROPCTRLR_STANDARDCONTROL_HXX_

