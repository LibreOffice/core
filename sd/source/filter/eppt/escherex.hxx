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

#ifndef _PptEscherEX_HXX
#define _PptEscherEX_HXX
#include <filter/msfilter/escherex.hxx>

/// Values for the sal_uLong in PPT_PST_TextHeaderAtom.
enum PPT_TextHeader
{
    PPTTH_TITLE,
    PPTTH_BODY,
    PPTTH_NOTES,
    PPTTH_NOTUSED,
    PPTTH_OTHER,       ///< Text in a Shape
    PPTTH_CENTERBODY,  ///< Subtitle in Title-Slide
    PPTTH_CENTERTITLE, ///< Title in Title-Slide
    PPTTH_HALFBODY,    ///< Body in two-column slide
    PPTTH_QUARTERBODY  ///< Body in four-body slide
};

class PptEscherEx : public EscherEx
{
        sal_uInt32  ImplDggContainerSize();
        void        ImplWriteDggContainer( SvStream& rSt );

        sal_uInt32  ImplOptAtomSize();
        void        ImplWriteOptAtom( SvStream& rSt );

        sal_uInt32  ImplSplitMenuColorsAtomSize();
        void        ImplWriteSplitMenuColorsAtom( SvStream& rSt );

    public:

                PptEscherEx( SvStream& rOut, const rtl::OUString& );
                ~PptEscherEx();

        void    OpenContainer( sal_uInt16 n_EscherContainer, int nRecInstance = 0 );
        void    CloseContainer();

        sal_uInt32 EnterGroup( Rectangle* pBoundRect, SvMemoryStream* pClientData );

        sal_uInt32  DrawingGroupContainerSize();
        void    WriteDrawingGroupContainer( SvStream& rSt );

        using EscherEx::EnterGroup;
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
