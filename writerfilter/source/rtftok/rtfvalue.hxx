/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFVALUE_HXX
#define INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFVALUE_HXX

#include <resourcemodel/WW8ResourceModel.hxx>
#include <com/sun/star/io/XInputStream.hpp>
#include <com/sun/star/embed/XEmbeddedObject.hpp>

namespace writerfilter
{
namespace rtftok
{
class RTFSprms;
class RTFShape;
/// Value of an RTF keyword
class RTFValue
    : public Value
{
public:
    typedef boost::shared_ptr<RTFValue> Pointer_t;
    RTFValue(int nValue, const OUString& sValue, RTFSprms rAttributes, RTFSprms rSprms, css::uno::Reference<css::drawing::XShape> rShape,
             css::uno::Reference<css::io::XInputStream> rStream, css::uno::Reference<css::embed::XEmbeddedObject> rObject, bool bForceString,
             const RTFShape& aShape);
    RTFValue();
    RTFValue(int nValue);
    RTFValue(const OUString& sValue, bool bForce = false);
    RTFValue(RTFSprms rAttributes);
    RTFValue(RTFSprms rAttributes, RTFSprms rSprms);
    RTFValue(css::uno::Reference<css::drawing::XShape> rShape);
    RTFValue(css::uno::Reference<css::io::XInputStream> rStream);
    RTFValue(css::uno::Reference<css::embed::XEmbeddedObject> rObject);
    RTFValue(const RTFShape& aShape);
    virtual ~RTFValue();
    void setString(const OUString& sValue);
    virtual int getInt() const SAL_OVERRIDE;
    virtual OUString getString() const SAL_OVERRIDE;
    virtual css::uno::Any getAny() const SAL_OVERRIDE;
    virtual writerfilter::Reference<Properties>::Pointer_t getProperties() SAL_OVERRIDE;
    virtual writerfilter::Reference<Stream>::Pointer_t getStream() SAL_OVERRIDE;
    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary() SAL_OVERRIDE;
    virtual std::string toString() const SAL_OVERRIDE;
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
    css::uno::Reference<css::drawing::XShape> m_xShape;
    css::uno::Reference<css::io::XInputStream> m_xStream;
    css::uno::Reference<css::embed::XEmbeddedObject> m_xObject;
    bool m_bForceString;
    boost::shared_ptr<RTFShape> m_pShape;
};
} // namespace rtftok
} // namespace writerfilter

#endif // INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFVALUE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
