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

#include <rtl/ustring.hxx>
#include <utility>
#include <xmloff/SoundReference.hxx>

/** A sound source referenced by a presentation: the slide-transition
    sound, an animation sound, or a click-action sound.

    Holds the source URL and tells apart a sound that lives inside the
    document package from one that points at an external file. A
    package sound uses the vnd.sun.star.Package: scheme, the same prefix
    the slideshow sound player treats as in-package media. Any other
    non-empty URL is a link to a file outside the document.
*/
class SdSoundLink
{
    OUString maURL;
    // Runtime allowed state: the user has allowed this external sound this
    // session. In-memory only, never written to the document.
    bool mbAllowed;

public:
    SdSoundLink()
        : mbAllowed(false)
    {
    }
    explicit SdSoundLink(OUString aURL)
        : maURL(std::move(aURL))
        , mbAllowed(false)
    {
    }
    SdSoundLink(OUString aURL, bool bAllowed)
        : maURL(std::move(aURL))
        , mbAllowed(bAllowed)
    {
    }

    const OUString& getURL() const { return maURL; }
    bool isEmpty() const { return maURL.isEmpty(); }

    // A sound stored inside the document package.
    bool isEmbedded() const { return xmloff::isPackageSoundURL(maURL); }

    // A link to a file outside the document package.
    bool isExternalLink() const { return !isEmpty() && !isEmbedded(); }

    // True when both refer to the same sound source. The runtime allowed state
    // is session state, not part of what the source is, so it is not compared.
    bool sameLink(const SdSoundLink& rOther) const { return maURL == rOther.maURL; }

    void setAllowed(bool bAllowed) { mbAllowed = bAllowed; }

    // True when this sound may be fetched: a sound inside the package is always
    // allowed, an external link needs the user to have allowed it this session.
    bool isAllowed() const { return !isExternalLink() || mbAllowed; }
};

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
