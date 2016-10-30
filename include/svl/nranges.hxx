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
#ifndef INCLUDED_SVL_NRANGES_HXX
#define INCLUDED_SVL_NRANGES_HXX

#include <cstdarg>
#include <sal/types.h>

/**
 * Creates a sal_uInt16-ranges-array in 'rpRanges' using 'nWh1' and 'nWh2' as
 * first range, 'nNull' as terminator or start of 2nd range and 'pArgs' as
 * remainder.
 *
 * It returns the number of sal_uInt16s which are contained in the described
 * set of sal_uInt16s.
 */
sal_uInt16 InitializeRanges_Impl( sal_uInt16 *&rpRanges, va_list pArgs,
                               sal_uInt16 nWh1, sal_uInt16 nWh2, sal_uInt16 nNull );

/**
 * Determines the number of sal_uInt16s in a 0-terminated array of pairs of
 * sal_uInt16s.
 * The terminating 0 is not included in the count.
 */
sal_uInt16 Count_Impl( const sal_uInt16 *pRanges );

/**
 * Determines the total number of sal_uInt16s described in a 0-terminated
 * array of pairs of sal_uInt16s, each representing an range of sal_uInt16s.
 */
sal_uInt16 Capacity_Impl( const sal_uInt16 *pRanges );

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
