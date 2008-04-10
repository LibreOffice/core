/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2008 by Sun Microsystems, Inc.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * $RCSfile: dbui.cxx,v $
 * $Revision: 1.7 $
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

// MARKER(update_precomp.py): autogen include statement, do not remove
#include "precompiled_sw.hxx"

// INCLUDE ---------------------------------------------------------------


#include <tools/list.hxx>
#include "wrtsh.hxx"


#include "dbui.hrc"
#include "dbui.hxx"

/*---------------------------------------------------------------------
    Beschreibung:
---------------------------------------------------------------------*/

PrintMonitor::PrintMonitor( Window *pParent, BOOL bEMail )
:   ModelessDialog( pParent, SW_RES(DLG_PRINTMONITOR) ),
    aDocName    (this, SW_RES( FT_DOCNAME )),
    aPrinting   (this, SW_RES( bEMail ? FT_SENDING : FT_PRINTING )),
    aPrinter    (this, SW_RES( FT_PRINTER       )),
    aPrintInfo  (this, SW_RES( FT_PRINTINFO     )),
    aCancel     (this, SW_RES( PB_CANCELPRNMON  ))
{
    if (bEMail)
    {
        SetText(SW_RES(STR_EMAILMON));
    }
    FreeResource();
}

/*---------------------------------------------------------------------
    Progress Indicator for Creation of personalized Mail Merge documents:
---------------------------------------------------------------------*/

CreateMonitor::CreateMonitor( Window *pParent )
:   ModelessDialog( pParent, SW_RES(DLG_MM_CREATIONMONITOR) ),
    m_aStatus           (this, SW_RES( FT_STATUS )),
    m_aProgress         (this, SW_RES( FT_PROGRESS )),
    m_aCreateDocuments  (this, SW_RES( FT_CREATEDOCUMENTS )),
    m_aCounting         (this, SW_RES( FT_COUNTING )),
    m_aCancelButton     (this, SW_RES( PB_CANCELPRNMON  )),
    m_sCountingPattern(),
    m_sVariable_Total( String::CreateFromAscii("%Y") ),
    m_sVariable_Position( String::CreateFromAscii("%X") ),
    m_nTotalCount(0),
    m_nCurrentPosition(0)
{
    FreeResource();

    m_sCountingPattern = m_aCounting.GetText();
    m_aCounting.SetText(String::CreateFromAscii("..."));
}

void CreateMonitor::UpdateCountingText()
{
    String sText(m_sCountingPattern);
    sText.SearchAndReplaceAll( m_sVariable_Total, String::CreateFromInt32( m_nTotalCount ) );
    sText.SearchAndReplaceAll( m_sVariable_Position, String::CreateFromInt32( m_nCurrentPosition ) );
    m_aCounting.SetText(sText);
}

void CreateMonitor::SetTotalCount( sal_Int32 nTotal )
{
    m_nTotalCount = nTotal;
    UpdateCountingText();
}

void CreateMonitor::SetCurrentPosition( sal_Int32 nCurrent )
{
    m_nCurrentPosition = nCurrent;
    UpdateCountingText();
}

void CreateMonitor::SetCancelHdl( const Link& rLink )
{
    m_aCancelButton.SetClickHdl( rLink );
}
