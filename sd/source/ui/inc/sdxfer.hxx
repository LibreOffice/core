/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: sdxfer.hxx,v $
 *
 *  $Revision: 1.18 $
 *
 *  last change: $Author: rt $ $Date: 2008-03-12 11:44:50 $
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

#ifndef _SD_SDXFER_HXX
#define _SD_SDXFER_HXX

#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef _SV_GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif
#ifndef _SFX_OBJSH_HXX
#include <sfx2/objsh.hxx>
#endif
#ifndef _SFXLSTNER_HXX
#include <svtools/lstner.hxx>
#endif

// ------------------
// - SdTransferable -
// ------------------

class SdDrawDocument;
class SdrObject;
class INetBookmark;
class ImageMap;
class VirtualDevice;

namespace sd {
class DrawDocShell;
class View;
}

class SfxObjectShellRef;
class SdTransferable : public TransferableHelper, public SfxListener
{
public:

                                    SdTransferable( SdDrawDocument* pSrcDoc, ::sd::View* pWorkView, BOOL bInitOnGetData );
                                    ~SdTransferable();

    void                            SetDocShell( const SfxObjectShellRef& rRef ) { maDocShellRef = rRef; }
    const SfxObjectShellRef&        GetDocShell() const { return maDocShellRef; }

    void                            SetWorkDocument( const SdDrawDocument* pWorkDoc ) { mpSdDrawDocument = mpSdDrawDocumentIntern = (SdDrawDocument*) pWorkDoc; }
    const SdDrawDocument*           GetWorkDocument() const { return mpSdDrawDocument; }

    void                            SetView( const ::sd::View* pView ) { mpSdView = pView; }
    const ::sd::View*                   GetView() const { return mpSdView; }

    void                            SetObjectDescriptor( const TransferableObjectDescriptor& rObjDesc );

    void                            SetStartPos( const Point& rStartPos ) { maStartPos = rStartPos; }
    const Point&                    GetStartPos() const { return maStartPos; }

    void                            SetInternalMove( BOOL bSet ) { mbInternalMove = bSet; }
    BOOL                            IsInternalMove() const { return mbInternalMove; }

    BOOL                            HasSourceDoc( const SdDrawDocument* pDoc ) const { return( mpSourceDoc == pDoc ); }

    void                            SetPageBookmarks( const List& rPageBookmarks, BOOL bPersistent );
    BOOL                            IsPageTransferable() const { return mbPageTransferable; }
    BOOL                            HasPageBookmarks() const { return( mpPageDocShell && ( maPageBookmarks.Count() > 0 ) ); }
    const List&                     GetPageBookmarks() const { return maPageBookmarks; }
    ::sd::DrawDocShell*                 GetPageDocShell() const { return mpPageDocShell; }

    sal_Bool                        SetTableRTF( SdDrawDocument*, const ::com::sun::star::datatransfer::DataFlavor& );

    static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();
    static SdTransferable*          getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxData ) throw();

    // SfxListener
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

protected:

    virtual void                    AddSupportedFormats();
    virtual sal_Bool                GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual sal_Bool                WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual void                    DragFinished( sal_Int8 nDropAction );
    virtual void                    ObjectReleased();

    virtual sal_Int64 SAL_CALL      getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( ::com::sun::star::uno::RuntimeException );

private:

    SfxObjectShellRef               maDocShellRef;
    ::sd::DrawDocShell*             mpPageDocShell;
    List                            maPageBookmarks;
    TransferableDataHelper*         mpOLEDataHelper;
    TransferableObjectDescriptor*   mpObjDesc;
    const ::sd::View*               mpSdView;
    ::sd::View*                     mpSdViewIntern;
    SdDrawDocument*                 mpSdDrawDocument;
    SdDrawDocument*                 mpSdDrawDocumentIntern;
    SdDrawDocument*                 mpSourceDoc;
    VirtualDevice*                  mpVDev;
    INetBookmark*                   mpBookmark;
    Graphic*                        mpGraphic;
    ImageMap*                       mpImageMap;
    Rectangle                       maVisArea;
    Point                           maStartPos;
    BOOL                            mbInternalMove               : 1;
    BOOL                            mbOwnDocument                : 1;
    BOOL                            mbOwnView                    : 1;
    BOOL                            mbLateInit                   : 1;
    BOOL                            mbPageTransferable           : 1;
    BOOL                            mbPageTransferablePersistent : 1;
    bool                            mbIsUnoObj                  : 1;

                                    // not available
                                    SdTransferable();
                                    SdTransferable( const SdTransferable& );
    SdTransferable&                 operator=( const SdTransferable& );

    void                            CreateObjectReplacement( SdrObject* pObj );
    void                            CreateData();

};

#endif // _SD_SDXFER_HXX
