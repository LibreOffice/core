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

#ifndef SC_PREVWSH_HXX
#define SC_PREVWSH_HXX

class ScrollBar;

#include "address.hxx"
#include <sfx2/viewfac.hxx>
#include <sfx2/viewsh.hxx>
#include <svx/zoomitem.hxx>

#include "shellids.hxx"

class ScDocument;
class ScDocShell;
class ScPreview;
struct ScHeaderFieldData;
class ScPreviewLocationData;
class CommandEvent;

//==================================================================


class ScPreviewShell: public SfxViewShell
{
    ScDocShell*     pDocShell;

    ScPreview*      pPreview;               // Ausgabe-Fenster
    ScrollBar*      pHorScroll;
    ScrollBar*      pVerScroll;
    Window*         pCorner;

    ::com::sun::star::uno::Sequence< ::com::sun::star::beans::PropertyValue > aSourceData;  // ViewData
    BYTE            nSourceDesignMode;      // form design mode from TabView
    SvxZoomType     eZoom;
    long            nMaxVertPos;

    SfxBroadcaster* pAccessibilityBroadcaster;

private:
    void            Construct( Window* pParent );
    DECL_LINK(ScrollHandler, ScrollBar* );
    void            DoScroll( USHORT nMode );

protected:
    virtual void    Activate(BOOL bMDI);
    virtual void    Deactivate(BOOL bMDI);

    virtual void    AdjustPosSizePixel( const Point &rPos, const Size &rSize );

    virtual void    InnerResizePixel( const Point &rOfs, const Size &rSize );
    virtual void    OuterResizePixel( const Point &rOfs, const Size &rSize );

    virtual Size    GetOptimalSizePixel() const;

    virtual String  GetDescription() const;

    virtual void    WriteUserData(String &, BOOL bBrowse = FALSE);
    virtual void    ReadUserData(const String &, BOOL bBrowse = FALSE);

    virtual void    WriteUserDataSequence (::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );
    virtual void    ReadUserDataSequence (const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );

public:
                    TYPEINFO();

                    SFX_DECL_INTERFACE(SCID_PREVIEW_SHELL)
                    SFX_DECL_VIEWFACTORY(ScPreviewShell);

                    ScPreviewShell( SfxViewFrame*           pViewFrame,
                                    SfxViewShell*           pOldSh );

    virtual         ~ScPreviewShell();

    void            InitStartTable(SCTAB nTab);

    void            UpdateScrollBars();
    BOOL            ScrollCommand( const CommandEvent& rCEvt );

    void            Execute( SfxRequest& rReq );
    void            GetState( SfxItemSet& rSet );

    void            FillFieldData( ScHeaderFieldData& rData );

    ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >
                    GetSourceData() const       { return aSourceData; }
    BYTE            GetSourceDesignMode() const { return nSourceDesignMode; }

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SfxPrinter*     GetPrinter( BOOL bCreate = FALSE );
    virtual USHORT          SetPrinter( SfxPrinter* pNewPrinter, USHORT nDiffFlags = SFX_PRINTER_ALL, bool bIsAPI=false );
    virtual PrintDialog*    CreatePrintDialog( Window* pParent );
    virtual SfxTabPage*     CreatePrintOptionsPage( Window *pParent, const SfxItemSet &rOptions );
    virtual void            PreparePrint( PrintDialog* pPrintDialog = NULL );
    virtual ErrCode         DoPrint( SfxPrinter *pPrinter, PrintDialog *pPrintDialog, BOOL bSilent, BOOL bIsAPI );
    virtual USHORT          Print( SfxProgress& rProgress, BOOL bIsAPI, PrintDialog* pPrintDialog = NULL );

    void    AddAccessibilityObject( SfxListener& rObject );
    void    RemoveAccessibilityObject( SfxListener& rObject );
    void    BroadcastAccessibility( const SfxHint &rHint );
    BOOL    HasAccessibilityObjects();

    const ScPreviewLocationData& GetLocationData();
    ScDocument*     GetDocument();
    ScPreview*      GetPreview() { return pPreview; }
};



#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
