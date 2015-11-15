/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/* writerperfect
 * Version: MPL 2.0 / LGPLv2.1+
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * Major Contributor(s):
 * Copyright (C) 2007 Fridrich Strba (fridrich.strba@bluewin.ch)
 *
 * For minor contributions see the git repository.
 *
 * Alternatively, the contents of this file may be used under the terms
 * of the GNU Lesser General Public License Version 2.1 or later
 * (LGPLv2.1+), in which case the provisions of the LGPLv2.1+ are
 * applicable instead of those above.
 *
 * For further information visit http://libwpd.sourceforge.net
 */

#include <com/sun/star/container/XChild.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/sdbc/XResultSet.hpp>
#include <com/sun/star/sdbc/XRow.hpp>
#include <com/sun/star/ucb/XContent.hpp>
#include <com/sun/star/ucb/XContentAccess.hpp>
#include <com/sun/star/ucb/XCommandEnvironment.hpp>

#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/Sequence.hxx>

#include <comphelper/processfactory.hxx>

#include <rtl/ustring.hxx>

#include <ucbhelper/content.hxx>

#include <DirectoryStream.hxx>
#include <WPXSvInputStream.hxx>

namespace container = com::sun::star::container;
namespace io = com::sun::star::io;
namespace sdbc = com::sun::star::sdbc;
namespace ucb = com::sun::star::ucb;
namespace uno = com::sun::star::uno;

namespace writerperfect
{

namespace
{

uno::Reference<io::XInputStream> findStream(ucbhelper::Content &rContent, const rtl::OUString &rName)
{
    uno::Reference<io::XInputStream> xInputStream;

    uno::Sequence<OUString> lPropNames { "Title" };
    try
    {
        const uno::Reference<sdbc::XResultSet> xResultSet(
            rContent.createCursor(lPropNames, ucbhelper::INCLUDE_DOCUMENTS_ONLY));
        if (xResultSet->first())
        {
            const uno::Reference<ucb::XContentAccess> xContentAccess(xResultSet, uno::UNO_QUERY_THROW);
            const uno::Reference<sdbc::XRow> xRow(xResultSet, uno::UNO_QUERY_THROW);
            do
            {
                const rtl::OUString aTitle(xRow->getString(1));
                if (aTitle == rName)
                {
                    const uno::Reference<ucb::XContent> xSubContent(xContentAccess->queryContent());
                    ucbhelper::Content aSubContent(xSubContent, uno::Reference<ucb::XCommandEnvironment>(), comphelper::getProcessComponentContext());
                    xInputStream = aSubContent.openStream();
                    break;
                }
            }
            while (xResultSet->next());
        }
    }
    catch (const uno::RuntimeException &)
    {
        // ignore
    }
    catch (const uno::Exception &)
    {
        // ignore
    }

    return xInputStream;
}

}

struct DirectoryStream::Impl
{
    explicit Impl(const uno::Reference<ucb::XContent> &rxContent);

    uno::Reference<ucb::XContent> xContent;
};

DirectoryStream::Impl::Impl(const uno::Reference<ucb::XContent> &rxContent)
    : xContent(rxContent)
{
}

DirectoryStream::DirectoryStream(const com::sun::star::uno::Reference<com::sun::star::ucb::XContent> &xContent)
    : m_pImpl(isDirectory(xContent) ? new Impl(xContent) : nullptr)
{
}

DirectoryStream::~DirectoryStream()
{
    delete m_pImpl;
}

DirectoryStream *DirectoryStream::createForParent(const com::sun::star::uno::Reference<com::sun::star::ucb::XContent> &xContent)
{
    try
    {
        if (!xContent.is())
            return nullptr;

        DirectoryStream *pDir(nullptr);

        const uno::Reference<container::XChild> xChild(xContent, uno::UNO_QUERY);
        if (xChild.is())
        {
            const uno::Reference<ucb::XContent> xDirContent(xChild->getParent(), uno::UNO_QUERY);
            if (xDirContent.is())
            {
                pDir = new writerperfect::DirectoryStream(xDirContent);
                if (!pDir->isStructured())
                {
                    delete pDir;
                    pDir = nullptr;
                }
            }
        }

        return pDir;
    }
    catch (...)
    {
        return nullptr;
    }
}

bool DirectoryStream::isDirectory(const com::sun::star::uno::Reference<com::sun::star::ucb::XContent> &xContent)
{
    try
    {
        if (!xContent.is())
            return false;

        ucbhelper::Content aContent(xContent, uno::Reference<ucb::XCommandEnvironment>(), comphelper::getProcessComponentContext());
        return aContent.isFolder();
    }
    catch (...)
    {
        return false;
    }
}

bool DirectoryStream::isStructured()
{
    if (!m_pImpl)
        return false;

    return true;
}

unsigned DirectoryStream::subStreamCount()
{
    // TODO: implement me
    return 0;
}

const char *DirectoryStream::subStreamName(unsigned /* id */)
{
    // TODO: implement me
    return nullptr;
}

bool DirectoryStream::existsSubStream(const char * /* name */)
{
    // TODO: implement me
    return false;
}

librevenge::RVNGInputStream *DirectoryStream::getSubStreamByName(const char *const pName)
{
    if (!m_pImpl)
        return nullptr;

    ucbhelper::Content aContent(m_pImpl->xContent, uno::Reference<ucb::XCommandEnvironment>(), comphelper::getProcessComponentContext());
    const uno::Reference<io::XInputStream> xInputStream(findStream(aContent, rtl::OUString::createFromAscii(pName)));
    if (xInputStream.is())
        return new WPXSvInputStream(xInputStream);

    return nullptr;
}

librevenge::RVNGInputStream *DirectoryStream::getSubStreamById(unsigned /* id */)
{
    // TODO: implement me
    return nullptr;
}

const unsigned char *DirectoryStream::read(unsigned long, unsigned long &nNumBytesRead)
{
    nNumBytesRead = 0;
    return nullptr;
}

int DirectoryStream::seek(long, librevenge::RVNG_SEEK_TYPE)
{
    return -1;
}

long DirectoryStream::tell()
{
    return 0;
}

bool DirectoryStream::isEnd()
{
    return true;
}

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
