/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _RTFSDRIMPORT_HXX_
#define _RTFSDRIMPORT_HXX_

#include <stack>

#include <rtfdocumentimpl.hxx>

namespace writerfilter {
    namespace rtftok {
        /// Handles the import of drawings using RTF markup.
        class RTFSdrImport
        {
            public:
                RTFSdrImport(RTFDocumentImpl& rImport, uno::Reference<lang::XComponent> const& xDstDoc);
                virtual ~RTFSdrImport();

                void resolve(RTFShape& rShape, bool bClose);
                void close();
                void append(OUString aKey, OUString aValue);
                void resolveDhgt(uno::Reference<beans::XPropertySet> xPropertySet, sal_Int32 nZOrder);
                void resolveFLine(uno::Reference<beans::XPropertySet> xPropertySet, sal_Int32 nFLine);
                /**
                 * These are the default in Word, but not in Writer.
                 *
                 * @param bNew if the frame is new-style or old-style.
                 */
                std::vector<beans::PropertyValue> getTextFrameDefaults(bool bNew);
                /// Push a new group shape to the parent stack.
                void pushParent(uno::Reference<drawing::XShapes> xParent);
                /// Pop the current group shape from the parent stack.
                void popParent();
            private:
                void createShape(OUString aService, uno::Reference<drawing::XShape>& xShape, uno::Reference<beans::XPropertySet>& xPropertySet);
                void applyProperty(uno::Reference<drawing::XShape> xShape, OUString aKey, OUString aValue);

                RTFDocumentImpl& m_rImport;
                std::stack< uno::Reference<drawing::XShapes> > m_aParents;
                uno::Reference<drawing::XShape> m_xShape;
                /// If m_xShape is imported as a Writer text frame (instead of a drawinglayer rectangle).
                bool m_bTextFrame;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFSDRIPORT_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
