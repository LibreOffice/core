/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */


#include <unotools/pathoptions.hxx>
#include <rtl/instance.hxx>
#include <sfx2/viewfrm.hxx>
#include "svx/gallery1.hxx"
#include "svx/galtheme.hxx"
#include "svx/galbrws.hxx"
#include "svx/gallery.hxx"
#include "galobj.hxx"

namespace
{
    class theLockListener : public rtl::Static< SfxListener, theLockListener > {};
}

// -------------------
// - GalleryExplorer -
// -------------------

bool GalleryExplorer::FillThemeList( std::vector<String>& rThemeList )
{
    Gallery* pGal = ::Gallery::GetGalleryInstance();

    if( pGal )
    {
        for( sal_uIntPtr i = 0, nCount = pGal->GetThemeCount(); i < nCount; i++ )
        {
            const GalleryThemeEntry* pEntry = pGal->GetThemeInfo( i );

            if( pEntry && !pEntry->IsReadOnly() && !pEntry->IsHidden() )
                rThemeList.push_back(pEntry->GetThemeName());
        }
    }

    return !rThemeList.empty();
}

// ------------------------------------------------------------------------

sal_Bool GalleryExplorer::FillObjList( const OUString& rThemeName, std::vector<String> &rObjList )
{
    Gallery* pGal = ::Gallery::GetGalleryInstance();

    if( pGal )
    {
        SfxListener     aListener;
        GalleryTheme*   pTheme = pGal->AcquireTheme( rThemeName, aListener );

        if( pTheme )
        {
            for( sal_uInt32 i = 0, nCount = pTheme->GetObjectCount(); i < nCount; i++ )
                rObjList.push_back( pTheme->GetObjectURL( i ).GetMainURL( INetURLObject::NO_DECODE ) );

            pGal->ReleaseTheme( pTheme, aListener );
        }
    }

    return !rObjList.empty();
}

// ------------------------------------------------------------------------

sal_Bool GalleryExplorer::FillObjList( const sal_uInt32 nThemeId, std::vector<String> &rObjList )
{
    Gallery* pGal = ::Gallery::GetGalleryInstance();

    if (!pGal)
        return false;

    return FillObjList( pGal->GetThemeName( nThemeId ), rObjList );
}

bool GalleryExplorer::FillObjList( const sal_uInt32 nThemeId, std::vector<OUString> &rObjList )
{
    std::vector<String> aObjList;
    if (!FillObjList(nThemeId, aObjList))
        return false;

    std::vector<OUString> aList;
    aList.reserve(aObjList.size());
    std::vector<String>::const_iterator it = aObjList.begin(), itEnd = aObjList.end();
    for (; it != itEnd; ++it)
        aList.push_back(*it);

    rObjList.swap(aList);
    return true;
}

// ------------------------------------------------------------------------

sal_Bool GalleryExplorer::FillObjListTitle( const sal_uInt32 nThemeId, std::vector< OUString >& rList )
{
    Gallery* pGal = ::Gallery::GetGalleryInstance();
    if( pGal )
    {
        SfxListener     aListener;
        GalleryTheme*   pTheme = pGal->AcquireTheme( pGal->GetThemeName( nThemeId ), aListener );

        if( pTheme )
        {
            for( sal_uIntPtr i = 0, nCount = pTheme->GetObjectCount(); i < nCount; i++ )
            {
                SgaObject*  pObj = pTheme->AcquireObject( i );
                if ( pObj )
                {
                    OUString aTitle( pObj->GetTitle() );
                    rList.push_back( aTitle );
                    pTheme->ReleaseObject( pObj );
                }
            }
            pGal->ReleaseTheme( pTheme, aListener );
        }
    }
    return !rList.empty();
}

// ------------------------------------------------------------------------

sal_Bool GalleryExplorer::InsertURL( const OUString& rThemeName, const OUString& rURL )
{
    Gallery*    pGal = ::Gallery::GetGalleryInstance();
    sal_Bool        bRet = sal_False;

    if( pGal )
    {
        SfxListener   aListener;
        GalleryTheme* pTheme = pGal->AcquireTheme( rThemeName, aListener );

        if( pTheme )
        {
            INetURLObject aURL( rURL );
            DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
            bRet = pTheme->InsertURL( aURL );
            pGal->ReleaseTheme( pTheme, aListener );
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

sal_Bool GalleryExplorer::InsertURL( sal_uIntPtr nThemeId, const OUString& rURL )
{
    Gallery* pGal = ::Gallery::GetGalleryInstance();
    return( pGal ? InsertURL( pGal->GetThemeName( nThemeId ), rURL ) : sal_False );
}

// ------------------------------------------------------------------------

sal_Bool GalleryExplorer::GetGraphicObj( const OUString& rThemeName, sal_uIntPtr nPos,
                                     Graphic* pGraphic, BitmapEx* pThumb,
                                     sal_Bool bProgress )
{
    Gallery*    pGal = ::Gallery::GetGalleryInstance();
    sal_Bool        bRet = sal_False;

    if( pGal )
    {
        SfxListener     aListener;
        GalleryTheme*   pTheme = pGal->AcquireTheme( rThemeName, aListener );

        if( pTheme )
        {
            if( pGraphic )
                bRet = bRet || pTheme->GetGraphic( nPos, *pGraphic, bProgress );

            if( pThumb )
                bRet = bRet || pTheme->GetThumb( nPos, *pThumb, bProgress );

            pGal->ReleaseTheme( pTheme, aListener );
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

sal_Bool GalleryExplorer::GetGraphicObj( sal_uIntPtr nThemeId, sal_uIntPtr nPos,
                                     Graphic* pGraphic, BitmapEx* pThumb,
                                     sal_Bool bProgress )
{
    Gallery* pGal = ::Gallery::GetGalleryInstance();
    return( pGal ? GetGraphicObj( pGal->GetThemeName( nThemeId ), nPos, pGraphic, pThumb, bProgress ) : sal_False );
}

// ------------------------------------------------------------------------

sal_uIntPtr GalleryExplorer::GetSdrObjCount( const OUString& rThemeName )
{
    Gallery*    pGal = ::Gallery::GetGalleryInstance();
    sal_uIntPtr     nRet = 0;

    if( pGal )
    {
        SfxListener     aListener;
        GalleryTheme*   pTheme = pGal->AcquireTheme( rThemeName, aListener );

        if( pTheme )
        {
            for( sal_uIntPtr i = 0, nCount = pTheme->GetObjectCount(); i < nCount; i++ )
                if( SGA_OBJ_SVDRAW == pTheme->GetObjectKind( i ) )
                    nRet++;

            pGal->ReleaseTheme( pTheme, aListener );
        }
    }

    return nRet;
}

// ------------------------------------------------------------------------

sal_uIntPtr GalleryExplorer::GetSdrObjCount( sal_uIntPtr nThemeId  )
{
    Gallery* pGal = ::Gallery::GetGalleryInstance();
    return( pGal ? GetSdrObjCount( pGal->GetThemeName( nThemeId ) ) : sal_False );
}

// ------------------------------------------------------------------------

sal_Bool GalleryExplorer::GetSdrObj( const OUString& rThemeName, sal_uIntPtr nSdrModelPos,
                                 SdrModel* pModel, BitmapEx* pThumb )
{
    Gallery*    pGal = ::Gallery::GetGalleryInstance();
    sal_Bool        bRet = sal_False;

    if( pGal )
    {
        SfxListener     aListener;
        GalleryTheme*   pTheme = pGal->AcquireTheme( rThemeName, aListener );

        if( pTheme )
        {
            for( sal_uIntPtr i = 0, nCount = pTheme->GetObjectCount(), nActPos = 0; ( i < nCount ) && !bRet; i++ )
            {
                if( SGA_OBJ_SVDRAW == pTheme->GetObjectKind( i ) )
                {
                    if( nActPos++ == nSdrModelPos )
                    {
                        if( pModel )
                            bRet = bRet || pTheme->GetModel( i, *pModel, sal_False );

                        if( pThumb )
                            bRet = bRet || pTheme->GetThumb( i, *pThumb );
                    }
                }
            }

            pGal->ReleaseTheme( pTheme, aListener );
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

sal_Bool GalleryExplorer::GetSdrObj( sal_uIntPtr nThemeId, sal_uIntPtr nSdrModelPos,
                                 SdrModel* pModel, BitmapEx* pThumb )
{
    Gallery* pGal = ::Gallery::GetGalleryInstance();
    return( pGal ? GetSdrObj( pGal->GetThemeName( nThemeId ), nSdrModelPos, pModel, pThumb ) : sal_False );
}

// -----------------------------------------------------------------------------

sal_Bool GalleryExplorer::BeginLocking( const OUString& rThemeName )
{
    Gallery*    pGal = ::Gallery::GetGalleryInstance();
    sal_Bool        bRet = sal_False;

    if( pGal )
    {
        GalleryTheme* pTheme = pGal->AcquireTheme( rThemeName, theLockListener::get() );

        if( pTheme )
        {
            pTheme->LockTheme();
            bRet = sal_True;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool GalleryExplorer::BeginLocking( sal_uIntPtr nThemeId )
{
    Gallery* pGal = ::Gallery::GetGalleryInstance();
    return( pGal ? BeginLocking( pGal->GetThemeName( nThemeId ) ) : sal_False );
}

// -----------------------------------------------------------------------------

sal_Bool GalleryExplorer::EndLocking( const OUString& rThemeName )
{
    Gallery*    pGal = ::Gallery::GetGalleryInstance();
    sal_Bool        bRet = sal_False;

    if( pGal )
    {
        SfxListener   aListener;
        GalleryTheme* pTheme = pGal->AcquireTheme( rThemeName, aListener );

        if( pTheme )
        {
            const sal_Bool bReleaseLockedTheme = pTheme->UnlockTheme();

            // release acquired theme
            pGal->ReleaseTheme( pTheme, aListener );

            if( bReleaseLockedTheme )
            {
                // release locked theme
                pGal->ReleaseTheme( pTheme, theLockListener::get() );
                bRet = sal_True;
            }
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

sal_Bool GalleryExplorer::EndLocking( sal_uIntPtr nThemeId )
{
    Gallery* pGal = ::Gallery::GetGalleryInstance();
    return( pGal ? EndLocking( pGal->GetThemeName( nThemeId ) ) : sal_False );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
