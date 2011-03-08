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
#ifndef SVTOOLS_HYPERLABEL_HXX
#define SVTOOLS_HYPERLABEL_HXX

#include <vcl/event.hxx>


#include <vcl/fixed.hxx>

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

        void                SetLabel( const ::rtl::OUString& _rText );
        sal_Int32           GetLogicWidth();

        ::rtl::OUString     GetLabel( );

        void                ToggleBackgroundColor( const Color& _rGBColor );
        void                SetInteractive( sal_Bool _bInteractive );

        void                SetClickHdl( const Link& rLink ) { maClickHdl = rLink; }
        const Link&         GetClickHdl() const { return maClickHdl; }

        Size                CalcMinimumSize( long nMaxWidth = 0 ) const;

    private:

        DECL_LINK(ImplClickHdl, HyperLabel*);

    private:
        using FixedText::CalcMinimumSize;
    };
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
