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
class RTFPicture;
/// Value of an RTF keyword
class RTFValue : public Value
{
public:
    using Pointer_t = std::shared_ptr<RTFValue>;
    RTFValue(int nValue, OUString sValue, RTFSprms rAttributes, RTFSprms rSprms,
             css::uno::Reference<css::drawing::XShape> xShape,
             css::uno::Reference<css::io::XInputStream> xStream,
             css::uno::Reference<css::embed::XEmbeddedObject> xObject, bool bForceString,
             const RTFShape& aShape, const RTFPicture& rPicture);
    RTFValue();
    explicit RTFValue(int nValue);
    RTFValue(OUString sValue, bool bForce = false);
    explicit RTFValue(RTFSprms rAttributes);
    RTFValue(RTFSprms rAttributes, RTFSprms rSprms);
    explicit RTFValue(css::uno::Reference<css::drawing::XShape> xShape);
    explicit RTFValue(css::uno::Reference<css::io::XInputStream> xStream);
    explicit RTFValue(css::uno::Reference<css::embed::XEmbeddedObject> xObject);
    explicit RTFValue(const RTFShape& aShape);
    explicit RTFValue(const RTFPicture& rPicture);
    ~RTFValue() override;
    void setString(const OUString& sValue);
    int getInt() const override;
    OUString getString() const override;
    css::uno::Any getAny() const override;
    writerfilter::Reference<Properties>::Pointer_t getProperties() override;
    writerfilter::Reference<BinaryObj>::Pointer_t getBinary() override;
#ifdef DEBUG_WRITERFILTER
    std::string toString() const override;
#endif
    RTFValue* Clone();
    RTFValue* CloneWithSprms(RTFSprms const& rAttributes, RTFSprms const& rSprms);
    RTFSprms& getAttributes();
    RTFSprms& getSprms();
    RTFShape& getShape() const;
    RTFPicture& getPicture() const;
    bool equals(RTFValue& rOther);
    RTFValue& operator=(RTFValue const& rOther) = delete;

private:
    int m_nValue = 0;
    OUString m_sValue;
    std::shared_ptr<RTFSprms> m_pAttributes;
    std::shared_ptr<RTFSprms> m_pSprms;
    css::uno::Reference<css::drawing::XShape> m_xShape;
    css::uno::Reference<css::io::XInputStream> m_xStream;
    css::uno::Reference<css::embed::XEmbeddedObject> m_xObject;
    bool m_bForceString = false;
    std::shared_ptr<RTFShape> m_pShape;
    std::shared_ptr<RTFPicture> m_pPicture;
};
} // namespace rtftok
} // namespace writerfilter

#endif // INCLUDED_WRITERFILTER_SOURCE_RTFTOK_RTFVALUE_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
