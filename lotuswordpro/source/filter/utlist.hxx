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
#ifndef INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_UTLIST_HXX
#define INCLUDED_LOTUSWORDPRO_SOURCE_FILTER_UTLIST_HXX

#include "ut.hxx"

namespace OpenStormBento
{

UtDefClassP(CUtListElmt);
UtDefClassP(CUtList);
UtDefClassP(CUtComparableListElmt);

class CUtListElmt
{
public: // Methods
    CUtListElmt()
        : cpNext(nullptr)
        , cpPrev(nullptr)
    {}
    explicit CUtListElmt(pCUtListElmt pPrev) { InsertAfter(pPrev); }
    explicit CUtListElmt(pCUtList pList);
    virtual ~CUtListElmt();
    bool OnList() { return cpNext != nullptr; }
    void MakeNotOnList() { cpNext = nullptr; } // Same as Remove but doesn't
                                            // patch up list
    pCUtListElmt GetNext() const { return cpNext; }
    void SetNext(pCUtListElmt pNext) { cpNext = pNext; }
    pCUtListElmt GetPrev() const { return cpPrev; }
    void SetPrev(pCUtListElmt pPrev) { cpPrev = pPrev; }
    void InsertAfter(pCUtListElmt pPrev)
    {
        cpNext = pPrev->cpNext;
        cpPrev = pPrev;
        cpNext->cpPrev = this;
        pPrev->cpNext = this;
    }

private: // Data
    pCUtListElmt cpNext;
    pCUtListElmt cpPrev;
};

class CUtComparableListElmt : public CUtListElmt
{
public: // Methods
    CUtComparableListElmt() { ; }
    explicit CUtComparableListElmt(pCUtComparableListElmt pPrev) :
      CUtListElmt(pPrev) { ; }
    explicit CUtComparableListElmt(pCUtList pList) : CUtListElmt(pList) { ; }

public: // Pure virtual methods
};

class CUtList
{
public: // Methods
    CUtList() { cDummyElmt.SetNext(&cDummyElmt);
      cDummyElmt.SetPrev(&cDummyElmt); }
    virtual ~CUtList();
    pCUtListElmt GetFirst() { return cDummyElmt.GetNext(); }
    pCUtListElmt GetLast() { return cDummyElmt.GetPrev(); }
    CUtListElmt& GetTerminating() { return cDummyElmt; }
    pCUtListElmt GetNextOrNULL(pCUtListElmt pCurr);

    void Destroy();

private: // Data
    CUtListElmt cDummyElmt;
};

class CUtOwningList : public CUtList
{
public: // Methods
    virtual ~CUtOwningList();
};
}//end namespace OpenStormBento
#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
