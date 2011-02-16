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

#ifndef _PARAGRAPH_DATA_HXX
#define _PARAGRAPH_DATA_HXX

#include <tools/solar.h>
#include <vector>

//////////////////////////////////////////////////////////////////////////////
// Only for internal use, oder some kind like hPara for the few
// functions where you need it outside ( eg. moving paragraphs... )
//
// Unfortunately NOT only local (formally in outliner.hxx), but also
// used in outlobj.hxx. Moved to own header

class ParagraphData
{
    friend class Paragraph;
    friend class OutlinerParaObject;

protected:
    sal_Int16           nDepth;
    sal_Int16           mnNumberingStartValue;
    sal_Bool            mbParaIsNumberingRestart;

public:
    ParagraphData( const ParagraphData& );
    ParagraphData();

    ParagraphData& operator=( const ParagraphData& );

    // compare operator
    bool operator==(const ParagraphData& rCandidate) const;

    // data read access
    sal_Int16 getDepth() const { return nDepth; }
};

//////////////////////////////////////////////////////////////////////////////

typedef ::std::vector< ParagraphData > ParagraphDataVector;

//////////////////////////////////////////////////////////////////////////////

#endif // _PARAGRAPH_DATA_HXX

// eof

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
