/*************************************************************************
 *
 *  $RCSfile: dxfgrprd.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:30:15 $
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

#ifndef _DXFGRPRD_HXX
#define _DXFGRPRD_HXX

#include <svtools/fltcall.hxx>

#define DXF_MAX_STRING_LEN 256 // Max Stringlaenge (ohne die 0)


class DXFGroupReader
{

public:

    // Anmerkkung:
    // sizeof(DXFGroupReader) ist gross, also nur dynamisch anlegen!

    DXFGroupReader(SvStream & rIStream,
                   PFilterCallback pCallback, void * pCallerData,
                   USHORT nMinPercent, USHORT nMaxPercent);

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
    void SetI(USHORT nG, long nI);
    void SetF(USHORT nG, double fF);
    void SetS(USHORT nG, const char * sS); // (wird kopiert)

private:

    void   FillIBuff();
    char   ReadChar();
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

    PFilterCallback pCallback;
    void * pCallerData;
    ULONG nMinPercent;
    ULONG nMaxPercent;
    ULONG nLastPercent;
    ULONG nFileSize;

    char   S0_9      [10][DXF_MAX_STRING_LEN+1]; // Strings  Gruppencodes 0..9
    double F10_59    [50];      // Floats   Gruppencodes 10..59
    long   I60_79    [20];      // Integers Gruppencodes 60..79
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

inline char DXFGroupReader::ReadChar()
{
    if (nIBuffPos>=nIBuffSize) FillIBuff();
    return sIBuff[nIBuffPos++];
}


#endif


