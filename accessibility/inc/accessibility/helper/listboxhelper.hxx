/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: listboxhelper.hxx,v $
 * $Revision: 1.3 $
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

#ifndef ACCESSIBILITY_HELPER_LISTBOXHELPER_HXX
#define ACCESSIBILITY_HELPER_LISTBOXHELPER_HXX

#include <accessibility/helper/IComboListBoxHelper.hxx>
#include <vcl/lstbox.hxx>
#include <vcl/combobox.hxx>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>

// -----------------------------------------------------------------------------
// globals
// -----------------------------------------------------------------------------

const sal_Int32 DEFAULT_INDEX_IN_PARENT = -1;

// -----------------------------------------------------------------------------
// class VCLListBoxHelper
// -----------------------------------------------------------------------------

template< class T > class VCLListBoxHelper : public ::accessibility::IComboListBoxHelper
{
private:
    T&  m_aComboListBox;

public:
    inline
    VCLListBoxHelper( T& _pListBox ) :
        m_aComboListBox( _pListBox ){}

    // -----------------------------------------------------------------------------
    virtual String          GetEntry( USHORT nPos ) const
    {
        return m_aComboListBox.GetEntry( nPos );
    }
    // -----------------------------------------------------------------------------
    virtual Rectangle       GetDropDownPosSizePixel() const
    {
        Rectangle aTemp = m_aComboListBox.GetWindowExtentsRelative(NULL);
        Rectangle aRet = m_aComboListBox.GetDropDownPosSizePixel();
        aRet.Move(aTemp.TopLeft().X(),aTemp.TopLeft().Y());
        return aRet;
    }
    // -----------------------------------------------------------------------------
    virtual Rectangle       GetBoundingRectangle( USHORT nItem ) const
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
    // -----------------------------------------------------------------------------
    virtual Rectangle       GetWindowExtentsRelative( Window* pRelativeWindow )
    {
        return m_aComboListBox.GetWindowExtentsRelative( pRelativeWindow );
    }
    // -----------------------------------------------------------------------------
    virtual BOOL            IsActive() const
    {
        return m_aComboListBox.IsActive();
    }
    // -----------------------------------------------------------------------------
    virtual BOOL            IsEntryVisible( USHORT nPos ) const
    {
        USHORT nTopEntry = m_aComboListBox.GetTopEntry();
        USHORT nLines = m_aComboListBox.GetDisplayLineCount();
        return ( nPos >= nTopEntry && nPos < ( nTopEntry + nLines ) );
    }
    // -----------------------------------------------------------------------------
    virtual USHORT          GetDisplayLineCount() const
    {
        return m_aComboListBox.GetDisplayLineCount();
    }
    // -----------------------------------------------------------------------------
    virtual void            GetMaxVisColumnsAndLines( USHORT& rnCols, USHORT& rnLines ) const
    {
        m_aComboListBox.GetMaxVisColumnsAndLines(rnCols,rnLines);
    }
    // -----------------------------------------------------------------------------
    virtual WinBits         GetStyle() const
    {
        return m_aComboListBox.GetStyle();
    }
    // -----------------------------------------------------------------------------
    virtual BOOL            IsMultiSelectionEnabled() const
    {
        return m_aComboListBox.IsMultiSelectionEnabled();
    }
    // -----------------------------------------------------------------------------
    virtual USHORT          GetTopEntry() const
    {
        return m_aComboListBox.GetTopEntry();
    }
    // -----------------------------------------------------------------------------
    virtual BOOL            IsEntryPosSelected( USHORT nPos ) const
    {
        return m_aComboListBox.IsEntryPosSelected(nPos);
    }
    // -----------------------------------------------------------------------------
    virtual USHORT          GetEntryCount() const
    {
        return m_aComboListBox.GetEntryCount();
    }
    // -----------------------------------------------------------------------------
    virtual void    Select()
    {
        m_aComboListBox.Select();
    }
    // -----------------------------------------------------------------------------
    virtual void    SelectEntryPos( USHORT nPos, BOOL bSelect = TRUE )
    {
        m_aComboListBox.SelectEntryPos(nPos,bSelect);
    }
    // -----------------------------------------------------------------------------
    virtual USHORT          GetSelectEntryCount() const
    {
        return m_aComboListBox.GetSelectEntryCount();
    }
    // -----------------------------------------------------------------------------
    virtual void    SetNoSelection()
    {
        m_aComboListBox.SetNoSelection();
    }
    // -----------------------------------------------------------------------------
    virtual USHORT          GetSelectEntryPos( USHORT nSelIndex = 0 ) const
    {
        return m_aComboListBox.GetSelectEntryPos(nSelIndex);
    }
    // -----------------------------------------------------------------------------
    virtual BOOL            IsInDropDown() const
    {
        return m_aComboListBox.IsInDropDown();
    }
    // -----------------------------------------------------------------------------
    virtual Rectangle GetEntryCharacterBounds( const sal_Int32 _nEntryPos, const sal_Int32 _nCharacterIndex ) const
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
    // -----------------------------------------------------------------------------
    long GetIndexForPoint( const Point& rPoint, USHORT& nPos ) const
    {
        return m_aComboListBox.GetIndexForPoint( rPoint, nPos );
    }
    // -----------------------------------------------------------------------------
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >
        GetClipboard()
    {
        return m_aComboListBox.GetClipboard();
    }
    // -----------------------------------------------------------------------------
};

#endif  // ACCESSIBILITY_HELPER_LISTBOXHELPER_HXX

