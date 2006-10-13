/*************************************************************************
 *
 *  OpenOffice.org - a multi-platform office productivity suite
 *
 *  $RCSfile: drwtrans.hxx,v $
 *
 *  $Revision: 1.13 $
 *
 *  last change: $Author: obo $ $Date: 2006-10-13 11:36:05 $
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

#ifndef SC_DRWTRANS_HXX
#define SC_DRWTRANS_HXX

#ifndef _COM_SUN_STAR_UNO_REFERENCE_HXX_
#include <com/sun/star/uno/Reference.hxx>
#endif

#ifndef _COM_SUN_STAR_EMBED_XEMBEDDEDOBJECT_HPP_
#include <com/sun/star/embed/XEmbeddedObject.hxx>
#endif

#ifndef _TRANSFER_HXX
#include <svtools/transfer.hxx>
#endif

//REMOVE    #ifndef _IPOBJ_HXX
//REMOVE    #include <so3/ipobj.hxx>
//REMOVE    #endif

#include <sfx2/objsh.hxx>

#ifndef SC_SCGLOB_HXX
#include "global.hxx"
#endif


class SdrModel;
class ScDocShell;
class INetBookmark;
class SdrObject;
class SdrView;
class ScDrawView;
class SdrOle2Obj;

class ScDrawTransferObj : public TransferableHelper
{
private:
    SdrModel*                       pModel;
    TransferableDataHelper          aOleData;
    TransferableObjectDescriptor    aObjDesc;
//REMOVE        SvEmbeddedObjectRef             aDocShellRef;
//REMOVE        SvEmbeddedObjectRef             aDrawPersistRef;
    SfxObjectShellRef               aDocShellRef;
    SfxObjectShellRef               aDrawPersistRef;

                                    // extracted from model in ctor:
    Size                            aSrcSize;
    INetBookmark*                   pBookmark;
    BOOL                            bGraphic;
    BOOL                            bGrIsBit;
    BOOL                            bOleObj;
                                    // source information for drag&drop:
                                    // (view is needed to handle drawing obejcts)
    SdrView*                        pDragSourceView;
    USHORT                          nDragSourceFlags;
    BOOL                            bDragWasInternal;

    sal_uInt32                      nSourceDocID;


    void                InitDocShell();
//REMOVE        SvInPlaceObjectRef  GetSingleObject();
    SdrOle2Obj* GetSingleObject();

public:
            ScDrawTransferObj( SdrModel* pClipModel, ScDocShell* pContainerShell,
                                const TransferableObjectDescriptor& rDesc );
    virtual ~ScDrawTransferObj();

    virtual void        AddSupportedFormats();
    virtual sal_Bool    GetData( const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual sal_Bool    WriteObject( SotStorageStreamRef& rxOStm, void* pUserObject, sal_uInt32 nUserObjectId,
                                        const ::com::sun::star::datatransfer::DataFlavor& rFlavor );
    virtual void        ObjectReleased();
    virtual void        DragFinished( sal_Int8 nDropAction );

    SdrModel*           GetModel()  { return pModel; }

    void                SetDrawPersist( const SfxObjectShellRef& rRef );
    void                SetDragSource( ScDrawView* pView );
    void                SetDragSourceObj( SdrObject* pObj, SCTAB nTab );
    void                SetDragSourceFlags( USHORT nFlags );
    void                SetDragWasInternal();

    SdrView*            GetDragSourceView()             { return pDragSourceView; }
    USHORT              GetDragSourceFlags() const      { return nDragSourceFlags; }

    void                SetSourceDocID( sal_uInt32 nVal )
                            { nSourceDocID = nVal; }
    sal_uInt32          GetSourceDocID() const      { return nSourceDocID; }

    static ScDrawTransferObj* GetOwnClipboard( Window* pUIWin );
};

#endif

