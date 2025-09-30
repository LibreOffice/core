/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 * Copyright 2014-2018 Adobe Systems Incorporated (http://www.adobe.com/). All Rights Reserved.
 * This software is licensed as OpenSource, under the Apache License, Version 2.0.
 * This license is available at: http://opensource.org/licenses/Apache-2.0.
 */

#include <sal/config.h>

#include <config_features.h>

#if HAVE_FEATURE_PDFIMPORT

#include <osl/file.hxx>
#include <rtl/strbuf.hxx>
#include <vcl/embeddedfontsmanager.hxx>
#include "afdko.hxx"

static bool convertTx(txCtx h)
{
    h->src.stm.fp = fopen(h->src.stm.filename, "rb");
    if (!h->src.stm.fp)
        return false;

    h->src.stm.flags = STM_DONT_CLOSE;

    buildFontList(h);

    for (long i = 0; i < h->fonts.cnt; ++i)
    {
        const FontRec& rec = h->fonts.array[i];

        h->src.type = rec.type;

        switch (h->src.type)
        {
            case src_Type1:
                t1rReadFont(h, rec.offset);
                break;
            case src_OTF:
                h->cfr.flags |= CFR_NO_ENCODING;
                [[fallthrough]];
            case src_CFF:
                cfrReadFont(h, rec.offset, rec.iTTC);
                break;
            case src_TrueType:
                ttrReadFont(h, rec.offset, rec.iTTC);
                break;
            default:
                SAL_WARN("vcl.fonts", "unhandled font type: " << h->src.type);
                break;
        }
    }

    return true;
}

// System afdko could be used by calling: tx -dump src dest here
bool EmbeddedFontsManager::tx_dump(const OUString& srcFontUrl, const OUString& destFileUrl)
{
    OUString srcFontPath, destFilePath;
    if (osl::FileBase::E_None != osl::FileBase::getSystemPathFromFileURL(srcFontUrl, srcFontPath)
        || osl::FileBase::E_None
               != osl::FileBase::getSystemPathFromFileURL(destFileUrl, destFilePath))
    {
        SAL_WARN("vcl.fonts", "path failure");
        return false;
    }

    txCtx h = txNew(nullptr);
    if (!h)
        return false;

    OString srcFontPathA(srcFontPath.toUtf8());
    h->src.stm.filename = const_cast<char*>(srcFontPathA.getStr());
    OString destFilePathA(destFilePath.toUtf8());
    h->dst.stm.filename = const_cast<char*>(destFilePathA.getStr());
    bool result = convertTx(h);
    txFree(h);
    return result;
}

// System afdko could be used by calling: tx -t1 src dest here
bool EmbeddedFontsManager::tx_t1(const OUString& srcFontUrl, const OUString& destFileUrl)
{
    OUString srcFontPath, destFilePath;
    if (osl::FileBase::E_None != osl::FileBase::getSystemPathFromFileURL(srcFontUrl, srcFontPath)
        || osl::FileBase::E_None
               != osl::FileBase::getSystemPathFromFileURL(destFileUrl, destFilePath))
    {
        SAL_WARN("vcl.fonts", "path failure");
        return false;
    }

    txCtx h = txNew(nullptr);
    if (!h)
        return false;

    setMode(h, mode_t1);

    OString srcFontPathA(srcFontPath.toUtf8());
    h->src.stm.filename = const_cast<char*>(srcFontPathA.getStr());
    OString destFilePathA(destFilePath.toUtf8());
    h->dst.stm.filename = const_cast<char*>(destFilePathA.getStr());
    bool result = convertTx(h);
    txFree(h);
    return result;
}

// System afdko could be used by calling: mergefonts -cid cidfontinfo destfile [glyphaliasfile mergefontfile]+ here
bool EmbeddedFontsManager::mergefonts(const OUString& cidFontInfoUrl, const OUString& destFileUrl,
                                      const std::vector<std::pair<OUString, OUString>>& fonts)
{
    OUString cidFontInfoPath, destFilePath;
    if (osl::FileBase::E_None
            != osl::FileBase::getSystemPathFromFileURL(cidFontInfoUrl, cidFontInfoPath)
        || osl::FileBase::E_None
               != osl::FileBase::getSystemPathFromFileURL(destFileUrl, destFilePath))
    {
        SAL_WARN("vcl.fonts", "path failure");
        return false;
    }

    std::vector<OString> paths;
    for (const auto& font : fonts)
    {
        OUString glyphAliasPath;
        OUString mergeFontFilePath;
        if (osl::FileBase::E_None
                != osl::FileBase::getSystemPathFromFileURL(font.first, glyphAliasPath)
            || osl::FileBase::E_None
                   != osl::FileBase::getSystemPathFromFileURL(font.second, mergeFontFilePath))
        {
            SAL_WARN("vcl.fonts", "path failure");
            return false;
        }
        paths.push_back(glyphAliasPath.toUtf8());
        paths.push_back(mergeFontFilePath.toUtf8());
    }

    txCtx h = mergeFontsNew(nullptr);
    if (!h)
        return false;

    OString cidFontInfoPathA(cidFontInfoPath.toUtf8());
    OString destFilePathA(destFilePath.toUtf8());

    OStringBuffer aBuffer;
    for (const auto& path : paths)
        aBuffer.append(" "_ostr + path);
    SAL_INFO("vcl.fonts",
             "mergefonts -cid " << cidFontInfoPathA << " " << destFilePathA << aBuffer.toString());

    readCIDFontInfo(h, const_cast<char*>(cidFontInfoPathA.getStr()));

    setMode(h, mode_cff);

    dstFileSetName(h, const_cast<char*>(destFilePathA.getStr()));
    h->cfw.flags |= CFW_CHECK_IF_GLYPHS_DIFFER;
    h->cfw.flags |= CFW_PRESERVE_GLYPH_ORDER;

    std::vector<char*> args;
    for (const auto& path : paths)
    {
        args.push_back(const_cast<char*>(path.getStr()));
    }
    // merge the input fonts into destfile
    size_t resultarg = doMergeFileSet(h, args.size(), args.data(), 0);
    SAL_WARN_IF(resultarg != args.size() - 1, "vcl.fonts",
                "suspicious doMergeFileSet result of: " << resultarg);

    // convert that merged cid result to Type 1
    h->src.stm.filename = const_cast<char*>(destFilePathA.getStr());
    OString tmpdestfile = destFilePathA + ".temp";
    h->dst.stm.filename = const_cast<char*>(tmpdestfile.getStr());
    setMode(h, mode_t1);
    bool result = convertTx(h);

    remove(destFilePathA.getStr());
    rename(h->dst.stm.filename, destFilePathA.getStr());

    mergeFontsFree(h);

    return result;
}

static void* cb_memory(ctlMemoryCallbacks* /*cb*/, void* old, size_t size)
{
    if (size == 0)
    {
        free(old);
        return nullptr;
    }

    if (old != nullptr)
        return realloc(old, size);

    return malloc(size);
}

// System afdko could be used by calling: makeotf[exe] -mf fontMenuNameDB -f srcFont -o destFile -ch charMap [-ff features]
bool EmbeddedFontsManager::makeotf(const OUString& srcFontUrl, const OUString& destFileUrl,
                                   const OUString& fontMenuNameDBUrl, const OUString& charMapUrl,
                                   const OUString& featuresUrl)
{
    OUString srcFontPath, destFilePath, charMapPath, fontMenuNameDBPath, featuresPath;
    if (osl::FileBase::E_None != osl::FileBase::getSystemPathFromFileURL(srcFontUrl, srcFontPath)
        || osl::FileBase::E_None
               != osl::FileBase::getSystemPathFromFileURL(destFileUrl, destFilePath)
        || osl::FileBase::E_None
               != osl::FileBase::getSystemPathFromFileURL(fontMenuNameDBUrl, fontMenuNameDBPath))
    {
        SAL_WARN("vcl.fonts", "path failure");
        return false;
    }

    if (!charMapUrl.isEmpty()
        && osl::FileBase::E_None
               != osl::FileBase::getSystemPathFromFileURL(charMapUrl, charMapPath))
    {
        SAL_WARN("vcl.fonts", "path failure");
        return false;
    }

    if (!featuresUrl.isEmpty()
        && osl::FileBase::E_None
               != osl::FileBase::getSystemPathFromFileURL(featuresUrl, featuresPath))
    {
        SAL_WARN("vcl.fonts", "path failure");
        return false;
    }

    ctlMemoryCallbacks cb_dna_memcb{ nullptr, cb_memory };
    dnaCtx mainDnaCtx = dnaNew(&cb_dna_memcb, DNA_CHECK_ARGS);

    cbCtx cbctx = cbNew(nullptr, const_cast<char*>(""), const_cast<char*>(""),
                        const_cast<char*>(""), const_cast<char*>(""), mainDnaCtx);

    OString fontMenuNameDBPathA(fontMenuNameDBPath.toUtf8());
    OString srcFontPathA(srcFontPath.toUtf8());
    OString destFilePathA(destFilePath.toUtf8());
    OString charMapPathA(charMapPath.toUtf8());
    OString featuresPathA(featuresPath.toUtf8());

    SAL_INFO(
        "vcl.fonts", "makeotf -mf "
                         << fontMenuNameDBPathA << " -f " << srcFontPathA << " -o " << destFilePathA
                         << (!charMapPathA.isEmpty() ? " -ch "_ostr + charMapPathA : OString())
                         << (!featuresPathA.isEmpty() ? " -ff "_ostr + featuresPathA : OString()));

    cbFCDBRead(cbctx, const_cast<char*>(fontMenuNameDBPathA.getStr()));

    cbConvert(cbctx, HOT_NO_OLD_OPS, nullptr, const_cast<char*>(srcFontPathA.getStr()),
              const_cast<char*>(destFilePathA.getStr()),
              !featuresPathA.isEmpty() ? const_cast<char*>(featuresPathA.getStr()) : nullptr,
              !charMapPathA.isEmpty() ? const_cast<char*>(charMapPathA.getStr()) : nullptr, nullptr,
              nullptr, nullptr, 0, 0, 0, 0, 0, -1, -1, 0, nullptr);

    return true;
}

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
