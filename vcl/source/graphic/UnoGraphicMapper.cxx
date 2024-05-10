/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#include <com/sun/star/uno/XComponentContext.hpp>
#include <com/sun/star/lang/XServiceInfo.hpp>
#include <com/sun/star/graphic/XGraphicMapper.hpp>
#include <com/sun/star/graphic/XGraphic.hpp>
#include <cppuhelper/implbase.hxx>
#include <cppuhelper/supportsservice.hxx>
#include <comphelper/unique_disposing_ptr.hxx>

#include <memory>
#include <unordered_map>

using namespace css;

namespace
{
class GraphicMapper : public cppu::WeakImplHelper<graphic::XGraphicMapper, lang::XServiceInfo>
{
private:
    std::unordered_map<OUString, uno::Reference<graphic::XGraphic>> maGraphicMap;

public:
    GraphicMapper() = default;

protected:
    // XServiceInfo
    OUString SAL_CALL getImplementationName() override
    {
        return u"com.sun.star.comp.graphic.GraphicMapper"_ustr;
    }
    sal_Bool SAL_CALL supportsService(const OUString& ServiceName) override
    {
        return cppu::supportsService(this, ServiceName);
    }
    css::uno::Sequence<OUString> SAL_CALL getSupportedServiceNames() override
    {
        return { u"com.sun.star.graphic.GraphicMapper"_ustr };
    }

    // XTypeProvider
    css::uno::Sequence<css::uno::Type> SAL_CALL getTypes() override
    {
        static const uno::Sequence<uno::Type> aTypes{
            cppu::UnoType<lang::XServiceInfo>::get(), cppu::UnoType<lang::XTypeProvider>::get(),
            cppu::UnoType<graphic::XGraphicMapper>::get()
        };
        return aTypes;
    }
    css::uno::Sequence<sal_Int8> SAL_CALL getImplementationId() override
    {
        return css::uno::Sequence<sal_Int8>();
    }

    // XGraphicMapper
    css::uno::Reference<css::graphic::XGraphic> SAL_CALL findGraphic(const OUString& rId) override
    {
        auto aIterator = maGraphicMap.find(rId);

        if (aIterator == maGraphicMap.end())
            return css::uno::Reference<css::graphic::XGraphic>();

        return aIterator->second;
    }
    void SAL_CALL putGraphic(const OUString& rId,
                             css::uno::Reference<css::graphic::XGraphic> const& rGraphic) override
    {
        maGraphicMap.emplace(rId, rGraphic);
    }
};
}

extern "C" SAL_DLLPUBLIC_EXPORT css::uno::XInterface*
com_sun_star_comp_graphic_GraphicMapper_get_implementation(css::uno::XComponentContext*,
                                                           css::uno::Sequence<css::uno::Any> const&)
{
    return cppu::acquire(new GraphicMapper);
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
