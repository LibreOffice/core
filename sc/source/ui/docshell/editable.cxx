/*************************************************************************
 *
 *  $RCSfile: editable.cxx,v $
 *
 *  $Revision: 1.1 $
 *
 *  last change: $Author: nn $ $Date: 2002-11-20 14:33:07 $
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

#ifdef PCH
#include "ui_pch.hxx"
#endif

#pragma hdrstop

#include "editable.hxx"
#include "document.hxx"
#include "viewfunc.hxx"
#include "globstr.hrc"

//------------------------------------------------------------------

ScEditableTester::ScEditableTester() :
    bIsEditable( TRUE ),
    bOnlyMatrix( TRUE )
{
}

void ScEditableTester::Reset()
{
    bIsEditable = bOnlyMatrix = TRUE;
}

ScEditableTester::ScEditableTester( ScDocument* pDoc, USHORT nTab,
                        USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow ) :
    bIsEditable( TRUE ),
    bOnlyMatrix( TRUE )
{
    TestBlock( pDoc, nTab, nStartCol, nStartRow, nEndCol, nEndRow );
}

ScEditableTester::ScEditableTester( ScDocument* pDoc,
                        USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                        const ScMarkData& rMark ) :
    bIsEditable( TRUE ),
    bOnlyMatrix( TRUE )
{
    TestSelectedBlock( pDoc, nStartCol, nStartRow, nEndCol, nEndRow, rMark );
}

ScEditableTester::ScEditableTester( ScDocument* pDoc, const ScRange& rRange ) :
    bIsEditable( TRUE ),
    bOnlyMatrix( TRUE )
{
    TestRange( pDoc, rRange );
}

ScEditableTester::ScEditableTester( ScDocument* pDoc, const ScMarkData& rMark ) :
    bIsEditable( TRUE ),
    bOnlyMatrix( TRUE )
{
    TestSelection( pDoc, rMark );
}

ScEditableTester::ScEditableTester( ScViewFunc* pView ) :
    bIsEditable( TRUE ),
    bOnlyMatrix( TRUE )
{
    TestView( pView );
}

//------------------------------------------------------------------

void ScEditableTester::TestBlock( ScDocument* pDoc, USHORT nTab,
                        USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow )
{
    if ( bIsEditable || bOnlyMatrix )
    {
        BOOL bThisMatrix;
        if ( !pDoc->IsBlockEditable( nTab, nStartCol, nStartRow, nEndCol, nEndRow, &bThisMatrix ) )
        {
            bIsEditable = FALSE;
            if ( !bThisMatrix )
                bOnlyMatrix = FALSE;
        }
    }
}

void ScEditableTester::TestSelectedBlock( ScDocument* pDoc,
                        USHORT nStartCol, USHORT nStartRow, USHORT nEndCol, USHORT nEndRow,
                        const ScMarkData& rMark )
{
    USHORT nTabCount = pDoc->GetTableCount();
    for (USHORT nTab=0; nTab<nTabCount; nTab++)
        if (rMark.GetTableSelect(nTab))
            TestBlock( pDoc, nTab, nStartCol, nStartRow, nEndCol, nEndRow );
}

void ScEditableTester::TestRange( ScDocument* pDoc, const ScRange& rRange )
{
    USHORT nStartCol = rRange.aStart.Col();
    USHORT nStartRow = rRange.aStart.Row();
    USHORT nStartTab = rRange.aStart.Tab();
    USHORT nEndCol = rRange.aEnd.Col();
    USHORT nEndRow = rRange.aEnd.Row();
    USHORT nEndTab = rRange.aEnd.Tab();
    for (USHORT nTab=nStartTab; nTab<=nEndTab; nTab++)
        TestBlock( pDoc, nTab, nStartCol, nStartRow, nEndCol, nEndRow );
}

void ScEditableTester::TestSelection( ScDocument* pDoc, const ScMarkData& rMark )
{
    if ( bIsEditable || bOnlyMatrix )
    {
        BOOL bThisMatrix;
        if ( !pDoc->IsSelectionEditable( rMark, &bThisMatrix ) )
        {
            bIsEditable = FALSE;
            if ( !bThisMatrix )
                bOnlyMatrix = FALSE;
        }
    }
}

void ScEditableTester::TestView( ScViewFunc* pView )
{
    if ( bIsEditable || bOnlyMatrix )
    {
        BOOL bThisMatrix;
        if ( !pView->SelectionEditable( &bThisMatrix ) )
        {
            bIsEditable = FALSE;
            if ( !bThisMatrix )
                bOnlyMatrix = FALSE;
        }
    }
}

//------------------------------------------------------------------

USHORT ScEditableTester::GetMessageId() const
{
    if (bIsEditable)
        return 0;
    else if (bOnlyMatrix)
        return STR_MATRIXFRAGMENTERR;
    else
        return STR_PROTECTIONERR;
}

