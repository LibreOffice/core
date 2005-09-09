/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: PageListWatcher.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 03:09:51 $
 *
 *  The Contents of this file are made available subject to
 *  the terms of GNU Lesser General Public License Version 2.1.
 *
 *
 *    GNU Lesser General Public License Version 2.1
 *    =============================================
 *    Copyright 2005 by Sun Microsystems, Inc.
 *    901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *    This library is free software; you can redistribute it and/or
 *    modify it under the terms of the GNU Lesser General Public
 *    License version 2.1, as published by the Free Software Foundation.
 *
 *    This library is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *    Lesser General Public License for more details.
 *
 *    You should have received a copy of the GNU Lesser General Public
 *    License along with this library; if not, write to the Free Software
 *    Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *    MA  02111-1307  USA
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
