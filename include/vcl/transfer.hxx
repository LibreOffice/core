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

#ifndef INCLUDED_VCL_TRANSFER_HXX
#define INCLUDED_VCL_TRANSFER_HXX

#include <vcl/dllapi.h>
#include <tools/globname.hxx>
#include <tools/gen.hxx>
#include <tools/solar.h>
#include <sot/formats.hxx>
#include <sot/exchange.hxx>
#include <cppuhelper/implbase.hxx>
#include <com/sun/star/frame/XTerminateListener.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/datatransfer/XTransferable2.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboard.hpp>
#include <com/sun/star/datatransfer/clipboard/XClipboardOwner.hpp>
#include <com/sun/star/datatransfer/dnd/DNDConstants.hpp>
#include <com/sun/star/datatransfer/dnd/XDragGestureListener.hpp>
#include <com/sun/star/datatransfer/dnd/DropTargetDragEvent.hpp>
#include <com/sun/star/datatransfer/dnd/DropTargetDropEvent.hpp>
#include <com/sun/star/datatransfer/dnd/XDragSourceListener.hpp>
#include <com/sun/star/datatransfer/dnd/XDropTargetListener.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <memory>
#include <mutex>
#include <utility>

namespace com::sun::star::datatransfer::dnd { class XDragGestureRecognizer; }
namespace com::sun::star::io { class XInputStream; }
namespace com::sun::star::datatransfer::dnd { class XDropTarget; }

namespace tools { template <typename T> class SvRef; }
template <typename Arg, typename Ret> class Link;

class BitmapEx;
class GDIMetaFile;
class Graphic;
class ImageMap;
class INetBookmark;
class INetImage;
class FileList;
class SotStorageStream;
namespace vcl { class Window; }

// Drag&Drop defines

#define DND_ACTION_NONE     css::datatransfer::dnd::DNDConstants::ACTION_NONE
#define DND_ACTION_COPY     css::datatransfer::dnd::DNDConstants::ACTION_COPY
#define DND_ACTION_MOVE     css::datatransfer::dnd::DNDConstants::ACTION_MOVE
#define DND_ACTION_COPYMOVE css::datatransfer::dnd::DNDConstants::ACTION_COPY_OR_MOVE
#define DND_ACTION_LINK     css::datatransfer::dnd::DNDConstants::ACTION_LINK

#define DND_POINTER_NONE    0
#define DND_IMAGE_NONE      0

struct TransferableObjectDescriptor
{
    SvGlobalName        maClassName;
    sal_uInt16          mnViewAspect;
    Point               maDragStartPos;
    Size                maSize;
    OUString            maTypeName;
    OUString            maDisplayName;

    TransferableObjectDescriptor()
        : mnViewAspect(css::embed::Aspects::MSOLE_CONTENT)
    {}

    VCL_DLLPUBLIC friend SvStream&  WriteTransferableObjectDescriptor( SvStream& rOStm, const TransferableObjectDescriptor& rObjDesc );
};

struct AcceptDropEvent
{
    Point                                                           maPosPixel;
    const css::datatransfer::dnd::DropTargetDragEvent               maDragEvent;
    sal_Int8                                                        mnAction;
    bool                                                            mbLeaving;
    bool                                                            mbDefault;

    AcceptDropEvent() :
        mnAction( DND_ACTION_NONE ),
        mbLeaving( false ),
        mbDefault( false ) {}

    AcceptDropEvent( sal_Int8 nAction,
                     const Point& rPosPixel,
                     css::datatransfer::dnd::DropTargetDragEvent aDragEvent ) :
        maPosPixel( rPosPixel ),
        maDragEvent(std::move( aDragEvent )),
        mnAction( nAction ),
        mbLeaving( false ),
        mbDefault( false ) {}
};

struct ExecuteDropEvent
{
    Point                                                           maPosPixel;
    const css::datatransfer::dnd::DropTargetDropEvent               maDropEvent;
    sal_Int8                                                        mnAction;
    bool                                                            mbDefault;

    ExecuteDropEvent( sal_Int8 nAction,
                      const Point& rPosPixel,
                      css::datatransfer::dnd::DropTargetDropEvent aDropEvent ) :
        maPosPixel( rPosPixel ),
        maDropEvent(std::move( aDropEvent )),
        mnAction( nAction ),
        mbDefault( false ) {}
};

class VCL_DLLPUBLIC TransferableHelper : public cppu::WeakImplHelper< css::datatransfer::XTransferable2,
                                                           css::datatransfer::clipboard::XClipboardOwner,
                                                           css::datatransfer::dnd::XDragSourceListener>
{
private:

    // nested class to implement the XTerminateListener interface
    class SAL_DLLPRIVATE TerminateListener final : public cppu::WeakImplHelper< css::frame::XTerminateListener, css::lang::XServiceInfo >
    {
    private:

        TransferableHelper& mrParent;

    private:

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // XTerminateListener
        virtual void SAL_CALL queryTermination( const css::lang::EventObject& aEvent ) override;
        virtual void SAL_CALL notifyTermination( const css::lang::EventObject& aEvent ) override;

        // XServiceInfo
        virtual OUString SAL_CALL getImplementationName() override;
        virtual sal_Bool SAL_CALL supportsService( const OUString& sServiceName ) override;
        virtual css::uno::Sequence< OUString > SAL_CALL getSupportedServiceNames() override;

    public:

                              TerminateListener( TransferableHelper& rDropTargetHelper );
        virtual               ~TerminateListener() override;
    };

    friend class TransferableHelper::TerminateListener;

private:

    css::uno::Any                                                             maAny;
    OUString                                                                  maLastFormat;
    mutable css::uno::Reference< css::datatransfer::clipboard::XClipboard >   mxClipboard;
    css::uno::Reference< css::frame::XTerminateListener >                     mxTerminateListener;
    DataFlavorExVector                                                        maFormats;
    std::unique_ptr<TransferableObjectDescriptor>                             mxObjDesc;

protected:
    ~TransferableHelper();

    const css::uno::Reference< css::datatransfer::clipboard::XClipboard >&
        getOwnClipboard() const { return mxClipboard; }

public:

    // XTransferable
    virtual css::uno::Any SAL_CALL getTransferData( const css::datatransfer::DataFlavor& rFlavor ) override;
    virtual css::uno::Sequence< css::datatransfer::DataFlavor > SAL_CALL getTransferDataFlavors() override;
    virtual sal_Bool SAL_CALL isDataFlavorSupported( const css::datatransfer::DataFlavor& rFlavor ) override;

    // Transferable2
    virtual css::uno::Any SAL_CALL getTransferData2(
        const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
    virtual sal_Bool SAL_CALL isComplex() override;

private:

    // XEventListener
    virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

    // XDragSourceListener
    virtual void SAL_CALL dragDropEnd( const css::datatransfer::dnd::DragSourceDropEvent& dsde ) override;
    virtual void SAL_CALL dragEnter( const css::datatransfer::dnd::DragSourceDragEvent& dsde ) override;
    virtual void SAL_CALL dragExit( const css::datatransfer::dnd::DragSourceEvent& dse ) override;
    virtual void SAL_CALL dragOver( const css::datatransfer::dnd::DragSourceDragEvent& dsde ) override;
    virtual void SAL_CALL dropActionChanged( const css::datatransfer::dnd::DragSourceDragEvent& dsde ) override;

    // XClipboardOwner
    virtual void SAL_CALL lostOwnership( const css::uno::Reference< css::datatransfer::clipboard::XClipboard >& xClipboard, const css::uno::Reference< css::datatransfer::XTransferable >& xTrans ) override;

protected:
    // derivees need to access lostOwnership in case hey override it
    // on windows, changing the access rights to a method gives unresolved externals, so we introduce
    // this impl-method here 'til the next incompatible update
    void    implCallOwnLostOwnership(
                        const css::uno::Reference< css::datatransfer::clipboard::XClipboard >& _rxClipboard,
                        const css::uno::Reference< css::datatransfer::XTransferable >& _rxTrans
                    )
            {
                TransferableHelper::lostOwnership( _rxClipboard, _rxTrans );
            }


private:

    SAL_DLLPRIVATE void             ImplFlush();

protected:

    void                AddFormat( SotClipboardFormatId nFormat );
    void                AddFormat( const css::datatransfer::DataFlavor& rFlavor );
    void                RemoveFormat( SotClipboardFormatId nFormat );
    void                RemoveFormat( const css::datatransfer::DataFlavor& rFlavor );
    bool                HasFormat( SotClipboardFormatId nFormat );
    void                ClearFormats();

    bool                SetAny( const css::uno::Any& rAny );
    bool                SetString( const OUString& rString );
    bool                SetBitmapEx( const BitmapEx& rBitmap, const css::datatransfer::DataFlavor& rFlavor );
    bool                SetGDIMetaFile( const GDIMetaFile& rMtf );
    bool                SetGraphic( const Graphic& rGraphic );
    bool                SetImageMap( const ImageMap& rIMap );
    bool                SetTransferableObjectDescriptor( const TransferableObjectDescriptor& rDesc );
    bool                SetINetBookmark( const INetBookmark& rBmk, const css::datatransfer::DataFlavor& rFlavor );
    bool                SetINetImage( const INetImage& rINtImg, const css::datatransfer::DataFlavor& rFlavor );
    bool                SetObject( void* pUserObject, sal_uInt32 nUserObjectId, const css::datatransfer::DataFlavor& rFlavor );

protected:

    virtual void        AddSupportedFormats() = 0;
    virtual bool        GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) = 0;
    virtual bool        WriteObject( SvStream& rOStm, void* pUserObject, sal_uInt32 nUserObjectId, const css::datatransfer::DataFlavor& rFlavor );
    virtual void        DragFinished( sal_Int8 nDropAction );
    virtual void        ObjectReleased();

    void                CopyToSelection(const css::uno::Reference<css::datatransfer::clipboard::XClipboard> &rClipboard) const;
public:

    void                PrepareOLE( const TransferableObjectDescriptor& rObjDesc );

    void                CopyToClipboard(const css::uno::Reference<css::datatransfer::clipboard::XClipboard> &rClipboard) const;

    // convenience versions of the above which extract the XClipboard from the pWindow
    void                CopyToClipboard( vcl::Window *pWindow ) const;
    void                CopyToPrimarySelection() const;

    void                StartDrag( vcl::Window* pWindow, sal_Int8 nDragSourceActions );

    static void         ClearPrimarySelection();
};

struct TransferableDataHelper_Impl;

class VCL_DLLPUBLIC TransferableDataHelper final
{
    friend class DropTargetHelper;

    css::uno::Reference< css::datatransfer::XTransferable >           mxTransfer;
    css::uno::Reference< css::datatransfer::clipboard::XClipboard >   mxClipboard;
    DataFlavorExVector                                                maFormats;
    std::unique_ptr<TransferableObjectDescriptor>                     mxObjDesc;
    std::unique_ptr<TransferableDataHelper_Impl>                      mxImpl;

    void                        InitFormats();

public:

    static void                 FillDataFlavorExVector( const css::uno::Sequence< css::datatransfer::DataFlavor >& rDataFlavorSeq,
                                                        DataFlavorExVector& rDataFlavorExVector );

                                TransferableDataHelper();
                                TransferableDataHelper( const TransferableDataHelper& rDataHelper );
                                TransferableDataHelper( TransferableDataHelper&& rDataHelper ) noexcept;
                                TransferableDataHelper( const css::uno::Reference< css::datatransfer::XTransferable >& rxTransferable );
                                ~TransferableDataHelper();

    TransferableDataHelper&     operator=( const TransferableDataHelper& rDataHelper );
    TransferableDataHelper&     operator=( TransferableDataHelper&& rDataHelper );

    const css::uno::Reference< css::datatransfer::XTransferable >&    GetTransferable() const { return mxTransfer; }
    css::uno::Reference< css::datatransfer::XTransferable >           GetXTransferable() const;

    bool                        HasFormat( SotClipboardFormatId nFormat ) const;
    bool                        HasFormat( const css::datatransfer::DataFlavor& rFlavor ) const;

    sal_uInt32                  GetFormatCount() const;

    SotClipboardFormatId           GetFormat( sal_uInt32 nFormat ) const;
    css::datatransfer::DataFlavor  GetFormatDataFlavor( sal_uInt32 nFormat ) const;

    const DataFlavorExVector&   GetDataFlavorExVector() const {return maFormats; }

    bool                        StartClipboardListening( );
    void                        StopClipboardListening( );

    void                        Rebind( const css::uno::Reference< css::datatransfer::XTransferable >& _rxNewData );

    css::uno::Any GetAny( SotClipboardFormatId nFormat, const OUString& rDestDoc ) const;
    css::uno::Any GetAny( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) const;

    bool                        GetString( SotClipboardFormatId nFormat, OUString& rStr ) const;
    bool                        GetString( const css::datatransfer::DataFlavor& rFlavor, OUString& rStr ) const;

    bool                        GetBitmapEx( SotClipboardFormatId nFormat, BitmapEx& rBmp ) const;
    bool                        GetBitmapEx( const css::datatransfer::DataFlavor& rFlavor, BitmapEx& rBmp ) const;

    /** Return as GDI metafile.

        @param nMaxAction Allows you to limit the amount of actions; defaults to 0 which means no limit.

        When you eg. Ctrl+a in Excel, you can get the entire sheet as
        metafile, with over 3 million (!) actions; which is just too large for
        any reasonable handling - and you need to set a limit.
    */
    bool                        GetGDIMetaFile( SotClipboardFormatId nFormat, GDIMetaFile& rMtf, size_t nMaxActions = 0 ) const;
    bool                        GetGDIMetaFile( const css::datatransfer::DataFlavor& rFlavor, GDIMetaFile& rMtf ) const;

    bool                        GetGraphic( SotClipboardFormatId nFormat, Graphic& rGraphic ) const;
    bool                        GetGraphic( const css::datatransfer::DataFlavor& rFlavor, Graphic& rGraphic ) const;

    bool                        GetImageMap( SotClipboardFormatId nFormat, ImageMap& rIMap ) const;
    bool                        GetImageMap( const css::datatransfer::DataFlavor& rFlavor, ImageMap& rImap ) const;

    bool                        GetTransferableObjectDescriptor( SotClipboardFormatId nFormat, TransferableObjectDescriptor& rDesc ) const;
    bool                        GetTransferableObjectDescriptor( TransferableObjectDescriptor& rDesc ) const;

    bool                        GetINetBookmark( SotClipboardFormatId nFormat, INetBookmark& rBmk ) const;
    bool                        GetINetBookmark( const css::datatransfer::DataFlavor& rFlavor, INetBookmark& rBmk ) const;

    bool                        GetINetImage( SotClipboardFormatId nFormat, INetImage& rINtImg ) const;
    bool                        GetINetImage( const css::datatransfer::DataFlavor& rFlavor, INetImage& rINtImg ) const;

    bool                        GetFileList( SotClipboardFormatId nFormat, FileList& rFileList ) const;
    bool                        GetFileList( FileList& rFileList ) const;

    css::uno::Sequence<sal_Int8> GetSequence( SotClipboardFormatId nFormat, const OUString& rDestDoc ) const;
    css::uno::Sequence<sal_Int8> GetSequence( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) const;

    std::unique_ptr<SvStream>    GetSotStorageStream( SotClipboardFormatId nFormat ) const;
    std::unique_ptr<SvStream>    GetSotStorageStream( const css::datatransfer::DataFlavor& rFlavor ) const;

    css::uno::Reference<css::io::XInputStream> GetInputStream( SotClipboardFormatId nFormat, const OUString& rDestDoc ) const;
    css::uno::Reference<css::io::XInputStream> GetInputStream( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) const;

    static TransferableDataHelper   CreateFromClipboard(const css::uno::Reference<css::datatransfer::clipboard::XClipboard>& rClipboard);
    static TransferableDataHelper   CreateFromSystemClipboard( vcl::Window * pWindow );
    static TransferableDataHelper   CreateFromPrimarySelection();
    static bool                     IsEqual( const css::datatransfer::DataFlavor& rInternalFlavor,
                                             const css::datatransfer::DataFlavor& rRequestFlavor );
    static bool WriteDDELink(SvStream& stream, std::u16string_view application,
                             std::u16string_view topic, std::u16string_view item,
                             std::u16string_view extra = {});
    bool ReadDDELink(OUString& application, OUString& topic, OUString& item, OUString& rest) const;
};

class VCL_DLLPUBLIC SAL_LOPLUGIN_ANNOTATE("crosscast") DragSourceHelper
{
private:

    // nested class to implement the XDragGestureListener interface
    class SAL_DLLPRIVATE DragGestureListener final : public cppu::WeakImplHelper< css::datatransfer::dnd::XDragGestureListener >
    {
    private:

        DragSourceHelper&   mrParent;

    private:

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // XDragGestureListener
        virtual void SAL_CALL dragGestureRecognized( const css::datatransfer::dnd::DragGestureEvent& rDGE ) override;

    public:

        DragGestureListener( DragSourceHelper& rDragSourceHelper );
        virtual ~DragGestureListener() override;
    };

    friend class DragSourceHelper::DragGestureListener;

private:
    std::mutex                                                            maMutex;
    css::uno::Reference< css::datatransfer::dnd::XDragGestureRecognizer > mxDragGestureRecognizer;

    css::uno::Reference< css::datatransfer::dnd::XDragGestureListener >   mxDragGestureListener;

    DragSourceHelper&   operator=( const DragSourceHelper& rDragSourceHelper ) = delete;
    bool                operator==( const DragSourceHelper& rDragSourceHelper ) const = delete;

public:

                        // to be overridden by the application
    virtual void        StartDrag( sal_Int8 nAction, const Point& rPosPixel );

                        DragSourceHelper( vcl::Window* pWindow );
    void                dispose();
    virtual             ~DragSourceHelper();
};

class VCL_DLLPUBLIC SAL_LOPLUGIN_ANNOTATE("crosscast") DropTargetHelper
{
private:

    // nested class to implement the XDropTargetListener interface
    class SAL_DLLPRIVATE DropTargetListener final : public cppu::WeakImplHelper< css::datatransfer::dnd::XDropTargetListener >
    {
    private:

        DropTargetHelper&                  mrParent;
        std::unique_ptr<AcceptDropEvent>   mpLastDragOverEvent;

        // XEventListener
        virtual void SAL_CALL disposing( const css::lang::EventObject& Source ) override;

        // XDropTargetListener
        virtual void SAL_CALL drop( const css::datatransfer::dnd::DropTargetDropEvent& dtde ) override;
        virtual void SAL_CALL dragEnter( const css::datatransfer::dnd::DropTargetDragEnterEvent& dtdee ) override;
        virtual void SAL_CALL dragExit( const css::datatransfer::dnd::DropTargetEvent& dte ) override;
        virtual void SAL_CALL dragOver( const css::datatransfer::dnd::DropTargetDragEvent& dtde ) override;
        virtual void SAL_CALL dropActionChanged( const css::datatransfer::dnd::DropTargetDragEvent& dtde ) override;

    public:

        DropTargetListener( DropTargetHelper& rDropTargetHelper );
        virtual ~DropTargetListener() override;
    };

    friend class DropTargetHelper::DropTargetListener;

private:
    std::mutex                                                            maMutex;
    css::uno::Reference< css::datatransfer::dnd::XDropTarget >            mxDropTarget;

    css::uno::Reference< css::datatransfer::dnd::XDropTargetListener >    mxDropTargetListener;
    DataFlavorExVector                                                    maFormats;

                        DropTargetHelper() = delete;
    DropTargetHelper&   operator=( const DropTargetHelper& rDropTargetHelper ) = delete;
    bool                operator==( const DropTargetHelper& rDropTargetHelper ) const = delete;

    void                ImplConstruct();

                        // called by our own implementation of XDropTargetListener (DropTargetListener instance)
    SAL_DLLPRIVATE void ImplBeginDrag( const css::uno::Sequence< css::datatransfer::DataFlavor >& rSupportedDataFlavors );
    SAL_DLLPRIVATE void ImplEndDrag();

public:

                        // to be overridden by the application
    SAL_DLLPRIVATE virtual sal_Int8    AcceptDrop( const AcceptDropEvent& rEvt );
    SAL_DLLPRIVATE virtual sal_Int8    ExecuteDrop( const ExecuteDropEvent& rEvt );

                        DropTargetHelper( vcl::Window* pWindow );
                        DropTargetHelper( const css::uno::Reference< css::datatransfer::dnd::XDropTarget >& rxDropTarget );

    void                dispose();
    virtual             ~DropTargetHelper();

                        // typically called by the application in ::AcceptDrop and ::ExecuteDrop and (see above)
    bool                IsDropFormatSupported(SotClipboardFormatId nFormat) const;

    const DataFlavorExVector& GetDataFlavorExVector() const {return maFormats; }

};

struct TransferDataContainer_Impl;

class VCL_DLLPUBLIC TransferDataContainer : public TransferableHelper
{
    std::unique_ptr<TransferDataContainer_Impl> pImpl;

    virtual void        AddSupportedFormats() override;
    virtual bool        GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;

public:

                        TransferDataContainer();
                        virtual ~TransferDataContainer() override;

    void                CopyINetBookmark( const INetBookmark& rBkmk );
    void                CopyString( const OUString& rStr );
    void                CopyString( SotClipboardFormatId nFmt, const OUString& rStr );

    void                CopyByteString( SotClipboardFormatId nFormatId, const OString& rStr );
    void                CopyAnyData( SotClipboardFormatId nFormatId, const char* pData, sal_uLong nLen );

    bool                HasAnyData() const;

    using TransferableHelper::StartDrag;
    void                StartDrag( vcl::Window* pWindow, sal_Int8 nDragSourceActions,
                                   const Link<sal_Int8,void>& rCallback );
    virtual void        DragFinished( sal_Int8 nDropAction ) override;
};

css::uno::Reference<css::datatransfer::clipboard::XClipboard> VCL_DLLPUBLIC GetSystemClipboard();
css::uno::Reference<css::datatransfer::clipboard::XClipboard> VCL_DLLPUBLIC GetSystemPrimarySelection();

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
