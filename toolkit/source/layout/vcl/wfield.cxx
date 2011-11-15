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

void Edit::SetModifyHdl( const Link& link )
{
    if (&getImpl () && getImpl().mxEdit.is ())
        getImpl().SetModifyHdl( link );
}

IMPL_GET_IMPL( Edit );

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

void ListBox::SetNoSelection ()
{
    GetListBox ()->SetNoSelection ();
}

IMPL_GET_IMPL (ListBox);
IMPL_GET_WINDOW (ListBox);

} // namespace layout

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
