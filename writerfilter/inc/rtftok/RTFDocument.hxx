/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERFILTER_INC_RTFTOK_RTFDOCUMENT_HXX
#define INCLUDED_WRITERFILTER_INC_RTFTOK_RTFDOCUMENT_HXX

#include <dmapper/resourcemodel.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XFrame.hpp>
#include <com/sun/star/task/XStatusIndicator.hpp>

namespace writerfilter
{
namespace rtftok
{
/// The RTFDocument opens and resolves the RTF document.
class RTFDocument
    : public writerfilter::Reference<Stream>
{
public:
    /// Pointer to this stream.
    typedef std::shared_ptr<RTFDocument> Pointer_t;

    virtual ~RTFDocument() { }

    /// Resolves this document to a stream handler.
    virtual void resolve(Stream& rHandler) override = 0;
};

/// Interface to create an RTFDocument instance.
class RTFDocumentFactory
{
public:
    static RTFDocument::Pointer_t
    createDocument(css::uno::Reference<css::uno::XComponentContext> const& xContext,
                   css::uno::Reference<css::io::XInputStream> const& xInputStream,
                   css::uno::Reference<css::lang::XComponent> const& xDstDoc,
                   css::uno::Reference<css::frame::XFrame> const& xFrame,
                   css::uno::Reference<css::task::XStatusIndicator> const& xStatusIndicator,
                   bool bIsNewDoc);
};
} // namespace rtftok
} // namespace writerfilter

#endif // INCLUDED_WRITERFILTER_INC_RTFTOK_RTFDOCUMENT_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
