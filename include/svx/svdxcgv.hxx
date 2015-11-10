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

#ifndef INCLUDED_SVX_SVDXCGV_HXX
#define INCLUDED_SVX_SVDXCGV_HXX

#include <svx/svdedxv.hxx>

#include <vcl/gdimtf.hxx>
#include <svx/svxdllapi.h>

class SVX_DLLPUBLIC SdrExchangeView: public SdrObjEditView
{
    friend class SdrPageView;

protected:

    void                ImpGetPasteObjList(Point& rPos, SdrObjList*& rpLst);
    void                ImpPasteObject(SdrObject* pObj, SdrObjList& rLst, const Point& rCenter, const Size& rSiz, const MapMode& rMap, SdrInsertFlags nOptions);
    bool                ImpGetPasteLayer(const SdrObjList* pObjList, SdrLayerID& rLayer) const;

    // Returns true if rPt has changed
    bool                ImpLimitToWorkArea(Point& rPt) const;

protected:
    // #i71538# make constructors of SdrView sub-components protected to avoid incomplete incarnations which may get casted to SdrView
    SdrExchangeView(SdrModel* pModel1, OutputDevice* pOut = nullptr);

public:
    // Output all marked objects on the specified OutputDevice
    ::std::vector< SdrObject* > GetMarkedObjects() const;
    virtual void        DrawMarkedObj(OutputDevice& rOut) const;

    // E.g. for Clipboard, Drag'n'Drop, ...
    // Add all marked objects to a metafile.
    // FIXME: This is known to be somewhat buggy still (Offset...,
    // foreign graphics objects (SdrGrafObj), virtual object
    // copies (SdrVirtObj) with anchor position <>(0,0)).
    GDIMetaFile GetMarkedObjMetaFile(bool bNoVDevIfOneMtfMarked = false) const;

    // Draw all marked objects onto a bitmap, with the display's color depth
    // and resolution
    BitmapEx GetMarkedObjBitmapEx(bool bNoVDevIfOneBmpMarked = false) const;

    // Copy all marked objects to a new model, consisting of exactly one page,
    // with the flag PageNotValid set. This means, that only the page's objects
    // are valid and not the page itself (page size, margins).
    // The new model is created on the free store and passed to the caller of
    // this method, which is responsible to dispose it later on.
    //
    // When inserting the marked objects into the one page of the new model,
    // the page-local layer is merged. If there's no more room left for
    // additional page-local layers, the corresponding objects are assigned
    // the default layer (layer 0, document-global standard layer).
    virtual SdrModel*   GetMarkedObjModel() const;

    Graphic         GetAllMarkedGraphic() const;

    /** Generate a Graphic for the given draw object in the given model

        @param pModel
        Must not be NULL. Denotes the draw model the object is a part
        of.

        @param pObj
        The object (can also be a group object) to retrieve a Graphic
        for. Must not be NULL.

        @return a graphical representation of the given object, as it
        appears on screen (e.g. with rotation, if any, applied).
     */
    static Graphic  GetObjGraphic( const SdrModel* pModel, const SdrObject* pObj );

    // The new Draw objects are marked for all paste methods.
    // If bAddMark is true, the new Draw objects are added to an existing
    // selection, which is meant for Drag'n'Drop with multiple items.
    //
    // The methods with a point parameter insert objects centered at that
    // position, all others are centered at the 1st OutputDevice of the View.
    //
    // If pPg is set, the objects are inserted at that page. The positioning
    // (rPos or alignment) are not relative to the View, but to the Page.
    //
    // Note: SdrObjList is the base class of SdrPage.
    // All methods return true, if the objects have been successfully created
    // and inserted.
    // If pLst = false and no TextEdit active, we can rely on the fact, that
    // it is marked at the View. Or else the marking only happens if the pLst
    // is also shown at the View at the moment.
    //
    // Valid values for nOptions are SDRINSERT_DONTMARK and SDRINSERT_ADDMARK
    // (@see svdedtv.hxx).
    virtual bool Paste(
        const SdrModel& rMod, const Point& rPos, SdrObjList* pLst, SdrInsertFlags nOptions,
        const OUString& rSrcShellID, const OUString& rDestShellID );

    bool            Paste(const OUString& rStr, const Point& rPos, SdrObjList* pLst=nullptr, SdrInsertFlags nOptions=SdrInsertFlags::NONE);
    bool            Paste(SvStream& rInput, const OUString& rBaseURL, sal_uInt16 eFormat, const Point& rPos, SdrObjList* pLst=nullptr, SdrInsertFlags nOptions=SdrInsertFlags::NONE);

    static bool     Cut( sal_uIntPtr nFormat = SDR_ANYFORMAT );

    static bool     Yank( sal_uIntPtr nFormat = SDR_ANYFORMAT );

    static bool     Paste( vcl::Window* pWin = nullptr, sal_uIntPtr nFormat = SDR_ANYFORMAT );
};

#endif // INCLUDED_SVX_SVDXCGV_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
