/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: swserv.hxx,v $
 *
 *  $Revision: 1.7 $
 *
 *  last change: $Author: vg $ $Date: 2007-10-22 15:07:32 $
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
#ifndef _SWSERV_HXX
#define _SWSERV_HXX

#ifndef _LINKSRC_HXX
#include <sfx2/linksrc.hxx>
#endif

class SwBookmark;
class SwSectionNode;
class SwBaseLink;
class SwTableNode;
struct SwPosition;
class SwPaM;

class SwServerObject : public ::sfx2::SvLinkSource
{
    using sfx2::SvLinkSource::SendDataChanged;

protected:
    enum ServerModes { BOOKMARK_SERVER, TABLE_SERVER, SECTION_SERVER, NONE_SERVER } eType;
    union {
        SwBookmark* pBkmk;
        SwTableNode* pTblNd;
        SwSectionNode* pSectNd;
    } CNTNT_TYPE;

    SwServerObject();

public:
    SwServerObject( SwBookmark& rBookmark )
        : eType( BOOKMARK_SERVER )
    {
        CNTNT_TYPE.pBkmk = &rBookmark;
    }
    SwServerObject( SwTableNode& rTableNd )
        : eType( TABLE_SERVER )
    {
        CNTNT_TYPE.pTblNd = &rTableNd;
    }
    SwServerObject( SwSectionNode& rSectNd )
        : eType( SECTION_SERVER )
    {
        CNTNT_TYPE.pSectNd = &rSectNd;
    }
    virtual ~SwServerObject();

    virtual BOOL GetData( ::com::sun::star::uno::Any & rData,
                             const String & rMimeType,
                             BOOL bSynchron = FALSE );

    BOOL SetData( const String & rMimeType,
                    const ::com::sun::star::uno::Any& rData );

    virtual void SendDataChanged( const SwPosition& rPos );
    virtual void SendDataChanged( const SwPaM& rRange );

    BOOL IsLinkInServer( const SwBaseLink* ) const;

    void SetNoServer() {  CNTNT_TYPE.pBkmk = 0, eType = NONE_SERVER; }
};

#ifndef SW_DECL_SWSERVEROBJECT_DEFINED
#define SW_DECL_SWSERVEROBJECT_DEFINED
SV_DECL_REF( SwServerObject )
#endif

#endif  // _SWSERV_HXX

