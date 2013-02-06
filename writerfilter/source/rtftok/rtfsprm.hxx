/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
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

#ifndef _RTFSPRM_HXX_
#define _RTFSPRM_HXX_

#include <boost/intrusive_ptr.hpp>
#include <rtfcontrolwords.hxx>
#include <rtfvalue.hxx>

namespace writerfilter {
    namespace rtftok {

        typedef std::vector< std::pair<Id, RTFValue::Pointer_t> > RTFSprmsImplBase;
        class RTFSprmsImpl : public RTFSprmsImplBase
        {
        public:
            sal_Int32 m_nRefCount;
            RTFSprmsImpl() : RTFSprmsImplBase(), m_nRefCount(0) {}
        };

        inline void intrusive_ptr_add_ref(RTFSprmsImpl* p)
        {
            ++(p->m_nRefCount);
        }
        inline void intrusive_ptr_release(RTFSprmsImpl* p)
        {
            if (!--(p->m_nRefCount))
                delete p;
        }

        /// A list of RTFSprm with a copy constructor that performs a deep copy.
        class RTFSprms
        {
        public:
            typedef ::boost::shared_ptr<RTFSprms> Pointer_t;
            typedef std::pair<Id, RTFValue::Pointer_t> Entry_t;
            typedef std::vector<Entry_t>::iterator Iterator_t;
            RTFSprms();
            RTFSprms(const RTFSprms& rSprms);
            ~RTFSprms();
            RTFSprms& operator=(const RTFSprms& rOther);
            RTFValue::Pointer_t find(Id nKeyword, bool bFirst = true);
            /// Does the same as ->push_back(), except that it can overwrite existing entries.
            void set(Id nKeyword, RTFValue::Pointer_t pValue, bool bOverwrite = true);
            bool erase(Id nKeyword);
            /// Removes elements, which are already in the reference set.
            void deduplicate(RTFSprms& rReference);
            size_t size() const { return m_pSprms->size(); }
            bool empty() const { return m_pSprms->empty(); }
            Entry_t& back() { return m_pSprms->back(); }
            Iterator_t begin() { return m_pSprms->begin(); }
            Iterator_t end() { return m_pSprms->end(); }
            void clear();
        private:
            void ensureCopyBeforeWrite();
            boost::intrusive_ptr<RTFSprmsImpl> m_pSprms;
        };

        /// RTF keyword with a parameter
        class RTFSprm
            : public Sprm
        {
            public:
                RTFSprm(Id nKeyword, RTFValue::Pointer_t& pValue);
                virtual ~RTFSprm() {}
                virtual sal_uInt32 getId() const;
                virtual Value::Pointer_t getValue();
                virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary();
                virtual writerfilter::Reference<Stream>::Pointer_t getStream();
                virtual writerfilter::Reference<Properties>::Pointer_t getProps();
                virtual Kind getKind();
                virtual std::string getName() const;
                virtual std::string toString() const;
            private:
                Id m_nKeyword;
                RTFValue::Pointer_t& m_pValue;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFSPRM_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
