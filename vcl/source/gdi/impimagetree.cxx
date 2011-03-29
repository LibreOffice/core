/*************************************************************************
 *
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * Copyright 2000, 2010 Oracle and/or its affiliates.
 *
 * OpenOffice.org - a multi-platform office productivity suite
 *
 * This file is part of OpenOffice.org.
 *
 * OpenOffice.org is free software: you can redistribute it and/or modify
 * it under the terms of the GNU Lesser General Public License version 3
 * only, as published by the Free Software Foundation.
 *
 * OpenOffice.org is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU Lesser General Public License version 3 for more details
 * (a copy is included in the LICENSE file that accompanied this code).
 *
 * You should have received a copy of the GNU Lesser General Public License
 * version 3 along with OpenOffice.org.  If not, see
 * <http://www.openoffice.org/license.html>
 * for a copy of the LGPLv3 License.
 *
 ***********************************************************************/

#include "precompiled_vcl.hxx"

#include "sal/config.h"

#include <list>
#include <memory>
#include <utility>
#include <vector>
#include <hash_map>

#include "com/sun/star/container/XNameAccess.hpp"
#include "com/sun/star/io/XInputStream.hpp"
#include "com/sun/star/lang/Locale.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"

#include "comphelper/processfactory.hxx"

#include "osl/file.hxx"
#include "osl/diagnose.h"

#include "rtl/bootstrap.hxx"
#include "rtl/string.h"
#include "rtl/textenc.h"
#include "rtl/ustrbuf.hxx"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"

#include "sal/types.h"

#include "tools/stream.hxx"
#include "tools/urlobj.hxx"

#include "vcl/bitmapex.hxx"
#include "vcl/pngread.hxx"
#include "vcl/settings.hxx"
#include "vcl/svapp.hxx"

#include "impimagetree.hxx"

namespace {

namespace css = com::sun::star;

rtl::OUString createPath(
    rtl::OUString const & name, sal_Int32 pos, rtl::OUString const & locale)
{
    rtl::OUStringBuffer b(name.copy(0, pos + 1));
    b.append(locale);
    b.append(name.copy(pos));
    return b.makeStringAndClear();
}

std::auto_ptr< SvStream > wrapStream(
    css::uno::Reference< css::io::XInputStream > const & stream)
{
    // This could use SvInputStream instead if that did not have a broken
    // SeekPos implementation for an XInputStream that is not also XSeekable
    // (cf. "@@@" at tags/DEV300_m37/svtools/source/misc1/strmadpt.cxx@264807
    // l. 593):
    OSL_ASSERT(stream.is());
    std::auto_ptr< SvStream > s(new SvMemoryStream);
    for (;;) {
        css::uno::Sequence< sal_Int8 > data;
        sal_Int32 const size = 30000;
        sal_Int32 n = stream->readBytes(data, size);
        s->Write(data.getConstArray(), n);
        if (n < size) {
            break;
        }
    }
    s->Seek(0);
    return s;
}

void loadFromStream(
    css::uno::Reference< css::io::XInputStream > const & stream,
    rtl::OUString const & path, BitmapEx & bitmap)
{
    std::auto_ptr< SvStream > s(wrapStream(stream));
    if (path.endsWithAsciiL(RTL_CONSTASCII_STRINGPARAM(".png")))
    {
        vcl::PNGReader aPNGReader( *s );
        aPNGReader.SetIgnoreGammaChunk( sal_True );
        bitmap = aPNGReader.Read();
    } else {
        *s >> bitmap;
    }
}

}

ImplImageTree::ImplImageTree() {}

ImplImageTree::~ImplImageTree() {}

bool ImplImageTree::checkStyle(rtl::OUString const & style)
{
    bool exists;

    // using cache because setStyle is an expensive operation
    // setStyle calls resetZips => closes any opened zip files with icons, cleans the icon cache, ...
    if (checkStyleCacheLookup(style, exists)) {
        return exists;
    }

    setStyle(style);

    exists = false;
    const rtl::OUString sBrandURLSuffix(RTL_CONSTASCII_USTRINGPARAM("_brand.zip"));
    for (Zips::iterator i(m_zips.begin()); i != m_zips.end() && !exists;) {
        ::rtl::OUString aZipURL = i->first;
        sal_Int32 nFromIndex = aZipURL.getLength() - sBrandURLSuffix.getLength();
        // skip brand-specific icon themes; they are incomplete and thus not useful for this check
        if (nFromIndex < 0 || !aZipURL.match(sBrandURLSuffix, nFromIndex)) {
            osl::File aZip(aZipURL);
            if (aZip.open(OpenFlag_Read) == ::osl::FileBase::E_None) {
                aZip.close();
                exists = true;
            }
        }
        ++i;
    }
    m_checkStyleCache[style] = exists;
    return exists;
}

bool ImplImageTree::loadImage(
    rtl::OUString const & name, rtl::OUString const & style, BitmapEx & bitmap,
    bool localized)
{
    setStyle(style);
    if (iconCacheLookup(name, localized, bitmap)) {
        return true;
    }
    if (!bitmap.IsEmpty()) {
        bitmap.SetEmpty();
    }
    std::vector< rtl::OUString > paths;
    paths.push_back(name);
    if (localized) {
        sal_Int32 pos = name.lastIndexOf('/');
        if (pos != -1) {
            css::lang::Locale const & loc =
                Application::GetSettings().GetUILocale();
            paths.push_back(createPath(name, pos, loc.Language));
            if (loc.Country.getLength() != 0) {
                rtl::OUStringBuffer b(loc.Language);
                b.append(sal_Unicode('-'));
                b.append(loc.Country);
                rtl::OUString p(createPath(name, pos, b.makeStringAndClear()));
                paths.push_back(p);
                if (loc.Variant.getLength() != 0) {
                    b.append(p);
                    b.append(sal_Unicode('-'));
                    b.append(loc.Variant);
                    paths.push_back(
                        createPath(name, pos, b.makeStringAndClear()));
                }
            }
        }
    }
    bool found = false;
    try {
        found = find(paths, bitmap);
    } catch (css::uno::RuntimeException &) {
        throw;
    } catch (css::uno::Exception & e) {
        OSL_TRACE(
            "ImplImageTree::loadImage exception \"%s\"",
            rtl::OUStringToOString(e.Message, RTL_TEXTENCODING_UTF8).getStr());
    }
    if (found) {
        m_iconCache[name.intern()] = std::make_pair(localized, bitmap);
    }
    return found;
}

void ImplImageTree::shutDown() {
    m_style = rtl::OUString();
        // for safety; empty m_style means "not initialized"
    m_zips.clear();
    m_iconCache.clear();
    m_checkStyleCache.clear();
}

void ImplImageTree::setStyle(rtl::OUString const & style) {
    OSL_ASSERT(style.getLength() != 0); // empty m_style means "not initialized"
    if (style != m_style) {
        m_style = style;
        resetZips();
        m_iconCache.clear();
    }
}

void ImplImageTree::resetZips() {
    m_zips.clear();
    {
        rtl::OUString url(
            RTL_CONSTASCII_USTRINGPARAM("$BRAND_BASE_DIR/program/edition/images.zip"));
        rtl::Bootstrap::expandMacros(url);
        INetURLObject u(url);
        OSL_ASSERT(!u.HasError());
        m_zips.push_back(
            std::make_pair(
                u.GetMainURL(INetURLObject::NO_DECODE),
                css::uno::Reference< css::container::XNameAccess >()));
    }
    {
        rtl::OUString url(
            RTL_CONSTASCII_USTRINGPARAM("$BRAND_BASE_DIR/share/config"));
        rtl::Bootstrap::expandMacros(url);
        INetURLObject u(url);
        OSL_ASSERT(!u.HasError());
        rtl::OUStringBuffer b;
        b.appendAscii(RTL_CONSTASCII_STRINGPARAM("images_"));
        b.append(m_style);
        b.appendAscii(RTL_CONSTASCII_STRINGPARAM("_brand.zip"));
        bool ok = u.Append(b.makeStringAndClear(), INetURLObject::ENCODE_ALL);
        OSL_ASSERT(ok); (void) ok;
        m_zips.push_back(
            std::make_pair(
                u.GetMainURL(INetURLObject::NO_DECODE),
                css::uno::Reference< css::container::XNameAccess >()));
    }
    {
        rtl::OUString url(
            RTL_CONSTASCII_USTRINGPARAM(
                "$BRAND_BASE_DIR/share/config/images_brand.zip"));
        rtl::Bootstrap::expandMacros(url);
        m_zips.push_back(
            std::make_pair(
                url, css::uno::Reference< css::container::XNameAccess >()));
    }
    {
        rtl::OUString url(
            RTL_CONSTASCII_USTRINGPARAM("$OOO_BASE_DIR/share/config"));
        rtl::Bootstrap::expandMacros(url);
        INetURLObject u(url);
        OSL_ASSERT(!u.HasError());
        rtl::OUStringBuffer b;
        b.appendAscii(RTL_CONSTASCII_STRINGPARAM("images_"));
        b.append(m_style);
        b.appendAscii(RTL_CONSTASCII_STRINGPARAM(".zip"));
        bool ok = u.Append(b.makeStringAndClear(), INetURLObject::ENCODE_ALL);
        OSL_ASSERT(ok); (void) ok;
        m_zips.push_back(
            std::make_pair(
                u.GetMainURL(INetURLObject::NO_DECODE),
                css::uno::Reference< css::container::XNameAccess >()));
    }
    if ( m_style.equals(::rtl::OUString::createFromAscii("default")) )
    {
        rtl::OUString url(
            RTL_CONSTASCII_USTRINGPARAM(
                "$OOO_BASE_DIR/share/config/images.zip"));
        rtl::Bootstrap::expandMacros(url);
        m_zips.push_back(
            std::make_pair(
                url, css::uno::Reference< css::container::XNameAccess >()));
    }
}

bool ImplImageTree::checkStyleCacheLookup(
    rtl::OUString const & style, bool &exists)
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
    rtl::OUString const & name, bool localized, BitmapEx & bitmap)
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
    std::vector< rtl::OUString > const & paths, BitmapEx & bitmap)
{
    for (Zips::iterator i(m_zips.begin()); i != m_zips.end();) {
        if (!i->second.is()) {
            css::uno::Sequence< css::uno::Any > args(1);
            args[0] <<= i->first;
            try {
                i->second.set(
                    comphelper::createProcessComponentWithArguments(
                        rtl::OUString(
                            RTL_CONSTASCII_USTRINGPARAM(
                                "com.sun.star.packages.zip.ZipFileAccess")),
                        args),
                    css::uno::UNO_QUERY_THROW);
            } catch (css::uno::RuntimeException &) {
                throw;
            } catch (css::uno::Exception & e) {
                OSL_TRACE(
                    "ImplImageTree::find exception \"%s\"",
                    rtl::OUStringToOString(
                        e.Message, RTL_TEXTENCODING_UTF8).getStr());
                i = m_zips.erase(i);
                continue;
            }
        }
        for (std::vector< rtl::OUString >::const_reverse_iterator j(
                 paths.rbegin());
             j != paths.rend(); ++j)
        {
            if (i->second->hasByName(*j)) {
                css::uno::Reference< css::io::XInputStream > s;
                bool ok = i->second->getByName(*j) >>= s;
                OSL_ASSERT(ok); (void) ok;
                loadFromStream(s, *j, bitmap);
                return true;
            }
        }
        ++i;
    }
    return false;
}
