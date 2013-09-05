/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#include <config_folders.h>

#include "sal/config.h"

#include <boost/shared_ptr.hpp>

#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/lang/Locale.hpp"
#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/packages/zip/ZipFileAccess.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "comphelper/processfactory.hxx"
#include "osl/file.hxx"
#include "osl/diagnose.h"
#include "rtl/bootstrap.hxx"

#include "tools/stream.hxx"
#include "tools/urlobj.hxx"
#include "vcl/bitmapex.hxx"
#include <vcl/dibtools.hxx>
#include "vcl/pngread.hxx"
#include "vcl/settings.hxx"
#include "vcl/svapp.hxx"
#include "impimagetree.hxx"
#include <vcl/dibtools.hxx>

namespace {

static OUString createPath(
    OUString const & name, sal_Int32 pos, OUString const & locale)
{
    return name.copy(0, pos + 1) + locale + name.copy(pos);
}

static boost::shared_ptr< SvStream > wrapFile(osl::File & file)
{
    // This could use SvInputStream instead if that did not have a broken
    // SeekPos implementation for an XInputStream that is not also XSeekable
    // (cf. "@@@" at tags/DEV300_m37/svtools/source/misc1/strmadpt.cxx@264807
    // l. 593):
    boost::shared_ptr< SvStream > s(new SvMemoryStream);
    for (;;) {
        void *data[2048];
        sal_uInt64 n;
        file.read(data, 2048, n);
        s->Write(data, n);
        if (n < 2048) {
            break;
        }
    }
    s->Seek(0);
    return s;
}

static boost::shared_ptr< SvStream > wrapStream(
    css::uno::Reference< css::io::XInputStream > const & stream)
{
    // This could use SvInputStream instead if that did not have a broken
    // SeekPos implementation for an XInputStream that is not also XSeekable
    // (cf. "@@@" at tags/DEV300_m37/svtools/source/misc1/strmadpt.cxx@264807
    // l. 593):
    OSL_ASSERT(stream.is());
    boost::shared_ptr< SvStream > s(new SvMemoryStream);
    for (;;) {
        sal_Int32 const size = 2048;
        css::uno::Sequence< sal_Int8 > data(size);
        sal_Int32 n = stream->readBytes(data, size);
        s->Write(data.getConstArray(), n);
        if (n < size) {
            break;
        }
    }
    s->Seek(0);
    return s;
}

static void loadImageFromStream(
    boost::shared_ptr< SvStream > pStream,
    OUString const & rPath, BitmapEx & rBitmap)
{
    if (rPath.endsWith(".png"))
    {
        vcl::PNGReader aPNGReader( *pStream );
        aPNGReader.SetIgnoreGammaChunk( true );
        rBitmap = aPNGReader.Read();
    } else {
        ReadDIBBitmapEx(rBitmap, *pStream);
    }
}

}

ImplImageTree::ImplImageTree() { m_cacheIcons = true; }

ImplImageTree::~ImplImageTree() {}

bool ImplImageTree::checkStyle(OUString const & style)
{
    bool exists;

    // using cache because setStyle is an expensive operation
    // setStyle calls resetPaths => closes any opened zip files with icons, cleans the icon cache, ...
    if (checkStyleCacheLookup(style, exists)) {
        return exists;
    }

    setStyle(style);

    exists = false;
    OUString aURL = m_path.first;

    osl::File aZip(aURL + ".zip");
    if (aZip.open(osl_File_OpenFlag_Read) == ::osl::FileBase::E_None) {
        aZip.close();
        exists = true;
    }

    osl::Directory aLookaside(aURL);
    if (aLookaside.open() == ::osl::FileBase::E_None) {
        aLookaside.close();
        exists = true;
        m_cacheIcons = false;
    } else {
        m_cacheIcons = true;
    }
    m_checkStyleCache[style] = exists;
    return exists;
}

bool ImplImageTree::loadImage(
    OUString const & name, OUString const & style, BitmapEx & bitmap,
    bool localized, bool loadMissing )
{
    bool found = false;
    try {
        found = doLoadImage(name, style, bitmap, localized);
    } catch (css::uno::RuntimeException &) {
        if (!loadMissing)
            throw;
    }
    if (found || !loadMissing)
        return found;

    SAL_INFO("vcl", "ImplImageTree::loadImage exception couldn't load \""
        << name << "\", fetching default image");

    return loadDefaultImage(style, bitmap);
}

bool ImplImageTree::loadDefaultImage(
    OUString const & style,
    BitmapEx& bitmap)
{
    return doLoadImage(
        OUString("res/grafikde.png"),
        style, bitmap, false);
}


bool ImplImageTree::doLoadImage(
    OUString const & name, OUString const & style, BitmapEx & bitmap,
    bool localized)
{
    setStyle(style);
    if (m_cacheIcons && iconCacheLookup(name, localized, bitmap)) {
        return true;
    }
    if (!bitmap.IsEmpty()) {
        bitmap.SetEmpty();
    }
    std::vector< OUString > paths;
    paths.push_back(getRealImageName(name));
    if (localized) {
        sal_Int32 pos = name.lastIndexOf('/');
        if (pos != -1) {
            // find() uses a reverse iterator, so push in reverse order.
            std::vector< OUString > aFallbacks( Application::GetSettings().GetUILanguageTag().getFallbackStrings( true));
            for (std::vector< OUString >::const_reverse_iterator it( aFallbacks.rbegin());
                    it != aFallbacks.rend(); ++it)
            {
                paths.push_back( getRealImageName( createPath(name, pos, *it) ) );
            }
        }
    }
    bool found = false;
    try {
        found = find(paths, bitmap);
    } catch (css::uno::RuntimeException &) {
        throw;
    } catch (const css::uno::Exception & e) {
        SAL_INFO("vcl", "ImplImageTree::doLoadImage exception " << e.Message);
    }
    if (m_cacheIcons && found) {
        m_iconCache[name.intern()] = std::make_pair(localized, bitmap);
    }
    return found;
}

void ImplImageTree::shutDown() {
    m_style = OUString();
        // for safety; empty m_style means "not initialized"
    m_iconCache.clear();
    m_checkStyleCache.clear();
    m_linkHash.clear();
}

void ImplImageTree::setStyle(OUString const & style) {
    OSL_ASSERT(!style.isEmpty()); // empty m_style means "not initialized"
    if (style != m_style) {
        m_style = style;
        resetPaths();
        m_iconCache.clear();
        m_linkHash.clear();
        loadImageLinks();
    }
}

void ImplImageTree::resetPaths() {
    OUString url( "$BRAND_BASE_DIR/" LIBO_SHARE_FOLDER "/config/" );
    rtl::Bootstrap::expandMacros(url);
    if ( m_style != "default" )
    {
        INetURLObject u(url);
        OSL_ASSERT(!u.HasError());
        bool ok = u.Append("images_" + m_style, INetURLObject::ENCODE_ALL);
        OSL_ASSERT(ok); (void) ok;
        url = u.GetMainURL(INetURLObject::NO_DECODE);
    }
    else
        url += "images";
    m_path = std::make_pair(
        url, css::uno::Reference< css::container::XNameAccess >());
}

bool ImplImageTree::checkStyleCacheLookup(
    OUString const & style, bool &exists)
{
    CheckStyleCache::iterator i(m_checkStyleCache.find(style));
    if (i != m_checkStyleCache.end()) {
        exists = i->second;
        return true;
    } else {
        return false;
    }
}

bool ImplImageTree::iconCacheLookup(
    OUString const & name, bool localized, BitmapEx & bitmap)
{
    IconCache::iterator i(m_iconCache.find(name));
    if (i != m_iconCache.end() && i->second.first == localized) {
        bitmap = i->second.second;
        return true;
    } else {
        return false;
    }
}

bool ImplImageTree::find(
    std::vector< OUString > const & paths, BitmapEx & bitmap)
{
    if (!m_cacheIcons) {
        for (std::vector< OUString >::const_reverse_iterator j(
                 paths.rbegin());
             j != paths.rend(); ++j)
        {
            osl::File file(m_path.first + "/" + *j);
            if (file.open(osl_File_OpenFlag_Read) == ::osl::FileBase::E_None) {
                loadImageFromStream( wrapFile(file), *j, bitmap );
                file.close();
                return true;
            }
        }
    }

    if (!m_path.second.is()) {
        try {
            m_path.second = css::packages::zip::ZipFileAccess::createWithURL(comphelper::getProcessComponentContext(), m_path.first + ".zip");
        } catch (const css::uno::RuntimeException &) {
            throw;
        } catch (const css::uno::Exception & e) {
            SAL_INFO("vcl", "ImplImageTree::find exception "
                << e.Message << " for " << m_path.first);
            return false;
        }
    }
    for (std::vector< OUString >::const_reverse_iterator j(paths.rbegin());
         j != paths.rend(); ++j)
    {
        if (m_path.second->hasByName(*j)) {
            css::uno::Reference< css::io::XInputStream > s;
            bool ok = m_path.second->getByName(*j) >>= s;
            OSL_ASSERT(ok); (void) ok;
            loadImageFromStream( wrapStream(s), *j, bitmap );
            return true;
        }
    }
    return false;
}

void ImplImageTree::loadImageLinks()
{
    const OUString aLinkFilename("links.txt");

    if (!m_cacheIcons)
    {
        osl::File file(m_path.first + "/" + aLinkFilename);
        if (file.open(osl_File_OpenFlag_Read) == ::osl::FileBase::E_None)
        {
            parseLinkFile( wrapFile(file) );
            file.close();
            return;
        }
    }

    if ( !m_path.second.is() )
    {
        try
        {
            m_path.second = css::packages::zip::ZipFileAccess::createWithURL(comphelper::getProcessComponentContext(), m_path.first + ".zip");
        } catch (const css::uno::RuntimeException &) {
            throw;
        }
        catch (const css::uno::Exception & e)
        {
            SAL_INFO("vcl", "ImplImageTree::find exception "
                << e.Message << " for " << m_path.first);
            return;
        }
    }
    if ( m_path.second->hasByName(aLinkFilename) )
    {
        css::uno::Reference< css::io::XInputStream > s;
        bool ok = m_path.second->getByName(aLinkFilename) >>= s;
        OSL_ASSERT(ok); (void) ok;

        parseLinkFile( wrapStream(s) );
        return;
    }
}

void ImplImageTree::parseLinkFile(boost::shared_ptr< SvStream > pStream)
{
    OString aLine;
    OUString aLink, aOriginal;
    while ( pStream->ReadLine( aLine ) )
    {
        sal_Int32 nIndex = 0;
        if ( aLine.isEmpty() )
            continue;
        aLink = OStringToOUString( aLine.getToken(0, ' ', nIndex), RTL_TEXTENCODING_UTF8 );
        aOriginal = OStringToOUString( aLine.getToken(0, ' ', nIndex), RTL_TEXTENCODING_UTF8 );
        if ( aLink.isEmpty() || aOriginal.isEmpty() )
        {
            SAL_INFO("vcl", "ImplImageTree::parseLinkFile: icon links.txt parse error. "
                "Link is incomplete." );
            continue;
        }
        m_linkHash[aLink] = aOriginal;
    }
}

OUString const & ImplImageTree::getRealImageName(OUString const & name)
{
    IconLinkHash::iterator it(m_linkHash.find(name));
    if (it == m_linkHash.end())
        return name;
    return it->second;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
