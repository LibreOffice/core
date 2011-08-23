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

#ifndef SC_EDITABLE_HXX
#define SC_EDITABLE_HXX

#ifndef _SOLAR_H
#include <tools/solar.h>
#endif
namespace binfilter {

class ScDocument;
class ScViewFunc;
class ScMarkData;
class ScRange;


class ScEditableTester
{
    BOOL	bIsEditable;
    BOOL	bOnlyMatrix;

public:
            // no test in ctor
            ScEditableTester();

            // calls TestBlock
            ScEditableTester( ScDocument* pDoc, USHORT nTab,
                        USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow );

            // calls TestSelectedBlock
            ScEditableTester( ScDocument* pDoc, 
                        USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                        const ScMarkData& rMark );

            // calls TestRange
            ScEditableTester( ScDocument* pDoc, const ScRange& rRange );

            // calls TestSelection
            ScEditableTester( ScDocument* pDoc, const ScMarkData& rMark );

            // calls TestView

            ~ScEditableTester() {}

            // Several calls to the Test... methods check if *all* of the ranges
            // are editable. For several independent checks, Reset() has to be used.
    void	TestBlock( ScDocument* pDoc, USHORT nTab,
                        USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow );
    void	TestSelectedBlock( ScDocument* pDoc, 
                        USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                        const ScMarkData& rMark );


    BOOL	IsEditable() const			{ return bIsEditable; }
    BOOL	IsFormatEditable() const	{ return bIsEditable || bOnlyMatrix; }
    USHORT	GetMessageId() const;
};

} //namespace binfilter
#endif

