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

#include <swtypes.hxx>
#include "mmlayoutpage.hxx"
#include <mailmergewizard.hxx>
#include <mmconfigitem.hxx>
#include <mailmergehelper.hxx>
#include <unotools.hxx>
#include <comphelper/string.hxx>
#include <i18nutil/unicode.hxx>
#include <unotools/tempfile.hxx>
#include <uitool.hxx>
#include <svx/dlgutil.hxx>
#include <view.hxx>
#include <swundo.hxx>
#include <sfx2/dispatch.hxx>
#include <svl/stritem.hxx>
#include <sfx2/docfilt.hxx>
#include <com/sun/star/text/XParagraphCursor.hpp>
#include <com/sun/star/view/XViewSettingsSupplier.hpp>
#include <com/sun/star/view/DocumentZoomType.hpp>
#include <fldmgr.hxx>
#include <fldbas.hxx>
#include <poolfmt.hxx>
#include <unotxdoc.hxx>
#include <docsh.hxx>
#include <doc.hxx>
#include <wrtsh.hxx>
#include <fmtsrnd.hxx>
#include <pagedesc.hxx>
#include <fmtanchr.hxx>
#include <fmtornt.hxx>
#include <fmtfsize.hxx>
#include <editeng/boxitem.hxx>
#include <svl/urihelper.hxx>
#include <shellio.hxx>
#include <osl/file.hxx>
#include <vcl/settings.hxx>
#include <unoprnms.hxx>
#include <iodetect.hxx>

#include <dbui.hrc>
#include <unomid.h>

using namespace osl;
using namespace svt;
using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::text;
using namespace ::com::sun::star::frame;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::view;

#define DEFAULT_LEFT_DISTANCE (MM50*5)  // 2,5 cm
#define DEFAULT_TOP_DISTANCE  (MM50*11) // 5,5 cm
#define GREETING_TOP_DISTANCE (MM50*25) //12,5 cm
#define DEFAULT_ADDRESS_WIDTH  (MM50*15)// 7,5 cm
#define DEFAULT_ADDRESS_HEIGHT (MM50*7) // 3,5cm

SwMailMergeLayoutPage::SwMailMergeLayoutPage( SwMailMergeWizard* _pParent) :
    svt::OWizardPage(_pParent, "MMLayoutPage",
        "modules/swriter/ui/mmlayoutpage.ui")
    , m_pExampleWrtShell(nullptr)
    , m_pAddressBlockFormat(nullptr)
    , m_bIsGreetingInserted(false)
    , m_pWizard(_pParent)
{
    get(m_pPosition, "addresspos");
    get(m_pGreetingLine, "greetingspos");
    get(m_pAlignToBodyCB, "align");
    get(m_pLeftFT, "leftft");
    get(m_pLeftMF, "left");
    get(m_pTopMF, "top");
    get(m_pUpPB, "up");
    get(m_pDownPB, "down");
    get(m_pExampleContainerWIN, "example");
    Size aSize(LogicToPixel(Size(124, 159), MapMode(MapUnit::MapAppFont)));
    m_pExampleContainerWIN->set_width_request(aSize.Width());
    m_pExampleContainerWIN->set_height_request(aSize.Height());
    get(m_pZoomLB, "zoom");

    std::shared_ptr<const SfxFilter> pSfxFlt = SwIoSystem::GetFilterOfFormat(
            FILTER_XML,
            SwDocShell::Factory().GetFilterContainer() );
    //save the current document into a temporary file
    {
        //temp file needs its own block
        //creating with extension is not supported by a static method :-(
        OUString const sExt(
            comphelper::string::stripStart(pSfxFlt->GetDefaultExtension(),'*'));
        utl::TempFile aTempFile( OUString(), true, &sExt );
        m_sExampleURL = aTempFile.GetURL();
        aTempFile.EnableKillingFile();
    }
    SwView* pView = m_pWizard->GetSwView();
    uno::Sequence< beans::PropertyValue > aValues(2);
    beans::PropertyValue* pValues = aValues.getArray();
    pValues[0].Name = "FilterName";
    pValues[0].Value <<= pSfxFlt->GetFilterName();
    // Don't save embedded data set! It would steal it from current document.
    pValues[1].Name = "NoEmbDataSet";
    pValues[1].Value <<= true;

    uno::Reference< frame::XStorable > xStore( pView->GetDocShell()->GetModel(), uno::UNO_QUERY);
    xStore->storeToURL( m_sExampleURL, aValues   );

    Link<SwOneExampleFrame&,void> aLink(LINK(this, SwMailMergeLayoutPage, PreviewLoadedHdl_Impl));
    m_pExampleFrame.reset( new SwOneExampleFrame( *m_pExampleContainerWIN,
                                    EX_SHOW_DEFAULT_PAGE, &aLink, &m_sExampleURL ) );

    m_pExampleContainerWIN->Show(false);

    m_pLeftMF->SetValue(m_pLeftMF->Normalize(DEFAULT_LEFT_DISTANCE), FieldUnit::TWIP);
    m_pTopMF->SetValue(m_pTopMF->Normalize(DEFAULT_TOP_DISTANCE), FieldUnit::TWIP);

    const LanguageTag& rLang = Application::GetSettings().GetUILanguageTag();
    m_pZoomLB->InsertEntry(unicode::formatPercent(50, rLang), 1);
    m_pZoomLB->InsertEntry(unicode::formatPercent(75, rLang), 2);
    m_pZoomLB->InsertEntry(unicode::formatPercent(100, rLang), 3);
    m_pZoomLB->SelectEntryPos(0); //page size
    m_pZoomLB->SetSelectHdl(LINK(this, SwMailMergeLayoutPage, ZoomHdl_Impl));

    Link<SpinField&,void> aFrameHdl = LINK(this, SwMailMergeLayoutPage, ChangeAddressHdl_Impl);
    Link<Control&,void> aFocusHdl = LINK(this, SwMailMergeLayoutPage, ChangeAddressLoseFocusHdl_Impl);
    m_pLeftMF->SetUpHdl(aFrameHdl);
    m_pLeftMF->SetDownHdl(aFrameHdl);
    m_pLeftMF->SetLoseFocusHdl(aFocusHdl);
    m_pTopMF->SetUpHdl(aFrameHdl);
    m_pTopMF->SetDownHdl(aFrameHdl);
    m_pTopMF->SetLoseFocusHdl(aFocusHdl);

    FieldUnit eFieldUnit = ::GetDfltMetric(false);
    ::SetFieldUnit( *m_pLeftMF, eFieldUnit );
    ::SetFieldUnit( *m_pTopMF, eFieldUnit );

    Link<Button*,void> aUpDownHdl = LINK(this, SwMailMergeLayoutPage, GreetingsHdl_Impl );
    m_pUpPB->SetClickHdl(aUpDownHdl);
    m_pDownPB->SetClickHdl(aUpDownHdl);
    m_pAlignToBodyCB->SetClickHdl(LINK(this, SwMailMergeLayoutPage, AlignToTextHdl_Impl));
    m_pAlignToBodyCB->Check();
}

SwMailMergeLayoutPage::~SwMailMergeLayoutPage()
{
    disposeOnce();
}

void SwMailMergeLayoutPage::dispose()
{
    m_pExampleFrame.reset();
    File::remove( m_sExampleURL );
    m_pPosition.clear();
    m_pAlignToBodyCB.clear();
    m_pLeftFT.clear();
    m_pLeftMF.clear();
    m_pTopMF.clear();
    m_pGreetingLine.clear();
    m_pUpPB.clear();
    m_pDownPB.clear();
    m_pExampleContainerWIN.clear();
    m_pZoomLB.clear();
    m_pWizard.clear();
    svt::OWizardPage::dispose();
}

void SwMailMergeLayoutPage::ActivatePage()
{
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    bool bGreetingLine = rConfigItem.IsGreetingLine(false) && !rConfigItem.IsGreetingInserted();
    bool bAddressBlock = rConfigItem.IsAddressBlock() && !rConfigItem.IsAddressInserted();

    m_pPosition->Enable(bAddressBlock);
    AlignToTextHdl_Impl(m_pAlignToBodyCB);

    m_pGreetingLine->Enable(bGreetingLine);

    //check if greeting and/or address frame have to be inserted/removed
    if(m_pExampleWrtShell) // initially there's nothing to check
    {
        if(!rConfigItem.IsGreetingInserted() &&
                m_bIsGreetingInserted != bGreetingLine )
        {
            if( m_bIsGreetingInserted )
            {
                m_pExampleWrtShell->DelFullPara();
                m_bIsGreetingInserted = false;
            }
            else
            {
                InsertGreeting(*m_pExampleWrtShell, m_pWizard->GetConfigItem(), true);
                m_bIsGreetingInserted = true;
            }
        }
        if(!rConfigItem.IsAddressInserted() &&
                rConfigItem.IsAddressBlock() != ( nullptr != m_pAddressBlockFormat ))
        {
            if( m_pAddressBlockFormat )
            {
                m_pExampleWrtShell->Push();
                m_pExampleWrtShell->GotoFly( m_pAddressBlockFormat->GetName() );
                m_pExampleWrtShell->DelRight();
                m_pAddressBlockFormat = nullptr;
                m_pExampleWrtShell->Pop(SwCursorShell::PopMode::DeleteCurrent);
            }
            else
            {
                long nLeft = static_cast< long >(m_pLeftMF->Denormalize(m_pLeftMF->GetValue(FieldUnit::TWIP)));
                long nTop  = static_cast< long >(m_pTopMF->Denormalize(m_pTopMF->GetValue(FieldUnit::TWIP)));
                m_pAddressBlockFormat = InsertAddressFrame(
                        *m_pExampleWrtShell, m_pWizard->GetConfigItem(),
                        Point(nLeft, nTop),
                        m_pAlignToBodyCB->IsChecked(), true);
            }
        }

    }
}

bool SwMailMergeLayoutPage::commitPage(::svt::WizardTypes::CommitPageReason eReason)
{
    //now insert the frame and the greeting
    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    if (eReason == ::svt::WizardTypes::eTravelForward || eReason == ::svt::WizardTypes::eFinish)
    {
        long nLeft = static_cast< long >(m_pLeftMF->Denormalize(m_pLeftMF->GetValue(FieldUnit::TWIP)));
        long nTop  = static_cast< long >(m_pTopMF->Denormalize(m_pTopMF->GetValue(FieldUnit::TWIP)));
        InsertAddressAndGreeting(
                    m_pWizard->GetSwView(),
                    rConfigItem,
                    Point(nLeft, nTop),
                    m_pAlignToBodyCB->IsChecked());
    }
    return true;
}

SwFrameFormat*  SwMailMergeLayoutPage::InsertAddressAndGreeting(SwView const * pView,
        SwMailMergeConfigItem& rConfigItem,
        const Point& rAddressPosition,
        bool bAlignToBody)
{
    SwFrameFormat* pAddressBlockFormat = nullptr;
    pView->GetWrtShell().StartUndo(SwUndoId::INSERT);
    if(rConfigItem.IsAddressBlock() && !rConfigItem.IsAddressInserted())
    {
        //insert the frame
        Point aAddressPosition(DEFAULT_LEFT_DISTANCE, DEFAULT_TOP_DISTANCE);
        if(rAddressPosition.X() > 0 && rAddressPosition.Y() > 0)
            aAddressPosition = rAddressPosition;
        pAddressBlockFormat = InsertAddressFrame( pView->GetWrtShell(),
                                        rConfigItem,
                                        aAddressPosition, bAlignToBody, false);
        rConfigItem.SetAddressInserted();
    }
    //now the greeting
    if(rConfigItem.IsGreetingLine(false) && !rConfigItem.IsGreetingInserted())
    {
        InsertGreeting( pView->GetWrtShell(), rConfigItem, false);
        rConfigItem.SetGreetingInserted();
    }
    pView->GetWrtShell().EndUndo(SwUndoId::INSERT);
    return pAddressBlockFormat;
}

SwFrameFormat* SwMailMergeLayoutPage::InsertAddressFrame(
        SwWrtShell& rShell,
        SwMailMergeConfigItem const & rConfigItem,
        const Point& rDestination,
        bool bAlignLeft,
        bool bExample)
{
    // insert the address block and the greeting line
    SfxItemSet aSet(
        rShell.GetAttrPool(),
        svl::Items<
            RES_FRM_SIZE, RES_FRM_SIZE,
            RES_SURROUND, RES_ANCHOR,
            RES_BOX, RES_BOX>{} );
    aSet.Put(SwFormatAnchor(RndStdIds::FLY_AT_PAGE, 1));
    if(bAlignLeft)
        aSet.Put(SwFormatHoriOrient( 0, text::HoriOrientation::NONE, text::RelOrientation::PAGE_PRINT_AREA ));
    else
        aSet.Put(SwFormatHoriOrient( rDestination.X(), text::HoriOrientation::NONE, text::RelOrientation::PAGE_FRAME ));
    aSet.Put(SwFormatVertOrient( rDestination.Y(), text::VertOrientation::NONE, text::RelOrientation::PAGE_FRAME ));
    aSet.Put(SwFormatFrameSize( ATT_MIN_SIZE, DEFAULT_ADDRESS_WIDTH, DEFAULT_ADDRESS_HEIGHT ));
    // the example gets a border around the frame, the real document doesn't get one
    if(!bExample)
        aSet.Put(SvxBoxItem( RES_BOX ));
    aSet.Put(SwFormatSurround( css::text::WrapTextMode_NONE ));

    rShell.NewFlyFrame(aSet, true );
    SwFrameFormat* pRet = rShell.GetFlyFrameFormat();
    OSL_ENSURE( pRet, "Fly not inserted" );

    rShell.UnSelectFrame();
    const Sequence< OUString> aBlocks = rConfigItem.GetAddressBlocks();
    if(bExample)
    {
        rShell.Insert(aBlocks[0]);
    }
    else
    {
        //the placeholders should be replaced by the appropriate fields
        SwFieldMgr aFieldMgr(&rShell);
        //create a database string source.command.commandtype.column
        const SwDBData& rData = rConfigItem.GetCurrentDBData();
        OUString sDBName(rData.sDataSource + OUStringLiteral1(DB_DELIM)
            + rData.sCommand + OUStringLiteral1(DB_DELIM));
        const OUString sDatabaseConditionPrefix(sDBName.replace(DB_DELIM, '.'));
        sDBName += OUString::number(rData.nCommandType) + OUStringLiteral1(DB_DELIM);

        // if only the country is in an address line the
        // paragraph has to be hidden depending on the
        // IsIncludeCountry()/GetExcludeCountry() settings

        bool bIncludeCountry = rConfigItem.IsIncludeCountry();
        bool bHideEmptyParagraphs = rConfigItem.IsHideEmptyParagraphs();
        const OUString rExcludeCountry = rConfigItem.GetExcludeCountry();
        bool bSpecialReplacementForCountry = (!bIncludeCountry || !rExcludeCountry.isEmpty());

        const std::vector<std::pair<OUString, int>>& rHeaders = rConfigItem.GetDefaultAddressHeaders();
        Sequence< OUString> aAssignment =
                        rConfigItem.GetColumnAssignment( rConfigItem.GetCurrentDBData() );
        const OUString* pAssignment = aAssignment.getConstArray();
        const OUString sCountryColumn(
            (aAssignment.getLength() > MM_PART_COUNTRY && !aAssignment[MM_PART_COUNTRY].isEmpty())
            ? aAssignment[MM_PART_COUNTRY]
            : rHeaders[MM_PART_COUNTRY].first);

        OUString sHideParagraphsExpression;
        SwAddressIterator aIter(aBlocks[0]);
        while(aIter.HasMore())
        {
            SwMergeAddressItem aItem = aIter.Next();
            if(aItem.bIsColumn)
            {
                OUString sConvertedColumn = aItem.sText;
                for(sal_uInt32 nColumn = 0;
                        nColumn < rHeaders.size() &&
                        nColumn < static_cast<sal_uInt32>(aAssignment.getLength());
                                                                                    ++nColumn)
                {
                    if (rHeaders[nColumn].first == aItem.sText &&
                        !pAssignment[nColumn].isEmpty())
                    {
                        sConvertedColumn = pAssignment[nColumn];
                        break;
                    }
                }
                const OUString sDB(sDBName + sConvertedColumn);

                if(!sHideParagraphsExpression.isEmpty())
                   sHideParagraphsExpression += " AND ";
                sHideParagraphsExpression += "![" + sDatabaseConditionPrefix + sConvertedColumn + "]";

                if( bSpecialReplacementForCountry && sCountryColumn == sConvertedColumn )
                {
                    // now insert a hidden paragraph field
                    if( !rExcludeCountry.isEmpty() )
                    {
                        const OUString sExpression("[" + sDatabaseConditionPrefix + sCountryColumn + "]");
                        SwInsertField_Data aData(TYP_CONDTXTFLD, 0,
                                               sExpression + " != \"" + rExcludeCountry + "\"",
                                               sExpression,
                                               0, &rShell );
                        aFieldMgr.InsertField( aData );
                    }
                    else
                    {
                        SwInsertField_Data aData(TYP_HIDDENPARAFLD, 0, "", "", 0, &rShell );
                        aFieldMgr.InsertField( aData );
                    }
                }
                else
                {
                    SwInsertField_Data aData(TYP_DBFLD, 0, sDB, OUString(), 0, &rShell);
                    aFieldMgr.InsertField( aData );
                }
            }
            else if(!aItem.bIsReturn)
            {
                rShell.Insert(aItem.sText);
            }
            else
            {
                if(bHideEmptyParagraphs)
                {
                    SwInsertField_Data aData(TYP_HIDDENPARAFLD, 0, sHideParagraphsExpression, OUString(), 0, &rShell);
                    aFieldMgr.InsertField( aData );
                }
                sHideParagraphsExpression.clear();
                //now add a new paragraph
                rShell.SplitNode();
            }
        }
        if(bHideEmptyParagraphs && !sHideParagraphsExpression.isEmpty())
        {
            SwInsertField_Data aData(TYP_HIDDENPARAFLD, 0, sHideParagraphsExpression, OUString(), 0, &rShell);
            aFieldMgr.InsertField( aData );
        }
    }
    return pRet;
}

void SwMailMergeLayoutPage::InsertGreeting(SwWrtShell& rShell, SwMailMergeConfigItem const & rConfigItem, bool bExample)
{
    //set the cursor to the desired position - if no text content is here then
    //new paragraphs are inserted
    const SwRect& rPageRect = rShell.GetAnyCurRect(CurRectType::Page);
    const Point aGreetingPos( DEFAULT_LEFT_DISTANCE + rPageRect.Left(), GREETING_TOP_DISTANCE );

    const bool bRet = rShell.SetShadowCursorPos( aGreetingPos, FILL_TAB_SPACE );

    if(!bRet)
    {
        //there's already text at the desired position
        //go to start of the doc, directly!
        rShell.SttEndDoc(true);
        //and go by paragraph until the position is reached
        long nYPos = rShell.GetCharRect().Top();
        while(nYPos < GREETING_TOP_DISTANCE)
        {
            if(!rShell.FwdPara())
                break;
            nYPos = rShell.GetCharRect().Top();
        }
        //text needs to be appended
        while(nYPos < GREETING_TOP_DISTANCE)
        {
            if(!rShell.AppendTextNode())
                break;
            nYPos = rShell.GetCharRect().Top();
        }
    }
    else
    {
        //we may end up inside of a paragraph if the left margin is not at DEFAULT_LEFT_DISTANCE
        rShell.MovePara(GoCurrPara, fnParaStart);
    }
    bool bSplitNode = !rShell.IsEndPara();
    sal_Int32 nMoves = rConfigItem.GetGreetingMoves();
    if( !bExample && 0 != nMoves )
    {
        if(nMoves < 0)
        {
            rShell.MoveParagraph( nMoves );
        }
        else
            while(nMoves)
            {
                bool bMoved = rShell.MoveParagraph();
                if(!bMoved)
                {
                    //insert a new paragraph before the greeting line
                    rShell.SplitNode();
                }
                --nMoves;
            }
    }
    //now insert the greeting text - if we have any?
    const bool bIndividual = rConfigItem.IsIndividualGreeting(false);
    if(bIndividual)
    {
        //lock expression fields - prevents hiding of the paragraph to insert into
        rShell.LockExpFields();
        if(bExample)
        {
            for(sal_Int8 eGender = SwMailMergeConfigItem::FEMALE;
                eGender <= SwMailMergeConfigItem::NEUTRAL; ++eGender)
            {
                Sequence< OUString > aEntries =
                            rConfigItem.GetGreetings(static_cast<SwMailMergeConfigItem::Gender>(eGender));
                sal_Int32 nCurrent = rConfigItem.GetCurrentGreeting(static_cast<SwMailMergeConfigItem::Gender>(eGender));
                if( nCurrent >= 0 && nCurrent < aEntries.getLength())
                {
                    // Greeting
                    rShell.Insert(aEntries[nCurrent]);
                    break;
                }
            }
        }
        else
        {
            SwFieldMgr aFieldMgr(&rShell);
            //three paragraphs, each with an appropriate hidden paragraph field
            //are to be inserted

            //name of the gender column
            const OUString sGenderColumn = rConfigItem.GetAssignedColumn(MM_PART_GENDER);
            const OUString sNameColumn = rConfigItem.GetAssignedColumn(MM_PART_LASTNAME);

            const OUString& rFemaleGenderValue = rConfigItem.GetFemaleGenderValue();
            bool bHideEmptyParagraphs = rConfigItem.IsHideEmptyParagraphs();
            const SwDBData& rData = rConfigItem.GetCurrentDBData();
            const OUString sCommonBase(rData.sDataSource + "." + rData.sCommand + ".");
            const OUString sConditionBase("[" + sCommonBase + sGenderColumn + "]");
            const OUString sNameColumnBase("[" + sCommonBase + sNameColumn + "]");

            const OUString sDBName(rData.sDataSource + OUStringLiteral1(DB_DELIM)
                + rData.sCommand + OUStringLiteral1(DB_DELIM)
                + OUString::number(rData.nCommandType) + OUStringLiteral1(DB_DELIM));

//          Female:  [database.sGenderColumn] != "rFemaleGenderValue" && [database.NameColumn]
//          Male:    [database.sGenderColumn] == "rFemaleGenderValue" && [database.rGenderColumn]
//          Neutral: [database.sNameColumn]
            OSL_ENSURE(!sGenderColumn.isEmpty() && !rFemaleGenderValue.isEmpty(),
                    "gender settings not available - how to form the condition?");
            //column used as lastname
            for(sal_Int8 eGender = SwMailMergeConfigItem::FEMALE;
                eGender <= SwMailMergeConfigItem::NEUTRAL; ++eGender)
            {
                Sequence< OUString> aEntries = rConfigItem.GetGreetings(static_cast<SwMailMergeConfigItem::Gender>(eGender));
                sal_Int32 nCurrent = rConfigItem.GetCurrentGreeting(static_cast<SwMailMergeConfigItem::Gender>(eGender));
                if( nCurrent >= 0 && nCurrent < aEntries.getLength())
                {
                    const OUString sGreeting = aEntries[nCurrent];
                    OUString sCondition;
                    OUString sHideParagraphsExpression;
                    switch(eGender)
                    {
                        case  SwMailMergeConfigItem::FEMALE:
                            sCondition = sConditionBase + " != \"" + rFemaleGenderValue
                                + "\" OR NOT " + sNameColumnBase;
                            sHideParagraphsExpression = "!" + sNameColumnBase;
                        break;
                        case  SwMailMergeConfigItem::MALE:
                            sCondition = sConditionBase + " == \"" + rFemaleGenderValue
                                + "\" OR NOT " + sNameColumnBase;
                        break;
                        case  SwMailMergeConfigItem::NEUTRAL:
                            sCondition = sNameColumnBase;
                        break;
                    }

                    if(bHideEmptyParagraphs && !sHideParagraphsExpression.isEmpty())
                    {
                        OUString sComplete = "(" + sCondition + ") OR (" + sHideParagraphsExpression + ")";
                        SwInsertField_Data aData(TYP_HIDDENPARAFLD, 0, sComplete, OUString(), 0, &rShell);
                        aFieldMgr.InsertField( aData );
                    }
                    else
                    {
                        SwInsertField_Data aData(TYP_HIDDENPARAFLD, 0, sCondition, OUString(), 0, &rShell);
                        aFieldMgr.InsertField( aData );
                    }
                    //now the text has to be inserted
                    const std::vector<std::pair<OUString, int>>& rHeaders = rConfigItem.GetDefaultAddressHeaders();
                    Sequence< OUString> aAssignment =
                                    rConfigItem.GetColumnAssignment( rConfigItem.GetCurrentDBData() );
                    const OUString* pAssignment = aAssignment.getConstArray();
                    SwAddressIterator aIter(sGreeting);
                    while(aIter.HasMore())
                    {
                        SwMergeAddressItem aItem = aIter.Next();
                        if(aItem.bIsColumn)
                        {
                            OUString sConvertedColumn = aItem.sText;
                            for(sal_uInt32 nColumn = 0;
                                    nColumn < rHeaders.size() &&
                                    nColumn < static_cast<sal_uInt32>(aAssignment.getLength());
                                                                                                ++nColumn)
                            {
                                if (rHeaders[nColumn].first == aItem.sText &&
                                    !pAssignment[nColumn].isEmpty())
                                {
                                    sConvertedColumn = pAssignment[nColumn];
                                    break;
                                }
                            }
                            SwInsertField_Data aData(TYP_DBFLD, 0,
                                sDBName + sConvertedColumn,
                                OUString(), 0, &rShell);
                            aFieldMgr.InsertField( aData );
                        }
                        else
                        {
                            rShell.Insert(aItem.sText);
                        }
                    }
                    //now add a new paragraph
                    rShell.SplitNode();
                }
            }

        }
        rShell.UnlockExpFields();
    }
    else
    {
        Sequence< OUString> aEntries = rConfigItem.GetGreetings(SwMailMergeConfigItem::NEUTRAL);
        sal_Int32 nCurrent = rConfigItem.GetCurrentGreeting(SwMailMergeConfigItem::NEUTRAL);
        // Greeting
        rShell.Insert(( nCurrent >= 0 && nCurrent < aEntries.getLength() )
            ? aEntries[nCurrent] : OUString());
    }
    // now insert a new paragraph here if necessary
    if(bSplitNode)
    {
        rShell.Push();
        rShell.SplitNode();
        rShell.Pop(SwCursorShell::PopMode::DeleteCurrent);
    }
    //put the cursor to the start of the paragraph
    rShell.SttPara();

    OSL_ENSURE(nullptr == rShell.GetTableFormat(), "What to do with a table here?");
}

IMPL_LINK_NOARG(SwMailMergeLayoutPage, PreviewLoadedHdl_Impl, SwOneExampleFrame&, void)
{
    m_pExampleContainerWIN->Show();

    Reference< XModel > & xModel = m_pExampleFrame->GetModel();
    //now the ViewOptions should be set properly
    Reference< XViewSettingsSupplier >  xSettings(xModel->getCurrentController(), UNO_QUERY);
    m_xViewProperties = xSettings->getViewSettings();
    Reference< XUnoTunnel > xDocTunnel(xModel, UNO_QUERY);
    SwXTextDocument* pXDoc = reinterpret_cast<SwXTextDocument*>(xDocTunnel->getSomething(SwXTextDocument::getUnoTunnelId()));
    SwDocShell* pDocShell = pXDoc->GetDocShell();
    m_pExampleWrtShell = pDocShell->GetWrtShell();
    OSL_ENSURE(m_pExampleWrtShell, "No SwWrtShell found!");
    if(!m_pExampleWrtShell)
        return;

    SwMailMergeConfigItem& rConfigItem = m_pWizard->GetConfigItem();
    if(rConfigItem.IsAddressBlock())
    {
        m_pAddressBlockFormat = InsertAddressFrame(
                *m_pExampleWrtShell, rConfigItem,
                Point(DEFAULT_LEFT_DISTANCE, DEFAULT_TOP_DISTANCE),
                m_pAlignToBodyCB->IsChecked(), true);
    }
    if(rConfigItem.IsGreetingLine(false))
    {
        InsertGreeting(*m_pExampleWrtShell, rConfigItem, true);
        m_bIsGreetingInserted = true;
    }

    Any aZoom;
    aZoom <<= sal_Int16(DocumentZoomType::ENTIRE_PAGE);
    m_xViewProperties->setPropertyValue(UNO_NAME_ZOOM_TYPE, aZoom);

    const SwFormatFrameSize& rPageSize = m_pExampleWrtShell->GetPageDesc(
                                     m_pExampleWrtShell->GetCurPageDesc()).GetMaster().GetFrameSize();
    m_pLeftMF->SetMax(rPageSize.GetWidth() - DEFAULT_LEFT_DISTANCE);
    m_pTopMF->SetMax(rPageSize.GetHeight() - DEFAULT_TOP_DISTANCE);
}

IMPL_LINK(SwMailMergeLayoutPage, ZoomHdl_Impl, ListBox&, rBox, void)
{
    if(m_pExampleWrtShell)
    {
        sal_Int16 eType = DocumentZoomType::BY_VALUE;
        short nZoom = 50;
        switch(rBox.GetSelectedEntryPos())
        {
            case 0 : eType = DocumentZoomType::ENTIRE_PAGE; break;
            case 1 : nZoom = 50; break;
            case 2 : nZoom = 75; break;
            case 3 : nZoom = 100; break;
        }
        Any aZoom;
        aZoom <<= eType;
        m_xViewProperties->setPropertyValue(UNO_NAME_ZOOM_TYPE, aZoom);
        aZoom <<= nZoom;
        m_xViewProperties->setPropertyValue(UNO_NAME_ZOOM_VALUE, aZoom);

    }
}

IMPL_LINK_NOARG(SwMailMergeLayoutPage, ChangeAddressLoseFocusHdl_Impl, Control&, void)
{
    ChangeAddressHdl_Impl(*m_pLeftMF);
}
IMPL_LINK_NOARG(SwMailMergeLayoutPage, ChangeAddressHdl_Impl, SpinField&, void)
{
    if(m_pExampleWrtShell && m_pAddressBlockFormat)
    {
        long nLeft = static_cast< long >(m_pLeftMF->Denormalize(m_pLeftMF->GetValue(FieldUnit::TWIP)));
        long nTop  = static_cast< long >(m_pTopMF->Denormalize(m_pTopMF->GetValue(FieldUnit::TWIP)));

        SfxItemSet aSet(
            m_pExampleWrtShell->GetAttrPool(),
            svl::Items<RES_VERT_ORIENT, RES_ANCHOR>{});
        if(m_pAlignToBodyCB->IsChecked())
            aSet.Put(SwFormatHoriOrient( 0, text::HoriOrientation::NONE, text::RelOrientation::PAGE_PRINT_AREA ));
        else
            aSet.Put(SwFormatHoriOrient( nLeft, text::HoriOrientation::NONE, text::RelOrientation::PAGE_FRAME ));
        aSet.Put(SwFormatVertOrient( nTop, text::VertOrientation::NONE, text::RelOrientation::PAGE_FRAME ));
        m_pExampleWrtShell->GetDoc()->SetFlyFrameAttr( *m_pAddressBlockFormat, aSet );
    }
}

IMPL_LINK(SwMailMergeLayoutPage, GreetingsHdl_Impl, Button*, pButton, void)
{
    bool bDown = pButton == m_pDownPB;
    bool bMoved = m_pExampleWrtShell->MoveParagraph( bDown ? 1 : -1 );
    if (bMoved || bDown)
        m_pWizard->GetConfigItem().MoveGreeting(bDown ? 1 : -1 );
    if(!bMoved && bDown)
    {
        //insert a new paragraph before the greeting line
        m_pExampleWrtShell->SplitNode();
    }
}

IMPL_LINK(SwMailMergeLayoutPage, AlignToTextHdl_Impl, Button*, pBox, void)
{
    bool bCheck = static_cast<CheckBox*>(pBox)->IsChecked() && pBox->IsEnabled();
    m_pLeftFT->Enable(!bCheck);
    m_pLeftMF->Enable(!bCheck);
    ChangeAddressHdl_Impl( *m_pLeftMF );
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
