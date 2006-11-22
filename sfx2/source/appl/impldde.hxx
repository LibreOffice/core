/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: impldde.hxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-22 10:55:09 $
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
#ifndef _IMPLDDE_HXX
#define _IMPLDDE_HXX

#include <linksrc.hxx>
#include <tools/string.hxx>

class DdeConnection;
class DdeData;
class DdeLink;
class DdeRequest;
class DdeTransaction;

namespace sfx2
{

class SvDDEObject : public SvLinkSource
{
    String sItem;

    DdeConnection* pConnection;
    DdeLink* pLink;
    DdeRequest* pRequest;
    ::com::sun::star::uno::Any * pGetData;

    BYTE bWaitForData : 1;                  // wird auf Daten gewartet?
    BYTE nError     : 7;                    // Error Code fuer den Dialog


    BOOL ImplHasOtherFormat( DdeTransaction& );
    DECL_LINK( ImplGetDDEData, DdeData* );
    DECL_LINK( ImplDoneDDEData, void* );

protected:
    virtual ~SvDDEObject();

public:
    SvDDEObject();

    virtual BOOL    GetData( ::com::sun::star::uno::Any & rData /*out param*/,
                                const String & aMimeType,
                                BOOL bSynchron = FALSE );

    virtual BOOL    Connect( SvBaseLink * );
    virtual void    Edit( Window* pParent, sfx2::SvBaseLink* pBaseLink, const Link& rEndEditHdl );

    virtual BOOL    IsPending() const;
    virtual BOOL    IsDataComplete() const;
};

}

#endif
