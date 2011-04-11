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
#ifndef _VIRTOUTP_HXX
#define _VIRTOUTP_HXX

#include <vcl/virdev.hxx>

#include "swtypes.hxx"      // UCHAR
#include "swrect.hxx"       // SwRect

class ViewShell;
#define VIRTUALHEIGHT 64

/*************************************************************************
 *                      class SwTxtVout
 *************************************************************************/

class SwLayVout
{
    friend void _FrmFinit();    //loescht das Vout
private:
    ViewShell*      pSh;
    OutputDevice*   pOut;
    VirtualDevice*  pVirDev;
    SwRect          aRect;
    SwRect          aOrgRect;
    Size            aSize;
    sal_uInt16          nCount;

    sal_Bool DoesFit( const Size &rOut );

public:
    SwLayVout() : pSh(0), pOut(0), pVirDev(0), aSize(0, VIRTUALHEIGHT), nCount(0) {}
    ~SwLayVout() { delete pVirDev; }

    /// OD 27.09.2002 #103636# - change 2nd parameter <rRect> - no longer <const>
    void Enter( ViewShell *pShell, SwRect &rRect, sal_Bool bOn );
    void Leave() { --nCount; Flush(); }

    void SetOrgRect( SwRect &rRect ) { aOrgRect = rRect; }
    const SwRect& GetOrgRect() const { return aOrgRect; }

    sal_Bool IsFlushable() { return 0 != pOut; }
    void _Flush();
    void Flush() { if( pOut ) _Flush(); }
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
