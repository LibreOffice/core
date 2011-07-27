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

#include <string>
#include <resourcemodel/WW8ResourceModel.hxx>
#include <rtfcontrolwords.hxx>
#include <rtfvalue.hxx>

namespace writerfilter {
    namespace rtftok {
        /// A list of RTFSprm
        typedef std::vector< std::pair<Id, RTFValue::Pointer_t> > RTFSprms_t;
        namespace RTFSprms {
            typedef ::boost::shared_ptr<RTFSprms_t> Pointer_t;
        }
        /// RTF keyword with a parameter
        class RTFSprm
            : public Sprm
        {
            public:
                RTFSprm(Id nKeyword, RTFValue::Pointer_t& pValue);
                virtual sal_uInt32 getId() const;
                virtual Value::Pointer_t getValue();
                virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary();
                virtual writerfilter::Reference<Stream>::Pointer_t getStream();
                virtual writerfilter::Reference<Properties>::Pointer_t getProps();
                virtual Kind getKind();
                virtual std::string getName() const;
                virtual std::string toString() const;
                static RTFValue::Pointer_t find(RTFSprms_t& rVector, Id nKeyword);
                static void erase(RTFSprms_t& rVector, Id nKeyword);
                static RTFSprms_t Clone(RTFSprms_t& rVector);
            private:
                Id m_nKeyword;
                RTFValue::Pointer_t& m_pValue;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFSPRM_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
