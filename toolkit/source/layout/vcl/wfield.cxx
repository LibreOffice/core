/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#include "wrapper.hxx"

#include <comphelper/processfactory.hxx>
#include <com/sun/star/awt/XMetricField.hpp>
#include <com/sun/star/awt/XNumericField.hpp>
#include <com/sun/star/awt/XTextComponent.hpp>
#include <com/sun/star/awt/XListBox.hpp>
#include <com/sun/star/awt/XComboBox.hpp>
#include <cppuhelper/implbase1.hxx>
#include <com/sun/star/awt/XActionListener.hpp>
#include <com/sun/star/awt/XItemListener.hpp>
#include <com/sun/star/awt/XMouseListener.hpp>
#include <vcl/combobox.hxx>
#include <vcl/lstbox.hxx>

#include <toolkit/awt/vclxwindows.hxx>

using namespace ::com::sun::star;
using rtl::OUString;

#define LAYOUT_API_CALLS_HANDLER 0

namespace layout
{

class EditImpl : public ControlImpl
               , public ::cppu::WeakImplHelper1< awt::XTextListener >
{
public:
    Link maModifyHdl;

    uno::Reference< awt::XTextComponent > mxEdit;
    EditImpl( Context *context, const PeerHandle &peer, Window *window )
        : ControlImpl( context, peer, window )
        , mxEdit( peer, uno::UNO_QUERY )
    {
    }

    ~EditImpl ();

    virtual void SAL_CALL disposing( lang::EventObject const& e )
        throw (uno::RuntimeException);

    virtual void SetModifyHdl( Link const& link );

    void SAL_CALL textChanged( const awt::TextEvent& /* rEvent */ )
        throw (uno::RuntimeException)
    {
        maModifyHdl.Call( mpWindow );
    }
};

EditImpl::~EditImpl ()
{
}

void SAL_CALL EditImpl::disposing( lang::EventObject const& e )
    throw (uno::RuntimeException)
{
    ControlImpl::disposing (e);
    mxEdit.clear ();
}

void EditImpl::SetModifyHdl( Link const& link )
{
    if (!link && !!maModifyHdl)
        mxEdit->removeTextListener( this );
    else if (!!link && !maModifyHdl)
        mxEdit->addTextListener( this );
    maModifyHdl = link;
}

Edit::~Edit ()
{
    SetModifyHdl (Link ());
}

void Edit::SetSelection( Selection const& rSelection )
{
#if LAYOUT_API_CALLS_HANDLER
    if ( !getImpl().mxEdit.is() )
        getImpl().mxEdit->setSelection( awt::Selection( rSelection.Min(), rSelection.Max() ) );
#else /* !LAYOUT_API_CALLS_HANDLER */
    GetEdit ()->SetSelection (rSelection);
#endif /* !LAYOUT_API_CALLS_HANDLER */
}

void Edit::SetText( OUString const& rStr )
{
#if LAYOUT_API_CALLS_HANDLER
    if ( getImpl().mxEdit.is() )
        /// this calls handlers; endless loop in numfmt.cxx
        getImpl().mxEdit->setText( rStr );
#else /* !LAYOUT_API_CALLS_HANDLER */
    GetEdit ()->SetText (rStr);
#endif /* !LAYOUT_API_CALLS_HANDLER */
}

String Edit::GetText() const
{
    if ( !getImpl().mxEdit.is() )
        return getImpl().mxEdit->getText();
    return OUString();
}

void Edit::SetModifyHdl( const Link& link )
{
    if (&getImpl () && getImpl().mxEdit.is ())
        getImpl().SetModifyHdl( link );
}

IMPL_CONSTRUCTORS( Edit, Control, "edit" );
IMPL_GET_IMPL( Edit );
IMPL_GET_WINDOW (Edit);

class MultiLineEditImpl : public EditImpl
{
public:
    MultiLineEditImpl( Context *context, const PeerHandle &peer, Window *window )
        : EditImpl( context, peer, window )
    {
    }
};

IMPL_CONSTRUCTORS( MultiLineEdit, Edit, "multilineedit" );
IMPL_GET_IMPL( MultiLineEdit );

class SpinFieldImpl : public EditImpl
{
  public:
    SpinFieldImpl( Context *context, const PeerHandle &peer, Window *window )
        : EditImpl( context, peer, window )
    {
    }
};

IMPL_CONSTRUCTORS( SpinField, Edit, "spinfield" );

class NumericFieldImpl : public SpinFieldImpl
{
  public:
    NumericFieldImpl( Context *context, const PeerHandle &peer, Window *window )
        : SpinFieldImpl( context, peer, window )
    {
    }
};

class MetricFieldImpl : public SpinFieldImpl
{
  public:
    MetricFieldImpl( Context *context, const PeerHandle &peer, Window *window )
        : SpinFieldImpl( context, peer, window )
    {
    }
};

IMPL_GET_IMPL( SpinField );
IMPL_GET_IMPL( NumericField );
IMPL_GET_IMPL( MetricField );

class FormatterBaseImpl
{
  protected:
    PeerHandle mpeer;
  public:
    explicit FormatterBaseImpl( const PeerHandle &peer )
        : mpeer( peer )
    {
    };
};

FormatterBase::FormatterBase( FormatterBaseImpl *pFormatImpl )
    : mpFormatImpl( pFormatImpl )
{
}

class NumericFormatterImpl : public FormatterBaseImpl
{
  public:
    uno::Reference< awt::XNumericField > mxField;
    explicit NumericFormatterImpl( const PeerHandle &peer )
        : FormatterBaseImpl( peer )
        , mxField( peer, uno::UNO_QUERY )
    {
    }

    // FIXME: burn that CPU ! cut/paste from vclxwindows.cxx
    double valueToDouble( sal_Int64 nValue )
    {
        sal_Int16 nDigits = mxField->getDecimalDigits();
        double n = (double)nValue;
        for ( sal_uInt16 d = 0; d < nDigits; d++ )
            n /= 10;
        return n;
    } // FIXME: burn that CPU ! cut/paste from vclxwindows.cxx
    sal_Int64 doubleToValue( double nValue )
    {
        sal_Int16 nDigits = mxField->getDecimalDigits();
        double n = nValue;
        for ( sal_uInt16 d = 0; d < nDigits; d++ )
            n *= 10;
        return (sal_Int64) n;
    }
};

class MetricFormatterImpl : public FormatterBaseImpl
{
  public:
    uno::Reference< awt::XMetricField > mxField;
    explicit MetricFormatterImpl( const PeerHandle &peer )
        : FormatterBaseImpl( peer )
        , mxField( peer, uno::UNO_QUERY )
    {
    }
};

NumericFormatter::NumericFormatter( FormatterBaseImpl *pImpl )
    : FormatterBase( pImpl )
{
}

NumericFormatterImpl& NumericFormatter::getFormatImpl() const
{
    return *( static_cast<NumericFormatterImpl *>( mpFormatImpl ) );
}

#define SET_IMPL(vclmethod, idlmethod) \
    void NumericFormatter::vclmethod( sal_Int64 nValue ) \
    { \
        if ( !getFormatImpl().mxField.is() ) \
            return; \
        getFormatImpl().mxField->idlmethod( getFormatImpl().valueToDouble( nValue ) ); \
    }

SET_IMPL( SetMin, setMin )
SET_IMPL( SetMax, setMax )
SET_IMPL( SetLast, setLast )
SET_IMPL( SetFirst, setFirst )
SET_IMPL( SetValue, setValue )
SET_IMPL( SetSpinSize, setSpinSize )

sal_Int64 NumericFormatter::GetValue() const
{
    if ( !getFormatImpl().mxField.is() )
        return 0;
    return getFormatImpl().doubleToValue( getFormatImpl().mxField->getValue() );
}

#undef SET_IMPL

IMPL_CONSTRUCTORS_2( NumericField, SpinField, NumericFormatter, "numericfield" );

MetricFormatter::MetricFormatter( FormatterBaseImpl *pImpl )
    : FormatterBase( pImpl )
{
}
MetricFormatterImpl& MetricFormatter::getFormatImpl() const
{    return *( static_cast<MetricFormatterImpl *>( mpFormatImpl ) );   }

#define MetricUnitVclToUno(a) ((sal_uInt16)(a))

#define SET_IMPL(vclmethod, idlmethod) \
    void MetricFormatter::vclmethod( sal_Int64 nValue, FieldUnit nUnit ) \
    { \
        if ( !getFormatImpl().mxField.is() ) \
            return; \
        getFormatImpl().mxField->idlmethod( nValue, MetricUnitVclToUno( nUnit ) ); \
    }

SET_IMPL( SetMin, setMin )
SET_IMPL( SetMax, setMax )
SET_IMPL( SetLast, setLast )
SET_IMPL( SetFirst, setFirst )
SET_IMPL( SetValue, setValue )

#undef SET_IMPL

void MetricFormatter::SetSpinSize( sal_Int64 nValue )
{
    if ( !getFormatImpl().mxField.is() )
        return;
    getFormatImpl().mxField->setSpinSize( nValue );
}

sal_Int64 MetricFormatter::GetValue( FieldUnit nUnit ) const
{
    if ( !getFormatImpl().mxField.is() )
        return 0;
    return getFormatImpl().mxField->getValue( MetricUnitVclToUno( nUnit ) );
}

IMPL_CONSTRUCTORS_2( MetricField, SpinField, MetricFormatter, "metricfield" );

class ComboBoxImpl : public EditImpl
                   , public ::cppu::WeakImplHelper1< awt::XActionListener >
                   , public ::cppu::WeakImplHelper1< awt::XItemListener >
{
public:
    uno::Reference< awt::XComboBox > mxComboBox;

    Link maClickHdl;
    Link maSelectHdl;

    Window *parent;

    ComboBoxImpl( Context *context, const PeerHandle &peer, Window *window )
        : EditImpl( context, peer, window )
        , mxComboBox( peer, uno::UNO_QUERY )
    {
    }

    ~ComboBoxImpl ();

    sal_uInt16 InsertEntry( OUString const& rStr, sal_uInt16 nPos )
    {
        if ( nPos == COMBOBOX_APPEND )
            nPos = GetEntryCount();
        mxComboBox->addItem( rtl::OUString( rStr ), nPos );
        return nPos;
    }

    void RemoveEntry( sal_uInt16 nPos )
    {
        mxComboBox->removeItems( nPos, 1 );
    }

    sal_uInt16 GetEntryPos( String const& rStr ) const
    {
        uno::Sequence< rtl::OUString> aItems( mxComboBox->getItems() );
        rtl::OUString rKey( rStr );
        sal_uInt16 n = sal::static_int_cast< sal_uInt16 >(aItems.getLength());
        for (sal_uInt16 i = 0; i < n; i++)
        {
            if ( aItems[ i ] == rKey )
                return i;
        }
        return COMBOBOX_ENTRY_NOTFOUND;
    }

    OUString GetEntry( sal_uInt16 nPos ) const
    {
        return OUString( mxComboBox->getItem( nPos ) );
    }

    sal_uInt16 GetEntryCount() const
    {
        return mxComboBox->getItemCount();
    }

    void SetClickHdl( Link const& link )
    {
        if (!link && !!maClickHdl)
            mxComboBox->removeActionListener( this );
        else if (!!link && !maClickHdl)
            mxComboBox->addActionListener( this );
        maClickHdl = link;
    }

    void SetSelectHdl( Link const& link )
    {
        if (!link && !!maSelectHdl)
            mxComboBox->removeItemListener( this );
        else if (!!link && !maSelectHdl)
            mxComboBox->addItemListener( this );
        maSelectHdl = link;
    }

    void SAL_CALL disposing( lang::EventObject const& e )
        throw (uno::RuntimeException);

    void SAL_CALL actionPerformed (const awt::ActionEvent&)
        throw (uno::RuntimeException)
    {
        ComboBox* pComboBox = static_cast<ComboBox*>( mpWindow );
        if ( !pComboBox )
            return;
        maClickHdl.Call( pComboBox );
    }

    void SAL_CALL itemStateChanged( awt::ItemEvent const&)
        throw (uno::RuntimeException)
    {
        ComboBox* pComboBox = static_cast<ComboBox*>( mpWindow );
        if ( !pComboBox )
            return;
        maSelectHdl.Call( pComboBox );
    }
};

ComboBox::~ComboBox ()
{
#ifndef __SUNPRO_CC
    OSL_TRACE ("%s: deleting ComboBox for window: %p", __FUNCTION__, GetWindow ());
#endif
}

ComboBoxImpl::~ComboBoxImpl ()
{
#ifndef __SUNPRO_CC
    OSL_TRACE ("%s: deleting ComboBoxImpl for window: %p", __FUNCTION__, mpWindow ? mpWindow->GetWindow () : 0);
    OSL_TRACE ("%s: deleting ComboBoxImpl for listener: %p", __FUNCTION__, static_cast<XFocusListener*> (this));
#endif
}

void ComboBoxImpl::disposing( lang::EventObject const& e )
    throw (uno::RuntimeException)
{
    EditImpl::disposing (e);
    mxComboBox.clear ();
}

sal_uInt16 ComboBox::InsertEntry( String const& rStr, sal_uInt16 nPos )
{
    return getImpl().InsertEntry( rStr, nPos );
}

void ComboBox::RemoveEntry( String const& rStr )
{
    getImpl().RemoveEntry( GetEntryPos( rStr ) );
}

void ComboBox::RemoveEntry( sal_uInt16 nPos )
{
    getImpl().RemoveEntry( nPos );
}

void ComboBox::Clear()
{
    uno::Sequence< rtl::OUString> aNoItems;
    getImpl().setProperty( "StringItemList", uno::Any( aNoItems ) );
}

sal_uInt16 ComboBox::GetEntryPos( String const& rStr ) const
{
    return getImpl().GetEntryPos( rStr );
}

String ComboBox::GetEntry( sal_uInt16 nPos ) const
{
    rtl::OUString rItem = getImpl().mxComboBox->getItem( nPos );
    return OUString( rItem );
}

sal_uInt16 ComboBox::GetEntryCount() const
{
    return getImpl().GetEntryCount();
}

void ComboBox::SetClickHdl( const Link& link )
{
    if (&getImpl () && getImpl().mxComboBox.is ())
        getImpl().SetClickHdl( link );
}

void ComboBox::SetSelectHdl( const Link& link )
{
    if (&getImpl () && getImpl().mxComboBox.is ())
        getImpl().SetSelectHdl( link );
}

void ComboBox::EnableAutocomplete (bool enable, bool matchCase)
{
    GetComboBox ()->EnableAutocomplete (enable, matchCase);
}

IMPL_CONSTRUCTORS_BODY( ComboBox, Edit, "combobox", getImpl().parent = parent; );
IMPL_GET_WINDOW (ComboBox);
/// IMPL_GET_IMPL( ComboBox );

static ComboBoxImpl* null_combobox_impl = 0;

ComboBoxImpl &ComboBox::getImpl () const
{
    if (ComboBoxImpl* c = static_cast<ComboBoxImpl *>(mpImpl))
        return *c;
    return *null_combobox_impl;
}

class ListBoxImpl : public ControlImpl
                  , public ::cppu::WeakImplHelper1< awt::XActionListener >
                  , public ::cppu::WeakImplHelper1< awt::XItemListener >
                  , public ::cppu::WeakImplHelper1< awt::XMouseListener >
{
    Link maClickHdl;
    Link maSelectHdl;
    Link maDoubleClickHdl;

public:
    uno::Reference< awt::XListBox > mxListBox;
    ListBoxImpl( Context *context, const PeerHandle &peer, Window *window )
        : ControlImpl( context, peer, window )
        , mxListBox( peer, uno::UNO_QUERY )
    {
        SelectEntryPos (0, true);
    }

    sal_uInt16 InsertEntry (String const& rStr, sal_uInt16 nPos)
    {
        if ( nPos == LISTBOX_APPEND )
            nPos = mxListBox->getItemCount();
        mxListBox->addItem( rtl::OUString( rStr ), nPos );
        return nPos;
    }

    void RemoveEntry( sal_uInt16 nPos )
    {
        mxListBox->removeItems( nPos, 1 );
    }

    sal_uInt16 RemoveEntry( String const& rStr, sal_uInt16 nPos)
    {
        if ( nPos == LISTBOX_APPEND )
            nPos = mxListBox->getItemCount();
        mxListBox->addItem( rtl::OUString( rStr ), nPos );
        return nPos;
    }

    sal_uInt16 GetEntryPos( String const& rStr ) const
    {
        uno::Sequence< rtl::OUString> aItems( mxListBox->getItems() );
        rtl::OUString rKey( rStr );
        sal_uInt16 n = sal::static_int_cast< sal_uInt16 >(aItems.getLength());
        for (sal_uInt16 i = 0; i < n; i++)
        {
            if ( aItems[ i ] == rKey )
                return i;
        }
        return LISTBOX_ENTRY_NOTFOUND;
    }

    OUString GetEntry( sal_uInt16 nPos ) const
    {
        return mxListBox->getItem( nPos );
    }

    sal_uInt16 GetEntryCount() const
    {
        return mxListBox->getItemCount();
    }

    void SelectEntryPos( sal_uInt16 nPos, bool bSelect )
    {
        mxListBox->selectItemPos( nPos, bSelect );
    }

    sal_uInt16 GetSelectEntryCount() const
    {
        return sal::static_int_cast< sal_uInt16 >( mxListBox->getSelectedItems().getLength() );
    }

    sal_uInt16 GetSelectEntryPos( sal_uInt16 nSelIndex ) const
    {
        sal_uInt16 nSelected = 0;
        if ( mxListBox->isMutipleMode() )
        {
            uno::Sequence< short > aItems( mxListBox->getSelectedItemsPos() );
            if ( nSelIndex < aItems.getLength() )
                nSelected = aItems[ nSelIndex ];
        }
        else
            nSelected = mxListBox->getSelectedItemPos();
        return nSelected;
    }

    virtual void SAL_CALL disposing( lang::EventObject const& e )
        throw (uno::RuntimeException)
    {
        ControlImpl::disposing (e);
        mxListBox.clear ();
    }

    Link& GetClickHdl ()
    {
        return maClickHdl;
    }

    void SetClickHdl( Link const& link )
    {
        if (!link && !!maClickHdl)
            mxListBox->removeActionListener( this );
        else if (!!link && !maClickHdl)
            mxListBox->addActionListener( this );
        maClickHdl = link;
    }

    void SAL_CALL actionPerformed( const awt::ActionEvent& /* rEvent */ )
        throw (uno::RuntimeException)
    {
        maClickHdl.Call( mpWindow );
    }

    Link& GetSelectHdl ()
    {
        return maSelectHdl;
    }

    void SetSelectHdl( Link const& link )
    {
        if (!link && !!maSelectHdl)
            mxListBox->removeItemListener( this );
        else if (!!link && !maSelectHdl)
            mxListBox->addItemListener( this );
        maSelectHdl = link;
    }

    void SAL_CALL itemStateChanged (awt::ItemEvent const&)
        throw (uno::RuntimeException)
    {
        maSelectHdl.Call (static_cast <ListBox*> (mpWindow));
    }

    Link& GetDoubleClickHdl ()
    {
        return maDoubleClickHdl;
    }

    void SetDoubleClickHdl (Link const& link)
    {
        if (!link && !!maDoubleClickHdl)
            mxWindow->removeMouseListener (this);
        else if (!!link && !maSelectHdl)
            mxWindow->addMouseListener (this);
        maDoubleClickHdl = link;
    }

    void SAL_CALL mousePressed (awt::MouseEvent const&) throw (uno::RuntimeException)
    {
    }
    void SAL_CALL mouseReleased (awt::MouseEvent const& e) throw (uno::RuntimeException)
    {
        if (e.ClickCount == 2)
            maDoubleClickHdl.Call (mpWindow);
    }
    void SAL_CALL mouseEntered (awt::MouseEvent const&) throw (uno::RuntimeException)
    {
    }
    void SAL_CALL mouseExited (awt::MouseEvent const&) throw (uno::RuntimeException)
    {
    }
};

ListBox::~ListBox ()
{
}

sal_uInt16 ListBox::InsertEntry (String const& rStr, sal_uInt16 nPos)
{
    return getImpl().InsertEntry(rStr, nPos);
}

void ListBox::RemoveEntry( sal_uInt16 nPos )
{
    return getImpl().RemoveEntry( nPos );
}

void ListBox::RemoveEntry( String const& rStr )
{
    return getImpl().RemoveEntry( GetEntryPos( rStr ) );
}

void ListBox::Clear()
{
    uno::Sequence< rtl::OUString> aNoItems;
    getImpl().setProperty( "StringItemList", uno::Any( aNoItems ) );
}

sal_uInt16 ListBox::GetEntryPos( String const& rStr ) const
{
    return getImpl().GetEntryPos( rStr );
}

String ListBox::GetEntry( sal_uInt16 nPos ) const
{
    return getImpl().GetEntry( nPos );
}

sal_uInt16 ListBox::GetEntryCount() const
{
    return getImpl().GetEntryCount();
}

void ListBox::SelectEntryPos( sal_uInt16 nPos, bool bSelect )
{
#if LAYOUT_API_CALLS_HANDLER
    getImpl().SelectEntryPos( nPos, bSelect );
#else /* !LAYOUT_API_CALLS_HANDLER */
    GetListBox ()->SelectEntryPos (nPos, bSelect);
#endif /* !LAYOUT_API_CALLS_HANDLER */
}

void ListBox::SelectEntry( String const& rStr, bool bSelect )
{
    SelectEntryPos( GetEntryPos( rStr ), bSelect );
}

sal_uInt16 ListBox::GetSelectEntryCount() const
{
    return getImpl().GetSelectEntryCount();
}

sal_uInt16 ListBox::GetSelectEntryPos( sal_uInt16 nSelIndex ) const
{
    return getImpl().GetSelectEntryPos( nSelIndex );
}

String ListBox::GetSelectEntry( sal_uInt16 nSelIndex ) const
{
    return GetEntry( GetSelectEntryPos( nSelIndex ) );
}

Link& ListBox::GetSelectHdl ()
{
    return getImpl ().GetSelectHdl ();
}

void ListBox::SetSelectHdl( Link const& link )
{
    getImpl().SetSelectHdl( link );
}

Link& ListBox::GetClickHdl ()
{
    return getImpl ().GetSelectHdl ();
}

void ListBox::SetClickHdl( Link const& link )
{
    if (&getImpl () && getImpl().mxListBox.is ())
        getImpl().SetClickHdl( link );
}

Link& ListBox::GetDoubleClickHdl ()
{
    return getImpl ().GetSelectHdl ();
}

void ListBox::SetDoubleClickHdl( Link const& link )
{
    getImpl().SetDoubleClickHdl( link );
}

void ListBox::SetEntryData( sal_uInt16 pos, void* data)
{
    GetListBox ()->SetEntryData (pos, data);
}

void* ListBox::GetEntryData( sal_uInt16 pos) const
{
    return GetListBox ()->GetEntryData (pos);
}

void ListBox::SetNoSelection ()
{
    GetListBox ()->SetNoSelection ();
}

IMPL_CONSTRUCTORS (ListBox, Control, "listbox");
IMPL_GET_IMPL (ListBox);
IMPL_GET_WINDOW (ListBox);

IMPL_IMPL (MultiListBox, ListBox)
IMPL_CONSTRUCTORS_BODY( MultiListBox, ListBox, "multilistbox", GetMultiListBox()->EnableMultiSelection( true ); );
IMPL_GET_IMPL( MultiListBox );
IMPL_GET_WINDOW( MultiListBox );
} // namespace layout

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
