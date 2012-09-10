/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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


#include <com/sun/star/beans/XPropertySet.hpp>

#include "DocumentRenderer.hxx"
#include "DocumentRenderer.hrc"

#include "drawdoc.hxx"
#include "optsitem.hxx"
#include "sdresid.hxx"
#include "strings.hrc"
#include "sdattr.hxx"
#include "Window.hxx"
#include "drawview.hxx"
#include "DrawViewShell.hxx"
#include "FrameView.hxx"
#include "Outliner.hxx"
#include "OutlineViewShell.hxx"

#include <basegfx/polygon/b2dpolygon.hxx>
#include <basegfx/polygon/b2dpolypolygon.hxx>
#include <basegfx/matrix/b2dhommatrix.hxx>
#include <sfx2/printer.hxx>
#include <editeng/editstat.hxx>
#include <editeng/outlobj.hxx>
#include <svx/svdetc.hxx>
#include <svx/svditer.hxx>
#include <svx/svdopage.hxx>
#include <svx/svdopath.hxx>
#include <svx/xlnclit.hxx>
#include <toolkit/awt/vclxdevice.hxx>
#include <tools/resary.hxx>
#include <unotools/localedatawrapper.hxx>
#include <vcl/msgbox.hxx>
#include <unotools/moduleoptions.hxx>

#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::uno;
using ::rtl::OUString;


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
            const ::std::vector<sal_Int32>& rSlidesPerPage)
            : mrProperties(rHelper),
              maSlidesPerPage(rSlidesPerPage)
        {
        }

        bool IsWarningOrientation() const
        {
            return GetBoolValue(NULL, true);
        }

        bool IsPrintPageName() const
        {
            return GetBoolValue("IsPrintName");
        }

        bool IsDate() const
        {
            return GetBoolValue("IsPrintDateTime");
        }

        bool IsTime() const
        {
            return GetBoolValue("IsPrintDateTime");
        }

        bool IsHiddenPages() const
        {
            return GetBoolValue("IsPrintHidden");
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

        bool IsPrintExcluded() const
        {
            return (IsNotes() || IsDraw() || IsHandout()) &&  IsHiddenPages();
        }

        bool IsPrintFrontPage() const
        {
            sal_Int32 nInclude = static_cast<sal_Int32>(mrProperties.getIntValue( "PrintProspectInclude", 0 ));
            return nInclude == 0 || nInclude == 1;
        }

        bool IsPrintBackPage() const
        {
            sal_Int32 nInclude = static_cast<sal_Int32>(mrProperties.getIntValue( "PrintProspectInclude", 0 ));
            return nInclude == 0 || nInclude == 2;
        }

        bool IsPaperBin() const
        {
            return GetBoolValue("PrintPaperFromSetup", false);
        }

        bool IsPrintMarkedOnly() const
        {
            return GetBoolValue("PrintContent", sal_Int32(2));
        }

        OUString GetPrinterSelection (sal_Int32 nPageCount, sal_Int32 nCurrentPageIndex) const
        {
            sal_Int32 nContent = static_cast<sal_Int32>(mrProperties.getIntValue( "PrintContent", 0 ));
            OUString sFullRange = ::rtl::OUStringBuffer()
                 .append(static_cast<sal_Int32>(1))
                 .append(static_cast<sal_Unicode>('-'))
                 .append(nPageCount).makeStringAndClear();

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
                return OUString::valueOf(nCurrentPageIndex + 1);
            }

            return OUString();
        }

    private:
        const vcl::PrinterOptionsHelper& mrProperties;
        const ::std::vector<sal_Int32> maSlidesPerPage;

        /** When the value of the property with name pName is a boolean then
            return its value. When the property is unknown then
            bDefaultValue is returned.  Otherwise <FALSE/> is returned.
        */
        bool GetBoolValue (
            const sal_Char* pName,
            const bool bDefaultValue = false) const
        {
            sal_Bool bValue = mrProperties.getBoolValue( pName, bDefaultValue );
            return bValue;
        }

        /** Return <TRUE/> when the value of the property with name pName is
            a string and its value equals pValue. When the property is
            unknown then bDefaultValue is returned.  Otherwise <FALSE/> is
            returned.
        */
        bool GetBoolValue (
            const sal_Char* pName,
            const sal_Char* pValue,
            const bool bDefaultValue = false) const
        {
            OUString sValue( mrProperties.getStringValue( pName ) );
            if (!sValue.isEmpty())
                return sValue.equalsAscii(pValue);
            else
                return bDefaultValue;
        }

        /** Return <TRUE/> when the value of the property with name pName is
            an integer and its value is nTriggerValue. Otherwise <FALSE/> is
            returned.
        */
        bool GetBoolValue (
            const sal_Char* pName,
            const sal_Int32 nTriggerValue) const
        {
            sal_Int32 nValue = static_cast<sal_Int32>(mrProperties.getIntValue( pName ));
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
            const Printer* pPrinter,
            const bool bPrintMarkedOnly)
            : mpPrinter(pPrinter),
              mnDrawMode(DRAWMODE_DEFAULT),
              msTimeDate(),
              msPageString(),
              maPrintSize(0,0),
              maPageSize(0,0),
              meOrientation(ORIENTATION_PORTRAIT),
              maMap(),
              mbPrintMarkedOnly(bPrintMarkedOnly)
        {}

        const Printer* mpPrinter;
        sal_uLong mnDrawMode;
        ::rtl::OUString msTimeDate;
        ::rtl::OUString msPageString;
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
        View* pView,
        const bool bPrintMarkedOnly,
        const SetOfByte& rVisibleLayers,
        const SetOfByte& rPrintableLayers)
    {
        rPrintView.ShowSdrPage(&rPage);

        const MapMode aOriginalMapMode (rPrinter.GetMapMode());

        // Set the visible layers
        SdrPageView* pPageView = rPrintView.GetSdrPageView();
        OSL_ASSERT(pPageView!=NULL);
        pPageView->SetVisibleLayers(rVisibleLayers);
        pPageView->SetPrintableLayers(rPrintableLayers);

        if (pView!=NULL && bPrintMarkedOnly)
            pView->DrawMarkedObj(rPrinter);
        else
            rPrintView.CompleteRedraw(&rPrinter, Rectangle(Point(0,0), rPage.GetSize()));

        rPrinter.SetMapMode(aOriginalMapMode);

        rPrintView.HideSdrPage();
    }




    /** Output a string (that typically is not part of a document page) to
        the given printer.
    */
    void PrintMessage (
        Printer& rPrinter,
        const ::rtl::OUString& rsPageString,
        const Point& rPageStringOffset)
    {
        const Font aOriginalFont (rPrinter.OutputDevice::GetFont());
        rPrinter.SetFont(Font(FAMILY_SWISS, Size(0, 423)));
        rPrinter.DrawText(rPageStringOffset, rsPageString);
        rPrinter.SetFont(aOriginalFont);
    }




    /** Read the resource file and process it into a sequence of properties
        that can be passed to the printing dialog.
    */
    class DialogCreator : Resource
    {
    public:
        DialogCreator (bool bImpress, sal_Int32 nCurPage)
            : Resource(SdResId(_STR_IMPRESS_PRINT_UI_OPTIONS))
            , mbImpress(bImpress)
            , mnCurPage(nCurPage)
        {
            ProcessResource();
        }

        Sequence< beans::PropertyValue > GetDialogControls() const
        {
            if (maProperties.empty())
                return Sequence< beans::PropertyValue >();
            else
            {
                return Sequence<beans::PropertyValue>(
                        &maProperties.front(),
                        maProperties.size());
            }
        }

        ::std::vector<sal_Int32> GetSlidesPerPage() const
        {
            return maSlidesPerPage;
        }

    private:
        Any maDialog;
        ::std::vector<beans::PropertyValue> maProperties;
        ::std::vector<sal_Int32> maSlidesPerPage;
        bool mbImpress;
        sal_Int32 mnCurPage;

        void ProcessResource()
        {
            // load the writer PrinterOptions into the custom tab
            beans::PropertyValue aOptionsUIFile;
            aOptionsUIFile.Name = rtl::OUString("OptionsUIFile");
            if( mbImpress )
                aOptionsUIFile.Value <<= rtl::OUString("modules/simpress/ui/printeroptions.ui");
            else
                aOptionsUIFile.Value <<= rtl::OUString("modules/sdraw/ui/printeroptions.ui");
            maProperties.push_back(aOptionsUIFile);

            SvtModuleOptions aOpt;
            String aAppGroupname( String( SdResId( _STR_IMPRESS_PRINT_UI_GROUP_NAME ) ) );
            aAppGroupname.SearchAndReplace( String( RTL_CONSTASCII_USTRINGPARAM( "%s" ) ),
                                           aOpt.GetModuleName( mbImpress ? SvtModuleOptions::E_SIMPRESS : SvtModuleOptions::E_SDRAW ) );
            AddDialogControl(vcl::PrinterOptionsHelper::setGroupControlOpt("tabcontrol-page2", aAppGroupname, ".HelpID:vcl:PrintDialog:TabPage:AppPage"));

            uno::Sequence< rtl::OUString > aHelpIds, aWidgetIds;
            if( mbImpress )
            {
                vcl::PrinterOptionsHelper::UIControlOptions aPrintOpt;
                aPrintOpt.maGroupHint = "JobPage" ;
                AddDialogControl( vcl::PrinterOptionsHelper::setSubgroupControlOpt("extraimpressprintoptions",
                                    String( SdResId(_STR_IMPRESS_PRINT_UI_PRINT_GROUP) ),
                                    "",
                                    aPrintOpt ));

                aHelpIds.realloc( 1 );
                aHelpIds[0] = ".HelpID:vcl:PrintDialog:PageContentType:ListBox" ;
                AddDialogControl( vcl::PrinterOptionsHelper::setChoiceListControlOpt(
                                    "impressdocument",
                                    String( SdResId( _STR_IMPRESS_PRINT_UI_CONTENT ) ),
                                    aHelpIds,
                                    "PageContentType" ,
                                    CreateChoice(_STR_IMPRESS_PRINT_UI_CONTENT_CHOICES),
                                    0)
                                );

                aHelpIds[0] = ".HelpID:vcl:PrintDialog:SlidesPerPage:ListBox" ;
                vcl::PrinterOptionsHelper::UIControlOptions aContentOpt( "PageContentType" , 1 );
                AddDialogControl( vcl::PrinterOptionsHelper::setChoiceListControlOpt(
                                    "slidesperpage",
                                    String( SdResId( _STR_IMPRESS_PRINT_UI_SLIDESPERPAGE ) ),
                                    aHelpIds,
                                    "SlidesPerPage" ,
                                    GetSlidesPerPageSequence(),
                                    0,
                                    Sequence< sal_Bool >(),
                                    aContentOpt
                                    )
                                );

                aHelpIds[0] = ".HelpID:vcl:PrintDialog:SlidesPerPageOrder:ListBox" ;
                vcl::PrinterOptionsHelper::UIControlOptions aSlidesPerPageOpt( "SlidesPerPage" , -1, sal_True );
                AddDialogControl( vcl::PrinterOptionsHelper::setChoiceListControlOpt(
                                    "slidesperpageorder",
                                    String( SdResId( _STR_IMPRESS_PRINT_UI_ORDER ) ),
                                    aHelpIds,
                                    "SlidesPerPageOrder" ,
                                    CreateChoice(_STR_IMPRESS_PRINT_UI_ORDER_CHOICES),
                                    0,
                                    Sequence< sal_Bool >(),
                                    aSlidesPerPageOpt )
                                );
            }

            AddDialogControl( vcl::PrinterOptionsHelper::setSubgroupControlOpt("contents",
                               String( SdResId(_STR_IMPRESS_PRINT_UI_INCLUDE_CONTENT) ), "" ) );


            if( mbImpress )
            {
                AddDialogControl( vcl::PrinterOptionsHelper::setBoolControlOpt("printname",
                                    String( SdResId(_STR_IMPRESS_PRINT_UI_IS_PRINT_NAME) ),
                                    ".HelpID:vcl:PrintDialog:IsPrintName:CheckBox" ,
                                    "IsPrintName" ,
                                    sal_False
                                    )
                                );
            }
            else
            {
                AddDialogControl( vcl::PrinterOptionsHelper::setBoolControlOpt("printname",
                                    String( SdResId(_STR_DRAW_PRINT_UI_IS_PRINT_NAME) ),
                                    ".HelpID:vcl:PrintDialog:IsPrintName:CheckBox" ,
                                    "IsPrintName" ,
                                    sal_False
                                    )
                                );
            }

            AddDialogControl( vcl::PrinterOptionsHelper::setBoolControlOpt("printdatetime",
                                String( SdResId(_STR_IMPRESS_PRINT_UI_IS_PRINT_DATE) ),
                                ".HelpID:vcl:PrintDialog:IsPrintDateTime:CheckBox" ,
                                "IsPrintDateTime" ,
                                sal_False
                                )
                            );

            if( mbImpress )
            {
                AddDialogControl( vcl::PrinterOptionsHelper::setBoolControlOpt("printhidden",
                                    String( SdResId(_STR_IMPRESS_PRINT_UI_IS_PRINT_HIDDEN) ),
                                    ".HelpID:vcl:PrintDialog:IsPrintHidden:CheckBox" ,
                                    "IsPrintHidden" ,
                                    sal_False
                                    )
                                );
            }

            AddDialogControl( vcl::PrinterOptionsHelper::setSubgroupControlOpt("color",
                               String( SdResId(_STR_IMPRESS_PRINT_UI_QUALITY) ), "" ) );

            aHelpIds.realloc( 3 );
            aHelpIds[0] = ".HelpID:vcl:PrintDialog:Quality:RadioButton:0" ;
            aHelpIds[1] = ".HelpID:vcl:PrintDialog:Quality:RadioButton:1" ;
            aHelpIds[2] = ".HelpID:vcl:PrintDialog:Quality:RadioButton:2" ;
            aWidgetIds.realloc( 3 );
            aWidgetIds[0] = "originalcolors";
            aWidgetIds[1] = "grayscale";
            aWidgetIds[2] = "blackandwhite";
            AddDialogControl( vcl::PrinterOptionsHelper::setChoiceRadiosControlOpt(
                                aWidgetIds,
                                "",
                                aHelpIds,
                                "Quality" ,
                                CreateChoice(_STR_IMPRESS_PRINT_UI_QUALITY_CHOICES),
                                0)
                            );

            AddDialogControl( vcl::PrinterOptionsHelper::setSubgroupControlOpt("pagesizes",
                               String( SdResId(_STR_IMPRESS_PRINT_UI_PAGE_OPTIONS) ), "" ) );

            aHelpIds.realloc( 4 );
            aHelpIds[0] = ".HelpID:vcl:PrintDialog:PageOptions:RadioButton:0" ;
            aHelpIds[1] = ".HelpID:vcl:PrintDialog:PageOptions:RadioButton:1" ;
            aHelpIds[2] = ".HelpID:vcl:PrintDialog:PageOptions:RadioButton:2" ;
            aHelpIds[3] = ".HelpID:vcl:PrintDialog:PageOptions:RadioButton:3" ;
            aWidgetIds.realloc( 4 );
            aWidgetIds[0] = "originalsize";
            aWidgetIds[1] = "fittoprintable";
            aWidgetIds[2] = "distributeonmultiple";
            aWidgetIds[3] = "tilesheet";
            if( mbImpress )
            {
                // FIXME: additional dependency on PrintProspect = false
                vcl::PrinterOptionsHelper::UIControlOptions aPageOptionsOpt( "PageContentType" , 0 );
                AddDialogControl( vcl::PrinterOptionsHelper::setChoiceRadiosControlOpt(
                                    aWidgetIds,
                                    "",
                                    aHelpIds,
                                    "PageOptions" ,
                                    CreateChoice(_STR_IMPRESS_PRINT_UI_PAGE_OPTIONS_CHOICES),
                                    0,
                                    Sequence< sal_Bool >(),
                                    aPageOptionsOpt
                                    )
                                );
            }
            else
            {
                vcl::PrinterOptionsHelper::UIControlOptions aPageOptionsOpt( "PrintProspect" , sal_False );
                AddDialogControl( vcl::PrinterOptionsHelper::setChoiceRadiosControlOpt(
                                    aWidgetIds,
                                    "",
                                    aHelpIds,
                                    "PageOptions" ,
                                    CreateChoice(_STR_IMPRESS_PRINT_UI_PAGE_OPTIONS_CHOICES_DRAW),
                                    0,
                                    Sequence< sal_Bool >(),
                                    aPageOptionsOpt
                                    )
                                );
            }

            vcl::PrinterOptionsHelper::UIControlOptions aBrochureOpt;
            aBrochureOpt.maGroupHint = "LayoutPage" ;
            AddDialogControl( vcl::PrinterOptionsHelper::setSubgroupControlOpt("pagesides",
                               String( SdResId(_STR_IMPRESS_PRINT_UI_PAGE_SIDES) ), "",
                               aBrochureOpt ) );

            // brochure printing
            AddDialogControl( vcl::PrinterOptionsHelper::setBoolControlOpt("brochure",
                                String( SdResId(_STR_IMPRESS_PRINT_UI_BROCHURE) ),
                                ".HelpID:vcl:PrintDialog:PrintProspect:CheckBox" ,
                                "PrintProspect" ,
                                sal_False,
                                aBrochureOpt
                                )
                            );

            vcl::PrinterOptionsHelper::UIControlOptions
                aIncludeOpt( "PrintProspect" , -1, sal_False );
            aIncludeOpt.maGroupHint =  "LayoutPage" ;
            aHelpIds.realloc( 1 );
            aHelpIds[0] = ".HelpID:vcl:PrintDialog:PrintProspectInclude:ListBox" ;
            AddDialogControl( vcl::PrinterOptionsHelper::setChoiceListControlOpt(
                                "brochureinclude",
                                String( SdResId(_STR_IMPRESS_PRINT_UI_BROCHURE_INCLUDE) ),
                                aHelpIds,
                                "PrintProspectInclude" ,
                                CreateChoice(_STR_IMPRESS_PRINT_UI_BROCHURE_INCLUDE_LIST),
                                0,
                                Sequence< sal_Bool >(),
                                aIncludeOpt
                                )
                            );

            // paper tray (on options page)
            vcl::PrinterOptionsHelper::UIControlOptions aPaperTrayOpt;
            aPaperTrayOpt.maGroupHint = "OptionsPageOptGroup" ;
            AddDialogControl( vcl::PrinterOptionsHelper::setBoolControlOpt("printpaperfromsetup",
                                String( SdResId(_STR_IMPRESS_PRINT_UI_PAPER_TRAY) ),
                                ".HelpID:vcl:PrintDialog:PrintPaperFromSetup:CheckBox" ,
                                "PrintPaperFromSetup" ,
                                sal_False,
                                aPaperTrayOpt
                                )
                            );
            // print range selection
            vcl::PrinterOptionsHelper::UIControlOptions aPrintRangeOpt;
            aPrintRangeOpt.mbInternalOnly = sal_True;
            aPrintRangeOpt.maGroupHint = "PrintRange" ;
            AddDialogControl( vcl::PrinterOptionsHelper::setSubgroupControlOpt("printrange",
                                String( SdResId( _STR_IMPRESS_PRINT_UI_PAGE_RANGE ) ),
                                "",
                                aPrintRangeOpt )
                             );

            // create a choice for the content to create
            rtl::OUString aPrintRangeName( "PrintContent" );
            aHelpIds.realloc( 3 );
            aHelpIds[0] = ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:0" ;
            aHelpIds[1] = ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:1" ;
            aHelpIds[2] = ".HelpID:vcl:PrintDialog:PrintContent:RadioButton:2" ;
            aWidgetIds.realloc( 3 );
            aWidgetIds[0] = "printallpages";
            aWidgetIds[1] = "printpages";
            aWidgetIds[2] = "printselection";
            AddDialogControl( vcl::PrinterOptionsHelper::setChoiceRadiosControlOpt(aWidgetIds, "",
                                aHelpIds,
                                aPrintRangeName,
                                CreateChoice(mbImpress
                                             ? _STR_IMPRESS_PRINT_UI_PAGE_RANGE_CHOICE
                                             : _STR_DRAW_PRINT_UI_PAGE_RANGE_CHOICE),
                                0 )
                            );
            // create a an Edit dependent on "Pages" selected
            vcl::PrinterOptionsHelper::UIControlOptions aPageRangeOpt( aPrintRangeName, 1, sal_True );
            AddDialogControl(vcl::PrinterOptionsHelper::setEditControlOpt("pagerange", "",
                                ".HelpID:vcl:PrintDialog:PageRange:Edit", "PageRange",
                                OUString::valueOf(mnCurPage + 1), aPageRangeOpt));

            FreeResource();
        }

        void AddDialogControl( const Any& i_rCtrl )
        {
            beans::PropertyValue aVal;
            aVal.Value = i_rCtrl;
            maProperties.push_back( aVal );
        }

        Sequence<rtl::OUString> CreateChoice (const sal_uInt16 nResourceId) const
        {
            SdResId aResourceId (nResourceId);
            ResStringArray aChoiceStrings (aResourceId);

            const sal_uInt32 nCount (aChoiceStrings.Count());
            Sequence<rtl::OUString> aChoices (nCount);
            for (sal_uInt32 nIndex=0; nIndex<nCount; ++nIndex)
                aChoices[nIndex] = aChoiceStrings.GetString(nIndex);

            return aChoices;
        }

        Sequence<rtl::OUString> GetSlidesPerPageSequence()
        {
            const Sequence<rtl::OUString> aChoice (
                CreateChoice(_STR_IMPRESS_PRINT_UI_SLIDESPERPAGE_CHOICES));
            maSlidesPerPage.clear();
            maSlidesPerPage.push_back(0); // first is using the default
            for (sal_Int32 nIndex=1,nCount=aChoice.getLength(); nIndex<nCount; ++nIndex)
                maSlidesPerPage.push_back(aChoice[nIndex].toInt32());
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
            const ::rtl::OUString& rsPageString,
            const Point& rPageStringOffset,
            const sal_uLong nDrawMode,
            const Orientation eOrientation,
            const sal_uInt16 nPaperTray)
            : mePageKind(ePageKind),
              maMap(rMapMode),
              mbPrintMarkedOnly(bPrintMarkedOnly),
              msPageString(rsPageString),
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
            const SetOfByte& rVisibleLayers,
            const SetOfByte& rPrintableLayers) const = 0;

        sal_uLong GetDrawMode() const { return mnDrawMode; }
        Orientation GetOrientation() const { return meOrientation; }
        sal_uInt16 GetPaperTray() const { return mnPaperTray; }

    protected:
        const PageKind mePageKind;
        const MapMode maMap;
        const bool mbPrintMarkedOnly;
        const ::rtl::OUString msPageString;
        const Point maPageStringOffset;
        const sal_uLong mnDrawMode;
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
            const ::rtl::OUString& rsPageString,
            const Point& rPageStringOffset,
            const sal_uLong nDrawMode,
            const Orientation eOrientation,
            const sal_uInt16 nPaperTray)
            : PrinterPage(ePageKind, rMapMode, bPrintMarkedOnly, rsPageString,
                rPageStringOffset, nDrawMode, eOrientation, nPaperTray),
              mnPageIndex(nPageIndex)
        {
        }

        virtual ~RegularPrinterPage() {}

        virtual void Print (
            Printer& rPrinter,
            SdDrawDocument& rDocument,
            ViewShell& rViewShell,
            View* pView,
            DrawView& rPrintView,
            const SetOfByte& rVisibleLayers,
            const SetOfByte& rPrintableLayers) const
        {
            (void)rViewShell;
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
            const sal_Int32 nGap,
            const bool bPrintMarkedOnly,
            const ::rtl::OUString& rsPageString,
            const Point& rPageStringOffset,
            const sal_uLong nDrawMode,
            const Orientation eOrientation,
            const sal_uInt16 nPaperTray)
            : PrinterPage(ePageKind, MapMode(), bPrintMarkedOnly, rsPageString,
                rPageStringOffset, nDrawMode, eOrientation, nPaperTray),
              mnPageIndex(nPageIndex),
              mnGap(nGap)
        {
        }

        virtual ~TiledPrinterPage() {}

        virtual void Print (
            Printer& rPrinter,
            SdDrawDocument& rDocument,
            ViewShell& rViewShell,
            View* pView,
            DrawView& rPrintView,
            const SetOfByte& rVisibleLayers,
            const SetOfByte& rPrintableLayers) const
        {
            (void)rViewShell;
            SdPage* pPageToPrint = rDocument.GetSdPage(mnPageIndex, mePageKind);
            if (pPageToPrint==NULL)
                return;
            MapMode aMap (rPrinter.GetMapMode());

            const Size aPageSize (pPageToPrint->GetSize());
            const Size aPrintSize (rPrinter.GetOutputSize());

            const sal_Int32 nPageWidth (aPageSize.Width() + mnGap
                - pPageToPrint->GetLftBorder() - pPageToPrint->GetRgtBorder());
            const sal_Int32 nPageHeight (aPageSize.Height() + mnGap
                - pPageToPrint->GetUppBorder() - pPageToPrint->GetLwrBorder());
            if (nPageWidth<=0 || nPageHeight<=0)
                return;

            // Print at least two rows and columns.  More if the document
            // page fits completely onto the printer page.
            const sal_Int32 nColumnCount (::std::max(sal_Int32(2),
                    sal_Int32(aPrintSize.Width() / nPageWidth)));
            const sal_Int32 nRowCount (::std::max(sal_Int32(2),
                    sal_Int32(aPrintSize.Height() / nPageHeight)));
            Point aPrintOrigin;
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
        const sal_Int32 mnGap;
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
            const sal_uLong nDrawMode,
            const Orientation eOrientation,
            const sal_uInt16 nPaperTray)
            : PrinterPage(ePageKind, rMapMode, bPrintMarkedOnly, "",
                Point(), nDrawMode, eOrientation, nPaperTray),
              mnFirstPageIndex(nFirstPageIndex),
              mnSecondPageIndex(nSecondPageIndex),
              maFirstOffset(rFirstOffset),
              maSecondOffset(rSecondOffset)
        {
        }

        virtual ~BookletPrinterPage() {}

        virtual void Print (
            Printer& rPrinter,
            SdDrawDocument& rDocument,
            ViewShell& rViewShell,
            View* pView,
            DrawView& rPrintView,
            const SetOfByte& rVisibleLayers,
            const SetOfByte& rPrintableLayers) const
        {
            (void)rViewShell;
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
            if( pPageToPrint )
            {
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
            const ::std::vector<sal_uInt16>& rPageIndices,
            const MapMode& rMapMode,
            const ::rtl::OUString& rsPageString,
            const Point& rPageStringOffset,
            const sal_uLong nDrawMode,
            const Orientation eOrientation,
            const sal_uInt16 nPaperTray)
            : PrinterPage(PK_HANDOUT, rMapMode, false, rsPageString,
                rPageStringOffset, nDrawMode, eOrientation, nPaperTray),
              mnHandoutPageIndex(nHandoutPageIndex),
              maPageIndices(rPageIndices)
        {
        }

        virtual void Print (
            Printer& rPrinter,
            SdDrawDocument& rDocument,
            ViewShell& rViewShell,
            View* pView,
            DrawView& rPrintView,
            const SetOfByte& rVisibleLayers,
            const SetOfByte& rPrintableLayers) const
        {
            SdPage& rHandoutPage (*rDocument.GetSdPage(0, PK_HANDOUT));

            Reference< com::sun::star::beans::XPropertySet > xHandoutPage( rHandoutPage.getUnoPage(), UNO_QUERY );
            const rtl::OUString sPageNumber( "Number" );

            // Collect the page objects of the handout master.
            std::vector<SdrPageObj*> aHandoutPageObjects;
            SdrObjListIter aShapeIter (rHandoutPage);
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
                if (*iPageIndex >= rDocument.GetSdPageCount(PK_STANDARD))
                    continue;

                SdrPageObj* pPageObj = (*aPageObjIter++);
                pPageObj->SetReferencedPage(rDocument.GetSdPage(*iPageIndex, PK_STANDARD));
            }

            // if there are more page objects than pages left, set the rest to invisible
            int nHangoverCount = 0;
            while (aPageObjIter != aHandoutPageObjects.end())
            {
                (*aPageObjIter++)->SetReferencedPage(0L);
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
                            pPathObj->SetMergedItem(XLineStyleItem(XLINE_NONE));
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

            MapMode aMap (rPrinter.GetMapMode());
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
                    if (pPathObj != NULL)
                        pPathObj->SetMergedItem(XLineStyleItem(XLINE_SOLID));
                }
            }

       }

    private:
        const sal_uInt16 mnHandoutPageIndex;
        const ::std::vector<sal_uInt16> maPageIndices;
    };




    /** The outline information (title, subtitle, outline objects) of the
        document.  There is no fixed mapping of slides to printer pages.
    */
    class OutlinerPrinterPage : public PrinterPage
    {
    public:
        OutlinerPrinterPage (
            OutlinerParaObject* pParaObject,
            const MapMode& rMapMode,
            const ::rtl::OUString& rsPageString,
            const Point& rPageStringOffset,
            const sal_uLong nDrawMode,
            const Orientation eOrientation,
            const sal_uInt16 nPaperTray)
            : PrinterPage(PK_HANDOUT, rMapMode, false, rsPageString,
                rPageStringOffset, nDrawMode, eOrientation, nPaperTray),
              mpParaObject(pParaObject)
        {
        }

        ~OutlinerPrinterPage()
        {
            mpParaObject.reset();
        }

        virtual void Print (
            Printer& rPrinter,
            SdDrawDocument& rDocument,
            ViewShell& rViewShell,
            View* pView,
            DrawView& rPrintView,
            const SetOfByte& rVisibleLayers,
            const SetOfByte& rPrintableLayers) const
        {
            (void)rViewShell;
            (void)pView;
            (void)rPrintView;
            (void)rVisibleLayers;
            (void)rPrintableLayers;

            // Set up the printer.
            rPrinter.SetMapMode(maMap);

            // Get and set up the outliner.
            const Rectangle aOutRect (rPrinter.GetPageOffset(), rPrinter.GetOutputSize());
            Outliner* pOutliner = rDocument.GetInternalOutliner();
            const sal_uInt16 nSavedOutlMode (pOutliner->GetMode());
            const sal_Bool bSavedUpdateMode (pOutliner->GetUpdateMode());
            const Size aSavedPaperSize (pOutliner->GetPaperSize());

            pOutliner->Init(OUTLINERMODE_OUTLINEVIEW);
            pOutliner->SetPaperSize(aOutRect.GetSize());
            pOutliner->SetUpdateMode(sal_True);
            pOutliner->Clear();
            pOutliner->SetText(*mpParaObject);

            pOutliner->Draw(&rPrinter, aOutRect);

            PrintMessage(
                rPrinter,
                msPageString,
                maPageStringOffset);

            // Restore outliner and printer.
            pOutliner->Clear();
            pOutliner->SetUpdateMode(bSavedUpdateMode);
            pOutliner->SetPaperSize(aSavedPaperSize);
            pOutliner->Init(nSavedOutlMode);
        }

    private:
        ::boost::scoped_ptr<OutlinerParaObject> mpParaObject;
    };
}


//===== DocumentRenderer::Implementation ======================================

class DocumentRenderer::Implementation
    : public SfxListener,
      public vcl::PrinterOptionsHelper
{
public:
    Implementation (ViewShellBase& rBase)
        : mxObjectShell(rBase.GetDocShell())
        , mrBase(rBase)
        , mbIsDisposed(false)
        , mpPrinter(NULL)
        , mpOptions()
        , maPrinterPages()
        , mpPrintView()
        , mbHasOrientationWarningBeenShown(false)
    {
        DialogCreator aCreator( mrBase.GetDocShell()->GetDocumentType() == DOCUMENT_TYPE_IMPRESS, GetCurrentPageIndex() );
        m_aUIProperties = aCreator.GetDialogControls();
        maSlidesPerPage = aCreator.GetSlidesPerPage();

        StartListening(mrBase);
    }




    virtual ~Implementation()
    {
        EndListening(mrBase);
    }




    virtual void Notify (SfxBroadcaster& rBroadcaster, const SfxHint& rHint)
    {
        const SfxSimpleHint* pSimpleHint = dynamic_cast<const SfxSimpleHint*>(&rHint);
        if (pSimpleHint != NULL
            && pSimpleHint->GetId() == SFX_HINT_DYING
            && &rBroadcaster == &static_cast<SfxBroadcaster&>(mrBase))
        {
            Dispose();
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
        Any aDev( getValue( "RenderDevice" ) );
        Reference<awt::XDevice> xRenderDevice;

        if (aDev >>= xRenderDevice)
        {
            VCLXDevice* pDevice = VCLXDevice::GetImplementation(xRenderDevice);
            OutputDevice* pOut = pDevice ? pDevice->GetOutputDevice() : NULL;
            mpPrinter = dynamic_cast<Printer*>(pOut);
            Size aPageSizePixel = mpPrinter ? mpPrinter->GetPaperSizePixel() : Size();
            if( aPageSizePixel != maPrinterPageSizePixel )
            {
                bIsPaperChanged = true;
                maPrinterPageSizePixel = aPageSizePixel;
            }
        }

        if (bIsValueChanged)
        {
            if ( ! mpOptions )
                mpOptions.reset(new PrintOptions(*this, maSlidesPerPage));
        }
        if( bIsValueChanged || bIsPaperChanged )
            PreparePages();
    }



    /** Return the number of pages that are to be printed.
    */
    sal_Int32 GetPrintPageCount()
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
    css::uno::Sequence<css::beans::PropertyValue> GetProperties (
        const css::uno::Sequence<css::beans::PropertyValue>& rOptions)
    {
        (void)rOptions;

        css::uno::Sequence<css::beans::PropertyValue> aProperties (3);

        aProperties[0].Name = "ExtraPrintUIOptions";
        aProperties[0].Value <<= m_aUIProperties;

        aProperties[1].Name = "PageSize";
        aProperties[1].Value <<= maPrintSize;

        // FIXME: is this always true ?
        aProperties[2].Name = "PageIncludesNonprintableArea";
        aProperties[2].Value = makeAny( sal_True );

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

        ::boost::shared_ptr<ViewShell> pViewShell (mrBase.GetMainViewShell());
        if ( ! pViewShell)
            return;

        SdDrawDocument* pDocument = pViewShell->GetDoc();
        OSL_ASSERT(pDocument!=NULL);

        ::boost::shared_ptr<DrawViewShell> pDrawViewShell(
            ::boost::dynamic_pointer_cast<DrawViewShell>(mrBase.GetMainViewShell()));

        if ( ! mpPrintView)
            mpPrintView.reset(new DrawView(mrBase.GetDocShell(), &rPrinter, NULL));

        if (nIndex<0 || sal::static_int_cast<sal_uInt32>(nIndex)>=maPrinterPages.size())
            return;

        const ::boost::shared_ptr<PrinterPage> pPage (maPrinterPages[nIndex]);
        OSL_ASSERT(pPage);
        if ( ! pPage)
            return;

        const Orientation eSavedOrientation (rPrinter.GetOrientation());
        const sal_uLong nSavedDrawMode (rPrinter.GetDrawMode());
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
                if (pViewShell)
                {
                    WarningBox aWarnBox(
                        pViewShell->GetActiveWindow(),
                        (WinBits)(WB_OK_CANCEL | WB_DEF_CANCEL),
                        String(SdResId(STR_WARN_PRINTFORMAT_FAILURE)));
                    if (aWarnBox.Execute() != RET_OK)
                        return;
                }
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
            pDrawViewShell ? pDrawViewShell->GetView() : NULL,
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
    Printer* mpPrinter;
    Size maPrinterPageSizePixel;
    ::boost::scoped_ptr<PrintOptions> mpOptions;
    ::std::vector< ::boost::shared_ptr< ::sd::PrinterPage> > maPrinterPages;
    ::boost::scoped_ptr<DrawView> mpPrintView;
    bool mbHasOrientationWarningBeenShown;
    ::std::vector<sal_Int32> maSlidesPerPage;
    awt::Size maPrintSize;

    void Dispose()
    {
        mbIsDisposed = true;
    }

    sal_Int32 GetCurrentPageIndex() const
    {
        const ViewShell *pShell = mrBase.GetMainViewShell().get();
        const SdPage *pCurrentPage = pShell ? pShell->getCurrentPage() : NULL;
        return pCurrentPage ? (pCurrentPage->GetPageNum()-1)/2 : -1;
    }

    /** Determine and set the paper orientation.
    */
    bool SetupPaperOrientation (
        const PageKind ePageKind,
        PrintInfo& rInfo)
    {
        SdDrawDocument* pDocument = mrBase.GetMainViewShell()->GetDoc();
        rInfo.meOrientation = ORIENTATION_PORTRAIT;

        if( ! mpOptions->IsBooklet())
        {
            rInfo.meOrientation = pDocument->GetSdPage(0, ePageKind)->GetOrientation();
        }
        else if (rInfo.maPageSize.Width() < rInfo.maPageSize.Height())
            rInfo.meOrientation = ORIENTATION_LANDSCAPE;

        const Size aPaperSize (rInfo.mpPrinter->GetPaperSize());
        if( (rInfo.meOrientation == ORIENTATION_LANDSCAPE &&
              (aPaperSize.Width() < aPaperSize.Height()))
           ||
            (rInfo.meOrientation == ORIENTATION_PORTRAIT &&
              (aPaperSize.Width() > aPaperSize.Height()))
          )
        {
            maPrintSize = awt::Size(aPaperSize.Height(), aPaperSize.Width());
        }
        else
        {
            maPrintSize = awt::Size(aPaperSize.Width(), aPaperSize.Height());
        }

        return true;
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

        if (aInfo.mpPrinter!=NULL && pShell!=NULL)
        {

            MapMode aMap (aInfo.mpPrinter->GetMapMode());
            aMap.SetMapUnit(MAP_100TH_MM);
            aInfo.maMap = aMap;
            mpPrinter->SetMapMode(aMap);

            ::Outliner& rOutliner = mrBase.GetDocument()->GetDrawOutliner();
            const sal_uLong nSavedControlWord (rOutliner.GetControlWord());
            sal_uLong nCntrl = nSavedControlWord;
            nCntrl &= ~EE_CNTRL_MARKFIELDS;
            nCntrl &= ~EE_CNTRL_ONLINESPELLING;
            rOutliner.SetControlWord( nCntrl );

            // When in outline view then apply all pending changes to the model.
            if (pShell->ISA(OutlineViewShell))
                static_cast<OutlineViewShell*>(pShell)->PrepareClose (sal_False, sal_False);

            // Collect some frequently used data.
            if (mpOptions->IsDate())
            {
                aInfo.msTimeDate += GetSdrGlobalData().GetLocaleData()->getDate( Date( Date::SYSTEM ) );
                aInfo.msTimeDate += ::rtl::OUString((sal_Unicode)' ');
            }

            if (mpOptions->IsTime())
                aInfo.msTimeDate += GetSdrGlobalData().GetLocaleData()->getTime( Time( Time::SYSTEM ), sal_False, sal_False );
            aInfo.maPrintSize = aInfo.mpPrinter->GetOutputSize();
            maPrintSize = awt::Size(
                aInfo.mpPrinter->GetPaperSize().Width(),
                aInfo.mpPrinter->GetPaperSize().Height());

            switch (mpOptions->GetOutputQuality())
            {
                case 1:
                    aInfo.mnDrawMode = DRAWMODE_GRAYLINE | DRAWMODE_GRAYFILL
                        | DRAWMODE_GRAYTEXT | DRAWMODE_GRAYBITMAP
                        | DRAWMODE_GRAYGRADIENT;
                    break;

                case 2:
                    aInfo.mnDrawMode = DRAWMODE_BLACKLINE | DRAWMODE_WHITEFILL
                        | DRAWMODE_BLACKTEXT | DRAWMODE_WHITEBITMAP
                        | DRAWMODE_WHITEGRADIENT;
                    break;

                default:
                    aInfo.mnDrawMode = DRAWMODE_DEFAULT;
            }

            if (mpOptions->IsDraw())
                PrepareStdOrNotes(PK_STANDARD, aInfo);
            if (mpOptions->IsNotes())
                PrepareStdOrNotes(PK_NOTES, aInfo);
            if (mpOptions->IsHandout())
            {
                InitHandoutTemplate();
                PrepareHandout(aInfo);
            }
            if (mpOptions->IsOutline())
                PrepareOutline(aInfo);

            rOutliner.SetControlWord(nSavedControlWord);
        }
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

        SdPage* pHandout = rModel.GetSdPage(0, PK_HANDOUT);
        if( !pHandout )
            return;

        // delete all previous shapes from handout page
        while( pHandout->GetObjCount() )
        {
            SdrObject* pObj = pHandout->NbcRemoveObject(0);
            if( pObj )
                SdrObject::Free( pObj  );
        }

        const bool bDrawLines (eLayout == AUTOLAYOUT_HANDOUT3);

        std::vector< Rectangle > aAreas;
        SdPage::CalculateHandoutAreas( rModel, eLayout, bHandoutHorizontal, aAreas );

        std::vector< Rectangle >::iterator iter( aAreas.begin() );
        while( iter != aAreas.end() )
        {
            pHandout->NbcInsertObject( new SdrPageObj((*iter++)) );

            if( bDrawLines && (iter != aAreas.end())  )
            {
                Rectangle aRect( (*iter++) );

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

                SdrPathObj* pPathObj = new SdrPathObj(OBJ_PATHLINE, aPathPoly );
                pPathObj->SetMergedItem(XLineStyleItem(XLINE_SOLID));
                pPathObj->SetMergedItem(XLineColorItem(String(), Color(COL_BLACK)));

                pHandout->NbcInsertObject( pPathObj );
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
        OSL_ASSERT(mrBase.GetDocument() != NULL);
        OSL_ASSERT(nPageIndex>=0);
        SdPage* pPage = mrBase.GetDocument()->GetSdPage(
            sal::static_int_cast<sal_uInt16>(nPageIndex),
            ePageKind);
        if (pPage == NULL)
            return NULL;
        if ( ! pPage->IsExcluded() || mpOptions->IsPrintExcluded())
            return pPage;
        else
            return NULL;
    }




    /** Prepare the outline of the document for printing.  There is no fixed
        number of slides whose outline data is put onto one printer page.
        If the current printer page has enough room for the outline of the
        current slide then that is added.  Otherwise a new printer page is
        started.
    */
    void PrepareOutline (PrintInfo& rInfo)
    {
        MapMode aMap (rInfo.maMap);
        Point aPageOfs (rInfo.mpPrinter->GetPageOffset() );
        aMap.SetScaleX(Fraction(1,2));
        aMap.SetScaleY(Fraction(1,2));
        mpPrinter->SetMapMode(aMap);

        Rectangle aOutRect(aPageOfs, rInfo.mpPrinter->GetOutputSize());
        if( aOutRect.GetWidth() > aOutRect.GetHeight() )
        {
            Size aPaperSize( rInfo.mpPrinter->PixelToLogic( rInfo.mpPrinter->GetPaperSizePixel(), MapMode( MAP_100TH_MM ) ) );
            maPrintSize.Width  = aPaperSize.Height();
            maPrintSize.Height = aPaperSize.Width();
            aOutRect = Rectangle( Point( aPageOfs.Y(), aPageOfs.X() ),
                                  Size( aOutRect.GetHeight(), aOutRect.GetWidth() ) );
        }

        Link aOldLink;
        Outliner* pOutliner = mrBase.GetDocument()->GetInternalOutliner();
        pOutliner->Init(OUTLINERMODE_OUTLINEVIEW);
        const sal_uInt16 nSavedOutlMode (pOutliner->GetMode());
        const sal_Bool bSavedUpdateMode (pOutliner->GetUpdateMode());
        const Size aSavedPaperSize (pOutliner->GetPaperSize());
        const MapMode aSavedMapMode (pOutliner->GetRefMapMode());
        pOutliner->SetPaperSize(aOutRect.GetSize());
        pOutliner->SetUpdateMode(sal_True);

        long nPageH = aOutRect.GetHeight();

        ::std::vector< sal_Int32 > aPages;
        sal_Int32 nPageCount = mrBase.GetDocument()->GetSdPageCount(PK_STANDARD);
        StringRangeEnumerator::getRangesFromString(
            mpOptions->GetPrinterSelection(nPageCount, GetCurrentPageIndex()),
            aPages, 0, nPageCount-1);

        for (size_t nIndex = 0, nCount = aPages.size(); nIndex < nCount;)
        {
            pOutliner->Clear();
            pOutliner->SetFirstPageNumber(aPages[nIndex]+1);

            Paragraph* pPara = NULL;
            sal_Int32 nH (0);
            while (nH < nPageH && nIndex<nCount)
            {
                SdPage* pPage = GetFilteredPage(aPages[nIndex], PK_STANDARD);
                ++nIndex;
                if (pPage == NULL)
                    continue;

                SdrTextObj* pTextObj = NULL;
                sal_uInt32 nObj (0);

                while (pTextObj==NULL && nObj < pPage->GetObjCount())
                {
                    SdrObject* pObj = pPage->GetObj(nObj++);
                    if (pObj->GetObjInventor() == SdrInventor
                        && pObj->GetObjIdentifier() == OBJ_TITLETEXT)
                    {
                        pTextObj = dynamic_cast<SdrTextObj*>(pObj);
                    }
                }

                pPara = pOutliner->GetParagraph(pOutliner->GetParagraphCount() - 1);

                if (pTextObj!=NULL
                    && !pTextObj->IsEmptyPresObj()
                    && pTextObj->GetOutlinerParaObject())
                {
                    pOutliner->AddText(*(pTextObj->GetOutlinerParaObject()));
                }
                else
                    pOutliner->Insert(String());

                pTextObj = NULL;
                nObj = 0;

                while (pTextObj==NULL && nObj<pPage->GetObjCount())
                {
                    SdrObject* pObj = pPage->GetObj(nObj++);
                    if (pObj->GetObjInventor() == SdrInventor
                        && pObj->GetObjIdentifier() == OBJ_OUTLINETEXT)
                    {
                        pTextObj = dynamic_cast<SdrTextObj*>(pObj);
                    }
                }

                bool bSubTitle (false);
                if (!pTextObj)
                {
                    bSubTitle = true;
                    pTextObj = dynamic_cast<SdrTextObj*>(pPage->GetPresObj(PRESOBJ_TEXT));  // Untertitel vorhanden?
                }

                sal_uLong nParaCount1 = pOutliner->GetParagraphCount();

                if (pTextObj!=NULL
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
                        if (pP!=NULL && pOutliner->GetDepth((sal_uInt16)nPara) > 0)
                            pOutliner->SetDepth(pP, 0);
                    }
                }

                nH = pOutliner->GetTextHeight();
            }

            // Remove the last paragraph when that does not fit completely on
            // the current page.
            if (nH > nPageH && pPara!=NULL)
            {
                sal_uLong nCnt = pOutliner->GetAbsPos(
                    pOutliner->GetParagraph( pOutliner->GetParagraphCount() - 1 ) );
                sal_uLong nParaPos = pOutliner->GetAbsPos( pPara );
                nCnt -= nParaPos;
                pPara = pOutliner->GetParagraph( ++nParaPos );
                if ( nCnt && pPara )
                {
                    pOutliner->Remove(pPara, nCnt);
                    --nIndex;
                }
            }

            maPrinterPages.push_back(
                ::boost::shared_ptr<PrinterPage>(
                    new OutlinerPrinterPage(
                        pOutliner->CreateParaObject(),
                        aMap,
                        rInfo.msTimeDate,
                        aPageOfs,
                        rInfo.mnDrawMode,
                        rInfo.meOrientation,
                        rInfo.mpPrinter->GetPaperBin())));
        }

        pOutliner->SetRefMapMode(aSavedMapMode);
        pOutliner->SetUpdateMode(bSavedUpdateMode);
        pOutliner->SetPaperSize(aSavedPaperSize);
        pOutliner->Init(nSavedOutlMode);
    }




    /** Prepare handout pages for slides that are to be printed.
    */
    void PrepareHandout (PrintInfo& rInfo)
    {
        SdDrawDocument* pDocument = mrBase.GetDocument();
        OSL_ASSERT(pDocument != NULL);
        SdPage& rHandoutPage (*pDocument->GetSdPage(0, PK_HANDOUT));

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
        if( (rInfo.meOrientation == ORIENTATION_LANDSCAPE &&
              (aPaperSize.Width() < aPaperSize.Height()))
           ||
            (rInfo.meOrientation == ORIENTATION_PORTRAIT &&
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

            const double fHorz = (double) aPrintSize.Width()    / aPageSize.Width();
            const double fVert = (double) aPrintSize.Height() / aPageSize.Height();

            Fraction    aFract;
            if ( fHorz < fVert )
                aFract = Fraction(aPrintSize.Width(), aPageSize.Width());
            else
                aFract = Fraction(aPrintSize.Height(), aPageSize.Height());

            aMap.SetScaleX(aFract);
            aMap.SetScaleY(aFract);
            aMap.SetOrigin(Point());
        }

        ::boost::shared_ptr<ViewShell> pViewShell (mrBase.GetMainViewShell());
        pViewShell->WriteFrameViewData();

        // Count page shapes.
        sal_uInt32 nShapeCount (0);
        SdrObjListIter aShapeIter (rHandoutPage);
        while (aShapeIter.IsMore())
        {
            SdrPageObj* pPageObj = dynamic_cast<SdrPageObj*>(aShapeIter.Next());
            if (pPageObj)
                ++nShapeCount;
        }

        const sal_uInt16 nPageCount = mrBase.GetDocument()->GetSdPageCount(PK_STANDARD);
        const sal_uInt16 nHandoutPageCount = nShapeCount ? (nPageCount + nShapeCount - 1) / nShapeCount : 0;
        pViewShell->SetPrintedHandoutPageCount( nHandoutPageCount );
        mrBase.GetDocument()->setHandoutPageCount( nHandoutPageCount );

        // Distribute pages to handout pages.
        StringRangeEnumerator aRangeEnum(
            mpOptions->GetPrinterSelection(nPageCount, GetCurrentPageIndex()),
            0, nPageCount-1);
        ::std::vector<sal_uInt16> aPageIndices;
        sal_uInt16 nPrinterPageIndex = 0;
        StringRangeEnumerator::Iterator it = aRangeEnum.begin(), itEnd = aRangeEnum.end();
        bool bLastLoop = (it == itEnd);
        while (!bLastLoop)
        {
            sal_Int32 nPageIndex = *it;
            ++it;
            bLastLoop = (it == itEnd);

            if (GetFilteredPage(nPageIndex, PK_STANDARD))
                aPageIndices.push_back(nPageIndex);
            else if (!bLastLoop)
                continue;

            // Create a printer page when we have found one page for each
            // placeholder or when this is the last (and special) loop.
            if (aPageIndices.size() == nShapeCount
                || bLastLoop)
            {
                maPrinterPages.push_back(
                    ::boost::shared_ptr<PrinterPage>(
                        new HandoutPrinterPage(
                            nPrinterPageIndex++,
                            aPageIndices,
                            aMap,
                            rInfo.msTimeDate,
                            aPageOfs,
                            rInfo.mnDrawMode,
                            rInfo.meOrientation,
                            nPaperBin)));
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
        OSL_ASSERT(rInfo.mpPrinter != NULL);

        // Fill in page kind specific data.
        SdDrawDocument* pDocument = mrBase.GetMainViewShell()->GetDoc();
        if (pDocument->GetSdPageCount(ePageKind) == 0)
            return;
        SdPage* pRefPage = pDocument->GetSdPage(0, ePageKind);
        rInfo.maPageSize = pRefPage->GetSize();

        if ( ! SetupPaperOrientation(ePageKind, rInfo))
            return;

        MapMode aMap (rInfo.maMap);
        rInfo.maMap = aMap;

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
        ::boost::shared_ptr<ViewShell> pViewShell (mrBase.GetMainViewShell());
        pViewShell->WriteFrameViewData();
        Point aPtZero;

        sal_Int32 nPageCount = mrBase.GetDocument()->GetSdPageCount(PK_STANDARD);
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
            if (pPage == NULL)
                continue;

            MapMode aMap (rInfo.maMap);
            // Kann sich die Seitengroesse geaendert haben?
            const Size aPageSize = pPage->GetSize();

            if (mpOptions->IsPageSize())
            {
                const double fHorz ((double) rInfo.maPrintSize.Width()  / aPageSize.Width());
                const double fVert ((double) rInfo.maPrintSize.Height() / aPageSize.Height());

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
                rInfo.msPageString = pPage->GetName();
                rInfo.msPageString += rtl::OUString(sal_Unicode(' '));
            }
            else
                rInfo.msPageString = "";
            rInfo.msPageString += rInfo.msTimeDate;

            long aPageWidth   = aPageSize.Width() - pPage->GetLftBorder() - pPage->GetRgtBorder();
            long aPageHeight  = aPageSize.Height() - pPage->GetUppBorder() - pPage->GetLwrBorder();
            // Bugfix zu 44530:
            // Falls implizit umgestellt wurde (Landscape/Portrait)
            // wird dies beim Kacheln, bzw. aufteilen (Poster) beruecksichtigt
            if( ( rInfo.maPrintSize.Width() > rInfo.maPrintSize.Height()
                    && aPageWidth < aPageHeight )
                || ( rInfo.maPrintSize.Width() < rInfo.maPrintSize.Height()
                    && aPageWidth > aPageHeight ) )
            {
                const sal_Int32 nTmp (rInfo.maPrintSize.Width());
                rInfo.maPrintSize.Width() = rInfo.maPrintSize.Height();
                rInfo.maPrintSize.Height() = nTmp;
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

        if (rInfo.meOrientation == ORIENTATION_LANDSCAPE)
            aPrintSize_2.Width() >>= 1;
        else
            aPrintSize_2.Height() >>= 1;

        const double fPageWH = (double) aPageSize_2.Width() / aPageSize_2.Height();
        const double fPrintWH = (double) aPrintSize_2.Width() / aPrintSize_2.Height();

        if( fPageWH < fPrintWH )
        {
            aPageSize_2.Width() = (long) ( aPrintSize_2.Height() * fPageWH );
            aPageSize_2.Height()= aPrintSize_2.Height();
        }
        else
        {
            aPageSize_2.Width() = aPrintSize_2.Width();
            aPageSize_2.Height() = (long) ( aPrintSize_2.Width() / fPageWH );
        }

        MapMode aMap (rInfo.maMap);
        aMap.SetScaleX( Fraction( aPageSize_2.Width(), rInfo.maPageSize.Width() ) );
        aMap.SetScaleY( Fraction( aPageSize_2.Height(), rInfo.maPageSize.Height() ) );

        // calculate adjusted print size
        const Size aAdjustedPrintSize (OutputDevice::LogicToLogic(
            rInfo.maPrintSize,
            aStdMap,
            aMap));

        if (rInfo.meOrientation == ORIENTATION_LANDSCAPE)
        {
            aOffset.X() = ( ( aAdjustedPrintSize.Width() >> 1 ) - rInfo.maPageSize.Width() ) >> 1;
            aOffset.Y() = ( aAdjustedPrintSize.Height() - rInfo.maPageSize.Height() ) >> 1;
        }
        else
        {
            aOffset.X() = ( aAdjustedPrintSize.Width() - rInfo.maPageSize.Width() ) >> 1;
            aOffset.Y() = ( ( aAdjustedPrintSize.Height() >> 1 ) - rInfo.maPageSize.Height() ) >> 1;
        }

        // create vector of pages to print
        sal_Int32 nPageCount = mrBase.GetDocument()->GetSdPageCount(ePageKind);
        StringRangeEnumerator aRangeEnum(
            mpOptions->GetPrinterSelection(nPageCount, GetCurrentPageIndex()),
            0, nPageCount-1);
        ::std::vector< sal_uInt16 > aPageVector;
        for (StringRangeEnumerator::Iterator
                 it = aRangeEnum.begin(),
                 itEnd = aRangeEnum.end();
             it != itEnd;
             ++it)
        {
            SdPage* pPage = GetFilteredPage(*it, ePageKind);
            if (pPage != NULL)
                aPageVector.push_back(*it);
        }

        // create pairs of pages to print on each page
        typedef ::std::vector< ::std::pair< sal_uInt16, sal_uInt16 > > PairVector;
        PairVector aPairVector;
        if ( ! aPageVector.empty())
        {
            sal_uInt32 nFirstIndex = 0, nLastIndex = aPageVector.size() - 1;

            if( aPageVector.size() & 1 )
                aPairVector.push_back( ::std::make_pair( (sal_uInt16) 65535, aPageVector[ nFirstIndex++ ] ) );
            else
                aPairVector.push_back( ::std::make_pair( aPageVector[ nLastIndex-- ], aPageVector[ nFirstIndex++ ] ) );

            while( nFirstIndex < nLastIndex )
            {
                if( nFirstIndex & 1 )
                    aPairVector.push_back( ::std::make_pair( aPageVector[ nFirstIndex++ ], aPageVector[ nLastIndex-- ] ) );
                else
                    aPairVector.push_back( ::std::make_pair( aPageVector[ nLastIndex-- ], aPageVector[ nFirstIndex++ ] ) );
            }
        }

        for (sal_uInt32
                 nIndex=0,
                 nCount=aPairVector.size();
             nIndex < nCount;
             ++nIndex)
        {
            const bool bIsIndexOdd (nIndex & 1);
            if ((!bIsIndexOdd && mpOptions->IsPrintFrontPage())
                || (bIsIndexOdd && mpOptions->IsPrintBackPage()))
            {
                const ::std::pair<sal_uInt16, sal_uInt16> aPair (aPairVector[nIndex]);
                Point aSecondOffset (aOffset);
                if (rInfo.meOrientation == ORIENTATION_LANDSCAPE)
                    aSecondOffset.X() += aAdjustedPrintSize.Width() / 2;
                else
                    aSecondOffset.Y() += aAdjustedPrintSize.Height() / 2;
                maPrinterPages.push_back(
                    ::boost::shared_ptr<PrinterPage>(
                        new BookletPrinterPage(
                            aPair.first,
                            aPair.second,
                            aOffset,
                            aSecondOffset,
                            ePageKind,
                            aMap,
                            rInfo.mbPrintMarkedOnly,
                            rInfo.mnDrawMode,
                            rInfo.meOrientation,
                            rInfo.mpPrinter->GetPaperBin())));

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

        maPrinterPages.push_back(
            ::boost::shared_ptr<PrinterPage>(
                new TiledPrinterPage(
                    sal::static_int_cast<sal_uInt16>(nPageIndex),
                    ePageKind,
                    500,
                    rInfo.mbPrintMarkedOnly,
                    rInfo.msPageString,
                    rInfo.mpPrinter->GetPageOffset(),
                    rInfo.mnDrawMode,
                    rInfo.meOrientation,
                    nPaperBin)));
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

        // For pages larger then the printable area there
        // are three options:
        // 1. Scale down to the page to the printable area.
        // 2. Print only the upper left part of the page
        //    (without the unprintable borders).
        // 3. Split the page into parts of the size of the
        // printable area.
        const bool bScalePage (mpOptions->IsPageSize());
        const bool bCutPage (mpOptions->IsCutPage());
        MapMode aMap (rInfo.maMap);
        if (bScalePage || bCutPage)
        {
            // Handle 1 and 2.

            // if CutPage is set then do not move it, otherwise move the
            // scaled page to printable area
            maPrinterPages.push_back(
                ::boost::shared_ptr<PrinterPage>(
                    new RegularPrinterPage(
                        sal::static_int_cast<sal_uInt16>(nPageIndex),
                        ePageKind,
                        aMap,
                        rInfo.mbPrintMarkedOnly,
                        rInfo.msPageString,
                        aPageOffset,
                        rInfo.mnDrawMode,
                        rInfo.meOrientation,
                        nPaperBin)));
        }
        else
        {
            // Handle 3.  Print parts of the page in the size of the
            // printable area until the whole page is covered.

            // keep the page content at its position if it fits, otherwise
            // move it to the printable area
            const long nPageWidth (
                rInfo.maPageSize.Width() - rPage.GetLftBorder() - rPage.GetRgtBorder());
            const long nPageHeight (
                rInfo.maPageSize.Height() - rPage.GetUppBorder() - rPage.GetLwrBorder());

            Point aOrigin ( 0, 0 );

            for (Point aPageOrigin = aOrigin;
                 -aPageOrigin.Y()<nPageHeight;
                 aPageOrigin.Y() -= rInfo.maPrintSize.Height())
            {
                for (aPageOrigin.X()=aOrigin.X();
                     -aPageOrigin.X()<nPageWidth;
                     aPageOrigin.X() -= rInfo.maPrintSize.Width())
                {
                    aMap.SetOrigin(aPageOrigin);
                    maPrinterPages.push_back(
                        ::boost::shared_ptr<PrinterPage>(
                            new RegularPrinterPage(
                                sal::static_int_cast<sal_uInt16>(nPageIndex),
                                ePageKind,
                                aMap,
                                rInfo.mbPrintMarkedOnly,
                                rInfo.msPageString,
                                aPageOffset,
                                rInfo.mnDrawMode,
                                rInfo.meOrientation,
                                nPaperBin)));
                }
            }
        }
    }
};




//===== DocumentRenderer ======================================================

DocumentRenderer::DocumentRenderer (ViewShellBase& rBase)
    : DocumentRendererInterfaceBase(m_aMutex),
      mpImpl(new Implementation(rBase))
{
}




DocumentRenderer::~DocumentRenderer()
{
}




//----- XRenderable -----------------------------------------------------------

sal_Int32 SAL_CALL DocumentRenderer::getRendererCount (
    const css::uno::Any& aSelection,
    const css::uno::Sequence<css::beans::PropertyValue >& rOptions)
    throw (css::lang::IllegalArgumentException, css::uno::RuntimeException)
{
    (void)aSelection;
    mpImpl->ProcessProperties(rOptions);
    return mpImpl->GetPrintPageCount();
}




Sequence<beans::PropertyValue> SAL_CALL DocumentRenderer::getRenderer (
    sal_Int32 nRenderer,
    const css::uno::Any& rSelection,
    const css::uno::Sequence<css::beans::PropertyValue>& rOptions)
    throw (css::lang::IllegalArgumentException, css::uno::RuntimeException)
{
    (void)nRenderer;
    (void)rSelection;
    mpImpl->ProcessProperties(rOptions);
    return mpImpl->GetProperties(rOptions);
}




void SAL_CALL DocumentRenderer::render (
    sal_Int32 nRenderer,
    const css::uno::Any& rSelection,
    const css::uno::Sequence<css::beans::PropertyValue>& rOptions)
    throw (css::lang::IllegalArgumentException, css::uno::RuntimeException)
{
    (void)rSelection;
    mpImpl->ProcessProperties(rOptions);
    mpImpl->PrintPage(nRenderer);
}



} // end of namespace sd

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
