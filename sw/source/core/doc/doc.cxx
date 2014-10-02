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

#include <doc.hxx>
#include <DocumentSettingManager.hxx>
#include <UndoManager.hxx>
#include <hintids.hxx>

#include <tools/shl.hxx>
#include <tools/globname.hxx>
#include <svx/svxids.hrc>
#include <rtl/random.h>

#include <com/sun/star/i18n/WordType.hpp>
#include <com/sun/star/i18n/ForbiddenCharacters.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/beans/NamedValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/document/XDocumentPropertiesSupplier.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>
#include <comphelper/string.hxx>
#include <tools/urlobj.hxx>
#include <tools/poly.hxx>
#include <tools/multisel.hxx>
#include <rtl/ustring.hxx>
#include <vcl/virdev.hxx>
#include <svl/itemiter.hxx>
#include <svl/poolitem.hxx>
#include <unotools/syslocale.hxx>
#include <sfx2/printer.hxx>
#include <editeng/keepitem.hxx>
#include <editeng/charsetcoloritem.hxx>
#include <editeng/formatbreakitem.hxx>
#include <sfx2/linkmgr.hxx>
#include <svx/svdmodel.hxx>
#include <editeng/pbinitem.hxx>
#include <unotools/charclass.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/timer.hxx>

#include <swatrset.hxx>
#include <swmodule.hxx>
#include <fmtpdsc.hxx>
#include <fmtanchr.hxx>
#include <fmtrfmrk.hxx>
#include <fmtinfmt.hxx>
#include <fmtfld.hxx>
#include <txtfld.hxx>
#include <dbfld.hxx>
#include <txtinet.hxx>
#include <txtrfmrk.hxx>
#include <frmatr.hxx>
#include <pagefrm.hxx>
#include <rootfrm.hxx>
#include <swtable.hxx>
#include <pam.hxx>
#include <ndtxt.hxx>
#include <swundo.hxx>
#include <UndoCore.hxx>
#include <UndoInsert.hxx>
#include <UndoSplitMove.hxx>
#include <UndoTable.hxx>
#include <pagedesc.hxx>
#include <ndole.hxx>
#include <ndgrf.hxx>
#include <rolbck.hxx>
#include <doctxm.hxx>
#include <grfatr.hxx>
#include <poolfmt.hxx>
#include <mvsave.hxx>
#include <SwGrammarMarkUp.hxx>
#include <scriptinfo.hxx>
#include <acorrect.hxx>
#include <mdiexp.hxx>
#include <docstat.hxx>
#include <docary.hxx>
#include <redline.hxx>
#include <fldupde.hxx>
#include <swbaslnk.hxx>
#include <printdata.hxx>
#include <cmdid.h>
#include <statstr.hrc>
#include <comcore.hrc>
#include <SwUndoTOXChange.hxx>
#include <unocrsr.hxx>
#include <docsh.hxx>
#include <viewopt.hxx>
#include <docfld.hxx>
#include <docufld.hxx>
#include <viewsh.hxx>
#include <shellres.hxx>
#include <txtfrm.hxx>
#include <attrhint.hxx>
#include <view.hxx>
#include <DocumentDeviceManager.hxx>

#include <wdocsh.hxx>
#include <prtopt.hxx>
#include <wrtsh.hxx>

#include <vector>
#include <map>

#include <osl/diagnose.h>
#include <osl/interlck.h>
#include <vbahelper/vbaaccesshelper.hxx>

#include "switerator.hxx"

/* @@@MAINTAINABILITY-HORROR@@@
   Probably unwanted dependency on SwDocShell
*/
#include <layouter.hxx>

using namespace ::com::sun::star;

/* IInterface */
sal_Int32 SwDoc::acquire()
{
    OSL_ENSURE(mReferenceCount >= 0, "Negative reference count detected! This is a sign for unbalanced acquire/release calls.");
    return osl_atomic_increment(&mReferenceCount);
}

sal_Int32 SwDoc::release()
{
    OSL_PRECOND(mReferenceCount >= 1, "Object is already released! Releasing it again leads to a negative reference count.");
    return osl_atomic_decrement(&mReferenceCount);
}

sal_Int32 SwDoc::getReferenceCount() const
{
    OSL_ENSURE(mReferenceCount >= 0, "Negative reference count detected! This is a sign for unbalanced acquire/release calls.");
    return mReferenceCount;
}

/* IDocumentSettingAccess */
bool SwDoc::get(/*[in]*/ DocumentSettingId id) const
{
    return m_pDocumentSettingManager->get(id);
}

void SwDoc::set(/*[in]*/ DocumentSettingId id, /*[in]*/ bool value)
{
    m_pDocumentSettingManager->set(id,value);
}

const i18n::ForbiddenCharacters*
    SwDoc::getForbiddenCharacters(/*[in]*/ sal_uInt16 nLang, /*[in]*/ bool bLocaleData ) const
{
    return m_pDocumentSettingManager->getForbiddenCharacters(nLang,bLocaleData);
}

void SwDoc::setForbiddenCharacters(/*[in]*/ sal_uInt16 nLang,
                                   /*[in]*/ const com::sun::star::i18n::ForbiddenCharacters& rFChars )
{
    m_pDocumentSettingManager->setForbiddenCharacters(nLang,rFChars);
}

rtl::Reference<SvxForbiddenCharactersTable>& SwDoc::getForbiddenCharacterTable()
{
    return m_pDocumentSettingManager->getForbiddenCharacterTable();
}

const rtl::Reference<SvxForbiddenCharactersTable>& SwDoc::getForbiddenCharacterTable() const
{
    return m_pDocumentSettingManager->getForbiddenCharacterTable();
}

sal_uInt16 SwDoc::getLinkUpdateMode( /*[in]*/bool bGlobalSettings ) const
{
    return m_pDocumentSettingManager->getLinkUpdateMode(bGlobalSettings);
}

void SwDoc::setLinkUpdateMode( /*[in]*/sal_uInt16 eMode )
{
    m_pDocumentSettingManager->setLinkUpdateMode(eMode);
}

sal_uInt32 SwDoc::getRsid() const
{
    return mnRsid;
}

void SwDoc::setRsid( sal_uInt32 nVal )
{
    static bool bHack = (getenv("LIBO_ONEWAY_STABLE_ODF_EXPORT") != NULL);

    sal_uInt32 nIncrease = 0;
    if (!bHack)
    {
        // Increase the rsid with a random number smaller than 2^17. This way we
        // expect to be able to edit a document 2^12 times before rsid overflows.
        static rtlRandomPool aPool = rtl_random_createPool();
        rtl_random_getBytes( aPool, &nIncrease, sizeof ( nIncrease ) );
        nIncrease &= ( 1<<17 ) - 1;
        nIncrease++; // make sure the new rsid is not the same
    }
    mnRsid = nVal + nIncrease;
}

sal_uInt32 SwDoc::getRsidRoot() const
{
    return mnRsidRoot;
}

void SwDoc::setRsidRoot( sal_uInt32 nVal )
{
    mnRsidRoot = nVal;
}

SwFldUpdateFlags SwDoc::getFieldUpdateFlags( /*[in]*/bool bGlobalSettings ) const
{
    return m_pDocumentSettingManager->getFieldUpdateFlags(bGlobalSettings);
}

void SwDoc::setFieldUpdateFlags(/*[in]*/SwFldUpdateFlags eMode )
{
    m_pDocumentSettingManager->setFieldUpdateFlags(eMode);
}

SwCharCompressType SwDoc::getCharacterCompressionType() const
{
    return m_pDocumentSettingManager->getCharacterCompressionType();
}

void SwDoc::setCharacterCompressionType( /*[in]*/SwCharCompressType n )
{
    m_pDocumentSettingManager->setCharacterCompressionType(n);
}

// IDocumentDeviceAccess
const IDocumentDeviceAccess* SwDoc::getIDocumentDeviceAccessConst() const
{
    return m_DeviceAccess.get();
}

IDocumentDeviceAccess* SwDoc::getIDocumentDeviceAccess()
{
    return m_DeviceAccess.get();
}

SfxPrinter* SwDoc::getPrinter(/*[in]*/ bool bCreate ) const
{
    return getIDocumentDeviceAccessConst()->getPrinter( bCreate );
}

void SwDoc::setPrinter(/*[in]*/ SfxPrinter* pP,/*[in]*/ bool bDeleteOld,/*[in]*/ bool bCallPrtDataChanged )
{
    getIDocumentDeviceAccess()->setPrinter( pP, bDeleteOld, bCallPrtDataChanged );
}

VirtualDevice* SwDoc::getVirtualDevice(/*[in]*/ bool bCreate ) const
{
    return getIDocumentDeviceAccessConst()->getVirtualDevice( bCreate );
}

void SwDoc::setVirtualDevice(/*[in]*/ VirtualDevice* pVd,/*[in]*/ bool bDeleteOld, /*[in]*/ bool bCallVirDevDataChanged )
{
    getIDocumentDeviceAccess()->setVirtualDevice( pVd, bDeleteOld, bCallVirDevDataChanged );
}

OutputDevice* SwDoc::getReferenceDevice(/*[in]*/ bool bCreate ) const
{
    return getIDocumentDeviceAccessConst()->getReferenceDevice( bCreate );
}

void SwDoc::setReferenceDeviceType(/*[in]*/ bool bNewVirtual,/*[in]*/ bool bNewHiRes )
{
    getIDocumentDeviceAccess()->setReferenceDeviceType( bNewVirtual, bNewHiRes );
}

const JobSetup* SwDoc::getJobsetup() const
{
    return getIDocumentDeviceAccessConst()->getJobsetup();
}

void SwDoc::setJobsetup(/*[in]*/ const JobSetup& rJobSetup )
{
    getIDocumentDeviceAccess()->setJobsetup( rJobSetup );
}

const SwPrintData & SwDoc::getPrintData() const
{
    return getIDocumentDeviceAccessConst()->getPrintData();
}

void SwDoc::setPrintData(/*[in]*/ const SwPrintData& rPrtData)
{
    getIDocumentDeviceAccess()->setPrintData( rPrtData );
}


/* Implementations the next Interface here */

/*
 * Document editing (Doc-SS) to fill the document
 * by the RTF parser and for the EditShell.
 */
void SwDoc::ChgDBData(const SwDBData& rNewData)
{
    if( rNewData != maDBData )
    {
        maDBData = rNewData;
        SetModified();
    }
    GetSysFldType(RES_DBNAMEFLD)->UpdateFlds();
}

bool SwDoc::SplitNode( const SwPosition &rPos, bool bChkTableStart )
{
    SwCntntNode *pNode = rPos.nNode.GetNode().GetCntntNode();
    if(0 == pNode)
        return false;

    {
        // BUG 26675: Send DataChanged before deleting, so that we notice which objects are in scope.
        //            After that they can be before/after the position.
        SwDataChanged aTmp( this, rPos );
    }

    SwUndoSplitNode* pUndo = 0;
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().ClearRedo();
        // insert the Undo object (currently only for TextNode)
        if( pNode->IsTxtNode() )
        {
            pUndo = new SwUndoSplitNode( this, rPos, bChkTableStart );
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }
    }

    // Update the rsid of the old and the new node unless
    // the old node is split at the beginning or at the end
    SwTxtNode *pTxtNode =  rPos.nNode.GetNode().GetTxtNode();
    const sal_Int32 nPos = rPos.nContent.GetIndex();
    if( pTxtNode && nPos && nPos != pTxtNode->Len() )
    {
        UpdateParRsid( pTxtNode );
    }

    //JP 28.01.97: Special case for SplitNode at table start:
    //             If it is at the beginning of a Doc/Fly/Footer/... or right at after a table
    //             then insert a paragraph before it.
    if( bChkTableStart && !rPos.nContent.GetIndex() && pNode->IsTxtNode() )
    {
        sal_uLong nPrevPos = rPos.nNode.GetIndex() - 1;
        const SwTableNode* pTblNd;
        const SwNode* pNd = GetNodes()[ nPrevPos ];
        if( pNd->IsStartNode() &&
            SwTableBoxStartNode == ((SwStartNode*)pNd)->GetStartNodeType() &&
            0 != ( pTblNd = GetNodes()[ --nPrevPos ]->GetTableNode() ) &&
            ((( pNd = GetNodes()[ --nPrevPos ])->IsStartNode() &&
               SwTableBoxStartNode != ((SwStartNode*)pNd)->GetStartNodeType() )
               || ( pNd->IsEndNode() && pNd->StartOfSectionNode()->IsTableNode() )
               || pNd->IsCntntNode() ))
        {
            if( pNd->IsCntntNode() )
            {
                //JP 30.04.99 Bug 65660:
                // There are no page breaks outside of the normal body area,
                // so this is not a valid condition to insert a paragraph.
                if( nPrevPos < GetNodes().GetEndOfExtras().GetIndex() )
                    pNd = 0;
                else
                {
                    // Only if the table has page breaks!
                    const SwFrmFmt* pFrmFmt = pTblNd->GetTable().GetFrmFmt();
                    if( SFX_ITEM_SET != pFrmFmt->GetItemState(RES_PAGEDESC, false) &&
                        SFX_ITEM_SET != pFrmFmt->GetItemState( RES_BREAK, false ) )
                        pNd = 0;
                }
            }

            if( pNd )
            {
                SwTxtNode* pTxtNd = GetNodes().MakeTxtNode(
                                        SwNodeIndex( *pTblNd ),
                                        GetTxtCollFromPool( RES_POOLCOLL_TEXT ));
                if( pTxtNd )
                {
                    ((SwPosition&)rPos).nNode = pTblNd->GetIndex()-1;
                    ((SwPosition&)rPos).nContent.Assign( pTxtNd, 0 );

                    // only add page breaks/styles to the body area
                    if( nPrevPos > GetNodes().GetEndOfExtras().GetIndex() )
                    {
                        SwFrmFmt* pFrmFmt = pTblNd->GetTable().GetFrmFmt();
                        const SfxPoolItem *pItem;
                        if( SFX_ITEM_SET == pFrmFmt->GetItemState( RES_PAGEDESC,
                            false, &pItem ) )
                        {
                            pTxtNd->SetAttr( *pItem );
                            pFrmFmt->ResetFmtAttr( RES_PAGEDESC );
                        }
                        if( SFX_ITEM_SET == pFrmFmt->GetItemState( RES_BREAK,
                            false, &pItem ) )
                        {
                            pTxtNd->SetAttr( *pItem );
                            pFrmFmt->ResetFmtAttr( RES_BREAK );
                        }
                    }

                    if( pUndo )
                        pUndo->SetTblFlag();
                    SetModified();
                    return true;
                }
            }
        }
    }

    std::vector<sal_uLong> aBkmkArr;
    _SaveCntntIdx( this, rPos.nNode.GetIndex(), rPos.nContent.GetIndex(),
                    aBkmkArr, SAVEFLY_SPLIT );
    // FIXME: only SwTxtNode has a valid implementation of SplitCntntNode!
    OSL_ENSURE(pNode->IsTxtNode(), "splitting non-text node?");
    pNode = pNode->SplitCntntNode( rPos );
    if (pNode)
    {
        // move all bookmarks, TOXMarks, FlyAtCnt
        if( !aBkmkArr.empty() )
            _RestoreCntntIdx( this, aBkmkArr, rPos.nNode.GetIndex()-1, 0, true );

        // To-Do - add 'SwExtraRedlineTbl' also ?
        if( IsRedlineOn() || (!IsIgnoreRedline() && !mpRedlineTbl->empty() ))
        {
            SwPaM aPam( rPos );
            aPam.SetMark();
            aPam.Move( fnMoveBackward );
            if( IsRedlineOn() )
                AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, aPam ), true);
            else
                SplitRedline( aPam );
        }
    }

    SetModified();
    return true;
}

bool SwDoc::AppendTxtNode( SwPosition& rPos )
{
    // create new node before EndOfContent
    SwTxtNode * pCurNode = rPos.nNode.GetNode().GetTxtNode();
    if( !pCurNode )
    {
        // so then one can be created!
        SwNodeIndex aIdx( rPos.nNode, 1 );
        pCurNode = GetNodes().MakeTxtNode( aIdx,
                        GetTxtCollFromPool( RES_POOLCOLL_STANDARD ));
    }
    else
        pCurNode = (SwTxtNode*)pCurNode->AppendNode( rPos );

    rPos.nNode++;
    rPos.nContent.Assign( pCurNode, 0 );

    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().AppendUndo( new SwUndoInsert( rPos.nNode ) );
    }

    // To-Do - add 'SwExtraRedlineTbl' also ?
    if( IsRedlineOn() || (!IsIgnoreRedline() && !mpRedlineTbl->empty() ))
    {
        SwPaM aPam( rPos );
        aPam.SetMark();
        aPam.Move( fnMoveBackward );
        if( IsRedlineOn() )
            AppendRedline( new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, aPam ), true);
        else
            SplitRedline( aPam );
    }

    SetModified();
    return true;
}

bool SwDoc::InsertString( const SwPaM &rRg, const OUString &rStr,
        const enum InsertFlags nInsertMode )
{
    // fetching DoesUndo is surprisingly expensive
    bool bDoesUndo = GetIDocumentUndoRedo().DoesUndo();
    if (bDoesUndo)
        GetIDocumentUndoRedo().ClearRedo(); // AppendUndo not always called!

    const SwPosition& rPos = *rRg.GetPoint();

    if( mpACEWord )                  // add to auto correction
    {
        if( 1 == rStr.getLength() && mpACEWord->IsDeleted() )
        {
            mpACEWord->CheckChar( rPos, rStr[ 0 ] );
        }
        delete mpACEWord, mpACEWord = 0;
    }

    SwTxtNode *const pNode = rPos.nNode.GetNode().GetTxtNode();
    if(!pNode)
        return false;

    SwDataChanged aTmp( rRg );

    if (!bDoesUndo || !GetIDocumentUndoRedo().DoesGroupUndo())
    {
        OUString const ins(pNode->InsertText(rStr, rPos.nContent, nInsertMode));
        if (bDoesUndo)
        {
            SwUndoInsert * const pUndo( new SwUndoInsert(rPos.nNode,
                    rPos.nContent.GetIndex(), ins.getLength(), nInsertMode));
            GetIDocumentUndoRedo().AppendUndo(pUndo);
        }
    }
    else
    {   // if Undo and grouping is enabled, everything changes!
        SwUndoInsert * pUndo = NULL;

        // don't group the start if hints at the start should be expanded
        if (!(nInsertMode & IDocumentContentOperations::INS_FORCEHINTEXPAND))
        {
            SwUndo *const pLastUndo = GetUndoManager().GetLastUndo();
            SwUndoInsert *const pUndoInsert(
                dynamic_cast<SwUndoInsert *>(pLastUndo) );
            if (pUndoInsert && pUndoInsert->CanGrouping(rPos))
            {
                pUndo = pUndoInsert;
            }
        }

        CharClass const& rCC = GetAppCharClass();
        sal_Int32 nInsPos = rPos.nContent.GetIndex();

        if (!pUndo)
        {
            pUndo = new SwUndoInsert( rPos.nNode, nInsPos, 0, nInsertMode,
                            !rCC.isLetterNumeric( rStr, 0 ) );
            GetIDocumentUndoRedo().AppendUndo( pUndo );
        }

        OUString const ins(pNode->InsertText(rStr, rPos.nContent, nInsertMode));

        for (sal_Int32 i = 0; i < ins.getLength(); ++i)
        {
            nInsPos++;
            // if CanGrouping() returns true, everything has already been done
            if (!pUndo->CanGrouping(ins[i]))
            {
                pUndo = new SwUndoInsert(rPos.nNode, nInsPos, 1, nInsertMode,
                            !rCC.isLetterNumeric(ins, i));
                GetIDocumentUndoRedo().AppendUndo( pUndo );
            }
        }
    }

    // To-Do - add 'SwExtraRedlineTbl' also ?
    if( IsRedlineOn() || (!IsIgnoreRedline() && !mpRedlineTbl->empty() ))
    {
        SwPaM aPam( rPos.nNode, aTmp.GetCntnt(),
                    rPos.nNode, rPos.nContent.GetIndex());
        if( IsRedlineOn() )
        {
            AppendRedline(
                new SwRangeRedline( nsRedlineType_t::REDLINE_INSERT, aPam ), true);
        }
        else
        {
            SplitRedline( aPam );
        }
    }

    SetModified();
    return true;
}

SwFlyFrmFmt* SwDoc::_InsNoTxtNode( const SwPosition& rPos, SwNoTxtNode* pNode,
                                    const SfxItemSet* pFlyAttrSet,
                                    const SfxItemSet* pGrfAttrSet,
                                    SwFrmFmt* pFrmFmt)
{
    SwFlyFrmFmt *pFmt = 0;
    if( pNode )
    {
        pFmt = _MakeFlySection( rPos, *pNode, FLY_AT_PARA,
                                pFlyAttrSet, pFrmFmt );
        if( pGrfAttrSet )
            pNode->SetAttr( *pGrfAttrSet );
    }
    return pFmt;
}

SwFlyFrmFmt* SwDoc::Insert( const SwPaM &rRg,
                            const OUString& rGrfName,
                            const OUString& rFltName,
                            const Graphic* pGraphic,
                            const SfxItemSet* pFlyAttrSet,
                            const SfxItemSet* pGrfAttrSet,
                            SwFrmFmt* pFrmFmt )
{
    if( !pFrmFmt )
        pFrmFmt = GetFrmFmtFromPool( RES_POOLFRM_GRAPHIC );
    SwGrfNode* pSwGrfNode = GetNodes().MakeGrfNode(
                            SwNodeIndex( GetNodes().GetEndOfAutotext() ),
                            rGrfName, rFltName, pGraphic,
                            mpDfltGrfFmtColl );
    SwFlyFrmFmt* pSwFlyFrmFmt = _InsNoTxtNode( *rRg.GetPoint(), pSwGrfNode,
                            pFlyAttrSet, pGrfAttrSet, pFrmFmt );
    pSwGrfNode->onGraphicChanged();
    return pSwFlyFrmFmt;
}

SwFlyFrmFmt* SwDoc::Insert( const SwPaM &rRg, const GraphicObject& rGrfObj,
                            const SfxItemSet* pFlyAttrSet,
                            const SfxItemSet* pGrfAttrSet,
                            SwFrmFmt* pFrmFmt )
{
    if( !pFrmFmt )
        pFrmFmt = GetFrmFmtFromPool( RES_POOLFRM_GRAPHIC );
    SwGrfNode* pSwGrfNode = GetNodes().MakeGrfNode(
                            SwNodeIndex( GetNodes().GetEndOfAutotext() ),
                            rGrfObj, mpDfltGrfFmtColl );
    SwFlyFrmFmt* pSwFlyFrmFmt = _InsNoTxtNode( *rRg.GetPoint(), pSwGrfNode,
                            pFlyAttrSet, pGrfAttrSet, pFrmFmt );
    pSwGrfNode->onGraphicChanged();
    return pSwFlyFrmFmt;
}

SwFlyFrmFmt* SwDoc::Insert(const SwPaM &rRg, const svt::EmbeddedObjectRef& xObj,
                        const SfxItemSet* pFlyAttrSet,
                        const SfxItemSet* pGrfAttrSet,
                        SwFrmFmt* pFrmFmt )
{
    if( !pFrmFmt )
    {
        sal_uInt16 nId = RES_POOLFRM_OLE;
        SvGlobalName aClassName( xObj->getClassID() );
        if (SotExchange::IsMath(aClassName))
            nId = RES_POOLFRM_FORMEL;

        pFrmFmt = GetFrmFmtFromPool( nId );
    }
    return _InsNoTxtNode( *rRg.GetPoint(), GetNodes().MakeOLENode(
                            SwNodeIndex( GetNodes().GetEndOfAutotext() ),
                            xObj,
                            mpDfltGrfFmtColl ),
                            pFlyAttrSet, pGrfAttrSet,
                            pFrmFmt );
}

SwFlyFrmFmt* SwDoc::InsertOLE(const SwPaM &rRg, const OUString& rObjName,
                        sal_Int64 nAspect,
                        const SfxItemSet* pFlyAttrSet,
                        const SfxItemSet* pGrfAttrSet,
                        SwFrmFmt* pFrmFmt )
{
    if( !pFrmFmt )
        pFrmFmt = GetFrmFmtFromPool( RES_POOLFRM_OLE );

    return _InsNoTxtNode( *rRg.GetPoint(),
                            GetNodes().MakeOLENode(
                                SwNodeIndex( GetNodes().GetEndOfAutotext() ),
                                rObjName,
                                nAspect,
                                mpDfltGrfFmtColl,
                                0 ),
                            pFlyAttrSet, pGrfAttrSet,
                            pFrmFmt );
}

/// @returns the field type of the Doc
SwFieldType *SwDoc::GetSysFldType( const sal_uInt16 eWhich ) const
{
    for( sal_uInt16 i = 0; i < INIT_FLDTYPES; ++i )
        if( eWhich == (*mpFldTypes)[i]->Which() )
            return (*mpFldTypes)[i];
    return 0;
}

void SwDoc::SetDocStat( const SwDocStat& rStat )
{
    *mpDocStat = rStat;
}

const SwDocStat& SwDoc::GetDocStat() const
{
    return *mpDocStat;
}

const SwDocStat& SwDoc::GetUpdatedDocStat( bool bCompleteAsync, bool bFields )
{
    if( mpDocStat->bModified )
    {
        UpdateDocStat( bCompleteAsync, bFields );
    }
    return *mpDocStat;
}

struct _PostItFld : public _SetGetExpFld
{
    _PostItFld( const SwNodeIndex& rNdIdx, const SwTxtFld* pFld,  const SwIndex* pIdx = 0 )
        : _SetGetExpFld( rNdIdx, pFld, pIdx ) {}

    sal_uInt16 GetPageNo( const StringRangeEnumerator &rRangeEnum,
            const std::set< sal_Int32 > &rPossiblePages,
            sal_uInt16& rVirtPgNo, sal_uInt16& rLineNo );

    SwPostItField* GetPostIt() const
    {
        return (SwPostItField*) GetTxtFld()->GetFmtFld().GetField();
    }
};

sal_uInt16 _PostItFld::GetPageNo(
    const StringRangeEnumerator &rRangeEnum,
    const std::set< sal_Int32 > &rPossiblePages,
    /* out */ sal_uInt16& rVirtPgNo, /* out */ sal_uInt16& rLineNo )
{
    //Problem: If a PostItFld is contained in a Node that is represented
    //by more than one layout instance,
    //we have to decide whether it should be printed once or n-times.
    //Probably only once. For the page number we don't select a random one,
    //but the PostIt's first occurrence in the selected area.
    rVirtPgNo = 0;
    sal_uInt16 nPos = GetCntnt();
    SwIterator<SwTxtFrm,SwTxtNode> aIter( GetTxtFld()->GetTxtNode() );
    for( SwTxtFrm* pFrm = aIter.First(); pFrm;  pFrm = aIter.Next() )
    {
        if( pFrm->GetOfst() > nPos ||
            (pFrm->HasFollow() && pFrm->GetFollow()->GetOfst() <= nPos) )
            continue;
        sal_uInt16 nPgNo = pFrm->GetPhyPageNum();
        if( rRangeEnum.hasValue( nPgNo, &rPossiblePages ))
        {
            rLineNo = (sal_uInt16)(pFrm->GetLineCount( nPos ) +
                      pFrm->GetAllLines() - pFrm->GetThisLines());
            rVirtPgNo = pFrm->GetVirtPageNum();
            return nPgNo;
        }
    }
    return 0;
}

bool sw_GetPostIts(
    IDocumentFieldsAccess* pIDFA,
    _SetGetExpFlds * pSrtLst )
{
    bool bHasPostIts = false;

    SwFieldType* pFldType = pIDFA->GetSysFldType( RES_POSTITFLD );
    OSL_ENSURE( pFldType, "no PostItType ? ");

    if( pFldType->GetDepends() )
    {
        // Found modify object; insert all fields into the array
        SwIterator<SwFmtFld,SwFieldType> aIter( *pFldType );
        const SwTxtFld* pTxtFld;
        for( SwFmtFld* pFld = aIter.First(); pFld;  pFld = aIter.Next() )
        {
            if( 0 != ( pTxtFld = pFld->GetTxtFld() ) &&
                pTxtFld->GetTxtNode().GetNodes().IsDocNodes() )
            {
                bHasPostIts = true;
                if (pSrtLst)
                {
                    SwNodeIndex aIdx( pTxtFld->GetTxtNode() );
                    _PostItFld* pNew = new _PostItFld( aIdx, pTxtFld );
                    pSrtLst->insert( pNew );
                }
                else
                    break;  // we just wanted to check for the existence of postits ...
            }
        }
    }

    return bHasPostIts;
}

static void lcl_FormatPostIt(
    IDocumentContentOperations* pIDCO,
    SwPaM& aPam,
    SwPostItField* pField,
    bool bNewPage, bool bIsFirstPostIt,
    sal_uInt16 nPageNo, sal_uInt16 nLineNo )
{
    static char const sTmp[] = " : ";

    OSL_ENSURE( SwViewShell::GetShellRes(), "missing ShellRes" );

    if (bNewPage)
    {
        pIDCO->InsertPoolItem( aPam, SvxFmtBreakItem( SVX_BREAK_PAGE_AFTER, RES_BREAK ), 0 );
        pIDCO->SplitNode( *aPam.GetPoint(), false );
    }
    else if (!bIsFirstPostIt)
    {
        // add an empty line between different notes
        pIDCO->SplitNode( *aPam.GetPoint(), false );
        pIDCO->SplitNode( *aPam.GetPoint(), false );
    }

    OUString aStr( SwViewShell::GetShellRes()->aPostItPage );
    aStr += sTmp;

    aStr += OUString::number( nPageNo );
    aStr += " ";
    if( nLineNo )
    {
        aStr += SwViewShell::GetShellRes()->aPostItLine;
        aStr += sTmp;
        aStr += OUString::number( nLineNo );
        aStr += " ";
    }
    aStr += SwViewShell::GetShellRes()->aPostItAuthor;
    aStr += sTmp;
    aStr += pField->GetPar1();
    aStr += " ";
    SvtSysLocale aSysLocale;
    aStr += /*(LocaleDataWrapper&)*/aSysLocale.GetLocaleData().getDate( pField->GetDate() );
    pIDCO->InsertString( aPam, aStr );

    pIDCO->SplitNode( *aPam.GetPoint(), false );
    aStr = pField->GetPar2();
#if defined( WNT )
    // Throw out all CR in Windows
    aStr = comphelper::string::remove(aStr, '\r');
#endif
    pIDCO->InsertString( aPam, aStr );
}

/// provide the paper tray to use according to the page style in use,
/// but do that only if the respective item is NOT just the default item
static sal_Int32 lcl_GetPaperBin( const SwPageFrm *pStartFrm )
{
    sal_Int32 nRes = -1;

    const SwFrmFmt &rFmt = pStartFrm->GetPageDesc()->GetMaster();
    const SfxPoolItem *pItem = NULL;
    SfxItemState eState = rFmt.GetItemState( RES_PAPER_BIN, false, &pItem );
    const SvxPaperBinItem *pPaperBinItem = dynamic_cast< const SvxPaperBinItem * >(pItem);
    if (eState > SFX_ITEM_DEFAULT && pPaperBinItem)
        nRes = pPaperBinItem->GetValue();

    return nRes;
}

void SwDoc::CalculatePagesForPrinting(
    const SwRootFrm& rLayout,
    /* out */ SwRenderData &rData,
    const SwPrintUIOptions &rOptions,
    bool bIsPDFExport,
    sal_Int32 nDocPageCount )
{
    const sal_Int64 nContent = rOptions.getIntValue( "PrintContent", 0 );
    const bool bPrintSelection = nContent == 2;

    // properties to take into account when calculating the set of pages
    // (PDF export UI does not allow for selecting left or right pages only)
    bool bPrintLeftPages    = bIsPDFExport || rOptions.IsPrintLeftPages();
    bool bPrintRightPages   = bIsPDFExport || rOptions.IsPrintRightPages();
    // #i103700# printing selections should not allow for automatic inserting empty pages
    bool bPrintEmptyPages   = bPrintSelection ? false : rOptions.IsPrintEmptyPages( bIsPDFExport );

    std::map< sal_Int32, sal_Int32 > &rPrinterPaperTrays = rData.GetPrinterPaperTrays();
    std::set< sal_Int32 > &rValidPages = rData.GetValidPagesSet();
    rValidPages.clear();

    sal_Int32 nPageNum = 1;
    const SwPageFrm *pStPage = dynamic_cast<const SwPageFrm*>( rLayout.Lower() );
    while (pStPage && nPageNum <= nDocPageCount)
    {
        const bool bPrintThisPage =
            ( (bPrintRightPages && pStPage->OnRightPage()) ||
              (bPrintLeftPages && !pStPage->OnRightPage()) ) &&
            ( bPrintEmptyPages || pStPage->Frm().Height() );

        if (bPrintThisPage)
        {
            rValidPages.insert( nPageNum );
            rPrinterPaperTrays[ nPageNum ] = lcl_GetPaperBin( pStPage );
        }

        ++nPageNum;
        pStPage = (SwPageFrm*)pStPage->GetNext();
    }

    // now that we have identified the valid pages for printing according
    // to the print settings we need to get the PageRange to use and
    // use both results to get the actual pages to be printed
    // (post-it settings need to be taken into account later on!)

    // get PageRange value to use
    OUString aPageRange;
    // #i116085# - adjusting fix for i113919
    if ( !bIsPDFExport )
    {
        // PageContent :
        // 0 -> print all pages (default if aPageRange is empty)
        // 1 -> print range according to PageRange
        // 2 -> print selection
        if (1 == nContent)
            aPageRange = rOptions.getStringValue( "PageRange", OUString() );
        if (2 == nContent)
        {
            // note that printing selections is actually implemented by copying
            // the selection to a new temporary document and printing all of that one.
            // Thus for Writer "PrintContent" must never be 2.
            // See SwXTextDocument::GetRenderDoc for evaluating if a selection is to be
            // printed and for creating the temporary document.
        }

        // please note
    }
    if (aPageRange.isEmpty())    // empty string -> print all
    {
        // set page range to print to 'all pages'
        aPageRange = OUString::number( 1 ) + "-" + OUString::number( nDocPageCount );
    }
    rData.SetPageRange( aPageRange );

    // get vector of pages to print according to PageRange and valid pages set from above
    // (result may be an empty vector, for example if the range string is not correct)
    StringRangeEnumerator::getRangesFromString(
            aPageRange, rData.GetPagesToPrint(),
            1, nDocPageCount, 0, &rData.GetValidPagesSet() );
}

void SwDoc::UpdatePagesForPrintingWithPostItData(
    /* out */ SwRenderData &rData,
    const SwPrintUIOptions &rOptions,
    bool /*bIsPDFExport*/,
    sal_Int32 nDocPageCount )
{

    sal_Int16 nPostItMode = (sal_Int16) rOptions.getIntValue( "PrintAnnotationMode", 0 );
    OSL_ENSURE(nPostItMode == POSTITS_NONE || rData.HasPostItData(),
            "print post-its without post-it data?" );
    const sal_uInt16 nPostItCount = rData.HasPostItData() ? rData.m_pPostItFields->size() : 0;
    if (nPostItMode != POSTITS_NONE && nPostItCount > 0)
    {
        SET_CURR_SHELL( rData.m_pPostItShell.get() );

        // clear document and move to end of it
        SwDoc & rPostItDoc(*rData.m_pPostItShell->GetDoc());
        SwPaM aPam(rPostItDoc.GetNodes().GetEndOfContent());
        aPam.Move( fnMoveBackward, fnGoDoc );
        aPam.SetMark();
        aPam.Move( fnMoveForward, fnGoDoc );
        rPostItDoc.DeleteRange( aPam );

        const StringRangeEnumerator aRangeEnum( rData.GetPageRange(), 1, nDocPageCount, 0 );

        // For mode POSTITS_ENDPAGE:
        // maps a physical page number to the page number in post-it document that holds
        // the first post-it for that physical page . Needed to relate the correct start frames
        // from the post-it doc to the physical page of the document
        std::map< sal_Int32, sal_Int32 >  aPostItLastStartPageNum;

        // add all post-its on valid pages within the page range to the
        // temporary post-it document.
        // Since the array of post-it fields is sorted by page and line number we will
        // already get them in the correct order
        sal_uInt16 nVirtPg = 0, nLineNo = 0, nLastPageNum = 0, nPhyPageNum = 0;
        bool bIsFirstPostIt = true;
        for (sal_uInt16 i = 0; i < nPostItCount; ++i)
        {
            _PostItFld& rPostIt = (_PostItFld&)*(*rData.m_pPostItFields)[ i ];
            nLastPageNum = nPhyPageNum;
            nPhyPageNum = rPostIt.GetPageNo(
                    aRangeEnum, rData.GetValidPagesSet(), nVirtPg, nLineNo );
            if (nPhyPageNum)
            {
                // need to insert a page break?
                // In POSTITS_ENDPAGE mode for each document page the following
                // post-it page needs to start on a new page
                const bool bNewPage = nPostItMode == POSTITS_ENDPAGE &&
                        !bIsFirstPostIt && nPhyPageNum != nLastPageNum;

                lcl_FormatPostIt( rData.m_pPostItShell->GetDoc(), aPam,
                        rPostIt.GetPostIt(), bNewPage, bIsFirstPostIt, nVirtPg, nLineNo );
                bIsFirstPostIt = false;

                if (nPostItMode == POSTITS_ENDPAGE)
                {
                    // get the correct number of current pages for the post-it document
                    rData.m_pPostItShell->CalcLayout();
                    const sal_Int32 nPages = rData.m_pPostItShell->GetPageCount();
                    aPostItLastStartPageNum[ nPhyPageNum ] = nPages;
                }
            }
        }

        // format post-it doc to get correct number of pages
        rData.m_pPostItShell->CalcLayout();
        const sal_Int32 nPostItDocPageCount = rData.m_pPostItShell->GetPageCount();

        if (nPostItMode == POSTITS_ONLY || nPostItMode == POSTITS_ENDDOC)
        {
            // now add those post-it pages to the vector of pages to print
            // or replace them if only post-its should be printed

            if (nPostItMode == POSTITS_ONLY)
            {
                // no document page to be printed
                rData.GetPagesToPrint().clear();
            }

            // now we just need to add the post-it pages to be printed to the
            // end of the vector of pages to print
            sal_Int32 nPageNum = 0;
            const SwPageFrm * pPageFrm = (SwPageFrm*)rData.m_pPostItShell->GetLayout()->Lower();
            while( pPageFrm && nPageNum < nPostItDocPageCount )
            {
                OSL_ENSURE( pPageFrm, "Empty page frame. How are we going to print this?" );
                ++nPageNum;
                // negative page number indicates page is from the post-it doc
                rData.GetPagesToPrint().push_back( -nPageNum );
                OSL_ENSURE( pPageFrm, "pPageFrm is NULL!" );
                pPageFrm = (SwPageFrm*)pPageFrm->GetNext();
            }
            OSL_ENSURE( nPageNum == nPostItDocPageCount, "unexpected number of pages" );
        }
        else if (nPostItMode == POSTITS_ENDPAGE)
        {
            // the next step is to find all the pages from the post-it
            // document that should be printed for a given physical page
            // of the document

            std::vector< sal_Int32 >            aTmpPagesToPrint;
            sal_Int32 nLastPostItPage(0);
            const size_t nNum = rData.GetPagesToPrint().size();
            for (size_t i = 0 ;  i < nNum;  ++i)
            {
                // add the physical page to print from the document
                const sal_Int32 nPhysPage = rData.GetPagesToPrint()[i];
                aTmpPagesToPrint.push_back( nPhysPage );

                // add the post-it document pages to print, i.e those
                // post-it pages that have the data for the above physical page
                ::std::map<sal_Int32, sal_Int32>::const_iterator const iter(
                        aPostItLastStartPageNum.find(nPhysPage));
                if (iter != aPostItLastStartPageNum.end())
                {
                    for (sal_Int32 j = nLastPostItPage + 1;
                            j <= iter->second; ++j)
                    {
                        // negative page number indicates page is from the
                        aTmpPagesToPrint.push_back(-j); // post-it document
                    }
                    nLastPostItPage = iter->second;
                }
            }

            // finally we need to assign those vectors to the resulting ones.
            // swapping the data should be more efficient than assigning since
            // we won't need the temporary vectors anymore
            rData.GetPagesToPrint().swap( aTmpPagesToPrint );
        }
    }
}

void SwDoc::CalculatePagePairsForProspectPrinting(
    const SwRootFrm& rLayout,
    /* out */ SwRenderData &rData,
    const SwPrintUIOptions &rOptions,
    sal_Int32 nDocPageCount )
{
    std::map< sal_Int32, sal_Int32 > &rPrinterPaperTrays = rData.GetPrinterPaperTrays();
    std::set< sal_Int32 > &rValidPagesSet = rData.GetValidPagesSet();
    std::vector< std::pair< sal_Int32, sal_Int32 > > &rPagePairs = rData.GetPagePairsForProspectPrinting();
    std::map< sal_Int32, const SwPageFrm * > validStartFrms;

    rPagePairs.clear();
    rValidPagesSet.clear();

    OUString aPageRange;
    // PageContent :
    // 0 -> print all pages (default if aPageRange is empty)
    // 1 -> print range according to PageRange
    // 2 -> print selection
    const sal_Int64 nContent = rOptions.getIntValue( "PrintContent", 0 );
    if (nContent == 1)
        aPageRange = rOptions.getStringValue( "PageRange", OUString() );
    if (aPageRange.isEmpty())    // empty string -> print all
    {
        // set page range to print to 'all pages'
        aPageRange = OUString::number( 1 ) + "-" + OUString::number( nDocPageCount );
    }
    StringRangeEnumerator aRange( aPageRange, 1, nDocPageCount, 0 );

    if ( aRange.size() <= 0)
        return;

    const SwPageFrm *pStPage  = dynamic_cast<const SwPageFrm*>( rLayout.Lower() );
    sal_Int32 i = 0;
    for ( i = 1; pStPage && i < nDocPageCount; ++i )
        pStPage = (SwPageFrm*)pStPage->GetNext();
    if ( !pStPage )          // Then it was that
        return;

    // currently for prospect printing all pages are valid to be printed
    // thus we add them all to the respective map and set for later use
    sal_Int32 nPageNum = 0;
    const SwPageFrm *pPageFrm  = dynamic_cast<const SwPageFrm*>( rLayout.Lower() );
    while( pPageFrm && nPageNum < nDocPageCount )
    {
        OSL_ENSURE( pPageFrm, "Empty page frame. How are we going to print this?" );
        ++nPageNum;
        rValidPagesSet.insert( nPageNum );
        validStartFrms[ nPageNum ] = pPageFrm;
        pPageFrm = (SwPageFrm*)pPageFrm->GetNext();

        rPrinterPaperTrays[ nPageNum ] = lcl_GetPaperBin( pStPage );
    }
    OSL_ENSURE( nPageNum == nDocPageCount, "unexpected number of pages" );

    // properties to take into account when calculating the set of pages
    // Note: here bPrintLeftPages and bPrintRightPages refer to the (virtual) resulting pages
    //      of the prospect!
    bool bPrintLeftPages     = rOptions.IsPrintLeftPages();
    bool bPrintRightPages    = rOptions.IsPrintRightPages();
    bool bPrintProspectRTL = rOptions.getIntValue( "PrintProspectRTL",  0 ) ? true : false;

    // get pages for prospect printing according to the 'PageRange'
    // (duplicates and any order allowed!)
    std::vector< sal_Int32 > aPagesToPrint;
    StringRangeEnumerator::getRangesFromString(
            aPageRange, aPagesToPrint, 1, nDocPageCount, 0 );

    if (aPagesToPrint.empty())
        return;

    // now fill the vector for calculating the page pairs with the start frames
    // from the above obtained vector
    std::vector< const SwPageFrm * > aVec;
    for ( i = 0; i < sal_Int32(aPagesToPrint.size()); ++i)
    {
        const sal_Int32 nPage = aPagesToPrint[i];
        const SwPageFrm *pFrm = validStartFrms[ nPage ];
        aVec.push_back( pFrm );
    }

    // just one page is special ...
    if ( 1 == aVec.size() )
        aVec.insert( aVec.begin() + 1, (SwPageFrm *)0 ); // insert a second empty page
    else
    {
        // now extend the number of pages to fit a multiple of 4
        // (4 'normal' pages are needed for a single prospect paper
        //  with back and front)
        while( aVec.size() & 3 )
            aVec.push_back( 0 );
    }

    // make sure that all pages are in correct order
    sal_uInt16 nSPg = 0;
    sal_uInt32 nEPg = aVec.size();
    sal_uInt16 nStep = 1;
    if ( 0 == (nEPg & 1 ))      // there are no uneven ones!
        --nEPg;

    if ( !bPrintLeftPages )
        ++nStep;
    else if ( !bPrintRightPages )
    {
        ++nStep;
        ++nSPg, --nEPg;
    }

    // the number of 'virtual' pages to be printed
    sal_Int32 nCntPage = (( nEPg - nSPg ) / ( 2 * nStep )) + 1;

    for ( sal_uInt16 nPrintCount = 0; nSPg < nEPg &&
            nPrintCount < nCntPage; ++nPrintCount )
    {
        pStPage = aVec[ nSPg ];
        const SwPageFrm* pNxtPage = nEPg < aVec.size() ? aVec[ nEPg ] : 0;

        short nRtlOfs = bPrintProspectRTL ? 1 : 0;
        if ( 0 == (( nSPg + nRtlOfs) & 1 ) )     // switch for odd number in LTR, even number in RTL
        {
            const SwPageFrm* pTmp = pStPage;
            pStPage = pNxtPage;
            pNxtPage = pTmp;
        }

        sal_Int32 nFirst = -1, nSecond = -1;
        for ( int nC = 0; nC < 2; ++nC )
        {
            sal_Int32 nPage = -1;
            if ( pStPage )
                nPage = pStPage->GetPhyPageNum();
            if (nC == 0)
                nFirst  = nPage;
            else
                nSecond = nPage;

            pStPage = pNxtPage;
        }
        rPagePairs.push_back( std::pair< sal_Int32, sal_Int32 >(nFirst, nSecond) );

        nSPg = nSPg + nStep;
        nEPg = nEPg - nStep;
    }
    OSL_ENSURE( size_t(nCntPage) == rPagePairs.size(), "size mismatch for number of page pairs" );

    // luckily prospect printing does not make use of post-its so far,
    // thus we are done here.
}

namespace
{
    class LockAllViews
    {
        std::vector<SwViewShell*> m_aViewWasUnLocked;
        SwViewShell* m_pViewShell;
    public:
        LockAllViews(SwViewShell *pViewShell)
            : m_pViewShell(pViewShell)
        {
            if (!m_pViewShell)
                return;
            SwViewShell *pSh = m_pViewShell;
            do
            {
                if (!pSh->IsViewLocked())
                {
                    m_aViewWasUnLocked.push_back(pSh);
                    pSh->LockView(true);
                }
                pSh = (SwViewShell*)pSh->GetNext();
            } while (pSh != m_pViewShell);
        }
        ~LockAllViews()
        {
            for (std::vector<SwViewShell*>::iterator aI = m_aViewWasUnLocked.begin(); aI != m_aViewWasUnLocked.end(); ++aI)
            {
                SwViewShell *pSh = *aI;
                pSh->LockView(false);
            }
        }
    };
}

// returns true while there is more to do
bool SwDoc::IncrementalDocStatCalculate(long nChars, bool bFields)
{
    mpDocStat->Reset();
    mpDocStat->nPara = 0; // default is 1!
    SwNode* pNd;

    // This is the inner loop - at least while the paras are dirty.
    for( sal_uLong i = GetNodes().Count(); i > 0 && nChars > 0; )
    {
        switch( ( pNd = GetNodes()[ --i ])->GetNodeType() )
        {
        case ND_TEXTNODE:
        {
            long const nOldChars(mpDocStat->nChar);
            SwTxtNode *pTxt = static_cast< SwTxtNode * >( pNd );
            if (pTxt->CountWords(*mpDocStat, 0, pTxt->GetTxt().getLength()))
            {
                nChars -= (mpDocStat->nChar - nOldChars);
            }
            break;
        }
        case ND_TABLENODE:      ++mpDocStat->nTbl;   break;
        case ND_GRFNODE:        ++mpDocStat->nGrf;   break;
        case ND_OLENODE:        ++mpDocStat->nOLE;   break;
        case ND_SECTIONNODE:    break;
        }
    }

    // #i93174#: notes contain paragraphs that are not nodes
    {
        SwFieldType * const pPostits( GetSysFldType(RES_POSTITFLD) );
        SwIterator<SwFmtFld,SwFieldType> aIter( *pPostits );
        for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld;  pFmtFld = aIter.Next() )
        {
            if (pFmtFld->IsFldInDoc())
            {
                SwPostItField const * const pField(
                    static_cast<SwPostItField const*>(pFmtFld->GetField()));
                mpDocStat->nAllPara += pField->GetNumberOfParagraphs();
            }
        }
    }

    mpDocStat->nPage     = GetCurrentLayout() ? GetCurrentLayout()->GetPageNum() : 0;
    mpDocStat->bModified = false;

    com::sun::star::uno::Sequence < com::sun::star::beans::NamedValue > aStat( mpDocStat->nPage ? 8 : 7);
    sal_Int32 n=0;
    aStat[n].Name = "TableCount";
    aStat[n++].Value <<= (sal_Int32)mpDocStat->nTbl;
    aStat[n].Name = "ImageCount";
    aStat[n++].Value <<= (sal_Int32)mpDocStat->nGrf;
    aStat[n].Name = "ObjectCount";
    aStat[n++].Value <<= (sal_Int32)mpDocStat->nOLE;
    if ( mpDocStat->nPage )
    {
        aStat[n].Name = "PageCount";
        aStat[n++].Value <<= (sal_Int32)mpDocStat->nPage;
    }
    aStat[n].Name = "ParagraphCount";
    aStat[n++].Value <<= (sal_Int32)mpDocStat->nPara;
    aStat[n].Name = "WordCount";
    aStat[n++].Value <<= (sal_Int32)mpDocStat->nWord;
    aStat[n].Name = "CharacterCount";
    aStat[n++].Value <<= (sal_Int32)mpDocStat->nChar;
    aStat[n].Name = "NonWhitespaceCharacterCount";
    aStat[n++].Value <<= (sal_Int32)mpDocStat->nCharExcludingSpaces;

    // For e.g. autotext documents there is no pSwgInfo (#i79945)
    SwDocShell* pObjShell(GetDocShell());
    if (pObjShell)
    {
        const uno::Reference<document::XDocumentPropertiesSupplier> xDPS(
                pObjShell->GetModel(), uno::UNO_QUERY_THROW);
        const uno::Reference<document::XDocumentProperties> xDocProps(
                xDPS->getDocumentProperties());
        // #i96786#: do not set modified flag when updating statistics
        const bool bDocWasModified( IsModified() );
        const ModifyBlocker_Impl b(pObjShell);
        // rhbz#1081176: don't jump to cursor pos because of (temporary)
        // activation of modified flag triggering move to input position
        LockAllViews aViewGuard((SwViewShell*)pObjShell->GetWrtShell());
        xDocProps->setDocumentStatistics(aStat);
        if (!bDocWasModified)
        {
            ResetModified();
        }
    }

    // optionally update stat. fields
    if (bFields)
    {
        SwFieldType *pType = GetSysFldType(RES_DOCSTATFLD);
        pType->UpdateFlds();
    }

    return nChars <= 0;
}

IMPL_LINK( SwDoc, DoIdleStatsUpdate, Timer *, pTimer )
{
    (void)pTimer;
    if (IncrementalDocStatCalculate(32000))
        maStatsUpdateTimer.Start();

    SwView* pView = GetDocShell() ? GetDocShell()->GetView() : NULL;
    if( pView )
        pView->UpdateDocStats();
    return 0;
}

void SwDoc::UpdateDocStat( bool bCompleteAsync, bool bFields )
{
    if( mpDocStat->bModified )
    {
        if (!bCompleteAsync)
        {
            while (IncrementalDocStatCalculate(
                        ::std::numeric_limits<long>::max(), bFields)) {}
            maStatsUpdateTimer.Stop();
        }
        else if (IncrementalDocStatCalculate(5000, bFields))
            maStatsUpdateTimer.Start();
    }
}

void SwDoc::DocInfoChgd( )
{
    GetSysFldType( RES_DOCINFOFLD )->UpdateFlds();
    GetSysFldType( RES_TEMPLNAMEFLD )->UpdateFlds();
    SetModified();
}

/// @return the reference in the doc for the name
const SwFmtRefMark* SwDoc::GetRefMark( const OUString& rName ) const
{
    const SfxPoolItem* pItem;
    sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_TXTATR_REFMARK );
    for( sal_uInt32 n = 0; n < nMaxItems; ++n )
    {
        if( 0 == (pItem = GetAttrPool().GetItem2( RES_TXTATR_REFMARK, n ) ))
            continue;

        const SwFmtRefMark* pFmtRef = (SwFmtRefMark*)pItem;
        const SwTxtRefMark* pTxtRef = pFmtRef->GetTxtRefMark();
        if( pTxtRef && &pTxtRef->GetTxtNode().GetNodes() == &GetNodes() &&
            rName == pFmtRef->GetRefName() )
            return pFmtRef;
    }
    return 0;
}

/// @return the RefMark per index - for Uno
const SwFmtRefMark* SwDoc::GetRefMark( sal_uInt16 nIndex ) const
{
    const SfxPoolItem* pItem;
    const SwTxtRefMark* pTxtRef;
    const SwFmtRefMark* pRet = 0;

    sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_TXTATR_REFMARK );
    sal_uInt32 nCount = 0;
    for( sal_uInt32 n = 0; n < nMaxItems; ++n )
        if( 0 != (pItem = GetAttrPool().GetItem2( RES_TXTATR_REFMARK, n )) &&
            0 != (pTxtRef = ((SwFmtRefMark*)pItem)->GetTxtRefMark()) &&
            &pTxtRef->GetTxtNode().GetNodes() == &GetNodes() )
        {
            if(nCount == nIndex)
            {
                pRet = (SwFmtRefMark*)pItem;
                break;
            }
            nCount++;
        }
   return pRet;
}

/// @return the names of all set references in the Doc
//JP 24.06.96: If the array pointer is 0, then just return whether a RefMark is set in the Doc
// OS 25.06.96: From now on we always return the reference count
sal_uInt16 SwDoc::GetRefMarks( std::vector<OUString>* pNames ) const
{
    const SfxPoolItem* pItem;
    const SwTxtRefMark* pTxtRef;

    const sal_uInt32 nMaxItems = GetAttrPool().GetItemCount2( RES_TXTATR_REFMARK );
    sal_uInt16 nCount = 0;
    for( sal_uInt32 n = 0; n < nMaxItems; ++n )
        if( 0 != (pItem = GetAttrPool().GetItem2( RES_TXTATR_REFMARK, n )) &&
            0 != (pTxtRef = ((SwFmtRefMark*)pItem)->GetTxtRefMark()) &&
            &pTxtRef->GetTxtNode().GetNodes() == &GetNodes() )
        {
            if( pNames )
            {
                OUString pTmp(((SwFmtRefMark*)pItem)->GetRefName());
                pNames->insert(pNames->begin() + nCount, pTmp);
            }
            ++nCount;
        }

    return nCount;
}

bool SwDoc::IsLoaded() const
{
    return mbLoaded;
}

bool SwDoc::IsUpdateExpFld() const
{
    return mbUpdateExpFld;
}

bool SwDoc::IsNewDoc() const
{
    return mbNewDoc;
}

bool SwDoc::IsPageNums() const
{
  return mbPageNums;
}

void SwDoc::SetPageNums(bool b)
{
    mbPageNums = b;
}

void SwDoc::SetNewDoc(bool b)
{
    mbNewDoc = b;
}

void SwDoc::SetUpdateExpFldStat(bool b)
{
    mbUpdateExpFld = b;
}

void SwDoc::SetLoaded(bool b)
{
    mbLoaded = b;
}

bool SwDoc::IsModified() const
{
    return mbModified;
}

//Load document from fdo#42534 under valgrind, drag the scrollbar down so full
//document layout is triggered. Close document before layout has completed, and
//SwAnchoredObject objects deleted by the deletion of layout remain referenced
//by the SwLayouter
void SwDoc::ClearSwLayouterEntries()
{
    SwLayouter::ClearMovedFwdFrms( *this );
    SwLayouter::ClearObjsTmpConsiderWrapInfluence( *this );
    // #i65250#
    SwLayouter::ClearMoveBwdLayoutInfo( *this );
}

void SwDoc::SetModified()
{
    ClearSwLayouterEntries();
    // give the old and new modified state to the link
    //  Bit 0:  -> old state
    //  Bit 1:  -> new state
    sal_IntPtr nCall = mbModified ? 3 : 2;
    mbModified = true;
    mpDocStat->bModified = true;
    if( maOle2Link.IsSet() )
    {
        mbInCallModified = true;
        maOle2Link.Call( (void*)nCall );
        mbInCallModified = false;
    }

    if( mpACEWord && !mpACEWord->IsDeleted() )
        delete mpACEWord, mpACEWord = 0;
}

void SwDoc::ResetModified()
{
    // give the old and new modified state to the link
    //  Bit 0:  -> old state
    //  Bit 1:  -> new state
    sal_IntPtr nCall = mbModified ? 1 : 0;
    mbModified = false;
    GetIDocumentUndoRedo().SetUndoNoModifiedPosition();
    if( nCall && maOle2Link.IsSet() )
    {
        mbInCallModified = true;
        maOle2Link.Call( (void*)nCall );
        mbInCallModified = false;
    }
}

void SwDoc::ReRead( SwPaM& rPam, const OUString& rGrfName,
                    const OUString& rFltName, const Graphic* pGraphic,
                    const GraphicObject* pGrafObj )
{
    SwGrfNode *pGrfNd;
    if( ( !rPam.HasMark()
         || rPam.GetPoint()->nNode.GetIndex() == rPam.GetMark()->nNode.GetIndex() )
         && 0 != ( pGrfNd = rPam.GetPoint()->nNode.GetNode().GetGrfNode() ) )
    {
        if (GetIDocumentUndoRedo().DoesUndo())
        {
            GetIDocumentUndoRedo().AppendUndo(new SwUndoReRead(rPam, *pGrfNd));
        }

        // Because we don't know if we can mirror the graphic, the mirror attribute is always reset
        if( RES_MIRROR_GRAPH_DONT != pGrfNd->GetSwAttrSet().
                                                GetMirrorGrf().GetValue() )
            pGrfNd->SetAttr( SwMirrorGrf() );

        pGrfNd->ReRead( rGrfName, rFltName, pGraphic, pGrafObj, true );
        SetModified();
    }
}

static bool lcl_SpellAndGrammarAgain( const SwNodePtr& rpNd, void* pArgs )
{
    SwTxtNode *pTxtNode = (SwTxtNode*)rpNd->GetTxtNode();
    bool bOnlyWrong = *(sal_Bool*)pArgs;
    if( pTxtNode )
    {
        if( bOnlyWrong )
        {
            if( pTxtNode->GetWrong() &&
                pTxtNode->GetWrong()->InvalidateWrong() )
                pTxtNode->SetWrongDirty( true );
            if( pTxtNode->GetGrammarCheck() &&
                pTxtNode->GetGrammarCheck()->InvalidateWrong() )
                pTxtNode->SetGrammarCheckDirty( true );
        }
        else
        {
            pTxtNode->SetWrongDirty( true );
            if( pTxtNode->GetWrong() )
                pTxtNode->GetWrong()->SetInvalid( 0, COMPLETE_STRING );
            pTxtNode->SetGrammarCheckDirty( true );
            if( pTxtNode->GetGrammarCheck() )
                pTxtNode->GetGrammarCheck()->SetInvalid( 0, COMPLETE_STRING );
        }
    }
    return true;
}

static bool lcl_CheckSmartTagsAgain( const SwNodePtr& rpNd, void*  )
{
    SwTxtNode *pTxtNode = (SwTxtNode*)rpNd->GetTxtNode();
    if( pTxtNode )
    {
        pTxtNode->SetSmartTagDirty( true );
        if( pTxtNode->GetSmartTags() )
        {
            pTxtNode->SetSmartTags( NULL );
        }
    }
    return true;
}

/** Re-trigger spelling in the idle handler.
 *
 * @param bInvalid if <true>, the WrongLists in all nodes are invalidated
 *                 and the SpellInvalid flag is set on all pages.
 * @param bOnlyWrong controls whether only the areas with wrong words are
 *                   checked or the whole area.
 * @param bSmartTags ???
 ************************************************************************/
void SwDoc::SpellItAgainSam( bool bInvalid, bool bOnlyWrong, bool bSmartTags )
{
    std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
    OSL_ENSURE( GetCurrentLayout(), "SpellAgain: Where's my RootFrm?" );
    if( bInvalid )
    {
        std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::bind2nd(std::mem_fun(&SwRootFrm::AllInvalidateSmartTagsOrSpelling),bSmartTags));
        std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::bind2nd(std::mem_fun(&SwRootFrm::SetNeedGrammarCheck), true) );
        if ( bSmartTags )
            GetNodes().ForEach( lcl_CheckSmartTagsAgain, &bOnlyWrong );
        GetNodes().ForEach( lcl_SpellAndGrammarAgain, &bOnlyWrong );
    }

    std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::mem_fun(&SwRootFrm::SetIdleFlags));
}

void SwDoc::InvalidateAutoCompleteFlag()
{
    SwRootFrm* pTmpRoot = GetCurrentLayout();
    if( pTmpRoot )
    {
        std::set<SwRootFrm*> aAllLayouts = GetAllLayouts();
        std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::mem_fun(&SwRootFrm::AllInvalidateAutoCompleteWords));
        for( sal_uLong nNd = 1, nCnt = GetNodes().Count(); nNd < nCnt; ++nNd )
        {
            SwTxtNode* pTxtNode = GetNodes()[ nNd ]->GetTxtNode();
            if ( pTxtNode ) pTxtNode->SetAutoCompleteWordDirty( true );
        }

        std::for_each( aAllLayouts.begin(), aAllLayouts.end(),std::mem_fun(&SwRootFrm::SetIdleFlags));
    }
}

const SwFmtINetFmt* SwDoc::FindINetAttr( const OUString& rName ) const
{
    const SwFmtINetFmt* pItem;
    const SwTxtINetFmt* pTxtAttr;
    const SwTxtNode* pTxtNd;
    sal_uInt32 n, nMaxItems = GetAttrPool().GetItemCount2( RES_TXTATR_INETFMT );
    for( n = 0; n < nMaxItems; ++n )
        if( 0 != (pItem = (SwFmtINetFmt*)GetAttrPool().GetItem2(
            RES_TXTATR_INETFMT, n ) ) &&
            pItem->GetName() == rName &&
            0 != ( pTxtAttr = pItem->GetTxtINetFmt()) &&
            0 != ( pTxtNd = pTxtAttr->GetpTxtNode() ) &&
            &pTxtNd->GetNodes() == &GetNodes() )
        {
            return pItem;
        }

    return 0;
}

void SwDoc::Summary( SwDoc* pExtDoc, sal_uInt8 nLevel, sal_uInt8 nPara, bool bImpress )
{
    const SwOutlineNodes& rOutNds = GetNodes().GetOutLineNds();
    if( pExtDoc && !rOutNds.empty() )
    {
        sal_uInt16 i;
        ::StartProgress( STR_STATSTR_SUMMARY, 0, rOutNds.size(), GetDocShell() );
        SwNodeIndex aEndOfDoc( pExtDoc->GetNodes().GetEndOfContent(), -1 );
        for( i = 0; i < rOutNds.size(); ++i )
        {
            ::SetProgressState( i, GetDocShell() );
            const sal_uLong nIndex = rOutNds[ i ]->GetIndex();

            const int nLvl = ((SwTxtNode*)GetNodes()[ nIndex ])->GetAttrOutlineLevel()-1;
            if( nLvl > nLevel )
                continue;
            sal_uInt16 nEndOfs = 1;
            sal_uInt8 nWish = nPara;
            sal_uLong nNextOutNd = i + 1 < (sal_uInt16)rOutNds.size() ?
                rOutNds[ i + 1 ]->GetIndex() : GetNodes().Count();
            bool bKeep = false;
            while( ( nWish || bKeep ) && nIndex + nEndOfs < nNextOutNd &&
                   GetNodes()[ nIndex + nEndOfs ]->IsTxtNode() )
            {
                SwTxtNode* pTxtNode = (SwTxtNode*)GetNodes()[ nIndex+nEndOfs ];
                if (pTxtNode->GetTxt().getLength() && nWish)
                    --nWish;
                bKeep = pTxtNode->GetSwAttrSet().GetKeep().GetValue();
                ++nEndOfs;
            }

            SwNodeRange aRange( *rOutNds[ i ], 0, *rOutNds[ i ], nEndOfs );
            GetNodes()._Copy( aRange, aEndOfDoc );
        }
        const SwTxtFmtColls *pColl = pExtDoc->GetTxtFmtColls();
        for( i = 0; i < pColl->size(); ++i )
            (*pColl)[ i ]->ResetFmtAttr( RES_PAGEDESC, RES_BREAK );
        SwNodeIndex aIndx( pExtDoc->GetNodes().GetEndOfExtras() );
        ++aEndOfDoc;
        while( aIndx < aEndOfDoc )
        {
            SwNode *pNode;
            bool bDelete = false;
            if( (pNode = &aIndx.GetNode())->IsTxtNode() )
            {
                SwTxtNode *pNd = (SwTxtNode*)pNode;
                if( pNd->HasSwAttrSet() )
                    pNd->ResetAttr( RES_PAGEDESC, RES_BREAK );
                if( bImpress )
                {
                    SwTxtFmtColl* pMyColl = pNd->GetTxtColl();

                    const sal_uInt16 nHeadLine = static_cast<sal_uInt16>(
                                !pMyColl->IsAssignedToListLevelOfOutlineStyle()
                                ? RES_POOLCOLL_HEADLINE2
                                : RES_POOLCOLL_HEADLINE1 );
                    pMyColl = pExtDoc->GetTxtCollFromPool( nHeadLine );
                    pNd->ChgFmtColl( pMyColl );
                }
                if( !pNd->Len() &&
                    pNd->StartOfSectionIndex()+2 < pNd->EndOfSectionIndex() )
                {
                    bDelete = true;
                    pExtDoc->GetNodes().Delete( aIndx );
                }
            }
            if( !bDelete )
                ++aIndx;
        }
        ::EndProgress( GetDocShell() );
    }
}

/// Remove the invisible content from the document e.g. hidden areas, hidden paragraphs
bool SwDoc::RemoveInvisibleContent()
{
    bool bRet = false;
    GetIDocumentUndoRedo().StartUndo( UNDO_UI_DELETE_INVISIBLECNTNT, NULL );

    {
        SwTxtNode* pTxtNd;
        SwIterator<SwFmtFld,SwFieldType> aIter( *GetSysFldType( RES_HIDDENPARAFLD )  );
        for( SwFmtFld* pFmtFld = aIter.First(); pFmtFld;  pFmtFld = aIter.Next() )
        {
            if( pFmtFld->GetTxtFld() &&
                0 != ( pTxtNd = (SwTxtNode*)pFmtFld->GetTxtFld()->GetpTxtNode() ) &&
                pTxtNd->GetpSwpHints() && pTxtNd->HasHiddenParaField() &&
                &pTxtNd->GetNodes() == &GetNodes() )
            {
                bRet = true;
                SwPaM aPam(*pTxtNd, 0, *pTxtNd, pTxtNd->GetTxt().getLength());

                // Remove hidden paragraph or delete contents:
                // Delete contents if
                // 1. removing the paragraph would result in an empty section or
                // 2. if the paragraph is the last paragraph in the section and
                //    there is no paragraph in front of the paragraph:
                if ( ( 2 == pTxtNd->EndOfSectionIndex() - pTxtNd->StartOfSectionIndex() ) ||
                     ( 1 == pTxtNd->EndOfSectionIndex() - pTxtNd->GetIndex() &&
                       !GetNodes()[ pTxtNd->GetIndex() - 1 ]->GetTxtNode() ) )
                {
                    DeleteRange( aPam );
                }
                else
                {
                    aPam.DeleteMark();
                    DelFullPara( aPam );
                }
            }
        }
    }

    // Remove any hidden paragraph (hidden text attribute)
    for( sal_uLong n = GetNodes().Count(); n; )
    {
        SwTxtNode* pTxtNd = GetNodes()[ --n ]->GetTxtNode();
        if ( pTxtNd )
        {
            bool bRemoved = false;
            SwPaM aPam(*pTxtNd, 0, *pTxtNd, pTxtNd->GetTxt().getLength());
            if ( pTxtNd->HasHiddenCharAttribute( true ) )
            {
                bRemoved = true;
                bRet = true;

                // Remove hidden paragraph or delete contents:
                // Delete contents if
                // 1. removing the paragraph would result in an empty section or
                // 2. if the paragraph is the last paragraph in the section and
                //    there is no paragraph in front of the paragraph:
                if ( ( 2 == pTxtNd->EndOfSectionIndex() - pTxtNd->StartOfSectionIndex() ) ||
                     ( 1 == pTxtNd->EndOfSectionIndex() - pTxtNd->GetIndex() &&
                       !GetNodes()[ pTxtNd->GetIndex() - 1 ]->GetTxtNode() ) )
                {
                    DeleteRange( aPam );
                }
                else
                {
                    aPam.DeleteMark();
                    DelFullPara( aPam );
                }
            }
            else if ( pTxtNd->HasHiddenCharAttribute( false ) )
            {
                bRemoved = true;
                bRet = true;
                SwScriptInfo::DeleteHiddenRanges( *pTxtNd );
            }

            // Footnotes/Frames may have been removed, therefore we have
            // to reset n:
            if ( bRemoved )
                n = aPam.GetPoint()->nNode.GetIndex();
        }
    }

    {
        // Delete/empty all hidden areas
        SwSectionFmts aSectFmts;
        SwSectionFmts& rSectFmts = GetSections();
        sal_uInt16 n;

        for( n = rSectFmts.size(); n; )
        {
            SwSectionFmt* pSectFmt = rSectFmts[ --n ];
            // don't add sections in Undo/Redo
            if( !pSectFmt->IsInNodesArr())
                continue;
            SwSection* pSect = pSectFmt->GetSection();
            if( pSect->CalcHiddenFlag() )
            {
                SwSection* pParent = pSect, *pTmp;
                while( 0 != (pTmp = pParent->GetParent() ))
                {
                    if( pTmp->IsHiddenFlag() )
                        pSect = pTmp;
                    pParent = pTmp;
                }

                SwSectionFmts::iterator it = std::find(
                        aSectFmts.begin(), aSectFmts.end(), pSect->GetFmt() );
                if (it == aSectFmts.end())
                    aSectFmts.insert( aSectFmts.begin(), pSect->GetFmt() );
            }
            if( !pSect->GetCondition().isEmpty() )
            {
                SwSectionData aSectionData( *pSect );
                aSectionData.SetCondition( OUString() );
                aSectionData.SetHidden( false );
                UpdateSection( n, aSectionData );
            }
        }

        if( 0 != ( n = aSectFmts.size() ))
        {
            while( n )
            {
                SwSectionFmt* pSectFmt = aSectFmts[ --n ];
                SwSectionNode* pSectNd = pSectFmt->GetSectionNode();
                if( pSectNd )
                {
                    bRet = true;
                    SwPaM aPam( *pSectNd );

                    if( pSectNd->StartOfSectionNode()->StartOfSectionIndex() ==
                        pSectNd->GetIndex() - 1 &&
                        pSectNd->StartOfSectionNode()->EndOfSectionIndex() ==
                        pSectNd->EndOfSectionIndex() + 1 )
                    {
                        // only delete the content
                        SwCntntNode* pCNd = GetNodes().GoNext(
                                                &aPam.GetPoint()->nNode );
                        aPam.GetPoint()->nContent.Assign( pCNd, 0 );
                        aPam.SetMark();
                        aPam.GetPoint()->nNode = *pSectNd->EndOfSectionNode();
                        pCNd = GetNodes().GoPrevious(
                                                &aPam.GetPoint()->nNode );
                        aPam.GetPoint()->nContent.Assign( pCNd, pCNd->Len() );

                        DeleteRange( aPam );
                    }
                    else
                    {
                        // delete the whole section
                        aPam.SetMark();
                        aPam.GetPoint()->nNode = *pSectNd->EndOfSectionNode();
                        DelFullPara( aPam );
                    }

                }
            }
            aSectFmts.clear();
        }
    }

    if( bRet )
        SetModified();
    GetIDocumentUndoRedo().EndUndo( UNDO_UI_DELETE_INVISIBLECNTNT, NULL );
    return bRet;
}

bool SwDoc::HasInvisibleContent() const
{
    bool bRet = false;

    SwClientIter aIter( *GetSysFldType( RES_HIDDENPARAFLD ) );
    if( aIter.First( TYPE( SwFmtFld ) ) )
        bRet = true;

    // Search for any hidden paragraph (hidden text attribute)
    if( ! bRet )
    {
        for( sal_uLong n = GetNodes().Count(); !bRet && (n > 0); )
        {
            SwTxtNode* pTxtNd = GetNodes()[ --n ]->GetTxtNode();
            if ( pTxtNd )
            {
                SwPaM aPam(*pTxtNd, 0, *pTxtNd, pTxtNd->GetTxt().getLength());
                if( pTxtNd->HasHiddenCharAttribute( true ) ||  ( pTxtNd->HasHiddenCharAttribute( false ) ) )
                {
                    bRet = true;
                }
            }
        }
    }

    if( ! bRet )
    {
        const SwSectionFmts& rSectFmts = GetSections();
        sal_uInt16 n;

        for( n = rSectFmts.size(); !bRet && (n > 0); )
        {
            SwSectionFmt* pSectFmt = rSectFmts[ --n ];
            // don't add sections in Undo/Redo
            if( !pSectFmt->IsInNodesArr())
                continue;
            SwSection* pSect = pSectFmt->GetSection();
            if( pSect->IsHidden() )
                bRet = true;
        }
    }
    return bRet;
}

bool SwDoc::RestoreInvisibleContent()
{
    SwUndoId nLastUndoId(UNDO_EMPTY);
    if (GetIDocumentUndoRedo().GetLastUndoInfo(0, & nLastUndoId)
        && (UNDO_UI_DELETE_INVISIBLECNTNT == nLastUndoId))
    {
        GetIDocumentUndoRedo().Undo();
        GetIDocumentUndoRedo().ClearRedo();
        return true;
    }
    return false;
}

bool SwDoc::ConvertFieldsToText()
{
    bool bRet = false;
    LockExpFlds();
    GetIDocumentUndoRedo().StartUndo( UNDO_UI_REPLACE, NULL );

    const SwFldTypes* pMyFldTypes = GetFldTypes();
    sal_uInt16 nCount = pMyFldTypes->size();
    //go backward, field types are removed
    for(sal_uInt16 nType = nCount;  nType > 0;  --nType)
    {
        const SwFieldType *pCurType = (*pMyFldTypes)[nType - 1];

        if ( RES_POSTITFLD == pCurType->Which() )
            continue;

        SwIterator<SwFmtFld,SwFieldType> aIter( *pCurType );
        ::std::vector<const SwFmtFld*> aFieldFmts;
        for( SwFmtFld* pCurFldFmt = aIter.First(); pCurFldFmt; pCurFldFmt = aIter.Next() )
            aFieldFmts.push_back(pCurFldFmt);

        ::std::vector<const SwFmtFld*>::iterator aBegin = aFieldFmts.begin();
        ::std::vector<const SwFmtFld*>::iterator aEnd = aFieldFmts.end();
        while(aBegin != aEnd)
        {
            const SwTxtFld *pTxtFld = (*aBegin)->GetTxtFld();
            // skip fields that are currently not in the document
            // e.g. fields in undo or redo array

            bool bSkip = !pTxtFld ||
                         !pTxtFld->GetpTxtNode()->GetNodes().IsDocNodes();

            if (!bSkip)
            {
                bool bInHeaderFooter = IsInHeaderFooter(SwNodeIndex(*pTxtFld->GetpTxtNode()));
                const SwFmtFld& rFmtFld = pTxtFld->GetFmtFld();
                const SwField*  pField = rFmtFld.GetField();

                //#i55595# some fields have to be excluded in headers/footers
                sal_uInt16 nWhich = pField->GetTyp()->Which();
                if(!bInHeaderFooter ||
                        (nWhich != RES_PAGENUMBERFLD &&
                        nWhich != RES_CHAPTERFLD &&
                        nWhich != RES_GETEXPFLD&&
                        nWhich != RES_SETEXPFLD&&
                        nWhich != RES_INPUTFLD&&
                        nWhich != RES_REFPAGEGETFLD&&
                        nWhich != RES_REFPAGESETFLD))
                {
                    OUString sText = pField->ExpandField(true);

                    // database fields should not convert their command into text
                    if( RES_DBFLD == pCurType->Which() && !static_cast<const SwDBField*>(pField)->IsInitialized())
                        sText = "";

                    SwPaM aInsertPam(*pTxtFld->GetpTxtNode(), *pTxtFld->GetStart());
                    aInsertPam.SetMark();

                    // go to the end of the field
                    const SwTxtFld *pTxtField = GetTxtFldAtPos( *aInsertPam.End() );
                    if (pTxtField && pTxtField->Which() == RES_TXTATR_INPUTFIELD)
                    {
                        SwPosition &rEndPos = *aInsertPam.GetPoint();
                        rEndPos.nContent = GetDocShell()->GetWrtShell()->EndOfInputFldAtPos( *aInsertPam.End() );
                    }
                    else
                    {
                        aInsertPam.Move();
                    }

                    // first insert the text after field to keep the field's attributes,
                    // then delete the field
                    if (!sText.isEmpty())
                    {
                        // to keep the position after insert
                        SwPaM aDelPam( *aInsertPam.GetMark(), *aInsertPam.GetPoint() );
                        aDelPam.Move( fnMoveBackward );
                        aInsertPam.DeleteMark();

                        InsertString( aInsertPam, sText );

                        aDelPam.Move();
                        // finally remove the field
                        DeleteAndJoin( aDelPam );
                    }
                    else
                    {
                        DeleteAndJoin( aInsertPam );
                    }

                    bRet = true;
                }
            }
            ++aBegin;
        }
    }

    if( bRet )
        SetModified();
    GetIDocumentUndoRedo().EndUndo( UNDO_UI_REPLACE, NULL );
    UnlockExpFlds();
    return bRet;

}

bool SwDoc::IsVisibleLinks() const
{
    return mbVisibleLinks;
}

void SwDoc::SetVisibleLinks(bool bFlag)
{
    mbVisibleLinks = bFlag;
}

sfx2::LinkManager& SwDoc::GetLinkManager()
{
    return *mpLinkMgr;
}

const sfx2::LinkManager& SwDoc::GetLinkManager() const
{
    return *mpLinkMgr;
}

void SwDoc::SetLinksUpdated(const bool bNewLinksUpdated)
{
    mbLinksUpdated = bNewLinksUpdated;
}

bool SwDoc::LinksUpdated() const
{
    return mbLinksUpdated;
}

static ::sfx2::SvBaseLink* lcl_FindNextRemovableLink( const ::sfx2::SvBaseLinks& rLinks, sfx2::LinkManager& rLnkMgr )
{
    for( sal_uInt16 n = 0; n < rLinks.size(); ++n )
    {
        ::sfx2::SvBaseLink* pLnk = &(*rLinks[ n ]);
        if( pLnk &&
            ( OBJECT_CLIENT_GRF == pLnk->GetObjType() ||
              OBJECT_CLIENT_FILE == pLnk->GetObjType() ) &&
            pLnk->ISA( SwBaseLink ) )
        {
                ::sfx2::SvBaseLinkRef xLink = pLnk;

                OUString sFName;
                rLnkMgr.GetDisplayNames( xLink, 0, &sFName, 0, 0 );

                INetURLObject aURL( sFName );
                if( INET_PROT_FILE == aURL.GetProtocol() ||
                    INET_PROT_CID == aURL.GetProtocol() )
                    return pLnk;
        }
    }
    return 0;
}

/// embedded all local links (Areas/Graphics)
bool SwDoc::EmbedAllLinks()
{
    bool bRet = false;
    sfx2::LinkManager& rLnkMgr = GetLinkManager();
    const ::sfx2::SvBaseLinks& rLinks = rLnkMgr.GetLinks();
    if( !rLinks.empty() )
    {
        ::sw::UndoGuard const undoGuard(GetIDocumentUndoRedo());

        ::sfx2::SvBaseLink* pLnk = 0;
        while( 0 != (pLnk = lcl_FindNextRemovableLink( rLinks, rLnkMgr ) ) )
        {
            ::sfx2::SvBaseLinkRef xLink = pLnk;
            // Tell the link that it's being destroyed!
            xLink->Closed();

            // if one forgot to remove itself
            if( xLink.Is() )
                rLnkMgr.Remove( xLink );

            bRet = true;
        }

        GetIDocumentUndoRedo().DelAllUndoObj();
        SetModified();
    }
    return bRet;
}

bool SwDoc::IsInsTblFormatNum() const
{
    return SW_MOD()->IsInsTblFormatNum(get(IDocumentSettingAccess::HTML_MODE));
}

bool SwDoc::IsInsTblChangeNumFormat() const
{
    return SW_MOD()->IsInsTblChangeNumFormat(get(IDocumentSettingAccess::HTML_MODE));
}

bool SwDoc::IsInsTblAlignNum() const
{
    return SW_MOD()->IsInsTblAlignNum(get(IDocumentSettingAccess::HTML_MODE));
}

/// Set up the InsertDB as Undo table
void SwDoc::AppendUndoForInsertFromDB( const SwPaM& rPam, bool bIsTable )
{
    if( bIsTable )
    {
        const SwTableNode* pTblNd = rPam.GetPoint()->nNode.GetNode().FindTableNode();
        if( pTblNd )
        {
            SwUndoCpyTbl* pUndo = new SwUndoCpyTbl;
            pUndo->SetTableSttIdx( pTblNd->GetIndex() );
            GetIDocumentUndoRedo().AppendUndo( pUndo );
        }
    }
    else if( rPam.HasMark() )
    {
        SwUndoCpyDoc* pUndo = new SwUndoCpyDoc( rPam );
        pUndo->SetInsertRange( rPam, false );
        GetIDocumentUndoRedo().AppendUndo( pUndo );
    }
}

void SwDoc::ChgTOX(SwTOXBase & rTOX, const SwTOXBase & rNew)
{
    if (GetIDocumentUndoRedo().DoesUndo())
    {
        GetIDocumentUndoRedo().DelAllUndoObj();

        SwUndo * pUndo = new SwUndoTOXChange(&rTOX, rNew);

        GetIDocumentUndoRedo().AppendUndo(pUndo);
    }

    rTOX = rNew;

    if (rTOX.ISA(SwTOXBaseSection))
    {
        static_cast<SwTOXBaseSection &>(rTOX).Update();
        static_cast<SwTOXBaseSection &>(rTOX).UpdatePageNum();
    }
}

OUString SwDoc::GetPaMDescr(const SwPaM & rPam) const
{
    if (rPam.GetNode(true) == rPam.GetNode(false))
    {
        SwTxtNode * pTxtNode = rPam.GetNode(true)->GetTxtNode();

        if (0 != pTxtNode)
        {
            const sal_Int32 nStart = rPam.Start()->nContent.GetIndex();
            const sal_Int32 nEnd = rPam.End()->nContent.GetIndex();

            return SW_RESSTR(STR_START_QUOTE)
                + ShortenString(pTxtNode->GetTxt().copy(nStart, nEnd - nStart),
                                nUndoStringLength,
                                SW_RESSTR(STR_LDOTS))
                + SW_RESSTR(STR_END_QUOTE);
        }
    }
    else if (0 != rPam.GetNode(true))
    {
        if (0 != rPam.GetNode(false))
        {
            return SW_RESSTR(STR_PARAGRAPHS);
        }
        return OUString();
    }

    return OUString("??");
}

SwField * SwDoc::GetFieldAtPos(const SwPosition & rPos)
{
    SwTxtFld * const pAttr = GetTxtFldAtPos(rPos);

    return (pAttr) ? const_cast<SwField *>( pAttr->GetFmtFld().GetField() ) : 0;
}

SwTxtFld * SwDoc::GetTxtFldAtPos(const SwPosition & rPos)
{
    SwTxtNode * const pNode = rPos.nNode.GetNode().GetTxtNode();

    return (pNode != NULL)
        ? pNode->GetFldTxtAttrAt( rPos.nContent.GetIndex(), true )
        : 0;
}

bool SwDoc::ContainsHiddenChars() const
{
    for( sal_uLong n = GetNodes().Count(); n; )
    {
        SwNode* pNd = GetNodes()[ --n ];
        if ( pNd->IsTxtNode() &&
             ((SwTxtNode*)pNd)->HasHiddenCharAttribute( false ) )
            return true;
    }

    return false;
}

SwUnoCrsr* SwDoc::CreateUnoCrsr( const SwPosition& rPos, bool bTblCrsr )
{
    SwUnoCrsr* pNew;
    if( bTblCrsr )
        pNew = new SwUnoTableCrsr( rPos );
    else
        pNew = new SwUnoCrsr( rPos );

    mpUnoCrsrTbl->insert( pNew );
    return pNew;
}

void SwDoc::ChkCondColls()
{
     for (sal_uInt16 n = 0; n < mpTxtFmtCollTbl->size(); n++)
     {
        SwTxtFmtColl *pColl = (*mpTxtFmtCollTbl)[n];
        if (RES_CONDTXTFMTCOLL == pColl->Which())
            pColl->CallSwClientNotify( SwAttrHint(RES_CONDTXTFMTCOLL) );
     }
}

uno::Reference< script::vba::XVBAEventProcessor >
SwDoc::GetVbaEventProcessor()
{
#ifndef DISABLE_SCRIPTING
    if( !mxVbaEvents.is() && mpDocShell && ooo::vba::isAlienWordDoc( *mpDocShell ) )
    {
        try
        {
            uno::Reference< frame::XModel > xModel( mpDocShell->GetModel(), uno::UNO_SET_THROW );
            uno::Sequence< uno::Any > aArgs(1);
            aArgs[0] <<= xModel;
            mxVbaEvents.set( ooo::vba::createVBAUnoAPIServiceWithArgs( mpDocShell, "com.sun.star.script.vba.VBATextEventProcessor" , aArgs ), uno::UNO_QUERY_THROW );
        }
        catch( uno::Exception& )
        {
        }
    }
#endif
    return mxVbaEvents;
}

void SwDoc::setExternalData(::sw::tExternalDataType eType,
                            ::sw::tExternalDataPointer pPayload)
{
    m_externalData[eType] = pPayload;
}

::sw::tExternalDataPointer SwDoc::getExternalData(::sw::tExternalDataType eType)
{
    return m_externalData[eType];
}

sal_uInt16 SwNumRuleTbl::GetPos(const SwNumRule* pRule) const
{
    const_iterator it = std::find(begin(), end(), pRule);
    return it == end() ? USHRT_MAX : it - begin();
}

SwNumRuleTbl::~SwNumRuleTbl()
{
    for(const_iterator it = begin(); it != end(); ++it)
        delete *it;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
