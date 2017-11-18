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
#include <svx/colorbox.hxx>

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
    class ListLikeControlWithModifyHandler : public TListboxWindow
    {
    public:
        ListLikeControlWithModifyHandler( vcl::Window* _pParent, WinBits _nStyle )
            : TListboxWindow( _pParent, _nStyle )
        {
            TListboxWindow::SetSelectHdl( LINK(this, ListLikeControlWithModifyHandler, OnSelect) );
        }

        void SetModifyHdl( const Link<TListboxWindow&,void>& _rLink ) { aModifyHdl = _rLink; }
    private:
        DECL_LINK(OnSelect, TListboxWindow&, void);
        Link<TListboxWindow&,void> aModifyHdl;
    };

    template< class LISTBOX_WINDOW >
    void ListLikeControlWithModifyHandler< LISTBOX_WINDOW >::LinkStubOnSelect(void * instance, LISTBOX_WINDOW& data) {
        return static_cast<ListLikeControlWithModifyHandler< LISTBOX_WINDOW > *>(instance)->OnSelect(data);
    }
    template< class LISTBOX_WINDOW >
    void ListLikeControlWithModifyHandler< LISTBOX_WINDOW >::OnSelect(LISTBOX_WINDOW& rListBox)
    {
        aModifyHdl.Call(rListBox);
    }

    //= OTimeControl

    typedef CommonBehaviourControl< css::inspection::XPropertyControl, TimeField > OTimeControl_Base;
    class OTimeControl : public OTimeControl_Base
    {
    public:
        OTimeControl( vcl::Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;
    };


    //= ODateControl

    typedef CommonBehaviourControl< css::inspection::XPropertyControl, CalendarField > ODateControl_Base;
    class ODateControl : public ODateControl_Base
    {
    public:
        ODateControl( vcl::Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;
    };


    //= OEditControl

    typedef CommonBehaviourControl< css::inspection::XPropertyControl, Edit > OEditControl_Base;
    class OEditControl final : public OEditControl_Base
    {
        bool m_bIsPassword : 1;

    public:
        OEditControl( vcl::Window* _pParent, bool _bPassWord, WinBits nWinStyle );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;

    private:
        // CommonBehaviourControlHelper::modified
        virtual void setModified() override;
    };


    //= ODateTimeControl

    typedef CommonBehaviourControl< css::inspection::XPropertyControl, FormattedField > ODateTimeControl_Base;
    class ODateTimeControl : public ODateTimeControl_Base
    {
    public:
        ODateTimeControl( vcl::Window* pParent,WinBits nWinStyle );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;
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
        virtual void        MouseMove( const MouseEvent& rMEvt ) override;
        virtual void        MouseButtonDown( const MouseEvent& rMEvt ) override;
        virtual void        MouseButtonUp( const MouseEvent& rMEvt ) override;
        virtual void        Tracking( const TrackingEvent& rTEvt ) override;

    private:
        void    impl_checkEndClick( const MouseEvent& rMEvt );
        bool    impl_textHitTest( const Point& rWindowPos );
    };


    //= OHyperlinkControl

    typedef CommonBehaviourControl< css::inspection::XHyperlinkControl, HyperlinkInput > OHyperlinkControl_Base;
    class OHyperlinkControl final : public OHyperlinkControl_Base
    {
        ::comphelper::OInterfaceContainerHelper2   m_aActionListeners;

    public:
        OHyperlinkControl( vcl::Window* _pParent, WinBits _nWinStyle );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;

        // XHyperlinkControl
        virtual void SAL_CALL addActionListener( const css::uno::Reference< css::awt::XActionListener >& listener ) override;
        virtual void SAL_CALL removeActionListener( const css::uno::Reference< css::awt::XActionListener >& listener ) override;

    private:
        // XComponent
        virtual void SAL_CALL disposing() override;

        DECL_LINK( OnHyperlinkClicked, void*, void );
    };


    //= CustomConvertibleNumericField

    class CustomConvertibleNumericField : public MetricField
    {
    public:
        CustomConvertibleNumericField( vcl::Window* _pParent, WinBits _nStyle )
            :MetricField( _pParent, _nStyle )
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
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;

        // XNumericControl
        virtual ::sal_Int16 SAL_CALL getDecimalDigits() override;
        virtual void SAL_CALL setDecimalDigits( ::sal_Int16 _decimaldigits ) override;
        virtual css::beans::Optional< double > SAL_CALL getMinValue() override;
        virtual void SAL_CALL setMinValue( const css::beans::Optional< double >& _minvalue ) override;
        virtual css::beans::Optional< double > SAL_CALL getMaxValue() override;
        virtual void SAL_CALL setMaxValue( const css::beans::Optional< double >& _maxvalue ) override;
        virtual ::sal_Int16 SAL_CALL getDisplayUnit() override;
        virtual void SAL_CALL setDisplayUnit( ::sal_Int16 _displayunit ) override;
        virtual ::sal_Int16 SAL_CALL getValueUnit() override;
        virtual void SAL_CALL setValueUnit( ::sal_Int16 _valueunit ) override;

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

    typedef CommonBehaviourControl  <   css::inspection::XPropertyControl
                                    ,   ListLikeControlWithModifyHandler<SvxColorListBox>
                                    >   OColorControl_Base;
    class OColorControl : public OColorControl_Base
    {
    public:
        OColorControl( vcl::Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;

    protected:
        // CommonBehaviourControlHelper::setModified
        virtual void setModified() override;
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
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;

        // XStringListControl
        virtual void SAL_CALL clearList(  ) override;
        virtual void SAL_CALL prependListEntry( const OUString& NewEntry ) override;
        virtual void SAL_CALL appendListEntry( const OUString& NewEntry ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getListEntries(  ) override;

    protected:
        // CommonBehaviourControlHelper::setModified
        virtual void setModified() override;
    };


    //= OComboboxControl

    typedef CommonBehaviourControl< css::inspection::XStringListControl, ComboBox > OComboboxControl_Base;
    class OComboboxControl final : public OComboboxControl_Base
    {
    public:
        OComboboxControl( vcl::Window* pParent, WinBits nWinStyle );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;

        // XStringListControl
        virtual void SAL_CALL clearList(  ) override;
        virtual void SAL_CALL prependListEntry( const OUString& NewEntry ) override;
        virtual void SAL_CALL appendListEntry( const OUString& NewEntry ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getListEntries(  ) override;

    private:
        DECL_LINK( OnEntrySelected, ComboBox&, void );
    };


    //= DropDownEditControl

    enum MultiLineOperationMode
    {
        eStringList,
        eMultiLineText
    };

    //= DropDownEditControl

    class OMultilineFloatingEdit;
    /** an Edit field which can be used as ControlWindow, and has a drop-down button
    */
    class DropDownEditControl final : public Edit
    {
        VclPtr<OMultilineFloatingEdit>      m_pFloatingEdit;
        VclPtr<MultiLineEdit>               m_pImplEdit;
        VclPtr<PushButton>                  m_pDropdownButton;
        MultiLineOperationMode              m_nOperationMode;
        bool                                m_bDropdown : 1;
        CommonBehaviourControlHelper*       m_pHelper;

    public:
        DropDownEditControl( vcl::Window* _pParent, WinBits _nStyle );
        virtual ~DropDownEditControl() override;
        virtual void dispose() override;

        void           setControlHelper( CommonBehaviourControlHelper& _rControlHelper );
        void setOperationMode( MultiLineOperationMode _eMode ) { m_nOperationMode = _eMode; }
        MultiLineOperationMode getOperationMode() const { return m_nOperationMode; }

        void            SetTextValue( const OUString& _rText );
        OUString        GetTextValue() const;

        void            SetStringListValue( const StlSyntaxSequence< OUString >& _rStrings );
        StlSyntaxSequence< OUString >
                        GetStringListValue() const;

    private:
        // Window overridables
        virtual bool    PreNotify( NotifyEvent& rNEvt ) override;
        virtual void    Resize() override;

        long            FindPos(long nSinglePos);

        DECL_LINK( ReturnHdl, FloatingWindow*, void );
        DECL_LINK( DropDownHdl, Button*, void );

        void ShowDropDown( bool bShow );
    };


    //= OMultilineEditControl

    typedef CommonBehaviourControl< css::inspection::XPropertyControl, DropDownEditControl > OMultilineEditControl_Base;
    class OMultilineEditControl : public OMultilineEditControl_Base
    {
    public:
        OMultilineEditControl( vcl::Window* pParent, MultiLineOperationMode _eMode, WinBits nWinStyle  );

        // XPropertyControl
        virtual css::uno::Any SAL_CALL getValue() override;
        virtual void SAL_CALL setValue( const css::uno::Any& _value ) override;
        virtual css::uno::Type SAL_CALL getValueType() override;
    };


} // namespace pcr


#endif // INCLUDED_EXTENSIONS_SOURCE_PROPCTRLR_STANDARDCONTROL_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
