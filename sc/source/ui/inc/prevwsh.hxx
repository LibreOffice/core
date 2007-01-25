/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: prevwsh.hxx,v $
 *
 *  $Revision: 1.9 $
 *
 *  last change: $Author: obo $ $Date: 2007-01-25 11:41:35 $
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

#ifndef SC_PREVWSH_HXX
#define SC_PREVWSH_HXX

class ScrollBar;

#ifndef SC_ADDRESS_HXX
#include "address.hxx"
#endif

#ifndef _VIEWFAC_HXX //autogen
#include <sfx2/viewfac.hxx>
#endif
#ifndef _SFXVIEWSH_HXX //autogen
#include <sfx2/viewsh.hxx>
#endif
#ifndef _SVX_ZOOMITEM_HXX //autogen
#include <svx/zoomitem.hxx>
#endif

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

                    SFX_DECL_INTERFACE(SCID_PREVIEW_SHELL);
                    SFX_DECL_VIEWFACTORY(ScPreviewShell);

                    ScPreviewShell( SfxViewFrame*           pViewFrame,
                                    const ScPreviewShell&   rWin );
                    ScPreviewShell( SfxViewFrame*           pViewFrame,
                                    Window*                 pParent);
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

    virtual void SFX_NOTIFY( SfxBroadcaster& rBC, const TypeId& rBCType,
                         const SfxHint& rHint, const TypeId& rHintType );

    virtual SfxPrinter*     GetPrinter( BOOL bCreate = FALSE );
    virtual USHORT          SetPrinter( SfxPrinter* pNewPrinter, USHORT nDiffFlags = SFX_PRINTER_ALL );
    virtual PrintDialog*    CreatePrintDialog( Window* pParent );
    virtual SfxTabPage*     CreatePrintOptionsPage( Window *pParent, const SfxItemSet &rOptions );
    virtual void            PreparePrint( PrintDialog* pPrintDialog = NULL );
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
