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

#ifndef _TRANSFER_HXX
#define _TRANSFER_HXX

#include "svtools/svtdllapi.h"
#include <tools/globname.hxx>
#include <tools/gen.hxx>
#include <sot/formats.hxx>
#include <cppuhelper/implbase1.hxx>
#include <cppuhelper/implbase4.hxx>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/lang/XUnoTunnel.hpp>
#include <com/sun/star/datatransfer/XTransferable.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/datatransfer/dnd/XDragGestureRecognizer.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTarget.hpp>
#include <com/sun/star/datatransfer/dnd/DropTargetDragEvent.hpp>
#include <com/sun/star/datatransfer/dnd/DropTargetDropEvent.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSourceListener.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/io/XInputStream.hpp>

class BitmapEx;
class GDIMetaFile;
class Graphic;
class ImageMap;
class SotStorageStreamRef;
class INetBookmark;
class INetImage;
class FileList;
class Window;
class Link;

// Drag&Drop defines

#define DND_ACTION_NONE     ::com::sun::star::datatransfer::dnd::DNDConstants::ACTION_NONE
#define DND_ACTION_COPY     ::com::sun::star::datatransfer::dnd::DNDConstants::ACTION_COPY
#define DND_ACTION_MOVE     ::com::sun::star::datatransfer::dnd::DNDConstants::ACTION_MOVE
#define DND_ACTION_COPYMOVE ::com::sun::star::datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE
#define DND_ACTION_LINK     ::com::sun::star::datatransfer::dnd::DNDConstants::ACTION_LINK

#define DND_POINTER_NONE    0
#define DND_IMAGE_NONE      0

struct TransferableObjectDescriptor
{
    SvGlobalName        maClassName;
    sal_uInt16          mnViewAspect;
    Point               maDragStartPos;
    Size                maSize;
    sal_uInt32          mnOle2Misc;
    OUString            maTypeName;
    OUString            maDisplayName;
    sal_Bool            mbCanLink;

    TransferableObjectDescriptor()
        : mnViewAspect(::com::sun::star::embed::Aspects::MSOLE_CONTENT)
        , mnOle2Misc(0)
        , mbCanLink(false)
    {}

    SVT_DLLPUBLIC friend SvStream&  operator>>( SvStream& rIStm, TransferableObjectDescriptor& rObjDesc );
    SVT_DLLPUBLIC friend SvStream&  operator<<( SvStream& rOStm, const TransferableObjectDescriptor& rObjDesc );
};

struct AcceptDropEvent
{
    sal_Int8                                                        mnAction;
    Point                                                           maPosPixel;
    const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent  maDragEvent;
    sal_Bool                                                        mbLeaving;
    sal_Bool                                                        mbDefault;

    AcceptDropEvent() :
        mnAction( DND_ACTION_NONE ),
        mbLeaving( sal_False ),
        mbDefault( sal_False ) {}

    AcceptDropEvent( sal_Int8 nAction,
                     const Point& rPosPixel,
                     const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& rDragEvent,
                     sal_Bool bLeaving = sal_False ) :
        mnAction( nAction ),
        maPosPixel( rPosPixel ),
        maDragEvent( rDragEvent ),
        mbLeaving( bLeaving ),
        mbDefault( sal_False ) {}
};

struct ExecuteDropEvent
{
    sal_Int8                                                        mnAction;
    Point                                                           maPosPixel;
    const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent  maDropEvent;
    sal_Bool                                                        mbDefault;

    ExecuteDropEvent() :
        mnAction( DND_ACTION_NONE ),
        mbDefault( sal_False ) {}

    ExecuteDropEvent( sal_Int8 nAction,
                      const Point& rPosPixel,
                      const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& rDropEvent ) :
        mnAction( nAction ),
        maPosPixel( rPosPixel ),
        maDropEvent( rDropEvent ),
        mbDefault( sal_False ) {}
};

class SVT_DLLPUBLIC TransferableHelper : public ::cppu::WeakImplHelper4< ::com::sun::star::datatransfer::XTransferable,
                                                           ::com::sun::star::datatransfer::clipboard::XClipboardOwner,
                                                           ::com::sun::star::datatransfer::dnd::XDragSourceListener,
                                                           ::com::sun::star::lang::XUnoTunnel >
{
private:

    // nested class to implement the XTerminateListener interface
    class TerminateListener : public ::cppu::WeakImplHelper1< ::com::sun::star::frame::XTerminateListener >
    {
    private:

        TransferableHelper& mrParent;

    private:

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw( ::com::sun::star::uno::RuntimeException );

        // XTerminateListener
        virtual void SAL_CALL queryTermination( const ::com::sun::star::lang::EventObject& aEvent ) throw( ::com::sun::star::frame::TerminationVetoException, ::com::sun::star::uno::RuntimeException );
        virtual void SAL_CALL notifyTermination( const ::com::sun::star::lang::EventObject& aEvent ) throw( ::com::sun::star::uno::RuntimeException );

    public:

                              TerminateListener( TransferableHelper& rDropTargetHelper );
        virtual               ~TerminateListener();
    };

    friend class TransferableHelper::TerminateListener;

private:

    ::com::sun::star::uno::Any                                                                          maAny;
    OUString                                                                                     maLastFormat;
    mutable ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >   mxClipboard;
    ::com::sun::star::uno::Reference< ::com::sun::star::frame::XTerminateListener >                     mxTerminateListener;
    DataFlavorExVector*                                                                                 mpFormats;
    TransferableObjectDescriptor*                                                                       mpObjDesc;

protected:
    inline const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >&
        getOwnClipboard() const { return mxClipboard; }

private:

    // XTransferable
    virtual ::com::sun::star::uno::Any SAL_CALL getTransferData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor ) throw(::com::sun::star::datatransfer::UnsupportedFlavorException, ::com::sun::star::io::IOException, ::com::sun::star::uno::RuntimeException);
    virtual ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors() throw(::com::sun::star::uno::RuntimeException);
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const ::com::sun::star::datatransfer::DataFlavor& rFlavor ) throw(::com::sun::star::uno::RuntimeException);

    // XEventListener
    virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

    // XDragSourceListener
    virtual void SAL_CALL dragDropEnd( const ::com::sun::star::datatransfer::dnd::DragSourceDropEvent& dsde ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL dragEnter( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL dragExit( const ::com::sun::star::datatransfer::dnd::DragSourceEvent& dse ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL dragOver( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw(::com::sun::star::uno::RuntimeException);
    virtual void SAL_CALL dropActionChanged( const ::com::sun::star::datatransfer::dnd::DragSourceDragEvent& dsde ) throw(::com::sun::star::uno::RuntimeException);

private:

    // XClipboardOwner
    virtual void SAL_CALL lostOwnership( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& xClipboard, const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& xTrans ) throw(::com::sun::star::uno::RuntimeException);

protected:
    // derivees need to access lostOwnership in case hey override it
    // on windows, changing the access rights to a method gives unresolved externals, so we introduce
    // this impl-method here 'til the next incompatible update
    inline  void    implCallOwnLostOwnership(
                        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >& _rxClipboard,
                        const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& _rxTrans
                    )
            {
                TransferableHelper::lostOwnership( _rxClipboard, _rxTrans );
            }


private:

    SVT_DLLPRIVATE void             ImplFlush();

protected:

    virtual             ~TransferableHelper();

    void                AddFormat( SotFormatStringId nFormat );
    void                AddFormat( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    void                RemoveFormat( SotFormatStringId nFormat );
    void                RemoveFormat( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    sal_Bool            HasFormat( SotFormatStringId nFormat );
    inline sal_Bool     HasFormat( const ::com::sun::star::datatransfer::DataFlavor& rFlavor ) { return isDataFlavorSupported( rFlavor ); }
    void                ClearFormats();

    sal_Bool            SetAny( const ::com::sun::star::uno::Any& rAny, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    sal_Bool            SetString( const OUString& rString, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    sal_Bool            SetBitmapEx( const BitmapEx& rBitmap, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    sal_Bool            SetGDIMetaFile( const GDIMetaFile& rMtf, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    sal_Bool            SetGraphic( const Graphic& rGraphic, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    sal_Bool            SetImageMap( const ImageMap& rIMap, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    sal_Bool            SetTransferableObjectDescriptor( const TransferableObjectDescriptor& rDesc, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    sal_Bool            SetINetBookmark( const INetBookmark& rBmk, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    sal_Bool            SetINetImage( const INetImage& rINtImg, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    sal_Bool            SetObject( void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );

protected:

    virtual void        AddSupportedFormats() = 0;
    virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor ) = 0;
    virtual sal_Bool    WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual void        DragFinished( sal_Int8 nDropAction );
    virtual void        ObjectReleased();

public:

                        TransferableHelper();

    void                PrepareOLE( const TransferableObjectDescriptor& rObjDesc );

    void                CopyToClipboard( Window *pWindow ) const;
    void                CopyToSelection( Window *pWindow ) const;
    void                StartDrag( Window* pWindow, sal_Int8 nDragSourceActions,
                                   sal_Int32 nDragPointer = DND_POINTER_NONE,
                                   sal_Int32 nDragImage = DND_IMAGE_NONE );

    static void         ClearSelection( Window *pWindow );

    static ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard > GetSystemClipboard();
    static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();

public:

    virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( ::com::sun::star::uno::RuntimeException );
};

struct TransferableDataHelper_Impl;

class SVT_DLLPUBLIC TransferableDataHelper
{
    friend class DropTargetHelper;

private:

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >           mxTransfer;
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::clipboard::XClipboard >   mxClipboard;
    DataFlavorExVector*                                                                         mpFormats;
    TransferableObjectDescriptor*                                                               mpObjDesc;
    TransferableDataHelper_Impl*                                                                mpImpl;

protected:
    void                        InitFormats();

public:

    static void                 FillDataFlavorExVector( const ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor >& rDataFlavorSeq,
                                                        DataFlavorExVector& rDataFlavorExVector );

                                TransferableDataHelper();
                                TransferableDataHelper( const TransferableDataHelper& rDataHelper );
                                TransferableDataHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& rxTransferable );
                                ~TransferableDataHelper();

    TransferableDataHelper&     operator=( const TransferableDataHelper& rDataHelper );

    const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >&    GetTransferable() const { return mxTransfer; }
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >           GetXTransferable() const;

    sal_Bool                    HasFormat( SotFormatStringId nFormat ) const;
    sal_Bool                    HasFormat( const ::com::sun::star::datatransfer::DataFlavor& rFlavor ) const;

    sal_uInt32                  GetFormatCount() const;

    SotFormatStringId                           GetFormat( sal_uInt32 nFormat ) const;
    ::com::sun::star::datatransfer::DataFlavor  GetFormatDataFlavor( sal_uInt32 nFormat ) const;

    DataFlavorExVector&         GetDataFlavorExVector() const {return *mpFormats; }

    sal_Bool                    StartClipboardListening( );
    void                        StopClipboardListening( );

    void                        Rebind( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::XTransferable >& _rxNewData );

public:

    ::com::sun::star::uno::Any  GetAny( SotFormatStringId nFormat ) const;
    ::com::sun::star::uno::Any  GetAny( const ::com::sun::star::datatransfer::DataFlavor& rFlavor ) const;

    sal_Bool                    GetString( SotFormatStringId nFormat, OUString& rStr );
    sal_Bool                    GetString( const ::com::sun::star::datatransfer::DataFlavor& rFlavor, OUString& rStr );

    sal_Bool                    GetBitmapEx( SotFormatStringId nFormat, BitmapEx& rBmp );
    sal_Bool                    GetBitmapEx( const ::com::sun::star::datatransfer::DataFlavor& rFlavor, BitmapEx& rBmp );

    sal_Bool                    GetGDIMetaFile( SotFormatStringId nFormat, GDIMetaFile& rMtf );
    sal_Bool                    GetGDIMetaFile( const ::com::sun::star::datatransfer::DataFlavor& rFlavor, GDIMetaFile& rMtf );

    sal_Bool                    GetGraphic( SotFormatStringId nFormat, Graphic& rGraphic );
    sal_Bool                    GetGraphic( const ::com::sun::star::datatransfer::DataFlavor& rFlavor, Graphic& rGraphic );

    sal_Bool                    GetImageMap( SotFormatStringId nFormat, ImageMap& rIMap );
    sal_Bool                    GetImageMap( const ::com::sun::star::datatransfer::DataFlavor& rFlavor, ImageMap& rImap );

    sal_Bool                    GetTransferableObjectDescriptor( SotFormatStringId nFormat, TransferableObjectDescriptor& rDesc );
    sal_Bool                    GetTransferableObjectDescriptor( const ::com::sun::star::datatransfer::DataFlavor& rFlavor, TransferableObjectDescriptor& rDesc );

    sal_Bool                    GetINetBookmark( SotFormatStringId nFormat, INetBookmark& rBmk );
    sal_Bool                    GetINetBookmark( const ::com::sun::star::datatransfer::DataFlavor& rFlavor, INetBookmark& rBmk );

    sal_Bool                    GetINetImage( SotFormatStringId nFormat, INetImage& rINtImg );
    sal_Bool                    GetINetImage( const ::com::sun::star::datatransfer::DataFlavor& rFlavor, INetImage& rINtImg );

    sal_Bool                    GetFileList( SotFormatStringId nFormat, FileList& rFileList );
    sal_Bool                    GetFileList( const ::com::sun::star::datatransfer::DataFlavor& rFlavor, FileList& rFileList );

    sal_Bool                    GetSequence( SotFormatStringId nFormat, ::com::sun::star::uno::Sequence< sal_Int8 >& rSeq );
    sal_Bool                    GetSequence( const ::com::sun::star::datatransfer::DataFlavor& rFlavor, ::com::sun::star::uno::Sequence< sal_Int8 >& rSeq );

    sal_Bool                    GetSotStorageStream( SotFormatStringId nFormat, SotStorageStreamRef& rStreamRef );
    sal_Bool                    GetSotStorageStream( const ::com::sun::star::datatransfer::DataFlavor& rFlavor, SotStorageStreamRef& rStreamRef );

    sal_Bool                    GetInputStream( SotFormatStringId nFormat, ::com::sun::star::uno::Reference < com::sun::star::io::XInputStream >& xStream );
    sal_Bool                    GetInputStream( const ::com::sun::star::datatransfer::DataFlavor& rFlavor, ::com::sun::star::uno::Reference < com::sun::star::io::XInputStream >& xStream );

    sal_Bool                    GetInterface( const ::com::sun::star::datatransfer::DataFlavor& rFlavor, ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rIf );

public:

    static TransferableDataHelper   CreateFromSystemClipboard( Window * pWindow );
    static TransferableDataHelper   CreateFromSelection( Window * pWindow );
    static sal_Bool                 IsEqual( const ::com::sun::star::datatransfer::DataFlavor& rInternalFlavor,
                                             const ::com::sun::star::datatransfer::DataFlavor& rRequestFlavor,
                                             sal_Bool bCompareParameters = sal_False );
};

class SVT_DLLPUBLIC DragSourceHelper
{
private:

    // nested class to implement the XDragGestureListener interface
    class DragGestureListener : public ::cppu::WeakImplHelper1< ::com::sun::star::datatransfer::dnd::XDragGestureListener >
    {
    private:

        DragSourceHelper&   mrParent;

    private:

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

        // XDragGestureListener
        virtual void SAL_CALL dragGestureRecognized( const ::com::sun::star::datatransfer::dnd::DragGestureEvent& rDGE ) throw( ::com::sun::star::uno::RuntimeException );

    public:

        DragGestureListener( DragSourceHelper& rDragSourceHelper );
        ~DragGestureListener();
    };

    friend class DragSourceHelper::DragGestureListener;

private:

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragGestureRecognizer > mxDragGestureRecognizer;
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDragGestureListener >   mxDragGestureListener;

                        // not available
                        DragSourceHelper();
    DragSourceHelper&   operator=( const DragSourceHelper& rDragSourceHelper );
    sal_Bool            operator==( const DragSourceHelper& rDragSourceHelper ) const;

public:

                        // to be overridden by the application
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );

                        DragSourceHelper( Window* pWindow );
    virtual             ~DragSourceHelper();
};

class SVT_DLLPUBLIC DropTargetHelper
{
private:

    // nested class to implement the XDropTargetListener interface
    class DropTargetListener : public ::cppu::WeakImplHelper1< ::com::sun::star::datatransfer::dnd::XDropTargetListener >
    {
    private:

        DropTargetHelper&   mrParent;
        AcceptDropEvent*    mpLastDragOverEvent;

    private:

        // XEventListener
        virtual void SAL_CALL disposing( const ::com::sun::star::lang::EventObject& Source ) throw(::com::sun::star::uno::RuntimeException);

        // XDropTargetListener
        virtual void SAL_CALL drop( const ::com::sun::star::datatransfer::dnd::DropTargetDropEvent& dtde ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL dragEnter( const ::com::sun::star::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL dragExit( const ::com::sun::star::datatransfer::dnd::DropTargetEvent& dte ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL dragOver( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde ) throw(::com::sun::star::uno::RuntimeException);
        virtual void SAL_CALL dropActionChanged( const ::com::sun::star::datatransfer::dnd::DropTargetDragEvent& dtde ) throw(::com::sun::star::uno::RuntimeException);

    public:

        DropTargetListener( DropTargetHelper& rDropTargetHelper );
        ~DropTargetListener();
    };

    friend class DropTargetHelper::DropTargetListener;

private:

    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTarget >            mxDropTarget;
    ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTargetListener >    mxDropTargetListener;
    DataFlavorExVector*                                                                             mpFormats;

                        // not available
                        DropTargetHelper();
    DropTargetHelper&   operator=( const DropTargetHelper& rDropTargetHelper );
    sal_Bool            operator==( const DropTargetHelper& rDropTargetHelper ) const;

    void                ImplConstruct();

                        // called by our own implementation of XDropTargetListener (DropTargetListener instance)
    void                ImplBeginDrag( const ::com::sun::star::uno::Sequence< ::com::sun::star::datatransfer::DataFlavor >& rSupportedDataFlavors );
    void                ImplEndDrag();

public:

                        // to be overridden by the application
    virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

                        DropTargetHelper( Window* pWindow );
                        DropTargetHelper( const ::com::sun::star::uno::Reference< ::com::sun::star::datatransfer::dnd::XDropTarget >& rxDropTarget );

    virtual             ~DropTargetHelper();

                        // typically called by the application in ::AcceptDrop and ::ExecuteDrop and (see above)
    sal_Bool            IsDropFormatSupported( SotFormatStringId nFormat );
    sal_Bool            IsDropFormatSupported( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );

    DataFlavorExVector& GetDataFlavorExVector() const {return *mpFormats; }

};

struct TransferDataContainer_Impl;

class SVT_DLLPUBLIC TransferDataContainer : public TransferableHelper
{
    TransferDataContainer_Impl* pImpl;

protected:

    virtual void        AddSupportedFormats();
    virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual void        DragFinished( sal_Int8 nDropAction );

public:

                        TransferDataContainer();
                        ~TransferDataContainer();

    void                CopyINetBookmark( const INetBookmark& rBkmk );
    void                CopyINetImage( const INetImage& rINtImg );
    void                CopyImageMap( const ImageMap& rImgMap );
    void                CopyGraphic( const Graphic& rGrf );
    void                CopyString( const OUString& rStr );
    void                CopyString( sal_uInt16 nFmt, const OUString& rStr );
    void                CopyAny( sal_uInt16 nFmt, const ::com::sun::star::uno::Any& rAny );

    void                CopyByteString( sal_uLong nFormatId, const OString& rStr );
    void                CopyAnyData( sal_uLong nFormatId, const sal_Char* pData, sal_uLong nLen );

    sal_Bool            HasAnyData() const;

    using TransferableHelper::StartDrag;
    void                StartDrag( Window* pWindow, sal_Int8 nDragSourceActions,
                                   const Link& rCallbck,
                                   sal_Int32 nDragPointer = DND_POINTER_NONE,
                                   sal_Int32 nDragImage = DND_IMAGE_NONE );
};

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
