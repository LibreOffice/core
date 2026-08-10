/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4; fill-column: 100 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <cmath>
#include <vector>

#include <com/sun/star/beans/PropertyValue.hpp>
#include <com/sun/star/beans/XPropertySet.hpp>
#include <com/sun/star/container/XEnumeration.hpp>
#include <com/sun/star/container/XEnumerationAccess.hpp>
#include <com/sun/star/container/XIndexAccess.hpp>
#include <com/sun/star/frame/XController.hpp>
#include <com/sun/star/frame/XModel.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <com/sun/star/graphic/XGraphicProvider.hpp>
#include <com/sun/star/io/XTempFile.hpp>
#include <com/sun/star/lang/XMultiServiceFactory.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/text/TextContentAnchorType.hpp>
#include <com/sun/star/text/XText.hpp>
#include <com/sun/star/text/XTextContent.hpp>
#include <com/sun/star/text/XTextDocument.hpp>
#include <com/sun/star/text/XTextRange.hpp>
#include <com/sun/star/text/XTextViewCursorSupplier.hpp>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/uno/RuntimeException.hpp>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/uno/XInterface.hpp>
#include <com/sun/star/view/XSelectionSupplier.hpp>
#include <comphelper/processfactory.hxx>
#include <cpo/uno/Any.hxx>
#include <cpo/uno/Sequence.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustrbuf.hxx>
#include <rtl/ustring.hxx>
#include <sal/config.h>
#include <sal/types.h>
#include <scriptinterop/ImageOptions.hpp>
#include <scriptinterop/XDocument.hpp>
#include <scriptinterop/XParagraph.hpp>
#include <scriptinterop/XSelection.hpp>

#include "document.hxx"

namespace
{
class SelectionImpl : public cppu::WeakImplHelper<scriptinterop::XSelection>
{
public:
    explicit SelectionImpl(css::uno::Reference<css::container::XIndexAccess> const& ranges)
        : ranges_(ranges)
    {
    }

    css::uno::Reference<css::uno::XInterface> SAL_CALL getuno() override { return ranges_; }

    OUString SAL_CALL getText() override
    {
        OUStringBuffer buf;
        if (ranges_.is())
        {
            auto const n = ranges_->getCount();
            for (sal_Int32 i = 0; i != n; ++i)
            {
                css::uno::Reference<css::text::XTextRange> range;
                ranges_->getByIndex(i) >>= range;
                if (range.is())
                {
                    if (!buf.isEmpty())
                    {
                        buf.append('\n');
                    }
                    buf.append(range->getString());
                }
            }
        }
        return buf.makeStringAndClear();
    }

    void SAL_CALL replace(OUString const& newText) override
    {
        if (!ranges_.is())
        {
            return;
        }
        auto const n = ranges_->getCount();
        for (sal_Int32 i = 0; i != n; ++i)
        {
            css::uno::Reference<css::text::XTextRange> range;
            ranges_->getByIndex(i) >>= range;
            if (range.is())
            {
                range->setString(newText);
            }
        }
    }

private:
    css::uno::Reference<css::container::XIndexAccess> ranges_;
};

class ParagraphImpl : public cppu::WeakImplHelper<scriptinterop::XParagraph>
{
public:
    explicit ParagraphImpl(css::uno::Reference<css::text::XTextContent> const& content)
        : content_(content)
    {
    }

    css::uno::Reference<css::uno::XInterface> SAL_CALL getuno() override { return content_; }

    OUString SAL_CALL getText() override
    {
        css::uno::Reference<css::text::XTextRange> const range(content_, css::uno::UNO_QUERY);
        return range.is() ? range->getString() : OUString();
    }

private:
    css::uno::Reference<css::text::XTextContent> content_;
};

class DocumentImpl : public cppu::WeakImplHelper<scriptinterop::XDocument>
{
public:
    explicit DocumentImpl(css::uno::Reference<css::frame::XModel> const& model)
        : model_(model)
    {
    }

    css::uno::Reference<css::uno::XInterface> SAL_CALL getuno() override { return model_; }

    css::uno::Reference<scriptinterop::XSelection> SAL_CALL getSelection() override
    {
        css::uno::Reference<css::text::XTextDocument> const doc(model_, css::uno::UNO_QUERY_THROW);
        css::uno::Reference<css::view::XSelectionSupplier> const sup(doc->getCurrentController(),
                                                                     css::uno::UNO_QUERY);
        css::uno::Reference<css::container::XIndexAccess> ranges;
        if (sup.is())
        {
            sup->getSelection() >>= ranges;
        }
        return new SelectionImpl(ranges);
    }

    OUString SAL_CALL getText() override
    {
        css::uno::Reference<css::text::XTextDocument> const doc(model_, css::uno::UNO_QUERY_THROW);
        auto const text = doc->getText();
        return text.is() ? text->getString() : OUString();
    }

    cpo::uno::Sequence<css::uno::Reference<scriptinterop::XParagraph>>
        SAL_CALL getParagraphs() override
    {
        css::uno::Reference<css::text::XTextDocument> const doc(model_, css::uno::UNO_QUERY_THROW);
        std::vector<css::uno::Reference<scriptinterop::XParagraph>> paragraphs;
        css::uno::Reference<css::container::XEnumerationAccess> const ea(doc->getText(),
                                                                         css::uno::UNO_QUERY);
        if (ea.is())
        {
            auto const en = ea->createEnumeration();
            while (en.is() && en->hasMoreElements())
            {
                css::uno::Reference<css::text::XTextContent> xtc;
                en->nextElement() >>= xtc;
                if (!xtc.is())
                {
                    continue;
                }
                css::uno::Reference<css::lang::XServiceInfo> const info(xtc, css::uno::UNO_QUERY);
                if (!info.is() || !info->supportsService(u"com.sun.star.text.Paragraph"_ustr))
                {
                    continue;
                }
                paragraphs.emplace_back(new ParagraphImpl(xtc));
            }
        }
        return cpo::uno::Sequence<css::uno::Reference<scriptinterop::XParagraph>>(
            paragraphs.data(), paragraphs.size());
    }

    void SAL_CALL insertImage(cpo::uno::Sequence<sal_Int8> const& data,
                              scriptinterop::ImageOptions const& opts) override
    {
        css::uno::Reference<css::text::XTextDocument> const doc(model_, css::uno::UNO_QUERY_THROW);
        auto const componentCtx = comphelper::getProcessComponentContext();
        auto const smgr = componentCtx->getServiceManager();
        // Stage the bytes in a TempFile to give GraphicProvider a file URL:
        css::uno::Reference<css::io::XTempFile> const tmp(
            smgr->createInstanceWithContext(u"com.sun.star.io.TempFile"_ustr, componentCtx),
            css::uno::UNO_QUERY_THROW);
        tmp->getOutputStream()->writeBytes(data);
        tmp->getOutputStream()->closeOutput();
        css::uno::Reference<css::graphic::XGraphicProvider> const gp(
            smgr->createInstanceWithContext(u"com.sun.star.graphic.GraphicProvider"_ustr,
                                            componentCtx),
            css::uno::UNO_QUERY_THROW);
        cpo::uno::Sequence<css::beans::PropertyValue> loaderArgs{
            { u"URL"_ustr, 0, cpo::uno::Any(tmp->getUri()), {} }
        };
        auto const xgraphic = gp->queryGraphic(loaderArgs);
        if (!xgraphic.is())
        {
            throw css::uno::RuntimeException(u"insertImage: failed to load graphic"_ustr);
        }
        css::uno::Reference<css::lang::XMultiServiceFactory> const docFactory(doc,
                                                                              css::uno::UNO_QUERY);
        css::uno::Reference<css::text::XTextContent> const graphic(
            docFactory.is()
                ? docFactory->createInstance(u"com.sun.star.text.TextGraphicObject"_ustr)
                : nullptr,
            css::uno::UNO_QUERY);
        css::uno::Reference<css::beans::XPropertySet> const props(graphic, css::uno::UNO_QUERY);
        if (!props.is())
        {
            throw css::uno::RuntimeException(
                u"insertImage: failed to create TextGraphicObject"_ustr);
        }
        props->setPropertyValue(u"Graphic"_ustr, cpo::uno::Any(xgraphic));
        // Width and Height are in 1/100 mm:
        props->setPropertyValue(
            u"Width"_ustr, cpo::uno::Any(static_cast<sal_Int32>(std::round(opts.widthCm * 1000))));
        props->setPropertyValue(u"Height"_ustr, cpo::uno::Any(static_cast<sal_Int32>(
                                                    std::round(opts.heightCm * 1000))));
        props->setPropertyValue(u"AnchorType"_ustr,
                                cpo::uno::Any(css::text::TextContentAnchorType_AS_CHARACTER));
        css::uno::Reference<css::text::XTextViewCursorSupplier> const cursorSupplier(
            doc->getCurrentController(), css::uno::UNO_QUERY);
        css::uno::Reference<css::text::XTextRange> const cursor(
            cursorSupplier.is() ? cursorSupplier->getViewCursor()
                                : css::uno::Reference<css::text::XTextViewCursor>());
        if (!cursor.is())
        {
            throw css::uno::RuntimeException(u"insertImage: no view cursor"_ustr);
        }
        doc->getText()->insertTextContent(cursor, graphic, false);
    }

private:
    css::uno::Reference<css::frame::XModel> model_;
};
}

namespace scriptinterop::detail
{
css::uno::Reference<scriptinterop::XDocument>
createDocument(css::uno::Reference<css::frame::XModel> const& model)
{
    return new DocumentImpl(model);
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
