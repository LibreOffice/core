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

#include <com/sun/star/beans/XPropertySet.hpp>

#include <DocumentRenderer.hxx>
#include <DocumentRenderer.hrc>
#include <ViewShellBase.hxx>

#include <drawdoc.hxx>
#include <sdpage.hxx>
#include <sdresid.hxx>
#include <strings.hrc>
#include <drawview.hxx>
#include <DrawViewShell.hxx>
#include <FrameView.hxx>
#include <Outliner.hxx>
#include <OutlineViewShell.hxx>
#include <SlideSorterViewShell.hxx>
#include <DrawDocShell.hxx>

#include <tools/multisel.hxx>
#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <comphelper/propertyvalue.hxx>
#include <comphelper/sequence.hxx>
#include <rtl/ustrbuf.hxx>
#include <editeng/editstat.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdetc.hxx>
#include <svx/svditer.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdopath.hxx>
#include <svx/svdpagv.hxx>
#include <svx/xlineit0.hxx>
#include <svx/xlnclit.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <unotools/localedatawrapper.hxx>
#include <utility>
#include <vcl/print.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <unotools/moduleoptions.hxx>
#include <xmloff/autolayout.hxx>
#include <sfx2/objsh.hxx>

#include <officecfg/Office/Draw.hxx>
#include <officecfg/Office/Impress.hxx>

#include <algorithm>
#include <memory>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;

namespace sd {

namespace {

    /** Convenience class to extract values from the sequence of properties
        given to one of the XRenderable methods.
    */
    class PrintOptions
    {
    public:
        PrintOptions (
            const vcl::PrinterOptionsHelper& rHelper,
            std::vector<sal_Int32>&& rSlidesPerPage)
            : mrProperties(rHelper),
              maSlidesPerPage(std::move(rSlidesPerPage))
        {
        }

        bool IsWarningOrientation() const
        {
            return GetBoolValue(nullptr, true);
        }

        bool IsPrintPageName() const
        {
            return GetBoolValue("IsPrintName", false);
        }

        bool IsDate() const
        {
            return GetBoolValue("IsPrintDateTime", false);
        }

        bool IsTime() const
        {
            return GetBoolValue("IsPrintDateTime", false);
        }

        bool IsHiddenPages() const
        {
            return GetBoolValue("IsPrintHidden", false);
        }

        bool IsHandoutHorizontal() const
        {
            return GetBoolValue("SlidesPerPageOrder", sal_Int32(0));
        }

        sal_Int32 GetHandoutPageCount() const
        {
            sal_uInt32 nIndex = static_cast<sal_Int32>(mrProperties.getIntValue("SlidesPerPage", sal_Int32(0)));
            if (nIndex<maSlidesPerPage.size())
                return maSlidesPerPage[nIndex];
            else if ( ! maSlidesPerPage.empty())
                return maSlidesPerPage[0];
            else
                return 0;
        }

        bool IsDraw() const
        {
            return GetBoolValue("PageContentType", sal_Int32(0));
        }

        bool IsHandout() const
        {
            return GetBoolValue("PageContentType", sal_Int32(1));
        }

        bool IsNotes() const
        {
            return GetBoolValue("PageContentType", sal_Int32(2));
        }

        bool IsOutline() const
        {
            return GetBoolValue("PageContentType", sal_Int32(3));
        }

        sal_uLong GetOutputQuality() const
        {
            sal_Int32 nQuality = static_cast<sal_Int32>(mrProperties.getIntValue( "Quality", sal_Int32(0) ));
            return nQuality;
        }

        bool IsPageSize() const
        {
            return GetBoolValue("PageOptions", sal_Int32(1));
        }

        bool IsTilePage() const
        {
            return GetBoolValue("PageOptions", sal_Int32(2)) || GetBoolValue("PageOptions", sal_Int32(3));
        }

        bool IsCutPage() const
        {
            return GetBoolValue("PageOptions", sal_Int32(0));
        }

        bool IsBooklet() const
        {
            return GetBoolValue("PrintProspect", false);
        }

        bool IsPrinterPreferred(DocumentType eDocType) const
        {
            bool bIsDraw = eDocType == DocumentType::Draw;
            return IsTilePage() || IsPageSize() || IsBooklet() || (!bIsDraw && !IsNotes());
        }

        bool IsPrintExcluded() const
        {
            return (IsNotes() || IsDraw() || IsHandout()) &&  IsHiddenPages();
        }

        bool IsPrintFrontPage() const
        {
            sal_Int32 nInclude = static_cast<sal_Int32>(mrProperties.getIntValue( "EvenOdd", 0 ));
            return nInclude != 2;
        }

        bool IsPrintBackPage() const
        {
            sal_Int32 nInclude = static_cast<sal_Int32>(mrProperties.getIntValue( "EvenOdd", 0 ));
            return nInclude != 1;
        }

        bool IsPaperBin() const
        {
            return GetBoolValue("PrintPaperFromSetup", false);
        }

        bool IsPrintMarkedOnly() const
        {
            return GetBoolValue("PrintContent", sal_Int32(4));
        }

        OUString GetPrinterSelection (sal_Int32 nPageCount, sal_Int32 nCurrentPageIndex) const
        {
            sal_Int32 nContent = static_cast<sal_Int32>(mrProperties.getIntValue( "PrintContent", 0 ));
            OUString sFullRange = "1-" + OUString::number(nPageCount);

            if (nContent == 0) // all pages/slides
            {
                return sFullRange;
            }

            if (nContent == 1) // range
            {
                OUString sValue = mrProperties.getStringValue("PageRange");
                return sValue.isEmpty() ? sFullRange : sValue;
            }

            if (nContent == 2 && // selection
                nCurrentPageIndex >= 0)
            {
                return OUString::number(nCurrentPageIndex + 1);
            }

            return OUString();
        }

    private:
        const vcl::PrinterOptionsHelper& mrProperties;
        const std::vector<sal_Int32> maSlidesPerPage;

        /** When the value of the property with name pName is a boolean then
            return its value. When the property is unknown then
            bDefaultValue is returned.  Otherwise <FALSE/> is returned.
        */
        bool GetBoolValue (
            const char* pName,
            const bool bDefaultValue) const
        {
            bool bValue = mrProperties.getBoolValue( pName, bDefaultValue );
            return bValue;
        }

        /** Return <TRUE/> when the value of the property with name pName is
            an integer and its value is nTriggerValue. Otherwise <FALSE/> is
            returned.
        */
        bool GetBoolValue (
            const char* pName,
            const sal_Int32 nTriggerValue) const
        {
            sal_Int32 nValue = static_cast<sal_Int32>(mrProperties.getIntValue( pName, 0 ));
            return nValue == nTriggerValue;
        }
    };

    /** A collection of values that helps to reduce the number of arguments
        given to some functions.  Note that not all values are set at the
        same time.
    */
    class PrintInfo
    {
    public:
        PrintInfo (
            Printer* pPrinter,
            const bool bPrintMarkedOnly)
            : mpPrinter(pPrinter),
              mnDrawMode(DrawModeFlags::Default),
              maPrintSize(0,0),
              maPageSize(0,0),
              meOrientation(Orientation::Portrait),
              mbPrintMarkedOnly(bPrintMarkedOnly)
        {}

        const VclPtr<Printer> mpPrinter;
        DrawModeFlags mnDrawMode;
        OUString msTimeDate;
        OUString msPageString;
        Size maPrintSize;
        Size maPageSize;
        Orientation meOrientation;
        MapMode maMap;
        const bool mbPrintMarkedOnly;
    };

    /** Output one page of the document to the given printer.  Note that
        more than one document page may be output to one printer page.
    */
    void PrintPage (
        Printer& rPrinter,
        ::sd::View& rPrintView,
        SdPage& rPage,
        View const * pView,
        const bool bPrintMarkedOnly,
        const SdrLayerIDSet& rVisibleLayers,
        const SdrLayerIDSet& rPrintableLayers)
    {
        rPrintView.ShowSdrPage(&rPage);

        const MapMode aOriginalMapMode (rPrinter.GetMapMode());

        // Set the visible layers
        SdrPageView* pPageView = rPrintView.GetSdrPageView();
        OSL_ASSERT(pPageView!=nullptr);
        pPageView->SetVisibleLayers(rVisibleLayers);
        pPageView->SetPrintableLayers(rPrintableLayers);

        if (pView!=nullptr && bPrintMarkedOnly)
            pView->DrawMarkedObj(rPrinter);
        else
            rPrintView.CompleteRedraw(&rPrinter,
                    vcl::Region(::tools::Rectangle(Point(0,0), rPage.GetSize())));

        rPrinter.SetMapMode(aOriginalMapMode);

        rPrintView.HideSdrPage();
    }

    /** Output a string (that typically is not part of a document page) to
        the given printer.
    */
    void PrintMessage (
        Printer& rPrinter,
        const OUString& rsPageString,
        const Point& rPageStringOffset)
    {
        const vcl::Font aOriginalFont (rPrinter.OutputDevice::GetFont());
        rPrinter.SetFont(vcl::Font(FAMILY_SWISS, Size(0, 423)));
        rPrinter.DrawText(rPageStringOffset, rsPageString);
        rPrinter.SetFont(aOriginalFont);
    }

    /** Read the resources and process then into a sequence of properties
        that can be passed to the printing dialog.
    */
    class DialogCreator
    {
    public:
        DialogCreator (ViewShellBase &rBase, bool bImpress, sal_Int32 nCurPage)
            : mrBase(rBase)
            , mbImpress(bImpress)
            , mnCurPage(nCurPage)
        {
            ProcessResource();
        }

        const std::vector< beans::PropertyValue >& GetDialogControls() const
        {
            return maProperties;
        }

        const std::vector<sal_Int32>& GetSlidesPerPage() const
        {
            return maSlidesPerPage;
        }

    private:
        ViewShellBase &mrBase;
        std::vector<beans::PropertyValue> maProperties;
        std::vector<sal_Int32> maSlidesPerPage;
        bool mbImpress;
        sal_Int32 mnCurPage;

        void ProcessResource()
        {
            // load the writer PrinterOptions into the custom tab
            beans::PropertyValue aOptionsUIFile;
            aOptionsUIFile.Name = "OptionsUIFile";
            if( mbImpress )
                aOptionsUIFile.Value <<= u"modules/simpress/ui/impressprinteroptions.ui"_ustr;
            else
                aOptionsUIFile.Value <<= u"modules/sdraw/ui/drawprinteroptions.ui"_ustr;
            maProperties.push_back(aOptionsUIFile);

            SvtModuleOptions aOpt;
            OUString aAppGroupname(SdResId(STR_IMPRESS_PRINT_UI_GROUP_NAME));
            aAppGroupname = aAppGroupname.replaceFirst("%s", aOpt.GetModuleName(
                mbImpress ? SvtModuleOptions::EModule::IMPRESS : SvtModuleOptions::EModule::DRAW));
            AddDialogControl(vcl::PrinterOptionsHelper::setGroupControlOpt(u"tabcontrol-page2"_ustr, aAppGroupname, u".HelpID:vcl:PrintDialog:TabPage:AppPage"_ustr));

            uno::Sequence< OUString > aHelpIds, aWidgetIds;
            if( mbImpress )
            {
                aHelpIds = { u".HelpID:vcl:PrintDialog:PageContentType:ListBox"_ustr };
                AddDialogControl( vcl::PrinterOptionsHelper::setChoiceListControlOpt(
                                    u"impressdocument"_ustr,
                                    SdResId(STR_IMPRESS_PRINT_UI_CONTENT),
                                    aHelpIds,
                                    u"PageContentType"_ustr ,
                                    CreateChoice(STR_IMPRESS_PRINT_UI_CONTENT_CHOICES, SAL_N_ELEMENTS(STR_IMPRESS_PRINT_UI_CONTENT_CHOICES)),
                                    0)
                                );

                aHelpIds = { u".HelpID:vcl:PrintDialog:SlidesPerPage:ListBox"_ustr };
                vcl::PrinterOptionsHelper::UIControlOptions aContentOpt( u"PageContentType"_ustr , 1 );
                AddDialogControl( vcl::PrinterOptionsHelper::setChoiceListControlOpt(
                                    u"slidesperpage"_ustr,
                                    SdResId(STR_IMPRESS_PRINT_UI_SLIDESPERPAGE),
                                    aHelpIds,
                                    u"SlidesPerPage"_ustr ,
                                    GetSlidesPerPageSequence(),
                                    0,
                                    Sequence< sal_Bool >(),
                                    aContentOpt
                                    )
                                );

                aHelpIds = { u".HelpID:vcl:PrintDialog:SlidesPerPageOrder:ListBox"_ustr };
                vcl::PrinterOptionsHelper::UIControlOptions aSlidesPerPageOpt( u"SlidesPerPage"_ustr , -1, true );
                AddDialogControl( vcl::PrinterOptionsHelper::setChoiceListControlOpt(
                                    u"slidesperpageorder"_ustr,
                                    SdResId(STR_IMPRESS_PRINT_UI_ORDER),
                                    aHelpIds,
                                    u"SlidesPerPageOrder"_ustr ,
                                    CreateChoice(STR_IMPRESS_PRINT_UI_ORDER_CHOICES, SAL_N_ELEMENTS(STR_IMPRESS_PRINT_UI_ORDER_CHOICES)),
                                    0,
                                    Sequence< sal_Bool >(),
                                    aSlidesPerPageOpt )
                                );
            }

            AddDialogControl( vcl::PrinterOptionsHelper::setSubgroupControlOpt(u"contents"_ustr,
                               SdResId(STR_IMPRESS_PRINT_UI_INCLUDE_CONTENT), u""_ustr ) );

            if( mbImpress )
            {
                AddDialogControl( vcl::PrinterOptionsHelper::setBoolControlOpt(u"printname"_ustr,
                                    SdResId(STR_IMPRESS_PRINT_UI_IS_PRINT_NAME),
                                    u".HelpID:vcl:PrintDialog:IsPrintName:CheckBox"_ustr ,
                                    u"IsPrintName"_ustr ,
                                    officecfg::Office::Impress::Print::Other::PageName::get()
                                    )
                                );
            }
            else
            {
                AddDialogControl( vcl::PrinterOptionsHelper::setBoolControlOpt(u"printname"_ustr,
                                    SdResId(STR_DRAW_PRINT_UI_IS_PRINT_NAME),
                                    u".HelpID:vcl:PrintDialog:IsPrintName:CheckBox"_ustr ,
                                    u"IsPrintName"_ustr ,
                                    officecfg::Office::Draw::Print::Other::PageName::get()
                                    )
                                );
            }

            AddDialogControl( vcl::PrinterOptionsHelper::setBoolControlOpt(u"printdatetime"_ustr,
                                SdResId(STR_IMPRESS_PRINT_UI_IS_PRINT_DATE),
                                u".HelpID:vcl:PrintDialog:IsPrintDateTime:CheckBox"_ustr ,
                                u"IsPrintDateTime"_ustr ,
                                // Separate settings for time and date in Impress/Draw -> Print page, check that both are set
                                mbImpress ?
                                officecfg::Office::Impress::Print::Other::Date::get() &&
                                officecfg::Office::Impress::Print::Other::Time::get() :
                                officecfg::Office::Draw::Print::Other::Date::get() &&
                                officecfg::Office::Draw::Print::Other::Time::get()
                                )
                            );

            if( mbImpress )
            {
                AddDialogControl( vcl::PrinterOptionsHelper::setBoolControlOpt(u"printhidden"_ustr,
                                    SdResId(STR_IMPRESS_PRINT_UI_IS_PRINT_HIDDEN),
                                    u".HelpID:vcl:PrintDialog:IsPrintHidden:CheckBox"_ustr ,
                                    u"IsPrintHidden"_ustr ,
                                    officecfg::Office::Impress::Print::Other::HiddenPage::get()
                                    )
                                );
            }

            AddDialogControl( vcl::PrinterOptionsHelper::setSubgroupControlOpt(u"color"_ustr,
                               SdResId(STR_IMPRESS_PRINT_UI_QUALITY), u""_ustr ) );

            aHelpIds = { u".HelpID:vcl:PrintDialog:Quality:RadioButton:0"_ustr,
                         u".HelpID:vcl:PrintDialog:Quality:RadioButton:1"_ustr,
                         u".HelpID:vcl:PrintDialog:Quality:RadioButton:2"_ustr };
            aWidgetIds = { u"originalcolors"_ustr, u"grayscale"_ustr, u"blackandwhite"_ustr };
            AddDialogControl( vcl::PrinterOptionsHelper::setChoiceRadiosControlOpt(
                                aWidgetIds,
                                u""_ustr,
                                aHelpIds,
                                u"Quality"_ustr ,
                                CreateChoice(STR_IMPRESS_PRINT_UI_QUALITY_CHOICES, SAL_N_ELEMENTS(STR_IMPRESS_PRINT_UI_QUALITY_CHOICES)),
                                mbImpress ? officecfg::Office::Impress::Print::Other::Quality::get() :
                                            officecfg::Office::Draw::Print::Other::Quality::get() )

                            );

            AddDialogControl( vcl::PrinterOptionsHelper::setSubgroupControlOpt(u"pagesizes"_ustr,
                               SdResId(STR_IMPRESS_PRINT_UI_PAGE_OPTIONS), u""_ustr ) );

            aHelpIds = { u".HelpID:vcl:PrintDialog:PageOptions:RadioButton:0"_ustr,
                         u".HelpID:vcl:PrintDialog:PageOptions:RadioButton:1"_ustr,
                         u".HelpID:vcl:PrintDialog:PageOptions:RadioButton:2"_ustr,
                         u".HelpID:vcl:PrintDialog:PageOptions:RadioButton:3"_ustr };
            aWidgetIds = { u"originalsize"_ustr, u"fittoprintable"_ustr, u"distributeonmultiple"_ustr, u"tilesheet"_ustr };

            // Mutually exclusive page options settings are stored in separate config keys...
            // TODO: There is no config key to set the distributeonmultiple option as default
            sal_Int32 nDefaultChoice = 0;
            if ( mbImpress ? officecfg::Office::Impress::Print::Page::PageSize::get() :
                             officecfg::Office::Draw::Print::Page::PageSize::get() )
            {
                nDefaultChoice = 1;
            }
            else if ( mbImpress ? officecfg::Office::Impress::Print::Page::PageTile::get() :
                                  officecfg::Office::Draw::Print::Page::PageTile::get() )
            {
                nDefaultChoice = 3;
            }
            vcl::PrinterOptionsHelper::UIControlOptions aPageOptionsOpt(u"PrintProspect"_ustr, 0);
            AddDialogControl( vcl::PrinterOptionsHelper::setChoiceRadiosControlOpt(
                                aWidgetIds,
                                u""_ustr,
                                aHelpIds,
                                u"PageOptions"_ustr ,
                                mbImpress ? CreateChoice(STR_IMPRESS_PRINT_UI_PAGE_OPTIONS_CHOICES, SAL_N_ELEMENTS(STR_IMPRESS_PRINT_UI_PAGE_OPTIONS_CHOICES)) :
                                            CreateChoice(STR_IMPRESS_PRINT_UI_PAGE_OPTIONS_CHOICES_DRAW, SAL_N_ELEMENTS(STR_IMPRESS_PRINT_UI_PAGE_OPTIONS_CHOICES_DRAW)),
                                nDefaultChoice,
                                Sequence< sal_Bool >(),
                                aPageOptionsOpt
                                )
                            );

            vcl::PrinterOptionsHelper::UIControlOptions aBrochureOpt;
            aBrochureOpt.maGroupHint = "LayoutPage" ;
            AddDialogControl( vcl::PrinterOptionsHelper::setSubgroupControlOpt(u"pagesides"_ustr,
                               SdResId(STR_IMPRESS_PRINT_UI_PAGE_SIDES), u""_ustr,
                               aBrochureOpt ) );

            // brochure printing
            AddDialogControl( vcl::PrinterOptionsHelper::setBoolControlOpt(u"brochure"_ustr,
                                SdResId(STR_IMPRESS_PRINT_UI_BROCHURE),
                                u".HelpID:vcl:PrintDialog:PrintProspect:CheckBox"_ustr ,
                                u"PrintProspect"_ustr ,
                                mbImpress ? officecfg::Office::Impress::Print::Page::Booklet::get() :
                                            officecfg::Office::Draw::Print::Page::Booklet::get(),
                                aBrochureOpt
                                )
                            );

            vcl::PrinterOptionsHelper::UIControlOptions
                aIncludeOpt( u"PrintProspect"_ustr , -1, false );
            aIncludeOpt.maGroupHint =  "LayoutPage" ;
            aHelpIds = { u".HelpID:vcl:PrintDialog:PrintProspectInclude:ListBox"_ustr };
            AddDialogControl( vcl::PrinterOptionsHelper::setChoiceListControlOpt(
                                u"brochureinclude"_ustr,
                                SdResId(STR_IMPRESS_PRINT_UI_BROCHURE_INCLUDE),
                                aHelpIds,
                                u"PrintProspectInclude"_ustr ,
                                CreateChoice(STR_IMPRESS_PRINT_UI_BROCHURE_INCLUDE_LIST, SAL_N_ELEMENTS(STR_IMPRESS_PRINT_UI_BROCHURE_INCLUDE_LIST)),
                                0,
                                Sequence< sal_Bool >(),
                                aIncludeOpt
                                )
                            );

            // paper tray (on options page)
            vcl::PrinterOptionsHelper::UIControlOptions aPaperTrayOpt;
            aPaperTrayOpt.maGroupHint = "OptionsPageOptGroup" ;
            AddDialogControl( vcl::PrinterOptionsHelper::setBoolControlOpt(u"printpaperfromsetup"_ustr,
                                SdResId(STR_IMPRESS_PRINT_UI_PAPER_TRAY),
                                u".HelpID:vcl:PrintDialog:PrintPaperFromSetup:CheckBox"_ustr ,
                                u"PrintPaperFromSetup"_ustr ,
                                false,
                                aPaperTrayOpt
                                )
                            );
            // print range selection
            vcl::PrinterOptionsHelper::UIControlOptions aPrintRangeOpt;
            aPrintRangeOpt.mbInternalOnly = true;
            aPrintRangeOpt.maGroupHint = "PrintRange" ;
            AddDialogControl( vcl::PrinterOptionsHelper::setSubgroupControlOpt(u"printrange"_ustr,
                                mbImpress ? SdResId(STR_IMPRESS_PRINT_UI_SLIDE_RANGE) : SdResId(STR_IMPRESS_PRINT_UI_PAGE_RANGE),
                                u""_ustr,
                                aPrintRangeOpt )
                             );

            // check if there is a selection of slides
            OUString aPageRange(OUString::number(mnCurPage + 1));
            int nPrintRange(0);
            using sd::slidesorter::SlideSorterViewShell;
            SlideSorterViewShell* const pSSViewSh(SlideSorterViewShell::GetSlideSorter(mrBase));
            if (pSSViewSh)
            {
                const std::shared_ptr<SlideSorterViewShell::PageSelection> pPageSelection(pSSViewSh->GetPageSelection());
                if (bool(pPageSelection) && pPageSelection->size() > 1)
                {
                    OUStringBuffer aBuf;
                    // TODO: this could be improved by writing ranges instead of consecutive page
                    // numbers if appropriate. Do we have a helper function for that somewhere?
                    bool bFirst(true);
                    for (auto pPage: *pPageSelection)
                    {
                        if (bFirst)
                            bFirst = false;
                        else
                            aBuf.append(',');
                        aBuf.append(static_cast<sal_Int32>(pPage->GetPageNum() / 2 + 1));
                    }
                    aPageRange = aBuf.makeStringAndClear();
                    nPrintRange = 1;
                }
            }
/*
            OUString aPrintRangeName( "PrintContent" );
            aHelpIds.realloc( 1 );
            aHelpIds[0] = ".HelpID:vcl:PrintDialog:PageContentType:ListBox";
            AddDialogControl( vcl::PrinterOptionsHelper::setChoiceListControlOpt( "printpagesbox", OUString(),
                                aHelpIds, aPrintRangeName,
                                mbImpress ? CreateChoice( STR_IMPRESS_PRINT_UI_PAGE_RANGE_CHOICE, SAL_N_ELEMENTS(STR_IMPRESS_PRINT_UI_PAGE_RANGE_CHOICE ) ) :
                                            CreateChoice( STR_DRAW_PRINT_UI_PAGE_RANGE_CHOICE, SAL_N_ELEMENTS(STR_DRAW_PRINT_UI_PAGE_RANGE_CHOICE ) ),
                                nPrintRange ) );
*/
            OUString aPrintRangeName( u"PrintContent"_ustr );
            aHelpIds = { u".HelpID:vcl:PrintDialog:PrintContent:RadioButton:0"_ustr,
                         u".HelpID:vcl:PrintDialog:PrintContent:RadioButton:1"_ustr,
                         u".HelpID:vcl:PrintDialog:PrintContent:RadioButton:2"_ustr };
            aWidgetIds = { u"rbAllPages"_ustr, u"rbRangePages"_ustr, u"rbRangeSelection"_ustr };

            AddDialogControl( vcl::PrinterOptionsHelper::setChoiceRadiosControlOpt(aWidgetIds, OUString(),
                                aHelpIds, aPrintRangeName,
                                mbImpress ? CreateChoice(STR_IMPRESS_PRINT_UI_PAGE_RANGE_CHOICE, SAL_N_ELEMENTS(STR_IMPRESS_PRINT_UI_PAGE_RANGE_CHOICE)) :
                                            CreateChoice(STR_DRAW_PRINT_UI_PAGE_RANGE_CHOICE, SAL_N_ELEMENTS(STR_DRAW_PRINT_UI_PAGE_RANGE_CHOICE)),
                                nPrintRange )
                            );
            // create an Edit dependent on "Pages" selected
            vcl::PrinterOptionsHelper::UIControlOptions aPageRangeOpt( aPrintRangeName, 1, true );
            AddDialogControl(vcl::PrinterOptionsHelper::setEditControlOpt(u"pagerange"_ustr, u""_ustr,
                                u".HelpID:vcl:PrintDialog:PageRange:Edit"_ustr, u"PageRange"_ustr,
                                aPageRange, aPageRangeOpt));
            vcl::PrinterOptionsHelper::UIControlOptions aEvenOddOpt(aPrintRangeName, -1, true);
            AddDialogControl(vcl::PrinterOptionsHelper::setChoiceListControlOpt(u"evenoddbox"_ustr, u""_ustr,
                                uno::Sequence<OUString>(), u"EvenOdd"_ustr, uno::Sequence<OUString>(),
                                0, uno::Sequence<sal_Bool>(), aEvenOddOpt));
        }

        void AddDialogControl( const Any& i_rCtrl )
        {
            beans::PropertyValue aVal;
            aVal.Value = i_rCtrl;
            maProperties.push_back( aVal );
        }

        static Sequence<OUString> CreateChoice(const TranslateId* pResourceId, size_t nCount)
        {
            Sequence<OUString> aChoices (nCount);
            std::transform(pResourceId, pResourceId + nCount, aChoices.getArray(),
                           [](const auto& id) { return SdResId(id); });
            return aChoices;
        }

        Sequence<OUString> GetSlidesPerPageSequence()
        {
            const Sequence<OUString> aChoice (
                CreateChoice(STR_IMPRESS_PRINT_UI_SLIDESPERPAGE_CHOICES, SAL_N_ELEMENTS(STR_IMPRESS_PRINT_UI_SLIDESPERPAGE_CHOICES)));
            maSlidesPerPage.clear();
            maSlidesPerPage.push_back(0); // first is using the default
            std::transform(std::next(aChoice.begin()), aChoice.end(), std::back_inserter(maSlidesPerPage),
                            [](const OUString& rChoice) -> sal_Int32 { return rChoice.toInt32(); });
            return aChoice;
        }
    };

    /** The Prepare... methods of the DocumentRenderer::Implementation class
        create a set of PrinterPage objects that contain all necessary
        information to do the actual printing.  There is one PrinterPage
        object per printed page.  Derived classes implement the actual, mode
        specific printing.

        This and all derived classes support the asynchronous printing
        process by not storing pointers to any data with lifetime shorter
        than the PrinterPage objects, i.e. slides, shapes, (one of) the
        outliner (of the document).
    */
    class PrinterPage
    {
    public:
        PrinterPage (
            const PageKind ePageKind,
            const MapMode& rMapMode,
            const bool bPrintMarkedOnly,
            OUString sPageString,
            const Point& rPageStringOffset,
            const DrawModeFlags nDrawMode,
            const Orientation eOrientation,
            const sal_uInt16 nPaperTray)
            : mePageKind(ePageKind),
              maMap(rMapMode),
              mbPrintMarkedOnly(bPrintMarkedOnly),
              msPageString(std::move(sPageString)),
              maPageStringOffset(rPageStringOffset),
              mnDrawMode(nDrawMode),
              meOrientation(eOrientation),
              mnPaperTray(nPaperTray)
        {
        }

        virtual ~PrinterPage() {}

        virtual void Print (
            Printer& rPrinter,
            SdDrawDocument& rDocument,
            ViewShell& rViewShell,
            View* pView,
            DrawView& rPrintView,
            const SdrLayerIDSet& rVisibleLayers,
            const SdrLayerIDSet& rPrintableLayers) const = 0;

        DrawModeFlags GetDrawMode() const { return mnDrawMode; }
        Orientation GetOrientation() const { return meOrientation; }
        sal_uInt16 GetPaperTray() const { return mnPaperTray; }

    protected:
        const PageKind mePageKind;
        const MapMode maMap;
        const bool mbPrintMarkedOnly;
        const OUString msPageString;
        const Point maPageStringOffset;
        const DrawModeFlags mnDrawMode;
        const Orientation meOrientation;
        const sal_uInt16 mnPaperTray;
    };

    /** The RegularPrinterPage is used for printing one regular slide (no
        notes, handout, or outline) to one printer page.
    */
    class RegularPrinterPage : public PrinterPage
    {
    public:
        RegularPrinterPage (
            const sal_uInt16 nPageIndex,
            const PageKind ePageKind,
            const MapMode& rMapMode,
            const bool bPrintMarkedOnly,
            const OUString& rsPageString,
            const Point& rPageStringOffset,
            const DrawModeFlags nDrawMode,
            const Orientation eOrientation,
            const sal_uInt16 nPaperTray)
            : PrinterPage(ePageKind, rMapMode, bPrintMarkedOnly, rsPageString,
                rPageStringOffset, nDrawMode, eOrientation, nPaperTray),
              mnPageIndex(nPageIndex)
        {
        }

        virtual void Print (
            Printer& rPrinter,
            SdDrawDocument& rDocument,
            ViewShell&,
            View* pView,
            DrawView& rPrintView,
            const SdrLayerIDSet& rVisibleLayers,
            const SdrLayerIDSet& rPrintableLayers) const override
        {
            SdPage* pPageToPrint = rDocument.GetSdPage(mnPageIndex, mePageKind);
            rPrinter.SetMapMode(maMap);
            PrintPage(
                rPrinter,
                rPrintView,
                *pPageToPrint,
                pView,
                mbPrintMarkedOnly,
                rVisibleLayers,
                rPrintableLayers);
            PrintMessage(
                rPrinter,
                msPageString,
                maPageStringOffset);
        }

    private:
        const sal_uInt16 mnPageIndex;
    };

    /** Print one slide multiple times on a printer page so that the whole
        printer page is covered.
    */
    class TiledPrinterPage : public PrinterPage
    {
    public:
        TiledPrinterPage (
            const sal_uInt16 nPageIndex,
            const PageKind ePageKind,
            const bool bPrintMarkedOnly,
            const OUString& rsPageString,
            const Point& rPageStringOffset,
            const DrawModeFlags nDrawMode,
            const Orientation eOrientation,
            const sal_uInt16 nPaperTray)
            : PrinterPage(ePageKind, MapMode(), bPrintMarkedOnly, rsPageString,
                rPageStringOffset, nDrawMode, eOrientation, nPaperTray),
              mnPageIndex(nPageIndex)
        {
        }

        virtual void Print (
            Printer& rPrinter,
            SdDrawDocument& rDocument,
            ViewShell&,
            View* pView,
            DrawView& rPrintView,
            const SdrLayerIDSet& rVisibleLayers,
            const SdrLayerIDSet& rPrintableLayers) const override
        {
            SdPage* pPageToPrint = rDocument.GetSdPage(mnPageIndex, mePageKind);
            if (pPageToPrint==nullptr)
                return;
            MapMode aMap (rPrinter.GetMapMode());

            const Size aPageSize (pPageToPrint->GetSize());
            const Size aPrintSize (rPrinter.GetOutputSize());

            const sal_Int32 nPageWidth (aPageSize.Width() + mnGap
                - pPageToPrint->GetLeftBorder() - pPageToPrint->GetRightBorder());
            const sal_Int32 nPageHeight (aPageSize.Height() + mnGap
                - pPageToPrint->GetUpperBorder() - pPageToPrint->GetLowerBorder());
            if (nPageWidth<=0 || nPageHeight<=0)
                return;

            // Print at least two rows and columns.  More if the document
            // page fits completely onto the printer page.
            const sal_Int32 nColumnCount (std::max(sal_Int32(2),
                    sal_Int32(aPrintSize.Width() / nPageWidth)));
            const sal_Int32 nRowCount (std::max(sal_Int32(2),
                    sal_Int32(aPrintSize.Height() / nPageHeight)));
            for (sal_Int32 nRow=0; nRow<nRowCount; ++nRow)
                for (sal_Int32 nColumn=0; nColumn<nColumnCount; ++nColumn)
                {
                    aMap.SetOrigin(Point(nColumn*nPageWidth,nRow*nPageHeight));
                    rPrinter.SetMapMode(aMap);
                    PrintPage(
                        rPrinter,
                        rPrintView,
                        *pPageToPrint,
                        pView,
                        mbPrintMarkedOnly,
                        rVisibleLayers,
                        rPrintableLayers);
                }

            PrintMessage(
                rPrinter,
                msPageString,
                maPageStringOffset);
        }

    private:
        const sal_uInt16 mnPageIndex;
        static const sal_Int32 mnGap = 500;
    };

    /** Print two slides to one printer page so that the resulting pages
        form a booklet.
    */
    class BookletPrinterPage : public PrinterPage
    {
    public:
        BookletPrinterPage (
            const sal_uInt16 nFirstPageIndex,
            const sal_uInt16 nSecondPageIndex,
            const Point& rFirstOffset,
            const Point& rSecondOffset,
            const PageKind ePageKind,
            const MapMode& rMapMode,
            const bool bPrintMarkedOnly,
            const DrawModeFlags nDrawMode,
            const Orientation eOrientation,
            const sal_uInt16 nPaperTray)
            : PrinterPage(ePageKind, rMapMode, bPrintMarkedOnly, u""_ustr,
                Point(), nDrawMode, eOrientation, nPaperTray),
              mnFirstPageIndex(nFirstPageIndex),
              mnSecondPageIndex(nSecondPageIndex),
              maFirstOffset(rFirstOffset),
              maSecondOffset(rSecondOffset)
        {
        }

        virtual void Print (
            Printer& rPrinter,
            SdDrawDocument& rDocument,
            ViewShell&,
            View* pView,
            DrawView& rPrintView,
            const SdrLayerIDSet& rVisibleLayers,
            const SdrLayerIDSet& rPrintableLayers) const override
        {
            MapMode aMap (maMap);
            SdPage* pPageToPrint = rDocument.GetSdPage(mnFirstPageIndex, mePageKind);
            if (pPageToPrint)
            {
                aMap.SetOrigin(maFirstOffset);
                rPrinter.SetMapMode(aMap);
                PrintPage(
                    rPrinter,
                    rPrintView,
                    *pPageToPrint,
                    pView,
                    mbPrintMarkedOnly,
                    rVisibleLayers,
                    rPrintableLayers);
            }

            pPageToPrint = rDocument.GetSdPage(mnSecondPageIndex, mePageKind);
            if( !pPageToPrint )
                return;

            aMap.SetOrigin(maSecondOffset);
            rPrinter.SetMapMode(aMap);
            PrintPage(
                rPrinter,
                rPrintView,
                *pPageToPrint,
                pView,
                mbPrintMarkedOnly,
                rVisibleLayers,
                rPrintableLayers);
        }

    private:
        const sal_uInt16 mnFirstPageIndex;
        const sal_uInt16 mnSecondPageIndex;
        const Point maFirstOffset;
        const Point maSecondOffset;
    };

    /** One handout page displays one to nine slides.
    */
    class HandoutPrinterPage : public PrinterPage
    {
    public:
        HandoutPrinterPage (
            const sal_uInt16 nHandoutPageIndex,
            std::vector<sal_uInt16>&& rPageIndices,
            const MapMode& rMapMode,
            const OUString& rsPageString,
            const Point& rPageStringOffset,
            const DrawModeFlags nDrawMode,
            const Orientation eOrientation,
            const sal_uInt16 nPaperTray)
            : PrinterPage(PageKind::Handout, rMapMode, false, rsPageString,
                rPageStringOffset, nDrawMode, eOrientation, nPaperTray),
              mnHandoutPageIndex(nHandoutPageIndex),
              maPageIndices(std::move(rPageIndices))
        {
        }

        virtual void Print (
            Printer& rPrinter,
            SdDrawDocument& rDocument,
            ViewShell& rViewShell,
            View* pView,
            DrawView& rPrintView,
            const SdrLayerIDSet& rVisibleLayers,
            const SdrLayerIDSet& rPrintableLayers) const override
        {
            SdPage& rHandoutPage (*rDocument.GetSdPage(0, PageKind::Handout));

            Reference< css::beans::XPropertySet > xHandoutPage( rHandoutPage.getUnoPage(), UNO_QUERY );
            static constexpr OUString sPageNumber( u"Number"_ustr );

            // Collect the page objects of the handout master.
            std::vector<SdrPageObj*> aHandoutPageObjects;
            SdrObjListIter aShapeIter (&rHandoutPage);
            while (aShapeIter.IsMore())
            {
                SdrPageObj* pPageObj = dynamic_cast<SdrPageObj*>(aShapeIter.Next());
                if (pPageObj)
                    aHandoutPageObjects.push_back(pPageObj);
            }
            if (aHandoutPageObjects.empty())
                return;

            // Connect page objects with pages.
            std::vector<SdrPageObj*>::iterator aPageObjIter (aHandoutPageObjects.begin());
            for (std::vector<sal_uInt16>::const_iterator
                     iPageIndex(maPageIndices.begin()),
                     iEnd(maPageIndices.end());
                 iPageIndex!=iEnd && aPageObjIter!=aHandoutPageObjects.end();
                 ++iPageIndex)
            {
                // Check if the page still exists.
                if (*iPageIndex >= rDocument.GetSdPageCount(PageKind::Standard))
                    continue;

                SdrPageObj* pPageObj = *aPageObjIter++;
                pPageObj->SetReferencedPage(rDocument.GetSdPage(*iPageIndex, PageKind::Standard));
            }

            // if there are more page objects than pages left, set the rest to invisible
            int nHangoverCount = 0;
            while (aPageObjIter != aHandoutPageObjects.end())
            {
                (*aPageObjIter++)->SetReferencedPage(nullptr);
                nHangoverCount++;
            }

            // Hide outlines for objects that have pages attached.
            if (nHangoverCount > 0)
            {
                int nSkip = aHandoutPageObjects.size() - nHangoverCount;
                aShapeIter.Reset();
                while (aShapeIter.IsMore())
                {
                    SdrPathObj* pPathObj = dynamic_cast<SdrPathObj*>(aShapeIter.Next());
                    if (pPathObj)
                    {
                        if (nSkip > 0)
                            --nSkip;
                        else
                            pPathObj->SetMergedItem(XLineStyleItem(drawing::LineStyle_NONE));
                    }
                }
            }

            if( xHandoutPage.is() ) try
            {
                xHandoutPage->setPropertyValue( sPageNumber, Any( static_cast<sal_Int16>(mnHandoutPageIndex) ) );
            }
            catch( Exception& )
            {
            }
            rViewShell.SetPrintedHandoutPageNum( mnHandoutPageIndex + 1 );

            rPrinter.SetMapMode(maMap);

            PrintPage(
                rPrinter,
                rPrintView,
                rHandoutPage,
                pView,
                false,
                rVisibleLayers,
                rPrintableLayers);
            PrintMessage(
                rPrinter,
                msPageString,
                maPageStringOffset);

            if( xHandoutPage.is() ) try
            {
                xHandoutPage->setPropertyValue( sPageNumber, Any( static_cast<sal_Int16>(0) ) );
            }
            catch( Exception& )
            {
            }
            rViewShell.SetPrintedHandoutPageNum(1);

            // Restore outlines.
            if (nHangoverCount > 0)
            {
                aShapeIter.Reset();
                while (aShapeIter.IsMore())
                {
                    SdrPathObj* pPathObj = dynamic_cast<SdrPathObj*>(aShapeIter.Next());
                    if (pPathObj != nullptr)
                        pPathObj->SetMergedItem(XLineStyleItem(drawing::LineStyle_SOLID));
                }
            }

       }

    private:
        const sal_uInt16 mnHandoutPageIndex;
        const std::vector<sal_uInt16> maPageIndices;
    };

    /** The outline information (title, subtitle, outline objects) of the
        document.  There is no fixed mapping of slides to printer pages.
    */
    class OutlinerPrinterPage : public PrinterPage
    {
    public:
        OutlinerPrinterPage (
            std::optional<OutlinerParaObject> pParaObject,
            const MapMode& rMapMode,
            const OUString& rsPageString,
            const Point& rPageStringOffset,
            const DrawModeFlags nDrawMode,
            const Orientation eOrientation,
            const sal_uInt16 nPaperTray)
            : PrinterPage(PageKind::Handout, rMapMode, false, rsPageString,
                rPageStringOffset, nDrawMode, eOrientation, nPaperTray),
              mpParaObject(std::move(pParaObject))
        {
        }

        virtual void Print (
            Printer& rPrinter,
            SdDrawDocument& rDocument,
            ViewShell&,
            View*,
            DrawView&,
            const SdrLayerIDSet&,
            const SdrLayerIDSet&) const override
        {
            // Set up the printer.
            rPrinter.SetMapMode(maMap);

            // Get and set up the outliner.
            const ::tools::Rectangle aOutRect (rPrinter.GetPageOffset(), rPrinter.GetOutputSize());
            Outliner* pOutliner = rDocument.GetInternalOutliner();
            const OutlinerMode nSavedOutlMode (pOutliner->GetOutlinerMode());
            const bool bSavedUpdateMode (pOutliner->IsUpdateLayout());
            const Size aSavedPaperSize (pOutliner->GetPaperSize());

            pOutliner->Init(OutlinerMode::OutlineView);
            pOutliner->SetPaperSize(aOutRect.GetSize());
            pOutliner->SetUpdateLayout(true);
            pOutliner->Clear();
            pOutliner->SetText(*mpParaObject);

            pOutliner->Draw(rPrinter, aOutRect);

            PrintMessage(
                rPrinter,
                msPageString,
                maPageStringOffset);

            // Restore outliner and printer.
            pOutliner->Clear();
            pOutliner->SetUpdateLayout(bSavedUpdateMode);
            pOutliner->SetPaperSize(aSavedPaperSize);
            pOutliner->Init(nSavedOutlMode);
        }

    private:
        std::optional<OutlinerParaObject> mpParaObject;
    };
}

//===== DocumentRenderer::Implementation ======================================

class DocumentRenderer::Implementation
    : public SfxListener,
      public vcl::PrinterOptionsHelper
{
public:
    explicit Implementation (ViewShellBase& rBase)
        : mxObjectShell(rBase.GetDocShell())
        , mrBase(rBase)
        , mbIsDisposed(false)
        , mpPrinter(nullptr)
        , mbHasOrientationWarningBeenShown(false)
    {
        DialogCreator aCreator( mrBase, mrBase.GetDocShell()->GetDocumentType() == DocumentType::Impress, GetCurrentPageIndex() );
        m_aUIProperties = aCreator.GetDialogControls();
        maSlidesPerPage = aCreator.GetSlidesPerPage();

        StartListening(mrBase);
    }

    virtual ~Implementation() override
    {
        EndListening(mrBase);
    }

    virtual void Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint) override
    {
        if (&rBroadcaster != &static_cast<SfxBroadcaster&>(mrBase))
            return;

        if (rHint.GetId() == SfxHintId::Dying)
        {
            mbIsDisposed = true;
        }
    }

    /** Process the sequence of properties given to one of the XRenderable
        methods.
    */
    void ProcessProperties (const css::uno::Sequence<css::beans::PropertyValue >& rOptions)
    {
        OSL_ASSERT(!mbIsDisposed);
        if (mbIsDisposed)
            return;

        bool bIsValueChanged = processProperties( rOptions );
        bool bIsPaperChanged = false;

        // The RenderDevice property is handled specially: its value is
        // stored in mpPrinter instead of being retrieved on demand.
        Any aDev( getValue( u"RenderDevice"_ustr ) );
        Reference<awt::XDevice> xRenderDevice;

        if (aDev >>= xRenderDevice)
        {
            VCLXDevice* pDevice = dynamic_cast<VCLXDevice*>(xRenderDevice.get());
            VclPtr< OutputDevice > pOut = pDevice ? pDevice->GetOutputDevice()
                                                  : VclPtr< OutputDevice >();
            mpPrinter = dynamic_cast<Printer*>(pOut.get());
            Size aPageSizePixel = mpPrinter ? mpPrinter->GetPaperSizePixel() : Size();
            if( aPageSizePixel != maPrinterPageSizePixel )
            {
                bIsPaperChanged = true;
                maPrinterPageSizePixel = aPageSizePixel;
            }
        }

        if (bIsValueChanged && ! mpOptions )
            mpOptions.reset(new PrintOptions(*this, std::vector(maSlidesPerPage)));
        if( bIsValueChanged || bIsPaperChanged )
            PreparePages();
    }

    /** Return the number of pages that are to be printed.
    */
    sal_Int32 GetPrintPageCount() const
    {
        OSL_ASSERT(!mbIsDisposed);
        if (mbIsDisposed)
            return 0;
        else
            return maPrinterPages.size();
    }

    /** Return a sequence of properties that can be returned by the
        XRenderable::getRenderer() method.
    */
    css::uno::Sequence<css::beans::PropertyValue> GetProperties () const
    {
        css::uno::Sequence<css::beans::PropertyValue> aProperties{
            comphelper::makePropertyValue(u"ExtraPrintUIOptions"_ustr,
                                          comphelper::containerToSequence(m_aUIProperties)),
            comphelper::makePropertyValue(u"PageSize"_ustr, maPrintSize),
            // FIXME: is this always true ?
            comphelper::makePropertyValue(u"PageIncludesNonprintableArea"_ustr, true)
        };

        return aProperties;
    }

    /** Print one of the prepared pages.
    */
    void PrintPage (const sal_Int32 nIndex)
    {
        OSL_ASSERT(!mbIsDisposed);
        if (mbIsDisposed)
            return;

        Printer& rPrinter (*mpPrinter);

        std::shared_ptr<ViewShell> pViewShell (mrBase.GetMainViewShell());
        if ( ! pViewShell)
            return;

        SdDrawDocument* pDocument = pViewShell->GetDoc();
        assert(pDocument!=nullptr);

        std::shared_ptr<DrawViewShell> pDrawViewShell(
            std::dynamic_pointer_cast<DrawViewShell>(mrBase.GetMainViewShell()));

        if (!mpPrintView)
            mpPrintView.reset(new DrawView(mrBase.GetDocShell(), &rPrinter, nullptr));

        if (nIndex<0 || sal::static_int_cast<sal_uInt32>(nIndex)>=maPrinterPages.size())
            return;

        const std::shared_ptr<PrinterPage> pPage (maPrinterPages[nIndex]);
        OSL_ASSERT(pPage);
        if ( ! pPage)
            return;

        const Orientation eSavedOrientation (rPrinter.GetOrientation());
        const DrawModeFlags nSavedDrawMode (rPrinter.GetDrawMode());
        const MapMode aSavedMapMode (rPrinter.GetMapMode());
        const sal_uInt16 nSavedPaperBin (rPrinter.GetPaperBin());

        // Set page orientation.
        if ( ! rPrinter.SetOrientation(pPage->GetOrientation()))
        {
            if ( ! mbHasOrientationWarningBeenShown
                && mpOptions->IsWarningOrientation())
            {
                mbHasOrientationWarningBeenShown = true;
                // Show warning that the orientation could not be set.
                std::unique_ptr<weld::MessageDialog> xWarn(Application::CreateMessageDialog(
                    pViewShell->GetFrameWeld(), VclMessageType::Warning, VclButtonsType::OkCancel,
                    SdResId(STR_WARN_PRINTFORMAT_FAILURE)));
                xWarn->set_default_response(RET_CANCEL);
                if (xWarn->run() != RET_OK)
                    return;
            }
        }

        // Set the draw mode.
        rPrinter.SetDrawMode(pPage->GetDrawMode());

        // Set paper tray.
        rPrinter.SetPaperBin(pPage->GetPaperTray());

        // Print the actual page.
        pPage->Print(
            rPrinter,
            *pDocument,
            *pViewShell,
            pDrawViewShell ? pDrawViewShell->GetView() : nullptr,
            *mpPrintView,
            pViewShell->GetFrameView()->GetVisibleLayers(),
            pViewShell->GetFrameView()->GetPrintableLayers());

        rPrinter.SetOrientation(eSavedOrientation);
        rPrinter.SetDrawMode(nSavedDrawMode);
        rPrinter.SetMapMode(aSavedMapMode);
        rPrinter.SetPaperBin(nSavedPaperBin);
    }

private:
    // rhbz#657394: keep the document alive: prevents crash when
    SfxObjectShellRef mxObjectShell; // destroying mpPrintView
    ViewShellBase& mrBase;
    bool mbIsDisposed;
    VclPtr<Printer> mpPrinter;
    Size maPrinterPageSizePixel;
    std::unique_ptr<PrintOptions> mpOptions;
    std::vector< std::shared_ptr< ::sd::PrinterPage> > maPrinterPages;
    std::unique_ptr<DrawView> mpPrintView;
    bool mbHasOrientationWarningBeenShown;
    std::vector<sal_Int32> maSlidesPerPage;
    awt::Size maPrintSize;

    sal_Int32 GetCurrentPageIndex() const
    {
        const ViewShell *pShell = mrBase.GetMainViewShell().get();
        const SdPage *pCurrentPage = pShell ? pShell->getCurrentPage() : nullptr;
        return pCurrentPage ? (pCurrentPage->GetPageNum()-1)/2 : -1;
    }

    /** Determine and set the paper orientation.
    */
    void SetupPaperOrientation (
        const PageKind ePageKind,
        PrintInfo& rInfo)
    {
        SdDrawDocument* pDocument = mrBase.GetMainViewShell()->GetDoc();
        rInfo.meOrientation = Orientation::Portrait;

        if( ! mpOptions->IsBooklet())
        {
            rInfo.meOrientation = pDocument->GetSdPage(0, ePageKind)->GetOrientation();
        }
        else if (rInfo.maPageSize.Width() < rInfo.maPageSize.Height())
            rInfo.meOrientation = Orientation::Landscape;

        // Draw and Notes should usually abide by their specified paper size
        Size aPaperSize;
        if (!mpOptions->IsPrinterPreferred(pDocument->GetDocumentType()))
        {
            aPaperSize.setWidth(rInfo.maPageSize.Width());
            aPaperSize.setHeight(rInfo.maPageSize.Height());
        }
        else
        {
            aPaperSize.setWidth(rInfo.mpPrinter->GetPaperSize().Width());
            aPaperSize.setHeight(rInfo.mpPrinter->GetPaperSize().Height());
        }

        maPrintSize = awt::Size(aPaperSize.Width(), aPaperSize.Height());

        if (mpOptions->IsPrinterPreferred(pDocument->GetDocumentType()))
        {
            if( (rInfo.meOrientation == Orientation::Landscape &&
                  (aPaperSize.Width() < aPaperSize.Height()))
               ||
                (rInfo.meOrientation == Orientation::Portrait &&
                  (aPaperSize.Width() > aPaperSize.Height()))
              )
            {
                maPrintSize = awt::Size(aPaperSize.Height(), aPaperSize.Width());
            }
        }
    }

    /** Top most method for preparing printer pages.  In this and the other
        Prepare... methods the various special cases are detected and
        handled.
        For every page that is to be printed (that may contain several
        slides) one PrinterPage object is created and inserted into
        maPrinterPages.
    */
    void PreparePages()
    {
        mpPrintView.reset();
        maPrinterPages.clear();
        mbHasOrientationWarningBeenShown = false;

        ViewShell* pShell = mrBase.GetMainViewShell().get();

        PrintInfo aInfo (mpPrinter, mpOptions->IsPrintMarkedOnly());

        if (aInfo.mpPrinter==nullptr || pShell==nullptr)
            return;

        MapMode aMap (aInfo.mpPrinter->GetMapMode());
        aMap.SetMapUnit(MapUnit::Map100thMM);
        aInfo.maMap = aMap;
        mpPrinter->SetMapMode(aMap);

        ::Outliner& rOutliner = mrBase.GetDocument()->GetDrawOutliner();
        const EEControlBits nSavedControlWord (rOutliner.GetControlWord());
        EEControlBits nCntrl = nSavedControlWord;
        nCntrl &= ~EEControlBits::MARKFIELDS;
        nCntrl &= ~EEControlBits::ONLINESPELLING;
        rOutliner.SetControlWord( nCntrl );

        // When in outline view then apply all pending changes to the model.
        if( auto pOutlineViewShell = dynamic_cast< OutlineViewShell *>( pShell ) )
            pOutlineViewShell->PrepareClose (false);

        // Collect some frequently used data.
        if (mpOptions->IsDate())
        {
            aInfo.msTimeDate += GetSdrGlobalData().GetLocaleData().getDate( Date( Date::SYSTEM ) );
            aInfo.msTimeDate += " ";
        }

        if (mpOptions->IsTime())
            aInfo.msTimeDate += GetSdrGlobalData().GetLocaleData().getTime( ::tools::Time( ::tools::Time::SYSTEM ), false );

        // Draw and Notes should usually use specified paper size when printing
        if (!mpOptions->IsPrinterPreferred(mrBase.GetDocShell()->GetDocumentType()))
        {
            aInfo.maPrintSize = mrBase.GetDocument()->GetSdPage(0, PageKind::Standard)->GetSize();
            maPrintSize = awt::Size(aInfo.maPrintSize.Width(),
                                    aInfo.maPrintSize.Height());
        }
        else
        {
            aInfo.maPrintSize = aInfo.mpPrinter->GetOutputSize();
            maPrintSize = awt::Size(
                aInfo.mpPrinter->GetPaperSize().Width(),
                aInfo.mpPrinter->GetPaperSize().Height());
        }

        switch (mpOptions->GetOutputQuality())
        {
            case 1: // Grayscale
                aInfo.mnDrawMode = DrawModeFlags::GrayLine | DrawModeFlags::GrayFill
                    | DrawModeFlags::GrayText | DrawModeFlags::GrayBitmap
                    | DrawModeFlags::GrayGradient;
                break;

            case 2: // Black & White
                aInfo.mnDrawMode = DrawModeFlags::BlackLine | DrawModeFlags::WhiteFill
                    | DrawModeFlags::BlackText | DrawModeFlags::GrayBitmap
                    | DrawModeFlags::WhiteGradient;
                break;

            default:
                aInfo.mnDrawMode = DrawModeFlags::Default;
        }

        if (mpOptions->IsDraw())
            PrepareStdOrNotes(PageKind::Standard, aInfo);
        if (mpOptions->IsNotes())
            PrepareStdOrNotes(PageKind::Notes, aInfo);
        if (mpOptions->IsHandout())
        {
            InitHandoutTemplate();
            PrepareHandout(aInfo);
        }
        if (mpOptions->IsOutline())
            PrepareOutline(aInfo);

        rOutliner.SetControlWord(nSavedControlWord);
    }

    /** Create the page objects of the handout template.  When the actual
        printing takes place then the page objects are assigned different
        sets of slides for each printed page (see HandoutPrinterPage::Print).
    */
    void InitHandoutTemplate()
    {
        const sal_Int32 nSlidesPerHandout (mpOptions->GetHandoutPageCount());
        const bool bHandoutHorizontal (mpOptions->IsHandoutHorizontal());

        AutoLayout eLayout = AUTOLAYOUT_HANDOUT6;
        switch (nSlidesPerHandout)
        {
            case 0: eLayout = AUTOLAYOUT_NONE; break; // AUTOLAYOUT_HANDOUT1; break;
            case 1: eLayout = AUTOLAYOUT_HANDOUT1; break;
            case 2: eLayout = AUTOLAYOUT_HANDOUT2; break;
            case 3: eLayout = AUTOLAYOUT_HANDOUT3; break;
            case 4: eLayout = AUTOLAYOUT_HANDOUT4; break;
            default:
            case 6: eLayout = AUTOLAYOUT_HANDOUT6; break;
            case 9: eLayout = AUTOLAYOUT_HANDOUT9; break;
        }

        if( !mrBase.GetDocument() )
            return;

        SdDrawDocument& rModel = *mrBase.GetDocument();

        // first, prepare handout page (not handout master)

        SdPage* pHandout = rModel.GetSdPage(0, PageKind::Handout);
        if( !pHandout )
            return;

        // delete all previous shapes from handout page
        while( pHandout->GetObjCount() )
            pHandout->NbcRemoveObject(0);

        const bool bDrawLines (eLayout == AUTOLAYOUT_HANDOUT3);

        std::vector< ::tools::Rectangle > aAreas;
        SdPage::CalculateHandoutAreas( rModel, eLayout, bHandoutHorizontal, aAreas );

        std::vector< ::tools::Rectangle >::iterator iter( aAreas.begin() );
        while( iter != aAreas.end() )
        {
            pHandout->NbcInsertObject(
                new SdrPageObj(
                    rModel,
                    (*iter++)));

            if( bDrawLines && (iter != aAreas.end())  )
            {
                ::tools::Rectangle aRect( *iter++ );

                basegfx::B2DPolygon aPoly;
                aPoly.insert(0, basegfx::B2DPoint( aRect.Left(), aRect.Top() ) );
                aPoly.insert(1, basegfx::B2DPoint( aRect.Right(), aRect.Top() ) );

                basegfx::B2DHomMatrix aMatrix;
                aMatrix.translate( 0.0, static_cast< double >( aRect.GetHeight() / 7 ) );

                basegfx::B2DPolyPolygon aPathPoly;
                for( sal_uInt16 nLine = 0; nLine < 7; nLine++ )
                {
                    aPoly.transform( aMatrix );
                    aPathPoly.append( aPoly );
                }

                rtl::Reference<SdrPathObj> pPathObj = new SdrPathObj(
                    rModel,
                    SdrObjKind::PathLine,
                    std::move(aPathPoly));
                pPathObj->SetMergedItem(XLineStyleItem(drawing::LineStyle_SOLID));
                pPathObj->SetMergedItem(XLineColorItem(OUString(), COL_BLACK));

                pHandout->NbcInsertObject( pPathObj.get() );
            }
        }
    }

    /** Detect whether the specified slide is to be printed.
        @return
            When the slide is not to be printed then <NULL/> is returned.
            Otherwise a pointer to the slide is returned.
    */
    SdPage* GetFilteredPage (
        const sal_Int32 nPageIndex,
        const PageKind ePageKind) const
    {
        OSL_ASSERT(mrBase.GetDocument() != nullptr);
        OSL_ASSERT(nPageIndex>=0);
        SdPage* pPage = mrBase.GetDocument()->GetSdPage(
            sal::static_int_cast<sal_uInt16>(nPageIndex),
            ePageKind);
        if (pPage == nullptr)
            return nullptr;
        if ( ! pPage->IsExcluded() || mpOptions->IsPrintExcluded())
            return pPage;
        else
            return nullptr;
    }

    /** Prepare the outline of the document for printing.  There is no fixed
        number of slides whose outline data is put onto one printer page.
        If the current printer page has enough room for the outline of the
        current slide then that is added.  Otherwise a new printer page is
        started.
    */
    void PrepareOutline (PrintInfo const & rInfo)
    {
        MapMode aMap (rInfo.maMap);
        Point aPageOfs (rInfo.mpPrinter->GetPageOffset() );
        aMap.SetScaleX(Fraction(1,2));
        aMap.SetScaleY(Fraction(1,2));
        mpPrinter->SetMapMode(aMap);

        ::tools::Rectangle aOutRect(aPageOfs, rInfo.mpPrinter->GetOutputSize());
        if( aOutRect.GetWidth() > aOutRect.GetHeight() )
        {
            Size aPaperSize( rInfo.mpPrinter->PixelToLogic( rInfo.mpPrinter->GetPaperSizePixel(), MapMode( MapUnit::Map100thMM ) ) );
            maPrintSize.Width  = aPaperSize.Height();
            maPrintSize.Height = aPaperSize.Width();
            const auto nRotatedWidth = aOutRect.GetHeight();
            const auto nRotatedHeight = aOutRect.GetWidth();
            const auto nRotatedX = aPageOfs.Y();
            const auto nRotatedY = aPageOfs.X();
            aOutRect = ::tools::Rectangle(Point( nRotatedX, nRotatedY),
                                  Size(nRotatedWidth, nRotatedHeight));
        }

        Outliner* pOutliner = mrBase.GetDocument()->GetInternalOutliner();
        pOutliner->Init(OutlinerMode::OutlineView);
        const OutlinerMode nSavedOutlMode (pOutliner->GetOutlinerMode());
        const bool bSavedUpdateMode (pOutliner->IsUpdateLayout());
        const Size aSavedPaperSize (pOutliner->GetPaperSize());
        const MapMode aSavedMapMode (pOutliner->GetRefMapMode());
        pOutliner->SetPaperSize(aOutRect.GetSize());
        pOutliner->SetUpdateLayout(true);

        ::tools::Long nPageH = aOutRect.GetHeight();

        std::vector< sal_Int32 > aPages;
        sal_Int32 nPageCount = mrBase.GetDocument()->GetSdPageCount(PageKind::Standard);
        StringRangeEnumerator::getRangesFromString(
            mpOptions->GetPrinterSelection(nPageCount, GetCurrentPageIndex()),
            aPages, 0, nPageCount-1);

        for (size_t nIndex = 0, nCount = aPages.size(); nIndex < nCount;)
        {
            pOutliner->Clear();

            Paragraph* pPara = nullptr;
            ::tools::Long nH (0);
            while (nH < nPageH && nIndex<nCount)
            {
                SdPage* pPage = GetFilteredPage(aPages[nIndex], PageKind::Standard);
                ++nIndex;
                if (pPage == nullptr)
                    continue;

                SdrTextObj* pTextObj = nullptr;

                for (const rtl::Reference<SdrObject>& pObj : *pPage)
                {
                    if (pObj->GetObjInventor() == SdrInventor::Default
                        && pObj->GetObjIdentifier() == SdrObjKind::TitleText)
                    {
                        pTextObj = DynCastSdrTextObj(pObj.get());
                        if (pTextObj)
                            break;
                    }
                }

                pPara = pOutliner->GetParagraph(pOutliner->GetParagraphCount() - 1);

                if (pTextObj!=nullptr
                    && !pTextObj->IsEmptyPresObj()
                    && pTextObj->GetOutlinerParaObject())
                {
                    pOutliner->AddText(*(pTextObj->GetOutlinerParaObject()));
                }
                else
                    pOutliner->Insert(OUString());

                pTextObj = nullptr;

                for (const rtl::Reference<SdrObject>& pObj : *pPage)
                {
                    if (pObj->GetObjInventor() == SdrInventor::Default
                        && pObj->GetObjIdentifier() == SdrObjKind::OutlineText)
                    {
                        pTextObj = DynCastSdrTextObj(pObj.get());
                        if (pTextObj)
                            break;
                    }
                }

                bool bSubTitle (false);
                if (!pTextObj)
                {
                    bSubTitle = true;
                    pTextObj = DynCastSdrTextObj(pPage->GetPresObj(PresObjKind::Text));  // is there a subtitle?
                }

                sal_Int32 nParaCount1 = pOutliner->GetParagraphCount();

                if (pTextObj!=nullptr
                    && !pTextObj->IsEmptyPresObj()
                    && pTextObj->GetOutlinerParaObject())
                {
                    pOutliner->AddText(*(pTextObj->GetOutlinerParaObject()));
                }

                if (bSubTitle )
                {
                    const sal_Int32 nParaCount2 (pOutliner->GetParagraphCount());
                    for (sal_Int32 nPara=nParaCount1; nPara<nParaCount2; ++nPara)
                    {
                        Paragraph* pP = pOutliner->GetParagraph(nPara);
                        if (pP!=nullptr && pOutliner->GetDepth(nPara) > 0)
                            pOutliner->SetDepth(pP, 0);
                    }
                }

                nH = pOutliner->GetTextHeight();
            }

            // Remove the last paragraph when that does not fit completely on
            // the current page.
            if (nH > nPageH && pPara!=nullptr)
            {
                sal_Int32 nCnt = pOutliner->GetAbsPos(
                    pOutliner->GetParagraph( pOutliner->GetParagraphCount() - 1 ) );
                sal_Int32 nParaPos = pOutliner->GetAbsPos( pPara );
                nCnt -= nParaPos;
                pPara = pOutliner->GetParagraph( ++nParaPos );
                if ( nCnt && pPara )
                {
                    pOutliner->Remove(pPara, nCnt);
                    --nIndex;
                }
            }

            if ( CheckForFrontBackPages( nIndex ) )
            {
                maPrinterPages.push_back(
                    std::make_shared<OutlinerPrinterPage>(
                        pOutliner->CreateParaObject(),
                        aMap,
                        rInfo.msTimeDate,
                        aPageOfs,
                        rInfo.mnDrawMode,
                        rInfo.meOrientation,
                        rInfo.mpPrinter->GetPaperBin()));
            }
        }

        pOutliner->SetRefMapMode(aSavedMapMode);
        pOutliner->SetUpdateLayout(bSavedUpdateMode);
        pOutliner->SetPaperSize(aSavedPaperSize);
        pOutliner->Init(nSavedOutlMode);
    }

    /** Prepare handout pages for slides that are to be printed.
    */
    void PrepareHandout (PrintInfo& rInfo)
    {
        SdDrawDocument* pDocument = mrBase.GetDocument();
        assert(pDocument != nullptr);
        SdPage& rHandoutPage (*pDocument->GetSdPage(0, PageKind::Handout));

        const bool bScalePage (mpOptions->IsPageSize());

        sal_uInt16 nPaperBin;
        if ( ! mpOptions->IsPaperBin())
            nPaperBin = rHandoutPage.GetPaperBin();
        else
            nPaperBin = rInfo.mpPrinter->GetPaperBin();

        // Change orientation?
        SdPage& rMaster (dynamic_cast<SdPage&>(rHandoutPage.TRG_GetMasterPage()));
        rInfo.meOrientation = rMaster.GetOrientation();

        const Size aPaperSize (rInfo.mpPrinter->GetPaperSize());
        if( (rInfo.meOrientation == Orientation::Landscape &&
              (aPaperSize.Width() < aPaperSize.Height()))
           ||
            (rInfo.meOrientation == Orientation::Portrait &&
              (aPaperSize.Width() > aPaperSize.Height()))
          )
        {
            maPrintSize = awt::Size(aPaperSize.Height(), aPaperSize.Width());
        }
        else
        {
            maPrintSize = awt::Size(aPaperSize.Width(), aPaperSize.Height());
        }

        MapMode aMap (rInfo.maMap);
        const Point aPageOfs (rInfo.mpPrinter->GetPageOffset());

        if ( bScalePage )
        {
            const Size aPageSize (rHandoutPage.GetSize());
            const Size aPrintSize (rInfo.mpPrinter->GetOutputSize());

            const double fHorz = static_cast<double>(aPrintSize.Width())    / aPageSize.Width();
            const double fVert = static_cast<double>(aPrintSize.Height()) / aPageSize.Height();

            Fraction    aFract;
            if ( fHorz < fVert )
                aFract = Fraction(aPrintSize.Width(), aPageSize.Width());
            else
                aFract = Fraction(aPrintSize.Height(), aPageSize.Height());

            aMap.SetScaleX(aFract);
            aMap.SetScaleY(aFract);
            aMap.SetOrigin(Point());
        }

        std::shared_ptr<ViewShell> pViewShell (mrBase.GetMainViewShell());
        pViewShell->WriteFrameViewData();

        // Count page shapes.
        sal_uInt32 nShapeCount (0);
        SdrObjListIter aShapeIter (&rHandoutPage);
        while (aShapeIter.IsMore())
        {
            SdrPageObj* pPageObj = dynamic_cast<SdrPageObj*>(aShapeIter.Next());
            if (pPageObj)
                ++nShapeCount;
        }

        const sal_uInt16 nPageCount = mrBase.GetDocument()->GetSdPageCount(PageKind::Standard);
        const sal_uInt16 nHandoutPageCount = nShapeCount ? (nPageCount + nShapeCount - 1) / nShapeCount : 0;
        pViewShell->SetPrintedHandoutPageCount( nHandoutPageCount );
        mrBase.GetDocument()->setHandoutPageCount( nHandoutPageCount );

        // Distribute pages to handout pages.
        StringRangeEnumerator aRangeEnum(
            mpOptions->GetPrinterSelection(nPageCount, GetCurrentPageIndex()),
            0, nPageCount-1);
        std::vector<sal_uInt16> aPageIndices;
        sal_uInt16 nPrinterPageIndex = 0;
        StringRangeEnumerator::Iterator it = aRangeEnum.begin(), itEnd = aRangeEnum.end();
        bool bLastLoop = (it == itEnd);
        while (!bLastLoop)
        {
            sal_Int32 nPageIndex = *it;
            ++it;
            bLastLoop = (it == itEnd);

            if (GetFilteredPage(nPageIndex, PageKind::Standard))
                aPageIndices.push_back(nPageIndex);
            else if (!bLastLoop)
                continue;

            // Create a printer page when we have found one page for each
            // placeholder or when this is the last (and special) loop.
            if ( !aPageIndices.empty() && CheckForFrontBackPages( nPageIndex )
                && (aPageIndices.size() == nShapeCount || bLastLoop) )
            {
                maPrinterPages.push_back(
                    std::make_shared<HandoutPrinterPage>(
                            nPrinterPageIndex++,
                            std::move(aPageIndices),
                            aMap,
                            rInfo.msTimeDate,
                            aPageOfs,
                            rInfo.mnDrawMode,
                            rInfo.meOrientation,
                            nPaperBin));
                aPageIndices.clear();
            }
        }
    }

    /** Prepare the notes pages or regular slides.
    */
    void PrepareStdOrNotes (
        const PageKind ePageKind,
        PrintInfo& rInfo)
    {
        OSL_ASSERT(rInfo.mpPrinter != nullptr);

        // Fill in page kind specific data.
        SdDrawDocument* pDocument = mrBase.GetMainViewShell()->GetDoc();
        if (pDocument->GetSdPageCount(ePageKind) == 0)
            return;
        SdPage* pRefPage = pDocument->GetSdPage(0, ePageKind);
        rInfo.maPageSize = pRefPage->GetSize();

        SetupPaperOrientation(ePageKind, rInfo);

        if (mpOptions->IsBooklet())
            PrepareBooklet(ePageKind, rInfo);
        else
            PrepareRegularPages(ePageKind, rInfo);
    }

    /** Prepare slides in a non-booklet way: one slide per one to many
        printer pages.
    */
    void PrepareRegularPages (
        const PageKind ePageKind,
        PrintInfo& rInfo)
    {
        std::shared_ptr<ViewShell> pViewShell (mrBase.GetMainViewShell());
        pViewShell->WriteFrameViewData();

        sal_Int32 nPageCount = mrBase.GetDocument()->GetSdPageCount(PageKind::Standard);
        StringRangeEnumerator aRangeEnum(
            mpOptions->GetPrinterSelection(nPageCount, GetCurrentPageIndex()),
            0, nPageCount-1);
        for (StringRangeEnumerator::Iterator
                 it = aRangeEnum.begin(),
                 itEnd = aRangeEnum.end();
             it != itEnd;
             ++it)
        {
            SdPage* pPage = GetFilteredPage(*it, ePageKind);
            if (pPage == nullptr)
                continue;

            MapMode aMap (rInfo.maMap);
            // is it possible that the page size changed?
            const Size aPageSize = pPage->GetSize();

            if (mpOptions->IsPageSize())
            {
                const double fHorz (static_cast<double>(rInfo.maPrintSize.Width())  / aPageSize.Width());
                const double fVert (static_cast<double>(rInfo.maPrintSize.Height()) / aPageSize.Height());

                Fraction aFract;
                if (fHorz < fVert)
                    aFract = Fraction(rInfo.maPrintSize.Width(), aPageSize.Width());
                else
                    aFract = Fraction(rInfo.maPrintSize.Height(), aPageSize.Height());

                aMap.SetScaleX(aFract);
                aMap.SetScaleY(aFract);
                aMap.SetOrigin(Point());
            }

            if (mpOptions->IsPrintPageName())
            {
                rInfo.msPageString = pPage->GetName() + " ";
            }
            else
                rInfo.msPageString.clear();
            rInfo.msPageString += rInfo.msTimeDate;

            ::tools::Long aPageWidth   = aPageSize.Width() - pPage->GetLeftBorder() - pPage->GetRightBorder();
            ::tools::Long aPageHeight  = aPageSize.Height() - pPage->GetUpperBorder() - pPage->GetLowerBorder();
            // Bugfix for 44530:
            // if it was implicitly changed (Landscape/Portrait),
            // this is considered for tiling, respectively for the splitting up
            // (Poster)
            if( ( rInfo.maPrintSize.Width() > rInfo.maPrintSize.Height()
                    && aPageWidth < aPageHeight )
                || ( rInfo.maPrintSize.Width() < rInfo.maPrintSize.Height()
                    && aPageWidth > aPageHeight ) )
            {
                const sal_Int32 nTmp (rInfo.maPrintSize.Width());
                rInfo.maPrintSize.setWidth( rInfo.maPrintSize.Height() );
                rInfo.maPrintSize.setHeight( nTmp );
            }

            if (mpOptions->IsTilePage()
                && aPageWidth < rInfo.maPrintSize.Width()
                && aPageHeight < rInfo.maPrintSize.Height())
            {
                // Put multiple slides on one printer page.
                PrepareTiledPage(*it, *pPage, ePageKind, rInfo);
            }
            else
            {
                rInfo.maMap = aMap;
                PrepareScaledPage(*it, *pPage, ePageKind, rInfo);
            }
        }
    }

    /** Put two slides on one printer page.
    */
    void PrepareBooklet (
        const PageKind ePageKind,
        const PrintInfo& rInfo)
    {
        MapMode aStdMap (rInfo.maMap);
        Point aOffset;
        Size aPrintSize_2 (rInfo.maPrintSize);
        Size aPageSize_2 (rInfo.maPageSize);

        if (rInfo.meOrientation == Orientation::Landscape)
            aPrintSize_2.setWidth( aPrintSize_2.Width() >> 1 );
        else
            aPrintSize_2.setHeight( aPrintSize_2.Height() >> 1 );

        const double fPageWH = static_cast<double>(aPageSize_2.Width()) / aPageSize_2.Height();
        const double fPrintWH = static_cast<double>(aPrintSize_2.Width()) / aPrintSize_2.Height();

        if( fPageWH < fPrintWH )
        {
            aPageSize_2.setWidth(  static_cast<::tools::Long>( aPrintSize_2.Height() * fPageWH ) );
            aPageSize_2.setHeight( aPrintSize_2.Height() );
        }
        else
        {
            aPageSize_2.setWidth( aPrintSize_2.Width() );
            aPageSize_2.setHeight( static_cast<::tools::Long>( aPrintSize_2.Width() / fPageWH ) );
        }

        MapMode aMap (rInfo.maMap);
        aMap.SetScaleX( Fraction( aPageSize_2.Width(), rInfo.maPageSize.Width() ) );
        aMap.SetScaleY( Fraction( aPageSize_2.Height(), rInfo.maPageSize.Height() ) );

        // calculate adjusted print size
        const Size aAdjustedPrintSize (OutputDevice::LogicToLogic(
            rInfo.maPrintSize,
            aStdMap,
            aMap));

        if (rInfo.meOrientation == Orientation::Landscape)
        {
            aOffset.setX( ( ( aAdjustedPrintSize.Width() >> 1 ) - rInfo.maPageSize.Width() ) >> 1 );
            aOffset.setY( ( aAdjustedPrintSize.Height() - rInfo.maPageSize.Height() ) >> 1 );
        }
        else
        {
            aOffset.setX( ( aAdjustedPrintSize.Width() - rInfo.maPageSize.Width() ) >> 1 );
            aOffset.setY( ( ( aAdjustedPrintSize.Height() >> 1 ) - rInfo.maPageSize.Height() ) >> 1 );
        }

        // create vector of pages to print
        sal_Int32 nPageCount = mrBase.GetDocument()->GetSdPageCount(ePageKind);
        StringRangeEnumerator aRangeEnum(
            mpOptions->GetPrinterSelection(nPageCount, GetCurrentPageIndex()),
            0, nPageCount-1);
        std::vector< sal_uInt16 > aPageVector;
        for (StringRangeEnumerator::Iterator
                 it = aRangeEnum.begin(),
                 itEnd = aRangeEnum.end();
             it != itEnd;
             ++it)
        {
            SdPage* pPage = GetFilteredPage(*it, ePageKind);
            if (pPage != nullptr)
                aPageVector.push_back(*it);
        }

        // create pairs of pages to print on each page
        std::vector< std::pair< sal_uInt16, sal_uInt16 > > aPairVector;
        if ( ! aPageVector.empty())
        {
            sal_uInt32 nFirstIndex = 0, nLastIndex = aPageVector.size() - 1;

            if( aPageVector.size() & 1 )
                aPairVector.emplace_back( sal_uInt16(65535), aPageVector[ nFirstIndex++ ] );
            else
                aPairVector.emplace_back( aPageVector[ nLastIndex-- ], aPageVector[ nFirstIndex++ ] );

            while( nFirstIndex < nLastIndex )
            {
                if( nFirstIndex & 1 )
                    aPairVector.emplace_back( aPageVector[ nFirstIndex++ ], aPageVector[ nLastIndex-- ] );
                else
                    aPairVector.emplace_back( aPageVector[ nLastIndex-- ], aPageVector[ nFirstIndex++ ] );
            }
        }

        for (sal_uInt32
                 nIndex=0,
                 nCount=aPairVector.size();
             nIndex < nCount;
             ++nIndex)
        {
            if ( CheckForFrontBackPages( nIndex ) )
            {
                const std::pair<sal_uInt16, sal_uInt16> aPair (aPairVector[nIndex]);
                Point aSecondOffset (aOffset);
                if (rInfo.meOrientation == Orientation::Landscape)
                    aSecondOffset.AdjustX( aAdjustedPrintSize.Width() / 2 );
                else
                    aSecondOffset.AdjustY( aAdjustedPrintSize.Height() / 2 );
                maPrinterPages.push_back(
                    std::make_shared<BookletPrinterPage>(
                            aPair.first,
                            aPair.second,
                            aOffset,
                            aSecondOffset,
                            ePageKind,
                            aMap,
                            rInfo.mbPrintMarkedOnly,
                            rInfo.mnDrawMode,
                            rInfo.meOrientation,
                            rInfo.mpPrinter->GetPaperBin()));

            }
        }
    }

    /** Print one slide multiple times on one printer page so that the whole
        printer page is covered.
    */
    void PrepareTiledPage (
        const sal_Int32 nPageIndex,
        const SdPage& rPage,
        const PageKind ePageKind,
        const PrintInfo& rInfo)
    {
        sal_uInt16 nPaperBin;
        if ( ! mpOptions->IsPaperBin())
            nPaperBin = rPage.GetPaperBin();
        else
            nPaperBin = rInfo.mpPrinter->GetPaperBin();

        if ( !CheckForFrontBackPages( nPageIndex ) )
            return;

        maPrinterPages.push_back(
            std::make_shared<TiledPrinterPage>(
                sal::static_int_cast<sal_uInt16>(nPageIndex),
                ePageKind,
                rInfo.mbPrintMarkedOnly,
                rInfo.msPageString,
                rInfo.mpPrinter->GetPageOffset(),
                rInfo.mnDrawMode,
                rInfo.meOrientation,
                nPaperBin));
    }

    /** Print one standard slide or notes page on one to many printer
        pages.  More than on printer page is used when the slide is larger
        than the printable area.
    */
    void PrepareScaledPage (
        const sal_Int32 nPageIndex,
        const SdPage& rPage,
        const PageKind ePageKind,
        const PrintInfo& rInfo)
    {
        const Point aPageOffset (rInfo.mpPrinter->GetPageOffset());

        sal_uInt16 nPaperBin;
        if ( ! mpOptions->IsPaperBin())
            nPaperBin = rPage.GetPaperBin();
        else
            nPaperBin = rInfo.mpPrinter->GetPaperBin();

        // For pages larger than the printable area there are three options:
        // 1. Scale down to the page to the printable area.
        // 2. Print only the upper left part of the page (without the unprintable borders).
        // 3. Split the page into parts of the size of the printable area.
        const bool bScalePage (mpOptions->IsPageSize());
        const bool bCutPage (mpOptions->IsCutPage());
        MapMode aMap (rInfo.maMap);
        if ( (bScalePage || bCutPage) && CheckForFrontBackPages( nPageIndex ) )
        {
            // Handle 1 and 2.

            // if CutPage is set then do not move it, otherwise move the
            // scaled page to printable area
            maPrinterPages.push_back(
                std::make_shared<RegularPrinterPage>(
                        sal::static_int_cast<sal_uInt16>(nPageIndex),
                        ePageKind,
                        aMap,
                        rInfo.mbPrintMarkedOnly,
                        rInfo.msPageString,
                        aPageOffset,
                        rInfo.mnDrawMode,
                        rInfo.meOrientation,
                        nPaperBin));
        }
        else
        {
            // Handle 3.  Print parts of the page in the size of the
            // printable area until the whole page is covered.

            // keep the page content at its position if it fits, otherwise
            // move it to the printable area
            const ::tools::Long nPageWidth (
                rInfo.maPageSize.Width() - rPage.GetLeftBorder() - rPage.GetRightBorder());
            const ::tools::Long nPageHeight (
                rInfo.maPageSize.Height() - rPage.GetUpperBorder() - rPage.GetLowerBorder());

            Point aOrigin ( 0, 0 );

            for (Point aPageOrigin = aOrigin;
                 -aPageOrigin.Y()<nPageHeight;
                 aPageOrigin.AdjustY( -rInfo.maPrintSize.Height() ))
            {
                for (aPageOrigin.setX(aOrigin.X());
                     -aPageOrigin.X()<nPageWidth;
                     aPageOrigin.AdjustX(-rInfo.maPrintSize.Width()))
                {
                    if ( CheckForFrontBackPages( nPageIndex ) )
                    {
                        aMap.SetOrigin(aPageOrigin);
                        maPrinterPages.push_back(
                            std::make_shared<RegularPrinterPage>(
                                    sal::static_int_cast<sal_uInt16>(nPageIndex),
                                    ePageKind,
                                    aMap,
                                    rInfo.mbPrintMarkedOnly,
                                    rInfo.msPageString,
                                    aPageOffset,
                                    rInfo.mnDrawMode,
                                    rInfo.meOrientation,
                                    nPaperBin));
                    }
                }
            }
        }
    }

bool CheckForFrontBackPages( sal_Int32 nPage )
{
    const bool bIsIndexOdd(nPage & 1);
    if ((!bIsIndexOdd && mpOptions->IsPrintFrontPage())
        || (bIsIndexOdd && mpOptions->IsPrintBackPage()))
    {
        return true;
    }
    else
        return false;
}
};

//===== DocumentRenderer ======================================================

DocumentRenderer::DocumentRenderer (ViewShellBase& rBase)
    : mpImpl(new Implementation(rBase))
{
}

DocumentRenderer::~DocumentRenderer()
{
}

//----- XRenderable -----------------------------------------------------------

sal_Int32 SAL_CALL DocumentRenderer::getRendererCount (
    const css::uno::Any&,
    const css::uno::Sequence<css::beans::PropertyValue >& rOptions)
{
    mpImpl->ProcessProperties(rOptions);
    return mpImpl->GetPrintPageCount();
}

Sequence<beans::PropertyValue> SAL_CALL DocumentRenderer::getRenderer (
    sal_Int32,
    const css::uno::Any&,
    const css::uno::Sequence<css::beans::PropertyValue>& rOptions)
{
    mpImpl->ProcessProperties(rOptions);
    return mpImpl->GetProperties();
}

void SAL_CALL DocumentRenderer::render (
    sal_Int32 nRenderer,
    const css::uno::Any&,
    const css::uno::Sequence<css::beans::PropertyValue>& rOptions)
{
    mpImpl->ProcessProperties(rOptions);
    mpImpl->PrintPage(nRenderer);
}

} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
