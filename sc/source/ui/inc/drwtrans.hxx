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

#ifndef SC_DRWTRANS_HXX
#define SC_DRWTRANS_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/embed/XEmbeddedObject.hpp>
#include <svtools/transfer.hxx>

#include <sfx2/objsh.hxx>
#include "global.hxx"
#include "charthelper.hxx"
#include "rangelst.hxx"


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
    sal_Bool                            bGraphic;
    sal_Bool                            bGrIsBit;
    sal_Bool                            bOleObj;
                                    // source information for drag&drop:
                                    // (view is needed to handle drawing obejcts)
    SdrView*                        pDragSourceView;
    sal_uInt16                          nDragSourceFlags;
    sal_Bool                            bDragWasInternal;

    sal_uInt32                      nSourceDocID;

    ScRangeListVector               m_aProtectedChartRangesVector;


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
    void                SetDragSourceFlags( sal_uInt16 nFlags );
    void                SetDragWasInternal();

    SdrView*            GetDragSourceView()             { return pDragSourceView; }
    sal_uInt16              GetDragSourceFlags() const      { return nDragSourceFlags; }

    void                SetSourceDocID( sal_uInt32 nVal )
                            { nSourceDocID = nVal; }
    sal_uInt32          GetSourceDocID() const      { return nSourceDocID; }

    static ScDrawTransferObj* GetOwnClipboard( Window* pUIWin );
    virtual sal_Int64 SAL_CALL getSomething( const com::sun::star::uno::Sequence< sal_Int8 >& rId ) throw( com::sun::star::uno::RuntimeException );
    static const com::sun::star::uno::Sequence< sal_Int8 >& getUnoTunnelId();

    const ScRangeListVector& GetProtectedChartRangesVector() const { return m_aProtectedChartRangesVector; }
};

#endif

