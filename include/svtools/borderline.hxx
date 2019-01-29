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

#ifndef INCLUDED_SVTOOLS_BORDERLINE_HXX
#define INCLUDED_SVTOOLS_BORDERLINE_HXX

#include <svtools/svtdllapi.h>
#include <o3tl/typed_flags_set.hxx>

/**
    Class computing border widths shared between Line style listbox and the
    SvxBorderLine implementation.

    This class doesn't know anything about units: it all depends on the different
    values set. A border is composed of 2 lines separated by a gap. The computed
    widths are the ones of each line and the gap and they can either be fix or vary.

    The #m_nflags member will define which widths will vary (value 0 means that all
    widths are fixed). The available flags are:
     - CHANGE_LINE1
     - CHANGE_LINE2
     - CHANGE_DIST

    For each line, the rate member is used as a multiplication factor is the width
    isn't fixed. Otherwise it is the width in the unit expected by the client code.
 */
enum class BorderWidthImplFlags
{
    FIXED           = 0,
    CHANGE_LINE1    = 1,
    CHANGE_LINE2    = 2,
    CHANGE_DIST     = 4,
};
namespace o3tl
{
    template<> struct typed_flags<BorderWidthImplFlags> : is_typed_flags<BorderWidthImplFlags, 0x07> {};
}
class SVT_DLLPUBLIC BorderWidthImpl
{
    BorderWidthImplFlags m_nFlags;
    double m_nRate1;
    double m_nRate2;
    double m_nRateGap;

public:

    BorderWidthImpl( BorderWidthImplFlags nFlags = BorderWidthImplFlags::CHANGE_LINE1, double nRate1 = 0.0,
            double nRate2 = 0.0, double nRateGap = 0.0 );

    bool operator== ( const BorderWidthImpl& r ) const;

    long GetLine1 ( long nWidth ) const;
    long GetLine2( long nWidth ) const;
    long GetGap( long nWidth ) const;

    long GuessWidth( long nLine1, long nLine2, long nGap );

    bool IsEmpty( ) const { return (0 == m_nRate1) && (0 == m_nRate2); }
    bool IsDouble( ) const { return (0 != m_nRate1) && (0 != m_nRate2);  }
};

#endif // INCLUDED_SVTOOLS_BORDERLINE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
