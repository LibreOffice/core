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

#pragma once
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

        static sal_uInt32  ImplOptAtomSize();
        static void        ImplWriteOptAtom( SvStream& rSt );

        static sal_uInt32  ImplSplitMenuColorsAtomSize();
        static void        ImplWriteSplitMenuColorsAtom( SvStream& rSt );

    public:

                PptEscherEx( SvStream& rOut, const OUString& );
                virtual ~PptEscherEx() override;

        void    OpenContainer( sal_uInt16 n_EscherContainer, int nRecInstance = 0 ) override;
        void    CloseContainer() override;

        sal_uInt32 EnterGroup( ::tools::Rectangle const * pBoundRect, SvMemoryStream* pClientData );

        sal_uInt32  DrawingGroupContainerSize();
        void    WriteDrawingGroupContainer( SvStream& rSt );

        using EscherEx::EnterGroup;
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
