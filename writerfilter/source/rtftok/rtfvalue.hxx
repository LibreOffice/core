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

#ifndef _RTFVALUE_HXX_
#define _RTFVALUE_HXX_

#include <resourcemodel/WW8ResourceModel.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

namespace writerfilter {
    namespace rtftok {
        class RTFSprms;
        /// Value of an RTF keyword
        class RTFValue
            : public Value
        {
            public:
                typedef boost::shared_ptr<RTFValue> Pointer_t;
                RTFValue(int nValue, OUString sValue, RTFSprms rAttributes, RTFSprms rSprms, uno::Reference<drawing::XShape> rShape,
                        uno::Reference<io::XInputStream> rStream, uno::Reference<embed::XEmbeddedObject> rObject, bool bForceString);
                RTFValue(int nValue);
                RTFValue(OUString sValue, bool bForce = false);
                RTFValue(RTFSprms rAttributes);
                RTFValue(RTFSprms rAttributes, RTFSprms rSprms);
                RTFValue(uno::Reference<drawing::XShape> rShape);
                RTFValue(uno::Reference<io::XInputStream> rStream);
                RTFValue(uno::Reference<embed::XEmbeddedObject> rObject);
                virtual ~RTFValue();
                void setString(OUString sValue);
                virtual int getInt() const;
                virtual OUString getString() const;
                virtual uno::Any getAny() const;
                virtual writerfilter::Reference<Properties>::Pointer_t getProperties();
                virtual writerfilter::Reference<Stream>::Pointer_t getStream();
                virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary();
                virtual std::string toString() const;
                virtual RTFValue* Clone();
                RTFSprms& getAttributes();
                RTFSprms& getSprms();
                bool equals(RTFValue& rOther);
            private:
                RTFValue& operator=(RTFValue const& rOther);
                int m_nValue;
                OUString m_sValue;
                boost::shared_ptr<RTFSprms> m_pAttributes;
                boost::shared_ptr<RTFSprms> m_pSprms;
                uno::Reference<drawing::XShape> m_xShape;
                uno::Reference<io::XInputStream> m_xStream;
                uno::Reference<embed::XEmbeddedObject> m_xObject;
                bool m_bForceString;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFVALUE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
