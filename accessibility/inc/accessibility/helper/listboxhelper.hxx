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

#ifndef INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_HELPER_LISTBOXHELPER_HXX
#define INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_HELPER_LISTBOXHELPER_HXX

#include <accessibility/helper/IComboListBoxHelper.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>


// globals


const sal_Int32 DEFAULT_INDEX_IN_PARENT = -1;


// class VCLListBoxHelper


template< class T > class VCLListBoxHelper : public ::accessibility::IComboListBoxHelper
{
private:
    T&  m_aComboListBox;

public:
    inline
    VCLListBoxHelper( T& _pListBox ) :
        m_aComboListBox( _pListBox ){}


    virtual OUString        GetEntry( sal_Int32  nPos ) const override
    {
        return m_aComboListBox.GetEntry( nPos );
    }

    virtual Rectangle       GetDropDownPosSizePixel() const override
    {
        Rectangle aTemp = m_aComboListBox.GetWindowExtentsRelative(NULL);
        Rectangle aRet = m_aComboListBox.GetDropDownPosSizePixel();
        aRet.Move(aTemp.TopLeft().X(),aTemp.TopLeft().Y());
        return aRet;
    }

    virtual Rectangle       GetBoundingRectangle( sal_uInt16 nItem ) const override
    {
        Rectangle aRect;
        if ( m_aComboListBox.IsInDropDown() && IsEntryVisible( nItem ) )
        {
            Rectangle aTemp = m_aComboListBox.GetDropDownPosSizePixel();
            Size aSize = aTemp.GetSize();
            aSize.Height() /= m_aComboListBox.GetDisplayLineCount();
            Point aTopLeft = aTemp.TopLeft();
            aTopLeft.Y() += aSize.Height() * ( nItem - m_aComboListBox.GetTopEntry() );
            aRect = Rectangle( aTopLeft, aSize );
        }
        else
            aRect = m_aComboListBox.GetBoundingRectangle( nItem );
        return aRect;
    }

    virtual Rectangle       GetWindowExtentsRelative( vcl::Window* pRelativeWindow ) override
    {
        return m_aComboListBox.GetWindowExtentsRelative( pRelativeWindow );
    }

    virtual bool            IsEnabled() const override
    {
        return m_aComboListBox.IsEnabled();
    }

    virtual bool            IsEntryVisible( sal_Int32  nPos ) const override
    {
        sal_Int32  nTopEntry = m_aComboListBox.GetTopEntry();
        sal_uInt16 nLines = m_aComboListBox.GetDisplayLineCount();
        return ( nPos >= nTopEntry && nPos < ( nTopEntry + nLines ) );
    }

    virtual sal_uInt16      GetDisplayLineCount() const override
    {
        return m_aComboListBox.GetDisplayLineCount();
    }

    virtual void            GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const override
    {
        m_aComboListBox.GetMaxVisColumnsAndLines(rnCols,rnLines);
    }

    virtual WinBits         GetStyle() const override
    {
        return m_aComboListBox.GetStyle();
    }

    virtual bool            IsMultiSelectionEnabled() const override
    {
        return m_aComboListBox.IsMultiSelectionEnabled();
    }

    virtual sal_Int32       GetTopEntry() const override
    {
        return m_aComboListBox.GetTopEntry();
    }

    virtual bool            IsEntryPosSelected( sal_Int32  nPos ) const override
    {
        return m_aComboListBox.IsEntryPosSelected(nPos);
    }

    virtual sal_Int32       GetEntryCount() const override
    {
        return m_aComboListBox.GetEntryCount();
    }

    virtual void            Select() override
    {
        m_aComboListBox.Select();
    }

    virtual void            SelectEntryPos( sal_Int32  nPos, bool bSelect = true ) override
    {
        m_aComboListBox.SelectEntryPos(nPos,bSelect);
    }

    virtual sal_Int32       GetSelectEntryCount() const override
    {
        return m_aComboListBox.GetSelectEntryCount();
    }

    virtual void    SetNoSelection() override
    {
        m_aComboListBox.SetNoSelection();
    }

    virtual sal_Int32       GetSelectEntryPos( sal_Int32  nSelIndex = 0 ) const override
    {
        return m_aComboListBox.GetSelectEntryPos(nSelIndex);
    }

    virtual bool            IsInDropDown() const override
    {
        return m_aComboListBox.IsInDropDown();
    }

    virtual Rectangle GetEntryCharacterBounds( const sal_Int32 _nEntryPos, const sal_Int32 _nCharacterIndex ) const override
    {
        Rectangle aRect;

        Pair aEntryCharacterRange = m_aComboListBox.GetLineStartEnd( _nEntryPos );
        if ( aEntryCharacterRange.A() + _nCharacterIndex <= aEntryCharacterRange.B() )
        {
            long nIndex = aEntryCharacterRange.A() + _nCharacterIndex;
            aRect = m_aComboListBox.GetCharacterBounds( nIndex );
        }
        return aRect;
    }

    long GetIndexForPoint( const Point& rPoint, sal_Int32& nPos ) const override
    {
        return m_aComboListBox.GetIndexForPoint( rPoint, nPos );
    }

    css::uno::Reference< css::datatransfer::clipboard::XClipboard >
        GetClipboard() override
    {
        return m_aComboListBox.GetClipboard();
    }

};

#endif // INCLUDED_ACCESSIBILITY_INC_ACCESSIBILITY_HELPER_LISTBOXHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
