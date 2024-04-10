/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <dmapper/resourcemodel.hxx>

#include <com/sun/star/text/GraphicCrop.hpp>

#include <optional>

namespace com::sun::star
{
namespace embed
{
class XEmbeddedObject;
}
namespace io
{
class XInputStream;
}
}

namespace writerfilter::rtftok
{
class RTFSprms;
class RTFShape;
class RTFPicture;
/// Value of an RTF keyword
class RTFValue : public Value
{
    RTFValue(int nValue, OUString sValue, const RTFSprms* pAttributes, const RTFSprms* pSprms,
             css::uno::Reference<css::drawing::XShape> xShape,
             css::uno::Reference<css::io::XInputStream> xStream,
             css::uno::Reference<css::embed::XEmbeddedObject> xObject, bool bForceString,
             const RTFShape* pShape, const RTFPicture* pPicture);

public:
    using Pointer_t = tools::SvRef<RTFValue>;
    RTFValue();
    explicit RTFValue(int nValue);
    RTFValue(OUString sValue, bool bForce = false);
    explicit RTFValue(const RTFSprms& rAttributes);
    RTFValue(const RTFSprms& rAttributes, const RTFSprms& rSprms);
    explicit RTFValue(css::uno::Reference<css::drawing::XShape> xShape);
    explicit RTFValue(css::uno::Reference<css::io::XInputStream> xStream);
    explicit RTFValue(css::uno::Reference<css::embed::XEmbeddedObject> xObject);
    explicit RTFValue(const RTFShape& aShape);
    explicit RTFValue(const RTFPicture& rPicture);
    explicit RTFValue(css::text::GraphicCrop const& rCrop);
    ~RTFValue() override;
    void setString(const OUString& sValue);
    int getInt() const override;
    OUString getString() const override;
    css::uno::Any getAny() const override;
    writerfilter::Reference<Properties>::Pointer_t getProperties() override;
    writerfilter::Reference<BinaryObj>::Pointer_t getBinary() override;
#ifdef DBG_UTIL
    std::string toString() const override;
#endif
    RTFValue* Clone() const;
    RTFValue* CloneWithSprms(RTFSprms const& rAttributes, RTFSprms const& rSprms) const;
    RTFSprms& getAttributes() const;
    RTFSprms& getSprms() const;
    RTFShape& getShape() const;
    RTFPicture& getPicture() const;
    bool equals(const RTFValue& rOther) const;
    RTFValue& operator=(RTFValue const& rOther) = delete;

private:
    int m_nValue = 0;
    OUString m_sValue;
    mutable tools::SvRef<RTFSprms> m_pAttributes;
    mutable tools::SvRef<RTFSprms> m_pSprms;
    css::uno::Reference<css::drawing::XShape> m_xShape;
    css::uno::Reference<css::io::XInputStream> m_xStream;
    css::uno::Reference<css::embed::XEmbeddedObject> m_xObject;
    bool m_bForceString = false;
    mutable tools::SvRef<RTFShape> m_pShape;
    mutable tools::SvRef<RTFPicture> m_pPicture;
    ::std::optional<css::text::GraphicCrop> m_oCrop;
};
} // namespace writerfilter::rtftok

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
