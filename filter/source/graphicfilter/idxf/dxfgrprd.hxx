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

#ifndef _DXFGRPRD_HXX
#define _DXFGRPRD_HXX

#include <svtools/fltcall.hxx>

#define DXF_MAX_STRING_LEN 256 // Max Stringlaenge (ohne die 0)


class DXFGroupReader
{

public:

    // Anmerkkung:
    // sizeof(DXFGroupReader) ist gross, also nur dynamisch anlegen!

    DXFGroupReader( SvStream & rIStream, USHORT nMinPercent, USHORT nMaxPercent );

    BOOL GetStatus();

    void SetError();

    USHORT Read();
        // Liesst die naechste Gruppe ein und liefert den Gruppencode zurueck.
        // Im Falle eines Fehlers liefert GetStatus() FALSE, Gruppencode wird 0
        // gesetzt, und es wird SetS(0,"EOF") ausgefuehrt.

    USHORT GetG();
        // Liefert den letzten Gruppencode (also was Read() zuletzt lieferte)

    long   GetI();
        // Liefert den Integer-Wert zur Gruppe, die vorher mit Read() gelesen wurde.
        // Dabei muss es sich um einen Gruppencode fuer den Datentyp Integer
        // gehandelt haben, wenn nicht, wird 0 gelieferet.

    double GetF();
        // Liefert den Floatingpoint-Wert zur Gruppe, die vorher mit Read() gelesen wurde.
        // Dabei muss es sich um einen Gruppencode fuer den Datentyp Floatingpoint
        // gehandelt haben, wenn nicht, wird 0 geliefert.

    const char * GetS();
        // Liefert den String zur Gruppe, die vorher mit Read() gelesen wurde.
        // Dabei muss es sich um einen Gruppencode fuer den Datentyp String
        // gehandelt haben, wenn nicht, wird NULL geliefert.

    // Folgende drei Methoden arbeiten wie die obigen, nur kann auch ein anderer als der
    // aktuelle Gruppencode angegeben werden. (DXFGroupReader speichert die Parameter
    // zu allen Gruppencodes. Dadurch ist es moeglich, dass zunaechst mit Read() einige
    // verschiedene Gruppen eingelesen werden, bevor sie ausgewertet werden.)
    long         GetI(USHORT nG);
    double       GetF(USHORT nG);
    const char * GetS(USHORT nG);

    // Mit folgenden Methoden koennen die aktuell gespeicherten Werte zu den
    // Gruppencodes veraendert werden. (z.B. um Defaultwerte zu setzen, bevor
    // 'blind' eine Menge von Gruppen eingelesen wird.)
    void SetF(USHORT nG, double fF);
    void SetS(USHORT nG, const char * sS); // (wird kopiert)

private:

    void   ReadLine(char * ptgt);
    long   ReadI();
    double ReadF();
    void   ReadS(char * ptgt);

    SvStream & rIS;
    char sIBuff[1024];
    USHORT nIBuffSize,nIBuffPos;
    BOOL bStatus;
    USHORT nLastG;
    ULONG nGCount;

    ULONG nMinPercent;
    ULONG nMaxPercent;
    ULONG nLastPercent;
    ULONG nFileSize;

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


inline BOOL DXFGroupReader::GetStatus()
{
    return bStatus;
}


inline void DXFGroupReader::SetError()
{
    bStatus=FALSE;
}

inline USHORT DXFGroupReader::GetG()
{
    return nLastG;
}

inline long DXFGroupReader::GetI()
{
    return GetI(nLastG);
}

inline double DXFGroupReader::GetF()
{
    return GetF(nLastG);
}

inline const char * DXFGroupReader::GetS()
{
    return GetS(nLastG);
}

#endif


