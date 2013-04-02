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

    // note:
    // sizeof(DXFGroupReader) is big, so only create dynamically

    DXFGroupReader( SvStream & rIStream, sal_uInt16 nMinPercent, sal_uInt16 nMaxPercent );

    sal_Bool GetStatus() const;

    void SetError();

    sal_uInt16 Read();
        // Reads next group and returns the group code.
        // In case of an error GetStatus() returns sal_False, group code will be set
        // to 0 and SetS(0,"EOF") will be executed.

    sal_uInt16 GetG() const;
        // Return the last group code (the one the last Read() did return).

    long   GetI() const;
        // Returns the integer value of the group which was read earlier with Read().
        // This read must have returned a group code for datatype Integer.
        // If not 0 is returend

    double GetF() const;
        // Returns the floating point value of the group which was read earlier with Read().
        // This read must have returned a group code for datatype Floatingpoint.
        // If not 0 is returend

    const char * GetS() const;
        // Returns the string of the group which was read earlier with Read().
        // This read must have returned a group code for datatype String.
        // If not NULL is returend

    // The following three methods work like the above execp that a different than the
    // current group code can bet set. (DXFGroupReader stroes the parameters of all
    // group codes. Therefore it is possible to first Read() some groups and then analyze
    // them afterwards.)

    long         GetI(sal_uInt16 nG) const;
    double       GetF(sal_uInt16 nG) const;
    const char * GetS(sal_uInt16 nG) const;

    // The following methods can change the current values of group codes
    // (e.g. to set default values, before 'blindly' reading lots of groups)

    void SetF(sal_uInt16 nG, double fF);
    void SetS(sal_uInt16 nG, const char * sS); // (will be copied)

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

    char   S0_9      [10][DXF_MAX_STRING_LEN+1]; // Strings  group codes  0..9
    double F10_59    [50];      // Floats   group codes 10..59
    long   I60_79    [20];      // Integers group codes 60..79
    long   I90_99    [10];
    char   S100      [DXF_MAX_STRING_LEN+1];
    char   S102      [DXF_MAX_STRING_LEN+1];
    double F140_147  [ 8];      // Floats   group codes 140..147
    long   I170_175  [ 6];      // Integers group codes 170..175
    double F210_239  [30];      // Floats   group codes 210..239
    char   S999_1009 [11][DXF_MAX_STRING_LEN+1]; // Strings  group codes 999..1009
    double F1010_1059[50];      // Floats   group codes 1010..1059
    long   I1060_1079[20];      // Integers group codes 1060..1079

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
