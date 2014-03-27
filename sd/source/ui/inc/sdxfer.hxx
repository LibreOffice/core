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

#ifndef INCLUDED_SD_SOURCE_UI_INC_SDXFER_HXX
#define INCLUDED_SD_SOURCE_UI_INC_SDXFER_HXX

#include <svtools/transfer.hxx>
#include <vcl/graph.hxx>
#include <sfx2/objsh.hxx>
#include <svl/lstner.hxx>


// SdTransferable
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

    void                            SetPageBookmarks( const std::vector<OUString>& rPageBookmarks, sal_Bool bPersistent );
    sal_Bool                            IsPageTransferable() const { return mbPageTransferable; }
    sal_Bool                            HasPageBookmarks() const { return( mpPageDocShell && ( !maPageBookmarks.empty() ) ); }
    const std::vector<OUString>& GetPageBookmarks() const { return maPageBookmarks; }
    ::sd::DrawDocShell*                 GetPageDocShell() const { return mpPageDocShell; }

    sal_Bool                        SetTableRTF( SdDrawDocument*, const ::com::sun::star::datatransfer::DataFlavor& );

    static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();
    static SdTransferable*          getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxData ) throw();

    // SfxListener
    virtual void            Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) SAL_OVERRIDE;

    virtual void                    DragFinished( sal_Int8 nDropAction ) SAL_OVERRIDE;
    SdDrawDocument*                 GetSourceDoc (void) const;

    /** User data objects can be used to store information temporarily
        at the transferable.  The slide sorter uses this to store
        previews of the slides that are referenced by the
        transferable.
    */
    class UserData {public:virtual~UserData(){}};

    /** Add a user data object.  When it was added before (and not
        removed) then this call is ignored.
    */
    void AddUserData (const ::boost::shared_ptr<UserData>& rpData);

    /** Return the number of user data objects.
    */
    sal_Int32 GetUserDataCount (void) const;

    /** Return the specified user data object.  When the index is not
        valid, ie not in the range [0,count) then an empty pointer is
        returned.
    */
    ::boost::shared_ptr<UserData> GetUserData (const sal_Int32 nIndex) const;

protected:

    virtual void                    AddSupportedFormats() SAL_OVERRIDE;
    virtual bool                    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor ) SAL_OVERRIDE;
    virtual bool                    WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& rFlavor ) SAL_OVERRIDE;
    virtual void                    ObjectReleased() SAL_OVERRIDE;

    virtual sal_Int64 SAL_CALL      getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( ::com::sun::star::uno::RuntimeException, std::exception ) SAL_OVERRIDE;

private:

    SfxObjectShellRef               maDocShellRef;
    ::sd::DrawDocShell*             mpPageDocShell;
    std::vector<OUString>      maPageBookmarks;
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
    ::std::vector<boost::shared_ptr<UserData> > maUserData;

                                    // not available
                                    SdTransferable();
                                    SdTransferable( const SdTransferable& );
    SdTransferable&                 operator=( const SdTransferable& );

    void                            CreateObjectReplacement( SdrObject* pObj );
    void                            CreateData();

};

#endif // INCLUDED_SD_SOURCE_UI_INC_SDXFER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
