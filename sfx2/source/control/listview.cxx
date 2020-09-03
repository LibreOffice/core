/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <vcl/headbar.hxx>
#include <sfx2/listview.hxx>

#include <sfx2/doctempl.hxx>
#include <sfx2/sfxresid.hxx>
#include <tools/urlobj.hxx>
#include <vcl/svapp.hxx>

#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>

#include <sfx2/strings.hrc>
#include <osl/file.hxx>
#include <osl/time.h>
#include <comphelper/fileurl.hxx>
#include <string>

#include <svtools/svtresid.hxx>
#include <svtools/strings.hrc>
#include <unotools/localedatawrapper.hxx>
#include <unotools/collatorwrapper.hxx>
#include <unotools/syslocale.hxx>
#include <unotools/intlwrapper.hxx>

#include <sfx2/strings.hrc>
#include <bitmaps.hlst>
#include <rtl/math.hxx>

#include <sfx2/templatelocalview.hxx>

#define COLUMN_IMG_ISDEFAULT -1
#define COLUMN_NAME 0
#define COLUMN_CATEGORY 1
#define COLUMN_APPLICATION 2
#define COLUMN_MODIFIED 3
#define COLUMN_SIZE 4
#define COLUMN_DISPLAY_PATH 5
#define NUMBER_OF_VISIBLE_COLUMNS 6
#define COLUMN_SORT_ISDEFAULT 6
#define COLUMN_SORT_MODIFIED 7
#define COLUMN_SORT_SIZE 8
#define COLUMN_PATH 9

using namespace ::com::sun::star;

using namespace ::com::sun::star::uno;

OUString getFileAccessTime(const OUString& rURL);
OUString getFileCreationTime(const OUString& rURL);
OUString getFileSize(const OUString& rURL);
OUString getFileModifyTime(const OUString& rURL);
OUString getDisplayFileSize(const OUString& rURL);
OUString getDisplayFileModifyTime(const OUString& rURL);
OUString getApplication(const OUString& rURL);

ListView::ListView (std::unique_ptr<weld::TreeView> xTreeView,
                                     std::unique_ptr<weld::ScrolledWindow> xWindow)
    : mxTreeView(std::move(xTreeView)),
    mxScrolledWindowListView(std::move(xWindow))

{

}
ListView::~ListView ()
{
}

void ListView::AppendItem(const OUString &rId, const OUString &rTitle, const OUString &rSubtitle, const OUString &rPath, const TriState isDefault)
{
    INetURLObject aUrl(rPath);
    OUString sPath = aUrl.GetURLPath();
    OUString sImgDefault(BMP_DEFAULT);

    std::unique_ptr<weld::TreeIter> xIter(mxTreeView->make_iterator());
    mxTreeView->append(xIter.get());
    mxTreeView->set_image(*xIter, sImgDefault, COLUMN_IMG_ISDEFAULT);
    mxTreeView->set_text(*xIter, rTitle, COLUMN_NAME);
    mxTreeView->set_text(*xIter, rSubtitle, COLUMN_CATEGORY);
    mxTreeView->set_text(*xIter, getApplication(rPath), COLUMN_APPLICATION);
    mxTreeView->set_text(*xIter, getDisplayFileModifyTime(rPath), COLUMN_MODIFIED);
    mxTreeView->set_text(*xIter, getDisplayFileSize(rPath), COLUMN_SIZE);
    mxTreeView->set_text(*xIter, sPath, COLUMN_DISPLAY_PATH);
    mxTreeView->set_text(*xIter, getFileModifyTime(rPath), COLUMN_SORT_MODIFIED);
    mxTreeView->set_text(*xIter, getFileSize(rPath), COLUMN_SORT_SIZE);
    mxTreeView->set_toggle(*xIter, isDefault, COLUMN_SORT_ISDEFAULT);
    mxTreeView->set_text(*xIter, rPath, COLUMN_PATH);
    mxTreeView->set_id(*xIter, rId);
}

void ListView::sortColumn(const int col)
{
    if(mxTreeView->n_children() < 2)
        return;

    if( col >= 0 && col < NUMBER_OF_VISIBLE_COLUMNS )
    {
        bool isAscending = mxTreeView->get_sort_indicator(col) == TriState::TRISTATE_FALSE ?
                                                         false : true;
        sal_Int32 res = -2;
        mxTreeView->set_sort_func([&](const weld::TreeIter& itA, const weld::TreeIter& itB){
        IntlWrapper aIntlWrapper(SvtSysLocale().GetUILanguageTag());
        const CollatorWrapper* pCollatorWrapper = aIntlWrapper.getCollator();
        switch ( col )
        {
            case COLUMN_NAME:
            {
                OUString sNameA = mxTreeView->get_text(itA, COLUMN_NAME);
                OUString sNameB = mxTreeView->get_text(itB, COLUMN_NAME);
                res = pCollatorWrapper->compareString(sNameA,sNameB);
            }
            break;
            case COLUMN_CATEGORY:
            {
                OUString sCategoryA = mxTreeView->get_text(itA, COLUMN_CATEGORY);
                OUString sCategoryB = mxTreeView->get_text(itB, COLUMN_CATEGORY);
                res = pCollatorWrapper->compareString(sCategoryA,sCategoryB);
            }
            break;
            case COLUMN_IMG_ISDEFAULT:
            {
                if(mxTreeView->get_toggle(itB, COLUMN_SORT_ISDEFAULT) == TriState::TRISTATE_TRUE &&
                    mxTreeView->get_toggle(itA, COLUMN_SORT_ISDEFAULT) == TriState::TRISTATE_FALSE)
                {
                    res = -1;
                }
                else if(mxTreeView->get_toggle(itB, COLUMN_SORT_ISDEFAULT) == TriState::TRISTATE_FALSE &&
                    mxTreeView->get_toggle(itA, COLUMN_SORT_ISDEFAULT) == TriState::TRISTATE_TRUE)
                {
                    res = 1;
                }
            }
            break;
            case COLUMN_MODIFIED:
            {
                OUString sModA = mxTreeView->get_text(itA, COLUMN_SORT_MODIFIED);
                OUString sModB = mxTreeView->get_text(itB, COLUMN_SORT_MODIFIED);
                res = pCollatorWrapper->compareString(sModA,sModB);
            }
            break;
            case COLUMN_SIZE:
            {
                sal_uInt64 nSizeA = mxTreeView->get_text(itA, COLUMN_SORT_SIZE).toInt64();
                sal_uInt64 nSizeB = mxTreeView->get_text(itB, COLUMN_SORT_SIZE).toInt64();

                if(nSizeA < nSizeB)
                    res = -1;
                else if(nSizeA > nSizeB)
                    res = 1;
            }
            break;
            case COLUMN_DISPLAY_PATH:
            {
                OUString sPathA = mxTreeView->get_text(itA, COLUMN_DISPLAY_PATH);
                OUString sPathB = mxTreeView->get_text(itB, COLUMN_DISPLAY_PATH);
                res = pCollatorWrapper->compareString(sPathA, sPathB);
            }
            break;
            case COLUMN_APPLICATION:
            {
                OUString sPathA = mxTreeView->get_text(itA, COLUMN_APPLICATION);
                OUString sPathB = mxTreeView->get_text(itB, COLUMN_APPLICATION);
                res = pCollatorWrapper->compareString(sPathA, sPathB);
            }
            break;
        }
        return isAscending? res : res*(-1);
        });
        mxTreeView->set_sort_column(col);
        mxTreeView->make_unsorted();
    }
}

int ListView::get_sort_indicator_column()
{
    for(int i = 0; i < NUMBER_OF_VISIBLE_COLUMNS; i++)
    {
        if(mxTreeView->get_sort_indicator(i) != TriState::TRISTATE_INDET)
            return i;
    }
    return -1;
}

void ListView::sort()
{
    int col = get_sort_indicator_column();
    sortColumn(col);
}

void ListView::updateIsDefaultColumn()
{
    mxTreeView->all_foreach([&](weld::TreeIter& rIter)
    {
        bool isDefault = SfxTemplateLocalView::IsDefaultTemplate(mxTreeView->get_text(rIter, COLUMN_PATH));
        mxTreeView->set_toggle(rIter, isDefault ? TriState::TRISTATE_TRUE : TriState::TRISTATE_FALSE, COLUMN_SORT_ISDEFAULT);
        return false;
    });
}

void ListView::rename(const OUString &rId, const OUString &rTitle)
{
    mxTreeView->set_text(mxTreeView->find_id(rId), rTitle, COLUMN_NAME);
}

void ListView::remove(const OUString &rId)
{
    mxTreeView->remove_id(rId);
}

void ListView::setDeleteTemplateHdl(const Link<const OUString&, void> &rLink)
{
    maDeleteTemplateHdl = rLink;
}

void ListView::setOpenTemplateHdl(const Link<const OUString&, void> &rLink)
{
    maOpenTemplateHdl = rLink;
}

void ListView::setEditTemplateHdl(const Link<const OUString&, void> &rLink)
{
    maEditTemplateHdl = rLink;
}

void ListView::setDefaultTemplateHdl(const Link<const OUString&, void> &rLink)
{
    maDefaultTemplateHdl = rLink;
}

IMPL_LINK(ListView, ColumnClickedHdl, const int, col, void)
{
    if(col >= 0 && col < NUMBER_OF_VISIBLE_COLUMNS)
    {
        for(int i = 0; i < NUMBER_OF_VISIBLE_COLUMNS; i++)
        {
            if(i != col)
               mxTreeView->set_sort_indicator(TriState::TRISTATE_INDET, i);
        }

        mxTreeView->set_sort_indicator((mxTreeView->get_sort_indicator(col) == TriState::TRISTATE_TRUE ?
                                     TriState::TRISTATE_FALSE : TriState::TRISTATE_TRUE), col);
        sortColumn(col);
    }
}

OUString getFileModifyTime(const OUString& rURL)
{
    if (!comphelper::isFileUrl(rURL))
        return OUString();

    osl::DirectoryItem aItem;
    if (osl::DirectoryItem::get(rURL, aItem) != osl::DirectoryItem::E_None)
        return OUString();

    osl::FileStatus aStatus(osl_FileStatus_Mask_Attributes);
    if (aItem.getFileStatus(aStatus) != osl::DirectoryItem::E_None)
        return OUString();

    TimeValue systemTimeValue = aStatus.getModifyTime();

    OUString sSec = OUString::number(systemTimeValue.Seconds);
    return sSec;
}
OUString getDisplayFileModifyTime(const OUString& rURL)
{
    if (!comphelper::isFileUrl(rURL))
        return OUString();

    osl::DirectoryItem aItem;
    if (osl::DirectoryItem::get(rURL, aItem) != osl::DirectoryItem::E_None)
        return OUString();

    osl::FileStatus aStatus(osl_FileStatus_Mask_Attributes);
    if (aItem.getFileStatus(aStatus) != osl::DirectoryItem::E_None)
        return OUString();

    TimeValue systemTimeValue = aStatus.getModifyTime();
    if (systemTimeValue.Seconds == 0)
        return OUString();
    TimeValue localTimeValue;
    oslDateTime dateTime;
    osl_getLocalTimeFromSystemTime(&systemTimeValue, &localTimeValue);
    osl_getDateTimeFromTimeValue(&localTimeValue, &dateTime);

    struct tm tm;
    tm.tm_sec = dateTime.Seconds;
    tm.tm_min = dateTime.Minutes;
    tm.tm_hour = dateTime.Hours;
    tm.tm_mday = dateTime.Day;
    tm.tm_mon = dateTime.Month - 1;
    tm.tm_year = dateTime.Year - 1900;
    char ts[100];
    strftime(ts, sizeof(ts), "%x %X", &tm);
    return OUString( ts, sizeof(ts), RTL_TEXTENCODING_UTF8 );
}

OUString getDisplayFileSize(const OUString& rURL)
{
    if (!comphelper::isFileUrl(rURL))
        return OUString();

    osl::DirectoryItem aItem;
    if (osl::DirectoryItem::get(rURL, aItem) != osl::DirectoryItem::E_None)
        return OUString();

    osl::FileStatus aStatus(osl_FileStatus_Mask_Attributes);
    if (aItem.getFileStatus(aStatus) != osl::DirectoryItem::E_None)
        return OUString();

    sal_uInt64 nSize = aStatus.getFileSize();
    double fSize( static_cast<double>(nSize) );
    sal_uInt32 nDec;

    sal_uInt64 nMega = 1024 * 1024;
    sal_uInt64 nGiga = nMega * 1024;

    OUString aUnitStr(' ');

    if ( nSize < 10000 )
    {
        aUnitStr += SvtResId(STR_SVT_BYTES );
        nDec = 0;
    }
    else if ( nSize < nMega )
    {
        fSize /= 1024;
        aUnitStr += SvtResId(STR_SVT_KB);
        nDec = 1;
    }
    else if ( nSize < nGiga )
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

    OUString aSizeStr( ::rtl::math::doubleToUString( fSize,
                rtl_math_StringFormat_F, nDec,
                SvtSysLocale().GetLocaleData().getNumDecimalSep()[0]) );
    aSizeStr += aUnitStr;

    return aSizeStr;
}

OUString getFileSize(const OUString& rURL)
{
    if (!comphelper::isFileUrl(rURL))
        return OUString();

    osl::DirectoryItem aItem;
    if (osl::DirectoryItem::get(rURL, aItem) != osl::DirectoryItem::E_None)
        return OUString();

    osl::FileStatus aStatus(osl_FileStatus_Mask_Attributes);
    if (aItem.getFileStatus(aStatus) != osl::DirectoryItem::E_None)
        return OUString();

    sal_uInt64 nSize = aStatus.getFileSize();
    OUString sSize = OUString::number(nSize);

    return sSize;
}

OUString getApplication(const OUString& rURL)
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

    return OUString("");
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
