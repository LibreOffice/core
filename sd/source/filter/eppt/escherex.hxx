/*************************************************************************
 *
 *  $RCSfile: escherex.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: rt $ $Date: 2004-11-26 19:49:57 $
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
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
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

        sal_uInt32 EnterGroup( Rectangle* pBoundRect = NULL, SvMemoryStream* pClientData = NULL );

        UINT32  DrawingGroupContainerSize();
        void    WriteDrawingGroupContainer( SvStream& rSt );
};


#endif
