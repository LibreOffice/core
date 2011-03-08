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

#ifndef SC_QPRO_HXX
#define SC_QPRO_HXX

#include <sal/config.h>
#include "filter.hxx"
#include "document.hxx"
#include "cell.hxx"
#include <tools/string.hxx>

#include <tools/color.hxx>
#include "flttypes.hxx"
#include "ftools.hxx"
#include "qprostyle.hxx"
#include "biff.hxx"

// Stream wrapper class
class ScQProReader : public ScBiffReader
{
    public:
    bool recordsLeft();
    void SetEof( bool bValue ){ mbEndOfFile = bValue; }
    bool nextRecord();
    sal_uInt16 getId() { return mnId; }
    sal_uInt16 getLength() { return mnLength; }
    void readString( String &rString, sal_uInt16 nLength );
    ScQProReader( SfxMedium &rMedium );
    ~ScQProReader(){ };
    FltError import( ScDocument *pDoc );
    FltError readSheet( SCTAB nTab, ScDocument* pDoc, ScQProStyle *pStyle );
};
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
