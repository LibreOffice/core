/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/listview.hxx>

#include <sfx2/sfxresid.hxx>
#include <tools/urlobj.hxx>
#include <tools/datetime.hxx>
#include <sfx2/strings.hrc>
#include <osl/file.hxx>
#include <osl/time.h>
#include <comphelper/fileurl.hxx>

#include <svtools/svtresid.hxx>
#include <svtools/strings.hrc>
#include <unotools/localedatawrapper.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/intlwrapper.hxx>
#include <vcl/wintypes.hxx>

#include <bitmaps.hlst>
#include <rtl/math.hxx>

#include <sfx2/templatelocalview.hxx>

#define COLUMN_IMG_ISDEFAULT 0
#define COLUMN_NAME 1
#define COLUMN_CATEGORY 2
#define COLUMN_APPLICATION 3
#define COLUMN_MODIFIED 4
#define COLUMN_SIZE 5
#define NUMBER_OF_COLUMNS 6

static sal_uInt64 getFileSize(const OUString& rURL);
static sal_uInt32 getFileModifyTime(const OUString& rURL);
static OUString getDisplayFileSize(const OUString& rURL);
static OUString getDisplayFileModifyTime(const OUString& rURL);
static OUString getApplication(std::u16string_view rURL);

ListView::ListView(std::unique_ptr<weld::TreeView> xTreeView)
    : mxTreeView(std::move(xTreeView))
    , mnSortColumn(-2)
{
    auto nDigitWidth = mxTreeView->get_approximate_digit_width();
    std::vector<int> aWidths{
        static_cast<int>(nDigitWidth * 5), /* Icon Column */
        static_cast<int>(nDigitWidth * 24), /* Name Column */
        static_cast<int>(nDigitWidth * 22), /* Category Column */
        static_cast<int>(nDigitWidth * 15), /* Application Column */
        static_cast<int>(nDigitWidth * 18) /* Modify Column */
    };

    // tdf#151143 Make the size of ListView and ThumbnailView the same
    mxTreeView->set_size_request(TEMPLATE_ITEM_MAX_WIDTH * 5, TEMPLATE_ITEM_MAX_HEIGHT_SUB * 3);

    mxTreeView->set_column_fixed_widths(aWidths);
    mxTreeView->set_selection_mode(SelectionMode::Multiple);
    mxTreeView->connect_query_tooltip(LINK(this, ListView, QueryTooltipHdl));
}
ListView::~ListView() {}

void ListView::AppendItem(const OUString& rId, const OUString& rTitle, const OUString& rSubtitle,
                          const OUString& rPath, bool bDefault)
{
    INetURLObject aUrl(rPath, INetProtocol::File);

    std::unique_ptr<ListViewItem> pItem(new ListViewItem);
    pItem->maId = rId;
    pItem->maTitle = rTitle;
    pItem->maSubtitle = rSubtitle;
    pItem->maApplication = getApplication(rPath);
    pItem->maPath = rPath;
    pItem->mbDefault = bDefault;
    pItem->mnModify = getFileModifyTime(rPath);
    pItem->mnSize = getFileSize(rPath);
    pItem->maDisplayModify = getDisplayFileModifyTime(rPath);
    pItem->maDisplaySize = getDisplayFileSize(rPath);
    pItem->maDisplayPath = aUrl.getFSysPath(FSysStyle::Detect);

    OUString sImage(u""_ustr);
    if (pItem->mbDefault)
        sImage = BMP_DEFAULT;

    AppendRow(sImage, pItem->maTitle, pItem->maSubtitle, pItem->maApplication,
              pItem->maDisplayModify, pItem->maDisplaySize, pItem->maId);

    mListViewItems.push_back(std::move(pItem));
}

void ListView::AppendRow(const OUString& rImage, const OUString& rTitle, const OUString& rSubtitle,
                         const OUString& rApplication, const OUString& rModify,
                         const OUString& rSize, const OUString& rId)
{
    std::unique_ptr<weld::TreeIter> xIter(mxTreeView->make_iterator());
    mxTreeView->append(xIter.get());
    mxTreeView->set_image(*xIter, rImage, COLUMN_IMG_ISDEFAULT);
    mxTreeView->set_text(*xIter, rTitle, COLUMN_NAME);
    mxTreeView->set_text(*xIter, rSubtitle, COLUMN_CATEGORY);
    mxTreeView->set_text(*xIter, rApplication, COLUMN_APPLICATION);
    mxTreeView->set_text(*xIter, rModify, COLUMN_MODIFIED);
    mxTreeView->set_text(*xIter, rSize, COLUMN_SIZE);
    mxTreeView->set_id(*xIter, rId);
}

void ListView::UpdateRow(int nIndex, const OUString& rImage, const OUString& rTitle,
                         const OUString& rSubtitle, const OUString& rApplication,
                         const OUString& rModify, const OUString& rSize, const OUString& rId)
{
    mxTreeView->set_image(nIndex, rImage, COLUMN_IMG_ISDEFAULT);
    mxTreeView->set_text(nIndex, rTitle, COLUMN_NAME);
    mxTreeView->set_text(nIndex, rSubtitle, COLUMN_CATEGORY);
    mxTreeView->set_text(nIndex, rApplication, COLUMN_APPLICATION);
    mxTreeView->set_text(nIndex, rModify, COLUMN_MODIFIED);
    mxTreeView->set_text(nIndex, rSize, COLUMN_SIZE);
    mxTreeView->set_id(nIndex, rId);
}

void ListView::ReloadRows()
{
    OUString sCursorId = get_id(get_cursor_index());
    mxTreeView->clear();
    for (const auto& pItem : mListViewItems)
    {
        OUString sImage(u""_ustr);
        if (pItem->mbDefault)
            sImage = BMP_DEFAULT;
        AppendRow(sImage, pItem->maTitle, pItem->maSubtitle, pItem->maApplication,
                  pItem->maDisplayModify, pItem->maDisplaySize, pItem->maId);
    }
    unselect_all();
    if (!sCursorId.isEmpty())
    {
        select_id(sCursorId);
        set_cursor(get_selected_index());
    }
}

bool ListView::UpdateRows()
{
    if (static_cast<int>(mListViewItems.size()) != mxTreeView->n_children())
        return false;
    OUString sCursorId = get_id(get_cursor_index());
    int nIndex = 0;
    for (const auto& pItem : mListViewItems)
    {
        OUString sImage(u""_ustr);
        if (pItem->mbDefault)
            sImage = BMP_DEFAULT;
        UpdateRow(nIndex, sImage, pItem->maTitle, pItem->maSubtitle, pItem->maApplication,
                  pItem->maDisplayModify, pItem->maDisplaySize, pItem->maId);
        ++nIndex;
    }
    unselect_all();
    if (!sCursorId.isEmpty())
    {
        select_id(sCursorId);
        set_cursor(get_selected_index());
    }
    return true;
}

IMPL_LINK(ListView, ColumnClickedHdl, const int, col, void)
{
    if (col <= 0 || col > NUMBER_OF_COLUMNS)
        return;

    if (mnSortColumn >= 0 && mnSortColumn != col)
        mxTreeView->set_sort_indicator(TriState::TRISTATE_INDET, mnSortColumn);

    mxTreeView->set_sort_indicator((mxTreeView->get_sort_indicator(col) == TriState::TRISTATE_TRUE
                                        ? TriState::TRISTATE_FALSE
                                        : TriState::TRISTATE_TRUE),
                                   col);
    sortColumn(col);
}

void ListView::sortColumn(const int col)
{
    if (col <= 0 || col > NUMBER_OF_COLUMNS)
        return;

    bool isAscending = mxTreeView->get_sort_indicator(col) != TriState::TRISTATE_FALSE;

    auto comp = [&](std::unique_ptr<ListViewItem> const& pItemA,
                    std::unique_ptr<ListViewItem> const& pItemB) {
        sal_Int32 res = 0;
        IntlWrapper aIntlWrapper(SvtSysLocale().GetUILanguageTag());
        const CollatorWrapper* pCollatorWrapper = aIntlWrapper.getCollator();
        switch (col)
        {
            case COLUMN_NAME:
            {
                OUString sNameA = pItemA->maTitle;
                OUString sNameB = pItemB->maTitle;
                res = pCollatorWrapper->compareString(sNameA, sNameB);
            }
            break;
            case COLUMN_CATEGORY:
            {
                OUString sCategoryA = pItemA->maSubtitle;
                OUString sCategoryB = pItemB->maSubtitle;
                res = pCollatorWrapper->compareString(sCategoryA, sCategoryB);
            }
            break;
            case COLUMN_MODIFIED:
            {
                sal_uInt32 nModA, nModB;
                nModA = pItemA->mnModify;
                nModB = pItemB->mnModify;

                if (nModA < nModB)
                    res = -1;
                else if (nModA > nModB)
                    res = 1;
            }
            break;
            case COLUMN_SIZE:
            {
                sal_uInt64 nSizeA, nSizeB;
                nSizeA = pItemA->mnSize;
                nSizeB = pItemB->mnSize;

                if (nSizeA < nSizeB)
                    res = -1;
                else if (nSizeA > nSizeB)
                    res = 1;
            }
            break;
            case COLUMN_APPLICATION:
            {
                OUString sPathA = pItemA->maApplication;
                OUString sPathB = pItemB->maApplication;
                res = pCollatorWrapper->compareString(sPathA, sPathB);
            }
            break;
        }
        return isAscending ? (res > 0) : (res < 0);
    };
    std::stable_sort(mListViewItems.begin(), mListViewItems.end(), comp);

    if (!UpdateRows())
        ReloadRows();
    mnSortColumn = col;
}

void ListView::sort() { sortColumn(mnSortColumn); }

void ListView::refreshDefaultColumn()
{
    for (const auto& pItem : mListViewItems)
    {
        bool bDefault = TemplateLocalView::IsDefaultTemplate(pItem->maPath);
        if (pItem->mbDefault != bDefault)
        {
            pItem->mbDefault = bDefault;
            OUString sImage(u""_ustr);
            if (bDefault)
                sImage = BMP_DEFAULT;
            mxTreeView->set_image(mxTreeView->find_id(pItem->maId), sImage, COLUMN_IMG_ISDEFAULT);
        }
    }
}

void ListView::rename(const OUString& rId, const OUString& rTitle)
{
    mxTreeView->set_text(mxTreeView->find_id(rId), rTitle, COLUMN_NAME);
    for (const auto& pItem : mListViewItems)
        if (pItem->maId == rId)
        {
            pItem->maTitle = rTitle;
            break;
        }
}

void ListView::clearListView()
{
    mxTreeView->clear();
    mListViewItems.clear();
}

IMPL_LINK(ListView, QueryTooltipHdl, const weld::TreeIter&, rIter, OUString)
{
    OUString sId = mxTreeView->get_id(rIter);
    for (const auto& pItem : mListViewItems)
    {
        if (pItem->maId == sId)
            return pItem->maDisplayPath;
    }
    return OUString();
}

sal_uInt16 ListView::get_nId(int pos) const
{
    return static_cast<sal_uInt16>(mxTreeView->get_id(pos).toInt32());
}

static sal_uInt32 getFileModifyTime(const OUString& rURL)
{
    sal_uInt32 nModify = 0;
    if (!comphelper::isFileUrl(rURL))
        return nModify;

    osl::DirectoryItem aItem;
    if (osl::DirectoryItem::get(rURL, aItem) != osl::DirectoryItem::E_None)
        return nModify;

    osl::FileStatus aStatus(osl_FileStatus_Mask_ModifyTime);
    if (aItem.getFileStatus(aStatus) != osl::DirectoryItem::E_None)
        return nModify;

    TimeValue systemTimeValue = aStatus.getModifyTime();

    nModify = systemTimeValue.Seconds;
    return nModify;
}
static OUString getDisplayFileModifyTime(const OUString& rURL)
{
    if (!comphelper::isFileUrl(rURL))
        return OUString();

    osl::DirectoryItem aItem;
    if (osl::DirectoryItem::get(rURL, aItem) != osl::DirectoryItem::E_None)
        return OUString();

    osl::FileStatus aStatus(osl_FileStatus_Mask_ModifyTime);
    if (aItem.getFileStatus(aStatus) != osl::DirectoryItem::E_None)
        return OUString();

    TimeValue systemTimeValue = aStatus.getModifyTime();
    if (systemTimeValue.Seconds == 0)
        return OUString();
    TimeValue localTimeValue;
    osl_getLocalTimeFromSystemTime(&systemTimeValue, &localTimeValue);
    const SvtSysLocale aSysLocale;
    const LocaleDataWrapper& rLocaleWrapper = aSysLocale.GetLocaleData();
    DateTime aDateTime = DateTime::CreateFromUnixTime(localTimeValue.Seconds);
    OUString aDisplayDateTime
        = rLocaleWrapper.getDate(aDateTime) + ", " + rLocaleWrapper.getTime(aDateTime, false);
    return aDisplayDateTime;
}

static OUString getDisplayFileSize(const OUString& rURL)
{
    if (!comphelper::isFileUrl(rURL))
        return OUString();

    osl::DirectoryItem aItem;
    if (osl::DirectoryItem::get(rURL, aItem) != osl::DirectoryItem::E_None)
        return OUString();

    osl::FileStatus aStatus(osl_FileStatus_Mask_FileSize);
    if (aItem.getFileStatus(aStatus) != osl::DirectoryItem::E_None)
        return OUString();

    sal_uInt64 nSize = aStatus.getFileSize();
    double fSize(static_cast<double>(nSize));
    sal_uInt32 nDec;

    sal_uInt64 nMega = 1024 * 1024;
    sal_uInt64 nGiga = nMega * 1024;

    OUString aUnitStr(' ');

    if (nSize < 10000)
    {
        aUnitStr += SvtResId(STR_SVT_BYTES);
        nDec = 0;
    }
    else if (nSize < nMega)
    {
        fSize /= 1024;
        aUnitStr += SvtResId(STR_SVT_KB);
        nDec = 1;
    }
    else if (nSize < nGiga)
    {
        fSize /= nMega;
        aUnitStr += SvtResId(STR_SVT_MB);
        nDec = 2;
    }
    else
    {
        fSize /= nGiga;
        aUnitStr += SvtResId(STR_SVT_GB);
        nDec = 3;
    }

    OUString aSizeStr
        = ::rtl::math::doubleToUString(fSize, rtl_math_StringFormat_F, nDec,
                                       SvtSysLocale().GetLocaleData().getNumDecimalSep()[0])
          + aUnitStr;

    return aSizeStr;
}

static sal_uInt64 getFileSize(const OUString& rURL)
{
    sal_uInt64 nSize = 0;
    if (!comphelper::isFileUrl(rURL))
        return nSize;

    osl::DirectoryItem aItem;
    if (osl::DirectoryItem::get(rURL, aItem) != osl::DirectoryItem::E_None)
        return nSize;

    osl::FileStatus aStatus(osl_FileStatus_Mask_FileSize);
    if (aItem.getFileStatus(aStatus) != osl::DirectoryItem::E_None)
        return nSize;

    nSize = aStatus.getFileSize();
    return nSize;
}

static OUString getApplication(std::u16string_view rURL)
{
    INetURLObject aUrl(rURL);
    OUString aExt = aUrl.getExtension();

    if (aExt == "ott" || aExt == "stw" || aExt == "oth" || aExt == "dot" || aExt == "dotx")
    {
        return SfxResId(STR_DOCUMENT);
    }
    else if (aExt == "ots" || aExt == "stc" || aExt == "xlt" || aExt == "xltm" || aExt == "xltx")
    {
        return SfxResId(STR_SPREADSHEET);
    }
    else if (aExt == "otp" || aExt == "sti" || aExt == "pot" || aExt == "potm" || aExt == "potx")
    {
        return SfxResId(STR_PRESENTATION);
    }
    else if (aExt == "otg" || aExt == "std")
    {
        return SfxResId(STR_DRAWING);
    }
    return OUString();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
