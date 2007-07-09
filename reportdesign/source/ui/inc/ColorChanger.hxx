#ifndef RPTUI_COLORCHANGER_HXX
#define RPTUI_COLORCHANGER_HXX
/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: ColorChanger.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: rt $ $Date: 2007-07-09 11:56:30 $
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

namespace rptui
{
    //=====================================================================
    //= ColorChanger
    //=====================================================================
    class ColorChanger
    {
    protected:
        OutputDevice*   m_pDev;

    public:
        ColorChanger( OutputDevice* _pDev, const Color& _rNewLineColor)
            :m_pDev( _pDev )
        {
            m_pDev->Push( PUSH_LINECOLOR );
            m_pDev->SetLineColor( _rNewLineColor );
        }
        ColorChanger( OutputDevice* _pDev, const Color& _rNewLineColor, const Color& _rNewFillColor )
            :m_pDev( _pDev )
        {
            m_pDev->Push( PUSH_LINECOLOR | PUSH_FILLCOLOR );
            m_pDev->SetLineColor( _rNewLineColor );
            m_pDev->SetFillColor( _rNewFillColor );
        }
        ColorChanger( OutputDevice* _pDev, const Color& _rNewLineColor, const Color& _rNewFillColor, const Color& _rNewTextColor )
            :m_pDev( _pDev )
        {
            m_pDev->Push( PUSH_LINECOLOR | PUSH_FILLCOLOR | PUSH_TEXTCOLOR);
            m_pDev->SetLineColor( _rNewLineColor );
            m_pDev->SetFillColor( _rNewFillColor );
            m_pDev->SetTextColor( _rNewTextColor );
        }

        ~ColorChanger()
        {
            m_pDev->Pop();
        }
    };
}
#endif //RPTUI_COLORCHANGER_HXX

