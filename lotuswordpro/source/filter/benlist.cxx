/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*************************************************************************
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
 *  The Initial Developer of the Original Code is: IBM Corporation
 *
 *  Copyright: 2008 by IBM Corporation
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/
#include "first.hxx"
namespace OpenStormBento
{

pCBenNamedObject
FindNamedObject(pCUtList pList, const char * sName,
  pCBenNamedObjectListElmt * ppPrev)
{
    pCUtListElmt pTerminating = pList->GetTerminating();
    for (pCUtListElmt pCurr = pList->GetLast(); pCurr != pTerminating;
      pCurr = pCurr->GetPrev())
    {
        pCBenNamedObjectListElmt pCurrNamedObjectListElmt =
          (pCBenNamedObjectListElmt) pCurr;


        //Old code
//		int Comp = strcmp(sName, pCurrNamedObjectListElmt->GetNamedObject()->
//		  GetName());
        int Comp = strcmp(sName, pCurrNamedObjectListElmt->GetNamedObject()->
          GetNameCStr());


        if (Comp == 0)
            return pCurrNamedObjectListElmt->GetNamedObject();
        else if (Comp < 0)
            continue;
        else
        {
            if (ppPrev != NULL)
                *ppPrev = pCurrNamedObjectListElmt;
            return NULL;
        }
    }

    if (ppPrev != NULL)
        *ppPrev = (pCBenNamedObjectListElmt) pTerminating;
    return NULL;
}

// Assume list is of BenIDListElmt list elements, sorted by ID
pCBenIDListElmt
FindID(pCUtList pList, BenObjectID ObjectID, pCBenIDListElmt * ppPrev)
{
    pCUtListElmt pTerminating = pList->GetTerminating();
    for (pCUtListElmt pCurr = pList->GetLast(); pCurr != pTerminating;
      pCurr = pCurr->GetPrev())
    {
        pCBenIDListElmt pCurrIDListElmt = (pCBenIDListElmt) pCurr;

        if (ObjectID == pCurrIDListElmt->GetID())
            return pCurrIDListElmt;
        else if (ObjectID < pCurrIDListElmt->GetID())
            continue;
        else
        {
            if (ppPrev != NULL)
                *ppPrev = pCurrIDListElmt;
            return NULL;
        }
    }

    if (ppPrev != NULL)
        *ppPrev = (pCBenIDListElmt) pTerminating;
    return NULL;
}
} //end namespace OpenStormBento

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
