/*************************************************************************
 *
 *  $RCSfile: PageListWatcher.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: obo $ $Date: 2004-01-20 10:25:23 $
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

#ifndef PAGE_LIST_WATCHER_HXX
#define PAGE_LIST_WATCHER_HXX

#include "pres.hxx"
#include <sal/types.h>
#include <vector>

class SdPage;
class SdrModel;

/** Maintain a map of page indices to page objects for faster access that
    remains valid during deletions and insertions of pages (#109538#).
*/
class ImpPageListWatcher
{
protected:
    // typedefs for a vector of SdPages
    typedef ::std::vector< SdPage* > SdPageVector;

    const SdrModel&                 mrModel;

    SdPageVector                    maPageVectorStandard;
    SdPageVector                    maPageVectorNotes;
    SdPage*                         mpHandoutPage;

    sal_Bool                        mbPageListValid;

    void ImpRecreateSortedPageListOnDemand();
    virtual sal_uInt32 ImpGetPageCount() const = 0;

    /** Return the page with the given index.
        @param nIndex
            When given an invalid index then NULL is returned.
    */
    virtual SdPage* ImpGetPage (sal_uInt32 nIndex) const = 0;

public:
    ImpPageListWatcher(const SdrModel& rModel);
    virtual ~ImpPageListWatcher();

    void Invalidate() { mbPageListValid = sal_False; }
    SdPage* GetSdPage(PageKind ePgKind, sal_uInt32 nPgNum = 0L);
    sal_uInt32 GetSdPageCount(PageKind ePgKind);
};

//////////////////////////////////////////////////////////////////////////////

class ImpDrawPageListWatcher : public ImpPageListWatcher
{
protected:
    virtual sal_uInt32 ImpGetPageCount() const;
    virtual SdPage* ImpGetPage(sal_uInt32 nIndex) const;

public:
    ImpDrawPageListWatcher(const SdrModel& rModel);
    virtual ~ImpDrawPageListWatcher();
};

//////////////////////////////////////////////////////////////////////////////

class ImpMasterPageListWatcher : public ImpPageListWatcher
{
protected:
    virtual sal_uInt32 ImpGetPageCount() const;
    virtual SdPage* ImpGetPage(sal_uInt32 nIndex) const;

public:
    ImpMasterPageListWatcher(const SdrModel& rModel);
    virtual ~ImpMasterPageListWatcher();
};

#endif
