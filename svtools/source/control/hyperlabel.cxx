/*************************************************************************
 *
 *  $RCSfile: hyperlabel.cxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: kz $ $Date: 2004-05-19 14:00:46 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESS OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc..
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#ifndef SVTOOLS_ROADMAP_HXX
#include "hyperlabel.hxx"
#endif

#ifndef _SV_BITMAP_HXX
#include <vcl/bitmap.hxx>
#endif

#ifndef _TOOLS_COLOR_HXX
#include <tools/color.hxx>
#endif

#ifndef _VCL_TABPAGE_HXX
#include <vcl/tabpage.hxx>
#endif


//.........................................................................
namespace svt
{
//.........................................................................

    //=====================================================================
    //= FontChanger
    //=====================================================================
    class FontChanger
    {
    protected:
        OutputDevice*   m_pDev;

    public:
        FontChanger( OutputDevice* _pDev, const Font& _rNewFont )
            :m_pDev( _pDev )
        {
            m_pDev->Push( PUSH_FONT );
            m_pDev->SetFont( _rNewFont );
        }

        ~FontChanger()
        {
            m_pDev->Pop( );
        }
    };

    class HyperLabelImpl
    {
    public:
        sal_Int16           ID;
        sal_Int32           Index;
        sal_Bool            bInteractive;
        Size                m_aMinSize;
        sal_Bool            m_bHyperMode;

        HyperLabelImpl();
    };

    //---------------------------------------------------------------------
    HyperLabelImpl::HyperLabelImpl()
    {
    }

    HyperLabel::HyperLabel( Window* _pParent, const ResId& _rId )
        :FixedText( _pParent, _rId )
        ,m_pImpl( NULL )
    {
        implInit(_pParent);
    }

    HyperLabel::HyperLabel( Window* _pParent, WinBits _nWinStyle )
        :FixedText( _pParent, _nWinStyle )
        ,m_pImpl( NULL )
    {
        implInit(_pParent);
    }

    void HyperLabel::SetLabelAndSize( ::rtl::OUString _rText, const Size& _rNewSize) //, sal_Bool _bInvalidate )
    {
        Size rLocSize = _rNewSize;
        Size rLogicLocSize = PixelToLogic( _rNewSize, MAP_APPFONT );
        SetLabel( _rText );
        if ( ImplCalcMinimumSize( rLocSize ) == sal_True )
            rLocSize.Height() = ( m_pImpl->m_aMinSize.Height() * 2 );
        else
            rLocSize = LogicToPixel( Size( rLogicLocSize.Width(), LABELBASEMAPHEIGHT ), MAP_APPFONT ) ;
        SetSizePixel( rLocSize );
        Show();
    }

    sal_Bool HyperLabel::ImplCalcMinimumSize(const Size& _rCompSize )
    {
        sal_Bool b_AdjustMinWidth = sal_False;
        m_pImpl->m_aMinSize = CalcMinimumSize( );
        if ( m_pImpl->m_aMinSize.Width() >= _rCompSize.Width() )    // the MinimumSize is used to size the FocusRectangle
        {
            m_pImpl->m_aMinSize.Width() = _rCompSize.Width();       // and for the MouseMove method
            b_AdjustMinWidth = sal_True;
        }
        return b_AdjustMinWidth;
    }


    void HyperLabel::implInit(Window* _pParent)
    {
        m_pImpl = new HyperLabelImpl;
        SetTextColor( Color( 89, 79, 191 ) );
        ToggleBackgroundColor( COL_WHITE );
        Show();

    }


    void HyperLabel::MouseMove( const MouseEvent& rMEvt )
    {
           Font aFont = GetControlFont( );
        const Color aColor = GetTextColor();

        if (rMEvt.IsLeaveWindow())
        {
            DeactivateHyperMode(aFont, aColor);
        }
        else
        {
            Point aPoint = GetPointerPosPixel();
            if (aPoint.X() < m_pImpl->m_aMinSize.Width())
            {
                if ( IsEnabled() && (m_pImpl->bInteractive) )
                {
                    ActivateHyperMode( aFont, aColor);
                    return;
                }
            }
            DeactivateHyperMode(aFont, aColor);
        }
    }

    void HyperLabel::ActivateHyperMode(Font aFont, const Color aColor)
    {
        aFont.SetUnderline(UNDERLINE_SINGLE);
        m_pImpl->m_bHyperMode = sal_True;
        SetPointer( POINTER_REFHAND );
        SetControlFont( aFont);
        SetTextColor( aColor);

    }

    void HyperLabel::DeactivateHyperMode(Font aFont, const Color aColor)
    {
        m_pImpl->m_bHyperMode = sal_False;
        aFont.SetUnderline(UNDERLINE_NONE);
        SetPointer( POINTER_ARROW );
        SetControlFont( aFont);
        SetTextColor( aColor);
    }

    void HyperLabel::MouseButtonDown( const MouseEvent& rMEvt )
    {
        if ( m_pImpl->m_bHyperMode && m_pImpl->bInteractive )
        {
            maClickHdl.Call( this );
        }
    }

    void HyperLabel::GetFocus()
    {
        if ( IsEnabled() && m_pImpl->bInteractive )
        {
            Point aPoint(0,0);
            Rectangle rRect(aPoint, Size( m_pImpl->m_aMinSize.Width(), GetSizePixel().Height() ) );
            ShowFocus( rRect );
        }
    }

    void HyperLabel::LoseFocus()
    {
        HideFocus();
    }

    HyperLabel::~HyperLabel( )
    {
        delete m_pImpl;
    }

    void HyperLabel::SetInteractive( sal_Bool _bInteractive )
    {
        m_pImpl->bInteractive = ( _bInteractive && IsEnabled() );
    }

    void HyperLabel::SetHyperLabelPosition(sal_uInt16 XPos, sal_uInt16 YPos)
    {
        SetPosPixel( LogicToPixel( Point( XPos, YPos ), MAP_APPFONT ) );
    }

    Point HyperLabel::GetHyperLabelPosition()
    {
        Point aPoint = GetPosPixel( );
        return PixelToLogic( aPoint, MAP_APPFONT );
    }

    sal_Int16 HyperLabel::GetID() const
    {
        return m_pImpl->ID;
    }

    sal_Int32 HyperLabel::GetIndex() const
    {
        return m_pImpl->Index;
    }

    void HyperLabel::SetID( sal_Int16 _ID )
    {
        m_pImpl->ID = _ID;
    }

    void HyperLabel::SetIndex( sal_Int32 _Index )
    {
        m_pImpl->Index = _Index;
    }

    ::rtl::OUString HyperLabel::GetLabel( )
    {
        return GetText();
    }

    void HyperLabel::SetLabel( ::rtl::OUString _rText )
    {
        SetText(_rText);
        Show();
    }






//.........................................................................
}   // namespace svt
//.........................................................................

