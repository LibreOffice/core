/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * Copyright the LibreOffice contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <com/sun/star/presentation/XSoundReference.hpp>
#include <com/sun/star/uno/Any.hxx>
#include <cppuhelper/implbase.hxx>
#include <rtl/ustring.hxx>
#include <utility>

namespace xmloff
{
/** A presentation sound carried across the UNO boundary as an object: the
    source URL together with the runtime allowed state. Allowed is decided this
    session and is never written to the document.

    Header-only and placed under the shared include tree so it can be built
    from the layers below the draw model as well.
*/
class SoundReference final : public cppu::WeakImplHelper<css::presentation::XSoundReference>
{
    OUString maURL;
    bool mbAllowed;

public:
    SoundReference(OUString aURL, bool bAllowed)
        : maURL(std::move(aURL))
        , mbAllowed(bAllowed)
    {
    }

    OUString SAL_CALL getURL() override { return maURL; }
    sal_Bool SAL_CALL getAllowed() override { return mbAllowed; }
};

// A sound URL that points inside the document package, the same scheme the
// slideshow sound player treats as in-package media. Any other non-empty URL is
// a link to a file outside the document.
inline bool isPackageSoundURL(const OUString& rURL)
{
    return rURL.startsWithIgnoreAsciiCase(u"vnd.sun.star.Package:");
}

// Wrap a sound URL as a source value for an animation audio node. A sound inside
// the document package is always allowed; only an external link waits for the
// user to allow it.
inline css::uno::Any makeSoundSource(const OUString& rURL, bool bAllowed = false)
{
    return css::uno::Any(css::uno::Reference<css::presentation::XSoundReference>(
        new SoundReference(rURL, bAllowed || isPackageSoundURL(rURL))));
}

// The source URL of an animation audio node, or empty when the source is not a
// sound (for example a media shape). A bare string URL, the way a source was set
// before it carried the allowed state, is returned as it is.
inline OUString getSoundURL(const css::uno::Any& rSource)
{
    css::uno::Reference<css::presentation::XSoundReference> xSound;
    if (rSource >>= xSound)
        return xSound.is() ? xSound->getURL() : OUString();
    OUString sURL;
    if (rSource >>= sURL)
        return sURL;
    return OUString();
}

// Whether the sound carried by rSource has been allowed this session. Only a
// SoundReference, set where the sound enters the model, carries that decision.
// Anything else (a bare string set the old way, or a source that is not a sound)
// is treated as not allowed, so an unaudited sound is guarded like an external one.
inline bool getSoundAllowed(const css::uno::Any& rSource)
{
    css::uno::Reference<css::presentation::XSoundReference> xSound;
    if (rSource >>= xSound)
        return xSound.is() && xSound->getAllowed();
    return false;
}
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
