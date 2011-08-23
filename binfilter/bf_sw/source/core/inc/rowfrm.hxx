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
#ifndef _ROWFRM_HXX
#define _ROWFRM_HXX

#include <tools/mempool.hxx>

#include "layfrm.hxx"
namespace binfilter {

class SwTableLine;
class SwBorderAttrs;

class SwRowFrm: public SwLayoutFrm
{
    virtual void Format( const SwBorderAttrs *pAttrs = 0 );
        //Aendern nur die Framesize, nicht die PrtArea-SSize
    virtual SwTwips ShrinkFrm( SwTwips, SZPTR
                               BOOL bTst = FALSE, BOOL bInfo = FALSE );
    virtual SwTwips GrowFrm  ( SwTwips, SZPTR
                               BOOL bTst = FALSE, BOOL bInfo = FALSE );

    const SwTableLine *pTabLine;

protected:
    virtual void MakeAll();

public:
    SwRowFrm( const SwTableLine & );
    ~SwRowFrm();

    virtual	void  Modify( SfxPoolItem*, SfxPoolItem* );

    //Zum Anmelden der Flys nachdem eine Zeile erzeugt _und_ eingefuegt wurde.
    //Muss vom Erzeuger gerufen werden, denn erst nach dem Konstruieren wird
    //Das Teil gepastet; mithin ist auch erst dann die Seite zum Anmelden der
    //Flys erreichbar.
    void RegistFlys( SwPageFrm *pPage = 0 );

    const SwTableLine *GetTabLine() const { return pTabLine; }

    //Passt die Zellen auf die aktuelle Hoehe an, invalidiert die Zellen
    //wenn die Direction nicht der Hoehe entspricht.
    void AdjustCells( const SwTwips nHeight, const BOOL bHeight );
    DECL_FIXEDMEMPOOL_NEWDEL(SwRowFrm)
};

} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
