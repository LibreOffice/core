/*************************************************************************
 *
 *  $RCSfile: svditer.cxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 17:01:24 $
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

#include "svditer.hxx"
#include "svdobj.hxx"
#include "svdogrp.hxx"
#include "svdpage.hxx"

////////////////////////////////////////////////////////////////////////////////////////////////////
//
//  @@ @@@@@@ @@@@@ @@@@@   @@@@  @@@@@@  @@@@  @@@@@
//  @@   @@   @@    @@  @@ @@  @@   @@   @@  @@ @@  @@
//  @@   @@   @@    @@  @@ @@  @@   @@   @@  @@ @@  @@
//  @@   @@   @@@@  @@@@@  @@@@@@   @@   @@  @@ @@@@@
//  @@   @@   @@    @@  @@ @@  @@   @@   @@  @@ @@  @@
//  @@   @@   @@    @@  @@ @@  @@   @@   @@  @@ @@  @@
//  @@   @@   @@@@@ @@  @@ @@  @@   @@    @@@@  @@  @@
//
////////////////////////////////////////////////////////////////////////////////////////////////////

SdrObjListIter::SdrObjListIter(const SdrObjList& rObjList, SdrIterMode eMode, FASTBOOL bRevSearch)
{
    pMainList=&rObjList;
    bReverse=bRevSearch;
    switch (eMode) {
        case IM_FLAT          : bRecurse=FALSE; bSkipGrp=FALSE; break;
        case IM_DEEPWITHGROUPS: bRecurse=TRUE;  bSkipGrp=FALSE; break;
        case IM_DEEPNOGROUPS  : bRecurse=TRUE;  bSkipGrp=TRUE;  break;
    }
    Reset();
}

SdrObjListIter::SdrObjListIter(const SdrObject& rGroup, SdrIterMode eMode, FASTBOOL bRevSearch)
{
    pMainList=rGroup.GetSubList();
    bReverse=bRevSearch;
    switch (eMode) {
        case IM_FLAT          : bRecurse=FALSE; bSkipGrp=FALSE; break;
        case IM_DEEPWITHGROUPS: bRecurse=TRUE;  bSkipGrp=FALSE; break;
        case IM_DEEPNOGROUPS  : bRecurse=TRUE;  bSkipGrp=TRUE;  break;
    }
    Reset();
}

void SdrObjListIter::Reset()
{
    pAktList=pMainList;
    pAktGroup=NULL;
    nAktNum=0;
    nObjAnz=pMainList->aList.Count();
    if (bReverse && nObjAnz!=0) {
        nAktNum=nObjAnz-1;
    }
    pNextObj=pAktList->GetObj(nAktNum);
    if (bReverse && bRecurse && pNextObj!=NULL) { // rekursive Ruekwaertssuche
        // hier wird nun das tiefste Objekt der letzten Gruppe gesucht,
        // falls das letzte Objekt der Hauptgruppe ein Gruppenobjekt ist.
        FASTBOOL bWeiter=TRUE;
        while (bWeiter) {
            SdrObjList* pOL=pNextObj->GetSubList();
            if (pOL!=NULL) {
                ULONG nTmpObjCnt=pOL->GetObjCount();
                if (nTmpObjCnt!=0) {
                    pAktList=pOL;
                    pAktGroup=pNextObj;
                    nObjAnz=nTmpObjCnt;
                    nAktNum=nObjAnz-1;
                    pNextObj=pAktList->GetObj(nAktNum);
                } else bWeiter=FALSE;
            } else bWeiter=FALSE;
        }
    }
    if (bSkipGrp) {
        while (pNextObj!=NULL && pNextObj->IsGroupObject()) {
            Next();
        }
    }
}

SdrObject* SdrObjListIter::Next()
{
    const SdrObject* r=pNextObj;
    do {
        if (pNextObj!=NULL) {
            if (!bReverse) {
                SdrObjList* pSub=pNextObj->GetSubList();
                if (bRecurse && pSub!=NULL && pSub->GetObjCount()!=0) { // Abstieg in eine Subliste
                    pAktGroup=/*(SdrObjGroup*)*/pNextObj;
                    ((SdrObject*)pAktGroup)->SetOrdNum(nAktNum);
                    pAktList=pAktGroup->GetSubList();
                    nAktNum=0;
                    pNextObj=(SdrObject*)pAktList->aList.GetObject(nAktNum);
                } else {
                    nAktNum++;
                    pNextObj=pAktList->GetObj(nAktNum);
                    while (pNextObj==NULL && pAktGroup!=NULL) { // Ende Subliste
                        nAktNum=pAktGroup->nOrdNum; //GetOrdNum();
                        pAktGroup=pAktGroup->GetUpGroup();
                        if (pAktGroup==NULL) pAktList=pMainList; // dann wird das wohl die Page sein
                        if (pAktGroup!=NULL) {
                            pAktList=pAktGroup->GetSubList();
                            if (pAktList==pMainList) pAktGroup=NULL; // nicht an der MainList nach oben vorbeischliddern
                        }
                        nAktNum++;
                        pNextObj=(SdrObject*)pAktList->aList.GetObject(nAktNum);
                    }
                }
            } else {
                if (nAktNum>0) {
                    nAktNum--;
                    pNextObj=(SdrObject*)pAktList->aList.GetObject(nAktNum);
                    SdrObjList* pSub=pNextObj->GetSubList();
                    if (bRecurse && pSub!=NULL && pSub->GetObjCount()!=0) { // rueckwaerts auf eine Gruppe gestossen
                        // hier wird nun das tiefste Objekt der letzten Gruppe gesucht.
                        FASTBOOL bWeiter=TRUE;
                        while (bWeiter) {
                            SdrObjList* pOL=pNextObj->GetSubList();
                            if (pOL!=NULL) {
                                ULONG nTmpObjCnt=pOL->GetObjCount();
                                if (nTmpObjCnt!=0) {
                                    pAktList=pOL;
                                    pAktGroup=pNextObj;
                                    nObjAnz=nTmpObjCnt;
                                    nAktNum=nObjAnz-1;
                                    pNextObj=pAktList->GetObj(nAktNum);
                                } else bWeiter=FALSE;
                            } else bWeiter=FALSE;
                        }
                    }
                } else {
                    if (bRecurse && pAktList!=pMainList && pAktGroup!=NULL) { // Ende der SubListe
                        // fortfahren mit dem
                        pNextObj=pAktGroup;
                        nAktNum=pAktGroup->GetOrdNum();
                        pAktGroup=pAktGroup->GetUpGroup();
                        if (pAktGroup!=NULL) {
                            pAktList=pAktGroup->GetSubList();
                        } else {
                            pAktList=pMainList;
                        }
                        if (pAktList==pMainList) pAktGroup=NULL; // nicht an der MainList nach oben vorbeischliddern
                    } else { // ansonsten Listenende
                        pNextObj=NULL;
                    }
                }
            }
        }
    } while (bSkipGrp && pNextObj!=NULL && pNextObj->IsGroupObject());
    return (SdrObject*)r;
}

