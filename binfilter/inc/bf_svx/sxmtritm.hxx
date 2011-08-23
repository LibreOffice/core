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
#ifndef _SXMTRITM_HXX
#define _SXMTRITM_HXX

#ifndef _SVDDEF_HXX //autogen
#include <bf_svx/svddef.hxx>
#endif

#ifndef _SDYNITM_HXX
#include <bf_svx/sdynitm.hxx>
#endif
namespace binfilter {

// Den Text quer zur Masslinie (90deg Drehung nach links)
class SdrMeasureTextRota90Item: public SdrYesNoItem {
public:
    SdrMeasureTextRota90Item(FASTBOOL bOn=FALSE): SdrYesNoItem(SDRATTR_MEASURETEXTROTA90,bOn) {}
    SdrMeasureTextRota90Item(SvStream& rIn): SdrYesNoItem(SDRATTR_MEASURETEXTROTA90,rIn) {}
};

// Das von mir berechnete TextRect um 180 deg drehen
// Der Text wird dann aber auch auf die andere Seite der
// Masslinie gebracht (wenn nicht Rota90)
class SdrMeasureTextUpsideDownItem: public SdrYesNoItem {
public:
    SdrMeasureTextUpsideDownItem(FASTBOOL bOn=FALSE): SdrYesNoItem(SDRATTR_MEASURETEXTUPSIDEDOWN,bOn) {}
    SdrMeasureTextUpsideDownItem(SvStream& rIn): SdrYesNoItem(SDRATTR_MEASURETEXTUPSIDEDOWN,rIn) {}
};

}//end of namespace binfilter
#endif
