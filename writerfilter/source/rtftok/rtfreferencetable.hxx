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

#ifndef _RTFREFERENCETABLE_HXX_
#define _RTFREFERENCETABLE_HXX_

#include <map>
#include <resourcemodel/WW8ResourceModel.hxx>

class SvStream;

namespace writerfilter {
    namespace rtftok {
        /// Sends tables (e.g. font table) to the domain mapper.
        class RTFReferenceTable
            : public writerfilter::Reference<Table>
        {
            public:
                typedef std::map<int, writerfilter::Reference<Properties>::Pointer_t> Entries_t;
                typedef std::pair<int, writerfilter::Reference<Properties>::Pointer_t> Entry_t;
                RTFReferenceTable(Entries_t const& rEntries);
                virtual ~RTFReferenceTable();
                virtual void resolve(Table & rHandler);
                virtual std::string getType() const;
            private:
                Entries_t m_aEntries;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFREFERENCETABLE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
