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
#include "inspectorhelpwindow.hxx"
#include "modulepcr.hxx"
#include "propresid.hrc"

//........................................................................
namespace pcr
{
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
        m_aSeparator.SetText( PcrRes(RID_STR_HELP_SECTION_LABEL).toString() );
        m_aSeparator.SetBackground();
        m_aSeparator.Show();

        m_aHelpText.SetControlBackground( /*m_aSeparator.GetBackground().GetColor() */);
        m_aHelpText.SetBackground();
        m_aHelpText.SetPaintTransparent(sal_True);
        m_aHelpText.Show();
    }

    //--------------------------------------------------------------------
    void InspectorHelpWindow::SetText( const OUString& _rStr )
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

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
