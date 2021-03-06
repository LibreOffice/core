/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

#pragma once

#include <vcl/image.hxx>
#include <o3tl/enumarray.hxx>

#include <com/sun/star/uno/Sequence.hxx>

#include "ImageList.hxx"

#include <memory>
#include <unordered_map>
#include <vector>

namespace vcl
{
class CommandImageResolver final
{
private:
    typedef std::unordered_map<OUString, OUString> CommandToImageNameMap;

    CommandToImageNameMap m_aCommandToImageNameMap;
    std::vector<OUString> m_aImageCommandNameVector;
    std::vector<OUString> m_aImageNameVector;

    o3tl::enumarray<ImageType, std::unique_ptr<ImageList>> m_pImageList;
    OUString m_sIconTheme;

    ImageList* getImageList(ImageType nImageType);

public:
    CommandImageResolver();
    ~CommandImageResolver();

    void registerCommands(css::uno::Sequence<OUString>& aCommandSequence);
    Image getImageFromCommandURL(ImageType nImageType, const OUString& rCommandURL);

    std::vector<OUString>& getCommandNames() { return m_aImageCommandNameVector; }

    bool hasImage(const OUString& rCommandURL);
};

} // end namespace vcl

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
