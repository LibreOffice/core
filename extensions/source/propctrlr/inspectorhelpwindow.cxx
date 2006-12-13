/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: inspectorhelpwindow.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-13 12:00:06 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
 *
 ************************************************************************/
#ifndef INSPECTORHELPWINDOW_HXX
#include "inspectorhelpwindow.hxx"
#endif
#ifndef EXTENSIONS_PROPCTRLR_MODULEPRC_HXX
#include "modulepcr.hxx"
#endif
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
        m_aSeparator.SetText( String( PcrRes( RID_STR_HELP_SECTION_LABEL ) ) );
        m_aSeparator.Show();

        m_aHelpText.SetControlBackground( m_aSeparator.GetBackground().GetColor() );
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
    long InspectorHelpWindow::GetMaximalHeightPixel()
    {
        return impl_getMaximalTextWindowHeight() + impl_getSpaceAboveTextWindow();
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

