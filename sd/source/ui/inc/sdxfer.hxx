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

#pragma once

#include <vcl/graph.hxx>
#include <vcl/transfer.hxx>
#include <vcl/vclptr.hxx>
#include <sfx2/objsh.hxx>
#include <svl/lstner.hxx>
#include <svl/urlbmk.hxx>

// SdTransferable
class SdDrawDocument;
class SdrObject;
class ImageMap;
class VirtualDevice;

namespace sd {
class DrawDocShell;
class View;
}

class SAL_DLLPUBLIC_RTTI SdTransferable : public TransferDataContainer, public SfxListener
{
public:

                                    SdTransferable( SdDrawDocument* pSrcDoc, ::sd::View* pWorkView, bool bInitOnGetData );
                                    virtual ~SdTransferable() override;

    void                            SetDocShell( const SfxObjectShellRef& rRef ) { maDocShellRef = rRef; }
    const SfxObjectShellRef&        GetDocShell() const { return maDocShellRef; }

    void                            SetWorkDocument( const SdDrawDocument* pWorkDoc ) { mpSdDrawDocument = mpSdDrawDocumentIntern = const_cast<SdDrawDocument*>(pWorkDoc); }
    const SdDrawDocument*           GetWorkDocument() const { return mpSdDrawDocument; }

    void                            SetView(const ::sd::View* pView);
    const ::sd::View*               GetView() const { return mpSdView; }

    void                            SetObjectDescriptor( std::unique_ptr<TransferableObjectDescriptor> pObjDesc );

    void                            SetStartPos( const Point& rStartPos ) { maStartPos = rStartPos; }
    const Point&                    GetStartPos() const { return maStartPos; }

    void                            SetInternalMove( bool bSet ) { mbInternalMove = bSet; }
    bool                            IsInternalMove() const { return mbInternalMove; }

    bool                            HasSourceDoc( const SdDrawDocument* pDoc ) const { return( mpSourceDoc == pDoc ); }

    void                            SetPageBookmarks( std::vector<OUString>&& rPageBookmarks, bool bPersistent );
    bool                            IsPageTransferable() const { return mbPageTransferable; }
    bool                            HasPageBookmarks() const { return( mpPageDocShell && ( !maPageBookmarks.empty() ) ); }
    const std::vector<OUString>&    GetPageBookmarks() const { return maPageBookmarks; }
    ::sd::DrawDocShell*             GetPageDocShell() const { return mpPageDocShell; }

    bool                            SetTableRTF( SdDrawDocument* );

    static SdTransferable*          getImplementation( const css::uno::Reference< css::uno::XInterface >& rxData ) noexcept;

    // SfxListener
    virtual void                    Notify( SfxBroadcaster& rBC, const SfxHint& rHint ) override;

    virtual void                    DragFinished( sal_Int8 nDropAction ) override;
    SdDrawDocument*                 GetSourceDoc() const { return mpSourceDoc;}

    /** User data objects can be used to store information temporarily
        at the transferable.  The slide sorter uses this to store
        previews of the slides that are referenced by the
        transferable.
    */
    class UserData {public:virtual~UserData(){}};

    /** Add a user data object.  When it was added before (and not
        removed) then this call is ignored.
    */
    void AddUserData (const std::shared_ptr<UserData>& rpData);

    /** Return the number of user data objects.
    */
    sal_Int32 GetUserDataCount() const;

    /** Return the specified user data object.  When the index is not
        valid, ie not in the range [0,count) then an empty pointer is
        returned.
    */
    std::shared_ptr<UserData> GetUserData (const sal_Int32 nIndex) const;

protected:

    virtual void                    AddSupportedFormats() override;
    virtual bool                    GetData( const css::datatransfer::DataFlavor& rFlavor, const OUString& rDestDoc ) override;
    virtual bool                    WriteObject( SvStream& rOStm, void* pUserObject, sal_uInt32 nUserObjectId, const css::datatransfer::DataFlavor& rFlavor ) override;
    virtual void                    ObjectReleased() override final;

private:

    SfxObjectShellRef               maDocShellRef;
    ::sd::DrawDocShell*             mpPageDocShell;
    std::vector<OUString>      maPageBookmarks;
    std::unique_ptr<TransferableDataHelper>  mpOLEDataHelper;
    std::unique_ptr<TransferableObjectDescriptor>  mpObjDesc;
    const ::sd::View*               mpSdView;
    ::sd::View*                     mpSdViewIntern;
    SdDrawDocument*                 mpSdDrawDocument;
    SdDrawDocument*                 mpSdDrawDocumentIntern;
    SdDrawDocument*                 mpSourceDoc;
    VclPtr<VirtualDevice>           mpVDev;
    std::optional<INetBookmark>     moBookmark;
    std::optional<Graphic>          moGraphic;
    std::unique_ptr<ImageMap>       mpImageMap;
    ::tools::Rectangle                       maVisArea;
    Point                           maStartPos;
    bool                            mbInternalMove               : 1;
    bool                            mbOwnDocument                : 1;
    bool                            mbOwnView                    : 1;
    bool                            mbLateInit                   : 1;
    bool                            mbPageTransferable           : 1;
    bool                            mbPageTransferablePersistent : 1;
    ::std::vector<std::shared_ptr<UserData> > maUserData;

                                    SdTransferable( const SdTransferable& ) = delete;
    SdTransferable&                 operator=( const SdTransferable& ) = delete;

    void                            CreateObjectReplacement( SdrObject* pObj );
    void                            CreateData();

};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
