/*************************************************************************
 *
 *  $RCSfile: xmleohlp.cxx,v $
 *
 *  $Revision: 1.4 $
 *
 *  last change: $Author: mib $
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

#include <stdio.h>

#ifndef _DEBUG_HXX
#include <tools/debug.hxx>
#endif

#ifndef _SO_CLSIDS_HXX
#include <so3/clsids.hxx>
#endif
#ifndef _PERSIST_HXX
#include <so3/persist.hxx>
#endif
#ifndef _FACTORY_HXX
#include <so3/factory.hxx>
#endif
#ifndef _EMBOBJ_HXX
#include <so3/embobj.hxx>
#endif

#ifndef _XMLEOHLP_HXX
#include "xmleohlp.hxx"
#endif

// -----------
// - Defines -
// -----------

using namespace ::rtl;
using namespace ::osl;
using namespace ::cppu;
using namespace ::com::sun::star::document;
using namespace ::com::sun::star::uno;

#define XML_CONTAINERSTORAGE_NAME       "Objects"
#define XML_EMBEDDEDOBJECT_URL_BASE     "vnd.sun.star.EmbeddedObject:"

#define EO_FLAG_STAR_OBJECT            0x0001
#define EO_FLAG_UCB_STORAGE            0x0002
#define EO_FLAG_NEEDS_CONVERSION       0x0004

struct ClassNameInfo
{
    struct GUIDDummy
    {
        sal_uInt32 n1;
        sal_uInt16 n2, n3;
        sal_uInt8 n4, n5, n6, n7, n8, n9, n10, n11;
    } aGUID;
    sal_uInt16 nFlags;
};

typedef SvGlobalName *SvGlobalNamePtr;

static ClassNameInfo aFlagsMap[] =
{
    { SO3_SM_CLASSID_50, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SCH_CLASSID_50, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SIM_CLASSID_50 , EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SDRAW_CLASSID_50, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SIMPRESS_CLASSID_50, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SC_CLASSID_50, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SW_CLASSID_50, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SWWEB_CLASSID_50, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SWGLOB_CLASSID_50, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },

    { SO3_SM_CLASSID_40, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SCH_CLASSID_40, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SIM_CLASSID_40 , EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SIMPRESS_CLASSID_40, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SC_CLASSID_40, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SW_CLASSID_40, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SWWEB_CLASSID_40, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SWGLOB_CLASSID_40, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },

    { SO3_SM_CLASSID_30, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SCH_CLASSID_30, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SIM_CLASSID_30 , EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SIMPRESS_CLASSID_30, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SC_CLASSID_30, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION },
    { SO3_SW_CLASSID_30, EO_FLAG_STAR_OBJECT|EO_FLAG_NEEDS_CONVERSION }
};

// -----------------------------
// - SvXMLEmbeddedObjectHelper -
// -----------------------------

SvXMLEmbeddedObjectHelper::SvXMLEmbeddedObjectHelper() :
    WeakComponentImplHelper1< XEmbeddedObjectResolver >( maMutex ),
    maDefaultContainerStorageName( RTL_CONSTASCII_USTRINGPARAM(XML_CONTAINERSTORAGE_NAME) ),
    mpRootStorage( 0 ),
    mpDocPersist( 0 ),
    mpGlobalNameMap( 0 ),
    meCreateMode( EMBEDDEDOBJECTHELPER_MODE_READ )
{
}

// -----------------------------------------------------------------------------

SvXMLEmbeddedObjectHelper::~SvXMLEmbeddedObjectHelper()
{
    if( mpGlobalNameMap )
    {
        size_t nCount = sizeof(aFlagsMap)/sizeof(ClassNameInfo);
        SvGlobalNamePtr *p = mpGlobalNameMap;
        while( nCount-- )
            delete *p++;

        delete mpGlobalNameMap;
    }
}

// -----------------------------------------------------------------------------

sal_Bool SvXMLEmbeddedObjectHelper::ImplGetStorageNames(
        const OUString& rURLStr,
        OUString& rContainerStorageName,
        OUString& rObjectStorageName,
        sal_Bool bInternalToExternal ) const
{
    // internal URL: vnd.sun.star.EmbeddedObject:<object-name>
    //           or: vnd.sun.star.EmbeddedObject:<path>/<object-name>
    // external URL: #./<path>/<object-name>
    //           or: #<path>/<object-name>
    //           or: #<object-name>
    // currently, path may only consist of a single directory name
    sal_Bool    bRet = sal_False;

    if( !rURLStr.getLength() )
        return sal_False;

    if( bInternalToExternal )
    {
        sal_Int32 nPos = rURLStr.indexOf( ':' );
        if( -1 == nPos ||
            0 != rURLStr.compareToAscii( XML_EMBEDDEDOBJECT_URL_BASE,
                                 sizeof( XML_EMBEDDEDOBJECT_URL_BASE ) -1 ) )
            return sal_False;

        sal_Int32 nPathStart = nPos + 1;
        nPos = rURLStr.lastIndexOf( '/' );
        if( -1 == nPos )
        {
            rContainerStorageName = OUString();
            rObjectStorageName = rURLStr.copy( nPathStart );
        }
        else if( nPos > nPathStart )
        {
            rContainerStorageName = rURLStr.copy( nPathStart, nPos-nPathStart);
            rObjectStorageName = rURLStr.copy( nPos+1 );
        }
        else
            return sal_False;
    }
    else
    {
        if( '#' != rURLStr[0 ] )
            return sal_False;

        sal_Int32 nPos = rURLStr.lastIndexOf( '/' );
        if( -1 == nPos )
        {
            rContainerStorageName = OUString();
            rObjectStorageName = rURLStr.copy( 1 );
        }
        else
        {
            sal_Int32 nPathStart = 1;
            if( 0 == rURLStr.compareToAscii( "#./", 3 ) )
                nPathStart = 3;
            rContainerStorageName = rURLStr.copy( nPathStart, nPos-nPathStart);
            rObjectStorageName = rURLStr.copy( nPos+1 );
        }
    }

    if( -1 != rContainerStorageName.indexOf( '/' ) )
    {
        DBG_ERROR( "SvXMLEmbeddedObjectHelper: invalid path name" );
        return sal_False;
    }

    return sal_True;
}


// -----------------------------------------------------------------------------

sal_uInt16 SvXMLEmbeddedObjectHelper::ImplGetFlags(
        const SvGlobalName& rClassId ) const
{
    size_t nCount = sizeof(aFlagsMap)/sizeof(ClassNameInfo);

    if( !mpGlobalNameMap )
    {
        ((SvXMLEmbeddedObjectHelper *)this)->mpGlobalNameMap =
            new SvGlobalNamePtr[nCount];
        size_t i = nCount;
        SvGlobalNamePtr *p = (SvGlobalNamePtr *)mpGlobalNameMap;
        while( i-- )
            *p++ = 0;
    }

    sal_uInt16 nFlags = 0;
    SvGlobalNamePtr *pNames = (SvGlobalNamePtr *)mpGlobalNameMap;
    const ClassNameInfo *pInfo = aFlagsMap;
    while( nCount-- )
    {
        if( !*pNames )
            *pNames = new SvGlobalName( pInfo->aGUID.n1, pInfo->aGUID.n2,
                                        pInfo->aGUID.n3, pInfo->aGUID.n4,
                                        pInfo->aGUID.n5, pInfo->aGUID.n6,
                                        pInfo->aGUID.n7, pInfo->aGUID.n8,
                                        pInfo->aGUID.n9, pInfo->aGUID.n10,
                                        pInfo->aGUID.n11 );
        if( **pNames == rClassId )
        {
            nFlags = pInfo->nFlags;
            break;
        }
        pNames++;
        pInfo++;
    }

    return nFlags;
}

SvStorageRef SvXMLEmbeddedObjectHelper::ImplGetContainerStorage(
        const OUString& rStorageName )
{
    DBG_ASSERT( -1 == rStorageName.indexOf( '/' ) &&
                -1 == rStorageName.indexOf( '\\' ),
                "nested embedded storages aren't supported" );
    if( !mxContainerStorage.Is() ||
        ( rStorageName != maCurContainerStorageName ) )
    {
        if( mxContainerStorage.Is() &&
            maCurContainerStorageName.getLength() > 0 &&
            EMBEDDEDOBJECTHELPER_MODE_WRITE == meCreateMode )
            mxContainerStorage->Commit();

        if( rStorageName.getLength() > 0 )
        {
            StreamMode eMode = EMBEDDEDOBJECTHELPER_MODE_WRITE == meCreateMode
                                    ? STREAM_STD_READWRITE
                                    : STREAM_STD_READ;
            mxContainerStorage = mpRootStorage->OpenStorage( rStorageName,
                                                             eMode );
        }
        else
        {
            mxContainerStorage = mpRootStorage;
        }
        maCurContainerStorageName = rStorageName;
    }

    return mxContainerStorage;
}

// -----------------------------------------------------------------------------

SvStorageRef SvXMLEmbeddedObjectHelper::ImplGetObjectStorage(
        const OUString& rContainerStorageName,
        const OUString& rObjectStorageName,
        sal_Bool bUCBStorage )
{
    SvStorageRef xObjStor;
    SvStorageRef xCntnrStor( ImplGetContainerStorage( rContainerStorageName ) );
    if( xCntnrStor.Is() )
    {
        StreamMode eMode = EMBEDDEDOBJECTHELPER_MODE_WRITE == meCreateMode
                                ? STREAM_STD_READWRITE
                                : STREAM_STD_READ;
        xObjStor = xCntnrStor->OpenStorage( rObjectStorageName, eMode );
    }

    return xObjStor;
}

// -----------------------------------------------------------------------------

String SvXMLEmbeddedObjectHelper::ImplGetUniqueName( SvStorage* pStg,
                                              const sal_Char* p ) const
{
    String aName;
    sal_Char cBuf[ 32 ];

    static ULONG nId = (ULONG) cBuf;

    nId++;
    for( ;; )
    {
        sprintf( cBuf, "%s%08lX", p, nId );
        aName.AssignAscii( cBuf );
        if( !pStg->IsContained( aName ) )
            break;
        nId++;
    }
    return aName;
}

sal_Bool SvXMLEmbeddedObjectHelper::ImplReadObject(
        const OUString& rContainerStorageName,
        const OUString& rObjName,
        const SvGlobalName *pClassId )
{
    SvStorageRef xDocStor( mpDocPersist->GetStorage() );
    SvStorageRef xCntnrStor( ImplGetContainerStorage(
                                        rContainerStorageName ) );
    if( !xCntnrStor.Is() )
        return sal_False;

    String aObjName( rObjName );
    String aSrcObjName( aObjName );
    if( xDocStor != xCntnrStor )
    {

        // Is the object name unique?
        if( mpDocPersist->GetObjectList() )
        {
            sal_uInt32 nCount = mpDocPersist->GetObjectList()->Count();
            for( sal_uInt32 i = 0; i < nCount; i++ )
            {
                SvInfoObject* pTst = mpDocPersist->GetObjectList()->GetObject(i);
                // TODO: unicode: is this correct?
                if( aObjName.EqualsIgnoreCaseAscii( pTst->GetObjName() ) ||
                    aObjName.EqualsIgnoreCaseAscii( pTst->GetStorageName() ) )
                {
                    aObjName = ImplGetUniqueName( xDocStor, "Obj" );
                    break;
                }
            }
        }

        if( !xCntnrStor->CopyTo( aSrcObjName, xDocStor, aObjName ) )
            return sal_False;
    }

    SvGlobalName aClassId;
    if( pClassId )
    {
        // If a class id is specifies, use it.
        aClassId = *pClassId;
    }
    else
    {
        // Otherwise try to get one from the storage. For packages, the
        // class id is derived from the package's mime type. The mime type
        // is stored in the packages manifest and the manifest is read when
        // the stoage is opened. Therfor, the class id is available without
        // realy accessing the storage.
        SvStorageRef xObjStor( ImplGetObjectStorage( rContainerStorageName,
                                                     aSrcObjName, sal_False ) );
        aClassId = xObjStor->GetClassName();
    }

    // For all unkown class id, the OLE object has to be wrapped by an
    // outplace object.
    SvGlobalName aOutClassId( SO3_OUT_CLASSID );
    if( SvGlobalName() == aClassId ||
        ( aOutClassId != aClassId &&
          !SvFactory::IsIntern( aClassId, 0 ) ) )
        aClassId = SvGlobalName( aOutClassId );

    SvInfoObjectRef xInfo = new SvEmbeddedInfoObject( aObjName, aClassId );
    mpDocPersist->Insert( xInfo );

    return sal_True;
}

// -----------------------------------------------------------------------------

sal_Bool SvXMLEmbeddedObjectHelper::ImplWriteObject(
        const OUString& rContainerStorageName,
        const OUString& rObjName )
{
    SvInfoObject *pInfo = mpDocPersist->Find( rObjName );
    DBG_ASSERT( pInfo, "OLE object not found" );

    if( !pInfo )
        return sal_False;

    // Get object type
    sal_uInt16 nFlags = ImplGetFlags( pInfo->GetClassName() );

    // If the objects needs a conversion it has to be loaded and stored
    // again later. The same would apply if the save into an old format.
    if( (nFlags & EO_FLAG_NEEDS_CONVERSION) != 0 )
        mpDocPersist->GetObject( rObjName );


    if( pInfo->GetPersist() )
    {
        // The object is loaded either because it is stored within the
        // wrong file format or it has been modified
        // Create either an UCB or an OLE storage, based on object's type
        SvStorageRef xObjStor( ImplGetObjectStorage(
                            rContainerStorageName,
                            rObjName,
                            (nFlags&EO_FLAG_UCB_STORAGE) != 0 ) );
        if( !xObjStor.Is() )
            return sal_False;

        // Save file (again)
        xObjStor->SetVersion( mpRootStorage->GetVersion() );
        if( !pInfo->GetPersist()->DoSaveAs( xObjStor ) )
            return sal_False;

        // Commit changes
        if( !xObjStor->Commit() )
            return sal_False;
    }
    else
    {
        SvStorageRef xCntnrStor( ImplGetContainerStorage(
                                            rContainerStorageName ) );
        if( !xCntnrStor.Is() )
            return sal_False;

        if( !mpDocPersist->GetStorage()->CopyTo( pInfo->GetStorageName(),
                                             xCntnrStor, rObjName ) )
            return sal_False;
    }

    return sal_True;
}

// -----------------------------------------------------------------------------

OUString SvXMLEmbeddedObjectHelper::ImplInsertEmbeddedObjectURL(
        const OUString& rURLStr )
{
    OUString sRetURL;

    OUString    aContainerStorageName, aObjectStorageName;
    if( !ImplGetStorageNames( rURLStr, aContainerStorageName,
                              aObjectStorageName,
                              EMBEDDEDOBJECTHELPER_MODE_WRITE == meCreateMode ) )
        return sRetURL;

    if( EMBEDDEDOBJECTHELPER_MODE_READ == meCreateMode )
    {
        SvGlobalName aClassId, *pClassId = 0;
        sal_Int32 nPos = aObjectStorageName.lastIndexOf( '!' );
        if( -1 != nPos && aClassId.MakeId( aObjectStorageName.copy( nPos+1 ) ) )
        {
            aObjectStorageName = aObjectStorageName.copy( 0, nPos );
            pClassId = &aClassId;
        }
        ImplReadObject( aContainerStorageName, aObjectStorageName, pClassId );
        sRetURL = OUString( RTL_CONSTASCII_USTRINGPARAM(XML_EMBEDDEDOBJECT_URL_BASE     ) );
        sRetURL += aObjectStorageName;
    }
    else
    {
        // Objects are written using SfxObjectShell::SaveAs
//      if( mbDirect )
//          ImplWriteObject( aContainerStorageName, aObjectStorageName );
//      else
//          maEmbeddedObjectURLs.push_back( rURLStr );
        sRetURL = OUString( RTL_CONSTASCII_USTRINGPARAM("#./") );
        if( aContainerStorageName.getLength() )
        {
            sRetURL += aContainerStorageName;
            sRetURL += OUString( '/' );
        }
        sRetURL += aObjectStorageName;
    }

    return sRetURL;
}

// -----------------------------------------------------------------------------

void SvXMLEmbeddedObjectHelper::Init(
        SvStorage& rRootStorage,
        SvPersist& rPersist,
        SvXMLEmbeddedObjectHelperMode eCreateMode,
        sal_Bool bDirect )
{
    mpRootStorage = &rRootStorage;
    mpDocPersist = &rPersist;
    meCreateMode = eCreateMode;
    mbDirect = bDirect;
}

// -----------------------------------------------------------------------------

SvXMLEmbeddedObjectHelper* SvXMLEmbeddedObjectHelper::Create(
        SvStorage& rRootStorage,
        SvPersist& rDocPersist,
        SvXMLEmbeddedObjectHelperMode eCreateMode,
        sal_Bool bDirect )
{
    SvXMLEmbeddedObjectHelper* pThis = new SvXMLEmbeddedObjectHelper;

    pThis->acquire();
    pThis->Init( rRootStorage, rDocPersist, eCreateMode, bDirect );

    return pThis;
}

// -----------------------------------------------------------------------------

void SvXMLEmbeddedObjectHelper::Destroy(
        SvXMLEmbeddedObjectHelper* pSvXMLEmbeddedObjectHelper )
{
    if( pSvXMLEmbeddedObjectHelper )
    {
        pSvXMLEmbeddedObjectHelper->Flush();
        pSvXMLEmbeddedObjectHelper->release();
    }
}

// -----------------------------------------------------------------------------

void SvXMLEmbeddedObjectHelper::Flush()
{
    if( ( EMBEDDEDOBJECTHELPER_MODE_WRITE == meCreateMode ) && !mbDirect )
    {
        OUString    aContainerStorageName, aObjectStorageName;
        URLVector::iterator aIter( maEmbeddedObjectURLs.begin() );
        URLVector::iterator aEnd( maEmbeddedObjectURLs.end() );

        while( aIter != aEnd )
        {
            if( ImplGetStorageNames( *aIter, aContainerStorageName,
                                       aObjectStorageName, sal_True ) )
                ImplWriteObject( aContainerStorageName, aObjectStorageName );
            aIter++;
        }
    }

    if( EMBEDDEDOBJECTHELPER_MODE_WRITE == meCreateMode &&
        mxContainerStorage.Is() )
        mxContainerStorage->Commit();
}

// XGraphicObjectResolver
OUString SAL_CALL SvXMLEmbeddedObjectHelper::resolveEmbeddedObjectURL( const OUString& aURL )
    throw(RuntimeException)
{
    MutexGuard          aGuard( maMutex );

    return ImplInsertEmbeddedObjectURL( aURL );
}
