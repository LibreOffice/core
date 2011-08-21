/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _SD_SDXFER_HXX
#define _SD_SDXFER_HXX

#include <svtools/transfer.hxx>
#include <vcl/graph.hxx>
#include <sfx2/objsh.hxx>
#include <svl/lstner.hxx>

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

                                    SdTransferable( SdDrawDocument* pSrcDoc, ::sd::View* pWorkView, sal_Bool bInitOnGetData );
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

    void                            SetInternalMove( sal_Bool bSet ) { mbInternalMove = bSet; }
    sal_Bool                            IsInternalMove() const { return mbInternalMove; }

    sal_Bool                            HasSourceDoc( const SdDrawDocument* pDoc ) const { return( mpSourceDoc == pDoc ); }

    void                            SetPageBookmarks( const List& rPageBookmarks, sal_Bool bPersistent );
    sal_Bool                            IsPageTransferable() const { return mbPageTransferable; }
    sal_Bool                            HasPageBookmarks() const { return( mpPageDocShell && ( maPageBookmarks.Count() > 0 ) ); }
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
    sal_Bool                            mbInternalMove               : 1;
    sal_Bool                            mbOwnDocument                : 1;
    sal_Bool                            mbOwnView                    : 1;
    sal_Bool                            mbLateInit                   : 1;
    sal_Bool                            mbPageTransferable           : 1;
    sal_Bool                            mbPageTransferablePersistent : 1;
    bool                            mbIsUnoObj                  : 1;

                                    // not available
                                    SdTransferable();
                                    SdTransferable( const SdTransferable& );
    SdTransferable&                 operator=( const SdTransferable& );

    void                            CreateObjectReplacement( SdrObject* pObj );
    void                            CreateData();

};

#endif // _SD_SDXFER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
