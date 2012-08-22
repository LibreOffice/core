/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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
 ************************************************************************/

#include "sal/config.h"

#include <cassert>
#include <vector>

#include "com/sun/star/lang/XMultiServiceFactory.hpp"
#include "com/sun/star/sdbc/XResultSet.hpp"
#include "com/sun/star/task/XInteractionHandler.hpp"
#include "com/sun/star/ucb/CommandAbortedException.hpp"
#include "com/sun/star/ucb/ContentInfo.hpp"
#include "com/sun/star/ucb/ContentInfoAttribute.hpp"
#include "com/sun/star/ucb/IOErrorCode.hpp"
#include "com/sun/star/ucb/InteractiveIOException.hpp"
#include "com/sun/star/ucb/NameClashException.hpp"
#include "com/sun/star/ucb/XCommandEnvironment.hpp"
#include "com/sun/star/ucb/XContentAccess.hpp"
#include "com/sun/star/ucb/XContentIdentifier.hpp"
#include "com/sun/star/ucb/XContentIdentifierFactory.hpp"
#include "com/sun/star/ucb/XProgressHandler.hpp"
#include "com/sun/star/uno/Any.hxx"
#include "com/sun/star/uno/Exception.hpp"
#include "com/sun/star/uno/Reference.hxx"
#include "com/sun/star/uno/RuntimeException.hpp"
#include "com/sun/star/uno/Sequence.hxx"
#include "com/sun/star/util/DateTime.hpp"
#include "comphelper/processfactory.hxx"
#include "cppuhelper/exc_hlp.hxx"
#include "osl/file.hxx"
#include "rtl/oustringostreaminserter.hxx"
#include "rtl/string.h"
#include "rtl/ustring.h"
#include "rtl/ustring.hxx"
#include "sal/log.hxx"
#include "sal/types.h"
#include "tools/datetime.hxx"
#include "tools/urlobj.hxx"
#include "ucbhelper/commandenvironment.hxx"
#include "ucbhelper/content.hxx"
#include "ucbhelper/contentbroker.hxx"
#include "unotools/localfilehelper.hxx"
#include "unotools/ucbhelper.hxx"

namespace {

namespace css = com::sun::star;

rtl::OUString canonic(rtl::OUString const & url) {
    INetURLObject o(url);
    SAL_WARN_IF(o.HasError(), "unotools", "Invalid URL \"" << url << '"');
    return o.GetMainURL(INetURLObject::NO_DECODE);
}

ucbhelper::Content content(rtl::OUString const & url) {
    return ucbhelper::Content(
        canonic(url),
        css::uno::Reference<css::ucb::XCommandEnvironment>());
}

ucbhelper::Content content(INetURLObject const & url) {
    return ucbhelper::Content(
        url.GetMainURL(INetURLObject::NO_DECODE),
        css::uno::Reference<css::ucb::XCommandEnvironment>());
}

std::vector<rtl::OUString> getContents(rtl::OUString const & url) {
    try {
        std::vector<rtl::OUString> cs;
        ucbhelper::Content c(content(url));
        css::uno::Sequence<rtl::OUString> args(1);
        args[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Title"));
        css::uno::Reference<css::sdbc::XResultSet> res(
            c.createCursor(args, ucbhelper::INCLUDE_FOLDERS_AND_DOCUMENTS),
            css::uno::UNO_SET_THROW);
        css::uno::Reference<com::sun::star::ucb::XContentAccess> acc(
            res, css::uno::UNO_QUERY_THROW);
        while (res->next()) {
            cs.push_back(acc->queryContentIdentifierString());
        }
        return cs;
    } catch (css::uno::RuntimeException const &) {
        throw;
    } catch (css::ucb::CommandAbortedException const &) {
        assert(false); // this cannot happen
        throw;
    } catch (css::uno::Exception const &) {
        css::uno::Any e(cppu::getCaughtException());
        SAL_INFO(
            "unotools",
            "getContents(" << url << ") " << e.getValueType().getTypeName()
                << " \"" << e.get<css::uno::Exception>().Message << '"');
        return std::vector<rtl::OUString>();
    }
}

rtl::OUString getCasePreservingUrl(INetURLObject url) {
    return
        content(url).executeCommand(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("getCasePreservingURL")),
            css::uno::Any()).
        get<rtl::OUString>();
}

DateTime convert(css::util::DateTime const & dt) {
    return DateTime(
        Date(dt.Day, dt.Month, dt.Year),
        Time(dt.Hours, dt.Minutes, dt.Seconds, dt.HundredthSeconds));
}

}

bool utl::UCBContentHelper::IsDocument(rtl::OUString const & url) {
    try {
        return content(url).isDocument();
    } catch (css::uno::RuntimeException const &) {
        throw;
    } catch (css::ucb::CommandAbortedException const &) {
        assert(false); // this cannot happen
        throw;
    } catch (css::uno::Exception const &) {
        css::uno::Any e(cppu::getCaughtException());
        SAL_INFO(
            "unotools",
            "UCBContentHelper::IsDocument(" << url << ") "
                << e.getValueType().getTypeName() << " \""
                << e.get<css::uno::Exception>().Message << '"');
        return false;
    }
}

css::uno::Any utl::UCBContentHelper::GetProperty(
    rtl::OUString const & url, rtl::OUString const & property)
{
    try {
        return content(url).getPropertyValue(property);
    } catch (css::uno::RuntimeException const &) {
        throw;
    } catch (css::ucb::CommandAbortedException const &) {
        assert(false); // this cannot happen
        throw;
    } catch (css::uno::Exception const &) {
        css::uno::Any e(cppu::getCaughtException());
        SAL_INFO(
            "unotools",
            "UCBContentHelper::GetProperty(" << url << ", " << property << ") "
                << e.getValueType().getTypeName() << " \""
                << e.get<css::uno::Exception>().Message << '"');
        return css::uno::Any();
    }
}

bool utl::UCBContentHelper::IsFolder(rtl::OUString const & url) {
    try {
        return content(url).isFolder();
    } catch (css::uno::RuntimeException const &) {
        throw;
    } catch (css::ucb::CommandAbortedException const &) {
        assert(false); // this cannot happen
        throw;
    } catch (css::uno::Exception const &) {
        css::uno::Any e(cppu::getCaughtException());
        SAL_INFO(
            "unotools",
            "UCBContentHelper::IsFolder(" << url << ") "
                << e.getValueType().getTypeName() << " \""
                << e.get<css::uno::Exception>().Message << '"');
        return false;
    }
}

bool utl::UCBContentHelper::GetTitle(
    rtl::OUString const & url, rtl::OUString * title)
{
    assert(title != 0);
    try {
        return
            content(url).getPropertyValue(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Title")))
            >>= *title;
    } catch (css::uno::RuntimeException const &) {
        throw;
    } catch (css::ucb::CommandAbortedException const &) {
        assert(false); // this cannot happen
        throw;
    } catch (css::uno::Exception const &) {
        css::uno::Any e(cppu::getCaughtException());
        SAL_INFO(
            "unotools",
            "UCBContentHelper::GetTitle(" << url << ") "
                << e.getValueType().getTypeName() << " \""
                << e.get<css::uno::Exception>().Message << '"');
        return false;
    }
}

bool utl::UCBContentHelper::Kill(rtl::OUString const & url) {
    try {
        content(url).executeCommand(
            rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("delete")),
            css::uno::makeAny(true));
        return true;
    } catch (css::uno::RuntimeException const &) {
        throw;
    } catch (css::ucb::CommandAbortedException const &) {
        assert(false); // this cannot happen
        throw;
    } catch (css::uno::Exception const &) {
        css::uno::Any e(cppu::getCaughtException());
        SAL_INFO(
            "unotools",
            "UCBContentHelper::Kill(" << url << ") "
                << e.getValueType().getTypeName() << " \""
                << e.get<css::uno::Exception>().Message << '"');
        return false;
    }
}

bool utl::UCBContentHelper::MakeFolder(
    rtl::OUString const & url, bool exclusive)
{
    INetURLObject o(url);
    SAL_WARN_IF(o.HasError(), "unotools", "Invalid URL \"" << url << '"');
    rtl::OUString title(
        o.getName(
            INetURLObject::LAST_SEGMENT, true,
            INetURLObject::DECODE_WITH_CHARSET));
    o.removeSegment();
    ucbhelper::Content parent;
    ucbhelper::Content res;
    return
        ucbhelper::Content::create(
            o.GetMainURL(INetURLObject::NO_DECODE),
            new ucbhelper::CommandEnvironment(
                css::uno::Reference<css::task::XInteractionHandler>(
                    (css::uno::Reference<css::lang::XMultiServiceFactory>(
                        comphelper::getProcessServiceFactory(),
                        css::uno::UNO_SET_THROW)->
                     createInstance(
                         rtl::OUString(
                             RTL_CONSTASCII_USTRINGPARAM(
                                 "com.sun.star.uui.InteractionHandler")))),
                    css::uno::UNO_QUERY_THROW),
                css::uno::Reference<css::ucb::XProgressHandler>()),
            parent)
        && MakeFolder(parent, title, res, exclusive);
}

bool utl::UCBContentHelper::MakeFolder(
    ucbhelper::Content & parent, rtl::OUString const & title,
    ucbhelper::Content & result, bool exclusive)
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
                if (info[i].Properties.getLength() != 1
                    || !info[i].Properties[0].Name.equalsAsciiL(
                        RTL_CONSTASCII_STRINGPARAM("Title")))
                {
                    continue;
                }
                css::uno::Sequence<rtl::OUString> keys(1);
                keys[0] = rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Title"));
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
                "unotools",
                "UCBContentHelper::MakeFolder(" << title
                    << ") InteractiveIOException \"" << e.Message
                    << "\", code " << +e.Code);
        }
    } catch (css::ucb::NameClashException const &) {
        exists = true;
    } catch (css::uno::RuntimeException const &) {
        throw;
    } catch (css::ucb::CommandAbortedException const &) {
        assert(false); // this cannot happen
        throw;
    } catch (css::uno::Exception const &) {
        css::uno::Any e(cppu::getCaughtException());
        SAL_INFO(
            "unotools",
            "UCBContentHelper::MakeFolder(" << title << ") "
                << e.getValueType().getTypeName() << " \""
                << e.get<css::uno::Exception>().Message << '"');
    }
    if (exists && !exclusive) {
        INetURLObject o(parent.getURL());
        o.Append(title);
        result = content(o);
        return true;
    } else {
        return false;
    }
}

sal_Int64 utl::UCBContentHelper::GetSize(rtl::OUString const & url) {
    try {
        sal_Int64 n = 0;
        bool ok =
            (content(url).getPropertyValue(
                rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("Size")))
             >>= n);
        SAL_INFO_IF(
            !ok, "unotools",
            "UCBContentHelper::GetSize(" << url
                << "): Size cannot be determined");
        return n;
    } catch (css::uno::RuntimeException const &) {
        throw;
    } catch (css::ucb::CommandAbortedException const &) {
        assert(false); // this cannot happen
        throw;
    } catch (css::uno::Exception const &) {
        css::uno::Any e(cppu::getCaughtException());
        SAL_INFO(
            "unotools",
            "UCBContentHelper::GetSize(" << url << ") "
                << e.getValueType().getTypeName() << " \""
                << e.get<css::uno::Exception>().Message << '"');
        return 0;
    }
}

bool utl::UCBContentHelper::IsYounger(
    rtl::OUString const & younger, rtl::OUString const & older)
{
    try {
        return
            convert(
                content(younger).getPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DateModified"))).
                get<css::util::DateTime>())
            > convert(
                content(older).getPropertyValue(
                    rtl::OUString(RTL_CONSTASCII_USTRINGPARAM("DateModified"))).
                get<css::util::DateTime>());
    } catch (css::uno::RuntimeException const &) {
        throw;
    } catch (css::ucb::CommandAbortedException const &) {
        assert(false); // this cannot happen
        throw;
    } catch (css::uno::Exception const &) {
        css::uno::Any e(cppu::getCaughtException());
        SAL_INFO(
            "unotools",
            "UCBContentHelper::IsYounger(" << younger << ", " << older << ") "
                << e.getValueType().getTypeName() << " \""
                << e.get<css::uno::Exception>().Message << '"');
        return false;
    }
}

bool utl::UCBContentHelper::Exists(rtl::OUString const & url) {
    rtl::OUString pathname;
    if (utl::LocalFileHelper::ConvertURLToPhysicalName(url, pathname)) {
        // Try to create a directory entry for the given URL:
        rtl::OUString url2;
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
        rtl::OUString name(
            o.getName(
                INetURLObject::LAST_SEGMENT, true,
                INetURLObject::DECODE_WITH_CHARSET));
        o.removeSegment();
        o.removeFinalSlash();
        std::vector<rtl::OUString> cs(
            getContents(o.GetMainURL(INetURLObject::NO_DECODE)));
        for (std::vector<rtl::OUString>::iterator i(cs.begin()); i != cs.end();
             ++i)
        {
            if (INetURLObject(*i).getName(
                    INetURLObject::LAST_SEGMENT, true,
                    INetURLObject::DECODE_WITH_CHARSET).
                equalsIgnoreAsciiCase(name))
            {
                return true;
            }
        }
        return false;
    }
}

bool utl::UCBContentHelper::IsSubPath(
    rtl::OUString const & parent, rtl::OUString const & child)
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
                || (candidate.GetProtocol() == INET_PROT_FILE
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
        assert(false); // this cannot happen
        throw;
    } catch (css::uno::Exception const &) {
        css::uno::Any e(cppu::getCaughtException());
        SAL_INFO(
            "unotools",
            "UCBContentHelper::IsSubPath(" << parent << ", " << child << ") "
                << e.getValueType().getTypeName() << " \""
                << e.get<css::uno::Exception>().Message << '"');
    }
    return false;
}

bool utl::UCBContentHelper::EqualURLs(
    rtl::OUString const & url1, rtl::OUString const & url2)
{
    if (url1.isEmpty() || url2.isEmpty()) {
        return false;
    }
    ucbhelper::ContentBroker * broker = ucbhelper::ContentBroker::get();
    if (broker == 0) {
        throw css::uno::RuntimeException(
            rtl::OUString(
                RTL_CONSTASCII_USTRINGPARAM("no ucbhelper::ContentBroker")),
            css::uno::Reference<css::uno::XInterface>());
    }
    return
        broker->getContentProviderInterface()->compareContentIds(
            (broker->getContentIdentifierFactoryInterface()->
             createContentIdentifier(canonic(url1))),
            (broker->getContentIdentifierFactoryInterface()->
             createContentIdentifier(canonic(url2))))
        == 0;
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
