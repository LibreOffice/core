/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vector>

#include <dmapper/resourcemodel.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>

namespace utl
{
class MediaDescriptor;
}

namespace writerfilter::dmapper
{
enum class SourceDocumentType
{
    OOXML,
    RTF
};

/// Interface to create a DomainMapper instance.
class DomainMapperFactory
{
public:
    static Stream::Pointer_t
    createMapper(css::uno::Reference<css::uno::XComponentContext> const& xContext,
                 css::uno::Reference<css::io::XInputStream> const& xInputStream,
                 css::uno::Reference<css::lang::XComponent> const& xModel, bool bRepairStorage,
                 SourceDocumentType eDocumentType, utl::MediaDescriptor const& rMediaDesc);
};

// export just for test
SAL_DLLPUBLIC_EXPORT std::tuple<OUString, std::vector<OUString>, std::vector<OUString>>
splitFieldCommand(const OUString& rCommand);

} // namespace writerfilter::dmapper

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
