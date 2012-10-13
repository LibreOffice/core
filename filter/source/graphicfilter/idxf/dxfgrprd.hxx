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

#ifndef _DXFGRPRD_HXX
#define _DXFGRPRD_HXX

#include <sal/types.h>

#include <tools/stream.hxx>

#define DXF_MAX_STRING_LEN 256 // Max Stringlaenge (ohne die 0)


class DXFGroupReader
{

public:

    // Anmerkkung:
    // sizeof(DXFGroupReader) ist gross, also nur dynamisch anlegen!

    DXFGroupReader( SvStream & rIStream, sal_uInt16 nMinPercent, sal_uInt16 nMaxPercent );

    sal_Bool GetStatus() const;

    void SetError();

    sal_uInt16 Read();
        // Liesst die naechste Gruppe ein und liefert den Gruppencode zurueck.
        // Im Falle eines Fehlers liefert GetStatus() sal_False, Gruppencode wird 0
        // gesetzt, und es wird SetS(0,"EOF") ausgefuehrt.

    sal_uInt16 GetG() const;
        // Liefert den letzten Gruppencode (also was Read() zuletzt lieferte)

    long   GetI() const;
        // Liefert den Integer-Wert zur Gruppe, die vorher mit Read() gelesen wurde.
        // Dabei muss es sich um einen Gruppencode fuer den Datentyp Integer
        // gehandelt haben, wenn nicht, wird 0 gelieferet.

    double GetF() const;
        // Liefert den Floatingpoint-Wert zur Gruppe, die vorher mit Read() gelesen wurde.
        // Dabei muss es sich um einen Gruppencode fuer den Datentyp Floatingpoint
        // gehandelt haben, wenn nicht, wird 0 geliefert.

    const char * GetS() const;
        // Liefert den String zur Gruppe, die vorher mit Read() gelesen wurde.
        // Dabei muss es sich um einen Gruppencode fuer den Datentyp String
        // gehandelt haben, wenn nicht, wird NULL geliefert.

    // Folgende drei Methoden arbeiten wie die obigen, nur kann auch ein anderer als der
    // aktuelle Gruppencode angegeben werden. (DXFGroupReader speichert die Parameter
    // zu allen Gruppencodes. Dadurch ist es moeglich, dass zunaechst mit Read() einige
    // verschiedene Gruppen eingelesen werden, bevor sie ausgewertet werden.)
    long         GetI(sal_uInt16 nG) const;
    double       GetF(sal_uInt16 nG) const;
    const char * GetS(sal_uInt16 nG) const;

    // Mit folgenden Methoden koennen die aktuell gespeicherten Werte zu den
    // Gruppencodes veraendert werden. (z.B. um Defaultwerte zu setzen, bevor
    // 'blind' eine Menge von Gruppen eingelesen wird.)
    void SetF(sal_uInt16 nG, double fF);
    void SetS(sal_uInt16 nG, const char * sS); // (wird kopiert)

private:

    void   ReadLine(char * ptgt);
    long   ReadI();
    double ReadF();
    void   ReadS(char * ptgt);

    SvStream & rIS;
    sal_uInt16 nIBuffSize,nIBuffPos;
    sal_Bool bStatus;
    sal_uInt16 nLastG;
    sal_uLong nGCount;

    sal_uLong nMinPercent;
    sal_uLong nMaxPercent;
    sal_uLong nLastPercent;
    sal_uLong nFileSize;

    char   S0_9      [10][DXF_MAX_STRING_LEN+1]; // Strings  Gruppencodes 0..9
    double F10_59    [50];      // Floats   Gruppencodes 10..59
    long   I60_79    [20];      // Integers Gruppencodes 60..79
    long   I90_99    [10];
    char   S100      [DXF_MAX_STRING_LEN+1];
    char   S102      [DXF_MAX_STRING_LEN+1];
    double F140_147  [ 8];      // Floats   Gruppencodes 140..147
    long   I170_175  [ 6];      // Integers Gruppencodes 170..175
    double F210_239  [30];      // Floats   Gruppencodes 210..239
    char   S999_1009 [11][DXF_MAX_STRING_LEN+1]; // Strings  Gruppencodes 999..1009
    double F1010_1059[50];      // Floats   Gruppencodes 1010..1059
    long   I1060_1079[20];      // Integers Gruppencodes 1060..1079

};


inline sal_Bool DXFGroupReader::GetStatus() const
{
    return bStatus;
}


inline void DXFGroupReader::SetError()
{
    bStatus=sal_False;
}

inline sal_uInt16 DXFGroupReader::GetG() const
{
    return nLastG;
}

inline long DXFGroupReader::GetI() const
{
    return GetI(nLastG);
}

inline double DXFGroupReader::GetF() const
{
    return GetF(nLastG);
}

inline const char * DXFGroupReader::GetS() const
{
    return GetS(nLastG);
}

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
