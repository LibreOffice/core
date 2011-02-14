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
// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_extensions.hxx"
#include "inspectorhelpwindow.hxx"
#include "modulepcr.hxx"
#ifndef EXTENSIONS_PROPRESID_HRC
#include "propresid.hrc"
#endif

/** === begin UNO includes === **/
/** === end UNO includes === **/

//........................................................................
namespace pcr
{
//........................................................................

    /** === begin UNO using === **/
    /** === end UNO using === **/

    //====================================================================
    //= InspectorHelpWindow
    //====================================================================
    //--------------------------------------------------------------------
    InspectorHelpWindow::InspectorHelpWindow( Window* _pParent )
        :Window( _pParent, WB_DIALOGCONTROL )
        ,m_aSeparator( this )
        ,m_aHelpText( this, WB_LEFT | WB_READONLY | WB_AUTOVSCROLL )
        ,m_nMinLines( 3 )
        ,m_nMaxLines( 8 )
    {
        SetBackground();
        SetPaintTransparent(sal_True);
        m_aSeparator.SetText( String( PcrRes( RID_STR_HELP_SECTION_LABEL ) ) );
        m_aSeparator.SetBackground();
        m_aSeparator.Show();

        m_aHelpText.SetControlBackground( /*m_aSeparator.GetBackground().GetColor() */);
        m_aHelpText.SetBackground();
        m_aHelpText.SetPaintTransparent(sal_True);
        m_aHelpText.Show();
    }

    //--------------------------------------------------------------------
    void InspectorHelpWindow::SetText( const XubString& _rStr )
    {
        m_aHelpText.SetText( _rStr );
    }

    //--------------------------------------------------------------------
    void InspectorHelpWindow::SetLimits( sal_Int32 _nMinLines, sal_Int32 _nMaxLines )
    {
        m_nMinLines = _nMinLines;
        m_nMaxLines = _nMaxLines;
    }

    //--------------------------------------------------------------------
    long InspectorHelpWindow::impl_getHelpTextBorderHeight()
    {
        sal_Int32 nTop(0), nBottom(0), nDummy(0);
        m_aHelpText.GetBorder( nDummy, nTop, nDummy, nBottom );
        return nTop + nBottom;
    }

    //--------------------------------------------------------------------
    long InspectorHelpWindow::impl_getSpaceAboveTextWindow()
    {
        Size aSeparatorSize( LogicToPixel( Size( 0, 8 ), MAP_APPFONT ) );
        Size a3AppFontSize( LogicToPixel( Size( 3, 3 ), MAP_APPFONT ) );
        return aSeparatorSize.Height() + a3AppFontSize.Height();
    }

    //--------------------------------------------------------------------
    long InspectorHelpWindow::GetMinimalHeightPixel()
    {
        return impl_getMinimalTextWindowHeight() + impl_getSpaceAboveTextWindow();
    }

    //--------------------------------------------------------------------
    long InspectorHelpWindow::impl_getMinimalTextWindowHeight()
    {
        return impl_getHelpTextBorderHeight() + m_aHelpText.GetTextHeight() * m_nMinLines;
    }

    //--------------------------------------------------------------------
    long InspectorHelpWindow::impl_getMaximalTextWindowHeight()
    {
        return impl_getHelpTextBorderHeight() + m_aHelpText.GetTextHeight() * m_nMaxLines;
    }

    //--------------------------------------------------------------------
    long InspectorHelpWindow::GetOptimalHeightPixel()
    {
        // --- calc the height as needed for the mere text window
        long nMinTextWindowHeight = impl_getMinimalTextWindowHeight();
        long nMaxTextWindowHeight = impl_getMaximalTextWindowHeight();

        Rectangle aTextRect( Point( 0, 0 ), m_aHelpText.GetOutputSizePixel() );
        aTextRect = m_aHelpText.GetTextRect( aTextRect, m_aHelpText.GetText(),
            TEXT_DRAW_LEFT | TEXT_DRAW_TOP | TEXT_DRAW_MULTILINE | TEXT_DRAW_WORDBREAK );
        long nActTextWindowHeight = impl_getHelpTextBorderHeight() + aTextRect.GetHeight();

        long nOptTextWindowHeight = ::std::max( nMinTextWindowHeight, ::std::min( nMaxTextWindowHeight, nActTextWindowHeight ) );

        // --- then add the space above the text window
        return nOptTextWindowHeight + impl_getSpaceAboveTextWindow();
    }

    //--------------------------------------------------------------------
    void InspectorHelpWindow::Resize()
    {
        Size a3AppFont( LogicToPixel( Size( 3, 3 ), MAP_APPFONT ) );

        Rectangle aPlayground( Point( 0, 0 ), GetOutputSizePixel() );

        Rectangle aSeparatorArea( aPlayground );
        aSeparatorArea.Bottom() = aSeparatorArea.Top() + LogicToPixel( Size( 0, 8 ), MAP_APPFONT ).Height();
        m_aSeparator.SetPosSizePixel( aSeparatorArea.TopLeft(), aSeparatorArea.GetSize() );

        Rectangle aTextArea( aPlayground );
        aTextArea.Top() = aSeparatorArea.Bottom() + a3AppFont.Height();
        m_aHelpText.SetPosSizePixel( aTextArea.TopLeft(), aTextArea.GetSize() );
    }

//........................................................................
} // namespace pcr
//........................................................................

