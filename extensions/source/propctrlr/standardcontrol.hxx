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

#ifndef INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_STANDARDCONTROL_HXX
#define INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_STANDARDCONTROL_HXX

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
    template< class TListboxWindow >
    class ListLikeControlWithModifyHandler : public ControlWindow< TListboxWindow >
    {
    protected:
        typedef ControlWindow< TListboxWindow >  ListBoxType;

    public:
        ListLikeControlWithModifyHandler( vcl::Window* _pParent, WinBits _nStyle )
            :ListBoxType( _pParent, _nStyle )
        {
        }

        void SetModifyHdl( const Link<>& _rLink ) { ListBoxType::SetSelectHdl( _rLink ); }
    };


    //= OTimeControl

    typedef CommonBehaviourControl< css::inspection::XPropertyControl, ControlWindow< TimeField > > OTimeControl_Base;
    class OTimeControl : public OTimeControl_Base
    {
    public:
        OTimeControl( vcl::Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::beans::IllegalTypeException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Type SAL_CALL getValueType() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    };


    //= ODateControl

    typedef CommonBehaviourControl< css::inspection::XPropertyControl, ControlWindow< CalendarField > > ODateControl_Base;
    class ODateControl : public ODateControl_Base
    {
    public:
        ODateControl( vcl::Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::beans::IllegalTypeException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Type SAL_CALL getValueType() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    };


    //= OEditControl

    typedef CommonBehaviourControl< css::inspection::XPropertyControl, ControlWindow< Edit > > OEditControl_Base;
    class OEditControl : public OEditControl_Base
    {
    protected:
        bool m_bIsPassword : 1;

    public:
        OEditControl( vcl::Window* _pParent, bool _bPassWord, WinBits nWinStyle );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::beans::IllegalTypeException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Type SAL_CALL getValueType() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    protected:
        virtual void modified() SAL_OVERRIDE;
    };


    //= ODateTimeControl

    typedef CommonBehaviourControl< css::inspection::XPropertyControl, ControlWindow< FormattedField > > ODateTimeControl_Base;
    class ODateTimeControl : public ODateTimeControl_Base
    {
    public:
        ODateTimeControl( vcl::Window* pParent,WinBits nWinStyle );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::beans::IllegalTypeException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Type SAL_CALL getValueType() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    };


    //= HyperlinkInput

    class HyperlinkInput : public Edit
    {
    private:
        Point             m_aMouseButtonDownPos;
        Link<void*,void>  m_aClickHandler;

    public:
        HyperlinkInput( vcl::Window* _pParent, WinBits _nWinStyle );

        /** sets the handler which will (asynchronously, with locked SolarMutex) be called
            when the hyperlink has been clicked by the user
        */
        void    SetClickHdl( const Link<void*,void>& _rHdl ) { m_aClickHandler = _rHdl; }

    protected:
        virtual void        MouseMove( const MouseEvent& rMEvt ) SAL_OVERRIDE;
        virtual void        MouseButtonDown( const MouseEvent& rMEvt ) SAL_OVERRIDE;
        virtual void        MouseButtonUp( const MouseEvent& rMEvt ) SAL_OVERRIDE;
        virtual void        Tracking( const TrackingEvent& rTEvt ) SAL_OVERRIDE;

    private:
        void    impl_checkEndClick( const MouseEvent& rMEvt );
        bool    impl_textHitTest( const Point& rWindowPos );
    };


    //= OHyperlinkControl

    typedef CommonBehaviourControl< css::inspection::XHyperlinkControl, ControlWindow< HyperlinkInput > > OHyperlinkControl_Base;
    class OHyperlinkControl : public OHyperlinkControl_Base
    {
    private:
        ::cppu::OInterfaceContainerHelper   m_aActionListeners;

    public:
        OHyperlinkControl( vcl::Window* _pParent, WinBits _nWinStyle );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::beans::IllegalTypeException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Type SAL_CALL getValueType() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XHyperlinkControl
        virtual void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& listener ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& listener ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    protected:
        // XComponent
        virtual void SAL_CALL disposing() SAL_OVERRIDE;

    protected:
        DECL_LINK_TYPED( OnHyperlinkClicked, void*, void );
    };


    //= CustomConvertibleNumericField

    class CustomConvertibleNumericField : public ControlWindow< MetricField >
    {
        typedef ControlWindow< MetricField > BaseClass;

    public:
        CustomConvertibleNumericField( vcl::Window* _pParent, WinBits _nStyle )
            :BaseClass( _pParent, _nStyle )
        {
        }

        sal_Int64 GetLastValue() const { return mnLastValue; }
    };


    //= ONumericControl

    typedef CommonBehaviourControl< css::inspection::XNumericControl, CustomConvertibleNumericField > ONumericControl_Base;
    class ONumericControl : public ONumericControl_Base
    {
    private:
        FieldUnit   m_eValueUnit;
        sal_Int16   m_nFieldToUNOValueFactor;

    public:
        ONumericControl( vcl::Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::beans::IllegalTypeException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Type SAL_CALL getValueType() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XNumericControl
        virtual ::sal_Int16 SAL_CALL getDecimalDigits() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setDecimalDigits( ::sal_Int16 _decimaldigits ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::beans::Optional< double > SAL_CALL getMinValue() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setMinValue( const css::beans::Optional< double >& _minvalue ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::beans::Optional< double > SAL_CALL getMaxValue() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setMaxValue( const css::beans::Optional< double >& _maxvalue ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::sal_Int16 SAL_CALL getDisplayUnit() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setDisplayUnit( ::sal_Int16 _displayunit ) throw (css::lang::IllegalArgumentException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual ::sal_Int16 SAL_CALL getValueUnit() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setValueUnit( ::sal_Int16 _valueunit ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

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

    typedef CommonBehaviourControl  <   css::inspection::XStringListControl
                                    ,   ListLikeControlWithModifyHandler< ColorListBox >
                                    >   OColorControl_Base;
    class OColorControl : public OColorControl_Base
    {
    private:
        ::std::set< OUString >   m_aNonColorEntries;

    public:
        OColorControl( vcl::Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::beans::IllegalTypeException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Type SAL_CALL getValueType() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XStringListControl
        virtual void SAL_CALL clearList(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL prependListEntry( const OUString& NewEntry ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL appendListEntry( const OUString& NewEntry ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< OUString > SAL_CALL getListEntries(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    protected:
        virtual void modified() SAL_OVERRIDE;
    };


    //= OListboxControl

    typedef CommonBehaviourControl  <   css::inspection::XStringListControl
                                    ,   ListLikeControlWithModifyHandler< ListBox >
                                    >   OListboxControl_Base;
    class OListboxControl : public OListboxControl_Base
    {
    public:
        OListboxControl( vcl::Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::beans::IllegalTypeException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Type SAL_CALL getValueType() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XStringListControl
        virtual void SAL_CALL clearList(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL prependListEntry( const OUString& NewEntry ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL appendListEntry( const OUString& NewEntry ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< OUString > SAL_CALL getListEntries(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

    protected:
        virtual void modified() SAL_OVERRIDE;
    };


    //= OComboboxControl

    typedef CommonBehaviourControl< css::inspection::XStringListControl, ControlWindow< ComboBox > > OComboboxControl_Base;
    class OComboboxControl : public OComboboxControl_Base
    {
    public:
        OComboboxControl( vcl::Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::beans::IllegalTypeException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Type SAL_CALL getValueType() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

        // XStringListControl
        virtual void SAL_CALL clearList(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL prependListEntry( const OUString& NewEntry ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL appendListEntry( const OUString& NewEntry ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Sequence< OUString > SAL_CALL getListEntries(  ) throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;

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
        VclPtr<OMultilineFloatingEdit>             m_pFloatingEdit;
        VclPtr<MultiLineEdit>               m_pImplEdit;
        VclPtr<PushButton>                         m_pDropdownButton;
        MultiLineOperationMode              m_nOperationMode;
        bool                                m_bDropdown : 1;

    public:
        DropDownEditControl( vcl::Window* _pParent, WinBits _nStyle );
        virtual ~DropDownEditControl();
        virtual void dispose() SAL_OVERRIDE;

        void setOperationMode( MultiLineOperationMode _eMode ) { m_nOperationMode = _eMode; }
        MultiLineOperationMode getOperationMode() const { return m_nOperationMode; }

        void            SetTextValue( const OUString& _rText );
        OUString        GetTextValue() const;

        void            SetStringListValue( const StlSyntaxSequence< OUString >& _rStrings );
        StlSyntaxSequence< OUString >
                        GetStringListValue() const;

        // ControlWindow overridables
        virtual void setControlHelper( ControlHelper& _rControlHelper ) SAL_OVERRIDE;

    protected:
        // Window overridables
        virtual bool    PreNotify( NotifyEvent& rNEvt ) SAL_OVERRIDE;
        virtual void    Resize() SAL_OVERRIDE;

    protected:
        long            FindPos(long nSinglePos);

    private:
        DECL_LINK_TYPED( ReturnHdl, FloatingWindow*, void );
        DECL_LINK_TYPED( DropDownHdl, Button*, void );

        bool ShowDropDown( bool bShow );
    };


    //= OMultilineEditControl

    typedef CommonBehaviourControl< css::inspection::XPropertyControl, DropDownEditControl > OMultilineEditControl_Base;
    class OMultilineEditControl : public OMultilineEditControl_Base
    {
    public:
        OMultilineEditControl( vcl::Window* pParent, MultiLineOperationMode _eMode, WinBits nWinStyle  );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) throw (css::beans::IllegalTypeException, css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
        virtual css::uno::Type SAL_CALL getValueType() throw (css::uno::RuntimeException, std::exception) SAL_OVERRIDE;
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_STANDARDCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
