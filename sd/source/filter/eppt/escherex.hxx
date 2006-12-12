/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: escherex.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2006-12-12 16:39:54 $
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

#ifndef _PptEscherEX_HXX
#define _PptEscherEX_HXX
#ifndef _SVX_ESCHEREX_HXX
#include <svx/escherex.hxx>
#endif

// ---------------------------------------------------------------------------------------------
// Werte fuer den ULONG im PPT_PST_TextHeaderAtom
enum PPT_TextHeader
{
    PPTTH_TITLE,
    PPTTH_BODY,
    PPTTH_NOTES,
    PPTTH_NOTUSED,
    PPTTH_OTHER,       // Text in a Shape
    PPTTH_CENTERBODY,  // Subtitle in Title-Slide
    PPTTH_CENTERTITLE, // Title in Title-Slide
    PPTTH_HALFBODY,    // Body in two-column slide
    PPTTH_QUARTERBODY  // Body in four-body slide
};

// ---------------------------------------------------------------------------------------------

class PptEscherEx : public EscherEx
{
        SvMemoryStream          maFIDCLs;

        sal_uInt32  ImplDggContainerSize();
        void        ImplWriteDggContainer( SvStream& rSt );

        sal_uInt32  ImplDggAtomSize();
        void        ImplWriteDggAtom( SvStream& rSt );

        sal_uInt32  ImplOptAtomSize();
        void        ImplWriteOptAtom( SvStream& rSt );

        sal_uInt32  ImplSplitMenuColorsAtomSize();
        void        ImplWriteSplitMenuColorsAtom( SvStream& rSt );

    public:

                PptEscherEx( SvStream& rOut, UINT32 nDrawings );
                ~PptEscherEx();

        void    OpenContainer( UINT16 n_EscherContainer, int nRecInstance = 0 );
        void    CloseContainer();

        sal_uInt32 EnterGroup( Rectangle* pBoundRect, SvMemoryStream* pClientData );

        UINT32  DrawingGroupContainerSize();
        void    WriteDrawingGroupContainer( SvStream& rSt );

        using EscherEx::EnterGroup;
};


#endif
