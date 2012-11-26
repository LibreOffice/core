/**************************************************************
 *
 * Licensed to the Apache Software Foundation (ASF) under one
 * or more contributor license agreements.  See the NOTICE file
 * distributed with this work for additional information
 * regarding copyright ownership.  The ASF licenses this file
 * to you under the Apache License, Version 2.0 (the
 * "License"); you may not use this file except in compliance
 * with the License.  You may obtain a copy of the License at
 *
 *   http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing,
 * software distributed under the License is distributed on an
 * "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY
 * KIND, either express or implied.  See the License for the
 * specific language governing permissions and limitations
 * under the License.
 *
 *************************************************************/



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
    sal_uInt8           nSourceDesignMode;      // form design mode from TabView
    SvxZoomType     eZoom;
    long            nMaxVertPos;

    SfxBroadcaster* pAccessibilityBroadcaster;

private:
    void            Construct( Window* pParent );
    DECL_LINK(ScrollHandler, ScrollBar* );
    void            DoScroll( sal_uInt16 nMode );

protected:
    virtual void    Activate(sal_Bool bMDI);
    virtual void    Deactivate(sal_Bool bMDI);

    virtual void    AdjustPosSizePixel( const Point &rPos, const Size &rSize );

    virtual void    InnerResizePixel( const Point &rOfs, const Size &rSize );
    virtual void    OuterResizePixel( const Point &rOfs, const Size &rSize );

    virtual Size    GetOptimalSizePixel() const;

    virtual String  GetDescription() const;

    virtual void    WriteUserData(String &, sal_Bool bBrowse = sal_False);
    virtual void    ReadUserData(const String &, sal_Bool bBrowse = sal_False);

    virtual void    WriteUserDataSequence (::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );
    virtual void    ReadUserDataSequence (const ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >&, sal_Bool bBrowse = sal_False );

public:
                    SFX_DECL_INTERFACE(SCID_PREVIEW_SHELL)
                    SFX_DECL_VIEWFACTORY(ScPreviewShell);

                    ScPreviewShell( SfxViewFrame*           pViewFrame,
                                    SfxViewShell*           pOldSh );

    virtual         ~ScPreviewShell();

    void            InitStartTable(SCTAB nTab);

    void            UpdateScrollBars();
    sal_Bool            ScrollCommand( const CommandEvent& rCEvt );

    void            Execute( SfxRequest& rReq );
    void            GetState( SfxItemSet& rSet );

    void            FillFieldData( ScHeaderFieldData& rData );

    ::com::sun::star::uno::Sequence < ::com::sun::star::beans::PropertyValue >
                    GetSourceData() const       { return aSourceData; }
    sal_uInt8           GetSourceDesignMode() const { return nSourceDesignMode; }

    virtual void Notify( SfxBroadcaster& rBC, const SfxHint& rHint );

    virtual SfxPrinter*     GetPrinter( sal_Bool bCreate = sal_False );
    virtual sal_uInt16          SetPrinter( SfxPrinter* pNewPrinter, sal_uInt16 nDiffFlags = SFX_PRINTER_ALL, bool bIsAPI=false );
    virtual SfxTabPage*     CreatePrintOptionsPage( Window *pParent, const SfxItemSet &rOptions );

    void    AddAccessibilityObject( SfxListener& rObject );
    void    RemoveAccessibilityObject( SfxListener& rObject );
    void    BroadcastAccessibility( const SfxHint &rHint );
    sal_Bool    HasAccessibilityObjects();

    const ScPreviewLocationData& GetLocationData();
    ScDocument*     GetDocument();
    ScPreview*      GetPreview() { return pPreview; }
};



#endif
