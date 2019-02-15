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

#include <sal/config.h>

#include <cassert>
#include <vector>

#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/task/XInteractionHandler.hpp>
#include <com/sun/star/task/InteractionHandler.hpp>
#include <com/sun/star/ucb/CommandAbortedException.hpp>
#include <com/sun/star/ucb/ContentInfo.hpp>
#include <com/sun/star/ucb/ContentInfoAttribute.hpp>
#include <com/sun/star/ucb/IOErrorCode.hpp>
#include <com/sun/star/ucb/InteractiveIOException.hpp>
#include <com/sun/star/ucb/NameClashException.hpp>
#include <com/sun/star/ucb/UniversalContentBroker.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/XContentIdentifier.hpp>
#include <com/sun/star/ucb/XProgressHandler.hpp>
#include <com/sun/star/ucb/XUniversalContentBroker.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <com/sun/star/uno/Exception.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/Sequence.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/util/DateTime.hpp>
#include <comphelper/processfactory.hxx>
#include <cppuhelper/exc_hlp.hxx>
#include <comphelper/simplefileaccessinteraction.hxx>
#include <osl/file.hxx>
#include <rtl/string.h>
#include <rtl/ustring.h>
#include <rtl/ustring.hxx>
#include <sal/log.hxx>
#include <sal/types.h>
#include <tools/datetime.hxx>
#include <tools/urlobj.hxx>
#include <tools/diagnose_ex.h>
#include <ucbhelper/commandenvironment.hxx>
#include <ucbhelper/content.hxx>
#include <unotools/ucbhelper.hxx>

namespace {

OUString canonic(OUString const & url) {
    INetURLObject o(url);
    SAL_WARN_IF(o.HasError(), "unotools.ucbhelper", "Invalid URL \"" << url << '"');
    return o.GetMainURL(INetURLObject::DecodeMechanism::NONE);
}

ucbhelper::Content content(OUString const & url) {
    return ucbhelper::Content(
        canonic(url),
        utl::UCBContentHelper::getDefaultCommandEnvironment(),
        comphelper::getProcessComponentContext());
}

ucbhelper::Content content(INetURLObject const & url) {
    return ucbhelper::Content(
        url.GetMainURL(INetURLObject::DecodeMechanism::NONE),
        utl::UCBContentHelper::getDefaultCommandEnvironment(),
        comphelper::getProcessComponentContext());
}

std::vector<OUString> getContents(OUString const & url) {
    try {
        std::vector<OUString> cs;
        ucbhelper::Content c(content(url));
        css::uno::Sequence<OUString> args { "Title" };
        css::uno::Reference<css::sdbc::XResultSet> res( c.createCursor(args), css::uno::UNO_SET_THROW);
        css::uno::Reference<css::ucb::XContentAccess> acc( res, css::uno::UNO_QUERY_THROW);
        while (res->next()) {
            cs.push_back(acc->queryContentIdentifierString());
        }
        return cs;
    } catch (css::uno::RuntimeException const &) {
        throw;
    } catch (css::ucb::CommandAbortedException const &) {
        assert(false && "this cannot happen");
        throw;
    } catch (css::uno::Exception const &) {
        css::uno::Any e(cppu::getCaughtException());
        SAL_INFO(
            "unotools.ucbhelper",
            "getContents(" << url << ") " << exceptionToString(e));
        return std::vector<OUString>();
    }
}

OUString getCasePreservingUrl(const INetURLObject& url) {
    return
        content(url).executeCommand(
            "getCasePreservingURL",
            css::uno::Any()).
        get<OUString>();
}

DateTime convert(css::util::DateTime const & dt) {
    return DateTime(dt);
}

}

css::uno::Reference< css::ucb::XCommandEnvironment > utl::UCBContentHelper::getDefaultCommandEnvironment()
{
    css::uno::Reference< css::task::XInteractionHandler > xIH(
        css::task::InteractionHandler::createWithParent(
            comphelper::getProcessComponentContext(), nullptr ) );

    css::uno::Reference< css::ucb::XProgressHandler > xProgress;
    ucbhelper::CommandEnvironment* pCommandEnv =
        new ::ucbhelper::CommandEnvironment(
            new comphelper::SimpleFileAccessInteraction( xIH ), xProgress );

    css::uno::Reference < css::ucb::XCommandEnvironment > xEnv(
        static_cast< css::ucb::XCommandEnvironment* >(pCommandEnv), css::uno::UNO_QUERY );
    return xEnv;
}

bool utl::UCBContentHelper::IsDocument(OUString const & url) {
    try {
        return content(url).isDocument();
    } catch (css::uno::RuntimeException const &) {
        throw;
    } catch (css::ucb::CommandAbortedException const &) {
        assert(false && "this cannot happen");
        throw;
    } catch (css::uno::Exception const &) {
        css::uno::Any e(cppu::getCaughtException());
        SAL_INFO(
            "unotools.ucbhelper",
            "UCBContentHelper::IsDocument(" << url << ") "
                << exceptionToString(e));
        return false;
    }
}

css::uno::Any utl::UCBContentHelper::GetProperty(
    OUString const & url, OUString const & property)
{
    try {
        return content(url).getPropertyValue(property);
    } catch (css::uno::RuntimeException const &) {
        throw;
    } catch (css::ucb::CommandAbortedException const &) {
        assert(false && "this cannot happen");
        throw;
    } catch (css::uno::Exception const &) {
        css::uno::Any e(cppu::getCaughtException());
        SAL_INFO(
            "unotools.ucbhelper",
            "UCBContentHelper::GetProperty(" << url << ", " << property << ") "
                << exceptionToString(e));
        return css::uno::Any();
    }
}

bool utl::UCBContentHelper::IsFolder(OUString const & url) {
    try {
        return content(url).isFolder();
    } catch (css::uno::RuntimeException const &) {
        throw;
    } catch (css::ucb::CommandAbortedException const &) {
        assert(false && "this cannot happen");
        throw;
    } catch (css::uno::Exception const &) {
        css::uno::Any e(cppu::getCaughtException());
        SAL_INFO(
            "unotools.ucbhelper",
            "UCBContentHelper::IsFolder(" << url << ") "
                << exceptionToString(e));
        return false;
    }
}

bool utl::UCBContentHelper::GetTitle(
    OUString const & url, OUString * title)
{
    assert(title != nullptr);
    try {
        return content(url).getPropertyValue("Title") >>= *title;
    } catch (css::uno::RuntimeException const &) {
        throw;
    } catch (css::ucb::CommandAbortedException const &) {
        assert(false && "this cannot happen");
        throw;
    } catch (css::uno::Exception const &) {
        css::uno::Any e(cppu::getCaughtException());
        SAL_INFO(
            "unotools.ucbhelper",
            "UCBContentHelper::GetTitle(" << url << ") "
                << exceptionToString(e));
        return false;
    }
}

bool utl::UCBContentHelper::Kill(OUString const & url) {
    try {
        content(url).executeCommand(
            "delete",
            css::uno::makeAny(true));
        return true;
    } catch (css::uno::RuntimeException const &) {
        throw;
    } catch (css::ucb::CommandAbortedException const &) {
        assert(false && "this cannot happen");
        throw;
    } catch (css::uno::Exception const &) {
        css::uno::Any e(cppu::getCaughtException());
        SAL_INFO(
            "unotools.ucbhelper",
            "UCBContentHelper::Kill(" << url << ") "
                << exceptionToString(e));
        return false;
    }
}

bool utl::UCBContentHelper::MakeFolder(
    ucbhelper::Content & parent, OUString const & title,
    ucbhelper::Content & result)
{
    bool exists = false;
    try {
        css::uno::Sequence<css::ucb::ContentInfo> info(
            parent.queryCreatableContentsInfo());
        for (sal_Int32 i = 0; i < info.getLength(); ++i) {
            // Simply look for the first KIND_FOLDER:
            if ((info[i].Attributes
                 & css::ucb::ContentInfoAttribute::KIND_FOLDER)
                != 0)
            {
                // Make sure the only required bootstrap property is "Title":
                if ( info[i].Properties.getLength() != 1 || info[i].Properties[0].Name != "Title" )
                {
                    continue;
                }
                css::uno::Sequence<OUString> keys { "Title" };
                css::uno::Sequence<css::uno::Any> values(1);
                values[0] <<= title;
                if (parent.insertNewContent(info[i].Type, keys, values, result))
                {
                    return true;
                }
            }
        }
    } catch (css::ucb::InteractiveIOException const & e) {
        if (e.Code == css::ucb::IOErrorCode_ALREADY_EXISTING) {
            exists = true;
        } else {
            SAL_INFO(
                "unotools.ucbhelper",
                "UCBContentHelper::MakeFolder(" << title
                    << ") InteractiveIOException \"" << e
                    << "\", code " << + static_cast<sal_Int32>(e.Code));
        }
    } catch (css::ucb::NameClashException const &) {
        exists = true;
    } catch (css::uno::RuntimeException const &) {
        throw;
    } catch (css::ucb::CommandAbortedException const &) {
        assert(false && "this cannot happen");
        throw;
    } catch (css::uno::Exception const &) {
        css::uno::Any e(cppu::getCaughtException());
        SAL_INFO(
            "unotools.ucbhelper",
            "UCBContentHelper::MakeFolder(" << title << ") "
                << exceptionToString(e));
    }
    if (exists) {
        INetURLObject o(parent.getURL());
        o.Append(title);
        result = content(o);
        return true;
    } else {
        return false;
    }
}

bool utl::UCBContentHelper::IsYounger(
    OUString const & younger, OUString const & older)
{
    try {
        return
            convert(
                content(younger).getPropertyValue(
                    "DateModified").
                get<css::util::DateTime>())
            > convert(
                content(older).getPropertyValue(
                    "DateModified").
                get<css::util::DateTime>());
    } catch (css::uno::RuntimeException const &) {
        throw;
    } catch (css::ucb::CommandAbortedException const &) {
        assert(false && "this cannot happen");
        throw;
    } catch (css::uno::Exception const &) {
        css::uno::Any e(cppu::getCaughtException());
        SAL_INFO(
            "unotools.ucbhelper",
            "UCBContentHelper::IsYounger(" << younger << ", " << older << ") "
                << exceptionToString(e));
        return false;
    }
}

bool utl::UCBContentHelper::Exists(OUString const & url) {
    OUString pathname;
    if (osl::FileBase::getSystemPathFromFileURL(url, pathname)
        == osl::FileBase::E_None)
    {
        // Try to create a directory entry for the given URL:
        OUString url2;
        if (osl::FileBase::getFileURLFromSystemPath(pathname, url2)
            == osl::FileBase::E_None)
        {
            // #106526 osl_getDirectoryItem is an existence check, no further
            // osl_getFileStatus call necessary:
            osl::DirectoryItem item;
            return osl::DirectoryItem::get(url2, item) == osl::FileBase::E_None;
        } else {
            return false;
        }
    } else {
        // Divide URL into folder and name part:
        INetURLObject o(url);
        OUString name(
            o.getName(
                INetURLObject::LAST_SEGMENT, true,
                INetURLObject::DecodeMechanism::WithCharset));
        o.removeSegment();
        o.removeFinalSlash();
        std::vector<OUString> cs(
            getContents(o.GetMainURL(INetURLObject::DecodeMechanism::NONE)));
        return std::any_of(cs.begin(), cs.end(),
            [&name](const OUString& rItem) {
                return INetURLObject(rItem).
                    getName(INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset).
                        equalsIgnoreAsciiCase(name); });
    }
}

bool utl::UCBContentHelper::IsSubPath(
    OUString const & parent, OUString const & child)
{
    // The comparison is done in the following way:
    // - First, compare case sensitively
    // - If names are different, try a fallback comparing case insensitively
    // - If the last comparison succeeded, get case preserving normalized names
    //   for the files and compare them
    // (The second step is required because retrieving the normalized names
    // might be very expensive in some cases.)
    INetURLObject candidate(child);
    INetURLObject folder(parent);
    if (candidate.GetProtocol() != folder.GetProtocol()) {
        return false;
    }
    INetURLObject candidateLower(child.toAsciiLowerCase());
    INetURLObject folderLower(parent.toAsciiLowerCase());
    try {
        INetURLObject tmp;
        do {
            if (candidate == folder
                || (candidate.GetProtocol() == INetProtocol::File
                    && candidateLower == folderLower
                    && (getCasePreservingUrl(candidate)
                        == getCasePreservingUrl(folder))))
            {
                return true;
            }
            tmp = candidate;
        } while (candidate.removeSegment() && candidateLower.removeSegment()
                 && candidate != tmp);
            // INetURLObject::removeSegment sometimes returns true without
            // modifying the URL, e.g., in case of "file:///"
    } catch (css::uno::RuntimeException const &) {
        throw;
    } catch (css::ucb::CommandAbortedException const &) {
        assert(false && "this cannot happen");
        throw;
    } catch (css::uno::Exception const &) {
        css::uno::Any e(cppu::getCaughtException());
        SAL_INFO(
            "unotools.ucbhelper",
            "UCBContentHelper::IsSubPath(" << parent << ", " << child << ") "
                << exceptionToString(e));
    }
    return false;
}

bool utl::UCBContentHelper::EqualURLs(
    OUString const & url1, OUString const & url2)
{
    if (url1.isEmpty() || url2.isEmpty()) {
        return false;
    }
    css::uno::Reference< css::ucb::XUniversalContentBroker > ucb(
        css::ucb::UniversalContentBroker::create(
            comphelper::getProcessComponentContext()));
    return
        ucb->compareContentIds(
            ucb->createContentIdentifier(canonic(url1)),
            ucb->createContentIdentifier(canonic(url2)))
        == 0;
}

bool utl::UCBContentHelper::ensureFolder(
    const css::uno::Reference< css::uno::XComponentContext >& xCtx,
    const css::uno::Reference< css::ucb::XCommandEnvironment >& xEnv,
    const OUString& rFolder, ucbhelper::Content & result) throw()
{
    try
    {
        INetURLObject aURL( rFolder );
        OUString aTitle = aURL.getName( INetURLObject::LAST_SEGMENT, true, INetURLObject::DecodeMechanism::WithCharset );
        aURL.removeSegment();
        ::ucbhelper::Content aParent;

        if ( ::ucbhelper::Content::create( aURL.GetMainURL( INetURLObject::DecodeMechanism::NONE ),
                                  xEnv, xCtx, aParent ) )
        {
            return ::utl::UCBContentHelper::MakeFolder(aParent, aTitle, result);
        }
    }
    catch (...)
    {
    }

    return false;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
