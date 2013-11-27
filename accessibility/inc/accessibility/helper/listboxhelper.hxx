/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    virtual String          GetEntry( sal_uInt16 nPos ) const
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
    virtual Rectangle       GetBoundingRectangle( sal_uInt16 nItem ) const
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
    virtual sal_Bool            IsActive() const
    {
        return m_aComboListBox.IsActive();
    }
    // -----------------------------------------------------------------------------
    virtual sal_Bool            IsEnabled() const
    {
        return m_aComboListBox.IsEnabled();
    }
    // -----------------------------------------------------------------------------
    virtual sal_Bool            IsEntryVisible( sal_uInt16 nPos ) const
    {
        sal_uInt16 nTopEntry = m_aComboListBox.GetTopEntry();
        sal_uInt16 nLines = m_aComboListBox.GetDisplayLineCount();
        return ( nPos >= nTopEntry && nPos < ( nTopEntry + nLines ) );
    }
    // -----------------------------------------------------------------------------
    virtual sal_uInt16          GetDisplayLineCount() const
    {
        return m_aComboListBox.GetDisplayLineCount();
    }
    // -----------------------------------------------------------------------------
    virtual void            GetMaxVisColumnsAndLines( sal_uInt16& rnCols, sal_uInt16& rnLines ) const
    {
        m_aComboListBox.GetMaxVisColumnsAndLines(rnCols,rnLines);
    }
    // -----------------------------------------------------------------------------
    virtual WinBits         GetStyle() const
    {
        return m_aComboListBox.GetStyle();
    }
    // -----------------------------------------------------------------------------
    virtual sal_Bool            IsMultiSelectionEnabled() const
    {
        return m_aComboListBox.IsMultiSelectionEnabled();
    }
    // -----------------------------------------------------------------------------
    virtual sal_uInt16          GetTopEntry() const
    {
        return m_aComboListBox.GetTopEntry();
    }
    // -----------------------------------------------------------------------------
    virtual sal_Bool            IsEntryPosSelected( sal_uInt16 nPos ) const
    {
        return m_aComboListBox.IsEntryPosSelected(nPos);
    }
    // -----------------------------------------------------------------------------
    virtual sal_uInt16          GetEntryCount() const
    {
        return m_aComboListBox.GetEntryCount();
    }
    // -----------------------------------------------------------------------------
    virtual void    Select()
    {
        m_aComboListBox.Select();
    }
    // -----------------------------------------------------------------------------
    virtual void    SelectEntryPos( sal_uInt16 nPos, sal_Bool bSelect = sal_True )
    {
        m_aComboListBox.SelectEntryPos(nPos,bSelect);
    }
    // -----------------------------------------------------------------------------
    virtual sal_uInt16          GetSelectEntryCount() const
    {
        return m_aComboListBox.GetSelectEntryCount();
    }
    // -----------------------------------------------------------------------------
    virtual void    SetNoSelection()
    {
        m_aComboListBox.SetNoSelection();
    }
    // -----------------------------------------------------------------------------
    virtual sal_uInt16          GetSelectEntryPos( sal_uInt16 nSelIndex = 0 ) const
    {
        return m_aComboListBox.GetSelectEntryPos(nSelIndex);
    }
    // -----------------------------------------------------------------------------
    virtual sal_Bool            IsInDropDown() const
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
    long GetIndexForPoint( const Point& rPoint, sal_uInt16& nPos ) const
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

