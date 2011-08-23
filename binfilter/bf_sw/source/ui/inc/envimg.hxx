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
#ifndef _ENVIMG_HXX
#define _ENVIMG_HXX

#include <bf_svtools/poolitem.hxx>
namespace binfilter {

// enum -----------------------------------------------------------------------

enum SwEnvAlign
{
    ENV_HOR_LEFT = 0,
    ENV_HOR_CNTR,
    ENV_HOR_RGHT,
    ENV_VER_LEFT,
    ENV_VER_CNTR,
    ENV_VER_RGHT
};

// class SwEnvItem ------------------------------------------------------------

class SwEnvItem : public SfxPoolItem
{
public:

    ::rtl::OUString   aAddrText;       // Text fuer Empfaenger
    sal_Bool      	bSend;           // Absender?
    ::rtl::OUString  	aSendText;       // Text fuer Absender
    sal_Int32       lAddrFromLeft;   // Linker Abstand fuer Empfaenger (twips)
    sal_Int32       lAddrFromTop;    // Oberer Abstand fuer Empfaenger (twips)
    sal_Int32       lSendFromLeft;   // Linker Abstand fuer Absender   (twips)
    sal_Int32       lSendFromTop;    // Oberer Abstand fuer Absender   (twips)
    sal_Int32       lWidth;          // Breite des Umschlags           (twips)
    sal_Int32       lHeight;         // Hoehe  des Umschlags           (twips)
    SwEnvAlign 		eAlign;          // Ausrichtung beim Einzug
    sal_Bool  		bPrintFromAbove; // Von oben drucken?
    sal_Int32       lShiftRight;     // Verschiebung nach rechts       (twips)
    sal_Int32       lShiftDown;      // Verschiebung nach unten        (twips)

    SwEnvItem();
    SwEnvItem(const SwEnvItem& rItem);

    TYPEINFO();


    virtual int operator ==(const SfxPoolItem& rItem) const;

    virtual SfxPoolItem*     Clone(SfxItemPool* = 0) const;
};

// class SwEnvCfgItem -------------------------------------------------------



} //namespace binfilter
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
