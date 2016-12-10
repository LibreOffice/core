/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <sal/main.h>
#include <tools/extendapplicationenvironment.hxx>

#include <cppuhelper/bootstrap.hxx>
#include <comphelper/processfactory.hxx>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <unotools/configmgr.hxx>
#include <rtl/strbuf.hxx>
#include <osl/file.hxx>
#include <vcl/svapp.hxx>
#include <vcl/wmf.hxx>
#include <unistd.h>
#include <stdlib.h>

using namespace ::com::sun::star::uno;
using namespace ::com::sun::star::lang;
using namespace cppu;

namespace
{
    void setFontConfigConf()
    {
        osl::File aFontConfig("file::///tmp/wmffuzzerfonts.conf");
        if (aFontConfig.open(osl_File_OpenFlag_Create | osl_File_OpenFlag_Write) == osl::File::E_None)
        {
            OUString uri;
            if (osl_getExecutableFile(&uri.pData) != osl_Process_E_None) {
                abort();
            }
            sal_Int32 lastDirSeperatorPos = uri.lastIndexOf('/');
            if (lastDirSeperatorPos >= 0) {
                uri = uri.copy(0, lastDirSeperatorPos + 1);
            }
            OUString path;
            osl::FileBase::getSystemPathFromFileURL(uri, path);
            OString sFontDir = OUStringToOString(path, osl_getThreadTextEncoding()) + "../share/fonts/truetype";

            rtl::OStringBuffer aBuffer("<?xml version=\"1.0\"?>\n<fontconfig><dir>");
            aBuffer.append(sFontDir);
            aBuffer.append("</dir><cachedir>/tmp/cache/fontconfig</cachedir></fontconfig>");
            rtl::OString aConf = aBuffer.makeStringAndClear();
            sal_uInt64 aBytesWritten;
            aFontConfig.write(aConf.getStr(), aConf.getLength(), aBytesWritten);
        }
        setenv("FONTCONFIG_FILE", "/tmp/wmffuzzerfonts.conf", 0);
    }
}

extern "C" int LLVMFuzzerInitialize(int *argc, char ***argv)
{
    setenv("SAL_USE_VCLPLUGIN", "svp", 1);

    osl_setCommandArgs(*argc, *argv);

    setFontConfigConf();

    tools::extendApplicationEnvironment();

    Reference< XComponentContext > xContext = defaultBootstrap_InitialComponentContext();
    Reference< XMultiServiceFactory > xServiceManager( xContext->getServiceManager(), UNO_QUERY );
    if( !xServiceManager.is() )
        Application::Abort( "Failed to bootstrap" );
    comphelper::setProcessServiceFactory( xServiceManager );
    utl::ConfigManager::EnableAvoidConfig();
    InitVCL();

    return 0;
}

extern "C" int LLVMFuzzerTestOneInput(const uint8_t* data, size_t size)
{
    SvMemoryStream aStream(const_cast<uint8_t*>(data), size, StreamMode::READ);
    GDIMetaFile aGDIMetaFile;
    (void)ReadWindowMetafile(aStream, aGDIMetaFile);
    return 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
