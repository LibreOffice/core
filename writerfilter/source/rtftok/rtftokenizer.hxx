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

#ifndef _RTFTOKENIZER_HXX_
#define _RTFTOKENIZER_HXX_

#include <rtfdocumentimpl.hxx>

class SvStream;

namespace writerfilter {
    namespace rtftok {
        /// RTF tokenizer that separates control words from text.
        class RTFTokenizer
        {
            public:
                RTFTokenizer(RTFDocumentImpl& rImport, SvStream* pInStream, uno::Reference<task::XStatusIndicator> const& xStatusIndicator);
                virtual ~RTFTokenizer();

                int resolveParse();
                int asHex(char ch);
                /// Number of states on the stack.
                int getGroup() const;
                /// To be invoked by the pushState() callback to signal when the importer enters a group.
                void pushGroup();
                /// To be invoked by the popState() callback to single when the importer leaves a group.
                void popGroup();
                OUString getPosition();
            private:
                SvStream& Strm();
                int resolveKeyword();
                int dispatchKeyword(rtl::OString& rKeyword, bool bParam, int nParam);

                RTFDocumentImpl& m_rImport;
                SvStream* m_pInStream;
                uno::Reference<task::XStatusIndicator> const& m_xStatusIndicator;
                // This is the same as m_aRTFControlWords, but sorted
                std::vector<RTFSymbol> m_aRTFControlWords;
                /// Same as the size of the importer's states, except that this can be negative for invalid input.
                int m_nGroup;
                sal_Int32 m_nLineNumber;
                sal_Int32 m_nLineStartPos;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFTOKENIZER_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
