/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: fileobj.hxx,v $
 *
 *  $Revision: 1.8 $
 *
 *  last change: $Author: vg $ $Date: 2006-11-22 10:37:29 $
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
#ifndef _FILEOBJ_HXX
#define _FILEOBJ_HXX

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _LINKSRC_HXX //autogen
#include <sfx2/linksrc.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <sfx2/docfile.hxx>
#endif

#ifndef _SVXLINKMGR_HXX
#include "linkmgr.hxx"
#endif

class Graphic;
struct Impl_DownLoadData;
namespace sfx2 { class FileDialogHelper; }

class SvFileObject : public sfx2::SvLinkSource
{
    String              sFileNm;
    String              sFilter;
    String              sReferer;
    Link                aEndEditLink;
    SfxMediumRef        xMed;
    Impl_DownLoadData*  pDownLoadData;
    Window*             pOldParent;

    BYTE nType;

    BOOL bLoadAgain : 1;
    BOOL bSynchron : 1;
    BOOL bLoadError : 1;
    BOOL bWaitForData : 1;
    BOOL bInNewData : 1;
    BOOL bDataReady : 1;
    BOOL bMedUseCache : 1;
    BOOL bNativFormat : 1;
    BOOL bClearMedium : 1;
    BOOL bStateChangeCalled : 1;
    BOOL bInCallDownLoad : 1;

    BOOL GetGraphic_Impl( Graphic&, SvStream* pStream = 0 );
    BOOL LoadFile_Impl();
    void SendStateChg_Impl( LinkState nState );

    DECL_STATIC_LINK( SvFileObject, DelMedium_Impl, SfxMediumRef* );
    DECL_STATIC_LINK( SvFileObject, LoadGrfReady_Impl, void* );
    DECL_STATIC_LINK( SvFileObject, LoadGrfNewData_Impl, void* );
    DECL_LINK( DialogClosedHdl, sfx2::FileDialogHelper* );

protected:
    virtual ~SvFileObject();

public:
    SvFileObject();

    virtual BOOL GetData( ::com::sun::star::uno::Any & rData /*out param*/,
                            const String & rMimeType,
                            BOOL bSynchron = FALSE );

    virtual BOOL    Connect( sfx2::SvBaseLink* );
    virtual void    Edit( Window *, sfx2::SvBaseLink *, const Link& rEndEditHdl );

    // erfrage ob das man direkt auf die Daten zugreifen kann oder ob das
    // erst angestossen werden muss
    virtual BOOL IsPending() const;
    virtual BOOL IsDataComplete() const;

    void CancelTransfers();
    void SetTransferPriority( USHORT nPrio );
};


#endif

