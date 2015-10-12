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

#include <dmapper/resourcemodel.hxx>
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
    typedef std::shared_ptr<RTFValue> Pointer_t;
    RTFValue(int nValue, const OUString& sValue, RTFSprms rAttributes, RTFSprms rSprms,
             css::uno::Reference<css::drawing::XShape> const& xShape,
             css::uno::Reference<css::io::XInputStream> const& xStream,
             css::uno::Reference<css::embed::XEmbeddedObject> const& xObject,
             bool bForceString, const RTFShape& aShape);
    RTFValue();
    RTFValue(int nValue);
    RTFValue(const OUString& sValue, bool bForce = false);
    RTFValue(RTFSprms rAttributes);
    RTFValue(RTFSprms rAttributes, RTFSprms rSprms);
    RTFValue(css::uno::Reference<css::drawing::XShape> const& xShape);
    RTFValue(css::uno::Reference<css::io::XInputStream> const& xStream);
    RTFValue(css::uno::Reference<css::embed::XEmbeddedObject> const& xObject);
    RTFValue(const RTFShape& aShape);
    virtual ~RTFValue();
    void setString(const OUString& sValue);
    virtual int getInt() const override;
    virtual OUString getString() const override;
    virtual css::uno::Any getAny() const override;
    virtual writerfilter::Reference<Properties>::Pointer_t getProperties() override;
    virtual writerfilter::Reference<BinaryObj>::Pointer_t getBinary() override;
#ifdef DEBUG_WRITERFILTER
    virtual std::string toString() const override;
#endif
    RTFValue* Clone();
    RTFValue* CloneWithSprms(RTFSprms const& rAttributes, RTFSprms const& rSprms);
    RTFSprms& getAttributes();
    RTFSprms& getSprms();
    RTFShape& getShape() const;
    bool equals(RTFValue& rOther);
private:
    RTFValue& operator=(RTFValue const& rOther) = delete;
    int m_nValue;
    OUString m_sValue;
    std::shared_ptr<RTFSprms> m_pAttributes;
    std::shared_ptr<RTFSprms> m_pSprms;
    css::uno::Reference<css::drawing::XShape> m_xShape;
    css::uno::Reference<css::io::XInputStream> m_xStream;
    css::uno::Reference<css::embed::XEmbeddedObject> m_xObject;
    bool m_bForceString;
    std::shared_ptr<RTFShape> m_pShape;
};
} // namespace rtftok
} // namespace writerfilter

#endif // INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFVALUE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
