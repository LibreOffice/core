/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_CUI_SOURCE_INC_SFXREDACTIONHELPER_HXX
#define INCLUDED_CUI_SOURCE_INC_SFXREDACTIONHELPER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/drawing/XDrawPage.hpp>

#include <sal/types.h>
#include <rtl/ustring.hxx>
#include <tools/gen.hxx>

#include <string_view>
#include <vector>

using namespace ::com::sun::star;
using namespace ::com::sun::star::lang;
using namespace ::com::sun::star::uno;

class SfxRequest;
class SfxStringItem;
class GDIMetaFile;
class DocumentToGraphicRenderer;
class SfxViewFrame;
struct RedactionTarget;

namespace i18nutil
{
struct SearchOptions2;
}

struct PageMargins
{
    // Page margins in mm100th
    sal_Int32 nTop;
    sal_Int32 nBottom;
    sal_Int32 nLeft;
    sal_Int32 nRight;
};

/*
 * Mostly a bunch of static methods to handle the redaction functionality at
 * different points of the process.
 **/
class SfxRedactionHelper
{
public:
    /// Checks to see if the request has a parameter of IsRedactMode:bool=true
    static bool isRedactMode(const SfxRequest& rReq);
    /*
     * Returns the value of the given string param as an OUString
     * Returns empty OUString if no param
     * */
    static OUString getStringParam(const SfxRequest& rReq, sal_uInt16 nParamId);
    /*
     * Creates metafiles from the pages of the given document,
     * and pushes into the given vector.
     * */
    static void getPageMetaFilesFromDoc(std::vector<GDIMetaFile>& aMetaFiles,
                                        std::vector<::Size>& aPageSizes, sal_Int32 nPages,
                                        DocumentToGraphicRenderer& aRenderer);
    /*
     * Creates one shape and one draw page for each gdimetafile,
     * and inserts the shapes into the newly created draw pages.
     * */
    static void
    addPagesToDraw(const uno::Reference<XComponent>& xComponent, sal_Int32 nPages,
                   const std::vector<GDIMetaFile>& aMetaFiles,
                   const std::vector<::Size>& aPageSizes, const PageMargins& aPageMargins,
                   const std::vector<std::pair<RedactionTarget*, OUString>>& r_aTableTargets,
                   bool bIsAutoRedact);
    /*
     * Makes the Redaction toolbar visible to the user.
     * Meant to be called after converting a document to a Draw doc
     * for redaction purposes.
     * */
    static void showRedactionToolbar(const SfxViewFrame* pViewFrame);

    /*
     * Used to get the page margins from the original/source Writer document. Then we apply these values to the
     * pages inserted into Draw for redaction.
     * */
    static PageMargins
    getPageMarginsForWriter(const css::uno::Reference<css::frame::XModel>& xModel);

    /*
     * Used to get the page margins from the original/source Calc document. Then we apply these values to the
     * pages inserted into Draw for redaction.
     * */
    static PageMargins getPageMarginsForCalc(const css::uno::Reference<css::frame::XModel>& xModel);

    /*
     * Used to find the text portions to be redacted. Returns a list of rectangles to cover those
     * areas to be redacted. Probably the most crucial part of the auto-redaction process.
     * */
    static void searchInMetaFile(const RedactionTarget* pRedactionTarget, const GDIMetaFile& rMtf,
                                 std::vector<tools::Rectangle>& aRedactionRectangles,
                                 const uno::Reference<XComponent>& xComponent);

    /*
     * Draws a redaction rectangle on the draw page referenced with its page number (0-based)
     * */
    static void addRedactionRectToPage(const uno::Reference<XComponent>& xComponent,
                                       const uno::Reference<drawing::XDrawPage>& xPage,
                                       const std::vector<tools::Rectangle>& aNewRectangles);

    /*
     * Search for the given term through the gdimetafile, which has the whole content of a draw page,
     * and draw redaction rectangles to the appropriate positions with suitable sizes.
     * */
    static void autoRedactPage(const RedactionTarget* pRedactionTarget,
                               const GDIMetaFile& rGDIMetaFile,
                               const uno::Reference<drawing::XDrawPage>& xPage,
                               const uno::Reference<XComponent>& xComponent);

    /// Fill the search options based on the given redaction target
    static void fillSearchOptions(i18nutil::SearchOptions2& rSearchOpt,
                                  const RedactionTarget* pTarget);

private:
    static constexpr std::u16string_view m_aPredefinedTargets[6] = {
        u"\\b(?:\\d[ -]*?){13,16}\\b", //Credit card numbers
        u"\\b[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\\.[a-zA-Z]{2,}\\b", //Email addresses
        u"\\b(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"
        "\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)\\.(25[0-5]|2[0-4][0-9]|[01]?[0-9][0-9]?)"
        "\\b", //IP addresses
        u"([12]\\d{3}[./-](0[1-9]|1[0-2])[./"
        "-](0[1-9]|[12]\\d|3[01]))|((0[1-9]|[12]\\d|3[01])[./-](0[1-9]|1[0-2])[./"
        "-][12]\\d{3})", //Dates (numerical)
        u"\\s*[a-zA-Z]{2}(?:\\s*\\d\\s*){6}[a-zA-Z]?\\s*", //National Insurance Number (UK)
        u"([1-9])(?!\\1{2}-\\1{2}-\\1{4})[1-9]{2}-[1-9]{2}-[1-9]{4}" //Social Security Number (US)
    };
};

#endif // INCLUDED_CUI_SOURCE_INC_SFXREDACTIONHELPER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
