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

// include ---------------------------------------------------------------

#include <tools/string.hxx>
#include <bf_svtools/svarray.hxx>

#include "cfgitem.hxx"
class SotStorage;
class SvStream;
namespace binfilter {

typedef SfxConfigItem* SfxConfigItemPtr;
SV_DECL_PTRARR( SfxConfigItems_Impl, SfxConfigItemPtr, 2, 2)//STRIP008 ;

struct SfxConfigItem_Impl
{
    SotStorageRef   xStorage;
    String          aName;              // old 5.0 format data
    String          aStreamName;
    SfxConfigItem*	pCItem;
    SfxConfigItems_Impl aItems;
    USHORT          nType;
    BOOL			bDefault;

                    SfxConfigItem_Impl( SfxConfigItem* pConf = NULL )
                        : pCItem( pConf )
                        , nType( pConf ? pConf->GetType() : 0 )
                        , bDefault( TRUE )
                    {}
};

typedef SfxConfigItem_Impl* SfxConfigItemPtr_Impl;
SV_DECL_PTRARR( SfxConfigItemArr_Impl, SfxConfigItemPtr_Impl, 2, 2)//STRIP008 ;

class SfxObjectShell;
class SfxIFConfig_Impl;
class SfxConfigManagerImExport_Impl
{
    SfxConfigItemArr_Impl* 	pItemArr;
    SfxObjectShell*			pObjShell;
    SfxIFConfig_Impl*       pIFaceConfig;

    BOOL            ImportItem( SfxConfigItem_Impl* pItem, SvStream* pStream, SotStorage* pStor );

public:
                    SfxConfigManagerImExport_Impl( SfxObjectShell* pDoc, SfxConfigItemArr_Impl* pArr )
                        : pItemArr( pArr )
                        , pObjShell( pDoc )
                        , pIFaceConfig( NULL )
                    {}

    USHORT          Import( SotStorage* pIn, SotStorage* pStor );
    USHORT          Export( SotStorage* pStor, SotStorage* pOut );

    static String   GetStreamName( USHORT nType );
    static USHORT   GetType( const String& rStreamName );
    static BOOL     HasConfiguration( SotStorage& rStorage );
};

}//end of namespace binfilter
