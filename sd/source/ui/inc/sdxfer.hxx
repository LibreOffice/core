/*************************************************************************
 *
 *  $RCSfile: sdxfer.hxx,v $
 *
 *  $Revision: 1.3 $
 *
 *  last change: $Author: ka $ $Date: 2001-03-30 10:33:43 $
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

#ifndef _SD_SDXFER_HXX
#define _SD_SDXFER_HXX

#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif
#ifndef _EMBOBJ_HXX //autogen
#include <so3/embobj.hxx>
#endif
#ifndef _SV_GRAPH_HXX //autogen
#include <vcl/graph.hxx>
#endif

// ------------------
// - SdTransferable -
// ------------------

class SdView;
class SdDrawDocument;
class INetBookmark;
class ImageMap;
class VirtualDevice;

class SdTransferable : public TransferableHelper
{
private:

    SvEmbeddedObjectRef             aDocShellRef;
    TransferableDataHelper          aOLEDataHelper;
    TransferableObjectDescriptor*   pObjDesc;
    SvObjectDescriptor*             pObjectDescriptor;
    const SdView*                   pSdView;
    SdView*                         pSdViewIntern;
    SdDrawDocument*                 pSdDrawDocument;
    SdDrawDocument*                 pSdDrawDocumentIntern;
    SdDrawDocument*                 pSourceDoc;
    VirtualDevice*                  pVDev;
    INetBookmark*                   pBookmark;
    Graphic*                        pGraphic;
    ImageMap*                       pImageMap;
    Rectangle                       aVisArea;
    Point                           aStartPos;
    BOOL                            bInternalMove;
    BOOL                            bOwnDocument;
    BOOL                            bOwnView;

                                    // not available
                                    SdTransferable();
                                    SdTransferable( const SdTransferable& );
    SdTransferable&                 operator=( const SdTransferable& );

    void                            CreateObjectReplacement( SdrObject* pObj );
    void                            CreateData();

protected:

    virtual void                    AddSupportedFormats();
    virtual sal_Bool                GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual sal_Bool                WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId, const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual void                    DragFinished( sal_Int8 nDropAction );
    virtual void                    ObjectReleased();

public:

                                    SdTransferable( SdDrawDocument* pSrcDoc, SdView* pWorkView = NULL );
                                    ~SdTransferable();

    void                            SetDocShell( const SvEmbeddedObjectRef& rRef ) { aDocShellRef = rRef; }
    const SvEmbeddedObjectRef&      GetDocShell() const { return aDocShellRef; }

    void                            SetWorkDocument( const SdDrawDocument* pWorkDoc ) { pSdDrawDocument = pSdDrawDocumentIntern = (SdDrawDocument*) pWorkDoc; }
    const SdDrawDocument*           GetWorkDocument() const { return pSdDrawDocument; }

    void                            SetView( const SdView* pView ) { pSdView = pView; }
    const SdView*                   GetView() const { return pSdView; }

    void                            SetObjectDescriptor( const TransferableObjectDescriptor& rObjDesc );

    void                            ResetPageView();
    void                            UpdatePageView();

    void                            SetStartPos( const Point& rStartPos ) { aStartPos = rStartPos; }
    const Point&                    GetStartPos() const { return aStartPos; }

    void                            SetInternalMove( BOOL bSet ) { bInternalMove = bSet; }
    BOOL                            IsInternalMove() const { return bInternalMove; }

    BOOL                            HasSourceDoc( const SdDrawDocument* pDoc ) const { return( pSourceDoc == pDoc ); }
};

#endif // _SD_SDXFER_HXX
