/*************************************************************************
 *
 *  $RCSfile: cusshow.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 00:16:46 $
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

#pragma hdrstop

#include "sdiocmpt.hxx"
#include "cusshow.hxx"
#include "sdpage.hxx"
#include "drawdoc.hxx"



/*************************************************************************
|*
|* Ctor
|*
\************************************************************************/
SdCustomShow::SdCustomShow(SdDrawDocument* pDrawDoc)
  : List(),
  pDoc(pDrawDoc)
{
}

/*************************************************************************
|*
|* Copy-Ctor
|*
\************************************************************************/
SdCustomShow::SdCustomShow( const SdCustomShow& rShow )
    : List( rShow )
{
    aName = rShow.GetName();
    pDoc = rShow.GetDoc();
}

/*************************************************************************
|*
|* Dtor
|*
\************************************************************************/
SdCustomShow::~SdCustomShow()
{
}


/*************************************************************************
|*
|* Inserter fuer SvStream zum Speichern
|*
\************************************************************************/
SvStream& operator << (SvStream& rOut, const SdCustomShow& rCustomShow)
{
    // Letzter Parameter ist die aktuelle Versionsnummer des Codes
    SdIOCompat aIO(rOut, STREAM_WRITE, 0);

    // Name
    rOut.WriteByteString( rCustomShow.aName, ::GetStoreCharSet( gsl_getSystemTextEncoding() ) );

    // Anzahl Seiten
    UINT32 nCount = rCustomShow.Count();
    rOut << nCount;

    for (UINT32 i = 0; i < nCount; i++)
    {
        // Seite aus Liste holen
        SdPage* pPage = (SdPage*) rCustomShow.GetObject(i);

        if (pPage)
        {
            // SdPage-Seitennummer
            UINT16 nPageNum = (pPage->GetPageNum() - 1) / 2;
            rOut << nPageNum;
        }
    }

    return rOut;
}

/*************************************************************************
|*
|* Extractor fuer SvStream zum Laden
|*
\************************************************************************/
SvStream& operator >> (SvStream& rIn, SdCustomShow& rCustomShow)
{
    SdIOCompat aIO(rIn, STREAM_READ);

    // Name
    rIn.ReadByteString( rCustomShow.aName, ::GetStoreCharSet( gsl_getSystemTextEncoding() ) );

    // Anzahl Seiten
    UINT32 nCount = 0;
    rIn >> nCount;

    rCustomShow.Clear();

    for (UINT32 i = 0; i < nCount; i++)
    {
        // Seitennummer
        UINT16 nPageNum;
        rIn >> nPageNum;

        // Seite in Liste einfuegen
        SdPage* pPage = (SdPage*) rCustomShow.pDoc->GetSdPage(nPageNum, PK_STANDARD);
        rCustomShow.Insert(pPage, LIST_APPEND);
    }

    return rIn;
}




