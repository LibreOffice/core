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

// For iOS, SAL_DLLPUBLIC_EXPORT actually expands to __attribute__
// ((visibility("hidden"))). (Ditto for other DISABLE_DYNLOADING
// cases, but let it be as is for them for now.) Undo that trick.

#ifdef IOS
#define BUILDER_FACTORY_EXPORT __attribute__ ((visibility("default")))
#else
#define BUILDER_FACTORY_EXPORT SAL_DLLPUBLIC_EXPORT
#endif

#define VCL_BUILDER_FACTORY(typeName) \
    extern "C" BUILDER_FACTORY_EXPORT void SAL_CALL make##typeName(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap) \
    { \
        (void)rMap; \
        rRet = VclPtr<typeName>::Create(pParent); \
    }

#define VCL_BUILDER_FACTORY_ARGS(typeName,arg1) \
    extern "C" BUILDER_FACTORY_EXPORT void SAL_CALL make##typeName(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap) \
    { \
        (void)rMap; \
        rRet = VclPtr<typeName>::Create(pParent,arg1); \
    }

#define VCL_BUILDER_FACTORY_CONSTRUCTOR(typeName,arg2) \
    extern "C" BUILDER_FACTORY_EXPORT void SAL_CALL make##typeName(VclPtr<vcl::Window> & rRet, VclPtr<vcl::Window> & pParent, VclBuilder::stringmap & rMap) \
    { \
        OUString sBorder = BuilderUtils::extractCustomProperty(rMap); \
        WinBits wb = arg2; \
        if (!sBorder.isEmpty()) \
            wb |= WB_BORDER; \
        rRet = VclPtr<typeName>::Create(pParent,wb); \
    }

#endif

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
