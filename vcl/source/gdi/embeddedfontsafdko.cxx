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
#include <config_afdko.h>

#include <osl/file.hxx>
#include <rtl/strbuf.hxx>
#include <vcl/embeddedfontsmanager.hxx>

#if HAVE_FEATURE_AFDKO && !USE_AFDKO_PROGRAMS

#include <memory>
#include <stdexcept>

#include "afdko.hxx"

namespace
{
// Routes afdko messages to the sal log so nothing is written to stderr.
class SalLogger final : public slogger
{
public:
    void msg(int level, const char* message) override
    {
        if (level >= sWARNING)
            SAL_WARN("vcl.fonts", "afdko: " << message);
        else
            SAL_INFO("vcl.fonts", "afdko: " << message);
    }
    int set_context(const char*, int, const char*) override { return log_context_new; }
    int clear_context(const char*) override { return 0; }
};

std::shared_ptr<slogger> createSalLogger(const char* /*name*/)
{
    static std::shared_ptr<slogger> aLogger = std::make_shared<SalLogger>();
    return aLogger;
}

// Both logger channels: getLogger covers the C++ contexts created by
// txNew/cbNew, extc_logger covers the sLog calls from the C sources.
void installSalLogger()
{
    slogger::getLogger = createSalLogger;
    slogger::extc_logger = createSalLogger(nullptr);
}

void txFatalCallback(txCtx h)
{
    txFree(h);
    throw std::runtime_error("fatal tx error");
}

void mergeFontsFatalCallback(txCtx h)
{
    mergeFontsFree(h);
    throw std::runtime_error("fatal mergeFonts error");
}
}
#endif

// System afdko could be used by calling: tx -dump src dest here
bool EmbeddedFontsManager::tx_dump(const OUString& srcFontUrl, const OUString& destFileUrl)
{
#if HAVE_FEATURE_AFDKO
    OUString srcFontPath, destFilePath;
    if (osl::FileBase::E_None != osl::FileBase::getSystemPathFromFileURL(srcFontUrl, srcFontPath)
        || osl::FileBase::E_None
               != osl::FileBase::getSystemPathFromFileURL(destFileUrl, destFilePath))
    {
        SAL_WARN("vcl.fonts", "path failure");
        return false;
    }

    OString srcFontPathA(srcFontPath.toUtf8());
    OString destFilePathA(destFilePath.toUtf8());
#if USE_AFDKO_PROGRAMS
    OString txCommand = TX " -dump " + srcFontPathA + " " + destFilePathA;
    SAL_INFO("vcl.fonts", txCommand);

    return system(txCommand.getStr()) == 0;
#else
    installSalLogger();
    try
    {
        struct txCtx_ aTx = {};
        txNew(&aTx, nullptr);
        aTx.fatalCallback = txFatalCallback;
        dstFileSetName(&aTx, destFilePathA.getStr());
        doSingleFileSet(&aTx, srcFontPathA.getStr());
        txFree(&aTx);
        return true;
    }
    catch (const std::exception& e)
    {
        SAL_WARN("vcl.fonts", "tx failure: " << e.what());
    }
#endif
#else
    (void)srcFontUrl;
    (void)destFileUrl;
#endif
    return false;
}

// System afdko could be used by calling: tx -t1 src dest here
bool EmbeddedFontsManager::tx_t1(const OUString& srcFontUrl, const OUString& destFileUrl)
{
#if HAVE_FEATURE_AFDKO
    OUString srcFontPath, destFilePath;
    if (osl::FileBase::E_None != osl::FileBase::getSystemPathFromFileURL(srcFontUrl, srcFontPath)
        || osl::FileBase::E_None
               != osl::FileBase::getSystemPathFromFileURL(destFileUrl, destFilePath))
    {
        SAL_WARN("vcl.fonts", "path failure");
        return false;
    }

    OString srcFontPathA(srcFontPath.toUtf8());
    OString destFilePathA(destFilePath.toUtf8());
#if USE_AFDKO_PROGRAMS
    OString txCommand = TX " -t1 " + srcFontPathA + " " + destFilePathA;
    SAL_INFO("vcl.fonts", txCommand);
    return system(txCommand.getStr()) == 0;
#else
    installSalLogger();
    try
    {
        struct txCtx_ aTx = {};
        txNew(&aTx, nullptr);
        aTx.fatalCallback = txFatalCallback;
        setMode(&aTx, mode_t1);
        dstFileSetName(&aTx, destFilePathA.getStr());
        doSingleFileSet(&aTx, srcFontPathA.getStr());
        txFree(&aTx);
        return true;
    }
    catch (const std::exception& e)
    {
        SAL_WARN("vcl.fonts", "tx failure: " << e.what());
    }
#endif
#else
    (void)srcFontUrl;
    (void)destFileUrl;
#endif
    return false;
}

// System afdko could be used by calling: mergefonts -cid cidfontinfo destfile [glyphaliasfile mergefontfile]+ here
bool EmbeddedFontsManager::mergefonts(const OUString& cidFontInfoUrl, const OUString& destFileUrl,
                                      const std::vector<std::pair<OUString, OUString>>& fonts)
{
#if HAVE_FEATURE_AFDKO
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

    OString cidFontInfoPathA(cidFontInfoPath.toUtf8());
    OString destFilePathA(destFilePath.toUtf8());

    bool result = false;
    OUString tmpdestfile = destFilePath + ".temp";
    OString tmpdestfileA = tmpdestfile.toUtf8();

#if USE_AFDKO_PROGRAMS
    OStringBuffer aBuffer;
    for (const auto& path : paths)
        aBuffer.append(" "_ostr + path);
    OString mergeFontsCommand
        = MERGEFONTS " -cid  " + cidFontInfoPathA + " " + destFilePathA + aBuffer;
    SAL_INFO("vcl.fonts", mergeFontsCommand);

    if (system(mergeFontsCommand.getStr()) != 0)
        return false;

    // convert that merged cid result to Type 1
    OString txCommand = TX " -t1 " + destFilePathA + " " + tmpdestfileA;
    SAL_INFO("vcl.fonts", txCommand);
    result = system(txCommand.getStr()) == 0;
#else
    installSalLogger();
    txCtx h = mergeFontsNew();
    if (!h)
        return false;
    h->fatalCallback = mergeFontsFatalCallback;

    try
    {
        mergeFontsReadCIDFontInfo(h, cidFontInfoPathA.getStr());

        setMode(h, mode_cff);

        dstFileSetName(h, destFilePathA.getStr());
        h->cfw.flags |= CFW_CHECK_IF_GLYPHS_DIFFER;
        h->cfw.flags |= CFW_PRESERVE_GLYPH_ORDER;

        std::vector<char*> args;
        for (const auto& path : paths)
        {
            args.push_back(const_cast<char*>(path.getStr()));
        }
        // merge the input fonts into destfile
        size_t resultarg = mergeFontsDoMergeFileSet(h, args.size(), args.data(), 0);
        SAL_WARN_IF(resultarg != args.size() - 1, "vcl.fonts",
                    "suspicious doMergeFileSet result of: " << resultarg);

        // convert that merged cid result to Type 1
        setMode(h, mode_t1);
        dstFileSetName(h, tmpdestfileA.getStr());
        doSingleFileSet(h, destFilePathA.getStr());
        mergeFontsFree(h);
        result = true;
    }
    catch (const std::exception& e)
    {
        SAL_WARN("vcl.fonts", "mergeFonts failure: " << e.what());
    }
#endif

    osl::File::remove(destFileUrl);
    osl::FileBase::getFileURLFromSystemPath(tmpdestfile, tmpdestfile);
    osl::File::move(tmpdestfile, destFileUrl);

    return result;
#else
    (void)cidFontInfoUrl;
    (void)destFileUrl;
    (void)fonts;
    return false;
#endif
}

#if HAVE_FEATURE_AFDKO && !USE_AFDKO_PROGRAMS
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

static void makeOtfFatalCallback(void*) { throw std::runtime_error("fatal addfeatures error"); }
#endif

// System afdko could be used by calling: makeotf -mf fontMenuNameDB -f srcFont -o destFile -ch charMap [-ff features]
bool EmbeddedFontsManager::makeotf(const OUString& srcFontUrl, const OUString& destFileUrl,
                                   const OUString& fontMenuNameDBUrl, const OUString& charMapUrl,
                                   const OUString& featuresUrl)
{
    bool ret = false;
#if HAVE_FEATURE_AFDKO
    // cbConvert may abort when dest file already exists
    assert(osl::File(destFileUrl).open(osl_File_OpenFlag_Read) == osl::FileBase::E_NOENT);

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

    OString fontMenuNameDBPathA(fontMenuNameDBPath.toUtf8());
    OString srcFontPathA(srcFontPath.toUtf8());
    OString destFilePathA(destFilePath.toUtf8());
    OString charMapPathA(charMapPath.toUtf8());
    OString featuresPathA(featuresPath.toUtf8());

#if USE_AFDKO_PROGRAMS
    OString makeotfCommand
        = MAKEOTF " -mf " + fontMenuNameDBPathA + " -f " + srcFontPathA + " -o " + destFilePathA;
    if (!charMapPathA.isEmpty())
        makeotfCommand += " -ch "_ostr + charMapPathA;
    if (!featuresPathA.isEmpty())
        makeotfCommand += " -ff "_ostr + featuresPathA;
    SAL_INFO("vcl.fonts", makeotfCommand);

    return system(makeotfCommand.getStr()) == 0;
#else
    installSalLogger();

    // hotconv only reads CFF (or an sfnt wrapping one), so first convert the
    // Type 1 input to bare CFF the same way the makeotf driver script does
    // with: tx -cff -std -F
    OString cffFilePathA = destFilePathA + ".cff";
    try
    {
        struct txCtx_ aTx = {};
        txNew(&aTx, nullptr);
        aTx.fatalCallback = txFatalCallback;
        setMode(&aTx, mode_cff);
        aTx.cfw.flags |= CFW_FORCE_STD_ENCODING;
        aTx.cfw.flags |= CFW_NO_FAMILY_OPT;
        dstFileSetName(&aTx, cffFilePathA.getStr());
        doSingleFileSet(&aTx, srcFontPathA.getStr());
        txFree(&aTx);
    }
    catch (const std::exception& e)
    {
        SAL_WARN("vcl.fonts", "tx failure: " << e.what());
        return false;
    }

    ctlMemoryCallbacks cb_dna_memcb{ nullptr, cb_memory };
    dnaCtx mainDnaCtx = dnaNew(&cb_dna_memcb, DNA_CHECK_ARGS);

    cbCtx cbctx = cbNew("addfeatures", "", "", "", "", mainDnaCtx, makeOtfFatalCallback);

    ret = true;
    try
    {
        cbFCDBRead(cbctx, fontMenuNameDBPathA.getStr());

        // the name table version string asserts on a null client version
        init_fdk_version();
        cbConvert(cbctx, 0, FDK_VERSION, cffFilePathA.getStr(), destFilePathA.getStr(),
                  !featuresPathA.isEmpty() ? featuresPathA.getStr() : nullptr,
                  !charMapPathA.isEmpty() ? charMapPathA.getStr() : nullptr, nullptr, nullptr,
                  nullptr, 0, HOT_CMAP_UNKNOWN, HOT_CMAP_UNKNOWN, 0, -1, -1, 0, nullptr);
    }
    catch (const std::exception& e)
    {
        SAL_WARN("vcl.fonts", "addfeatures failure: " << e.what());
        ret = false;
    }

    cbFree(cbctx);

    OUString cffFileUrl;
    osl::FileBase::getFileURLFromSystemPath(destFilePath + ".cff", cffFileUrl);
    osl::File::remove(cffFileUrl);
#endif
#else
    (void)srcFontUrl;
    (void)destFileUrl;
    (void)fontMenuNameDBUrl;
    (void)charMapUrl;
    (void)featuresUrl;
#endif
    return ret;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab cinoptions=b1,g0,N-s cinkeys+=0=break: */
