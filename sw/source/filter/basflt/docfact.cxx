/*************************************************************************
 *
 *  $RCSfile: docfact.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-19 10:59:14 $
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
#ifdef PRECOMPILED
#include "filt_pch.hxx"
#endif

#pragma hdrstop

#ifndef _REF_HXX
#include <tools/ref.hxx>
#endif

#ifndef _SHELLIO_HXX
#include <shellio.hxx>
#endif
#ifndef _DOC_HXX
#include <doc.hxx>
#endif
#ifndef _NODE_HXX
#include <node.hxx>
#endif

#ifndef _CMDID_H
#include <cmdid.h>
#endif


/******************************************************************************
 *  Methode     :   SwDocFac::SwDocFac( SwDoc *pDoc )
 *  Beschreibung:
 *  Erstellt    :   OK 01-24-94 11:32am
 *  Aenderung   :   OK 01-24-94 11:32am
 ******************************************************************************/


SwDocFac::SwDocFac( SwDoc *pDc )
    : pDoc( pDc )
{
    if( pDoc )
        pDoc->AddLink();
}

/******************************************************************************
 *  Methode     :   SwDocFac::~SwDocFac()
 *  Beschreibung:
 *  Erstellt    :   OK 01-24-94 11:33am
 *  Aenderung   :   OK 01-24-94 11:33am
 ******************************************************************************/


SwDocFac::~SwDocFac()
{
    if( pDoc && !pDoc->RemoveLink() )
        delete pDoc;
}

/******************************************************************************
 *  Methode     :   SwDoc *SwDocFac::GetDoc()
 *  Beschreibung:   Diese Methode legt immer einen Drucker an.
 *  Erstellt    :   OK 01-24-94 11:34am
 *  Aenderung   :   OK 01-24-94 11:34am
 ******************************************************************************/


SwDoc *SwDocFac::GetDoc()
{
    if( !pDoc )
    {
        pDoc = new SwDoc;
        pDoc->AddLink();
    }
    return pDoc;
}


/******************************************************************************
 *  Erstellt    :   JP 01-27-94 11:37am
 *  Aenderung   :   JP 01-27-94 11:37am
 ******************************************************************************/

ULONG SwDocFac::Count() const
{
    // minus den Grund-SectionNodes
    return pDoc ? pDoc->GetNodes().GetEndOfContent().GetIndex() - 10 : 0;
}

/*************************************************************************

      Source Code Control System - Header

      $Header: /zpool/svn/migration/cvs_rep_09_09_08/code/sw/source/filter/basflt/docfact.cxx,v 1.1.1.1 2000-09-19 10:59:14 hr Exp $

      Source Code Control System - Update

      $Log: not supported by cvs2svn $
      Revision 1.36  2000/09/18 16:04:39  willem.vandorp
      OpenOffice header added.

      Revision 1.35  2000/05/08 16:42:28  jp
      Changes for Unicode

      Revision 1.34  1999/06/29 10:14:48  KZ
      ins. #include <tools/ref.hxx>


      Rev 1.33   29 Jun 1999 12:14:48   KZ
   ins. #include <tools/ref.hxx>

      Rev 1.32   25 Nov 1997 19:07:32   MA
   includes

      Rev 1.31   03 Nov 1997 14:04:36   MA
   precomp entfernt

      Rev 1.30   09 Oct 1997 14:30:40   JP
   Umstellung NodeIndex/-Array/BigPtrArray

      Rev 1.29   03 Dec 1996 16:55:58   AMA
   Chg: Der Drucker wird nur im !Browsemodus angelegt.

      Rev 1.28   29 Oct 1996 12:05:40   JP
   am Doc ist das NodesArray nur noch ueber Get..() zugaenglich

      Rev 1.27   28 Jun 1996 14:18:54   MA
   includes

      Rev 1.26   24 Nov 1995 17:24:12   OM
   PCH->PRECOMPILED

      Rev 1.25   22 Nov 1995 15:31:34   JP
   Segementierung - virtual Methoden impl.

      Rev 1.24   02 Oct 1995 18:30:42   JP
   Durcker ItemSet erweitert

      Rev 1.23   28 Apr 1995 17:05:34   OS
   Range fuer Printer-Options-Item veraendert

      Rev 1.22   25 Apr 1995 19:41:54   JP
   ueberfluessige Methoden entfernt (rund um JobSetup)

      Rev 1.21   22 Apr 1995 19:09:04   JP
   CTOR: AddLink auf vorhandenes Doc; GetDoc: Printer nur auf bedarf anlegen (Bug 11653)

      Rev 1.20   04 Apr 1995 15:05:14   MA
   unuetze deletez entfernt.

      Rev 1.19   12 Feb 1995 18:14:40   JP
   GetDoc: falls Printer schon vorhanden ist nicht neu setzen - ClipBoard !!!!

      Rev 1.18   15 Dec 1994 20:49:14   SWG
   *ARR* Ersetzungen, svmem, __far_data etc.

      Rev 1.17   08 Nov 1994 16:17:58   OK
   Pre.Hdr gerichtet

      Rev 1.16   03 Nov 1994 17:38:52   PK
   sfxitemset fuer printer

*************************************************************************/


