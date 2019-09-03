/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#ifndef INCLUDED_VCL_BUILDER_FACTORY_HXX
#define INCLUDED_VCL_BUILDER_FACTORY_HXX

#include <vcl/vclptr.hxx>
#include <vcl/builder.hxx>

#define VCL_BUILDER_FACTORY(typeName) \
    extern "C" SAL_DLLPUBLIC_EXPORT void make##typeName(VclPtr<vcl::Window> & rRet, const VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap) \
    { \
        static_assert(std::is_same_v<std::remove_pointer_t<VclBuilder::customMakeWidget>,          \
                                     decltype(make##typeName)>);                                   \
        (void)rMap; \
        rRet = VclPtr<typeName>::Create(pParent); \
    }

#define VCL_BUILDER_FACTORY_ARGS(typeName,arg1) \
    extern "C" SAL_DLLPUBLIC_EXPORT void make##typeName(VclPtr<vcl::Window> & rRet, const VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap) \
    { \
        static_assert(std::is_same_v<std::remove_pointer_t<VclBuilder::customMakeWidget>,          \
                                     decltype(make##typeName)>);                                   \
        (void)rMap; \
        rRet = VclPtr<typeName>::Create(pParent,arg1); \
    }

#define VCL_BUILDER_FACTORY_CONSTRUCTOR(typeName,arg2) \
    extern "C" SAL_DLLPUBLIC_EXPORT void make##typeName(VclPtr<vcl::Window> & rRet, const VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap) \
    { \
        static_assert(std::is_same_v<std::remove_pointer_t<VclBuilder::customMakeWidget>,          \
                                     decltype(make##typeName)>);                                   \
        OUString sBorder = BuilderUtils::extractCustomProperty(rMap); \
        WinBits wb = arg2; \
        if (!sBorder.isEmpty()) \
            wb |= WB_BORDER; \
        rRet = VclPtr<typeName>::Create(pParent,wb); \
    }

#define VCL_BUILDER_FACTORY_EXTERN(typeName) \
    extern "C" void make##typeName(VclPtr<vcl::Window> & rRet, const VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap)

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
