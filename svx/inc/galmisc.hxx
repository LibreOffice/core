/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: galmisc.hxx,v $
 *
 *  $Revision: 1.15 $
 *
 *  last change: $Author: ihi $ $Date: 2006-11-14 12:38:01 $
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

#ifndef _SVX_GALMISC_HXX_
#define _SVX_GALMISC_HXX_

#include <sot/storage.hxx>
#include <tools/urlobj.hxx>
#include <svtools/imap.hxx>
#include <svtools/hint.hxx>
#include <svtools/transfer.hxx>
#include "svdobj.hxx"
#include "galobj.hxx"

#ifndef _COM_SUN_STAR_UNO_REFERENCE_H_
#include <com/sun/star/uno/Reference.h>
#endif
#ifndef _COM_SUN_STAR_AWT_XPROGRESSMONITOR_HPP
#include <com/sun/star/awt/XProgressMonitor.hpp>
#endif

#ifndef INCLUDED_SVXDLLAPI_H
#include "svx/svxdllapi.h"
#endif

// -----------
// - Defines -
// -----------

#define IV_IMAPINFO             (UINT32('S')*0x00000001+UINT32('D')*0x00000100+UINT32('U')*0x00010000+UINT32('D')*0x01000000)
#define ID_IMAPINFO             2

#define USERDATA_HDL()          (LINK(this,SgaUserDataFactory,MakeUserData))

#define GAL_RESID( nId )        ResId( nId, GetGalleryResMgr() )
#define STREAMBUF_SIZE          16384L

#define SGA_IMPORT_NONE         0x0000
#define SGA_IMPORT_FILE         0x0001
#define SGA_IMPORT_INET         0x0002

#define GALLERY_PROGRESS_RANGE  10000

#define GALLERY_FG_COLOR        Application::GetSettings().GetStyleSettings().GetWindowTextColor()
#define GALLERY_BG_COLOR        Application::GetSettings().GetStyleSettings().GetWindowColor()
#define GALLERY_DLG_COLOR       Application::GetSettings().GetStyleSettings().GetDialogColor()

// -------------
// - Functions -
// -------------

class ResMgr;
class String;
class SvStream;
class Graphic;
class FmFormModel;
class ImageMap;
class Gallery;

SVX_DLLPUBLIC ResMgr*           GetGalleryResMgr();
USHORT          GalleryGraphicImport( const INetURLObject& rURL, Graphic& rGraphic, String& rFilterName, BOOL bShowProgress = FALSE );
BOOL            GallerySvDrawImport( SvStream& rIStm, FmFormModel& rModel );
BOOL            CreateIMapGraphic( const FmFormModel& rModel, Graphic& rGraphic, ImageMap& rImageMap );
SVX_DLLPUBLIC String            GetReducedString( const INetURLObject& rURL, ULONG nMaxLen );
String          GetSvDrawStreamNameFromURL( const INetURLObject& rSvDrawObjURL );

BOOL            FileExists( const INetURLObject& rURL );
BOOL            CreateDir(  const INetURLObject& rURL );
BOOL            CopyFile(  const INetURLObject& rSrcURL, const INetURLObject& rDstURL );
BOOL            KillFile( const INetURLObject& rURL );
BitmapEx        GalleryResGetBitmapEx( sal_uInt32 nId );


// ---------------
// - SgaIMapInfo -
// ---------------

class SgaIMapInfo : public SdrObjUserData, public SfxListener
{
    ImageMap                aImageMap;

public:
                            SgaIMapInfo() : SdrObjUserData( IV_IMAPINFO, ID_IMAPINFO, 0 ) {};

                            SgaIMapInfo( const ImageMap& rImageMap) :
                                SdrObjUserData( IV_IMAPINFO, ID_IMAPINFO, 0 ),
                                aImageMap( rImageMap ) {};

    virtual                 ~SgaIMapInfo() {};

    virtual SdrObjUserData* Clone( SdrObject* ) const
                            {
                                SgaIMapInfo* pInfo = new SgaIMapInfo;
                                pInfo->aImageMap = aImageMap;
                                return pInfo;
                            }

    const ImageMap&         GetImageMap() const { return aImageMap; }
};

// ----------------------
// - SgaUserDataFactory -
// ----------------------

class SgaUserDataFactory
{
public:
        SgaUserDataFactory() { SdrObjFactory::InsertMakeUserDataHdl( USERDATA_HDL() ); }
        ~SgaUserDataFactory() { SdrObjFactory::RemoveMakeUserDataHdl( USERDATA_HDL() ); }

        DECL_LINK( MakeUserData, SdrObjFactory* );
};

// -------------------
// - GalleryProgress -
// -------------------

class GraphicFilter;

class SVX_DLLPUBLIC GalleryProgress
{
    ::com::sun::star::uno::Reference< ::com::sun::star::awt::XProgressBar > mxProgressBar;
    GraphicFilter*                                                          mpFilter;

    public:

                                    GalleryProgress( GraphicFilter* pFilter = NULL );
                                    ~GalleryProgress();

    void                            Update( ULONG nVal, ULONG nMaxVal );
};

// -----------------------
// - GalleryTransferable -
// -----------------------

class Gallery;
class GalleryTheme;
class GraphicObject;

class GalleryTransferable : public TransferableHelper
{
friend class GalleryTheme;
using TransferableHelper::CopyToClipboard;

private:

    GalleryTheme*                   mpTheme;
    SgaObjKind                      meObjectKind;
    sal_uInt32                      mnObjectPos;
    SotStorageStreamRef             mxModelStream;
    GraphicObject*                  mpGraphicObject;
    ImageMap*                       mpImageMap;
    INetURLObject*                  mpURL;

protected:

                                    GalleryTransferable( GalleryTheme* pTheme, ULONG nObjectPos, bool bLazy );
                                    ~GalleryTransferable();

    void                            InitData( bool bLazy );

    // TransferableHelper
    virtual void                    AddSupportedFormats();
    virtual sal_Bool                GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual sal_Bool                WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual void                    DragFinished( sal_Int8 nDropAction );
    virtual void                    ObjectReleased();

    void                            CopyToClipboard( Window* pWindow );
    void                            StartDrag( Window* pWindow, sal_Int8 nDragSourceActions,
                                               sal_Int32 nDragPointer = DND_POINTER_NONE,
                                               sal_Int32 nDragImage = DND_IMAGE_NONE );
};

// ---------------
// - GalleryHint -
// ---------------

#define GALLERY_HINT_NONE               0x00000000
#define GALLERY_HINT_CLOSE_THEME        0x00000001
#define GALLERY_HINT_THEME_REMOVED      0x00000002
#define GALLERY_HINT_THEME_RENAMED      0x00000004
#define GALLERY_HINT_THEME_CREATED      0x00000008
#define GALLERY_HINT_THEME_UPDATEVIEW   0x00000010
#define GALLERY_HINT_CLOSE_OBJECT       0x00000020
#define GALLERY_HINT_OBJECT_REMOVED     0x00000040

// -----------------------------------------------------------------------------

class GalleryHint : public SfxHint
{
private:

    ULONG           mnType;
    String          maThemeName;
    String          maStringData;
    ULONG           mnData1;
    ULONG           mnData2;

public:

                    GalleryHint( ULONG nType, const String& rThemeName, ULONG nData1 = 0UL, ULONG nData2 = 0UL ) :
                        mnType( nType ), maThemeName( rThemeName ), mnData1( nData1 ), mnData2( nData2 ) {}

                    GalleryHint( ULONG nType, const String& rThemeName, const String& rStringData, ULONG nData1 = 0UL, ULONG nData2 = 0UL ) :
                        mnType( nType ), maThemeName( rThemeName ), maStringData( rStringData ), mnData1( nData1 ), mnData2( nData2 ) {}

    ULONG           GetType() const { return mnType; }
    const String&   GetThemeName() const { return maThemeName; }
    const String&   GetStringData() const { return maStringData; }
    ULONG           GetData1() const { return mnData1; }
    ULONG           GetData2() const { return mnData2; }
};

    #endif
