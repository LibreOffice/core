/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: pglink.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: rt $ $Date: 2005-09-09 02:56:34 $
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

#ifndef _PGLINK_HXX
#define _PGLINK_HXX

#ifndef SVX_LIGHT

#ifndef _LNKBASE_HXX
#include <sfx2/lnkbase.hxx>
#endif

class SdPage;



class SdPageLink : public ::sfx2::SvBaseLink
{
    SdPage* pPage;

public:
    SdPageLink(SdPage* pPg, const String& rFileName, const String& rBookmarkName);
    virtual ~SdPageLink();

    virtual void Closed();
    virtual void DataChanged( const String& ,
                              const ::com::sun::star::uno::Any & );
    FASTBOOL     Connect() { return 0 != SvBaseLink::GetRealObject(); }
};

#else

class SdPageLink;

#endif

#endif     // _PGLINK_HXX




