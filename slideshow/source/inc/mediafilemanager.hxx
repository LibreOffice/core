/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#ifndef INCLUDED_SLIDESHOW_SOURCE_INC_MEDIAFILEMANAGER_HXX
#define INCLUDED_SLIDESHOW_SOURCE_INC_MEDIAFILEMANAGER_HXX

#include <com/sun/star/uno/Reference.hxx>
#include <memory>

namespace avmedia
{
struct MediaTempFile;
}

namespace slideshow
{
namespace internal
{
class MediaFileManager
{
public:
    virtual ~MediaFileManager(){};
    virtual std::shared_ptr<avmedia::MediaTempFile> getMediaTempFile(const OUString& aUrl) = 0;
};
}
}
#endif // INCLUDED_SLIDESHOW_SOURCE_INC_MEDIAFILEMANAGER_HXX

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
