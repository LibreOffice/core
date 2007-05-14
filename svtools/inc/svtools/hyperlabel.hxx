/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: hyperlabel.hxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: kz $ $Date: 2007-05-14 08:16:42 $
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
#ifndef SVTOOLS_HYPERLABEL_HXX
#define SVTOOLS_HYPERLABEL_HXX

#ifndef _SV_EVENT_HXX
#include <vcl/event.hxx>
#endif


#ifndef _SV_FIXED_HXX
#include <vcl/fixed.hxx>
#endif

#define LABELBASEMAPHEIGHT      8


//.........................................................................
namespace svt
{
//.........................................................................

       class HyperLabelImpl;

    class HyperLabel : public FixedText
    {
    protected:
        HyperLabelImpl*     m_pImpl;
        Link                maClickHdl;

        virtual void        MouseMove( const MouseEvent& rMEvt );
        virtual void        MouseButtonDown( const MouseEvent& rMEvt );
        virtual void        GetFocus();
        virtual void        LoseFocus();

        void                DeactivateHyperMode(Font aFont, const Color aColor);
        void                ActivateHyperMode(Font aFont, const Color aColor);

    protected:
        void                implInit();

    public:
        HyperLabel( Window* _pParent, const ResId& _rId );
        HyperLabel( Window* _pParent, WinBits _nWinStyle = 0 );
        ~HyperLabel( );

        virtual void    DataChanged( const DataChangedEvent& rDCEvt );

        void                SetID( sal_Int16 _ID );
        sal_Int16           GetID() const;

        void                SetIndex( sal_Int32 _Index );
        sal_Int32           GetIndex() const;

        void                SetLabelAndSize( ::rtl::OUString _rText, const Size& rNewSize);
        void                SetLabel( ::rtl::OUString _rText );
        sal_Int32           GetLogicWidth();

        ::rtl::OUString     GetLabel( );

        void                SetHyperLabelPosition(sal_uInt16 XPos, sal_uInt16 YPos);
        Point               GetLogicalPosition();

        void                ToggleBackgroundColor( const Color& _rGBColor );
        void                SetInteractive( sal_Bool _bInteractive );

        void                SetClickHdl( const Link& rLink ) { maClickHdl = rLink; }
        const Link&         GetClickHdl() const { return maClickHdl; }

    private:

        DECL_LINK(ImplClickHdl, HyperLabel*);

        sal_Bool            ImplCalcMinimumSize(const Size& _rCompSize );
    };
}

#endif

