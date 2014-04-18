/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _RTFSPRM_HXX_
#define _RTFSPRM_HXX_

#include <boost/intrusive_ptr.hpp>
#include <rtfcontrolwords.hxx>
#include <rtfvalue.hxx>

namespace writerfilter {
    namespace rtftok {

        typedef std::vector< std::pair<Id, RTFValue::Pointer_t> > RTFSprmsImplBase;

        /// The payload of RTFSprms which is only copied on write.
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
            RTFValue::Pointer_t find(Id nKeyword, bool bFirst = true, bool bForWrite = false);
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
