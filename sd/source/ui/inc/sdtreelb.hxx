/*************************************************************************
 *
 *  $RCSfile: sdtreelb.hxx,v $
 *
 *  $Revision: 1.1.1.1 $
 *
 *  last change: $Author: hr $ $Date: 2000-09-18 16:48:41 $
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

#ifndef _SD_SDRESID_HXX
#include "sdresid.hxx"
#endif

#ifndef _STRING_HXX //autogen
#include <tools/string.hxx>
#endif

#ifndef _SVTREEBOX_HXX //autogen
#include <svtools/svtreebx.hxx>
#endif

#ifndef _REF_HXX //autogen
#include <tools/ref.hxx>
#endif

#ifndef SV_DECL_SDDRAWDOCSHELL_DEFINED
#define SV_DECL_SDDRAWDOCSHELL_DEFINED
SV_DECL_REF(SdDrawDocShell)
#endif

class SdDrawDocument;
class SfxMedium;
class SfxViewFrame;

/*************************************************************************
|*
|* Effekte-Tab-Dialog
|*
\************************************************************************/

class SdPageObjsTLB : public SvTreeListBox
{
private:
    static BOOL             bIsInDrag;      // static, falls der Navigator im ExecuteDrag geloescht wird

protected:
    Window*                 pParent;
    const SdDrawDocument*   pDoc;
    SdDrawDocument*         pBookmarkDoc;
    SfxMedium*              pMedium;
    SfxMedium*              pOwnMedium;
    Color                   aColor;
    Image                   aImgOle;
    Image                   aImgGraphic;
    BOOL                    bOleSelected;
    BOOL                    bGraphicSelected;
    BOOL                    bDragEnabled;
    String                  aDocName;
    SdDrawDocShellRef       xBookmarkDocShRef;  // Zum Laden von Bookmarks
    SdDrawDocShell*         pDropDocSh;
    SfxViewFrame*           pFrame;

    virtual void            RequestingChilds( SvLBoxEntry* pParent );
    void                    DoDrag();
    DECL_STATIC_LINK(SdPageObjsTLB, ExecDragHdl, void*);

public:
    SdPageObjsTLB( Window* pParent, const SdResId& rSdResId, BOOL bEnableDrop = FALSE );
    ~SdPageObjsTLB();

    virtual void    SelectHdl();
    virtual void    KeyInput( const KeyEvent& rKEvt );
    virtual void    Command(const CommandEvent& rCEvt );
    virtual BOOL    QueryDrop(DropEvent& rEvt);
    virtual BOOL    Drop(const DropEvent& rEvt);

    void            SetViewFrame( SfxViewFrame* pViewFrame ) { pFrame = pViewFrame; }

    void            Fill( const SdDrawDocument*, BOOL bAllPages,
                          const String& rDocName );
    void            Fill( const SdDrawDocument*, SfxMedium* pSfxMedium,
                          const String& rDocName );
    BOOL            IsEqualToDoc( const SdDrawDocument* pInDoc = NULL );
    BOOL            HasSelectedChilds( const String& rName );
    BOOL            SelectEntry( const String& rName );
    String          GetSelectEntry();
    List*           GetSelectEntryList( USHORT nDepth );
    List*           GetBookmarkList( USHORT nType );
    SdDrawDocument* GetBookmarkDoc(SfxMedium* pMedium = NULL);
    SdDrawDocShell* GetDropDocSh() { return(pDropDocSh); }
    void            CloseBookmarkDoc();
    BOOL            IsOleSelected() const { return( bOleSelected ); }
    BOOL            IsGraphicSelected() const { return( bGraphicSelected ); }

    static BOOL     IsInDrag()  { return bIsInDrag; }
};

#endif      // _SDTREELB_HXX

