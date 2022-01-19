/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sfx2/templatelocalview.hxx>

#include <comphelper/string.hxx>
#include <sfx2/doctempl.hxx>
#include <sfx2/inputdlg.hxx>
#include <sfx2/sfxresid.hxx>
#include <templatecontaineritem.hxx>
#include <templateviewitem.hxx>
#include <sfx2/docfac.hxx>
#include <tools/urlobj.hxx>
#include <unotools/moduleoptions.hxx>
#include <vcl/svapp.hxx>
#include <vcl/weld.hxx>
#include <vcl/commandevent.hxx>
#include <vcl/event.hxx>
#include <tools/stream.hxx>
#include <vcl/pngwrite.hxx>
#include <sys/stat.h>
#include <fstream>
#include <iostream>
#include <vcl/svapp.hxx>
#include <vcl/wrkwin.hxx>
#include <vcl/pngread.hxx>

#include <sfx2/strings.hrc>
#include <bitmaps.hlst>

#include <comphelper/processfactory.hxx>
#include <com/sun/star/util/thePathSettings.hpp>
#include <unotools/ucbhelper.hxx>
#include <sfxurlrelocator.hxx>
#include <../doc/doctemplateslocal.hxx>

#if defined(_WIN32)
    #include <direct.h>
    #include <systools/win32/uwinapi.h>
    #include <config_folders.h>
#else
    #include <rtl/bootstrap.hxx>
    #include <osl/file.h>
    #include <osl/file.hxx>
    #include <unotools/pathoptions.hxx>
#endif

#define PATH_SIZE 256
#define LINE_SIZE 256

    sal_uInt16 firstrun = 0, exist_listfile = 0;
    using namespace std;

    struct ThumbnailCache {
        int no;
        char date[40];
        char mapname[20];
        char filename[120];
    };

    vector<struct ThumbnailCache> cachedata ;

    #define X_OFFSET 15
    #define Y_OFFSET 15

    class WaitWindow_Impl : public WorkWindow
    {
        tools::Rectangle     maRect;
        DrawTextFlags mnTextStyle;
        OUString      maText;

    public:
        WaitWindow_Impl(sal_uInt16);
        virtual ~WaitWindow_Impl();
        virtual void dispose() override;
        virtual void Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& rRect) override;
    };

    WaitWindow_Impl::WaitWindow_Impl(sal_uInt16 first_run) : WorkWindow(nullptr, WB_BORDER | WB_3DLOOK)
    {
        tools::Rectangle aRect = tools::Rectangle(0, 0, 400, 30000);
        mnTextStyle = DrawTextFlags::Center | DrawTextFlags::VCenter | DrawTextFlags::WordBreak | DrawTextFlags::MultiLine;
        if(first_run)
            maText = SfxResId(RID_CNT_STR_CACHE);
        else
            maText = SfxResId(RID_CNT_STR_LOAD);
        maRect = GetTextRect(aRect, maText, mnTextStyle);
        aRect = maRect;
        aRect.AdjustRight(2 * X_OFFSET );
        aRect.AdjustBottom(2 * Y_OFFSET );
        maRect.SetPos(Point(X_OFFSET, Y_OFFSET));
        SetOutputSizePixel(aRect.GetSize());

        Show();
        PaintImmediately();
        fflush();
    }

    WaitWindow_Impl::~WaitWindow_Impl()
    {
        disposeOnce();
    }

    void  WaitWindow_Impl::dispose()
    {
        Hide();
        WorkWindow::dispose();
    }

    void WaitWindow_Impl::Paint(vcl::RenderContext& rRenderContext, const tools::Rectangle& /*rRect*/)
    {
        rRenderContext.DrawText(maRect, maText, mnTextStyle);
    }

using namespace ::com::sun::star;

bool ViewFilter_Application::isFilteredExtension(FILTER_APPLICATION filter, std::u16string_view rExt)
{
    bool bRet = rExt == u"ott" || rExt == u"stw" || rExt == u"oth" || rExt == u"dot" || rExt == u"dotx" || rExt == u"otm"
          || rExt == u"ots" || rExt == u"stc" || rExt == u"xlt" || rExt == u"xltm" || rExt == u"xltx"
          || rExt == u"otp" || rExt == u"sti" || rExt == u"pot" || rExt == u"potm" || rExt == u"potx"
          || rExt == u"otg" || rExt == u"std";

    if (filter == FILTER_APPLICATION::WRITER)
    {
        bRet = rExt == u"ott" || rExt == u"stw" || rExt == u"oth" || rExt == u"dot" || rExt == u"dotx" || rExt == u"otm";
    }
    else if (filter == FILTER_APPLICATION::CALC)
    {
        bRet = rExt == u"ots" || rExt == u"stc" || rExt == u"xlt" || rExt == u"xltm" || rExt == u"xltx";
    }
    else if (filter == FILTER_APPLICATION::IMPRESS)
    {
        bRet = rExt == u"otp" || rExt == u"sti" || rExt == u"pot" || rExt == u"potm" || rExt == u"potx";
    }
    else if (filter == FILTER_APPLICATION::DRAW)
    {
        bRet = rExt == u"otg" || rExt == u"std";
    }

    return bRet;
}

bool ViewFilter_Application::isValid (const OUString &rPath) const
{
    INetURLObject aUrl(rPath);
    return isFilteredExtension(mApp, aUrl.getExtension());
}

bool ViewFilter_Application::operator () (const ThumbnailViewItem *pItem)
{
    const TemplateViewItem *pTempItem = dynamic_cast<const TemplateViewItem*>(pItem);
    if (pTempItem)
        return isValid(pTempItem->getPath());

    return true;
}

void TemplateLocalView::updateThumbnailDimensions(tools::Long itemMaxSize)
{
    mnThumbnailWidth = itemMaxSize;
    mnThumbnailHeight = itemMaxSize;
}

TemplateLocalView::TemplateLocalView(std::unique_ptr<weld::ScrolledWindow> xWindow,
                                           std::unique_ptr<weld::Menu> xMenu)
    : ThumbnailView(std::move(xWindow), std::move(xMenu))
    , mnCurRegionId(0)
    , maSelectedItem(nullptr)
    , mnThumbnailWidth(TEMPLATE_THUMBNAIL_MAX_WIDTH)
    , mnThumbnailHeight(TEMPLATE_THUMBNAIL_MAX_HEIGHT)
    , maPosition(0,0)
    , mpDocTemplates(new SfxDocumentTemplates)
{
}

TemplateLocalView::~TemplateLocalView()
{
}

OUString getCacheFolder()
{
    #if defined(_WIN32)
        OUString url("${$BRAND_BASE_DIR/" LIBO_ETC_FOLDER "/" SAL_CONFIGFILE("bootstrap") ":UserInstallation}/cache/");
    #else
        SvtPathOptions aPathOpt;
        OUString url = aPathOpt.GetUserConfigPath() + "/../../cache";
    #endif

    rtl::Bootstrap::expandMacros(url);
    osl::Directory::create(url);
    if( url.startsWith( "file://" ) )
    {
        OUString aSysPath;
        if( osl_getSystemPathFromFileURL( url.pData, &aSysPath.pData ) == osl_File_E_None )
            url = aSysPath;
    }
        return url;
    }

void TemplateLocalView::readlistdata()
    {
        cachedata.clear();
        // read list.txt data to struct
        char line[200];
        unsigned int nAllCount = 0;
        struct ThumbnailCache tmpcachedata = {0,"","",""};
        #if defined(_WIN32)
            OUString Listurl = getCacheFolder() + "\\pic\\list.txt";
        #else
            OUString Listurl = getCacheFolder() + "/pic/list.txt";
        #endif
        FILE *pf = NULL;

        if ((pf = fopen (OUStringToOString( Listurl, RTL_TEXTENCODING_UTF8 ).getStr(), "r")) == NULL) {
            printf ( "could not open file\n");
        }else{
            printf ( "Success open file\n");
            while ((fgets (line, sizeof (line), pf))) {
                if ((sscanf (line, "%d %24[^\n] %10s %120[^\n]"
                ,&tmpcachedata.no ,tmpcachedata.date ,tmpcachedata.mapname ,tmpcachedata.filename)) == 4) {
                    cachedata.push_back(tmpcachedata);
                    exist_listfile = 1;
                }
            }

            sal_uInt16 nCount = mpDocTemplates->GetRegionCount();
            for (sal_uInt16 i = 0; i < nCount; ++i)
            {
                sal_uInt16 nEntries = mpDocTemplates->GetCount(i);

                for (sal_uInt16 j = 0; j < nEntries; ++j)
                {
                    OUString OUS_mapname;
                    if(exist_listfile)
                        OUS_mapname += OStringToOUString(string(cachedata[nAllCount].filename).c_str(), RTL_TEXTENCODING_UTF8);
                    nAllCount++;
                }
            }
            fclose(pf);
        }

        if ((nAllCount != cachedata.size()) || cachedata.size() == 0)
            firstrun = 1;
    }

    OUString cToOUString( const char* s )
    {
        return OUString( s, (sal_Int32) strlen( s ), RTL_TEXTENCODING_UTF8 );
    }

void TemplateLocalView::Populate()
{
    maRegions.clear();
    maAllTemplates.clear();

    VclPtrInstance< WaitWindow_Impl > pWin(firstrun);

    #if defined(_WIN32)
        OUString PicPath = getCacheFolder() + "\\pic";
    #else
        OUString PicPath = getCacheFolder() + "/pic";
    #endif

    FILE* fstream = NULL;
    sal_uInt16 nAllCount = 1, nAllCount2 = 0;
    char cList[PATH_SIZE],cIndex[PATH_SIZE],cDir[PATH_SIZE],cDirB[PATH_SIZE],cDirS[PATH_SIZE];
    char cFile[PATH_SIZE],csFile[PATH_SIZE];
    snprintf(cDir,PATH_SIZE,"%s",OUStringToOString( PicPath, RTL_TEXTENCODING_UTF8 ).getStr());

    #if defined(_WIN32)
        snprintf(cDirB,PATH_SIZE,"%s\\b",cDir);
        snprintf(cDirS,PATH_SIZE,"%s\\s",cDir);
        snprintf(cList,PATH_SIZE,"%s\\list.txt",cDir);
    #else
        snprintf(cDirB,PATH_SIZE,"%s/b",cDir);
        snprintf(cDirS,PATH_SIZE,"%s/s",cDir);
        snprintf(cList,PATH_SIZE,"%s/list.txt",cDir);
    #endif

    /* if directory does not exist, create it */
    struct stat *cDirbuf = (struct stat *) malloc(sizeof(struct stat));
    struct stat *cFilebuf = (struct stat *) malloc(sizeof(struct stat));
    if (stat(cDirB, cDirbuf) != 0 || stat(cDirS, cDirbuf) != 0)
    {
        #if defined(_WIN32)
            CreateDirectory(cDir, NULL);
            CreateDirectory(cDirB, NULL);
            CreateDirectory(cDirS, NULL);
        #else
            mkdir(cDir,S_IRWXU | S_IRWXG |S_IRWXO);
            mkdir(cDirB,S_IRWXU | S_IRWXG |S_IRWXO);
            mkdir(cDirS,S_IRWXU | S_IRWXG |S_IRWXO);
        #endif
    }else{
        if( remove( cList ) != 0 )
            perror( "Error deleting file" );
        else
            printf("remove list.txt OK!\n");
    }

    fstream = fopen(cList,"a");

    sal_uInt16 nCount = mpDocTemplates->GetRegionCount();
    for (sal_uInt16 i = 0; i < nCount; ++i)
    {
        OUString aRegionName(mpDocTemplates->GetFullRegionName(i));

        std::unique_ptr<TemplateContainerItem> pItem(new TemplateContainerItem( i+1 ));
        pItem->mnRegionId = i;
        pItem->maTitle = aRegionName;

        sal_uInt16 nEntries = mpDocTemplates->GetCount(i);

        for (sal_uInt16 j = 0; j < nEntries; ++j)
        {
            OUString aName = mpDocTemplates->GetName(i,j);
            OUString aURL = mpDocTemplates->GetPath(i,j);

            TemplateItemProperties aProperties;
            aProperties.nId = j+1;
            aProperties.nDocId = j;
            aProperties.nRegionId = i;
            aProperties.aName = aName;
            aProperties.aPath = aURL;
            aProperties.aRegionName = aRegionName;

            std::fill_n(cIndex, PATH_SIZE, 0);
            std::fill_n(cFile, PATH_SIZE, 0);
            std::fill_n(csFile, PATH_SIZE, 0);
            BitmapEx cImg;
            OUString imgurl/*,imgurl2*/;
            OUString ii = OStringToOUString(std::to_string(i).c_str(), RTL_TEXTENCODING_UTF8);
            OUString jj = OStringToOUString(std::to_string(j).c_str(), RTL_TEXTENCODING_UTF8);

            #if defined(_WIN32)
                if(mnThumbnailWidth < TEMPLATE_ITEM_MAX_WIDTH) {
                    imgurl += PicPath + "\\s\\" + ii + jj +".png";
                    firstrun = 0;
                } else {
                    imgurl += PicPath + "\\b\\" + ii + jj +".png";
                }
            #else
                if(mnThumbnailWidth < TEMPLATE_ITEM_MAX_WIDTH) {
                    imgurl += PicPath + "/s/" + ii + jj +".png";
                    firstrun = 0;
                } else {
                    imgurl += PicPath + "/b/" + ii + jj +".png";
                }
            #endif

            snprintf(cFile,PATH_SIZE,OUStringToOString( imgurl, RTL_TEXTENCODING_UTF8 ).getStr());

            // check png file exist
            OUString OUS_mapname;
            if(exist_listfile)
                OUS_mapname += OStringToOUString(string(cachedata[nAllCount2].filename).c_str(), RTL_TEXTENCODING_UTF8);
            if((aName.compareTo(OUS_mapname) != 0) || (stat(cFile, cFilebuf) != 0))
            {
                snprintf(csFile,PATH_SIZE,OUStringToOString( OUString(PicPath + "/s/" + ii + jj +".png"), RTL_TEXTENCODING_UTF8 ).getStr());
                remove(csFile);

                // wirte list.txt
                snprintf(cIndex,LINE_SIZE,"%-4d %-20s  %d%d  %-s\n",nAllCount++,"Sun Oct 29 15:03:07 2015",i,j,OUStringToOString( aName, RTL_TEXTENCODING_UTF8 ).getStr());
                fwrite(cIndex,1,strlen(cIndex),fstream);

                cImg = TemplateLocalView::fetchThumbnail(aURL, mnThumbnailWidth, mnThumbnailHeight);
                // write cache png
                SvFileStream aNew(imgurl, StreamMode::WRITE|StreamMode::TRUNC);
                vcl::PNGWriter aPNGWriter(cImg);
                aPNGWriter.Write(aNew);
                aNew.Close();
            } else {
                // wirte list.txt
                snprintf(cIndex,LINE_SIZE,"%-4d %-20s  %d%d  %-s\n",nAllCount++,"Sun Oct 29 15:03:07 2017",i,j,cachedata[nAllCount2].filename);
                fwrite(cIndex,1,strlen(cIndex),fstream);

                OUString PngURL;

                #if defined(_WIN32)

                    if(mnThumbnailHeight < TEMPLATE_ITEM_MAX_WIDTH)
                    {
                        PngURL += PicPath + "\\s\\" + OStringToOUString(string(cachedata[nAllCount2].mapname).c_str(), RTL_TEXTENCODING_UTF8) +".png";
                    } else {
                        PngURL += PicPath + "\\b\\" + OStringToOUString(string(cachedata[nAllCount2].mapname).c_str(), RTL_TEXTENCODING_UTF8) +".png";
                    }
                #else
                    if(mnThumbnailHeight < TEMPLATE_ITEM_MAX_WIDTH)
                    {
                        PngURL += PicPath + "/s/" + OStringToOUString(string(cachedata[nAllCount2].mapname).c_str(), RTL_TEXTENCODING_UTF8) +".png";
                    } else {
                        PngURL += PicPath + "/b/" + OStringToOUString(string(cachedata[nAllCount2].mapname).c_str(), RTL_TEXTENCODING_UTF8) +".png";
                    }

                #endif

                SvFileStream aFileStream(PngURL, StreamMode::READ);
                vcl::PNGReader aPNGReader(aFileStream);
                cImg = aPNGReader.Read();
            }
            nAllCount2++;
            aProperties.aThumbnail = cImg;
            pItem->maTemplates.push_back(aProperties);
            maAllTemplates.push_back(aProperties);
        }

        maRegions.push_back(std::move(pItem));
    }

    pWin.disposeAndClear();
    free(cDirbuf);
    free(cFilebuf);
    fclose(fstream);
}

void TemplateLocalView::reload()
{
    mpDocTemplates->Update();
    OUString sCurRegionName = getRegionItemName(mnCurRegionId);
    Populate();
    mnCurRegionId = getRegionId(sCurRegionName);

    // Check if we are currently browsing a region or root folder
    if (mnCurRegionId)
    {
        sal_uInt16 nRegionId = mnCurRegionId - 1;   //Is offset by 1

        for (auto const & pRegion : maRegions)
        {
            if (pRegion->mnRegionId == nRegionId)
            {
                showRegion(pRegion.get());
                break;
            }
        }
    }
    else
        showAllTemplates();

    //No items should be selected by default
    deselectItems();
}

void TemplateLocalView::showAllTemplates()
{
    mnCurRegionId = 0;

    insertItems(maAllTemplates, false, true);

    maOpenRegionHdl.Call(nullptr);
}

void TemplateLocalView::showRegion(TemplateContainerItem const *pItem)
{
    mnCurRegionId = pItem->mnRegionId+1;

    insertItems(pItem->maTemplates);

    maOpenRegionHdl.Call(nullptr);
}

TemplateContainerItem* TemplateLocalView::getRegion(std::u16string_view rName)
{
    for (auto const & pRegion : maRegions)
        if (pRegion->maTitle == rName)
            return pRegion.get();

    return nullptr;
}

void TemplateLocalView::ContextMenuSelectHdl(std::string_view  rIdent)
{
    if (rIdent == "open")
        maOpenTemplateHdl.Call(maSelectedItem);
    else if (rIdent == "edit")
        maEditTemplateHdl.Call(maSelectedItem);
    else if (rIdent == "rename")
    {
        InputDialog aTitleEditDlg(GetDrawingArea(), SfxResId(STR_RENAME_TEMPLATE));
        OUString sOldTitle = maSelectedItem->getTitle();
        aTitleEditDlg.SetEntryText(sOldTitle);
        aTitleEditDlg.HideHelpBtn();

        auto aCurRegionItems = getFilteredItems([&](const TemplateItemProperties& rItem) {
            return rItem.aRegionName == getRegionName(maSelectedItem->mnRegionId);
        });
        OUString sTooltip(SfxResId(STR_TOOLTIP_ERROR_RENAME_TEMPLATE));
        sTooltip = sTooltip.replaceFirst("$2", getRegionName(maSelectedItem->mnRegionId));
        aTitleEditDlg.setCheckEntry([&](OUString sNewTitle) {
            if (sNewTitle.isEmpty() || sNewTitle == sOldTitle)
                return true;
            for (const auto& rItem : aCurRegionItems)
            {
                if (rItem.aName == sNewTitle)
                {
                    aTitleEditDlg.SetTooltip(sTooltip.replaceFirst("$1", sNewTitle));
                    return false;
                }
            }
            return true;
        });
        if (!aTitleEditDlg.run())
            return;
        OUString sNewTitle = comphelper::string::strip(aTitleEditDlg.GetEntryText(), ' ');

        if ( !sNewTitle.isEmpty() && sNewTitle != sOldTitle )
        {
            maSelectedItem->setTitle(sNewTitle);
        }
    }
    else if (rIdent == "delete")
    {
        std::unique_ptr<weld::MessageDialog> xQueryDlg(Application::CreateMessageDialog(GetDrawingArea(), VclMessageType::Question, VclButtonsType::YesNo,
                                                       SfxResId(STR_QMSG_SEL_TEMPLATE_DELETE)));
        if (xQueryDlg->run() != RET_YES)
            return;

        maDeleteTemplateHdl.Call(maSelectedItem);
        reload();
    }
    else if (rIdent == "default")
        maDefaultTemplateHdl.Call(maSelectedItem);
}

sal_uInt16 TemplateLocalView::getRegionId(size_t pos) const
{
    assert(pos < maRegions.size());

    return maRegions[pos]->mnId;
}

sal_uInt16 TemplateLocalView::getRegionId(std::u16string_view sRegion) const
{
    for (auto const & pRegion : maRegions)
    {
        if (pRegion->maTitle == sRegion)
            return pRegion->mnId;
    }

    return 0;
}

OUString TemplateLocalView::getRegionName(const sal_uInt16 nRegionId) const
{
    return mpDocTemplates->GetRegionName(nRegionId);
}

OUString TemplateLocalView::getRegionItemName(const sal_uInt16 nItemId) const
{
    for (auto const & pRegion : maRegions)
    {
        if (pRegion->mnId == nItemId)
            return pRegion->maTitle;
    }

    return OUString();
}

std::vector<OUString> TemplateLocalView::getFolderNames()
{
    size_t n = maRegions.size();
    std::vector<OUString> ret(n);

    for (size_t i = 0; i < n; ++i)
        ret[i] = maRegions[i]->maTitle;

    return ret;
}

std::vector<TemplateItemProperties>
TemplateLocalView::getFilteredItems(const std::function<bool (const TemplateItemProperties&)> &rFunc) const
{
    std::vector<TemplateItemProperties> aItems;

    if (mnCurRegionId)
    {
        TemplateContainerItem *pFolderItem = maRegions[mnCurRegionId-1].get();

        for (const TemplateItemProperties & rItemProps : pFolderItem->maTemplates)
        {
            if (rFunc(rItemProps))
                aItems.push_back(rItemProps);
        }
    }
    else
    {
        for (auto const & pFolderItem : maRegions)
        {
            for (const TemplateItemProperties & rItemProps : pFolderItem->maTemplates)
            {
                if (rFunc(rItemProps))
                    aItems.push_back(rItemProps);
            }
        }
    }

    return aItems;
}

sal_uInt16 TemplateLocalView::createRegion(const OUString &rName)
{
    sal_uInt16 nRegionId = mpDocTemplates->GetRegionCount();    // Next regionId
    sal_uInt16 nItemId = maRegions.size() + 1;

    if (!mpDocTemplates->InsertDir(rName,nRegionId))
        return 0;

    // Insert to the region cache list and to the thumbnail item list
    std::unique_ptr<TemplateContainerItem> pItem(new TemplateContainerItem( nItemId ));
    pItem->mnRegionId = nRegionId;
    pItem->maTitle = rName;

    maRegions.push_back(std::move(pItem));

    return nItemId;
}

bool TemplateLocalView::renameRegion(std::u16string_view rTitle, const OUString &rNewTitle)
{
    TemplateContainerItem *pRegion = getRegion(rTitle);

    if(pRegion)
    {
        sal_uInt16 nRegionId = pRegion->mnRegionId;
        return mpDocTemplates->SetName( rNewTitle, nRegionId, USHRT_MAX/*nDocId*/ );
    }
    return false;
}

bool TemplateLocalView::removeRegion(const sal_uInt16 nItemId)
{
    sal_uInt16 nRegionId = USHRT_MAX;

    // Remove from the region cache list
    for (auto pRegionIt = maRegions.begin(); pRegionIt != maRegions.end();)
    {
        if ( (*pRegionIt)->mnId == nItemId )
        {
            if (!mpDocTemplates->Delete((*pRegionIt)->mnRegionId,USHRT_MAX))
                return false;

            nRegionId = (*pRegionIt)->mnRegionId;

            pRegionIt = maRegions.erase(pRegionIt);
        }
        else
        {
            // Synchronize regions cache ids with SfxDocumentTemplates
            if (nRegionId != USHRT_MAX && (*pRegionIt)->mnRegionId > nRegionId)
                --(*pRegionIt)->mnRegionId;

            ++pRegionIt;
        }
    }

    if (nRegionId == USHRT_MAX)
        return false;

    // Synchronize view regions ids with SfxDocumentTemplates
    for (auto const& region : maRegions)
    {
        if (region->mnRegionId > nRegionId)
            --region->mnRegionId;
    }

    return true;
}

bool TemplateLocalView::removeTemplate (const sal_uInt16 nItemId, const sal_uInt16 nSrcItemId)
{
    for (auto const & pRegion : maRegions)
    {
        if (pRegion->mnId == nSrcItemId)
        {
            TemplateContainerItem *pItem = pRegion.get();
            auto pIter = std::find_if(pItem->maTemplates.begin(), pItem->maTemplates.end(),
                [nItemId](const TemplateItemProperties& rTemplate) { return rTemplate.nId == nItemId; });
            if (pIter != pItem->maTemplates.end())
            {
                if (!mpDocTemplates->Delete(pItem->mnRegionId,pIter->nDocId))
                    return false;

                pIter = pItem->maTemplates.erase(pIter);

                if (pRegion->mnRegionId == mnCurRegionId-1)
                {
                    RemoveItem(nItemId);
                    Invalidate();
                }

                // Update Doc Idx for all templates that follow
                for (; pIter != pItem->maTemplates.end(); ++pIter)
                    pIter->nDocId = pIter->nDocId - 1;
            }

            CalculateItemPositions();
            break;
        }
    }

    return true;
}

void TemplateLocalView::moveTemplates(const std::set<const ThumbnailViewItem*, selection_cmp_fn> &rItems,
                                      const sal_uInt16 nTargetItem)
{
    TemplateContainerItem *pTarget = nullptr;
    TemplateContainerItem *pSrc = nullptr;

    for (auto const & pRegion : maRegions)
    {
        if (pRegion->mnId == nTargetItem)
            pTarget = pRegion.get();
    }

    if (!pTarget)
        return;

    bool refresh = false;

    sal_uInt16 nTargetRegion = pTarget->mnRegionId;
    sal_uInt16 nTargetIdx = mpDocTemplates->GetCount(nTargetRegion);    // Next Idx
    std::vector<sal_uInt16> aItemIds;    // List of moved items ids (also prevents the invalidation of rItems iterators when we remove them as we go)

    std::set<const ThumbnailViewItem*,selection_cmp_fn>::const_iterator aSelIter;
    for ( aSelIter = rItems.begin(); aSelIter != rItems.end(); ++aSelIter, ++nTargetIdx )
    {
        const TemplateViewItem *pViewItem = static_cast<const TemplateViewItem*>(*aSelIter);
        sal_uInt16 nSrcRegionId = pViewItem->mnRegionId;

        for (auto const & pRegion : maRegions)
        {
            if (pRegion->mnRegionId == nSrcRegionId)
                pSrc = pRegion.get();
        }

        if(pSrc)
        {
            bool bCopy = !mpDocTemplates->Move(nTargetRegion,nTargetIdx,nSrcRegionId,pViewItem->mnDocId);

            if (bCopy)
            {
                OUString sQuery = SfxResId(STR_MSG_QUERY_COPY).replaceFirst("$1", pViewItem->maTitle).replaceFirst("$2",
                    getRegionName(nTargetRegion));
                std::unique_ptr<weld::MessageDialog> xQueryDlg(Application::CreateMessageDialog(GetDrawingArea(), VclMessageType::Question, VclButtonsType::YesNo, sQuery));
                if (xQueryDlg->run() != RET_YES)
                {
                    OUString sMsg(SfxResId(STR_MSG_ERROR_LOCAL_MOVE));
                    sMsg = sMsg.replaceFirst("$1",getRegionName(nTargetRegion));
                    std::unique_ptr<weld::MessageDialog> xBox(Application::CreateMessageDialog(GetDrawingArea(),
                                                              VclMessageType::Warning, VclButtonsType::Ok, sMsg.replaceFirst( "$2",pViewItem->maTitle)));
                    xBox->run();

                    return; //return if any single move operation fails
                }

                if (!mpDocTemplates->Copy(nTargetRegion,nTargetIdx,nSrcRegionId,pViewItem->mnDocId))
                {
                    continue;
                }
            }

            // move template to destination

            TemplateItemProperties aTemplateItem;
            aTemplateItem.nId = nTargetIdx + 1;
            aTemplateItem.nDocId = nTargetIdx;
            aTemplateItem.nRegionId = nTargetRegion;
            aTemplateItem.aName = pViewItem->maTitle;
            aTemplateItem.aPath = mpDocTemplates->GetPath(nTargetRegion,nTargetIdx);
            aTemplateItem.aRegionName = pViewItem->maHelpText;
            aTemplateItem.aThumbnail = pViewItem->maPreview1;

            pTarget->maTemplates.push_back(aTemplateItem);

            if (!bCopy)
            {
                // remove template from region cached data

                std::vector<TemplateItemProperties>::iterator pPropIter;
                for (pPropIter = pSrc->maTemplates.begin(); pPropIter != pSrc->maTemplates.end();)
                {
                    if (pPropIter->nDocId == pViewItem->mnDocId)
                    {
                        pPropIter = pSrc->maTemplates.erase(pPropIter);
                        aItemIds.push_back(pViewItem->mnDocId + 1);//mnid
                    }
                    else
                    {
                        // Keep region document id synchronized with SfxDocumentTemplates
                        if (pPropIter->nDocId > pViewItem->mnDocId)
                            --pPropIter->nDocId;

                        ++pPropIter;
                    }
                }

                // Keep view document id synchronized with SfxDocumentTemplates
                for (auto const& item : mItemList)
                {
                    auto pTemplateViewItem = static_cast<TemplateViewItem*>(item.get());
                    if (pTemplateViewItem->mnDocId > pViewItem->mnDocId)
                        --pTemplateViewItem->mnDocId;
                }
            }
        }

        refresh = true;
    }

    // Remove items from the current view
    for (auto const& itemId : aItemIds)
        RemoveItem(itemId);

    if (refresh)
    {
        CalculateItemPositions();
        Invalidate();
    }
}

bool TemplateLocalView::copyFrom (TemplateContainerItem *pItem, const OUString &rPath)
{
    sal_uInt16 nId = 1;
    sal_uInt16 nDocId = 0;
    sal_uInt16 nRegionId = pItem->mnRegionId;
    OUString aPath(rPath);

    if (!pItem->maTemplates.empty())
    {
        nId = pItem->maTemplates.back().nId+1;
        nDocId = pItem->maTemplates.back().nDocId+1;
    }

    if (mpDocTemplates->CopyFrom(nRegionId,nDocId,aPath))
    {
        TemplateItemProperties aTemplate;
        aTemplate.nId = nId;
        aTemplate.nDocId = nDocId;
        aTemplate.nRegionId = nRegionId;
        aTemplate.aName = aPath;
        aTemplate.aThumbnail = TemplateLocalView::fetchThumbnail(rPath,
                                                                    TEMPLATE_THUMBNAIL_MAX_WIDTH,
                                                                    TEMPLATE_THUMBNAIL_MAX_HEIGHT);
        aTemplate.aPath = rPath;
        aTemplate.aRegionName = getRegionName(nRegionId);

        pItem->maTemplates.push_back(aTemplate);

        CalculateItemPositions();

        return true;
    }

    return false;
}

bool TemplateLocalView::exportTo(const sal_uInt16 nItemId, const sal_uInt16 nRegionItemId, const OUString &rName)
{
    for (auto const & pRegItem : maRegions)
    {
        if (pRegItem->mnId == nRegionItemId)
        {
            for (auto const& elem : pRegItem->maTemplates)
            {
                if (elem.nId == nItemId)
                {
                    return mpDocTemplates->CopyTo(pRegItem->mnRegionId,elem.nDocId,rName);
                }
            }

            break;
        }
    }

    return false;
}

bool TemplateLocalView::renameItem(ThumbnailViewItem* pItem, const OUString& sNewTitle)
{
    sal_uInt16 nRegionId = 0;
    sal_uInt16 nDocId = USHRT_MAX;
    TemplateViewItem* pDocItem = dynamic_cast<TemplateViewItem*>( pItem );

    if ( pDocItem )
    {
        nRegionId = pDocItem->mnRegionId;
        nDocId = pDocItem->mnDocId;
    }

    bool bRes = mpDocTemplates->SetName( sNewTitle, nRegionId, nDocId );
    if(bRes)
    {
        for (auto & pRegion : maRegions)
        {
            if (pRegion->mnId == nRegionId + 1 )
            {
                for(auto & aTemplate : pRegion->maTemplates)
                {
                    if(aTemplate.nId == nDocId + 1)
                    {
                        aTemplate.aName = sNewTitle;
                        break;
                    }
                }
                break;
            }
        }
        OUString sRegionName;
        for (auto & aTemplate : maAllTemplates)
        {
            if (aTemplate.nRegionId == nRegionId && aTemplate.nDocId == nDocId)
            {
                aTemplate.aName = sNewTitle;
                sRegionName = aTemplate.aRegionName;
                break;
            }
        }

        OUString sHelpText = SfxResId(STR_TEMPLATE_TOOLTIP);
        sHelpText = (sHelpText.replaceFirst("$1", sNewTitle)).replaceFirst("$2", sRegionName);
        pItem->setHelpText(sHelpText);
        pItem->maTitle = sNewTitle;
    }
    return bRes;
}

void TemplateLocalView::insertItems(const std::vector<TemplateItemProperties> &rTemplates, bool isRegionSelected, bool bShowCategoryInTooltip)
{
    std::vector<std::unique_ptr<ThumbnailViewItem>> aItems(rTemplates.size());
    for (size_t i = 0, n = rTemplates.size(); i < n; ++i )
    {
        const TemplateItemProperties *pCur = &rTemplates[i];

        std::unique_ptr<TemplateViewItem> pChild;
        if(isRegionSelected)
            pChild.reset(new TemplateViewItem(*this, pCur->nId));
        else
            pChild.reset(new TemplateViewItem(*this, i+1));

        pChild->mnDocId = pCur->nDocId;
        pChild->mnRegionId = pCur->nRegionId;
        pChild->maTitle = pCur->aName;
        pChild->setPath(pCur->aPath);

        if(!bShowCategoryInTooltip)
            pChild->setHelpText(pCur->aName);
        else
        {
            OUString sHelpText = SfxResId(STR_TEMPLATE_TOOLTIP);
            sHelpText = (sHelpText.replaceFirst("$1", pCur->aName)).replaceFirst("$2", pCur->aRegionName);
            pChild->setHelpText(sHelpText);
        }

        pChild->maPreview1 = pCur->aThumbnail;

        if(IsDefaultTemplate(pCur->aPath))
            pChild->showDefaultIcon(true);

        if ( pCur->aThumbnail.IsEmpty() )
        {
            // Use the default thumbnail if we have nothing else
            pChild->maPreview1 = TemplateLocalView::getDefaultThumbnail(pCur->aPath);
        }

        aItems[i] = std::move(pChild);
    }

    updateItems(std::move(aItems));
}

bool TemplateLocalView::MouseButtonDown( const MouseEvent& rMEvt )
{
    GrabFocus();
    return ThumbnailView::MouseButtonDown(rMEvt);
}

bool TemplateLocalView::Command(const CommandEvent& rCEvt)
{
    if (rCEvt.GetCommand() != CommandEventId::ContextMenu)
        return CustomWidgetController::Command(rCEvt);

    if (rCEvt.IsMouseEvent())
    {
        size_t nPos = ImplGetItem(rCEvt.GetMousePosPixel());
        Point aPosition(rCEvt.GetMousePosPixel());
        maPosition = aPosition;
        ThumbnailViewItem* pItem = ImplGetItem(nPos);
        const TemplateViewItem *pViewItem = dynamic_cast<const TemplateViewItem*>(pItem);

        if(pViewItem)
        {
            if(!pItem->isSelected())
            {
                deselectItems();
                pItem->setSelection(true);
                maItemStateHdl.Call(pItem);
            }
            maSelectedItem = dynamic_cast<TemplateViewItem*>(pItem);
            maCreateContextMenuHdl.Call(pItem);
        }
    }
    else
    {
        for (ThumbnailViewItem* pItem : mFilteredItemList)
        {
            if (pItem->isSelected())
            {
                tools::Rectangle aRect = pItem->getDrawArea();
                maPosition = aRect.Center();
                maSelectedItem = dynamic_cast<TemplateViewItem*>(pItem);
                maCreateContextMenuHdl.Call(pItem);
                break;
            }
        }
    }
    return true;
}

bool TemplateLocalView::KeyInput( const KeyEvent& rKEvt )
{
    vcl::KeyCode aKeyCode = rKEvt.GetKeyCode();

    if(aKeyCode == ( KEY_MOD1 | KEY_A ) )
    {
        for (ThumbnailViewItem* pItem : mFilteredItemList)
        {
            if (!pItem->isSelected())
            {
                pItem->setSelection(true);
                maItemStateHdl.Call(pItem);
            }
        }

        if (IsReallyVisible() && IsUpdateMode())
            Invalidate();
        return true;
    }
    else if( aKeyCode == KEY_DELETE && !mFilteredItemList.empty())
    {
        std::unique_ptr<weld::MessageDialog> xQueryDlg(Application::CreateMessageDialog(GetDrawingArea(), VclMessageType::Question, VclButtonsType::YesNo,
                                                       SfxResId(STR_QMSG_SEL_TEMPLATE_DELETE)));
        if (xQueryDlg->run() != RET_YES)
            return true;

        //copy to avoid changing filtered item list during deletion
        ThumbnailValueItemList mFilteredItemListCopy = mFilteredItemList;

        for (ThumbnailViewItem* pItem : mFilteredItemListCopy)
        {
            if (pItem->isSelected())
            {
                maDeleteTemplateHdl.Call(pItem);
            }
        }
        reload();
    }

    return ThumbnailView::KeyInput(rKEvt);
}

void TemplateLocalView::setOpenRegionHdl(const Link<void*,void> &rLink)
{
    maOpenRegionHdl = rLink;
}

void TemplateLocalView::setCreateContextMenuHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maCreateContextMenuHdl = rLink;
}

void TemplateLocalView::setOpenTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maOpenTemplateHdl = rLink;
}

void TemplateLocalView::setEditTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maEditTemplateHdl = rLink;
}

void TemplateLocalView::setDeleteTemplateHdl(const Link<void*,void> &rLink)
{
    maDeleteTemplateHdl = rLink;
}

void TemplateLocalView::setDefaultTemplateHdl(const Link<ThumbnailViewItem*,void> &rLink)
{
    maDefaultTemplateHdl = rLink;
}

void TemplateLocalView::setMoveTemplateHdl(const Link<void*,void> &rLink)
{
    maMoveTemplateHdl = rLink;
}

void TemplateLocalView::setExportTemplateHdl(const Link<void*,void> &rLink)
{
    maExportTemplateHdl = rLink;
}

BitmapEx TemplateLocalView::scaleImg (const BitmapEx &rImg, tools::Long width, tools::Long height)
{
    BitmapEx aImg = rImg;

    if (!rImg.IsEmpty())
    {
        Size aSize = rImg.GetSizePixel();

        if (aSize.Width() == 0)
            aSize.setWidth( 1 );

        if (aSize.Height() == 0)
            aSize.setHeight( 1 );

        // make the picture fit the given width/height constraints
        double nRatio = std::min(double(width)/double(aSize.Width()), double(height)/double(aSize.Height()));
        // when scaling use the full 24bit RGB values
        aImg.Convert(BmpConversion::N24Bit);
        aImg.Scale(Size(aSize.Width() * nRatio, aSize.Height() * nRatio));
    }

    return aImg;
}

bool TemplateLocalView::IsDefaultTemplate(const OUString& rPath)
{
    SvtModuleOptions aModOpt;
    const css::uno::Sequence<OUString> &aServiceNames = aModOpt.GetAllServiceNames();

    return std::any_of(aServiceNames.begin(), aServiceNames.end(), [&rPath](const OUString& rName) {
        return SfxObjectFactory::GetStandardTemplate(rName).match(rPath); });
}

void TemplateLocalView::RemoveDefaultTemplateIcon(std::u16string_view rPath)
{
    for (const std::unique_ptr<ThumbnailViewItem>& pItem : mItemList)
    {
        TemplateViewItem* pViewItem = dynamic_cast<TemplateViewItem*>(pItem.get());
        if (pViewItem && pViewItem->getPath().match(rPath))
        {
            pViewItem->showDefaultIcon(false);
            Invalidate();
            return;
        }
    }
}

BitmapEx TemplateLocalView::getDefaultThumbnail( const OUString& rPath )
{
    BitmapEx aImg;
    INetURLObject aUrl(rPath);
    OUString aExt = aUrl.getExtension();

    if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::WRITER, aExt) )
        aImg = BitmapEx(SFX_THUMBNAIL_TEXT);
    else if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::CALC, aExt) )
        aImg = BitmapEx(SFX_THUMBNAIL_SHEET);
    else if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::IMPRESS, aExt) )
        aImg = BitmapEx(SFX_THUMBNAIL_PRESENTATION);
    else if ( ViewFilter_Application::isFilteredExtension( FILTER_APPLICATION::DRAW, aExt) )
        aImg = BitmapEx(SFX_THUMBNAIL_DRAWING);

    return aImg;
}

BitmapEx TemplateLocalView::fetchThumbnail (const OUString &msURL, tools::Long width, tools::Long height)
{
    return TemplateLocalView::scaleImg(ThumbnailView::readThumbnail(msURL), width, height);
}

void TemplateLocalView::OnItemDblClicked (ThumbnailViewItem *pItem)
{
    TemplateViewItem* pViewItem = dynamic_cast<TemplateViewItem*>(pItem);

    if( pViewItem )
        maOpenTemplateHdl.Call(pViewItem);
}

bool TemplateLocalView::IsInternalTemplate(const OUString& rPath)
{
    uno::Reference< uno::XComponentContext > xContext = ::comphelper::getProcessComponentContext();
    css::uno::Reference< css::util::XPathSettings > xPathSettings = css::util::thePathSettings::get(xContext);
    uno::Sequence<OUString> aInternalTemplateDirs;
    uno::Any aAny = xPathSettings->getPropertyValue("Template_internal");
    aAny >>= aInternalTemplateDirs;
    SfxURLRelocator_Impl aRelocator(xContext);
    for (OUString& rInternalTemplateDir : asNonConstRange(aInternalTemplateDirs))
    {
        aRelocator.makeRelocatableURL(rInternalTemplateDir);
        aRelocator.makeAbsoluteURL(rInternalTemplateDir);
        if(::utl::UCBContentHelper::IsSubPath(rInternalTemplateDir, rPath))
            return true;
    }
    return false;
}

bool TemplateLocalView::IsBuiltInRegion(const OUString& rRegionName)
{
    bool isBuiltInCategory = false;
    auto aGroupNames = DocTemplLocaleHelper::GetBuiltInGroupNames();
    isBuiltInCategory = std::find(aGroupNames.begin(), aGroupNames.end(),
                                  rRegionName) != aGroupNames.end();
    if(isBuiltInCategory)
        return true;
    //check if it contains any internal template
    for(const auto& rItem : maRegions)
    {
        if(rItem->maTitle == rRegionName)
        {
            for(const auto& rTemplateItem : rItem->maTemplates)
            {
                if(IsInternalTemplate(rTemplateItem.aPath))
                    return true;
            }
            break;
        }
    }
    return false;
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
