/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: svdopage.hxx,v $
 *
 *  $Revision: 1.2 $
 *
 *  last change: $Author: vg $ $Date: 2007-04-11 16:25:03 $
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

#ifndef _SVDOPAGE_HXX
#define _SVDOPAGE_HXX

#ifndef _SVDOBJ_HXX
#include <svx/svdobj.hxx>
#endif

#ifndef _SDR_PAGEUSER_HXX
#include <svx/sdrpageuser.hxx>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

class SVX_DLLPUBLIC SdrPageObj : public SdrObject, public sdr::PageUser
{
public:
    // this method is called form the destructor of the referenced page.
    // do all necessary action to forget the page. It is not necessary to call
    // RemovePageUser(), that is done form the destructor.
    virtual void PageInDestruction(const SdrPage& rPage);

private:
    // BaseProperties section
    virtual sdr::properties::BaseProperties* CreateObjectSpecificProperties();

    // DrawContact section
private:
    virtual sdr::contact::ViewContact* CreateObjectSpecificViewContact();

    // #111111#
    // To make things more safe, remember the page, not a number
    SdrPage*                                mpShownPage;

public:
    TYPEINFO();
    SdrPageObj(SdrPage* pNewPage = 0L);
    SdrPageObj(const Rectangle& rRect, SdrPage* pNewPage = 0L);
    ~SdrPageObj();

    // #111111#
    SdrPage* GetReferencedPage() const;
    void SetReferencedPage(SdrPage* pNewPage);

    virtual UINT16 GetObjIdentifier() const;
    virtual void TakeObjInfo(SdrObjTransformInfoRec& rInfo) const;
    virtual void operator=(const SdrObject& rObj);

    virtual void TakeObjNameSingul(String& rName) const;
    virtual void TakeObjNamePlural(String& rName) const;
};

////////////////////////////////////////////////////////////////////////////////////////////////////

#endif //_SVDOPAGE_HXX

