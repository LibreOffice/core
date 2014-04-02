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

#include <com/sun/star/embed/ElementModes.hpp>
#include <com/sun/star/embed/XTransactedObject.hpp>
#include <com/sun/star/document/XStorageBasedDocument.hpp>
#include <com/sun/star/embed/XStorage.hpp>
#include <osl/file.hxx>
#include <comphelper/processfactory.hxx>
#include <tools/urlobj.hxx>
#include <ucbhelper/content.hxx>

#include <boost/property_tree/ptree.hpp>
#include <boost/property_tree/json_parser.hpp>
#include <boost/foreach.hpp>

#include <string>
#include <vector>

using namespace ::com::sun::star;
using namespace boost::property_tree;

namespace avmedia {

static void lcl_EmbedExternals(const OUString& rSourceURL, uno::Reference<embed::XStorage> xSubStorage, ::ucbhelper::Content& rContent)
{
    // Create a temp file with which json parser can work.
    OUString sTempFileURL;
    const ::osl::FileBase::RC  aErr =
        ::osl::FileBase::createTempFile(0, 0, &sTempFileURL);
    if (::osl::FileBase::E_None != aErr)
    {
        SAL_WARN("avmedia.model", "cannot create temp file");
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
        SAL_WARN("avmedia.model", "exception: '" << e.Message << "'");
        return;
    }

    // Convert URL to a file path for loading
    const INetURLObject aURLObj(sTempFileURL);
    std::string sUrl = OUStringToOString( aURLObj.getFSysPath(INetURLObject::FSYS_DETECT), RTL_TEXTENCODING_UTF8 ).getStr();

    // Parse json, read externals' URI and modify this relative URI's so they remain valid in the new context.
    std::vector<std::string> vExternals;
    ptree aTree;
    try
    {
        json_parser::read_json( sUrl, aTree );

        // Buffers for geometry and animations
        BOOST_FOREACH(ptree::value_type &rVal,aTree.get_child("buffers"))
        {
            const std::string sBufferUri(rVal.second.get<std::string>("path"));
            vExternals.push_back(sBufferUri);
            // Change path: make it contain only a file name
            aTree.put("buffers." + rVal.first + ".path.",sBufferUri.substr(sBufferUri.find_last_of('/')+1));
        }
        // Images for textures
        BOOST_FOREACH(ptree::value_type &rVal,aTree.get_child("images"))
        {
            const std::string sImageUri(rVal.second.get<std::string>("path"));
            vExternals.push_back(sImageUri);
            // Change path: make it contain only a file name
            aTree.put("images." + rVal.first + ".path.",sImageUri.substr(sImageUri.find_last_of('/')+1));
        }
        // Shaders (contains names only)
        BOOST_FOREACH(ptree::value_type &rVal,aTree.get_child("programs"))
        {
            vExternals.push_back(rVal.second.get<std::string>("fragmentShader") + ".glsl");
            vExternals.push_back(rVal.second.get<std::string>("vertexShader") + ".glsl");
        }

        // Write out modified json
        json_parser::write_json( sUrl, aTree );
    }
    catch ( boost::exception const& )
    {
        SAL_WARN("avmedia.model", "failed to parse json file");
        return;
    }

    // Reload json with modified path to external resources
    rContent = ::ucbhelper::Content("file://" + OUString::createFromAscii(sUrl.c_str()),
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
            SAL_WARN("avmedia.model", "openStream to storage failed");
            return;
        }
    }
}

bool Embed3DModel( const uno::Reference<frame::XModel>& xModel,
        const OUString& rSourceURL, OUString& o_rEmbeddedURL)
{
    try
    {
        ::ucbhelper::Content aSourceContent(rSourceURL,
                uno::Reference<ucb::XCommandEnvironment>(),
                comphelper::getProcessComponentContext());

        // Base storage
        uno::Reference<document::XStorageBasedDocument> const xSBD(xModel,
                uno::UNO_QUERY_THROW);
        uno::Reference<embed::XStorage> const xStorage(
                xSBD->getDocumentStorage(), uno::UNO_QUERY_THROW);

        // Model storage
        const OUString sModel("Model");
        uno::Reference<embed::XStorage> const xModelStorage(
            xStorage->openStorageElement(sModel, embed::ElementModes::WRITE));

        // Own storage of the corresponding model
        const OUString sFilename(GetFilename(rSourceURL));
        const OUString sGLTFDir(sFilename.copy(0,sFilename.lastIndexOf('.')));
        uno::Reference<embed::XStorage> const xSubStorage(
            xModelStorage->openStorageElement(sGLTFDir, embed::ElementModes::WRITE));

        // Embed external resources
        lcl_EmbedExternals(rSourceURL, xSubStorage, aSourceContent);

        // Save model file (.json)
        uno::Reference<io::XStream> const xStream(
            CreateStream(xSubStorage, sFilename), uno::UNO_SET_THROW);
        uno::Reference<io::XOutputStream> const xOutStream(
            xStream->getOutputStream(), uno::UNO_SET_THROW);

        if (!aSourceContent.openStream(xOutStream))
        {
            SAL_INFO("avmedia.model", "openStream to storage failed");
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
        SAL_WARN("avmedia.model", "Exception while trying to embed model");
    }
    return false;
}

} // namespace avemdia

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
