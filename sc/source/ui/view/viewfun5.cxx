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

#include <com/sun/star/embed/XEmbedObjectClipboardCreator.hpp>
#include <com/sun/star/embed/Aspects.hpp>
#include <com/sun/star/embed/MSOLEObjectSystemCreator.hpp>

#include <svx/unomodel.hxx>
#include <unotools/streamwrap.hxx>

#include <svx/fmmodel.hxx>
#include <svx/svdetc.hxx>
#include <svx/svditer.hxx>
#include <svx/svdobj.hxx>
#include <svx/svdogrp.hxx>
#include <svx/svdouno.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpage.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <comphelper/classids.hxx>
#include <sot/formats.hxx>
#include <sot/filelist.hxx>
#include <sot/storage.hxx>
#include <svl/ptitem.hxx>
#include <svl/stritem.hxx>
#include <vcl/transfer.hxx>
#include <vcl/graph.hxx>

#include <comphelper/lok.hxx>
#include <comphelper/processfactory.hxx>
#include <comphelper/storagehelper.hxx>
#include <comphelper/string.hxx>

#include <viewfunc.hxx>
#include <docsh.hxx>
#include <drawview.hxx>
#include <impex.hxx>
#include <dbfunc.hxx>
#include <dbdata.hxx>
#include <sc.hrc>
#include <filter.hxx>
#include <globstr.hrc>
#include <global.hxx>
#include <scextopt.hxx>
#include <tabvwsh.hxx>
#include <compiler.hxx>
#include <scmod.hxx>

#include <asciiopt.hxx>
#include <scabstdlg.hxx>
#include <clipparam.hxx>
#include <markdata.hxx>
#include <sfx2/viewfrm.hxx>
#include <svx/dbaexchange.hxx>
#include <memory>

using namespace com::sun::star;

bool ScViewFunc::PasteDataFormat( SotClipboardFormatId nFormatId,
                    const uno::Reference<datatransfer::XTransferable>& rxTransferable,
                    SCCOL nPosX, SCROW nPosY, const Point* pLogicPos, bool bLink, bool bAllowDialogs )
{
    ScDocument* pDoc = GetViewData().GetDocument();
    pDoc->SetPastingDrawFromOtherDoc( true );

    Point aPos;                     //  inserting position (1/100 mm)
    if (pLogicPos)
        aPos = *pLogicPos;
    else
    {
        //  inserting position isn't needed for text formats
        bool bIsTextFormat = ( ScImportExport::IsFormatSupported( nFormatId ) ||
                                nFormatId == SotClipboardFormatId::RTF );
        if ( !bIsTextFormat )
        {
            //  Window MapMode isn't drawing MapMode if DrawingLayer hasn't been created yet

            SCTAB nTab = GetViewData().GetTabNo();
            long nXT = 0;
            for (SCCOL i=0; i<nPosX; i++)
                nXT += pDoc->GetColWidth(i,nTab);
            if (pDoc->IsNegativePage(nTab))
                nXT = -nXT;
            sal_uLong nYT = pDoc->GetRowHeight( 0, nPosY-1, nTab);
            aPos = Point( static_cast<long>(nXT * HMM_PER_TWIPS), static_cast<long>(nYT * HMM_PER_TWIPS) );
        }
    }

    TransferableDataHelper aDataHelper( rxTransferable );
    bool bRet = false;

    //  handle individual formats

    if ( nFormatId == SotClipboardFormatId::EMBED_SOURCE ||
         nFormatId == SotClipboardFormatId::LINK_SOURCE ||
         nFormatId == SotClipboardFormatId::EMBED_SOURCE_OLE ||
         nFormatId == SotClipboardFormatId::LINK_SOURCE_OLE ||
         nFormatId == SotClipboardFormatId::EMBEDDED_OBJ_OLE )
    {
        uno::Reference < io::XInputStream > xStm;
        TransferableObjectDescriptor   aObjDesc;

        if (aDataHelper.GetTransferableObjectDescriptor(SotClipboardFormatId::OBJECTDESCRIPTOR, aObjDesc))
            xStm = aDataHelper.GetInputStream(nFormatId, OUString());

        if (xStm.is())
        {
            if ( aObjDesc.maClassName == SvGlobalName( SO3_SC_CLASSID_60 ) )
            {
                uno::Reference < embed::XStorage > xStore = ::comphelper::OStorageHelper::GetStorageFromInputStream( xStm );

                // mba: BaseURL doesn't make sense for clipboard
                // #i43716# Medium must be allocated with "new".
                // DoLoad stores the pointer and deletes it with the SfxObjectShell.
                SfxMedium* pMedium = new SfxMedium( xStore, OUString() );

                //  TODO/LATER: is it a problem that we don't support binary formats here?
                ScDocShellRef xDocShRef = new ScDocShell(SfxModelFlags::EMBEDDED_OBJECT);
                if (xDocShRef->DoLoad(pMedium))
                {
                    ScDocument& rSrcDoc = xDocShRef->GetDocument();
                    SCTAB nSrcTab = rSrcDoc.GetVisibleTab();
                    if (!rSrcDoc.HasTable(nSrcTab))
                        nSrcTab = 0;

                    ScMarkData aSrcMark;
                    aSrcMark.SelectOneTable( nSrcTab );         // for CopyToClip
                    ScDocumentUniquePtr pClipDoc(new ScDocument( SCDOCMODE_CLIP ));

                    SCCOL nFirstCol, nLastCol;
                    SCROW nFirstRow, nLastRow;
                    if ( rSrcDoc.GetDataStart( nSrcTab, nFirstCol, nFirstRow ) )
                        rSrcDoc.GetCellArea( nSrcTab, nLastCol, nLastRow );
                    else
                    {
                        nFirstCol = nLastCol = 0;
                        nFirstRow = nLastRow = 0;
                    }
                    ScClipParam aClipParam(ScRange(nFirstCol, nFirstRow, nSrcTab, nLastCol, nLastRow, nSrcTab), false);
                    rSrcDoc.CopyToClip(aClipParam, pClipDoc.get(), &aSrcMark, false, false);
                    ScGlobal::SetClipDocName( xDocShRef->GetTitle( SFX_TITLE_FULLNAME ) );

                    SetCursor( nPosX, nPosY );
                    Unmark();
                    PasteFromClip( InsertDeleteFlags::ALL, pClipDoc.get(),
                                    ScPasteFunc::NONE, false, false, false, INS_NONE, InsertDeleteFlags::NONE,
                                    bAllowDialogs );
                    bRet = true;
                }

                xDocShRef->DoClose();
                xDocShRef.clear();
            }
            else
            {
                OUString aName;
                uno::Reference < embed::XEmbeddedObject > xObj = GetViewData().GetViewShell()->GetObjectShell()->
                        GetEmbeddedObjectContainer().InsertEmbeddedObject( xStm, aName );
                if ( xObj.is() )
                {
                    // try to get the replacement image from the clipboard
                    Graphic aGraphic;
                    SotClipboardFormatId nGrFormat = SotClipboardFormatId::NONE;

                    // limit the size of the preview metafile to 100000 actions
                    GDIMetaFile aMetafile;
                    if (aDataHelper.GetGDIMetaFile(SotClipboardFormatId::GDIMETAFILE, aMetafile, 100000))
                    {
                        nGrFormat = SotClipboardFormatId::GDIMETAFILE;
                        aGraphic = aMetafile;
                    }

                    // insert replacement image ( if there is one ) into the object helper
                    if ( nGrFormat != SotClipboardFormatId::NONE )
                    {
                        datatransfer::DataFlavor aDataFlavor;
                        SotExchange::GetFormatDataFlavor( nGrFormat, aDataFlavor );
                        PasteObject( aPos, xObj, &aObjDesc.maSize, &aGraphic, aDataFlavor.MimeType, aObjDesc.mnViewAspect );
                    }
                    else
                        PasteObject( aPos, xObj, &aObjDesc.maSize );

                    bRet = true;
                }
                else
                {
                    OSL_FAIL("Error in CreateAndLoad");
                }
            }
        }
        else
        {
            if ( aDataHelper.GetTransferableObjectDescriptor( SotClipboardFormatId::OBJECTDESCRIPTOR_OLE, aObjDesc ) )
            {
                OUString aName;
                uno::Reference < embed::XEmbeddedObject > xObj;
                xStm = aDataHelper.GetInputStream(SotClipboardFormatId::EMBED_SOURCE_OLE, OUString());
                if (!xStm.is())
                    aDataHelper.GetInputStream(SotClipboardFormatId::EMBEDDED_OBJ_OLE, OUString());

                if (xStm.is())
                {
                    xObj = GetViewData().GetDocShell()->GetEmbeddedObjectContainer().InsertEmbeddedObject( xStm, aName );
                }
                else
                {
                    try
                    {
                        uno::Reference< embed::XStorage > xTmpStor = ::comphelper::OStorageHelper::GetTemporaryStorage();
                        uno::Reference < embed::XEmbedObjectClipboardCreator > xClipboardCreator =
                            embed::MSOLEObjectSystemCreator::create( ::comphelper::getProcessComponentContext() );

                        embed::InsertedObjectInfo aInfo = xClipboardCreator->createInstanceInitFromClipboard(
                                                            xTmpStor,
                                                            "DummyName",
                                                            uno::Sequence< beans::PropertyValue >() );

                        // TODO/LATER: in future InsertedObjectInfo will be used to get container related information
                        // for example whether the object should be an iconified one
                        xObj = aInfo.Object;
                        if ( xObj.is() )
                            GetViewData().GetDocShell()->GetEmbeddedObjectContainer().InsertEmbeddedObject( xObj, aName );
                    }
                    catch( uno::Exception& )
                    {}
                }

                if ( xObj.is() )
                {
                    // try to get the replacement image from the clipboard
                    Graphic aGraphic;
                    SotClipboardFormatId nGrFormat = SotClipboardFormatId::NONE;

// (for Selection Manager in Trusted Solaris)
#ifndef __sun
                    if( aDataHelper.GetGraphic( SotClipboardFormatId::SVXB, aGraphic ) )
                        nGrFormat = SotClipboardFormatId::SVXB;
                    else if( aDataHelper.GetGraphic( SotClipboardFormatId::GDIMETAFILE, aGraphic ) )
                        nGrFormat = SotClipboardFormatId::GDIMETAFILE;
                    else if( aDataHelper.GetGraphic( SotClipboardFormatId::BITMAP, aGraphic ) )
                        nGrFormat = SotClipboardFormatId::BITMAP;
#endif

                    // insert replacement image ( if there is one ) into the object helper
                    if ( nGrFormat != SotClipboardFormatId::NONE )
                    {
                        datatransfer::DataFlavor aDataFlavor;
                        SotExchange::GetFormatDataFlavor( nGrFormat, aDataFlavor );
                        PasteObject( aPos, xObj, &aObjDesc.maSize, &aGraphic, aDataFlavor.MimeType, aObjDesc.mnViewAspect );
                    }
                    else
                        PasteObject( aPos, xObj, &aObjDesc.maSize );

                    // let object stay in loaded state after insertion
                    SdrOle2Obj::Unload( xObj, embed::Aspects::MSOLE_CONTENT );
                    bRet = true;
                }
                else
                {
                    OSL_FAIL("Error creating external OLE object");
                }
            }
            //TODO/LATER: if format is not available, create picture
        }
    }
    else if ( nFormatId == SotClipboardFormatId::LINK )      // LINK is also in ScImportExport
    {
        bRet = PasteLink( rxTransferable );
    }
    else if ( ScImportExport::IsFormatSupported( nFormatId ) || nFormatId == SotClipboardFormatId::RTF ||
                nFormatId == SotClipboardFormatId::EDITENGINE_ODF_TEXT_FLAT )
    {
        if ( nFormatId == SotClipboardFormatId::RTF && ( aDataHelper.HasFormat( SotClipboardFormatId::EDITENGINE_ODF_TEXT_FLAT ) ) )
        {
            //  use EditView's PasteSpecial / Drop
            PasteRTF( nPosX, nPosY, rxTransferable );
            bRet = true;
        }
        else
        {
            ScAddress aCellPos( nPosX, nPosY, GetViewData().GetTabNo() );
            std::shared_ptr<ScImportExport> pObj(new ScImportExport(GetViewData().GetDocument(), aCellPos));
            pObj->SetOverwriting( true );


            std::shared_ptr<OUString> pStrBuffer(new OUString());
            tools::SvRef<SotStorageStream> xStream;
            if ( aDataHelper.GetSotStorageStream( nFormatId, xStream ) && xStream.is() )
            {
                if (nFormatId == SotClipboardFormatId::HTML)
                {
                    // Launch the text import options dialog.  For now, we do
                    // this for html pasting only, but in the future it may
                    // make sense to do it for other data types too.
                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    vcl::Window* pParent = GetActiveWin();
                    ScopedVclPtr<AbstractScTextImportOptionsDlg> pDlg(
                        pFact->CreateScTextImportOptionsDlg(pParent ? pParent->GetFrameWeld() : nullptr));

                    if (pDlg->Execute() == RET_OK)
                    {
                        ScAsciiOptions aOptions;
                        aOptions.SetLanguage(pDlg->GetLanguageType());
                        aOptions.SetDetectSpecialNumber(pDlg->IsDateConversionSet());
                        pObj->SetExtOptions(aOptions);
                    }
                    else
                    {
                        // prevent error dialog for user cancel action
                        bRet = true;
                    }
                }
                if(!bRet)
                    bRet = pObj->ImportStream( *xStream, OUString(), nFormatId );
                // mba: clipboard always must contain absolute URLs (could be from alien source)
            }
            else if ((nFormatId == SotClipboardFormatId::STRING || nFormatId == SotClipboardFormatId::STRING_TSVC)
                    && aDataHelper.GetString( nFormatId, *pStrBuffer ))
            {
                // Do CSV dialog if more than one line.
                sal_Int32 nDelim = pStrBuffer->indexOf('\n');
                if (nDelim >= 0 && nDelim != pStrBuffer->getLength () - 1)
                {
                    vcl::Window* pParent = GetActiveWin();

                    std::shared_ptr<ScImportStringStream> pStrm(new ScImportStringStream(*pStrBuffer));

                    ScAbstractDialogFactory* pFact = ScAbstractDialogFactory::Create();
                    VclPtr<AbstractScImportAsciiDlg> pDlg(
                        pFact->CreateScImportAsciiDlg(pParent, OUString(), pStrm.get(), SC_PASTETEXT));

                    bAllowDialogs = bAllowDialogs && !SC_MOD()->IsInExecuteDrop();

                    pDlg->StartExecuteAsync([this, pDlg, pDoc, pStrm, nFormatId, pStrBuffer, pObj, bAllowDialogs](sal_Int32 nResult){
                        bool bShowErrorDialog = bAllowDialogs;
                        if (RET_OK == nResult)
                        {
                            ScAsciiOptions aOptions;
                            pDlg->GetOptions( aOptions );
                            pDlg->SaveParameters();
                            pObj->SetExtOptions( aOptions );
                            pObj->ImportString( *pStrBuffer, nFormatId );

                            // TODO: what if (aObj.IsOverflow())
                            // Content was partially pasted, which can be undone by
                            // the user though.
                            bShowErrorDialog = bShowErrorDialog && pObj->IsOverflow();
                        }
                        else
                        {
                            bShowErrorDialog = false;
                            // Yes, no failure, don't raise a "couldn't paste"
                            // dialog if user cancelled.
                        }

                        InvalidateAttribs();
                        GetViewData().UpdateInputHandler();

                        pDoc->SetPastingDrawFromOtherDoc( false );

                        if (bShowErrorDialog)
                            ErrorMessage(STR_PASTE_ERROR);
                    });
                    return true;
                }
                else
                    bRet = pObj->ImportString( *pStrBuffer, nFormatId );
            }
            else if ((nFormatId != SotClipboardFormatId::STRING && nFormatId != SotClipboardFormatId::STRING_TSVC)
                    && aDataHelper.GetString( nFormatId, *pStrBuffer ))
                bRet = pObj->ImportString( *pStrBuffer, nFormatId );

            InvalidateAttribs();
            GetViewData().UpdateInputHandler();
        }
    }
    else if (nFormatId == SotClipboardFormatId::SBA_DATAEXCHANGE)
    {
        //  import of database data into table

        const DataFlavorExVector& rVector = aDataHelper.GetDataFlavorExVector();
        if ( svx::ODataAccessObjectTransferable::canExtractObjectDescriptor(rVector) )
        {
            // transport the whole ODataAccessDescriptor as slot parameter
            svx::ODataAccessDescriptor aDesc = svx::ODataAccessObjectTransferable::extractObjectDescriptor(aDataHelper);
            uno::Any aDescAny;
            uno::Sequence<beans::PropertyValue> aProperties = aDesc.createPropertyValueSequence();
            aDescAny <<= aProperties;
            SfxUnoAnyItem aDataDesc(SID_SBA_IMPORT, aDescAny);

            ScDocShell* pDocSh = GetViewData().GetDocShell();
            SCTAB nTab = GetViewData().GetTabNo();

            ClickCursor(nPosX, nPosY, false);               // set cursor position

            //  Creation of database area "Import1" isn't here, but in the DocShell
            //  slot execute, so it can be added to the undo action

            ScDBData* pDBData = pDocSh->GetDBData( ScRange(nPosX,nPosY,nTab), SC_DB_OLD, ScGetDBSelection::Keep );
            OUString sTarget;
            if (pDBData)
                sTarget = pDBData->GetName();
            else
            {
                ScAddress aCellPos( nPosX,nPosY,nTab );
                sTarget = aCellPos.Format(ScRefFlags::ADDR_ABS_3D, pDoc, pDoc->GetAddressConvention());
            }
            SfxStringItem aTarget(FN_PARAM_1, sTarget);

            bool bAreaIsNew = !pDBData;
            SfxBoolItem aAreaNew(FN_PARAM_2, bAreaIsNew);

            //  asynchronous, to avoid doing the whole import in drop handler
            SfxDispatcher& rDisp = GetViewData().GetDispatcher();
            rDisp.ExecuteList(SID_SBA_IMPORT, SfxCallMode::ASYNCHRON,
                    { &aDataDesc, &aTarget, &aAreaNew });

            bRet = true;
        }
    }
    else if (nFormatId == SotClipboardFormatId::SBA_FIELDDATAEXCHANGE)
    {
        //  insert database field control

        if ( svx::OColumnTransferable::canExtractColumnDescriptor( aDataHelper.GetDataFlavorExVector(), ColumnTransferFormatFlags::COLUMN_DESCRIPTOR | ColumnTransferFormatFlags::CONTROL_EXCHANGE ) )
        {
            MakeDrawLayer();
            ScDrawView* pScDrawView = GetScDrawView();
            SdrObjectUniquePtr pObj = pScDrawView->CreateFieldControl( svx::OColumnTransferable::extractColumnDescriptor( aDataHelper ) );
            if (pObj)
            {
                Point aInsPos = aPos;
                tools::Rectangle aRect(pObj->GetLogicRect());
                aInsPos.AdjustX( -(aRect.GetSize().Width()  / 2) );
                aInsPos.AdjustY( -(aRect.GetSize().Height() / 2) );
                if ( aInsPos.X() < 0 ) aInsPos.setX( 0 );
                if ( aInsPos.Y() < 0 ) aInsPos.setY( 0 );
                aRect.SetPos(aInsPos);
                pObj->SetLogicRect(aRect);

                if ( dynamic_cast<const SdrUnoObj*>( pObj.get() ) !=  nullptr )
                    pObj->NbcSetLayer(SC_LAYER_CONTROLS);
                else
                    pObj->NbcSetLayer(SC_LAYER_FRONT);
                if (dynamic_cast<const SdrObjGroup*>( pObj.get() ) !=  nullptr)
                {
                    SdrObjListIter aIter( *pObj, SdrIterMode::DeepWithGroups );
                    SdrObject* pSubObj = aIter.Next();
                    while (pSubObj)
                    {
                        if ( dynamic_cast<const SdrUnoObj*>( pSubObj) !=  nullptr )
                            pSubObj->NbcSetLayer(SC_LAYER_CONTROLS);
                        else
                            pSubObj->NbcSetLayer(SC_LAYER_FRONT);
                        pSubObj = aIter.Next();
                    }
                }

                pScDrawView->InsertObjectSafe(pObj.release(), *pScDrawView->GetSdrPageView());

                GetViewData().GetViewShell()->SetDrawShell( true );
                bRet = true;
            }
        }
    }
    else if (nFormatId == SotClipboardFormatId::BITMAP || nFormatId == SotClipboardFormatId::PNG)
    {
        BitmapEx aBmpEx;
        if( aDataHelper.GetBitmapEx( SotClipboardFormatId::BITMAP, aBmpEx ) )
            bRet = PasteBitmapEx( aPos, aBmpEx );
    }
    else if (nFormatId == SotClipboardFormatId::GDIMETAFILE)
    {
        GDIMetaFile aMtf;
        if( aDataHelper.GetGDIMetaFile( SotClipboardFormatId::GDIMETAFILE, aMtf ) )
            bRet = PasteMetaFile( aPos, aMtf );
    }
    else if (nFormatId == SotClipboardFormatId::SVXB)
    {
        tools::SvRef<SotStorageStream> xStm;
        if( aDataHelper.GetSotStorageStream( SotClipboardFormatId::SVXB, xStm ) )
        {
            Graphic aGraphic;
            ReadGraphic( *xStm, aGraphic );
            bRet = PasteGraphic( aPos, aGraphic, EMPTY_OUSTRING, EMPTY_OUSTRING );
        }
    }
    else if ( nFormatId == SotClipboardFormatId::DRAWING )
    {
        tools::SvRef<SotStorageStream> xStm;
        if( aDataHelper.GetSotStorageStream( SotClipboardFormatId::DRAWING, xStm ) )
        {
            MakeDrawLayer();    // before loading model, so 3D factory has been created

            ScDocShellRef aDragShellRef( new ScDocShell );
            aDragShellRef->DoInitNew();

            std::unique_ptr<FmFormModel> pModel(
                new FmFormModel(
                    nullptr,
                    aDragShellRef.get()));

            pModel->GetItemPool().FreezeIdRanges();
            xStm->Seek(0);

            css::uno::Reference< css::io::XInputStream > xInputStream( new utl::OInputStreamWrapper( *xStm ) );
            SvxDrawingLayerImport( pModel.get(), xInputStream );

            // set everything to right layer:
            size_t nObjCount = 0;
            sal_uInt16 nPages = pModel->GetPageCount();
            for (sal_uInt16 i=0; i<nPages; i++)
            {
                SdrPage* pPage = pModel->GetPage(i);
                SdrObjListIter aIter( pPage, SdrIterMode::DeepWithGroups );
                SdrObject* pObject = aIter.Next();
                while (pObject)
                {
                    if ( dynamic_cast<const SdrUnoObj*>( pObject) !=  nullptr )
                        pObject->NbcSetLayer(SC_LAYER_CONTROLS);
                    else
                        pObject->NbcSetLayer(SC_LAYER_FRONT);
                    pObject = aIter.Next();
                }

                nObjCount += pPage->GetObjCount();          // count group object only once
            }

            PasteDraw(aPos, pModel.get(), (nObjCount > 1), "A", "B");     // grouped if more than 1 object
            pModel.reset();
            aDragShellRef->DoClose();
            bRet = true;
        }
    }
    else if ( (nFormatId == SotClipboardFormatId::BIFF_5) || (nFormatId == SotClipboardFormatId::BIFF_8) )
    {
        //  do excel import into a clipboard document
        //TODO/MBA: testing
        uno::Reference <io::XInputStream> xStm = aDataHelper.GetInputStream(nFormatId, OUString());
        if (xStm.is())
        {
            std::unique_ptr<ScDocument> pInsDoc(new ScDocument( SCDOCMODE_CLIP ));
            SCTAB nSrcTab = 0;      // Biff5 in clipboard: always sheet 0
            pInsDoc->ResetClip( pDoc, nSrcTab );

            SfxMedium aMed;
            aMed.GetItemSet()->Put( SfxUnoAnyItem( SID_INPUTSTREAM, uno::makeAny( xStm ) ) );
            ErrCode eErr = ScFormatFilter::Get().ScImportExcel( aMed, pInsDoc.get(), EIF_AUTO );
            if ( eErr == ERRCODE_NONE )
            {
                ScRange aSource;
                const ScExtDocOptions* pExtOpt = pInsDoc->GetExtDocOptions();
                const ScExtTabSettings* pTabSett = pExtOpt ? pExtOpt->GetTabSettings( nSrcTab ) : nullptr;
                if( pTabSett && pTabSett->maUsedArea.IsValid() )
                {
                    aSource = pTabSett->maUsedArea;
                    // ensure correct sheet indexes
                    aSource.aStart.SetTab( nSrcTab );
                    aSource.aEnd.SetTab( nSrcTab );
// don't use selection area: if cursor is moved in Excel after Copy, selection
// represents the new cursor position and not the copied area
                }
                else
                {
                    OSL_FAIL("no dimension");   //! possible?
                    SCCOL nFirstCol, nLastCol;
                    SCROW nFirstRow, nLastRow;
                    if ( pInsDoc->GetDataStart( nSrcTab, nFirstCol, nFirstRow ) )
                        pInsDoc->GetCellArea( nSrcTab, nLastCol, nLastRow );
                    else
                    {
                        nFirstCol = nLastCol = 0;
                        nFirstRow = nLastRow = 0;
                    }
                    aSource = ScRange( nFirstCol, nFirstRow, nSrcTab,
                                        nLastCol, nLastRow, nSrcTab );
                }

                if ( pLogicPos )
                {
                    // position specified (Drag&Drop) - change selection
                    MoveCursorAbs( nPosX, nPosY, SC_FOLLOW_NONE, false, false );
                    Unmark();
                }

                pInsDoc->SetClipArea( aSource );
                PasteFromClip( InsertDeleteFlags::ALL, pInsDoc.get(),
                                ScPasteFunc::NONE, false, false, false, INS_NONE, InsertDeleteFlags::NONE,
                                bAllowDialogs );
                bRet = true;
            }
        }
    }
    else if ( nFormatId == SotClipboardFormatId::SIMPLE_FILE )
    {
        OUString aFile;
        if ( aDataHelper.GetString( nFormatId, aFile ) )
            bRet = PasteFile( aPos, aFile, bLink );
    }
    else if ( nFormatId == SotClipboardFormatId::FILE_LIST )
    {
        FileList aFileList;
        if ( aDataHelper.GetFileList( nFormatId, aFileList ) )
        {
            sal_uLong nCount = aFileList.Count();
            for( sal_uLong i = 0; i < nCount ; i++ )
            {
                OUString aFile = aFileList.GetFile( i );

                PasteFile( aPos, aFile, bLink );

                aPos.AdjustX(400 );
                aPos.AdjustY(400 );
            }
            bRet = true;
        }
    }
    else if ( nFormatId == SotClipboardFormatId::SOLK ||
              nFormatId == SotClipboardFormatId::UNIFORMRESOURCELOCATOR ||
              nFormatId == SotClipboardFormatId::NETSCAPE_BOOKMARK ||
              nFormatId == SotClipboardFormatId::FILEGRPDESCRIPTOR )
    {
        bRet = PasteBookmark( nFormatId, rxTransferable, nPosX, nPosY );
    }

    pDoc->SetPastingDrawFromOtherDoc( false );

    return bRet;
}

bool ScViewFunc::PasteLink( const uno::Reference<datatransfer::XTransferable>& rxTransferable )
{
    TransferableDataHelper aDataHelper( rxTransferable );

    //  get link data from transferable before string data,
    //  so the source knows it will be used for a link

    uno::Sequence<sal_Int8> aSequence = aDataHelper.GetSequence(SotClipboardFormatId::LINK, OUString());
    if (!aSequence.getLength())
    {
        OSL_FAIL("DDE Data not found.");
        return false;
    }

    //  check size (only if string is available in transferable)

    sal_uInt16 nCols = 1;
    sal_uInt16 nRows = 1;
    if ( aDataHelper.HasFormat( SotClipboardFormatId::STRING ) )
    {
        OUString aDataStr;
        if ( aDataHelper.GetString( SotClipboardFormatId::STRING, aDataStr ) )
        {
            //  get size from string the same way as in ScDdeLink::DataChanged

            aDataStr = convertLineEnd(aDataStr, LINEEND_LF);
            sal_Int32 nLen = aDataStr.getLength();
            if (nLen && aDataStr[nLen-1] == '\n')
                aDataStr = aDataStr.copy(0, nLen-1);

            if (!aDataStr.isEmpty())
            {
                nRows = comphelper::string::getTokenCount(aDataStr, '\n');
                OUString aLine = aDataStr.getToken( 0, '\n' );
                if (!aLine.isEmpty())
                    nCols = comphelper::string::getTokenCount(aLine, '\t');
            }
        }
    }

    //  create formula

    sal_Int32 nSeqLen = aSequence.getLength();
    const char* p = reinterpret_cast<const char*>(aSequence.getConstArray());

    rtl_TextEncoding eSysEnc = osl_getThreadTextEncoding();

    // char array delimited by \0.
    // app \0 topic \0 item \0 (extra \0) where the extra is optional.
    ::std::vector<OUString> aStrs;
    const char* pStart = p;
    sal_Int32 nStart = 0;
    for (sal_Int32 i = 0; i < nSeqLen; ++i, ++p)
    {
        if (*p == '\0')
        {
            sal_Int32 nLen = i - nStart;
            aStrs.emplace_back(pStart, nLen, eSysEnc);
            nStart = ++i;
            pStart = ++p;
        }
    }

    if (aStrs.size() < 3)
        return false;

    const OUString* pApp   = &aStrs[0];
    const OUString* pTopic = &aStrs[1];
    const OUString* pItem  = &aStrs[2];
    const OUString* pExtra = nullptr;
    if (aStrs.size() > 3)
        pExtra = &aStrs[3];

    if ( pExtra && *pExtra == "calc:extref" )
    {
        // Paste this as an external reference.  Note that paste link always
        // uses Calc A1 syntax even when another formula syntax is specified
        // in the UI.
        EnterMatrix("='"
            + ScGlobal::GetAbsDocName(*pTopic, GetViewData().GetDocument()->GetDocumentShell())
            + "'#" + *pItem
                , ::formula::FormulaGrammar::GRAM_NATIVE);
        return true;
    }
    else
    {
        // DDE in all other cases.

        // TODO: we could define ocQuote for "
        EnterMatrix("=" + ScCompiler::GetNativeSymbol(ocDde)
            + ScCompiler::GetNativeSymbol(ocOpen)
            + "\"" + *pApp + "\""
            + ScCompiler::GetNativeSymbol(ocSep)
            + "\"" + *pTopic + "\""
            + ScCompiler::GetNativeSymbol(ocSep)
            + "\"" + *pItem + "\""
            + ScCompiler::GetNativeSymbol(ocClose)
                , ::formula::FormulaGrammar::GRAM_NATIVE);
    }

    //  mark range

    SCTAB nTab = GetViewData().GetTabNo();
    SCCOL nCurX = GetViewData().GetCurX();
    SCROW nCurY = GetViewData().GetCurY();
    HideAllCursors();
    DoneBlockMode();
    InitBlockMode( nCurX, nCurY, nTab );
    MarkCursor( nCurX+static_cast<SCCOL>(nCols)-1, nCurY+static_cast<SCROW>(nRows)-1, nTab );
    ShowAllCursors();
    CursorPosChanged();

    return true;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
