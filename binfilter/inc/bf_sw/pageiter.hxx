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
#ifndef _PAGEITER_HXX
#define _PAGEITER_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
namespace binfilter {

 class SwDoc;
 class SwPageFrm;
 struct SwPosition;
 class SwPageDesc;

 class SwPageIter {
    const SwDoc &rPDoc;
    const SwPageFrm *pPage;

 public:
    SwPageIter( const SwDoc &rDoc, const SwPosition &rStartPos );

    BOOL IsEnd() const { return 0 == pPage; }
    BOOL NextPage();
    const SwPageDesc* GetPageDesc() const;
    BOOL Seek( const SwPosition &rPos );
    BOOL GetPosition( SwPosition &rPos ) const;
 };


} //namespace binfilter
#endif

