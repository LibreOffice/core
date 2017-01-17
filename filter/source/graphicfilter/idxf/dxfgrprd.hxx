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

#ifndef INCLUDED_FILTER_SOURCE_GRAPHICFILTER_IDXF_DXFGRPRD_HXX
#define INCLUDED_FILTER_SOURCE_GRAPHICFILTER_IDXF_DXFGRPRD_HXX

#include <rtl/string.hxx>
#include <sal/types.h>
#include <tools/solar.h>

class SvStream;

class DXFGroupReader
{
public:
    explicit DXFGroupReader( SvStream & rIStream );

    bool GetStatus() const;

    void SetError();

    sal_uInt16 Read();
        // Reads next group and returns the group code.
        // In case of an error GetStatus() returns sal_False, group code will be set
        // to 0 and SetS(0,"EOF") will be executed.
    bool Read(sal_uInt16 nExpectedG) { return Read() == nExpectedG; }

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

    const OString& GetS() const;
        // Returns the string of the group which was read earlier with Read().
        // This read must have returned a group code for datatype String.
        // If not NULL is returend

    // The following methods can change the current values of group codes
    // (e.g. to set default values, before 'blindly' reading lots of groups)

    void SetS(); // (will be copied)

    sal_uInt64 remainingSize() const;
private:

    long   ReadI();
    double ReadF();
    void   ReadS();

    SvStream & rIS;
    bool bStatus;
    sal_uInt16 nLastG;
    sal_uLong nGCount;

    sal_uLong nFileSize;

    OString S;
    union {
        double F;
        long I;
    };
};


inline bool DXFGroupReader::GetStatus() const
{
    return bStatus;
}


inline void DXFGroupReader::SetError()
{
    bStatus=false;
}

inline sal_uInt16 DXFGroupReader::GetG() const
{
    return nLastG;
}

inline long DXFGroupReader::GetI() const
{
    return I;
}

inline double DXFGroupReader::GetF() const
{
    return F;
}

inline const OString& DXFGroupReader::GetS() const
{
    return S;
}

#endif


/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
