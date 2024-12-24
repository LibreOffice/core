/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 * This file incorporates work covered by the following license notice:
 *
 *   Licensed to the Apache Software Foundation (ASF) under one or more
 *   contributor license agreements. See the NOTICE file distributed
 *   with this work for additional information regarding copyright
 *   ownership. The ASF licenses this file to you under the Apache
 *   License, Version 2.0 (the "License"); you may not use this file
 *   except in compliance with the License. You may obtain a copy of
 *   the License at http://www.apache.org/licenses/LICENSE-2.0 .
 */

#ifndef INCLUDED_SFX2_SFXMODELFACTORY_HXX
#define INCLUDED_SFX2_SFXMODELFACTORY_HXX

#include <sfx2/dllapi.h>

#include <com/sun/star/uno/Reference.hxx>
#include <o3tl/typed_flags_set.hxx>
#include <functional>

namespace com::sun::star::uno { class XInterface; }
namespace com::sun::star::uno { template <class E> class Sequence; }

enum class SfxModelFlags
{
    NONE                       = 0x00,
    EMBEDDED_OBJECT            = 0x01,
    EXTERNAL_LINK              = 0x02,
    DISABLE_EMBEDDED_SCRIPTS   = 0x04,
    DISABLE_DOCUMENT_RECOVERY  = 0x08,
};
namespace o3tl
{
    template<> struct typed_flags<SfxModelFlags> : is_typed_flags<SfxModelFlags, 0x0f> {};
}

namespace sfx2
{
    /**
     * Intended to be called from UNO constructor functions
     * This evaluates certain creation arguments (passed to createInstanceWithArguments)
     * and passes them to the factory function of the derived class.
     */
    css::uno::Reference<css::uno::XInterface>
        SFX2_DLLPUBLIC createSfxModelInstance(
            const css::uno::Sequence<css::uno::Any> & rxArgs,
            std::function<css::uno::Reference<css::uno::XInterface>( SfxModelFlags )> creationFunc
        );

} // namespace sfx2


#endif // INCLUDED_SFX2_SFXMODELFACTORY_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
