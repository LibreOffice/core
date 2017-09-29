/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <avmedia/modeltools.hxx>
#include <avmedia/mediaitem.hxx>
#include "mediamisc.hxx"

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <com/sun/star/packages/zip/ZipFileAccess.hpp>
#include <osl/file.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/tempfile.hxx>
#include <unotools/ucbstreamhelper.hxx>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/optional.hpp>
#include <boost/exception/diagnostic_information.hpp>

#include <config_features.h>

#if HAVE_FEATURE_COLLADA
#include <collada_headers.hxx>
#include <GLTFAsset.h>
#endif

#include <string>
#include <vector>

using namespace ::com::sun::star;
using namespace boost::property_tree;

namespace avmedia {

#if HAVE_FEATURE_COLLADA

static void lcl_UnzipKmz(const OUString& rSourceURL, const OUString& rOutputFolderURL, OUString& o_rDaeFileURL)
{
    o_rDaeFileURL.clear();
    uno::Reference<packages::zip::XZipFileAccess2> xNameAccess =
        packages::zip::ZipFileAccess::createWithURL(comphelper::getProcessComponentContext(), rSourceURL);
    uno::Sequence< OUString > aNames = xNameAccess->getElementNames();
    for( sal_Int32 i = 0; i < aNames.getLength(); ++i )
    {
        const OUString sCopy = rOutputFolderURL + "/" + aNames[i];
        if( aNames[i].endsWithIgnoreAsciiCase(".dae") )
            o_rDaeFileURL = sCopy;

        uno::Reference<io::XInputStream> xInputStream(xNameAccess->getByName(aNames[i]), uno::UNO_QUERY);

        ::ucbhelper::Content aCopyContent(sCopy,
            uno::Reference<ucb::XCommandEnvironment>(),
            comphelper::getProcessComponentContext());

        aCopyContent.writeStream(xInputStream, true);
    }
}


bool KmzDae2Gltf(const OUString& rSourceURL, OUString& o_rOutput)
{
    o_rOutput.clear();
    const bool bIsDAE = rSourceURL.endsWithIgnoreAsciiCase(".dae");
    const bool bIsKMZ = rSourceURL.endsWithIgnoreAsciiCase(".kmz");
    if( !bIsDAE && !bIsKMZ )
    {
        SAL_WARN("avmedia.opengl", "KmzDae2Gltf converter got a file with wrong extension " << rSourceURL);
        return false;
    }

    // Create a temporary folder for conversion
    OUString sOutput;
    osl::FileBase::getFileURLFromSystemPath(::utl::TempFile::CreateTempName(), sOutput);
    // remove .tmp extension
    sOutput = sOutput.copy(0, sOutput.getLength()-4);

    std::shared_ptr <GLTF::GLTFAsset> asset(new GLTF::GLTFAsset());
    asset->setBundleOutputPath(OUStringToOString( sOutput, RTL_TEXTENCODING_UTF8 ).getStr());

    // If *.dae file is not in the local file system, then copy it to a temp folder for the conversion
    OUString sInput = rSourceURL;
    const INetURLObject aSourceURLObj(rSourceURL);
    if( aSourceURLObj.GetProtocol() != INetProtocol::File )
    {
        try
        {
            ::ucbhelper::Content aSourceContent(rSourceURL,
                uno::Reference<ucb::XCommandEnvironment>(),
                comphelper::getProcessComponentContext());

            const OUString sTarget = sOutput + "/" + GetFilename(rSourceURL);
            ::ucbhelper::Content aTempContent(sTarget,
                uno::Reference<ucb::XCommandEnvironment>(),
                comphelper::getProcessComponentContext());

            aTempContent.writeStream(aSourceContent.openStream(), true);
            sInput = sTarget;
        }
        catch (const uno::Exception&)
        {
            SAL_WARN("avmedia.opengl", "Exception while trying to copy source file to the temp folder for conversion: " << sInput);
            return false;
        }
    }

    asset->setInputFilePath(OUStringToOString( sInput, RTL_TEXTENCODING_UTF8 ).getStr());

    if (bIsKMZ)
    {
        OUString sDaeFilePath;
        lcl_UnzipKmz(sInput, sOutput, sDaeFilePath);
        if ( sDaeFilePath.isEmpty() )
        {
            SAL_WARN("avmedia.opengl", "Cannot find the file in kmz: " << rSourceURL);
            return false;
        }

        asset->setInputFilePath(OUStringToOString( sDaeFilePath, RTL_TEXTENCODING_UTF8 ).getStr());
    }

    GLTF::COLLADA2GLTFWriter writer(asset);
    writer.write();
    // Path to the .json file created by COLLADA2GLTFWriter
    o_rOutput = sOutput + "/" + GetFilename(sOutput) + ".json";
    return true;
}
#endif // HAVE_FEATURE_COLLADA


static void lcl_EmbedExternals(const OUString& rSourceURL, const uno::Reference<embed::XStorage>& xSubStorage, ::ucbhelper::Content& rContent)
{
    // Create a temp file with which json parser can work.
    OUString sTempFileURL;
    const ::osl::FileBase::RC  aErr =
        ::osl::FileBase::createTempFile(nullptr, nullptr, &sTempFileURL);
    if (aErr != ::osl::FileBase::E_None)
    {
        SAL_WARN("avmedia.opengl", "Cannot create temp file");
        return;
    }
    try
    {
        // Write json content to the temp file
        ::ucbhelper::Content aTempContent(sTempFileURL,
            uno::Reference<ucb::XCommandEnvironment>(),
            comphelper::getProcessComponentContext());
        aTempContent.writeStream(rContent.openStream(), true);
    }
    catch (uno::Exception const& e)
    {
        SAL_WARN("avmedia.opengl", e);
        return;
    }

    // Convert URL to a file path for loading
    const INetURLObject aURLObj(sTempFileURL);
    std::string sUrl = OUStringToOString( aURLObj.getFSysPath(FSysStyle::Detect), RTL_TEXTENCODING_UTF8 ).getStr();

    // Parse json, read externals' URI and modify this relative URI's so they remain valid in the new context.
    std::vector<std::string> vExternals;
    ptree aTree;
    try
    {
        json_parser::read_json( sUrl, aTree );

        // Buffers for geometry and animations
        for( ptree::value_type &rVal : aTree.get_child("buffers") )
        {
            const std::string sBufferUri(rVal.second.get<std::string>("path"));
            vExternals.push_back(sBufferUri);
            // Change path: make it contain only a file name
            aTree.put("buffers." + rVal.first + ".path.",sBufferUri.substr(sBufferUri.find_last_of('/')+1));
        }
        // Images for textures
        boost::optional< ptree& > aImages = aTree.get_child_optional("images");
        if( aImages )
        {
            for( ptree::value_type &rVal : aImages.get() )
            {
                const std::string sImageUri(rVal.second.get<std::string>("path"));
                if( !sImageUri.empty() )
                {
                    vExternals.push_back(sImageUri);
                    // Change path: make it contain only a file name
                    aTree.put("images." + rVal.first + ".path.",sImageUri.substr(sImageUri.find_last_of('/')+1));
                }
            }
        }
        // Shaders (contains names only)
        for( ptree::value_type &rVal : aTree.get_child("programs") )
        {
            vExternals.push_back(rVal.second.get<std::string>("fragmentShader") + ".glsl");
            vExternals.push_back(rVal.second.get<std::string>("vertexShader") + ".glsl");
        }

        // Write out modified json
        json_parser::write_json( sUrl, aTree );
    }
    catch ( boost::exception const& e )
    {
        SAL_WARN("avmedia.opengl", "Exception while parsing *.json file " << boost::diagnostic_information(e));
        return;
    }

    // Reload json with modified path to external resources
    rContent = ::ucbhelper::Content(sTempFileURL,
        uno::Reference<ucb::XCommandEnvironment>(),
        comphelper::getProcessComponentContext());

    // Store all external files next to the json file
    for( std::vector<std::string>::iterator aCIter = vExternals.begin(); aCIter != vExternals.end(); ++aCIter )
    {
        const OUString sAbsURL = INetURLObject::GetAbsURL(rSourceURL,OUString::createFromAscii(aCIter->c_str()));

        ::ucbhelper::Content aContent(sAbsURL,
                uno::Reference<ucb::XCommandEnvironment>(),
                comphelper::getProcessComponentContext());

        uno::Reference<io::XStream> const xStream(
            CreateStream(xSubStorage, GetFilename(sAbsURL)), uno::UNO_SET_THROW);
        uno::Reference<io::XOutputStream> const xOutStream(
            xStream->getOutputStream(), uno::UNO_SET_THROW);

        if (!aContent.openStream(xOutStream))
        {
            SAL_WARN("avmedia.opengl", "openStream to storage failed");
            return;
        }
    }
}


bool Embed3DModel( const uno::Reference<frame::XModel>& xModel,
        const OUString& rSourceURL, OUString& o_rEmbeddedURL)
{
    OUString sSource = rSourceURL;

#if HAVE_FEATURE_COLLADA
    if( !rSourceURL.endsWithIgnoreAsciiCase(".json") )
        KmzDae2Gltf(rSourceURL, sSource);
#endif

    try
    {
        ::ucbhelper::Content aSourceContent(sSource,
                uno::Reference<ucb::XCommandEnvironment>(),
                comphelper::getProcessComponentContext());

        // Base storage
        uno::Reference<document::XStorageBasedDocument> const xSBD(xModel,
                uno::UNO_QUERY_THROW);
        uno::Reference<embed::XStorage> const xStorage(
                xSBD->getDocumentStorage(), uno::UNO_QUERY_THROW);

        // Model storage
        const OUString sModel("Models");
        uno::Reference<embed::XStorage> const xModelStorage(
            xStorage->openStorageElement(sModel, embed::ElementModes::WRITE));

        // Own storage of the corresponding model
        const OUString sFilename(GetFilename(sSource));
        const OUString sGLTFDir(sFilename.copy(0,sFilename.lastIndexOf('.')));
        uno::Reference<embed::XStorage> const xSubStorage(
            xModelStorage->openStorageElement(sGLTFDir, embed::ElementModes::WRITE));

        // Embed external resources
        lcl_EmbedExternals(sSource, xSubStorage, aSourceContent);

        // Save model file (.json)
        uno::Reference<io::XStream> const xStream(
            CreateStream(xSubStorage, sFilename), uno::UNO_SET_THROW);
        uno::Reference<io::XOutputStream> const xOutStream(
            xStream->getOutputStream(), uno::UNO_SET_THROW);

        if (!aSourceContent.openStream(xOutStream))
        {
            SAL_WARN("avmedia.opengl", "openStream to storage failed");
            return false;
        }

        const uno::Reference<embed::XTransactedObject> xSubTransaction(xSubStorage, uno::UNO_QUERY);
        if (xSubTransaction.is())
        {
            xSubTransaction->commit();
        }
        const uno::Reference<embed::XTransactedObject> xModelTransaction(xModelStorage, uno::UNO_QUERY);
        if (xModelTransaction.is())
        {
            xModelTransaction->commit();
        }
        const uno::Reference<embed::XTransactedObject> xTransaction(xStorage, uno::UNO_QUERY);
        if (xTransaction.is())
        {
            xTransaction->commit();
        }

        o_rEmbeddedURL = "vnd.sun.star.Package:" + sModel + "/" + sGLTFDir + "/" + sFilename;
        return true;
    }
    catch (uno::Exception const&)
    {
        SAL_WARN("avmedia.opengl", "Exception while trying to embed model");
    }
    return false;
}


bool IsModel(const OUString& rMimeType)
{
    return rMimeType == AVMEDIA_MIMETYPE_JSON;
}

} // namespace avmedia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
