/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 * 
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ************************************************************************/
#ifndef _FILEOBJ_HXX
#define _FILEOBJ_HXX

#ifndef _LINKSRC_HXX //autogen
#include <bf_so3/linksrc.hxx>
#endif
#ifndef _SFXDOCFILE_HXX //autogen
#include <bf_sfx2/docfile.hxx>
#endif
class Graphic;
namespace binfilter {

struct Impl_DownLoadData;


class SvFileObject : public ::binfilter::SvLinkSource
{
    String sFileNm;
    String sFilter;
    String sReferer;
    SfxMediumRef xMed;
    Impl_DownLoadData* pDownLoadData;

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
    void SendStateChg_Impl( USHORT nState );

    DECL_STATIC_LINK( SvFileObject, DelMedium_Impl, SfxMediumRef* );
    DECL_STATIC_LINK( SvFileObject, LoadGrfReady_Impl, void* );
    DECL_STATIC_LINK( SvFileObject, LoadGrfNewData_Impl, void* );
protected:
    virtual ~SvFileObject();

public:
    SvFileObject();

    virtual BOOL GetData( ::com::sun::star::uno::Any & rData /*out param*/,
                            const String & rMimeType,
                            BOOL bSynchron = FALSE );

    virtual BOOL Connect( ::binfilter::SvBaseLink* );

    // erfrage ob das man direkt auf die Daten zugreifen kann oder ob das
    // erst angestossen werden muss

    void CancelTransfers();
};


}//end of namespace binfilter
#endif

