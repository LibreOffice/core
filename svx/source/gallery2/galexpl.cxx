/*************************************************************************
 *
 *  $RCSfile: galexpl.cxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2000-11-16 12:16:21 $
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

#include <svtools/pathoptions.hxx>
#include <sfx2/viewfrm.hxx>
#include "gallery1.hxx"
#include "galtheme.hxx"
#include "galbrws.hxx"
#include "gallery.hxx"

// -----------
// - Statics -
// -----------

static SfxListener aDummyListener;

// -------------------
// - GalleryExplorer -
// -------------------

Gallery* GalleryExplorer::ImplGetGallery()
{
    static Gallery* pGallery = NULL;

    if( !pGallery )
        pGallery = Gallery::AcquireGallery( SvtPathOptions().GetGalleryPath() );

    return pGallery;
}

// ------------------------------------------------------------------------

GalleryExplorer* GalleryExplorer::GetGallery()
{
    static GalleryExplorer* pThis = NULL;

    // only create a dummy object which can be accessed
    if( !pThis )
        pThis = new GalleryExplorer;

    return pThis;
}

// ------------------------------------------------------------------------

INetURLObject GalleryExplorer::GetURL() const
{
    return GALLERYBROWSER()->GetURL();
}

String GalleryExplorer::GetFilterName() const
{
    return GALLERYBROWSER()->GetFilterName();
}

// ------------------------------------------------------------------------

Graphic GalleryExplorer::GetGraphic() const
{
    return GALLERYBROWSER()->GetGraphic();
}

// ------------------------------------------------------------------------

BOOL GalleryExplorer::GetVCDrawModel( FmFormModel& rModel ) const
{
    return GALLERYBROWSER()->GetVCDrawModel( rModel );
}

// ------------------------------------------------------------------------

BOOL GalleryExplorer::IsLinkage() const
{
    return GALLERYBROWSER()->IsLinkage();
}

// ------------------------------------------------------------------------

BOOL GalleryExplorer::FillThemeList( List& rThemeList )
{
    Gallery* pGal = ImplGetGallery();

    if( pGal )
    {
        for( ULONG i = 0, nCount = pGal->GetThemeCount(); i < nCount; i++ )
        {
            const GalleryThemeEntry* pEntry = pGal->GetThemeInfo( i );

            if( pEntry && !pEntry->IsReadOnly() && !pEntry->IsHidden() )
                rThemeList.Insert( new String( pEntry->GetThemeName() ), LIST_APPEND );
        }
    }

    return( rThemeList.Count() > 0 );
}

// ------------------------------------------------------------------------

BOOL GalleryExplorer::FillObjList( const String& rThemeName, List& rObjList )
{
    Gallery* pGal = ImplGetGallery();

    if( pGal )
    {
        GalleryTheme* pTheme = pGal->AcquireTheme( rThemeName, aDummyListener );

        if( pTheme )
        {
            for( ULONG i = 0, nCount = pTheme->GetObjectCount(); i < nCount; i++ )
                rObjList.Insert( new String( pTheme->GetObjectURL( i ).GetMainURL() ), LIST_APPEND );

            pGal->ReleaseTheme( pTheme, aDummyListener );
        }
    }

    return( rObjList.Count() > 0 );
}

// ------------------------------------------------------------------------

BOOL GalleryExplorer::FillObjList( ULONG nThemeId, List& rObjList )
{
    Gallery* pGal = ImplGetGallery();
    return( pGal ? FillObjList( pGal->GetThemeName( nThemeId ), rObjList ) : FALSE );
}

// ------------------------------------------------------------------------

BOOL GalleryExplorer::InsertURL( const String& rThemeName, const String& rURL )
{
    return InsertURL( rThemeName, rURL, SGA_FORMAT_ALL );
}

// ------------------------------------------------------------------------

BOOL GalleryExplorer::InsertURL( ULONG nThemeId, const String& rURL )
{
    return InsertURL( nThemeId, rURL, SGA_FORMAT_ALL );
}

// ------------------------------------------------------------------------

BOOL GalleryExplorer::InsertURL( const String& rThemeName, const String& rURL, const ULONG nSgaFormat )
{
    Gallery*    pGal = ImplGetGallery();
    BOOL        bRet = FALSE;

    if( pGal )
    {
        GalleryTheme* pTheme = pGal->AcquireTheme( rThemeName, aDummyListener );

        if( pTheme )
        {
            INetURLObject aURL( rURL );
            DBG_ASSERT( aURL.GetProtocol() != INET_PROT_NOT_VALID, "invalid URL" );
            bRet = pTheme->InsertURL( aURL );
            pGal->ReleaseTheme( pTheme, aDummyListener );
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL GalleryExplorer::InsertURL( ULONG nThemeId, const String& rURL, const ULONG nSgaFormat )
{
    Gallery* pGal = ImplGetGallery();
    return( pGal ? InsertURL( pGal->GetThemeName( nThemeId ), rURL, nSgaFormat ) : FALSE );
}

// ------------------------------------------------------------------------

ULONG GalleryExplorer::GetObjCount( const String& rThemeName )
{
    Gallery*    pGal = ImplGetGallery();
    ULONG       nRet = 0;

    if( pGal )
    {
        GalleryTheme* pTheme = pGal->AcquireTheme( rThemeName, aDummyListener );

        if( pTheme )
        {
            nRet = pTheme->GetObjectCount();
            pGal->ReleaseTheme( pTheme, aDummyListener );
        }
    }

    return nRet;
}

// ------------------------------------------------------------------------

ULONG GalleryExplorer::GetObjCount( ULONG nThemeId )
{
    Gallery* pGal = ImplGetGallery();
    return( pGal ? GetObjCount( pGal->GetThemeName( nThemeId ) ) : FALSE );
}

// ------------------------------------------------------------------------

BOOL GalleryExplorer::GetGraphicObj( const String& rThemeName, ULONG nPos,
                                     Graphic* pGraphic, Bitmap* pThumb,
                                     BOOL bProgress )
{
    Gallery*    pGal = ImplGetGallery();
    BOOL        bRet = FALSE;

    if( pGal )
    {
        GalleryTheme* pTheme = pGal->AcquireTheme( rThemeName, aDummyListener );

        if( pTheme )
        {
            if( pGraphic )
                bRet = bRet || pTheme->GetGraphic( nPos, *pGraphic, bProgress );

            if( pThumb )
                bRet = bRet || pTheme->GetThumb( nPos, *pThumb, bProgress );

            pGal->ReleaseTheme( pTheme, aDummyListener );
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL GalleryExplorer::GetGraphicObj( ULONG nThemeId, ULONG nPos,
                                     Graphic* pGraphic, Bitmap* pThumb,
                                     BOOL bProgress )
{
    Gallery* pGal = ImplGetGallery();
    return( pGal ? GetGraphicObj( pGal->GetThemeName( nThemeId ), nPos, pGraphic, pThumb, bProgress ) : FALSE );
}

// ------------------------------------------------------------------------

BOOL GalleryExplorer::InsertGraphicObj( const String& rThemeName, const Graphic& rGraphic )
{
    Gallery*    pGal = ImplGetGallery();
    BOOL        bRet = FALSE;

    if( pGal )
    {
        GalleryTheme* pTheme = pGal->AcquireTheme( rThemeName, aDummyListener );

        if( pTheme )
        {
            bRet = pTheme->InsertGraphic( rGraphic );
            pGal->ReleaseTheme( pTheme, aDummyListener );
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL GalleryExplorer::InsertGraphicObj( ULONG nThemeId, const Graphic& rGraphic )
{
    Gallery* pGal = ImplGetGallery();
    return( pGal ? InsertGraphicObj( pGal->GetThemeName( nThemeId ), rGraphic ) : FALSE );
}

// ------------------------------------------------------------------------

ULONG GalleryExplorer::GetSdrObjCount( const String& rThemeName )
{
    Gallery*    pGal = ImplGetGallery();
    ULONG       nRet = 0;

    if( pGal )
    {
        GalleryTheme* pTheme = pGal->AcquireTheme( rThemeName, aDummyListener );

        if( pTheme )
        {
            for( ULONG i = 0, nCount = pTheme->GetObjectCount(); i < nCount; i++ )
                if( SGA_OBJ_SVDRAW == pTheme->GetObjectKind( i ) )
                    nRet++;

            pGal->ReleaseTheme( pTheme, aDummyListener );
        }
    }

    return nRet;
}

// ------------------------------------------------------------------------

ULONG GalleryExplorer::GetSdrObjCount( ULONG nThemeId  )
{
    Gallery* pGal = ImplGetGallery();
    return( pGal ? GetSdrObjCount( pGal->GetThemeName( nThemeId ) ) : FALSE );
}

// ------------------------------------------------------------------------

BOOL GalleryExplorer::GetSdrObj( const String& rThemeName, ULONG nSdrModelPos,
                                 FmFormModel* pModel, Bitmap* pThumb )
{
    Gallery*    pGal = ImplGetGallery();
    BOOL        bRet = FALSE;

    if( pGal )
    {
        GalleryTheme* pTheme = pGal->AcquireTheme( rThemeName, aDummyListener );

        if( pTheme )
        {
            BOOL bFound = FALSE;

            for( ULONG i = 0, nCount = pTheme->GetObjectCount(), nActPos = 0; ( i < nCount ) && !bRet; i++ )
            {
                if( SGA_OBJ_SVDRAW == pTheme->GetObjectKind( i ) )
                {
                    if( nActPos++ == nSdrModelPos )
                    {
                        if( pModel )
                        {
                            ULONG nStmErr = ERRCODE_NONE;
                            bRet = bRet || pTheme->GetModel( i, *pModel, nStmErr );
                        }

                        if( pThumb )
                            bRet = bRet || pTheme->GetThumb( i, *pThumb );
                    }
                }
            }

            pGal->ReleaseTheme( pTheme, aDummyListener );
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL GalleryExplorer::GetSdrObj( ULONG nThemeId, ULONG nSdrModelPos,
                                 FmFormModel* pModel, Bitmap* pThumb )
{
    Gallery* pGal = ImplGetGallery();
    return( pGal ? GetSdrObj( pGal->GetThemeName( nThemeId ), nSdrModelPos, pModel, pThumb ) : FALSE );
}

// ------------------------------------------------------------------------

BOOL GalleryExplorer::InsertSdrObj( const String& rThemeName, FmFormModel& rModel )
{
    Gallery*    pGal = ImplGetGallery();
    BOOL        bRet = FALSE;

    if( pGal )
    {
        GalleryTheme* pTheme = pGal->AcquireTheme( rThemeName, aDummyListener );

        if( pTheme )
        {
            bRet = pTheme->InsertModel( rModel );
            pGal->ReleaseTheme( pTheme, aDummyListener );
        }
    }

    return bRet;
}

// ------------------------------------------------------------------------

BOOL GalleryExplorer::InsertSdrObj( ULONG nThemeId, FmFormModel& rModel )
{
    Gallery* pGal = ImplGetGallery();
    return( pGal ? InsertSdrObj( pGal->GetThemeName( nThemeId ), rModel ) : FALSE );
}

// -----------------------------------------------------------------------------

BOOL GalleryExplorer::BeginLocking( const String& rThemeName )
{
    Gallery*    pGal = ImplGetGallery();
    BOOL        bRet = FALSE;

    if( pGal )
    {
        GalleryTheme* pTheme = pGal->AcquireTheme( rThemeName, aDummyListener );

        if( pTheme )
            bRet = TRUE;
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryExplorer::BeginLocking( ULONG nThemeId )
{
    Gallery* pGal = ImplGetGallery();
    return( pGal ? BeginLocking( pGal->GetThemeName( nThemeId ) ) : FALSE );
}

// -----------------------------------------------------------------------------

BOOL GalleryExplorer::EndLocking( const String& rThemeName )
{
    Gallery*    pGal = ImplGetGallery();
    BOOL        bRet = FALSE;

    if( pGal )
    {
        GalleryTheme* pTheme = pGal->AcquireTheme( rThemeName, aDummyListener );

        if( pTheme )
        {
            // release twice ( 1. acquired theme, 2. locked theme )
            pGal->ReleaseTheme( pTheme, aDummyListener );
            pGal->ReleaseTheme( pTheme, aDummyListener );
            bRet = TRUE;
        }
    }

    return bRet;
}

// -----------------------------------------------------------------------------

BOOL GalleryExplorer::EndLocking( ULONG nThemeId )
{
    Gallery* pGal = ImplGetGallery();
    return( pGal ? EndLocking( pGal->GetThemeName( nThemeId ) ) : FALSE );
}
