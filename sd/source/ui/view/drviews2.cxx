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

#include <config_features.h>

#include <avmedia/mediaplayer.hxx>

#include <basic/sberrors.hxx>
#include <basic/sbstar.hxx>

#include <com/sun/star/drawing/XMasterPagesSupplier.hpp>
#include <com/sun/star/drawing/XDrawPages.hpp>
#include <com/sun/star/lang/XInitialization.hpp>
#include <com/sun/star/ui/dialogs/XExecutableDialog.hpp>
#include <com/sun/star/ui/dialogs/XSLTFilterDialog.hpp>
#include <com/sun/star/frame/XDispatchProvider.hpp>
#include <com/sun/star/util/URLTransformer.hpp>
#include <com/sun/star/util/XURLTransformer.hpp>
#include <com/sun/star/scanner/XScannerManager2.hpp>
#include <com/sun/star/document/XDocumentProperties.hpp>

#include <comphelper/processfactory.hxx>
#include <comphelper/propertysequence.hxx>
#include <comphelper/scopeguard.hxx>
#include <comphelper/lok.hxx>

#include <editeng/contouritem.hxx>
#include <editeng/editdata.hxx>
#include <editeng/eeitem.hxx>
#include <editeng/flditem.hxx>
#include <editeng/section.hxx>
#include <editeng/editobj.hxx>
#include <editeng/CustomPropertyField.hxx>
#include <editeng/urlfieldhelper.hxx>

#include <sal/log.hxx>

#include <sfx2/bindings.hxx>
#include <sfx2/dispatch.hxx>
#include <sfx2/docfile.hxx>
#include <sfx2/msgpool.hxx>
#include <sfx2/msg.hxx>
#include <sfx2/request.hxx>
#include <sfx2/viewfrm.hxx>
#include <sfx2/zoomitem.hxx>

#include <svx/compressgraphicdialog.hxx>
#include <svx/ClassificationDialog.hxx>
#include <svx/ClassificationCommon.hxx>
#include <svx/bmpmask.hxx>
#include <svx/extedit.hxx>
#include <svx/extrusionbar.hxx>
#include <svx/f3dchild.hxx>
#include <svx/fontwork.hxx>
#include <svx/fontworkbar.hxx>
#include <svx/graphichelper.hxx>
#include <svx/hlnkitem.hxx>
#include <svx/imapdlg.hxx>
#include <svx/sdtagitm.hxx>
#include <svx/svdograf.hxx>
#include <svx/svdoole2.hxx>
#include <svx/svdpagv.hxx>
#include <svx/svdundo.hxx>
#include <svx/svxdlg.hxx>
#include <svx/svxids.hrc>
#include <svx/sdtfsitm.hxx>
#include <svx/sdmetitm.hxx>
#include <svx/zoomslideritem.hxx>
#include <svx/xflclit.hxx>
#include <svx/xlnwtit.hxx>
#include <svx/chrtitem.hxx>
#include <svx/xlnclit.hxx>
#include <svx/xflgrit.hxx>

#include <comphelper/diagnose_ex.hxx>
#include <tools/UnitConversion.hxx>

#include <unotools/useroptions.hxx>

#include <vcl/abstdlg.hxx>
#include <vcl/graph.hxx>
#include <vcl/svapp.hxx>
#include <vcl/unohelp2.hxx>
#include <vcl/weld.hxx>

#include <editeng/cmapitem.hxx>
#include <editeng/escapementitem.hxx>
#include <editeng/kernitem.hxx>
#include <editeng/wghtitem.hxx>
#include <editeng/postitem.hxx>
#include <editeng/udlnitem.hxx>
#include <editeng/crossedoutitem.hxx>
#include <editeng/shdditem.hxx>
#include <editeng/numitem.hxx>
#include <svx/svdobj.hxx>
#include <svx/SvxColorChildWindow.hxx>
#include <editeng/outlobj.hxx>
#include <editeng/flstitem.hxx>
#include <editeng/fontitem.hxx>
#include <editeng/fhgtitem.hxx>
#include <editeng/colritem.hxx>

#include <svl/poolitem.hxx>
#include <svl/style.hxx>
#include <svl/whiter.hxx>

#include <app.hrc>
#include <strings.hrc>

#include <AnimationChildWindow.hxx>
#include <DrawDocShell.hxx>
#include <DrawViewShell.hxx>
#include <LayerTabBar.hxx>
#include <Outliner.hxx>
#include <ViewShellHint.hxx>
#include <ViewShellImplementation.hxx>
#include <Window.hxx>
#include <drawdoc.hxx>
#include <drawview.hxx>
#include <fuarea.hxx>
#include <fubullet.hxx>
#include <fuchar.hxx>
#include <fucushow.hxx>
#include <fuconnct.hxx>
#include <fucopy.hxx>
#include <fudspord.hxx>
#include <fuexecuteinteraction.hxx>
#include <fuexpand.hxx>
#include <fuinsert.hxx>
#include <fuinsfil.hxx>
#include <fuline.hxx>
#include <fulinend.hxx>
#include <fulink.hxx>
#include <fumeasur.hxx>
#include <fumorph.hxx>
#include <fuoaprms.hxx>
#include <fuolbull.hxx>
#include <fupage.hxx>
#include <fuparagr.hxx>
#include <fuprlout.hxx>
#include <fuscale.hxx>
#include <fusel.hxx>
#include <fusldlg.hxx>
#include <fusnapln.hxx>
#include <fusumry.hxx>
#include <futempl.hxx>
#include <futhes.hxx>
#include <futransf.hxx>
#include <futxtatt.hxx>
#include <fuvect.hxx>
#include <futext.hxx>
#include <helpids.h>
#include <sdabstdlg.hxx>
#include <sdattr.hxx>
#include <sdpage.hxx>
#include <sdresid.hxx>
#include <unokywds.hxx>
#include <slideshow.hxx>
#include <stlsheet.hxx>
#include <undolayer.hxx>
#include <sfx2/sidebar/Sidebar.hxx>
#include <sfx2/classificationhelper.hxx>
#include <sdmod.hxx>
#include <model/SlsPageDescriptor.hxx>
#include <model/SlsPageEnumerationProvider.hxx>
#include <SlideSorter.hxx>
#include <view/SlideSorterView.hxx>
#include <SlideSorterViewShell.hxx>
#include <controller/SlideSorterController.hxx>
#include <controller/SlsPageSelector.hxx>
#include <tools/GraphicSizeCheck.hxx>

#include <theme/ThemeColorChanger.hxx>
#include <svx/dialog/ThemeDialog.hxx>
#include <LibreOfficeKit/LibreOfficeKitEnums.h>

#include <ViewShellBase.hxx>
#include <memory>

#include <sfx2/newstyle.hxx>
#include <SelectLayerDlg.hxx>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

#define MIN_ACTIONS_FOR_DIALOG  5000    ///< if there are more meta objects, we show a dialog during the break up

namespace sd {

namespace {

const SvxFieldItem* findField(editeng::Section const & rSection)
{
    for (SfxPoolItem const * pPool: rSection.maAttributes)
    {
        if (pPool->Which() == EE_FEATURE_FIELD)
            return static_cast<const SvxFieldItem*>(pPool);
    }
    return nullptr;
}

bool hasCustomPropertyField(std::vector<editeng::Section> const & aSections, std::u16string_view rName)
{
    for (editeng::Section const & rSection : aSections)
    {
        const SvxFieldItem* pFieldItem = findField(rSection);
        if (pFieldItem)
        {
            const editeng::CustomPropertyField* pCustomPropertyField = dynamic_cast<const editeng::CustomPropertyField*>(pFieldItem->GetField());
            if (pCustomPropertyField && pCustomPropertyField->GetName() == rName)
                return true;
        }
    }
    return false;
}

OUString getWeightString(SfxItemSet const & rItemSet)
{
    OUString sWeightString = u"NORMAL"_ustr;

    if (const SfxPoolItem* pItem = rItemSet.GetItem(EE_CHAR_WEIGHT, false))
    {
        const SvxWeightItem* pWeightItem = dynamic_cast<const SvxWeightItem*>(pItem);
        if (pWeightItem && pWeightItem->GetWeight() == WEIGHT_BOLD)
            sWeightString = "BOLD";
    }
    return sWeightString;
}

class ClassificationCommon
{
protected:
    sd::DrawViewShell& m_rDrawViewShell;
    uno::Reference<document::XDocumentProperties> m_xDocumentProperties;
    uno::Reference<beans::XPropertyContainer> m_xPropertyContainer;
    sfx::ClassificationKeyCreator m_aKeyCreator;
public:
    ClassificationCommon(sd::DrawViewShell& rDrawViewShell, const css::uno::Reference<css::document::XDocumentProperties>& rDocProps)
        : m_rDrawViewShell(rDrawViewShell)
        , m_xDocumentProperties(rDocProps)
        , m_xPropertyContainer(m_xDocumentProperties->getUserDefinedProperties())
        , m_aKeyCreator(SfxClassificationHelper::getPolicyType())
    {}
};

class ClassificationCollector : public ClassificationCommon
{
private:
    std::vector<svx::ClassificationResult> m_aResults;

    void iterateSectionsAndCollect(std::vector<editeng::Section> const & rSections, EditTextObject const & rEditText)
    {
        sal_Int32 nCurrentParagraph = -1;
        OUString sBlank;

        for (editeng::Section const & rSection : rSections)
        {
            // Insert new paragraph if needed
            while (nCurrentParagraph < rSection.mnParagraph)
            {
                nCurrentParagraph++;
                // Get Weight of current paragraph
                OUString sWeightProperty = getWeightString(rEditText.GetParaAttribs(nCurrentParagraph));
                // Insert new paragraph into collection
                m_aResults.emplace_back(svx::ClassificationType::PARAGRAPH, sWeightProperty, sBlank, sBlank);
            }

            const SvxFieldItem* pFieldItem = findField(rSection);
            const editeng::CustomPropertyField* pCustomPropertyField = pFieldItem ?
                dynamic_cast<const editeng::CustomPropertyField*>(pFieldItem->GetField()) :
                nullptr;
            if (pCustomPropertyField)
            {
                const OUString& aKey = pCustomPropertyField->GetName();
                if (m_aKeyCreator.isMarkingTextKey(aKey))
                {
                    m_aResults.emplace_back(svx::ClassificationType::TEXT,
                                           svx::classification::getProperty(m_xPropertyContainer, aKey),
                                           sBlank, sBlank);
                }
                else if (m_aKeyCreator.isCategoryNameKey(aKey) || m_aKeyCreator.isCategoryIdentifierKey(aKey))
                {
                    m_aResults.emplace_back(svx::ClassificationType::CATEGORY,
                                           svx::classification::getProperty(m_xPropertyContainer, aKey),
                                           sBlank, sBlank);
                }
                else if (m_aKeyCreator.isMarkingKey(aKey))
                {
                    m_aResults.emplace_back(svx::ClassificationType::MARKING,
                                           svx::classification::getProperty(m_xPropertyContainer, aKey),
                                           sBlank, sBlank);
                }
                else if (m_aKeyCreator.isIntellectualPropertyPartKey(aKey))
                {
                    m_aResults.emplace_back(svx::ClassificationType::INTELLECTUAL_PROPERTY_PART,
                                           svx::classification::getProperty(m_xPropertyContainer, aKey),
                                           sBlank, sBlank);
                }
            }
        }
    }

public:
    ClassificationCollector(sd::DrawViewShell & rDrawViewShell, const css::uno::Reference<css::document::XDocumentProperties>& rDocProps)
        : ClassificationCommon(rDrawViewShell, rDocProps)
    {}

    std::vector<svx::ClassificationResult> const & getResults() const
    {
        return m_aResults;
    }

    void collect()
    {
        // Set to MASTER mode
        EditMode eOldMode = m_rDrawViewShell.GetEditMode();
        if (eOldMode != EditMode::MasterPage)
            m_rDrawViewShell.ChangeEditMode(EditMode::MasterPage, false);

        // Scoped guard to revert to the previous mode
        comphelper::ScopeGuard const aGuard([this, eOldMode] () {
            m_rDrawViewShell.ChangeEditMode(eOldMode, false);
        });

        const sal_uInt16 nCount = m_rDrawViewShell.GetDoc()->GetMasterSdPageCount(PageKind::Standard);

        for (sal_uInt16 nPageIndex = 0; nPageIndex < nCount; ++nPageIndex)
        {
            SdPage* pMasterPage = m_rDrawViewShell.GetDoc()->GetMasterSdPage(nPageIndex, PageKind::Standard);
            for (const rtl::Reference<SdrObject>& pObject : *pMasterPage)
            {
                SdrRectObj* pRectObject = dynamic_cast<SdrRectObj*>(pObject.get());
                if (pRectObject && pRectObject->GetTextKind() == SdrObjKind::Text)
                {
                    OutlinerParaObject* pOutlinerParagraphObject = pRectObject->GetOutlinerParaObject();
                    if (pOutlinerParagraphObject)
                    {
                        const EditTextObject& rEditText = pOutlinerParagraphObject->GetTextObject();
                        std::vector<editeng::Section> aSections;
                        rEditText.GetAllSections(aSections);

                        // Search for a custom property field that has the classification category identifier key
                        if (hasCustomPropertyField(aSections, m_aKeyCreator.makeCategoryNameKey()))
                        {
                            iterateSectionsAndCollect(aSections, rEditText);
                            return;
                        }
                    }
                }
            }
        }
    }
};

class ClassificationInserter : public ClassificationCommon
{
private:
    /// Delete the previous existing classification object(s) - if they exist
    void deleteExistingObjects()
    {
        OUString sKey = m_aKeyCreator.makeCategoryNameKey();

        const sal_uInt16 nCount = m_rDrawViewShell.GetDoc()->GetMasterSdPageCount(PageKind::Standard);

        for (sal_uInt16 nPageIndex = 0; nPageIndex < nCount; ++nPageIndex)
        {
            SdPage* pMasterPage = m_rDrawViewShell.GetDoc()->GetMasterSdPage(nPageIndex, PageKind::Standard);
            for (const rtl::Reference<SdrObject>& pObject : *pMasterPage)
            {
                SdrRectObj* pRectObject = dynamic_cast<SdrRectObj*>(pObject.get());
                if (pRectObject && pRectObject->GetTextKind() == SdrObjKind::Text)
                {
                    OutlinerParaObject* pOutlinerParagraphObject = pRectObject->GetOutlinerParaObject();
                    if (pOutlinerParagraphObject)
                    {
                        const EditTextObject& rEditText = pOutlinerParagraphObject->GetTextObject();
                        std::vector<editeng::Section> aSections;
                        rEditText.GetAllSections(aSections);

                        if (hasCustomPropertyField(aSections, sKey))
                        {
                            pMasterPage->RemoveObject(pRectObject->GetOrdNum());
                        }
                    }
                }
            }
        }
    }

    void fillTheOutliner(Outliner* pOutliner, std::vector<svx::ClassificationResult> const & rResults)
    {
        sal_Int32 nParagraph = -1;
        for (svx::ClassificationResult const & rResult : rResults)
        {

            ESelection aPosition(nParagraph, EE_TEXTPOS_MAX_COUNT, nParagraph, EE_TEXTPOS_MAX_COUNT);

            switch (rResult.meType)
            {
                case svx::ClassificationType::TEXT:
                {
                    OUString sKey = m_aKeyCreator.makeNumberedTextKey();
                    svx::classification::addOrInsertDocumentProperty(m_xPropertyContainer, sKey, rResult.msName);
                    pOutliner->QuickInsertField(SvxFieldItem(editeng::CustomPropertyField(sKey, rResult.msName), EE_FEATURE_FIELD), aPosition);
                }
                break;

                case svx::ClassificationType::CATEGORY:
                {
                    OUString sKey = m_aKeyCreator.makeCategoryNameKey();
                    pOutliner->QuickInsertField(SvxFieldItem(editeng::CustomPropertyField(sKey, rResult.msName), EE_FEATURE_FIELD), aPosition);
                }
                break;

                case svx::ClassificationType::MARKING:
                {
                    OUString sKey = m_aKeyCreator.makeNumberedMarkingKey();
                    svx::classification::addOrInsertDocumentProperty(m_xPropertyContainer, sKey, rResult.msName);
                    pOutliner->QuickInsertField(SvxFieldItem(editeng::CustomPropertyField(sKey, rResult.msName), EE_FEATURE_FIELD), aPosition);
                }
                break;

                case svx::ClassificationType::INTELLECTUAL_PROPERTY_PART:
                {
                    OUString sKey = m_aKeyCreator.makeNumberedIntellectualPropertyPartKey();
                    svx::classification::addOrInsertDocumentProperty(m_xPropertyContainer, sKey, rResult.msName);
                    pOutliner->QuickInsertField(SvxFieldItem(editeng::CustomPropertyField(sKey, rResult.msName), EE_FEATURE_FIELD), aPosition);
                }
                break;

                case svx::ClassificationType::PARAGRAPH:
                {
                    nParagraph++;
                    pOutliner->Insert(u""_ustr);

                    SfxItemSetFixed<EE_ITEMS_START, EE_ITEMS_END> aItemSet(m_rDrawViewShell.GetDoc()->GetPool());

                    if (rResult.msName == "BOLD")
                        aItemSet.Put(SvxWeightItem(WEIGHT_BOLD, EE_CHAR_WEIGHT));
                    else
                        aItemSet.Put(SvxWeightItem(WEIGHT_NORMAL, EE_CHAR_WEIGHT));

                    SvxNumRule aDefaultNumRule(SvxNumRuleFlags::NONE, 0, false);
                    aItemSet.Put(SvxNumBulletItem(std::move(aDefaultNumRule), EE_PARA_NUMBULLET));

                    pOutliner->SetParaAttribs(nParagraph, aItemSet);
                }
                break;

                default:
                break;
            }
        }
    }

public:
    ClassificationInserter(sd::DrawViewShell & rDrawViewShell, const css::uno::Reference<css::document::XDocumentProperties>& rDocProps)
        : ClassificationCommon(rDrawViewShell, rDocProps)
    {
    }

    void insert(std::vector<svx::ClassificationResult> const & rResults)
    {
        // Set to MASTER mode
        EditMode eOldMode = m_rDrawViewShell.GetEditMode();
        if (eOldMode != EditMode::MasterPage)
            m_rDrawViewShell.ChangeEditMode(EditMode::MasterPage, false);

        // Scoped guard to revert the mode
        comphelper::ScopeGuard const aGuard([this, eOldMode] () {
            m_rDrawViewShell.ChangeEditMode(eOldMode, false);
        });

        // Delete the previous existing object - if exists
        deleteExistingObjects();

        // Clear properties
        svx::classification::removeAllProperties(m_xPropertyContainer);

        SfxClassificationHelper aHelper(m_xDocumentProperties);

        // Apply properties from the BA policy
        for (svx::ClassificationResult const & rResult : rResults)
        {
            if (rResult.meType == svx::ClassificationType::CATEGORY)
                aHelper.SetBACName(rResult.msName, SfxClassificationHelper::getPolicyType());
        }

        // Insert full text as document property
        svx::classification::insertFullTextualRepresentationAsDocumentProperty(m_xPropertyContainer, m_aKeyCreator, rResults);

        // Create the outliner from the
        Outliner* pOutliner = m_rDrawViewShell.GetDoc()->GetInternalOutliner();
        OutlinerMode eOutlinerMode = pOutliner->GetOutlinerMode();

        comphelper::ScopeGuard const aOutlinerGuard([pOutliner, eOutlinerMode] () {
            pOutliner->Init(eOutlinerMode);
        });

        pOutliner->Init(OutlinerMode::TextObject);

        // Fill the outliner with the text from classification result
        fillTheOutliner(pOutliner, rResults);

        // Calculate to outliner text size
        pOutliner->UpdateFields();
        pOutliner->SetUpdateLayout(true);
        Size aTextSize(pOutliner->CalcTextSize());
        pOutliner->SetUpdateLayout(false);

        // Create objects, apply the outliner and add them (objects) to all master pages
        const sal_uInt16 nCount = m_rDrawViewShell.GetDoc()->GetMasterSdPageCount(PageKind::Standard);

        for (sal_uInt16 nPageIndex = 0; nPageIndex < nCount; ++nPageIndex)
        {
            SdPage* pMasterPage = m_rDrawViewShell.GetDoc()->GetMasterSdPage(nPageIndex, PageKind::Standard);
            if (!pMasterPage)
                continue;

            rtl::Reference<SdrRectObj> pObject = new SdrRectObj(
                *m_rDrawViewShell.GetDoc(), // TTTT should be reference
                SdrObjKind::Text);
            pObject->SetMergedItem(makeSdrTextAutoGrowWidthItem(true));
            pObject->SetOutlinerParaObject(pOutliner->CreateParaObject());
            pMasterPage->InsertObject(pObject.get());

            // Calculate position
            ::tools::Rectangle aRectangle(Point(), pMasterPage->GetSize());
            Point aPosition(aRectangle.Center().X(), aRectangle.Bottom());

            aPosition.AdjustX( -(aTextSize.Width() / 2) );
            aPosition.AdjustY( -(aTextSize.Height()) );

            pObject->SetLogicRect(::tools::Rectangle(aPosition, aTextSize));
        }
    }
};

    void lcl_convertStringArguments(const std::unique_ptr<SfxItemSet>& pArgs)
    {
        const SfxPoolItem* pItem = nullptr;

        if (SfxItemState::SET == pArgs->GetItemState(SID_ATTR_LINE_WIDTH_ARG, false, &pItem))
        {
            double fValue = static_cast<const SvxDoubleItem*>(pItem)->GetValue();
            // FIXME: different units...
            int nPow = 100;
            int nValue = fValue * nPow;

            XLineWidthItem aItem(nValue);
            pArgs->Put(aItem);
        }
        if (SfxItemState::SET == pArgs->GetItemState(SID_FILL_GRADIENT_JSON, false, &pItem))
        {
            const SfxStringItem* pJSON = static_cast<const SfxStringItem*>(pItem);
            if (pJSON)
            {
                basegfx::BGradient aGradient = basegfx::BGradient::fromJSON(pJSON->GetValue());
                XFillGradientItem aItem(aGradient);
                pArgs->Put(aItem);
            }
        }
    }
}

/**
 * SfxRequests for temporary actions
 */

void DrawViewShell::FuTemporary(SfxRequest& rReq)
{
    // during a native slide show nothing gets executed!
    if(SlideShow::IsRunning( GetViewShellBase() )
        && !SlideShow::IsInteractiveSlideshow( &GetViewShellBase() ) // IASS
        && (rReq.GetSlot() != SID_NAVIGATOR))
        return;

    DBG_ASSERT( mpDrawView, "sd::DrawViewShell::FuTemporary(), no draw view!" );
    if( !mpDrawView )
        return;

    CheckLineTo (rReq);

    DeactivateCurrentFunction();

    sal_uInt16 nSId = rReq.GetSlot();

    switch ( nSId )
    {
        case SID_OUTLINE_TEXT_AUTOFIT:
        {
            SfxUndoManager* pUndoManager = GetDocSh()->GetUndoManager();
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            if( rMarkList.GetMarkCount() == 1 )
            {
                pUndoManager->EnterListAction(u""_ustr, u""_ustr, 0, GetViewShellBase().GetViewShellId());
                mpDrawView->BegUndo();

                SdrObject* pObj = rMarkList.GetMark(0)->GetMarkedSdrObj();
                bool bSet = pObj->GetMergedItemSet().GetItem<SdrTextFitToSizeTypeItem>(SDRATTR_TEXT_FITTOSIZE)->GetValue() != drawing::TextFitToSizeType_NONE;

                mpDrawView->AddUndo(GetDoc()->GetSdrUndoFactory().CreateUndoAttrObject(*pObj));

                if (!bSet)
                {
                    //If we are turning on AutoFit we have to turn these off if already on
                    if (pObj->GetMergedItemSet().GetItem<SdrOnOffItem>(SDRATTR_TEXT_AUTOGROWHEIGHT)->GetValue())
                        pObj->SetMergedItem(makeSdrTextAutoGrowHeightItem(false));
                    if (pObj->GetMergedItemSet().GetItem<SdrOnOffItem>(SDRATTR_TEXT_AUTOGROWWIDTH)->GetValue())
                        pObj->SetMergedItem(makeSdrTextAutoGrowWidthItem(false));
                }

                pObj->SetMergedItem(SdrTextFitToSizeTypeItem(bSet ? drawing::TextFitToSizeType_NONE : drawing::TextFitToSizeType_AUTOFIT));

                mpDrawView->EndUndo();
                pUndoManager->LeaveListAction();
            }
            Cancel();
            rReq.Done();
        }
        break;

        // area and line attributes: shall have
        // an own Execute method (like StateMethode)
        case SID_ATTR_FILL_STYLE:
        case SID_ATTR_FILL_COLOR:
        case SID_ATTR_FILL_GRADIENT:
        case SID_ATTR_FILL_HATCH:
        case SID_ATTR_FILL_BITMAP:
        case SID_ATTR_FILL_SHADOW:
        case SID_ATTR_SHADOW_COLOR:
        case SID_ATTR_SHADOW_TRANSPARENCE:
        case SID_ATTR_SHADOW_BLUR:
        case SID_ATTR_SHADOW_XDISTANCE:
        case SID_ATTR_SHADOW_YDISTANCE:
        case SID_ATTR_FILL_USE_SLIDE_BACKGROUND:
        case SID_ATTR_FILL_TRANSPARENCE:
        case SID_ATTR_FILL_FLOATTRANSPARENCE:

        case SID_ATTR_LINE_STYLE:
        case SID_ATTR_LINE_DASH:
        case SID_ATTR_LINE_WIDTH:
        case SID_ATTR_LINE_COLOR:
        case SID_ATTR_LINEEND_STYLE:
        case SID_ATTR_LINE_START:
        case SID_ATTR_LINE_END:
        case SID_ATTR_LINE_TRANSPARENCE:
        case SID_ATTR_LINE_JOINT:
        case SID_ATTR_LINE_CAP:

        case SID_ATTR_TEXT_FITTOSIZE:
        {
            if( rReq.GetArgs() )
            {
                std::unique_ptr<SfxItemSet> pNewArgs = rReq.GetArgs()->Clone();
                lcl_convertStringArguments(pNewArgs);
                mpDrawView->SetAttributes(*pNewArgs);
                rReq.Done();
            }
            else
            {
                switch( rReq.GetSlot() )
                {
                    case SID_ATTR_FILL_SHADOW:
                    case SID_ATTR_SHADOW_COLOR:
                    case SID_ATTR_SHADOW_TRANSPARENCE:
                    case SID_ATTR_SHADOW_BLUR:
                    case SID_ATTR_SHADOW_XDISTANCE:
                    case SID_ATTR_SHADOW_YDISTANCE:
                    case SID_ATTR_FILL_STYLE:
                    case SID_ATTR_FILL_COLOR:
                    case SID_ATTR_FILL_GRADIENT:
                    case SID_ATTR_FILL_HATCH:
                    case SID_ATTR_FILL_BITMAP:
                    case SID_ATTR_FILL_USE_SLIDE_BACKGROUND:
                    case SID_ATTR_FILL_TRANSPARENCE:
                    case SID_ATTR_FILL_FLOATTRANSPARENCE:
                        GetViewFrame()->GetDispatcher()->Execute( SID_ATTRIBUTES_AREA, SfxCallMode::ASYNCHRON );
                        break;
                    case SID_ATTR_LINE_STYLE:
                    case SID_ATTR_LINE_DASH:
                    case SID_ATTR_LINE_WIDTH:
                    case SID_ATTR_LINE_COLOR:
                    case SID_ATTR_LINE_TRANSPARENCE:
                    case SID_ATTR_LINE_JOINT:
                    case SID_ATTR_LINE_CAP:
                        GetViewFrame()->GetDispatcher()->Execute( SID_ATTRIBUTES_LINE, SfxCallMode::ASYNCHRON );
                        break;
                    case SID_ATTR_TEXT_FITTOSIZE:
                        GetViewFrame()->GetDispatcher()->Execute( SID_TEXTATTR_DLG, SfxCallMode::ASYNCHRON );
                        break;
                }
            }
            Cancel();
        }
        break;

        case SID_HYPHENATION:
        {
            const SfxBoolItem* pItem = rReq.GetArg<SfxBoolItem>(SID_HYPHENATION);

            if( pItem )
            {
                SfxItemSetFixed<EE_PARA_HYPHENATE, EE_PARA_HYPHENATE> aSet( GetPool() );
                bool bValue = pItem->GetValue();
                aSet.Put( SfxBoolItem( EE_PARA_HYPHENATE, bValue ) );
                mpDrawView->SetAttributes( aSet );
            }
            else // only for testing purpose
            {
                OSL_FAIL(" no value for hyphenation!");
                SfxItemSetFixed<EE_PARA_HYPHENATE, EE_PARA_HYPHENATE> aSet( GetPool() );
                aSet.Put( SfxBoolItem( EE_PARA_HYPHENATE, true ) );
                mpDrawView->SetAttributes( aSet );
            }
            rReq.Done();
            Cancel();
        }
        break;

        case SID_INSERTPAGE:
        case SID_INSERTPAGE_QUICK:
        {
            SdPage* pNewPage = CreateOrDuplicatePage (rReq, mePageKind, GetActualPage());
            Cancel();
            if(HasCurrentFunction(SID_BEZIER_EDIT) )
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            if (pNewPage != nullptr)
                SwitchPage((pNewPage->GetPageNum()-1)/2);
            rReq.Done ();
        }
        break;

        case SID_DUPLICATE_PAGE:
        {
            auto slideSorter = sd::slidesorter::SlideSorterViewShell::GetSlideSorter(GetViewShellBase());
            SdPage* pNewPage = nullptr;
            if(slideSorter)
                DuplicateSelectedSlides(rReq);
            else
                pNewPage = CreateOrDuplicatePage (rReq, mePageKind, GetActualPage());
            Cancel();
            if(HasCurrentFunction(SID_BEZIER_EDIT) )
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            if(!slideSorter && pNewPage != nullptr)
                SwitchPage((pNewPage->GetPageNum()-1)/2);
            rReq.Done();
        }
        break;

        case SID_INSERT_MASTER_PAGE:
        {
            // Use the API to create a new page.
            Reference<drawing::XMasterPagesSupplier> xMasterPagesSupplier (
                GetDoc()->getUnoModel(), UNO_QUERY);
            if (xMasterPagesSupplier.is())
            {
                Reference<drawing::XDrawPages> xMasterPages (
                    xMasterPagesSupplier->getMasterPages());
                if (xMasterPages.is())
                {
                    sal_uInt16 nIndex = GetCurPagePos() + 1;
                    xMasterPages->insertNewByIndex (nIndex);

                    // Create shapes for the default layout.
                    SdPage* pMasterPage = GetDoc()->GetMasterSdPage(
                        nIndex, PageKind::Standard);
                    pMasterPage->CreateTitleAndLayout (true,true);
                }
            }

            Cancel();
            if(HasCurrentFunction(SID_BEZIER_EDIT))
                GetViewFrame()->GetDispatcher()->Execute(
                    SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            rReq.Done ();
        }
        break;

        case SID_MODIFYPAGE:
        {
            if (mePageKind==PageKind::Standard || mePageKind==PageKind::Notes ||
                (mePageKind==PageKind::Handout && meEditMode==EditMode::MasterPage) )
            {
                if ( mpDrawView->IsTextEdit() )
                {
                    mpDrawView->SdrEndTextEdit();
                }
                sal_uInt16 nPage = maTabControl->GetCurPagePos();
                mpActualPage = GetDoc()->GetSdPage(nPage, mePageKind);
                ::sd::ViewShell::mpImpl->ProcessModifyPageSlot (
                    rReq,
                    mpActualPage,
                    mePageKind);
            }

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_ASSIGN_LAYOUT:
        {
            if (mePageKind==PageKind::Standard || mePageKind==PageKind::Notes || (mePageKind==PageKind::Handout && meEditMode==EditMode::MasterPage))
            {
                if ( mpDrawView->IsTextEdit() )
                    mpDrawView->SdrEndTextEdit();

                ::sd::ViewShell::mpImpl->AssignLayout(rReq, mePageKind);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_RENAMEPAGE:
        case SID_RENAME_MASTER_PAGE:
        {
            if (mePageKind==PageKind::Standard || mePageKind==PageKind::Notes )
            {
                if ( mpDrawView->IsTextEdit() )
                {
                    mpDrawView->SdrEndTextEdit();
                }

                sal_uInt16 nPage = maTabControl->GetCurPagePos();
                SdPage* pCurrentPage = ( GetEditMode() == EditMode::Page )
                    ? GetDoc()->GetSdPage( nPage, GetPageKind() )
                    : GetDoc()->GetMasterSdPage( nPage, GetPageKind() );

                OUString aTitle = SdResId(STR_TITLE_RENAMESLIDE);
                OUString aDescr = SdResId(STR_DESC_RENAMESLIDE);
                const OUString& aPageName = pCurrentPage->GetName();

                if(rReq.GetArgs())
                {
                    OUString aName = rReq.GetArgs()->GetItem<const SfxStringItem>(SID_RENAMEPAGE)->GetValue();

                    bool bResult = RenameSlide( maTabControl->GetPageId(nPage), aName );
                    DBG_ASSERT( bResult, "Couldn't rename slide" );
                }
                else
                {
                    SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                    ScopedVclPtr<AbstractSvxNameDialog> aNameDlg(pFact->CreateSvxNameDialog(GetFrameWeld(), aPageName, aDescr));
                    aNameDlg->SetText( aTitle );
                    aNameDlg->SetCheckNameHdl( LINK( this, DrawViewShell, RenameSlideHdl ) );
                    aNameDlg->SetEditHelpId( HID_SD_NAMEDIALOG_PAGE );

                    if( aNameDlg->Execute() == RET_OK )
                    {
                        OUString aNewName = aNameDlg->GetName();
                        if (aNewName != aPageName)
                        {
                            bool bResult = RenameSlide( maTabControl->GetPageId(nPage), aNewName );
                            DBG_ASSERT( bResult, "Couldn't rename slide" );
                        }
                    }
                }
            }
            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_RENAMEPAGE_QUICK:
        {
            if (mePageKind==PageKind::Standard || mePageKind==PageKind::Notes )
            {
                if ( mpDrawView->IsTextEdit() )
                {
                    mpDrawView->SdrEndTextEdit();
                }

                maTabControl->StartEditMode( maTabControl->GetCurPageId() );
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_PAGESIZE :  // either this (no menu entries or something else!)
        {
            const SfxItemSet *pArgs = rReq.GetArgs ();

            if (pArgs && pArgs->Count () == 3)
            {
                const SfxUInt32Item* pWidth = rReq.GetArg<SfxUInt32Item>(ID_VAL_PAGEWIDTH);
                const SfxUInt32Item* pHeight = rReq.GetArg<SfxUInt32Item>(ID_VAL_PAGEHEIGHT);
                const SfxBoolItem* pScaleAll = rReq.GetArg<SfxBoolItem>(ID_VAL_SCALEOBJECTS);

                Size aSize (pWidth->GetValue (), pHeight->GetValue ());

                SetupPage (aSize, 0, 0, 0, 0, true, false, pScaleAll->GetValue ());
                rReq.Ignore ();
                break;
            }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            rReq.Ignore ();
            break;
        }

        case SID_PAGEMARGIN :  // or this (no menu entries or something else!)
        {
            const SfxItemSet *pArgs = rReq.GetArgs ();

            if (pArgs && pArgs->Count () == 5)
            {
                const SfxUInt32Item* pLeft = rReq.GetArg<SfxUInt32Item>(ID_VAL_PAGELEFT);
                const SfxUInt32Item* pRight = rReq.GetArg<SfxUInt32Item>(ID_VAL_PAGERIGHT);
                const SfxUInt32Item* pUpper = rReq.GetArg<SfxUInt32Item>(ID_VAL_PAGETOP);
                const SfxUInt32Item* pLower = rReq.GetArg<SfxUInt32Item>(ID_VAL_PAGEBOTTOM);
                const SfxBoolItem* pScaleAll = rReq.GetArg<SfxBoolItem>(ID_VAL_SCALEOBJECTS);

                Size aEmptySize (0, 0);

                SetupPage (aEmptySize, pLeft->GetValue (), pRight->GetValue (),
                           pUpper->GetValue (), pLower->GetValue (),
                           false, true, pScaleAll->GetValue ());
                rReq.Ignore ();
                break;
            }
#if HAVE_FEATURE_SCRIPTING
            StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
            rReq.Ignore ();
            break;
        }

        case SID_ATTR_ZOOMSLIDER:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();

            const SfxUInt16Item* pScale = (pArgs && pArgs->Count () == 1) ?
                rReq.GetArg(SID_ATTR_ZOOMSLIDER) : nullptr;
            if (pScale && CHECK_RANGE (5, pScale->GetValue (), 3000))
            {
                SetZoom (pScale->GetValue ());

                SfxBindings& rBindings = GetViewFrame()->GetBindings();
                rBindings.Invalidate( SID_ATTR_ZOOM );
                rBindings.Invalidate( SID_ZOOM_IN );
                rBindings.Invalidate( SID_ZOOM_OUT );
                rBindings.Invalidate( SID_ATTR_ZOOMSLIDER );

            }

            Cancel();
            rReq.Done ();
            break;
        }

        case SID_ATTR_ZOOM:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            mbZoomOnPage = false;

            if ( pArgs )
            {
                SvxZoomType eZT = pArgs->Get( SID_ATTR_ZOOM ).GetType();
                switch( eZT )
                {
                    case SvxZoomType::PERCENT:
                        SetZoom( static_cast<::tools::Long>( pArgs->Get( SID_ATTR_ZOOM ).GetValue()) );
                        break;

                    case SvxZoomType::OPTIMAL:
                        GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_ALL,
                                    SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
                        break;

                    case SvxZoomType::PAGEWIDTH:
                        GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_PAGE_WIDTH,
                                    SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
                        break;

                    case SvxZoomType::WHOLEPAGE:
                        GetViewFrame()->GetDispatcher()->Execute( SID_SIZE_PAGE,
                                    SfxCallMode::ASYNCHRON | SfxCallMode::RECORD );
                        break;
                    case SvxZoomType::PAGEWIDTH_NOBORDER:
                        OSL_FAIL("sd::DrawViewShell::FuTemporary(), SvxZoomType::PAGEWIDTH_NOBORDER not handled!" );
                        break;
                }
                rReq.Ignore ();
            }
            else
            {
                // open zoom dialog
                SetCurrentFunction( FuScale::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            }
            Cancel();
        }
        break;

        case SID_CHANGEBEZIER:
        case SID_CHANGEPOLYGON:
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            }

            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                if( rReq.GetSlot() == SID_CHANGEBEZIER )
                {
                    weld::WaitObject aWait(GetFrameWeld());
                    mpDrawView->ConvertMarkedToPathObj(false);
                }
                else
                {
                    if( mpDrawView->IsVectorizeAllowed() )
                    {
                        SetCurrentFunction( FuVectorize::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
                    }
                    else
                    {
                        weld::WaitObject aWait(GetFrameWeld());
                        mpDrawView->ConvertMarkedToPolyObj();
                    }
                }

                Invalidate(SID_CHANGEBEZIER);
                Invalidate(SID_CHANGEPOLYGON);
            }
            Cancel();

            if( HasCurrentFunction(SID_BEZIER_EDIT) )
            {   // where applicable, activate right edit action
                GetViewFrame()->GetDispatcher()->Execute(SID_SWITCH_POINTEDIT,
                                        SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);
            }
            rReq.Ignore ();
            break;

        case SID_CONVERT_TO_CONTOUR:
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            }

            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->ConvertMarkedToPathObj(true);

                Invalidate(SID_CONVERT_TO_CONTOUR);
            }
            Cancel();

            rReq.Ignore ();
            break;

        case SID_CONVERT_TO_METAFILE:
        case SID_CONVERT_TO_BITMAP:
        {
            // End text edit mode when it is active because the metafile or
            // bitmap that will be created does not support it.
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
                GetViewFrame()->GetDispatcher()->Execute(SID_OBJECT_SELECT, SfxCallMode::ASYNCHRON);
            }

            if ( mpDrawView->IsPresObjSelected(true,true,true) )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                weld::WaitObject aWait(GetFrameWeld());

                // create SdrGrafObj from metafile/bitmap
                Graphic aGraphic;
                switch (nSId)
                {
                    case SID_CONVERT_TO_METAFILE:
                    {
                        // switch on undo for the next operations
                        mpDrawView->BegUndo(SdResId(STR_UNDO_CONVERT_TO_METAFILE));
                        GDIMetaFile aMetaFile(mpDrawView->GetMarkedObjMetaFile());
                        aGraphic = Graphic(aMetaFile);
                    }
                    break;
                    case SID_CONVERT_TO_BITMAP:
                    {
                        // Disable spelling during conversion
                        bool bOnlineSpell = GetDoc()->GetOnlineSpell();
                        GetDoc()->SetOnlineSpell(false);

                        // switch on undo for the next operations
                        mpDrawView->BegUndo(SdResId(STR_UNDO_CONVERT_TO_BITMAP));
                        bool bDone(false);

                        // I have to get the image here directly since GetMarkedObjBitmapEx works
                        // based on Bitmaps, but not on BitmapEx, thus throwing away the alpha
                        // channel. Argh! GetMarkedObjBitmapEx itself is too widely used to safely
                        // change that, e.g. in the exchange formats. For now I can only add this
                        // exception to get good results for Svgs. This is how the code gets more
                        // and more crowded, at last I made a remark for myself to change this
                        // as one of the next tasks.
                        if(1 == mpDrawView->GetMarkedObjectList().GetMarkCount())
                        {
                            const SdrGrafObj* pSdrGrafObj = dynamic_cast< const SdrGrafObj* >(mpDrawView->GetMarkedObjectByIndex(0));

                            if(pSdrGrafObj && pSdrGrafObj->isEmbeddedVectorGraphicData())
                            {
                                aGraphic = Graphic(pSdrGrafObj->GetGraphic().getVectorGraphicData()->getReplacement());
                                bDone = true;
                            }
                        }

                        if(!bDone)
                        {
                            aGraphic = Graphic(mpDrawView->GetMarkedObjBitmapEx());
                        }
                        // Restore online spelling
                        GetDoc()->SetOnlineSpell(bOnlineSpell);
                    }
                    break;
                }

                // create new object
                rtl::Reference<SdrGrafObj> pGraphicObj = new SdrGrafObj(
                    *GetDoc(),
                    aGraphic);

                // get some necessary info and ensure it
                const SdrMarkList& rMarkList(mpDrawView->GetMarkedObjectList());
                const size_t nMarkCount(rMarkList.GetMarkCount());
                SdrPageView* pPageView = mpDrawView->GetSdrPageView();
                OSL_ENSURE(nMarkCount, "DrawViewShell::FuTemporary: SID_CONVERT_TO_BITMAP with empty selection (!)");
                OSL_ENSURE(pPageView, "DrawViewShell::FuTemporary: SID_CONVERT_TO_BITMAP without SdrPageView (!)");

                // fit rectangle of new graphic object to selection's mark rect
                ::tools::Rectangle aAllMarkedRect;
                rMarkList.TakeBoundRect(pPageView, aAllMarkedRect);
                pGraphicObj->SetLogicRect(aAllMarkedRect);

                // #i71540# to keep the order, it is necessary to replace the lowest object
                // of the selection with the new object. This also means that with multi
                // selection, all other objects need to be deleted first
                SdrMark* pFirstMark = rMarkList.GetMark(0);
                SdrObject* pReplacementCandidate = pFirstMark->GetMarkedSdrObj();

                if(nMarkCount > 1)
                {
                    // take first object out of selection
                    mpDrawView->MarkObj(pReplacementCandidate, pPageView, true, true);

                    // clear remaining selection
                    mpDrawView->DeleteMarkedObj();
                }

                // #i124816# copy layer from lowest object which gets replaced
                pGraphicObj->SetLayer(pReplacementCandidate->GetLayer());

                // now replace lowest object with new one
                mpDrawView->ReplaceObjectAtView(pReplacementCandidate, *pPageView, pGraphicObj.get());

                // switch off undo
                mpDrawView->EndUndo();
            }
        }

        Cancel();

        rReq.Done ();
        break;

        case SID_REMOVE_HYPERLINK:
        {
            if (mpDrawView->IsTextEdit())
            {
                OutlinerView* pOutView = mpDrawView->GetTextEditOutlinerView();
                if (pOutView)
                    URLFieldHelper::RemoveURLField(pOutView->GetEditView());
            }
        }
        Cancel();
        rReq.Done ();
        break;

        case SID_SET_DEFAULT:
        {
            std::optional<SfxItemSet> pSet;

            if (mpDrawView->IsTextEdit())
            {
                pSet.emplace( GetPool(), svl::Items<EE_ITEMS_START, EE_ITEMS_END> );
                mpDrawView->SetAttributes( *pSet, true );
            }
            else
            {
                const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
                const size_t nCount = rMarkList.GetMarkCount();

                // For every presentation object a SfxItemSet of hard attributes
                // and the UserCall is stored in this list. This is because
                // at the following mpDrawView->SetAttributes( *pSet, sal_True )
                // they get lost and have to be restored.
                std::vector<std::pair<std::unique_ptr<SfxItemSet>,SdrObjUserCall*> > aAttrList;
                SdPage* pPresPage = static_cast<SdPage*>( mpDrawView->GetSdrPageView()->GetPage() );

                for ( size_t i = 0; i < nCount; ++i )
                {
                    SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();

                    if( pPresPage->IsPresObj( pObj ) )
                    {
                        auto pNewSet = std::make_unique<SfxItemSetFixed<SDRATTR_TEXT_MINFRAMEHEIGHT, SDRATTR_TEXT_AUTOGROWHEIGHT>>( GetDoc()->GetPool() );
                        pNewSet->Put(pObj->GetMergedItemSet());
                        aAttrList.emplace_back(std::move(pNewSet), pObj->GetUserCall());
                    }
                }

                pSet.emplace( GetPool() );
                mpDrawView->SetAttributes( *pSet, true );

                sal_uLong j = 0;

                for ( size_t i = 0; i < nCount; ++i )
                {
                    SfxStyleSheet* pSheet = nullptr;
                    SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();

                    if (pObj->GetObjIdentifier() == SdrObjKind::TitleText)
                    {
                        pSheet = mpActualPage->GetStyleSheetForPresObj(PresObjKind::Title);
                        if (pSheet)
                            pObj->SetStyleSheet(pSheet, false);
                    }
                    else if(pObj->GetObjIdentifier() == SdrObjKind::OutlineText)
                    {
                        for (sal_uInt16 nLevel = 1; nLevel < 10; nLevel++)
                        {
                            pSheet = mpActualPage->GetStyleSheetForPresObj( PresObjKind::Outline );
                            DBG_ASSERT(pSheet, "Template for outline object not found");
                            if (pSheet)
                            {
                                pObj->StartListening(*pSheet);

                                if( nLevel == 1 )
                                    // text frame listens on StyleSheet of level1
                                    pObj->NbcSetStyleSheet(pSheet, false);
                            }
                        }
                    }

                    if( pPresPage->IsPresObj( pObj ) )
                    {
                        std::pair<std::unique_ptr<SfxItemSet>,SdrObjUserCall*> &rAttr = aAttrList[j++];

                        std::unique_ptr<SfxItemSet> & pNewSet(rAttr.first);
                        SdrObjUserCall* pUserCall = rAttr.second;

                        if ( pNewSet && pNewSet->GetItemState( SDRATTR_TEXT_MINFRAMEHEIGHT ) == SfxItemState::SET )
                        {
                            pObj->SetMergedItem(pNewSet->Get(SDRATTR_TEXT_MINFRAMEHEIGHT));
                        }

                        if ( pNewSet && pNewSet->GetItemState( SDRATTR_TEXT_AUTOGROWHEIGHT ) == SfxItemState::SET )
                        {
                            pObj->SetMergedItem(pNewSet->Get(SDRATTR_TEXT_AUTOGROWHEIGHT));
                        }

                        if( pUserCall )
                            pObj->SetUserCall( pUserCall );
                    }
                }
            }

            pSet.reset();
            Cancel();
        }
        break;

        case SID_DELETE_SNAPITEM:
        {
            SdrPageView* pPV;
            Point   aMPos = GetActiveWindow()->PixelToLogic( maMousePos );
            sal_uInt16  nHitLog = static_cast<sal_uInt16>(GetActiveWindow()->PixelToLogic( Size(
                FuPoor::HITPIX, 0 ) ).Width());
            sal_uInt16  nHelpLine;

            if( mpDrawView->PickHelpLine( aMPos, nHitLog, *GetActiveWindow()->GetOutDev(), nHelpLine, pPV) )
            {
                pPV->DeleteHelpLine( nHelpLine );
            }
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_DELETE_PAGE:
        case SID_DELETE_MASTER_PAGE:
            DeleteActualPage();
            Cancel();
            rReq.Ignore ();
        break;

        case SID_DELETE_LAYER:
            DeleteActualLayer();
            Cancel();
            rReq.Ignore ();
        break;

        case SID_ORIGINAL_SIZE:
            mpDrawView->SetMarkedOriginalSize();
            Cancel();
            rReq.Done();
        break;

        case SID_DRAW_FONTWORK:
        case SID_DRAW_FONTWORK_VERTICAL:
        {
            svx::FontworkBar::execute(*mpView, rReq, GetViewFrame()->GetBindings());       // SJ: can be removed  (I think)
            Cancel();
            rReq.Done();
        }
        break;

        case SID_SAVE_GRAPHIC:
        {
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            if( rMarkList.GetMarkCount() == 1 )
            {
                const SdrGrafObj* pObj = dynamic_cast<const SdrGrafObj*>(rMarkList.GetMark(0)->GetMarkedSdrObj());
                if (pObj && pObj->GetGraphicType() == GraphicType::Bitmap)
                {
                    weld::Window* pFrame = GetFrameWeld();
                    GraphicAttr aGraphicAttr = pObj->GetGraphicAttr();
                    short nState = RET_CANCEL;
                    if (aGraphicAttr != GraphicAttr()) // the image has been modified
                    {
                        if (pFrame)
                        {
                            nState = GraphicHelper::HasToSaveTransformedImage(pFrame);
                        }
                    }
                    else
                    {
                        nState = RET_NO;
                    }

                    if (nState == RET_YES)
                    {
                        GraphicHelper::ExportGraphic(pFrame, pObj->GetTransformedGraphic(), u""_ustr);
                    }
                    else if (nState == RET_NO)
                    {
                        const GraphicObject& aGraphicObject(pObj->GetGraphicObject());
                        GraphicHelper::ExportGraphic(pFrame, aGraphicObject.GetGraphic(), u""_ustr);
                    }
                }
            }
            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_EXTERNAL_EDIT:
        {
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            if( rMarkList.GetMarkCount() == 1 )
            {
                SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();
                if( auto pGraphicObj = dynamic_cast<SdrGrafObj*>( pObj ) )
                    if( pGraphicObj->GetGraphicType() == GraphicType::Bitmap )
                    {
                        GraphicObject aGraphicObject( pGraphicObj->GetGraphicObject() );
                        m_ExternalEdits.push_back(
                            std::make_unique<SdrExternalToolEdit>(
                                mpDrawView.get(), pGraphicObj));
                        m_ExternalEdits.back()->Edit( &aGraphicObject );
                    }
            }
            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_COMPRESS_GRAPHIC:
        {
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            if( rMarkList.GetMarkCount() == 1 )
            {
                SdrObject* pObj = rMarkList.GetMark( 0 )->GetMarkedSdrObj();

                if( auto pGraphicObj = dynamic_cast<SdrGrafObj*>( pObj ) )
                    if( pGraphicObj->GetGraphicType() == GraphicType::Bitmap )
                    {
                        CompressGraphicsDialog dialog(GetFrameWeld(), pGraphicObj, GetViewFrame()->GetBindings() );
                        if (dialog.run() == RET_OK)
                        {
                            rtl::Reference<SdrGrafObj> pNewObject = dialog.GetCompressedSdrGrafObj();
                            SdrPageView* pPageView = mpDrawView->GetSdrPageView();
                            OUString aUndoString = mpDrawView->GetDescriptionOfMarkedObjects() + " Compress";
                            mpDrawView->BegUndo( aUndoString );
                            mpDrawView->ReplaceObjectAtView( pObj, *pPageView, pNewObject.get() );
                            mpDrawView->EndUndo();
                        }
                    }
            }
            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_GRAPHIC_SIZE_CHECK:
        {
            sd::GraphicSizeCheckGUIResult aResult(GetDoc());
            svx::GenericCheckDialog aDialog(GetFrameWeld(), aResult);
            aDialog.run();

            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_ATTRIBUTES_LINE:  // BASIC
        {
            SetCurrentFunction( FuLine::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            if (rReq.GetArgs())
                Cancel();
        }
        break;

        case SID_ATTRIBUTES_AREA:  // BASIC
        {
            SetCurrentFunction( FuArea::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            if (rReq.GetArgs())
                Cancel();
        }
        break;

        case SID_ATTR_TRANSFORM:
        {
            SetCurrentFunction( FuTransform::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            // tdf#138963 conditions tested for here must be the same as those
            // of the early returns from FuTransform::DoExecute
            if (rReq.GetArgs() || !mpDrawView->AreObjectsMarked())
            {
                Invalidate(SID_RULER_OBJECT);
                Cancel();
            }
        }
        break;
        case SID_MOVE_SHAPE_HANDLE:
        {
            const SfxItemSet *pArgs = rReq.GetArgs ();
            if (pArgs && pArgs->Count () >= 3)
            {
                const SfxUInt32Item* handleNumItem = rReq.GetArg<SfxUInt32Item>(FN_PARAM_1);
                const SfxUInt32Item* newPosXTwips = rReq.GetArg<SfxUInt32Item>(FN_PARAM_2);
                const SfxUInt32Item* newPosYTwips = rReq.GetArg<SfxUInt32Item>(FN_PARAM_3);
                const SfxInt32Item* OrdNum = rReq.GetArg<SfxInt32Item>(FN_PARAM_4);

                const sal_uLong handleNum = handleNumItem->GetValue();
                const sal_uLong newPosX = convertTwipToMm100(newPosXTwips->GetValue());
                const sal_uLong newPosY = convertTwipToMm100(newPosYTwips->GetValue());

                mpDrawView->MoveShapeHandle(handleNum, Point(newPosX, newPosY), OrdNum ? OrdNum->GetValue() : -1);
                Cancel();
            }
            break;
        }
        case SID_CHAR_DLG_EFFECT:
        case SID_CHAR_DLG:  // BASIC
        {
            SetCurrentFunction( FuChar::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_PARA_DLG:
        {
            SetCurrentFunction( FuParagraph::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case FN_NUM_BULLET_ON:
        {
            // The value (sal_uInt16)0xFFFF means set bullet on/off.
            SfxUInt16Item aItem(FN_SVX_SET_BULLET, sal_uInt16(0xFFFF));
            GetViewFrame()->GetDispatcher()->ExecuteList(FN_SVX_SET_BULLET,
                    SfxCallMode::RECORD, { &aItem });
        }
        break;

        case FN_NUM_NUMBERING_ON:
        {
            // The value (sal_uInt16)0xFFFF means set bullet on/off.
            SfxUInt16Item aItem(FN_SVX_SET_NUMBER, sal_uInt16(0xFFFF));
            GetViewFrame()->GetDispatcher()->ExecuteList(FN_SVX_SET_NUMBER,
                    SfxCallMode::RECORD, { &aItem });
        }
        break;

        case SID_OUTLINE_BULLET:
        case FN_SVX_SET_BULLET:
        case FN_SVX_SET_NUMBER:
        {
            SetCurrentFunction( FuBulletAndPosition::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case FN_INSERT_SOFT_HYPHEN:
        case FN_INSERT_HARDHYPHEN:
        case FN_INSERT_HARD_SPACE:
        case FN_INSERT_NNBSP:
        case SID_INSERT_RLM :
        case SID_INSERT_LRM :
        case SID_INSERT_WJ :
        case SID_INSERT_ZWSP:
        case SID_CHARMAP:
        {
            SetCurrentFunction( FuBullet::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_PRESENTATION_LAYOUT:
        {
            SetCurrentFunction( FuPresentationLayout::Create(this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq) );
            Cancel();
        }
        break;

        case SID_PASTE_SPECIAL:
        {
            SetCurrentFunction( FuInsertClipboard::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_CHANGE_PICTURE:
        case SID_INSERT_GRAPHIC:
        {
            SetCurrentFunction( FuInsertGraphic::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq,
                                                         nSId == SID_CHANGE_PICTURE ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_INSERT_AVMEDIA:
        {
            SetCurrentFunction( FuInsertAVMedia::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_INSERT_OBJECT:
        case SID_INSERT_FLOATINGFRAME:
        case SID_INSERT_MATH:
        case SID_INSERT_DIAGRAM:
        case SID_ATTR_TABLE:
        {
            SetCurrentFunction( FuInsertOLE::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            // Set the selection tool as the old one. This in particular important for the
            // zoom function, in which clicking without dragging zooms as well, and that
            // makes exiting the object editing mode impossible.
            if (dynamic_cast<FuSelection*>( GetOldFunction().get() ) == nullptr)
                SetOldFunction( FuSelection::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;
        case SID_CLASSIFICATION_APPLY:
        {
            const SfxItemSet* pArgs = rReq.GetArgs();
            const SfxPoolItem* pItem = nullptr;
            if (pArgs && pArgs->GetItemState(nSId, false, &pItem) == SfxItemState::SET)
            {
                const OUString& rName = static_cast<const SfxStringItem*>(pItem)->GetValue();
                auto eType = SfxClassificationPolicyType::IntellectualProperty;
                if (pArgs->GetItemState(SID_TYPE_NAME, false, &pItem) == SfxItemState::SET)
                {
                    const OUString& rType = static_cast<const SfxStringItem*>(pItem)->GetValue();
                    eType = SfxClassificationHelper::stringToPolicyType(rType);
                }
                if (SfxViewFrame* pViewFrame = GetViewFrame())
                {
                    if (SfxObjectShell* pObjectShell = pViewFrame->GetObjectShell())
                    {
                        SfxClassificationHelper aHelper(pObjectShell->getDocProperties());
                        aHelper.SetBACName(rName, eType);
                    }
                }
            }
            else
                SAL_WARN("sd.ui", "missing parameter for SID_CLASSIFICATION_APPLY");

            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_CLASSIFICATION_DIALOG:
        {
            if (SfxObjectShell* pObjShell = SfxObjectShell::Current())
            {
                css::uno::Reference<css::document::XDocumentProperties> xDocProps(pObjShell->getDocProperties());
                auto xDialog = std::make_shared<svx::ClassificationDialog>(GetFrameWeld(), xDocProps, false, [](){} );
                ClassificationCollector aCollector(*this, xDocProps);
                aCollector.collect();

                xDialog->setupValues(std::vector(aCollector.getResults()));

                if (RET_OK == xDialog->run())
                {
                    ClassificationInserter aInserter(*this, xDocProps);
                    aInserter.insert(xDialog->getResult());
                }
                xDialog.reset();
            }

            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_COPYOBJECTS:
        {
            if ( mpDrawView->IsPresObjSelected(false) )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                if ( mpDrawView->IsTextEdit() )
                {
                    mpDrawView->SdrEndTextEdit();
                }

                SetCurrentFunction( FuCopy::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            }
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_INSERTFILE:  // BASIC
        {
            Broadcast (ViewShellHint(ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_START));
            SetCurrentFunction( FuInsertFile::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            Broadcast (ViewShellHint(ViewShellHint::HINT_COMPLEX_MODEL_CHANGE_END));
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_SELECT_BACKGROUND:
        case SID_SAVE_BACKGROUND:
        case SID_ATTR_PAGE_SIZE:
        case SID_ATTR_PAGE:
        case SID_PAGESETUP:  // BASIC ??
        {
            SetCurrentFunction( FuPage::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore (); // we generate independent macros !!
        }
        break;

        case SID_BEFORE_OBJ:
        case SID_BEHIND_OBJ:
        {
            SetCurrentFunction( FuDisplayOrder::Create(this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq) );
            rReq.Done();
            // finishes itself, no Cancel() needed!
        }
        break;

        case SID_REVERSE_ORDER:   // BASIC
        {
            mpDrawView->ReverseOrderOfMarked();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_ANIMATION_EFFECTS:
        {
            SetCurrentFunction( FuObjectAnimationParameters::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq) );
            Cancel();
        }
        break;

        case SID_EXECUTE_ANIMATION_EFFECT:
        {
            SetCurrentFunction(FuExecuteInteraction::Create(this, GetActiveWindow(),
                                                            mpDrawView.get(), GetDoc(), rReq));
            Cancel();
        }
        break;

        case SID_LINEEND_POLYGON:
        {
            SetCurrentFunction( FuLineEnd::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_CAPTUREPOINT:
            // negative value to signal call from menu
            maMousePos = Point(-1,-1);
            [[fallthrough]];
        case SID_SET_SNAPITEM:
        {
            SetCurrentFunction( FuSnapLine::Create(this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq) );
            Cancel();
        }
        break;

        case SID_MANAGE_LINKS:
        {
            SetCurrentFunction( FuLink::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_THESAURUS:
        {
            SetCurrentFunction( FuThesaurus::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_TEXTATTR_DLG:
        {
            if (mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();
            SetCurrentFunction( FuTextAttrDlg::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_MEASURE_DLG:
        {
            SetCurrentFunction( FuMeasureDlg::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_CONNECTION_DLG:
        {
            SetCurrentFunction( FuConnectionDlg::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            Cancel();
            rReq.Done();
        }
        break;

        case SID_CONNECTION_NEW_ROUTING:
        {
            SfxItemSetFixed<SDRATTR_EDGELINE1DELTA, SDRATTR_EDGELINE3DELTA> aDefAttr( GetPool() );
            GetView()->SetAttributes( aDefAttr, true ); // (ReplaceAll)

            Cancel();
            rReq.Done();
        }
        break;

        case SID_TWAIN_SELECT:
        {
            if( mxScannerManager.is() )
            {
                try
                {
                    const css::uno::Sequence< css::scanner::ScannerContext >
                        aContexts( mxScannerManager->getAvailableScanners() );

                    if( aContexts.hasElements() )
                    {
                        css::scanner::ScannerContext aContext( aContexts.getConstArray()[ 0 ] );

                        Reference<lang::XInitialization> xInit(mxScannerManager, UNO_QUERY);
                        if (xInit.is())
                        {
                            //  initialize dialog
                            weld::Window* pWindow = rReq.GetFrameWeld();
                            uno::Sequence<uno::Any> aSeq(comphelper::InitAnyPropertySequence(
                            {
                                {"ParentWindow", pWindow ? uno::Any(pWindow->GetXWindow()) : uno::Any(Reference<awt::XWindow>())}
                            }));
                            xInit->initialize( aSeq );
                        }

                        mxScannerManager->configureScannerAndScan( aContext, mxScannerListener );
                    }
                }
                catch(...)
                {
                }
            }

            Cancel();
            rReq.Done();
        }
        break;

        case SID_TWAIN_TRANSFER:
        {
            bool bDone = false;

            if( mxScannerManager.is() )
            {
                try
                {
                    const css::uno::Sequence< css::scanner::ScannerContext > aContexts( mxScannerManager->getAvailableScanners() );

                    if( aContexts.hasElements() )
                    {
                        mxScannerManager->startScan( aContexts.getConstArray()[ 0 ], mxScannerListener );
                        bDone = true;
                    }
                }
                catch( ... )
                {
                }
            }

            if( !bDone )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
#ifndef UNX
                                                              SdResId(STR_TWAIN_NO_SOURCE)
#else
                                                              SdResId(STR_TWAIN_NO_SOURCE_UNX)
#endif
                                                              ));
                xInfoBox->run();

            }
            else
            {
                SfxBindings& rBindings = GetViewFrame()->GetBindings();
                rBindings.Invalidate( SID_TWAIN_SELECT );
                rBindings.Invalidate( SID_TWAIN_TRANSFER );
            }

            Cancel();
            rReq.Done();
        }
        break;

        case SID_POLYGON_MORPHING:
        {
            SetCurrentFunction( FuMorph::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_INSERTLAYER:
        {
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
            }

            SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
            sal_uInt16 nLayerCnt = rLayerAdmin.GetLayerCount();
            sal_uInt16 nLayer = nLayerCnt - 2 + 1;
            OUString aLayerName = SdResId(STR_LAYER) + OUString::number(nLayer);
            OUString aLayerTitle, aLayerDesc;
            bool bIsVisible = false;
            bool bIsLocked = false;
            bool bIsPrintable = false;

            const SfxItemSet* pArgs = rReq.GetArgs();

            if (! pArgs)
            {
                SfxItemSetFixed<ATTR_LAYER_START, ATTR_LAYER_END> aNewAttr( GetDoc()->GetPool() );

                aNewAttr.Put( makeSdAttrLayerName( aLayerName ) );
                aNewAttr.Put( makeSdAttrLayerTitle() );
                aNewAttr.Put( makeSdAttrLayerDesc() );
                aNewAttr.Put( makeSdAttrLayerVisible() );
                aNewAttr.Put( makeSdAttrLayerPrintable() );
                aNewAttr.Put( makeSdAttrLayerLocked() );
                aNewAttr.Put( makeSdAttrLayerThisPage() );

                SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
                vcl::Window* pWin = GetActiveWindow();
                ScopedVclPtr<AbstractSdInsertLayerDlg> pDlg( pFact->CreateSdInsertLayerDlg(pWin ? pWin->GetFrameWeld() : nullptr, aNewAttr, true, SdResId(STR_INSERTLAYER)) );
                pDlg->SetHelpId( SD_MOD()->GetSlotPool()->GetSlot( SID_INSERTLAYER )->GetCommand() );

                // test for already existing names
                bool bLoop = true;
                while( bLoop && pDlg->Execute() == RET_OK )
                {
                    pDlg->GetAttr( aNewAttr );
                    aLayerName   = aNewAttr.Get(ATTR_LAYER_NAME).GetValue ();

                    if( rLayerAdmin.GetLayer( aLayerName )
                        || aLayerName.isEmpty()
                        || LayerTabBar::IsLocalizedNameOfStandardLayer( aLayerName) )
                    {
                        // name already exists
                        std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(GetFrameWeld(),
                                                                   VclMessageType::Warning, VclButtonsType::Ok,
                                                                   SdResId(STR_WARN_NAME_DUPLICATE)));
                        xWarn->run();
                    }
                    else
                        bLoop = false;
                }
                if( bLoop ) // was canceled
                {
                    pDlg.disposeAndClear();
                    Cancel();
                    rReq.Ignore ();
                    break;
                }
                else
                {
                    aLayerTitle  = aNewAttr.Get(ATTR_LAYER_TITLE).GetValue();
                    aLayerDesc   = aNewAttr.Get(ATTR_LAYER_DESC).GetValue ();
                    bIsVisible   = static_cast<const SfxBoolItem &>( aNewAttr.Get (ATTR_LAYER_VISIBLE)).GetValue ();
                    bIsLocked    = static_cast<const SfxBoolItem &>( aNewAttr.Get (ATTR_LAYER_LOCKED)).GetValue () ;
                    bIsPrintable = static_cast<const SfxBoolItem &>( aNewAttr.Get (ATTR_LAYER_PRINTABLE)).GetValue () ;
                }
            }
            else if (pArgs->Count () != 4)
            {
#if HAVE_FEATURE_SCRIPTING
                 StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
                 Cancel();
                 rReq.Ignore ();
                 break;
            }
            else
            {
                 const SfxStringItem* pLayerName = rReq.GetArg<SfxStringItem>(ID_VAL_LAYERNAME);
                 const SfxBoolItem* pIsVisible = rReq.GetArg<SfxBoolItem>(ID_VAL_ISVISIBLE);
                 const SfxBoolItem* pIsLocked = rReq.GetArg<SfxBoolItem>(ID_VAL_ISLOCKED);
                 const SfxBoolItem* pIsPrintable = rReq.GetArg<SfxBoolItem>(ID_VAL_ISPRINTABLE);

                 aLayerName   = pLayerName->GetValue ();
                 bIsVisible   = pIsVisible->GetValue ();
                 bIsLocked    = pIsLocked->GetValue ();
                 bIsPrintable = pIsPrintable->GetValue ();
            }

            OUString aPrevLayer = mpDrawView->GetActiveLayer();
            SdrLayer* pLayer;
            sal_uInt16 nPrevLayer = 0;
            nLayerCnt = rLayerAdmin.GetLayerCount();

            for ( nLayer = 0; nLayer < nLayerCnt; nLayer++ )
            {
                pLayer = rLayerAdmin.GetLayer(nLayer);
                OUString aName = pLayer->GetName();

                if ( aPrevLayer == aName )
                {
                    nPrevLayer = std::max(nLayer, sal_uInt16(4));
                }
            }

            mpDrawView->InsertNewLayer(aLayerName, nPrevLayer + 1);
            pLayer = rLayerAdmin.GetLayer(aLayerName);
            if( pLayer )
            {
                pLayer->SetTitle( aLayerTitle );
                pLayer->SetDescription( aLayerDesc );
            }

            mpDrawView->SetLayerVisible( aLayerName, bIsVisible );
            mpDrawView->SetLayerLocked( aLayerName, bIsLocked);
            mpDrawView->SetLayerPrintable(aLayerName, bIsPrintable);

            mpDrawView->SetActiveLayer(aLayerName);

            ResetActualLayer();

            GetDoc()->SetChanged();

            GetViewFrame()->GetDispatcher()->Execute(SID_SWITCHLAYER,
                    SfxCallMode::ASYNCHRON | SfxCallMode::RECORD);

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_MODIFYLAYER:
        {
            if(!GetLayerTabControl()) // #i87182#
            {
                OSL_ENSURE(false, "No LayerTabBar (!)");
                Cancel();
                rReq.Ignore();
                break;
            }

            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
            }

            SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
            sal_uInt16 nCurPage = GetLayerTabControl()->GetCurPageId();
            OUString aLayerName = GetLayerTabControl()->GetLayerName(nCurPage);
            SdrLayer* pLayer = rLayerAdmin.GetLayer(aLayerName);

            OUString aLayerTitle = pLayer->GetTitle();
            OUString aLayerDesc = pLayer->GetDescription();

            OUString aOldLayerName(aLayerName);
            OUString aOldLayerTitle(aLayerTitle);
            OUString aOldLayerDesc(aLayerDesc);

            bool bIsVisible, bIsLocked, bIsPrintable;
            bool bOldIsVisible = bIsVisible = mpDrawView->IsLayerVisible(aLayerName);
            bool bOldIsLocked = bIsLocked = mpDrawView->IsLayerLocked(aLayerName);
            bool bOldIsPrintable = bIsPrintable = mpDrawView->IsLayerPrintable(aLayerName);

            const SfxItemSet* pArgs = rReq.GetArgs();
            // is it allowed to delete the layer?
            bool bDelete = !( LayerTabBar::IsRealNameOfStandardLayer(aLayerName) );

            if (! pArgs)
            {
                SfxItemSetFixed<ATTR_LAYER_START, ATTR_LAYER_END> aNewAttr( GetDoc()->GetPool() );

                aNewAttr.Put( makeSdAttrLayerName( aLayerName ) );
                aNewAttr.Put( makeSdAttrLayerTitle( aLayerTitle ) );
                aNewAttr.Put( makeSdAttrLayerDesc( aLayerDesc ) );
                aNewAttr.Put( makeSdAttrLayerVisible( bIsVisible ) );
                aNewAttr.Put( makeSdAttrLayerLocked( bIsLocked ) );
                aNewAttr.Put( makeSdAttrLayerPrintable( bIsPrintable ) );
                aNewAttr.Put( makeSdAttrLayerThisPage() );

                SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
                vcl::Window* pWin = GetActiveWindow();
                ScopedVclPtr<AbstractSdInsertLayerDlg> pDlg( pFact->CreateSdInsertLayerDlg(pWin ? pWin->GetFrameWeld() : nullptr, aNewAttr, bDelete, SdResId(STR_MODIFYLAYER)) );
                pDlg->SetHelpId( SD_MOD()->GetSlotPool()->GetSlot( SID_MODIFYLAYER )->GetCommand() );

                // test for already existing names
                bool    bLoop = true;
                sal_uInt16  nRet = 0;
                while( bLoop )
                {
                    nRet = pDlg->Execute();
                    if (nRet != RET_OK)
                        break;
                    pDlg->GetAttr( aNewAttr );
                    aLayerName   = aNewAttr.Get(ATTR_LAYER_NAME).GetValue ();
                    if (bDelete)
                    {
                        if( (rLayerAdmin.GetLayer( aLayerName ) && aLayerName != aOldLayerName)
                            || LayerTabBar::IsRealNameOfStandardLayer(aLayerName)
                            || LayerTabBar::IsLocalizedNameOfStandardLayer(aLayerName)
                            || aLayerName.isEmpty() )
                        {
                            // name already exists
                            std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(GetFrameWeld(),
                                                                   VclMessageType::Warning, VclButtonsType::Ok,
                                                                   SdResId(STR_WARN_NAME_DUPLICATE)));
                            xWarn->run();
                        }
                        else
                            bLoop = false;
                    }
                    else
                        bLoop = false; // altering name is already disabled in the dialog itself
                }
                switch (nRet)
                {
                    case RET_OK :
                        aLayerTitle  = aNewAttr.Get(ATTR_LAYER_TITLE).GetValue ();
                        aLayerDesc   = aNewAttr.Get(ATTR_LAYER_DESC).GetValue ();
                        bIsVisible   = static_cast<const SfxBoolItem &>( aNewAttr.Get (ATTR_LAYER_VISIBLE)).GetValue ();
                        bIsLocked    = static_cast<const SfxBoolItem &>( aNewAttr.Get (ATTR_LAYER_LOCKED)).GetValue ();
                        bIsPrintable = static_cast<const SfxBoolItem &>( aNewAttr.Get (ATTR_LAYER_PRINTABLE)).GetValue ();
                        break;

                    default :
                        pDlg.disposeAndClear();
                        rReq.Ignore ();
                        Cancel ();
                        return;
                }
            }
            else if (pArgs->Count () == 4)
            {
                const SfxStringItem* pLayerName = rReq.GetArg<SfxStringItem>(ID_VAL_LAYERNAME);
                const SfxBoolItem* pIsVisible = rReq.GetArg<SfxBoolItem>(ID_VAL_ISVISIBLE);
                const SfxBoolItem* pIsLocked = rReq.GetArg<SfxBoolItem>(ID_VAL_ISLOCKED);
                const SfxBoolItem* pIsPrintable = rReq.GetArg<SfxBoolItem>(ID_VAL_ISPRINTABLE);

                aLayerName   = pLayerName->GetValue ();
                bIsVisible   = pIsVisible->GetValue ();
                bIsLocked    = pIsLocked->GetValue ();
                bIsPrintable = pIsPrintable->GetValue ();
            }
            else
            {
#if HAVE_FEATURE_SCRIPTING
                StarBASIC::FatalError (ERRCODE_BASIC_WRONG_ARGS);
#endif
                Cancel ();
                rReq.Ignore ();
                break;
            }

            SfxUndoManager* pManager = GetDoc()->GetDocSh()->GetUndoManager();
            std::unique_ptr<SdLayerModifyUndoAction> pAction( new SdLayerModifyUndoAction(
                GetDoc(),
                pLayer,
                // old values
                aOldLayerName,
                aOldLayerTitle,
                aOldLayerDesc,
                bOldIsVisible,
                bOldIsLocked,
                bOldIsPrintable,
                // new values
                aLayerName,
                aLayerTitle,
                aLayerDesc,
                bIsVisible,
                bIsLocked,
                bIsPrintable
                ) );
            pManager->AddUndoAction( std::move(pAction) );

            ModifyLayer( pLayer, aLayerName, aLayerTitle, aLayerDesc, bIsVisible, bIsLocked, bIsPrintable );

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_TOGGLELAYERVISIBILITY:
        {
            // tdf#113439; duplicates LayerTabBar::MouseButtonDown()
            sal_uInt16 aTabId = GetLayerTabControl()->GetCurPageId();
            OUString aName( GetLayerTabControl()->GetLayerName(aTabId) );

            SdrPageView* pPV = mpDrawView->GetSdrPageView();
            bool bVisible = !pPV->IsLayerVisible(aName);

            pPV->SetLayerVisible(aName, bVisible);

            ResetActualLayer();
            GetDoc()->SetChanged();

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_RENAMELAYER:
        {
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
            }

            if(GetLayerTabControl()) // #i87182#
            {
                GetLayerTabControl()->StartEditMode(GetLayerTabControl()->GetCurPageId());
            }
            else
            {
                OSL_ENSURE(false, "No LayerTabBar (!)");
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_EDIT_HYPERLINK :
        {
            // Ensure the field is selected first
            OutlinerView* pOutView = mpDrawView->GetTextEditOutlinerView();
            if (pOutView)
                pOutView->SelectFieldAtCursor();

            GetViewFrame()->GetDispatcher()->Execute( SID_HYPERLINK_DIALOG );

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OPEN_HYPERLINK:
        {
            OutlinerView* pOutView = mpDrawView->GetTextEditOutlinerView();
            if ( pOutView )
            {
                const SvxFieldItem* pFieldItem
                    = pOutView->GetFieldAtSelection(/*AlsoCheckBeforeCursor=*/true);
                const SvxFieldData* pField = pFieldItem ? pFieldItem->GetField() : nullptr;
                if( auto pURLField = dynamic_cast< const SvxURLField *>( pField ) )
                {
                    SfxStringItem aUrl( SID_FILE_NAME, pURLField->GetURL() );
                    SfxStringItem aTarget( SID_TARGETNAME, pURLField->GetTargetFrame() );

                    OUString aReferName;
                    SfxViewFrame* pFrame = GetViewFrame();
                    SfxMedium* pMed = pFrame->GetObjectShell()->GetMedium();
                    if (pMed)
                        aReferName = pMed->GetName();

                    SfxFrameItem aFrm( SID_DOCFRAME, pFrame );
                    SfxStringItem aReferer( SID_REFERER, aReferName );

                    SfxBoolItem aNewView( SID_OPEN_NEW_VIEW, false );
                    SfxBoolItem aBrowsing( SID_BROWSE, true );

                    if (SfxViewFrame* pViewFrm = SfxViewFrame::Current())
                    {
                        pViewFrm->GetDispatcher()->ExecuteList(SID_OPENDOC,
                            SfxCallMode::ASYNCHRON | SfxCallMode::RECORD,
                            { &aUrl, &aTarget, &aFrm, &aReferer,
                                &aNewView, &aBrowsing });
                    }
                }
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_COPY_HYPERLINK_LOCATION:
        {
            OutlinerView* pOutView = mpDrawView->GetTextEditOutlinerView();
            if ( pOutView )
            {
                const SvxFieldItem* pFieldItem
                    = pOutView->GetFieldAtSelection(/*AlsoCheckBeforeCursor=*/true);
                const SvxFieldData* pField = pFieldItem ? pFieldItem->GetField() : nullptr;
                if (const SvxURLField* pURLField = dynamic_cast<const SvxURLField*>(pField))
                {
                    uno::Reference<datatransfer::clipboard::XClipboard> xClipboard
                        = pOutView->GetWindow()->GetClipboard();

                    vcl::unohelper::TextDataObject::CopyStringTo(pURLField->GetURL(), xClipboard, SfxViewShell::Current());
                }
            }

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_HYPERLINK_SETLINK:
        {
            const SfxItemSet* pReqArgs = rReq.GetArgs();

            if (pReqArgs)
            {
                const SvxHyperlinkItem* pHLItem =
                    &pReqArgs->Get(SID_HYPERLINK_SETLINK);

                if (pHLItem->GetInsertMode() == HLINK_FIELD)
                {
                    InsertURLField(pHLItem->GetURL(), pHLItem->GetName(),
                                   pHLItem->GetTargetFrame());
                }
                else if (pHLItem->GetInsertMode() == HLINK_BUTTON)
                {
                    InsertURLButton(pHLItem->GetURL(), pHLItem->GetName(),
                                    pHLItem->GetTargetFrame(), nullptr);
                }
                else if (pHLItem->GetInsertMode() == HLINK_DEFAULT)
                {
                    OutlinerView* pOlView = mpDrawView->GetTextEditOutlinerView();

                    if (pOlView || comphelper::LibreOfficeKit::isActive())
                    {
                        InsertURLField(pHLItem->GetURL(), pHLItem->GetName(),
                                       pHLItem->GetTargetFrame());
                    }
                    else
                    {
                        InsertURLButton(pHLItem->GetURL(), pHLItem->GetName(),
                                        pHLItem->GetTargetFrame(), nullptr);
                    }
                }
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_HIDE_LAST_LEVEL:
        {
            ESelection aSel;
            // fdo#78151 editing a PresObjKind::Outline in a master page ?
            ::Outliner* pOL = GetOutlinerForMasterPageOutlineTextObj(aSel);
            if (pOL)
            {
                //we are on the last paragraph
                aSel.Adjust();
                if (aSel.nEndPara == pOL->GetParagraphCount() - 1)
                {
                    sal_uInt16 nDepth = pOL->GetDepth(aSel.nEndPara);
                    //there exists a previous numbering level
                    if (nDepth != sal_uInt16(-1) && nDepth > 0)
                    {
                        Paragraph* pPara = pOL->GetParagraph(aSel.nEndPara);
                        pOL->Remove(pPara, 1);
                    }
                }
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_SHOW_NEXT_LEVEL:
        {
            const TranslateId STR_PRESOBJ_MPOUTLINE_ARY[]
            {
                STR_PRESOBJ_MPOUTLINE,
                STR_PRESOBJ_MPOUTLLAYER2,
                STR_PRESOBJ_MPOUTLLAYER3,
                STR_PRESOBJ_MPOUTLLAYER4,
                STR_PRESOBJ_MPOUTLLAYER5,
                STR_PRESOBJ_MPOUTLLAYER6,
                STR_PRESOBJ_MPOUTLLAYER7,
                STR_PRESOBJ_MPNOTESTITLE,
                STR_PRESOBJ_MPNOTESTEXT,
                STR_PRESOBJ_NOTESTEXT
            };

            ESelection aSel;
            // fdo#78151 editing a PresObjKind::Outline in a master page ?
            ::Outliner* pOL = GetOutlinerForMasterPageOutlineTextObj(aSel);
            if (pOL)
            {
                //we are on the last paragraph
                aSel.Adjust();
                if (aSel.nEndPara == pOL->GetParagraphCount() - 1)
                {
                    sal_uInt16 nDepth = pOL->GetDepth(aSel.nEndPara);
                    //there exists a previous numbering level
                    if (nDepth < 8)
                    {
                        sal_uInt16 nNewDepth = nDepth+1;
                        pOL->Insert(SdResId(STR_PRESOBJ_MPOUTLINE_ARY[nNewDepth]), EE_PARA_APPEND, nNewDepth);
                    }
                }
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_INSERT_FLD_DATE_FIX:
        case SID_INSERT_FLD_DATE_VAR:
        case SID_INSERT_FLD_TIME_FIX:
        case SID_INSERT_FLD_TIME_VAR:
        case SID_INSERT_FLD_AUTHOR:
        case SID_INSERT_FLD_PAGE:
        case SID_INSERT_FLD_PAGE_TITLE:
        case SID_INSERT_FLD_PAGES:
        case SID_INSERT_FLD_FILE:
        {
            sal_uInt16 nMul = 1;
            std::unique_ptr<SvxFieldItem> pFieldItem;

            switch( nSId )
            {
                case SID_INSERT_FLD_DATE_FIX:
                    pFieldItem.reset(new SvxFieldItem(
                        SvxDateField( Date( Date::SYSTEM ), SvxDateType::Fix ), EE_FEATURE_FIELD ));
                break;

                case SID_INSERT_FLD_DATE_VAR:
                    pFieldItem.reset(new SvxFieldItem( SvxDateField(), EE_FEATURE_FIELD ));
                break;

                case SID_INSERT_FLD_TIME_FIX:
                    pFieldItem.reset(new SvxFieldItem(
                        SvxExtTimeField( ::tools::Time( ::tools::Time::SYSTEM ), SvxTimeType::Fix ), EE_FEATURE_FIELD ));
                break;

                case SID_INSERT_FLD_TIME_VAR:
                    pFieldItem.reset(new SvxFieldItem( SvxExtTimeField(), EE_FEATURE_FIELD ));
                break;

                case SID_INSERT_FLD_AUTHOR:
                {
                    SvtUserOptions aUserOptions;
                    pFieldItem.reset(new SvxFieldItem(
                            SvxAuthorField(
                                aUserOptions.GetFirstName(), aUserOptions.GetLastName(), aUserOptions.GetID() ), EE_FEATURE_FIELD ));
                }
                break;

                case SID_INSERT_FLD_PAGE:
                {
                    pFieldItem.reset(new SvxFieldItem( SvxPageField(), EE_FEATURE_FIELD ));
                    nMul = 3;
                }
                break;

                case SID_INSERT_FLD_PAGE_TITLE:
                {
                    pFieldItem.reset(new SvxFieldItem( SvxPageTitleField(), EE_FEATURE_FIELD));
                    nMul = 3;
                }
                break;

                case SID_INSERT_FLD_PAGES:
                {
                    pFieldItem.reset(new SvxFieldItem( SvxPagesField(), EE_FEATURE_FIELD ));
                    nMul = 3;
                }
                break;

                case SID_INSERT_FLD_FILE:
                {
                    OUString aName;
                    if( GetDocSh()->HasName() )
                        aName = GetDocSh()->GetMedium()->GetName();
                    pFieldItem.reset(new SvxFieldItem( SvxExtFileField( aName ), EE_FEATURE_FIELD ));
                }
                break;
            }

            OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();

            if( pOLV )
            {
                const SvxFieldItem* pOldFldItem = pOLV->GetFieldAtSelection();

                if( pOldFldItem && ( nullptr != dynamic_cast< const SvxURLField *>( pOldFldItem->GetField() ) ||
                                    nullptr != dynamic_cast< const SvxDateField *>( pOldFldItem->GetField() ) ||
                                    nullptr != dynamic_cast< const SvxTimeField *>( pOldFldItem->GetField() ) ||
                                    nullptr != dynamic_cast< const SvxExtTimeField *>( pOldFldItem->GetField() ) ||
                                    nullptr != dynamic_cast< const SvxExtFileField *>( pOldFldItem->GetField() ) ||
                                    nullptr != dynamic_cast< const SvxAuthorField *>( pOldFldItem->GetField() ) ||
                                    nullptr != dynamic_cast< const SvxPageField *>( pOldFldItem->GetField() ) ) )
                {
                    // select field, then it will be deleted when inserting
                    ESelection aSel = pOLV->GetSelection();
                    if( aSel.nStartPos == aSel.nEndPos )
                        aSel.nEndPos++;
                    pOLV->SetSelection( aSel );
                }

                if( pFieldItem )
                    pOLV->InsertField( *pFieldItem );
            }
            else
            {
                Outliner* pOutl = GetDoc()->GetInternalOutliner();
                pOutl->Init( OutlinerMode::TextObject );
                OutlinerMode nOutlMode = pOutl->GetOutlinerMode();
                pOutl->SetStyleSheet( 0, nullptr );
                pOutl->QuickInsertField( *pFieldItem, ESelection() );
                std::optional<OutlinerParaObject> pOutlParaObject = pOutl->CreateParaObject();

                rtl::Reference<SdrRectObj> pRectObj = new SdrRectObj(
                    *GetDoc(),
                    SdrObjKind::Text);
                pRectObj->SetMergedItem(makeSdrTextAutoGrowWidthItem(true));

                pOutl->UpdateFields();
                pOutl->SetUpdateLayout( true );
                Size aSize( pOutl->CalcTextSize() );
                aSize.setWidth( aSize.Width() * nMul );
                pOutl->SetUpdateLayout( false );

                Point aPos;
                ::tools::Rectangle aRect( aPos, GetActiveWindow()->GetOutputSizePixel() );
                aPos = aRect.Center();
                aPos = GetActiveWindow()->PixelToLogic(aPos);
                aPos.AdjustX( -(aSize.Width() / 2) );
                aPos.AdjustY( -(aSize.Height() / 2) );

                ::tools::Rectangle aLogicRect(aPos, aSize);
                pRectObj->SetLogicRect(aLogicRect);
                pRectObj->SetOutlinerParaObject( std::move(pOutlParaObject) );
                mpDrawView->InsertObjectAtView(pRectObj.get(), *mpDrawView->GetSdrPageView());
                pOutl->Init( nOutlMode );
            }

            pFieldItem.reset();

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_MODIFY_FIELD:
        {
            OutlinerView* pOLV = mpDrawView->GetTextEditOutlinerView();

            if( pOLV )
            {
                const SvxFieldItem* pFldItem = pOLV->GetFieldAtSelection();

                if( pFldItem && (nullptr != dynamic_cast< const SvxDateField *>( pFldItem->GetField() ) ||
                                 nullptr != dynamic_cast< const SvxAuthorField *>( pFldItem->GetField() ) ||
                                 nullptr != dynamic_cast< const SvxExtFileField *>( pFldItem->GetField() ) ||
                                 nullptr != dynamic_cast< const SvxExtTimeField *>( pFldItem->GetField() ) ) )
                {
                    // Dialog...
                    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
                    vcl::Window* pWin = GetActiveWindow();
                    ScopedVclPtr<AbstractSdModifyFieldDlg> pDlg( pFact->CreateSdModifyFieldDlg(pWin ? pWin->GetFrameWeld() : nullptr, pFldItem->GetField(), pOLV->GetAttribs() ) );
                    if( pDlg->Execute() == RET_OK )
                    {
                        // To make a correct SetAttribs() call at the utlinerView
                        // it is necessary to split the actions here
                        std::unique_ptr<SvxFieldData> pField(pDlg->GetField());
                        ESelection aSel = pOLV->GetSelection();
                        bool bSelectionWasModified(false);

                        if( pField )
                        {
                            SvxFieldItem aFieldItem( *pField, EE_FEATURE_FIELD );

                            if( aSel.nStartPos == aSel.nEndPos )
                            {
                                bSelectionWasModified = true;
                                aSel.nEndPos++;
                                pOLV->SetSelection( aSel );
                            }

                            pOLV->InsertField( aFieldItem );

                            // select again for eventual SetAttribs call
                            pOLV->SetSelection( aSel );
                        }

                        SfxItemSet aSet( pDlg->GetItemSet() );

                        if( aSet.Count() )
                        {
                            pOLV->SetAttribs( aSet );

                            ::Outliner* pOutliner = pOLV->GetOutliner();
                            if( pOutliner )
                                pOutliner->UpdateFields();
                        }

                        if(pField)
                        {
                            // restore selection to original
                            if(bSelectionWasModified)
                            {
                                aSel.nEndPos--;
                                pOLV->SetSelection( aSel );
                            }
                        }
                    }
                }
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_OPEN_XML_FILTERSETTINGS:
        {
            try
            {
                css::uno::Reference < css::ui::dialogs::XExecutableDialog > xDialog = css::ui::dialogs::XSLTFilterDialog::create( ::comphelper::getProcessComponentContext() );
                xDialog->execute();
            }
            catch( css::uno::RuntimeException& )
            {
                DBG_UNHANDLED_EXCEPTION("sd.view");
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_GROUP:  // BASIC
        {
            if ( mpDrawView->IsPresObjSelected( true, true, true ) )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                mpDrawView->GroupMarked();
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_UNGROUP:  // BASIC
        {
            mpDrawView->UnGroupMarked();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_NAME_GROUP:
        {
            // only allow for single object selection since the name of an object needs
            // to be unique
            if(1 == mpDrawView->GetMarkedObjectList().GetMarkCount())
            {
                // #i68101#
                rtl::Reference<SdrObject> pSelected = mpDrawView->GetMarkedObjectByIndex(0);
                OSL_ENSURE(pSelected, "DrawViewShell::FuTemp03: nMarkCount, but no object (!)");
                OUString aName(pSelected->GetName());

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                VclPtr<AbstractSvxObjectNameDialog> pDlg(pFact->CreateSvxObjectNameDialog(GetFrameWeld(), aName));

                pDlg->SetCheckNameHdl(LINK(this, DrawViewShell, NameObjectHdl));

                pDlg->StartExecuteAsync(
                    [this, pDlg, pSelected] (sal_Int32 nResult)->void
                    {
                        if (nResult == RET_OK)
                        {
                            pSelected->SetName(pDlg->GetName());

                            SdPage* pPage = GetActualPage();
                            if (pPage)
                                pPage->notifyObjectRenamed(pSelected.get());
                        }
                        pDlg->disposeOnce();
                        SfxBindings& rBindings = GetViewFrame()->GetBindings();
                        rBindings.Invalidate( SID_NAVIGATOR_STATE, true );
                        rBindings.Invalidate( SID_CONTEXT );
                    }
                );
            }

            Cancel();
            rReq.Ignore();
            break;
        }

        // #i68101#
        case SID_OBJECT_TITLE_DESCRIPTION:
        {
            if(1 == mpDrawView->GetMarkedObjectList().GetMarkCount())
            {
                rtl::Reference<SdrObject> pSelected = mpDrawView->GetMarkedObjectByIndex(0);
                OSL_ENSURE(pSelected, "DrawViewShell::FuTemp03: nMarkCount, but no object (!)");
                OUString aTitle(pSelected->GetTitle());
                OUString aDescription(pSelected->GetDescription());
                bool isDecorative(pSelected->IsDecorative());

                SvxAbstractDialogFactory* pFact = SvxAbstractDialogFactory::Create();
                VclPtr<AbstractSvxObjectTitleDescDialog> pDlg(pFact->CreateSvxObjectTitleDescDialog(
                            GetFrameWeld(), aTitle, aDescription, isDecorative));

                pDlg->StartExecuteAsync(
                    [this, pDlg, pSelected] (sal_Int32 nResult)->void
                    {
                        if (nResult == RET_OK)
                        {
                            pSelected->SetTitle(pDlg->GetTitle());
                            pSelected->SetDescription(pDlg->GetDescription());
                            pSelected->SetDecorative(pDlg->IsDecorative());
                        }
                        pDlg->disposeOnce();
                        SfxBindings& rBindings = GetViewFrame()->GetBindings();
                        rBindings.Invalidate( SID_NAVIGATOR_STATE, true );
                        rBindings.Invalidate( SID_CONTEXT );
                    }
                );
            }

            Cancel();
            rReq.Ignore();
            break;
        }

        case SID_SETLAYER:
        {
            const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
            const size_t nMarkCount = rMarkList.GetMarkCount();
            if (nMarkCount >= 1 && mpLayerTabBar)
            {
                SdSelectLayerDlg aDlg(GetFrameWeld());

                weld::TreeView& rTreeView = aDlg.GetTreeView();
                auto nPageCount = mpLayerTabBar->GetPageCount();
                for (auto i = 0; i < nPageCount; i++)
                    rTreeView.append_text(LayerTabBar::convertToLocalizedName(
                                        mpLayerTabBar->GetLayerName(mpLayerTabBar->GetPageId(i))));
                rTreeView.select(0);

                if (aDlg.run() == RET_OK && rTreeView.get_selected_index() != -1)
                {
                    SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
                    SdrLayerID aSdrLayerId = rLayerAdmin.GetLayerID(mpLayerTabBar->GetLayerName(
                                        mpLayerTabBar->GetPageId(rTreeView.get_selected_index())));
                    for (size_t i = 0; i < nMarkCount; ++i)
                    {
                        SdrObject* pObj = rMarkList.GetMark(i)->GetMarkedSdrObj();
                        pObj->SetLayer(aSdrLayerId);
                    }
                }
            }
            Cancel();
            rReq.Ignore();
            break;
        }

        case SID_ENTER_GROUP:  // BASIC
        {
            mpDrawView->EnterMarkedGroup();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_LEAVE_GROUP:  // BASIC
        {
            mpDrawView->LeaveOneGroup();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_LEAVE_ALL_GROUPS:  // BASIC
        {
            mpDrawView->LeaveAllGroup();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_TEXT_COMBINE:  // BASIC
        {
            // End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->CombineMarkedTextObjects();
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_COMBINE:  // BASIC
        {
            // End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->CombineMarkedObjects(false);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_DISTRIBUTE_HLEFT:
        case SID_DISTRIBUTE_HCENTER:
        case SID_DISTRIBUTE_HDISTANCE:
        case SID_DISTRIBUTE_HRIGHT:
        case SID_DISTRIBUTE_VTOP:
        case SID_DISTRIBUTE_VCENTER:
        case SID_DISTRIBUTE_VDISTANCE:
        case SID_DISTRIBUTE_VBOTTOM:
        {
            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                mpDrawView->DistributeMarkedObjects(nSId);
            }
            Cancel();
            rReq.Done ();
        }
        break;
        case SID_POLY_MERGE:
        {
            // End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->MergeMarkedObjects(SdrMergeMode::Merge);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_POLY_SUBSTRACT:
        {
            // End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->MergeMarkedObjects(SdrMergeMode::Subtract);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_POLY_INTERSECT:
        {
            // End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->MergeMarkedObjects(SdrMergeMode::Intersect);
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_EQUALIZEWIDTH:
        case SID_EQUALIZEHEIGHT:
        {
            // End text edit to avoid conflicts
            if(mpDrawView->IsTextEdit())
                mpDrawView->SdrEndTextEdit();

            mpDrawView->EqualizeMarkedObjects(nSId == SID_EQUALIZEWIDTH);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_DISMANTLE:  // BASIC
        {
            if ( mpDrawView->IsDismantlePossible() )
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->DismantleMarkedObjects();
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_CONNECT:  // BASIC
        {
            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->CombineMarkedObjects();
            }
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_BREAK:  // BASIC
        {
            if ( mpDrawView->IsTextEdit() )
            {
                mpDrawView->SdrEndTextEdit();
            }

            if ( mpDrawView->IsBreak3DObjPossible() )
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->Break3DObj();
            }
            else if ( mpDrawView->IsDismantlePossible(true) )
            {
                weld::WaitObject aWait(GetFrameWeld());
                mpDrawView->DismantleMarkedObjects(true);
            }
            else if ( mpDrawView->IsImportMtfPossible() )
            {
                weld::WaitObject aWait(GetFrameWeld());
                const SdrMarkList& rMarkList = mpDrawView->GetMarkedObjectList();
                const size_t nCnt=rMarkList.GetMarkCount();

                // determine the sum of meta objects of all selected meta files
                sal_uLong nCount = 0;
                for(size_t nm=0; nm<nCnt; ++nm)
                {
                    SdrMark*     pM=rMarkList.GetMark(nm);
                    SdrObject*   pObj=pM->GetMarkedSdrObj();
                    SdrGrafObj*  pGraf= dynamic_cast< SdrGrafObj *>( pObj );
                    SdrOle2Obj*  pOle2= dynamic_cast< SdrOle2Obj *>( pObj );

                    if (pGraf != nullptr)
                    {
                        if (pGraf->HasGDIMetaFile())
                        {
                            nCount += pGraf->GetGraphic().GetGDIMetaFile().GetActionSize();
                        }
                        else if (pGraf->isEmbeddedVectorGraphicData())
                        {
                            nCount += pGraf->getMetafileFromEmbeddedVectorGraphicData().GetActionSize();
                        }
                    }

                    if(pOle2 && pOle2->GetGraphic())
                    {
                         nCount += pOle2->GetGraphic()->GetGDIMetaFile().GetActionSize();
                    }
                }

                // decide with the sum of all meta objects if we should show a dialog
                if(nCount < MIN_ACTIONS_FOR_DIALOG)
                {
                    // nope, no dialog
                    mpDrawView->DoImportMarkedMtf();
                }
                else
                {
                    SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
                    ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateBreakDlg(GetFrameWeld(), mpDrawView.get(), GetDocSh(), nCount, static_cast<sal_uLong>(nCnt) ));
                    pDlg->Execute();
                }
            }

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_CONVERT_TO_3D:
        {
            if ( mpDrawView->IsPresObjSelected() )
            {
                std::unique_ptr<weld::MessageDialog> xInfoBox(Application::CreateMessageDialog(GetFrameWeld(),
                                                              VclMessageType::Info, VclButtonsType::Ok,
                                                              SdResId(STR_ACTION_NOTPOSSIBLE)));
                xInfoBox->run();
            }
            else
            {
                if (mpDrawView->IsConvertTo3DObjPossible())
                {
                    if (mpDrawView->IsTextEdit())
                    {
                        mpDrawView->SdrEndTextEdit();
                    }

                    weld::WaitObject aWait(GetFrameWeld());
                    mpDrawView->ConvertMarkedObjTo3D();
                }
            }

            Cancel();
            rReq.Done();
        }
        break;

        case SID_FRAME_TO_TOP:  // BASIC
        {
            mpDrawView->PutMarkedToTop();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_MOREFRONT:  // BASIC
        case SID_FRAME_UP:  // BASIC
        {
            mpDrawView->MovMarkedToTop();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_MOREBACK:  // BASIC
        case SID_FRAME_DOWN:  // BASIC
        {
            mpDrawView->MovMarkedToBtm();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_FRAME_TO_BOTTOM:   // BASIC
        {
            mpDrawView->PutMarkedToBtm();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_HORIZONTAL:  // BASIC
        case SID_FLIP_HORIZONTAL:
        {
            mpDrawView->MirrorAllMarkedHorizontal();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_VERTICAL:  // BASIC
        case SID_FLIP_VERTICAL:
        {
            mpDrawView->MirrorAllMarkedVertical();
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_LEFT:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SdrHorAlign::Left, SdrVertAlign::NONE);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_CENTER:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SdrHorAlign::Center, SdrVertAlign::NONE);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_RIGHT:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SdrHorAlign::Right, SdrVertAlign::NONE);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_UP:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SdrHorAlign::NONE, SdrVertAlign::Top);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_MIDDLE:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SdrHorAlign::NONE, SdrVertAlign::Center);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_OBJECT_ALIGN_DOWN:  // BASIC
        {
            mpDrawView->AlignMarkedObjects(SdrHorAlign::NONE, SdrVertAlign::Bottom);
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_SELECTALL:  // BASIC
        {
            if( (dynamic_cast<FuSelection*>( GetOldFunction().get() ) != nullptr) &&
                !GetView()->IsFrameDragSingles() && GetView()->HasMarkablePoints())
            {
                if ( !mpDrawView->IsAction() )
                    mpDrawView->MarkAllPoints();
            }
            else
                mpDrawView->SelectAll();

            FreshNavigatrTree();

            Cancel();
            rReq.Done ();
        }
        break;

        case SID_STYLE_NEW: // BASIC ???
        case SID_STYLE_APPLY:
        case SID_STYLE_EDIT:
        case SID_STYLE_DELETE:
        case SID_STYLE_HIDE:
        case SID_STYLE_SHOW:
        case SID_STYLE_FAMILY:
        case SID_STYLE_WATERCAN:
        case SID_STYLE_UPDATE_BY_EXAMPLE:
        case SID_STYLE_NEW_BY_EXAMPLE:
        {
            if (!rReq.GetArgs()
                && (nSId == SID_STYLE_EDIT || nSId == SID_STYLE_UPDATE_BY_EXAMPLE
                     || nSId == SID_STYLE_NEW_BY_EXAMPLE))
            {
                SfxStyleSheet* pStyleSheet = mpDrawView->GetStyleSheet();
                if( pStyleSheet && pStyleSheet->GetFamily() == SfxStyleFamily::Page)
                    pStyleSheet = static_cast<SdStyleSheet*>(pStyleSheet)->GetPseudoStyleSheet();

                if( (pStyleSheet == nullptr) && GetView()->IsTextEdit() )
                {
                    GetView()->SdrEndTextEdit();

                    pStyleSheet = mpDrawView->GetStyleSheet();
                    if(pStyleSheet && pStyleSheet->GetFamily() == SfxStyleFamily::Page)
                        pStyleSheet = static_cast<SdStyleSheet*>(pStyleSheet)->GetPseudoStyleSheet();
                }

                if( pStyleSheet == nullptr )
                {
                    rReq.Ignore();
                    break;
                }

                SfxAllItemSet aSet(GetDoc()->GetPool());

                aSet.Put(SfxUInt16Item(SID_STYLE_FAMILY,
                                       static_cast<sal_uInt16>(pStyleSheet->GetFamily())));

                if (nSId == SID_STYLE_NEW_BY_EXAMPLE)
                {
                    weld::Window* pWindow = GetViewFrame()->GetFrameWeld();
                    SfxNewStyleDlg aDlg(pWindow, *pStyleSheet->GetPool(), pStyleSheet->GetFamily());
                    auto nResult = aDlg.run();
                    if (nResult == RET_OK)
                    {
                        aSet.Put(SfxStringItem(SID_STYLE_NEW_BY_EXAMPLE, aDlg.GetName()));
                        aSet.Put(SfxStringItem(SID_STYLE_REFERENCE, pStyleSheet->GetName()));
                    }
                    else
                    {
                        Cancel();
                        rReq.Ignore();
                        break;
                    }
                }
                else
                    aSet.Put(SfxStringItem(nSId, pStyleSheet->GetName()));

                rReq.SetArgs(aSet);
            }

            if( rReq.GetArgs() )
            {
                SetCurrentFunction( FuTemplate::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
                if( rReq.GetSlot() == SID_STYLE_APPLY )
                    GetViewFrame()->GetBindings().Invalidate( SID_STYLE_APPLY );
                Cancel();
            }
            else if( rReq.GetSlot() == SID_STYLE_APPLY )
                GetViewFrame()->GetDispatcher()->Execute( SID_STYLE_DESIGNER, SfxCallMode::ASYNCHRON );
            rReq.Ignore ();
        }
        break;

        case SID_IMAP:
        {
            sal_uInt16      nId = SvxIMapDlgChildWindow::GetChildWindowId();

            GetViewFrame()->ToggleChildWindow( nId );
            GetViewFrame()->GetBindings().Invalidate( SID_IMAP );

            if ( GetViewFrame()->HasChildWindow( nId )
                && ( ( ViewShell::Implementation::GetImageMapDialog() ) != nullptr ) )
            {
                const SdrMarkList&  rMarkList = mpDrawView->GetMarkedObjectList();

                if ( rMarkList.GetMarkCount() == 1 )
                    UpdateIMapDlg( rMarkList.GetMark( 0 )->GetMarkedSdrObj() );
            }

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_GRID_FRONT:
        {
            mpDrawView->SetGridFront( !mpDrawView->IsGridFront() );
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_HELPLINES_FRONT:
        {
            mpDrawView->SetHlplFront( !mpDrawView->IsHlplFront() );
            Cancel();
            rReq.Done ();
        }
        break;

        case SID_FONTWORK:
        {
            if ( rReq.GetArgs() )
            {
                GetViewFrame()->SetChildWindow(SvxFontWorkChildWindow::GetChildWindowId(),
                                        static_cast<const SfxBoolItem&>(rReq.GetArgs()->
                                        Get(SID_FONTWORK)).GetValue());
            }
            else
            {
                GetViewFrame()->ToggleChildWindow( SvxFontWorkChildWindow::GetChildWindowId() );
            }

            GetViewFrame()->GetBindings().Invalidate(SID_FONTWORK);
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_COLOR_CONTROL:
        {
            if ( rReq.GetArgs() )
                GetViewFrame()->SetChildWindow(SvxColorChildWindow::GetChildWindowId(),
                                        rReq.GetArgs()->Get(SID_COLOR_CONTROL).GetValue());
            else
                GetViewFrame()->ToggleChildWindow(SvxColorChildWindow::GetChildWindowId() );

            GetViewFrame()->GetBindings().Invalidate(SID_COLOR_CONTROL);
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_EXTRUSION_TOGGLE:
        case SID_EXTRUSION_TILT_DOWN:
        case SID_EXTRUSION_TILT_UP:
        case SID_EXTRUSION_TILT_LEFT:
        case SID_EXTRUSION_TILT_RIGHT:
        case SID_EXTRUSION_3D_COLOR:
        case SID_EXTRUSION_DEPTH:
        case SID_EXTRUSION_DIRECTION:
        case SID_EXTRUSION_PROJECTION:
        case SID_EXTRUSION_LIGHTING_DIRECTION:
        case SID_EXTRUSION_LIGHTING_INTENSITY:
        case SID_EXTRUSION_SURFACE:
        case SID_EXTRUSION_DEPTH_FLOATER:
        case SID_EXTRUSION_DIRECTION_FLOATER:
        case SID_EXTRUSION_LIGHTING_FLOATER:
        case SID_EXTRUSION_SURFACE_FLOATER:
        case SID_EXTRUSION_DEPTH_DIALOG:
            svx::ExtrusionBar::execute( mpDrawView.get(), rReq, GetViewFrame()->GetBindings() );
            Cancel();
            rReq.Ignore ();
            break;

        case SID_FONTWORK_SHAPE:
        case SID_FONTWORK_SHAPE_TYPE:
        case SID_FONTWORK_ALIGNMENT:
        case SID_FONTWORK_SAME_LETTER_HEIGHTS:
        case SID_FONTWORK_CHARACTER_SPACING:
        case SID_FONTWORK_KERN_CHARACTER_PAIRS:
        case SID_FONTWORK_GALLERY_FLOATER:
        case SID_FONTWORK_CHARACTER_SPACING_FLOATER:
        case SID_FONTWORK_ALIGNMENT_FLOATER:
        case SID_FONTWORK_CHARACTER_SPACING_DIALOG:
            svx::FontworkBar::execute(*mpDrawView, rReq, GetViewFrame()->GetBindings());
            Cancel();
            rReq.Ignore ();
            break;

        case SID_BMPMASK:
        {
            GetViewFrame()->ToggleChildWindow( SvxBmpMaskChildWindow::GetChildWindowId() );
            GetViewFrame()->GetBindings().Invalidate( SID_BMPMASK );

            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_NAVIGATOR:
        {
            if (comphelper::LibreOfficeKit::isActive())
            {
                GetViewFrame()->ShowChildWindow(SID_SIDEBAR);
                OUString panelId = u"SdNavigatorPanel"_ustr;
                ::sfx2::sidebar::Sidebar::TogglePanel(
                    panelId, GetViewFrame()->GetFrame().GetFrameInterface());

                Cancel();
                rReq.Done();
            } else {
                if ( rReq.GetArgs() )
                    GetViewFrame()->SetChildWindow(SID_NAVIGATOR,
                                            static_cast<const SfxBoolItem&>(rReq.GetArgs()->
                                            Get(SID_NAVIGATOR)).GetValue());
                else
                    GetViewFrame()->ToggleChildWindow( SID_NAVIGATOR );

                GetViewFrame()->GetBindings().Invalidate(SID_NAVIGATOR);
                Cancel();
                rReq.Ignore ();
            }
        }
        break;

        case SID_SLIDE_TRANSITIONS_PANEL:
        case SID_MASTER_SLIDES_PANEL:
        case SID_CUSTOM_ANIMATION_PANEL:
        case SID_GALLERY:
        {
            // First make sure that the sidebar is visible
            GetViewFrame()->ShowChildWindow(SID_SIDEBAR);

            OUString panelId;
            if (nSId == SID_CUSTOM_ANIMATION_PANEL)
                panelId = "SdCustomAnimationPanel";
            else if (nSId == SID_GALLERY)
                panelId = "GalleryPanel";
            else if (nSId == SID_SLIDE_TRANSITIONS_PANEL)
                panelId = "SdSlideTransitionPanel";
            else if (nSId == SID_MASTER_SLIDES_PANEL)
                panelId = "SdAllMasterPagesPanel";

            ::sfx2::sidebar::Sidebar::TogglePanel(
                panelId,
                GetViewFrame()->GetFrame().GetFrameInterface());

            Cancel();
            rReq.Done();
        }
        break;

        case SID_ANIMATION_OBJECTS:
        {
            if ( rReq.GetArgs() )
                GetViewFrame()->SetChildWindow(
                    AnimationChildWindow::GetChildWindowId(),
                    static_cast<const SfxBoolItem&>(rReq.GetArgs()->
                        Get(SID_ANIMATION_OBJECTS)).GetValue());
            else
                GetViewFrame()->ToggleChildWindow(
                    AnimationChildWindow::GetChildWindowId() );

            GetViewFrame()->GetBindings().Invalidate(SID_ANIMATION_OBJECTS);
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_3D_WIN:
        {
            if ( rReq.GetArgs() )
                GetViewFrame()->SetChildWindow( Svx3DChildWindow::GetChildWindowId(),
                                        static_cast<const SfxBoolItem&>(rReq.GetArgs()->
                                        Get( SID_3D_WIN )).GetValue());
            else
                GetViewFrame()->ToggleChildWindow( Svx3DChildWindow::GetChildWindowId() );

            GetViewFrame()->GetBindings().Invalidate( SID_3D_WIN );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_CONVERT_TO_3D_LATHE_FAST:
        {
            /* The call is enough. The initialization via Start3DCreation and
               CreateMirrorPolygons is no longer needed if the parameter
               sal_True is provided. Then a tilted rotary body with an axis left
               besides the bounding rectangle of the selected objects is drawn
               immediately and without user interaction.  */
            mpDrawView->SdrEndTextEdit();
            if(GetActiveWindow())
                GetActiveWindow()->EnterWait();
            mpDrawView->End3DCreation(true);
            Cancel();
            rReq.Ignore();
            if(GetActiveWindow())
                GetActiveWindow()->LeaveWait();
        }
        break;

        case SID_PRESENTATION_DLG:
        {
            SetCurrentFunction( FuSlideShowDlg::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_REMOTE_DLG:
        {
#ifdef ENABLE_SDREMOTE
             SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
             ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateRemoteDialog(GetFrameWeld()));
             pDlg->Execute();
#endif
        }
        break;

        case SID_CUSTOMSHOW_DLG:
        {
            SetCurrentFunction( FuCustomShowDlg::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_EXPAND_PAGE:
        {
            SetCurrentFunction( FuExpandPage::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            Cancel();
        }
        break;

        case SID_SUMMARY_PAGE:
        {
            mpDrawView->SdrEndTextEdit();
            SetCurrentFunction( FuSummaryPage::Create( this, GetActiveWindow(), mpDrawView.get(), GetDoc(), rReq ) );
            Cancel();
        }
        break;

#if HAVE_FEATURE_AVMEDIA
        case SID_AVMEDIA_PLAYER:
        {
            GetViewFrame()->ToggleChildWindow( ::avmedia::MediaPlayer::GetChildWindowId() );
            GetViewFrame()->GetBindings().Invalidate( SID_AVMEDIA_PLAYER );
            Cancel();
            rReq.Ignore ();
        }
        break;
#endif

        case SID_PRESENTATION_MINIMIZER:
        {
            Reference<XComponentContext> xContext(::comphelper::getProcessComponentContext());
            Reference<util::XURLTransformer> xParser(util::URLTransformer::create(xContext));
            Reference<frame::XDispatchProvider> xProvider(GetViewShellBase().GetController()->getFrame(), UNO_QUERY);
            if (xProvider.is())
            {
                util::URL aURL;
                aURL.Complete = "vnd.com.sun.star.comp.PresentationMinimizer:execute";
                xParser->parseStrict(aURL);
                uno::Reference<frame::XDispatch> xDispatch(xProvider->queryDispatch(aURL, OUString(), 0));
                if (xDispatch.is())
                {
                    xDispatch->dispatch(aURL, uno::Sequence< beans::PropertyValue >());
                }
            }
            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_DISPLAY_MASTER_BACKGROUND:
        case SID_DISPLAY_MASTER_OBJECTS:
        {
            // Determine current page and toggle visibility of layers
            // associated with master page background or master page shapes.
            // FIXME: This solution is wrong, because shapes of master pages need
            // not be on layer "background" or "backgroundobjects".
            // See tdf#118613
            SdPage* pPage = GetActualPage();
            if (pPage != nullptr
                && GetDoc() != nullptr)
            {
                SdrLayerIDSet aVisibleLayers = pPage->TRG_GetMasterPageVisibleLayers();
                SdrLayerAdmin& rLayerAdmin = GetDoc()->GetLayerAdmin();
                SdrLayerID aLayerId;
                if (nSId == SID_DISPLAY_MASTER_BACKGROUND)
                    aLayerId = rLayerAdmin.GetLayerID(sUNO_LayerName_background);
                else
                    aLayerId = rLayerAdmin.GetLayerID(sUNO_LayerName_background_objects);
                aVisibleLayers.Set(aLayerId, !aVisibleLayers.IsSet(aLayerId));
                pPage->TRG_SetMasterPageVisibleLayers(aVisibleLayers);
            }
            Cancel();
            rReq.Done(); // Mark task as done to auto-update the state of each buttons tdf#132816
        }
        break;

        case SID_PHOTOALBUM:
        {
            SdAbstractDialogFactory* pFact = SdAbstractDialogFactory::Create();
            vcl::Window* pWin = GetActiveWindow();
            ScopedVclPtr<VclAbstractDialog> pDlg(pFact->CreateSdPhotoAlbumDialog(
                pWin ? pWin->GetFrameWeld() : nullptr,
                GetDoc()));

            pDlg->Execute();
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_INSERT_QRCODE:
        case SID_EDIT_QRCODE:
        {
            VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
            const uno::Reference<frame::XModel> xModel = GetViewShellBase().GetController()->getModel();
            ScopedVclPtr<AbstractQrCodeGenDialog> pDlg(pFact->CreateQrCodeGenDialog(
                GetFrameWeld(), xModel, rReq.GetSlot() == SID_EDIT_QRCODE));
            pDlg->Execute();
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_THEME_DIALOG:
        {
            SdrPage* pMasterPage = &GetActualPage()->TRG_GetMasterPage();
            auto pTheme = pMasterPage->getSdrPageProperties().getTheme();
            auto pDialog = std::make_shared<svx::ThemeDialog>(GetFrameWeld(), pTheme.get());
            auto* pDocShell = GetDocSh();
            weld::DialogController::runAsync(pDialog, [pDialog, pMasterPage, pDocShell](sal_uInt32 nResult)
            {
                if (RET_OK != nResult)
                    return;

                auto pColorSet = pDialog->getCurrentColorSet();
                if (pColorSet)
                {
                    sd::ThemeColorChanger aChanger(pMasterPage, pDocShell);
                    aChanger.apply(pColorSet);
                }
            });

            Cancel();
            rReq.Ignore();
        }
        break;

        case SID_ADDITIONS_DIALOG:
        {
            OUString sAdditionsTag = u""_ustr;

            const SfxStringItem* pStringArg = rReq.GetArg<SfxStringItem>(FN_PARAM_ADDITIONS_TAG);
            if (pStringArg)
                sAdditionsTag = pStringArg->GetValue();

            VclAbstractDialogFactory* pFact = VclAbstractDialogFactory::Create();
            VclPtr<AbstractAdditionsDialog> pDlg(
                pFact->CreateAdditionsDialog(GetFrameWeld(), sAdditionsTag));
            pDlg->StartExecuteAsync(
                [pDlg] (sal_Int32 /*nResult*/)->void
                {
                    pDlg->disposeOnce();
                }
            );
            Cancel();
            rReq.Ignore ();
        }
        break;

        case SID_ATTR_GLOW_COLOR:
        case SID_ATTR_GLOW_RADIUS:
        case SID_ATTR_GLOW_TRANSPARENCY:
        case SID_ATTR_SOFTEDGE_RADIUS:
        case SID_ATTR_TEXTCOLUMNS_NUMBER:
        case SID_ATTR_TEXTCOLUMNS_SPACING:
            if (const SfxItemSet* pNewArgs = rReq.GetArgs())
                mpDrawView->SetAttributes(*pNewArgs);
            rReq.Done();
            Cancel();
            break;

        default:
        {
            SAL_WARN( "sd.ui", "Slot without function" );
            Cancel();
            rReq.Ignore ();
        }
        break;
    }

    if(HasCurrentFunction())
    {
        GetCurrentFunction()->Activate();
    }
}

void DrawViewShell::ExecChar( SfxRequest &rReq )
{
    SdDrawDocument* pDoc = GetDoc();
    if (!pDoc || !mpDrawView)
        return;

    SfxItemSet aEditAttr( pDoc->GetPool() );
    mpDrawView->GetAttributes( aEditAttr );

    //modified by wj for sym2_1580, if put old itemset into new set,
    //when mpDrawView->SetAttributes(aNewAttr) it will invalidate all the item
    // and use old attr to update all the attributes
//  SfxItemSet aNewAttr( GetPool(),
//  EE_ITEMS_START, EE_ITEMS_END );
//  aNewAttr.Put( aEditAttr, sal_False );
    SfxItemSet aNewAttr( pDoc->GetPool() );
    //modified end

    sal_uInt16 nSId = rReq.GetSlot();

    switch ( nSId )
    {
    case SID_ATTR_CHAR_FONT:
        if( rReq.GetArgs() )
        {
            const SvxFontItem* pItem = rReq.GetArg<SvxFontItem>(SID_ATTR_CHAR_FONT);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_FONTHEIGHT:
        if( rReq.GetArgs() )
        {
            const SvxFontHeightItem* pItem = rReq.GetArg<SvxFontHeightItem>(SID_ATTR_CHAR_FONTHEIGHT);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_WEIGHT:
        if( rReq.GetArgs() )
        {
            const SvxWeightItem* pItem = rReq.GetArg<SvxWeightItem>(SID_ATTR_CHAR_WEIGHT);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_POSTURE:
        if( rReq.GetArgs() )
        {
            const SvxPostureItem* pItem = rReq.GetArg<SvxPostureItem>(SID_ATTR_CHAR_POSTURE);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_UNDERLINE:
        if( rReq.GetArgs() )
        {
            const SvxUnderlineItem* pItem = rReq.GetArg<SvxUnderlineItem>(SID_ATTR_CHAR_UNDERLINE);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
            else
            {
                FontLineStyle eFU = aEditAttr.Get( EE_CHAR_UNDERLINE ).GetLineStyle();
                aNewAttr.Put( SvxUnderlineItem( eFU != LINESTYLE_NONE ?LINESTYLE_NONE : LINESTYLE_SINGLE,  EE_CHAR_UNDERLINE ) );
            }
        }
        break;
    case SID_ATTR_CHAR_OVERLINE:
        if( rReq.GetArgs() )
        {
            const SvxOverlineItem* pItem = rReq.GetArg<SvxOverlineItem>(SID_ATTR_CHAR_OVERLINE);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
            else
            {
                FontLineStyle eFU = aEditAttr.Get( EE_CHAR_OVERLINE ).GetLineStyle();
                aNewAttr.Put( SvxOverlineItem( eFU != LINESTYLE_NONE ?LINESTYLE_NONE : LINESTYLE_SINGLE,  EE_CHAR_OVERLINE ) );
            }
        }
        break;

        case SID_ULINE_VAL_NONE:
        {
            aNewAttr.Put(SvxUnderlineItem(LINESTYLE_NONE, EE_CHAR_UNDERLINE));
            break;
        }

        case SID_ULINE_VAL_SINGLE:
        case SID_ULINE_VAL_DOUBLE:
        case SID_ULINE_VAL_DOTTED:
        {
            FontLineStyle eOld = aEditAttr.Get(EE_CHAR_UNDERLINE).GetLineStyle();
            FontLineStyle eNew = eOld;

            switch (nSId)
            {
                case SID_ULINE_VAL_SINGLE:
                    eNew = ( eOld == LINESTYLE_SINGLE ) ? LINESTYLE_NONE : LINESTYLE_SINGLE;
                    break;
                case SID_ULINE_VAL_DOUBLE:
                    eNew = ( eOld == LINESTYLE_DOUBLE ) ? LINESTYLE_NONE : LINESTYLE_DOUBLE;
                    break;
                case SID_ULINE_VAL_DOTTED:
                    eNew = ( eOld == LINESTYLE_DOTTED ) ? LINESTYLE_NONE : LINESTYLE_DOTTED;
                    break;
            }

            SvxUnderlineItem aUnderline(eNew, EE_CHAR_UNDERLINE);
            aNewAttr.Put(aUnderline);
        }
        break;

    case SID_ATTR_CHAR_SHADOWED:
        if( rReq.GetArgs() )
        {
            const SvxShadowedItem* pItem = rReq.GetArg<SvxShadowedItem>(SID_ATTR_CHAR_SHADOWED);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_CONTOUR:
        if( rReq.GetArgs() )
        {
            const SvxContourItem* pItem = rReq.GetArg<SvxContourItem>(SID_ATTR_CHAR_CONTOUR);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;

    case SID_ATTR_CHAR_STRIKEOUT:
        if( rReq.GetArgs() )
        {
            const SvxCrossedOutItem* pItem = rReq.GetArg<SvxCrossedOutItem>(SID_ATTR_CHAR_STRIKEOUT);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_COLOR:
        if( rReq.GetArgs() )
        {
            const SvxColorItem* pItem = rReq.GetArg<SvxColorItem>(SID_ATTR_CHAR_COLOR);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_KERNING:
        if( rReq.GetArgs() )
        {
            const SvxKerningItem* pItem = rReq.GetArg<SvxKerningItem>(SID_ATTR_CHAR_KERNING);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_ATTR_CHAR_CASEMAP:
        if( rReq.GetArgs() )
        {
            const SvxCaseMapItem* pItem = rReq.GetArg<SvxCaseMapItem>(SID_ATTR_CHAR_CASEMAP);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    case SID_SET_SMALL_CAPS:
        {
            SvxCaseMap eCaseMap = aEditAttr.Get(EE_CHAR_CASEMAP).GetCaseMap();
            if (eCaseMap == SvxCaseMap::SmallCaps)
                eCaseMap = SvxCaseMap::NotMapped;
            else
                eCaseMap = SvxCaseMap::SmallCaps;
            SvxCaseMapItem aItem(eCaseMap, EE_CHAR_CASEMAP);
            aNewAttr.Put(aItem);
        }
        break;
    case SID_SET_SUB_SCRIPT:
        {
            SvxEscapementItem aItem( EE_CHAR_ESCAPEMENT );
            SvxEscapement eEsc = static_cast<SvxEscapement>(aEditAttr.Get( EE_CHAR_ESCAPEMENT ).GetEnumValue());
            if( eEsc == SvxEscapement::Subscript )
                aItem.SetEscapement( SvxEscapement::Off );
            else
                aItem.SetEscapement( SvxEscapement::Subscript );
            aNewAttr.Put( aItem );
        }
        break;
    case SID_SET_SUPER_SCRIPT:
        {
            SvxEscapementItem aItem( EE_CHAR_ESCAPEMENT );
            SvxEscapement eEsc = static_cast<SvxEscapement>(aEditAttr.Get( EE_CHAR_ESCAPEMENT ).GetEnumValue());
            if( eEsc == SvxEscapement::Superscript )
                aItem.SetEscapement( SvxEscapement::Off );
            else
                aItem.SetEscapement( SvxEscapement::Superscript );
            aNewAttr.Put( aItem );
        }
        break;
    case SID_SHRINK_FONT_SIZE:
    case SID_GROW_FONT_SIZE:
        {
            const SvxFontListItem* pFonts = dynamic_cast<const SvxFontListItem*>(GetDocSh()->GetItem( SID_ATTR_CHAR_FONTLIST ) );
            const FontList* pFontList = pFonts ? pFonts->GetFontList() : nullptr;
            if( pFontList )
            {
                FuText::ChangeFontSize( nSId == SID_GROW_FONT_SIZE, nullptr, pFontList, mpView );
                GetViewFrame()->GetBindings().Invalidate( SID_ATTR_CHAR_FONTHEIGHT );
            }
            break;
        }
    case SID_ATTR_CHAR_BACK_COLOR:
        if( rReq.GetArgs() )
        {
            const SvxColorItem* pItem = rReq.GetArg<SvxColorItem>(SID_ATTR_CHAR_BACK_COLOR);
            if (pItem)
            {
                aNewAttr.Put(*pItem);
            }
        }
        break;
    default:
        break;
    }

    mpDrawView->SetAttributes(aNewAttr);
    rReq.Done();
    Cancel();
}

/** This method consists basically of three parts:
    1. Process the arguments of the SFX request.
    2. Use the model to create a new page or duplicate an existing one.
    3. Update the tab control and switch to the new page.
*/
SdPage* DrawViewShell::CreateOrDuplicatePage (
    SfxRequest& rRequest,
    PageKind ePageKind,
    SdPage* pPage,
    const sal_Int32 nInsertPosition)
{
    SdPage* pNewPage = nullptr;
    if (ePageKind == PageKind::Standard && meEditMode != EditMode::MasterPage)
    {
        if ( mpDrawView->IsTextEdit() )
        {
            mpDrawView->SdrEndTextEdit();
        }
        pNewPage = ViewShell::CreateOrDuplicatePage (rRequest, ePageKind, pPage, nInsertPosition);
    }
    return pNewPage;
}

void DrawViewShell::DuplicateSelectedSlides (SfxRequest& rRequest)
{
    // Create a list of the pages that are to be duplicated.  The process of
    // duplication alters the selection.
    sal_Int32 nInsertPosition (0);
    ::std::vector<SdPage*> aPagesToDuplicate;
    sd::slidesorter::SlideSorter &mrSlideSorter = sd::slidesorter::SlideSorterViewShell::GetSlideSorter(GetViewShellBase())->GetSlideSorter();
    sd::slidesorter::model::PageEnumeration aSelectedPages (
        sd::slidesorter::model::PageEnumerationProvider::CreateSelectedPagesEnumeration(mrSlideSorter.GetModel()));
    while (aSelectedPages.HasMoreElements())
    {
        sd::slidesorter::model::SharedPageDescriptor pDescriptor (aSelectedPages.GetNextElement());
        if (pDescriptor && pDescriptor->GetPage())
        {
            aPagesToDuplicate.push_back(pDescriptor->GetPage());
            nInsertPosition = pDescriptor->GetPage()->GetPageNum()+2;
        }
    }

    // Duplicate the pages in aPagesToDuplicate and collect the newly
    // created pages in aPagesToSelect.
    const bool bUndo (aPagesToDuplicate.size()>1 && mrSlideSorter.GetView().IsUndoEnabled());
    if (bUndo)
        mrSlideSorter.GetView().BegUndo(SdResId(STR_INSERTPAGE));

    ::std::vector<SdPage*> aPagesToSelect;
    for(::std::vector<SdPage*>::const_iterator
            iPage(aPagesToDuplicate.begin()),
            iEnd(aPagesToDuplicate.end());
        iPage!=iEnd;
        ++iPage, nInsertPosition+=2)
    {
        aPagesToSelect.push_back(
            mrSlideSorter.GetViewShell()->CreateOrDuplicatePage(
                rRequest, PageKind::Standard, *iPage, nInsertPosition));
    }
    aPagesToDuplicate.clear();

    if (bUndo)
        mrSlideSorter.GetView().EndUndo();

    // Set the selection to the pages in aPagesToSelect.
    sd::slidesorter::controller::PageSelector& rSelector (mrSlideSorter.GetController().GetPageSelector());
    rSelector.DeselectAllPages();
    for (auto const& it: aPagesToSelect)
    {
        rSelector.SelectPage(it);
    }
}

void DrawViewShell::ExecutePropPanelAttr (SfxRequest const & rReq)
{
    if(SlideShow::IsRunning( GetViewShellBase() )
        && !SlideShow::IsInteractiveSlideshow( &GetViewShellBase() )) // IASS
        return;

    SdDrawDocument* pDoc = GetDoc();
    if (!pDoc || !mpDrawView)
        return;

    sal_uInt16 nSId = rReq.GetSlot();
    SfxItemSet aAttrs( pDoc->GetPool() );

    switch ( nSId )
    {
    case SID_TABLE_VERT_NONE:
    case SID_TABLE_VERT_CENTER:
    case SID_TABLE_VERT_BOTTOM:
        SdrTextVertAdjust eTVA = SDRTEXTVERTADJUST_TOP;
        if (nSId == SID_TABLE_VERT_CENTER)
            eTVA = SDRTEXTVERTADJUST_CENTER;
        else if (nSId == SID_TABLE_VERT_BOTTOM)
            eTVA = SDRTEXTVERTADJUST_BOTTOM;

        aAttrs.Put( SdrTextVertAdjustItem(eTVA) );
        mpDrawView->SetAttributes(aAttrs);

        break;
    }
}

void DrawViewShell::GetStatePropPanelAttr(SfxItemSet& rSet)
{
    SfxWhichIter    aIter( rSet );
    sal_uInt16          nWhich = aIter.FirstWhich();

    SdDrawDocument* pDoc = GetDoc();
    if (!pDoc || !mpDrawView)
        return;

    SfxItemSet aAttrs( pDoc->GetPool() );
    mpDrawView->GetAttributes( aAttrs );

    while ( nWhich )
    {
        sal_uInt16 nSlotId = SfxItemPool::IsWhich(nWhich)
            ? GetPool().GetSlotId(nWhich)
            : nWhich;
        switch ( nSlotId )
        {
            case SID_TABLE_VERT_NONE:
            case SID_TABLE_VERT_CENTER:
            case SID_TABLE_VERT_BOTTOM:
                bool bContour = false;
                SfxItemState eConState = aAttrs.GetItemState( SDRATTR_TEXT_CONTOURFRAME );
                if( eConState != SfxItemState::INVALID )
                {
                    bContour = aAttrs.Get( SDRATTR_TEXT_CONTOURFRAME ).GetValue();
                }
                if (bContour) break;

                SfxItemState eVState = aAttrs.GetItemState( SDRATTR_TEXT_VERTADJUST );
                //SfxItemState eHState = aAttrs.GetItemState( SDRATTR_TEXT_HORZADJUST );

                //if(SfxItemState::INVALID != eVState && SfxItemState::INVALID != eHState)
                if(SfxItemState::INVALID != eVState)
                {
                    SdrTextVertAdjust eTVA = aAttrs.Get(SDRATTR_TEXT_VERTADJUST).GetValue();
                    bool bSet = (nSlotId == SID_TABLE_VERT_NONE && eTVA == SDRTEXTVERTADJUST_TOP) ||
                            (nSlotId == SID_TABLE_VERT_CENTER && eTVA == SDRTEXTVERTADJUST_CENTER) ||
                            (nSlotId == SID_TABLE_VERT_BOTTOM && eTVA == SDRTEXTVERTADJUST_BOTTOM);
                    rSet.Put(SfxBoolItem(nSlotId, bSet));
                }
                else
                {
                    rSet.Put(SfxBoolItem(nSlotId, false));
                }
                break;
        }
        nWhich = aIter.NextWhich();
    }
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
