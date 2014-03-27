/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _RTFREFERENCETABLE_HXX_
#define _RTFREFERENCETABLE_HXX_

#include <map>
#include <resourcemodel/WW8ResourceModel.hxx>

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
                virtual void resolve(Table & rHandler) SAL_OVERRIDE;
                virtual std::string getType() const SAL_OVERRIDE;
            private:
                Entries_t m_aEntries;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFREFERENCETABLE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
