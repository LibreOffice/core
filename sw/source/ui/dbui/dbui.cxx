/*************************************************************************
 *
 *  $RCSfile: dbui.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: obo $ $Date: 2004-11-16 16:57:35 $
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

// INCLUDE ---------------------------------------------------------------


#pragma hdrstop

#ifndef _LIST_HXX //autogen
#include <tools/list.hxx>
#endif
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
