/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef _RTFVALUE_HXX_
#define _RTFVALUE_HXX_

#include <resourcemodel/WW8ResourceModel.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

namespace writerfilter {
    namespace rtftok {
        class RTFSprms;
        class RTFShape;
        /// Value of an RTF keyword
        class RTFValue
            : public Value
        {
            public:
                typedef boost::shared_ptr<RTFValue> Pointer_t;
                RTFValue(int nValue, const OUString& sValue, RTFSprms rAttributes, RTFSprms rSprms, uno::Reference<drawing::XShape> rShape,
                        uno::Reference<io::XInputStream> rStream, uno::Reference<embed::XEmbeddedObject> rObject, bool bForceString,
                        RTFShape aShape);
                RTFValue();
                RTFValue(int nValue);
                RTFValue(const OUString& sValue, bool bForce = false);
                RTFValue(RTFSprms rAttributes);
                RTFValue(RTFSprms rAttributes, RTFSprms rSprms);
                RTFValue(uno::Reference<drawing::XShape> rShape);
                RTFValue(uno::Reference<io::XInputStream> rStream);
                RTFValue(uno::Reference<embed::XEmbeddedObject> rObject);
                RTFValue(RTFShape aShape);
                virtual ~RTFValue();
                void setString(const OUString& sValue);
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
                RTFShape& getShape() const;
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
                boost::shared_ptr<RTFShape> m_pShape;
        };
    } // namespace rtftok
} // namespace writerfilter

#endif // _RTFVALUE_HXX_

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
