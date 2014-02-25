/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef _EXTENSIONS_PROPCTRLR_STANDARDCONTROL_HXX_
#define _EXTENSIONS_PROPCTRLR_STANDARDCONTROL_HXX_

#include "commoncontrol.hxx"
#include "pcrcommon.hxx"

#include <com/sun/star/inspection/XNumericControl.hpp>
#include <com/sun/star/inspection/XStringListControl.hpp>
#include <com/sun/star/inspection/XHyperlinkControl.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <vcl/field.hxx>
#include <vcl/longcurr.hxx>
#include <svtools/ctrlbox.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <svtools/calendar.hxx>
#include <svtools/fmtfield.hxx>

#include <set>

class PushButton;
class MultiLineEdit;

namespace pcr
{



    //= ListLikeControlWithModifyHandler

    /** Very small helper class which adds a SetModifyHdl to a ListBox-derived class,
        thus giving this class the same API (as far as the CommonBehaviourControl is concerned)
        as all other windows.
    */
    template< class LISTBOX_WINDOW >
    class ListLikeControlWithModifyHandler : public ControlWindow< LISTBOX_WINDOW >
    {
    protected:
        typedef ControlWindow< LISTBOX_WINDOW >  ListBoxType;

    public:
        ListLikeControlWithModifyHandler( Window* _pParent, WinBits _nStyle )
            :ListBoxType( _pParent, _nStyle )
        {
        }

        void SetModifyHdl( const Link& _rLink ) { ListBoxType::SetSelectHdl( _rLink ); }

    protected:
        bool    PreNotify( NotifyEvent& _rNEvt );
    };


    template< class LISTBOX_WINDOW >
    bool ListLikeControlWithModifyHandler< LISTBOX_WINDOW >::PreNotify( NotifyEvent& _rNEvt )
    {
        if ( _rNEvt.GetType() == EVENT_KEYINPUT )
        {
            const ::KeyEvent* pKeyEvent = _rNEvt.GetKeyEvent();
            if  (   ( pKeyEvent->GetKeyCode().GetModifier() == 0 )
                &&  (   ( pKeyEvent->GetKeyCode().GetCode() == KEY_PAGEUP )
                    ||  ( pKeyEvent->GetKeyCode().GetCode() == KEY_PAGEDOWN )
                    )
                )
            {
                if ( !ListBoxType::IsInDropDown() )
                {
                    // don't give the base class a chance to consume the event, in the property browser, it is
                    // intended to scroll the complete property page
                    return ListBoxType::GetParent()->PreNotify( _rNEvt );
                }
            }
        }
        return ListBoxType::PreNotify( _rNEvt );
    }


    //= OTimeControl

    typedef CommonBehaviourControl< ::com::sun::star::inspection::XPropertyControl, ControlWindow< TimeField > > OTimeControl_Base;
    class OTimeControl : public OTimeControl_Base
    {
    public:
        OTimeControl( Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual ::com::sun::star::uno::Any SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setValue( const ::com::sun::star::uno::Any& _value ) throw (::com::sun::star::beans::IllegalTypeException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Type SAL_CALL getValueType() throw (::com::sun::star::uno::RuntimeException, std::exception);
    };


    //= ODateControl

    typedef CommonBehaviourControl< ::com::sun::star::inspection::XPropertyControl, ControlWindow< CalendarField > > ODateControl_Base;
    class ODateControl : public ODateControl_Base
    {
    public:
        ODateControl( Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual ::com::sun::star::uno::Any SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setValue( const ::com::sun::star::uno::Any& _value ) throw (::com::sun::star::beans::IllegalTypeException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Type SAL_CALL getValueType() throw (::com::sun::star::uno::RuntimeException, std::exception);
    };


    //= OEditControl

    typedef CommonBehaviourControl< ::com::sun::star::inspection::XPropertyControl, ControlWindow< Edit > > OEditControl_Base;
    class OEditControl : public OEditControl_Base
    {
    protected:
        sal_Bool m_bIsPassword : 1;

    public:
        OEditControl( Window* _pParent, sal_Bool _bPassWord, WinBits nWinStyle );

        // XPropertyControl
        virtual ::com::sun::star::uno::Any SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setValue( const ::com::sun::star::uno::Any& _value ) throw (::com::sun::star::beans::IllegalTypeException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Type SAL_CALL getValueType() throw (::com::sun::star::uno::RuntimeException, std::exception);

    protected:
        virtual void modified();
    };


    //= ODateTimeControl

    typedef CommonBehaviourControl< ::com::sun::star::inspection::XPropertyControl, ControlWindow< FormattedField > > ODateTimeControl_Base;
    class ODateTimeControl : public ODateTimeControl_Base
    {
    public:
        ODateTimeControl( Window* pParent,WinBits nWinStyle );

        // XPropertyControl
        virtual ::com::sun::star::uno::Any SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setValue( const ::com::sun::star::uno::Any& _value ) throw (::com::sun::star::beans::IllegalTypeException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Type SAL_CALL getValueType() throw (::com::sun::star::uno::RuntimeException, std::exception);
    };


    //= HyperlinkInput

    class HyperlinkInput : public Edit
    {
    private:
        Point   m_aMouseButtonDownPos;
        Link    m_aClickHandler;

    public:
        HyperlinkInput( Window* _pParent, WinBits _nWinStyle );

        /** sets the handler which will (asynchronously, with locked SolarMutex) be called
            when the hyperlink has been clicked by the user
        */
        void        SetClickHdl( const Link& _rHdl ) { m_aClickHandler = _rHdl; }
        const Link& GetClickHdl( ) const { return m_aClickHandler; }

    protected:
        virtual void        MouseMove( const MouseEvent& rMEvt );
        virtual void        MouseButtonDown( const MouseEvent& rMEvt );
        virtual void        MouseButtonUp( const MouseEvent& rMEvt );
        virtual void        Tracking( const TrackingEvent& rTEvt );

    private:
        void    impl_checkEndClick( const MouseEvent rMEvt );
        bool    impl_textHitTest( const Point& _rWindowPos );
    };


    //= OHyperlinkControl

    typedef CommonBehaviourControl< ::com::sun::star::inspection::XHyperlinkControl, ControlWindow< HyperlinkInput > > OHyperlinkControl_Base;
    class OHyperlinkControl : public OHyperlinkControl_Base
    {
    private:
        ::cppu::OInterfaceContainerHelper   m_aActionListeners;

    public:
        OHyperlinkControl( Window* _pParent, WinBits _nWinStyle );

        // XPropertyControl
        virtual ::com::sun::star::uno::Any SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setValue( const ::com::sun::star::uno::Any& _value ) throw (::com::sun::star::beans::IllegalTypeException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Type SAL_CALL getValueType() throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XHyperlinkControl
        virtual void SAL_CALL addActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& listener ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL removeActionListener( const ::com::sun::star::uno::Reference< ::com::sun::star::awt::XActionListener >& listener ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    protected:
        // XComponent
        virtual void SAL_CALL disposing();

    protected:
        DECL_LINK( OnHyperlinkClicked, void* );
    };


    //= CustomConvertibleNumericField

    class CustomConvertibleNumericField : public ControlWindow< MetricField >
    {
        typedef ControlWindow< MetricField > BaseClass;

    public:
        CustomConvertibleNumericField( Window* _pParent, WinBits _nStyle )
            :BaseClass( _pParent, _nStyle )
        {
        }

        sal_Int64 GetLastValue() const { return mnLastValue; }
    };


    //= ONumericControl

    typedef CommonBehaviourControl< ::com::sun::star::inspection::XNumericControl, CustomConvertibleNumericField > ONumericControl_Base;
    class ONumericControl : public ONumericControl_Base
    {
    private:
        FieldUnit   m_eValueUnit;
        sal_Int16   m_nFieldToUNOValueFactor;

    public:
        ONumericControl( Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual ::com::sun::star::uno::Any SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setValue( const ::com::sun::star::uno::Any& _value ) throw (::com::sun::star::beans::IllegalTypeException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Type SAL_CALL getValueType() throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XNumericControl
        virtual ::sal_Int16 SAL_CALL getDecimalDigits() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setDecimalDigits( ::sal_Int16 _decimaldigits ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::beans::Optional< double > SAL_CALL getMinValue() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setMinValue( const ::com::sun::star::beans::Optional< double >& _minvalue ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::beans::Optional< double > SAL_CALL getMaxValue() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setMaxValue( const ::com::sun::star::beans::Optional< double >& _maxvalue ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::sal_Int16 SAL_CALL getDisplayUnit() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setDisplayUnit( ::sal_Int16 _displayunit ) throw (::com::sun::star::lang::IllegalArgumentException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::sal_Int16 SAL_CALL getValueUnit() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setValueUnit( ::sal_Int16 _valueunit ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    private:
        /** converts an API value (<code>double</code>, as passed into <code>set[Max|Min|]Value) into
            a <code>long</code> value which can be passed to our NumericField.

            The conversion respects our decimal digits as well as our value factor (<member>m_nFieldToUNOValueFactor</member>).
        */
        long    impl_apiValueToFieldValue_nothrow( double _nApiValue ) const;

        /** converts a control value, as obtained from our Numeric field, into a value which can passed
            to outer callers via our UNO API.
        */
        double  impl_fieldValueToApiValue_nothrow( sal_Int64 _nFieldValue ) const;
    };


    //= OColorControl

    typedef CommonBehaviourControl  <   ::com::sun::star::inspection::XStringListControl
                                    ,   ListLikeControlWithModifyHandler< ColorListBox >
                                    >   OColorControl_Base;
    class OColorControl : public OColorControl_Base
    {
    private:
        ::std::set< OUString >   m_aNonColorEntries;

    public:
        OColorControl( Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual ::com::sun::star::uno::Any SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setValue( const ::com::sun::star::uno::Any& _value ) throw (::com::sun::star::beans::IllegalTypeException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Type SAL_CALL getValueType() throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XStringListControl
        virtual void SAL_CALL clearList(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL prependListEntry( const OUString& NewEntry ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL appendListEntry( const OUString& NewEntry ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getListEntries(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    protected:
        virtual void modified();
    };


    //= OListboxControl

    typedef CommonBehaviourControl  <   ::com::sun::star::inspection::XStringListControl
                                    ,   ListLikeControlWithModifyHandler< ListBox >
                                    >   OListboxControl_Base;
    class OListboxControl : public OListboxControl_Base
    {
    public:
        OListboxControl( Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual ::com::sun::star::uno::Any SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setValue( const ::com::sun::star::uno::Any& _value ) throw (::com::sun::star::beans::IllegalTypeException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Type SAL_CALL getValueType() throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XStringListControl
        virtual void SAL_CALL clearList(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL prependListEntry( const OUString& NewEntry ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL appendListEntry( const OUString& NewEntry ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getListEntries(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    protected:
        virtual void modified();
    };


    //= OComboboxControl

    typedef CommonBehaviourControl< ::com::sun::star::inspection::XStringListControl, ControlWindow< ComboBox > > OComboboxControl_Base;
    class OComboboxControl : public OComboboxControl_Base
    {
    public:
        OComboboxControl( Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual ::com::sun::star::uno::Any SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setValue( const ::com::sun::star::uno::Any& _value ) throw (::com::sun::star::beans::IllegalTypeException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Type SAL_CALL getValueType() throw (::com::sun::star::uno::RuntimeException, std::exception);

        // XStringListControl
        virtual void SAL_CALL clearList(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL prependListEntry( const OUString& NewEntry ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL appendListEntry( const OUString& NewEntry ) throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Sequence< OUString > SAL_CALL getListEntries(  ) throw (::com::sun::star::uno::RuntimeException, std::exception);

    protected:
        DECL_LINK( OnEntrySelected, void* );
    };


    //= DropDownEditControl

    enum MultiLineOperationMode
    {
        eStringList,
        eMultiLineText
    };

    //= DropDownEditControl

    class OMultilineFloatingEdit;
    typedef ControlWindow< Edit > DropDownEditControl_Base;
    /** an Edit field which can be used as ControlWindow, and has a drop-down button
    */
    class DropDownEditControl : public DropDownEditControl_Base
    {
    private:
        OMultilineFloatingEdit*             m_pFloatingEdit;
        MultiLineEdit*                      m_pImplEdit;
        PushButton*                         m_pDropdownButton;
        MultiLineOperationMode              m_nOperationMode;
        sal_Bool                            m_bDropdown : 1;

    public:
        DropDownEditControl( Window* _pParent, WinBits _nStyle );
        ~DropDownEditControl();

        void setOperationMode( MultiLineOperationMode _eMode ) { m_nOperationMode = _eMode; }
        MultiLineOperationMode getOperationMode() const { return m_nOperationMode; }

        void            SetTextValue( const OUString& _rText );
        OUString GetTextValue() const;

        void            SetStringListValue( const StlSyntaxSequence< OUString >& _rStrings );
        StlSyntaxSequence< OUString >
                        GetStringListValue() const;

        // ControlWindow overridables
        virtual void setControlHelper( ControlHelper& _rControlHelper );

    protected:
        // Window overridables
        virtual bool    PreNotify( NotifyEvent& rNEvt );
        virtual void    Resize();

    protected:
        long            FindPos(long nSinglePos);

    private:
        DECL_LINK( ReturnHdl, OMultilineFloatingEdit* );
        DECL_LINK( DropDownHdl, PushButton* );

        sal_Bool ShowDropDown( sal_Bool bShow );
    };


    //= OMultilineEditControl

    typedef CommonBehaviourControl< ::com::sun::star::inspection::XPropertyControl, DropDownEditControl > OMultilineEditControl_Base;
    class OMultilineEditControl : public OMultilineEditControl_Base
    {
    public:
        OMultilineEditControl( Window* pParent, MultiLineOperationMode _eMode, WinBits nWinStyle  );

        // XPropertyControl
        virtual ::com::sun::star::uno::Any SAL_CALL getValue() throw (::com::sun::star::uno::RuntimeException, std::exception);
        virtual void SAL_CALL setValue( const ::com::sun::star::uno::Any& _value ) throw (::com::sun::star::beans::IllegalTypeException, ::com::sun::star::uno::RuntimeException, std::exception);
        virtual ::com::sun::star::uno::Type SAL_CALL getValueType() throw (::com::sun::star::uno::RuntimeException, std::exception);
    };


} // namespace pcr


#endif // _EXTENSIONS_PROPCTRLR_STANDARDCONTROL_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
