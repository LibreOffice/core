/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _RTFDOCUMENT_HXX_
#define _RTFDOCUMENT_HXX_

#include <resourcemodel/WW8ResourceModel.hxx>
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
class WRITERFILTER_RTFTOK_DLLPUBLIC RTFDocument
    : public writerfilter::Reference<Stream>
{
public:
    /// Pointer to this stream.
    typedef boost::shared_ptr<RTFDocument> Pointer_t;

    virtual ~RTFDocument() { }

    /// Resolves this document to a stream handler.
    virtual void resolve(Stream& rHandler) SAL_OVERRIDE = 0;

    /// Returns string representation of the type of this reference. (Debugging purpose only.)
    virtual ::std::string getType() const SAL_OVERRIDE = 0;
};

/// Interface to create an RTFDocument instance.
class WRITERFILTER_RTFTOK_DLLPUBLIC RTFDocumentFactory
{
public:
    static RTFDocument::Pointer_t
    createDocument(css::uno::Reference<css::uno::XComponentContext> const& xContext,
                   css::uno::Reference<css::io::XInputStream> const& xInputStream,
                   css::uno::Reference<css::lang::XComponent> const& xDstDoc,
                   css::uno::Reference<css::frame::XFrame> const& xFrame,
                   css::uno::Reference<css::task::XStatusIndicator> const& xStatusIndicator);
};
} // namespace rtftok
} // namespace writerfilter

#endif // _RTFDOCUMENT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
