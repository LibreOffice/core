/*************************************************************************
 *
 *  $RCSfile: sdtreelb.hxx,v $
 *
 *  $Revision: 1.11 $
 *
 *  last change: $Author: kz $ $Date: 2005-01-18 15:17:59 $
 *
 *  The Contents of this file are made available subject to the terms of
 *  either of the following licenses
 *
 *         - GNU Lesser General Public License Version 2.1
 *         - Sun Industry Standards Source License Version 1.1
 *
 *  Sun Microsystems Inc., October, 2000
 *
 *  GNU Lesser General Public License Version 2.1
 *  =============================================
 *  Copyright 2000 by Sun Microsystems, Inc.
 *  901 San Antonio Road, Palo Alto, CA 94303, USA
 *
 *  This library is free software; you can redistribute it and/or
 *  modify it under the terms of the GNU Lesser General Public
 *  License version 2.1, as published by the Free Software Foundation.
 *
 *  This library is distributed in the hope that it will be useful,
 *  but WITHOUT ANY WARRANTY; without even the implied warranty of
 *  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 *  Lesser General Public License for more details.
 *
 *  You should have received a copy of the GNU Lesser General Public
 *  License along with this library; if not, write to the Free Software
 *  Foundation, Inc., 59 Temple Place, Suite 330, Boston,
 *  MA  02111-1307  USA
 *
 *
 *  Sun Industry Standards Source License Version 1.1
 *  =================================================
 *  The contents of this file are subject to the Sun Industry Standards
 *  Source License Version 1.1 (the "License"); You may not use this file
 *  except in compliance with the License. You may obtain a copy of the
 *  License at http://www.openoffice.org/license.html.
 *
 *  Software provided under this License is provided on an "AS IS" basis,
 *  WITHOUT WARRANTY OF ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING,
 *  WITHOUT LIMITATION, WARRANTIES THAT THE SOFTWARE IS FREE OF DEFECTS,
 *  MERCHANTABLE, FIT FOR A PARTICULAR PURPOSE, OR NON-INFRINGING.
 *  See the License for the specific provisions governing your rights and
 *  obligations concerning the Software.
 *
 *  The Initial Developer of the Original Code is: Sun Microsystems, Inc.
 *
 *  Copyright: 2000 by Sun Microsystems, Inc.
 *
 *  All Rights Reserved.
 *
 *  Contributor(s): _______________________________________
 *
 *
 ************************************************************************/

#ifndef _SDTREELB_HXX
#define _SDTREELB_HXX

#ifndef _TRANSFER_HXX //autogen
#include <svtools/transfer.hxx>
#endif

#ifndef _SD_SDRESID_HXX
#include "sdresid.hxx"
#endif
#ifndef _PRESENTATION_HXX
#include "pres.hxx"
#endif
#ifndef INCLUDED_SDDLLAPI_H
#include "sddllapi.h"
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif
#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif
#ifndef _URLBMK_HXX
#include <svtools/urlbmk.hxx>
#endif
#ifndef _REF_HXX //autogen
#include <tools/ref.hxx>
#endif


class SdDrawDocument;
class SfxMedium;
class SfxViewFrame;
class SdNavigatorWin;
class SdrObject;
class SdPage;

namespace sd {
class DrawDocShell;
#ifndef SV_DECL_DRAW_DOC_SHELL_DEFINED
#define SV_DECL_DRAW_DOC_SHELL_DEFINED
SV_DECL_REF(DrawDocShell)
#endif
}

/*************************************************************************
|*
|* Effekte-Tab-Dialog
|*
\************************************************************************/

class SD_DLLPUBLIC SdPageObjsTLB : public SvTreeListBox
{
private:

    static BOOL             bIsInDrag;      // static, falls der Navigator im ExecuteDrag geloescht wird

public:

    // nested class to implement the TransferableHelper
    class SdPageObjsTransferable : public TransferableHelper
    {
    private:

        SdPageObjsTLB&      mrParent;
        INetBookmark        maBookmark;
        ::sd::DrawDocShell&     mrDocShell;
        NavigatorDragType   meDragType;

        SD_DLLPRIVATE virtual               ~SdPageObjsTransferable();

        SD_DLLPRIVATE virtual void      AddSupportedFormats();
        SD_DLLPRIVATE virtual sal_Bool  GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
        SD_DLLPRIVATE virtual void      DragFinished( sal_Int8 nDropAction );

        SD_DLLPRIVATE virtual sal_Int64 SAL_CALL getSomething( const ::com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( ::com::sun::star::uno::RuntimeException );

    public:

        SdPageObjsTransferable(
            SdPageObjsTLB& rParent,
            const INetBookmark& rBookmark,
            ::sd::DrawDocShell& rDocShell,
            NavigatorDragType eDragType )
            : mrParent( rParent ),
              maBookmark( rBookmark ),
              mrDocShell( rDocShell ),
              meDragType( eDragType )
        {}

        ::sd::DrawDocShell&     GetDocShell() const;
        NavigatorDragType   GetDragType() const;

    public:

        static const ::com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();
        static SdPageObjsTransferable* getImplementation( const ::com::sun::star::uno::Reference< ::com::sun::star::uno::XInterface >& rxData ) throw();
    };

    friend class SdPageObjsTLB::SdPageObjsTransferable;

    /** Determine whether the specified page belongs to the current show
        which is either the standard show or a custom show.
        @param pPage
            Pointer to the page for which to check whether it belongs to the
            show.
        @return
            Returns <FALSE/> if there is no custom show or if the current
            show does not contain the specified page at least once.
    */
    bool PageBelongsToCurrentShow (const SdPage* pPage) const;

protected:

    Window*                 pParent;
    const SdDrawDocument*   pDoc;
    SdDrawDocument*         pBookmarkDoc;
    SfxMedium*              pMedium;
    SfxMedium*              pOwnMedium;
    /// The color used for masking transparency.
    Color                   aTransparencyColor;
    Image                   aImgOle;
    Image                   aImgGraphic;
    Image                   aImgOleH;
    Image                   aImgGraphicH;
    BOOL                    bLinkableSelected;
    BOOL                    bDragEnabled;
    String                  aDocName;
    ::sd::DrawDocShellRef       xBookmarkDocShRef;  // Zum Laden von Bookmarks
    ::sd::DrawDocShell*         pDropDocSh;
    SdNavigatorWin*         pDropNavWin;
    SfxViewFrame*           pFrame;

    // DragSourceHelper
    virtual void            StartDrag( sal_Int8 nAction, const Point& rPosPixel );

    // DropTargetHelper
    virtual sal_Int8        AcceptDrop( const AcceptDropEvent& rEvt );
    virtual sal_Int8        ExecuteDrop( const ExecuteDropEvent& rEvt );

    virtual void            RequestingChilds( SvLBoxEntry* pParent );

    void                    DoDrag();
    void                    DragFinished( sal_uInt8 nDropAction );

    String                  GetObjectName( const SdrObject* pObj ) const;
    void                    CloseBookmarkDoc();
                            DECL_STATIC_LINK(SdPageObjsTLB, ExecDragHdl, void*);

public:

                            SdPageObjsTLB( Window* pParent, const SdResId& rSdResId, BOOL bEnableDrop = FALSE );
                            ~SdPageObjsTLB();

    virtual void            SelectHdl();
    virtual void            KeyInput( const KeyEvent& rKEvt );

    void                    SetViewFrame( SfxViewFrame* pViewFrame ) { pFrame = pViewFrame; }
    SfxViewFrame*           GetViewFrame() const { return pFrame; }

    void                    Fill( const SdDrawDocument*, BOOL bAllPages, const String& rDocName );
    void                    Fill( const SdDrawDocument*, SfxMedium* pSfxMedium, const String& rDocName );
    BOOL                    IsEqualToDoc( const SdDrawDocument* pInDoc = NULL );
    BOOL                    HasSelectedChilds( const String& rName );
    BOOL                    SelectEntry( const String& rName );
    String                  GetSelectEntry();
    List*                   GetSelectEntryList( USHORT nDepth );
    List*                   GetBookmarkList( USHORT nType );
    SdDrawDocument*         GetBookmarkDoc(SfxMedium* pMedium = NULL);
    ::sd::DrawDocShell*         GetDropDocSh() { return(pDropDocSh); }

    BOOL                    IsLinkableSelected() const { return bLinkableSelected; }

    static BOOL             IsInDrag()  { return bIsInDrag; }
};

#endif      // _SDTREELB_HXX
