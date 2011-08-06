/*
 * Version: MPL 1.1 / GPLv3+ / LGPLv3+
 *
 * The contents of this file are subject to the Mozilla Public License Version
 * 1.1 (the "License"); you may not use this file except in compliance with
 * the License. You may obtain a copy of the License at
 * http://www.mozilla.org/MPL/
 *
 * Software distributed under the License is distributed on an "AS IS" basis,
 * WITHOUT WARRANTY OF ANY KIND, either express or implied. See the License
 * for the specific language governing rights and limitations under the
 * License.
 *
 * The Initial Developer of the Original Code is
 *       Miklos Vajna <vmiklos@frugalware.org>
 * Portions created by the Initial Developer are Copyright (C) 2011 the
 * Initial Developer. All Rights Reserved.
 *
 * Contributor(s):
 *
 * Alternatively, the contents of this file may be used under the terms of
 * either the GNU General Public License Version 3 or later (the "GPLv3+"), or
 * the GNU Lesser General Public License Version 3 or later (the "LGPLv3+"),
 * in which case the provisions of the GPLv3+ or the LGPLv3+ are applicable
 * instead of those above.
 */

#ifndef _RTFDOCUMENT_HXX_
#define _RTFDOCUMENT_HXX_

#include <resourcemodel/WW8ResourceModel.hxx>
#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/lang/XComponent.hpp>
#include <com/sun/star/frame/XFrame.hpp>

namespace writerfilter {
    namespace rtftok {
        /// The RTFDocument opens and resolves the RTF document.
        class WRITERFILTER_RTFTOK_DLLPUBLIC RTFDocument
            : public writerfilter::Reference<Stream>
        {
            public:
                /// Pointer to this stream.
                typedef ::boost::shared_ptr<RTFDocument> Pointer_t;

                virtual ~RTFDocument() { }

                /// Resolves this document to a stream handler.
                virtual void resolve(Stream & rHandler) = 0;

                /// Returns string representation of the type of this reference. (Debugging purpose only.)
                virtual ::std::string getType() const = 0;
        };

        /// Interface to create an RTFDocument instance.
        class WRITERFILTER_RTFTOK_DLLPUBLIC RTFDocumentFactory
        {
            public:
                static RTFDocument::Pointer_t
                    createDocument(
                            ::com::sun::star::uno::Reference< ::com::sun::star::uno::XComponentContext > const & xContext,
                            ::com::sun::star::uno::Reference< ::com::sun::star::io::XInputStream > const & xInputStream,
                            ::com::sun::star::uno::Reference< ::com::sun::star::lang::XComponent > const & xDstDoc,
                            ::com::sun::star::uno::Reference< ::com::sun::star::frame::XFrame > const & xFrame);
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFDOCUMENT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
