/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 *
 */

#include <vcl/ImageTree.hxx>
#include <implimagetree.hxx>
#include <com/sun/star/uno/Reference.hxx>
#include <com/sun/star/container/XNameAccess.hpp>

ImageTree & ImageTree::get() {
    static ImageTree s_ImageTree;
    return s_ImageTree;
}

ImageTree::ImageTree()
    : mpImplImageTree(new ImplImageTree)
{
}

OUString ImageTree::getImageUrl(OUString const & rName, OUString const & rStyle, OUString const & rLang)

{
    return mpImplImageTree->getImageUrl(rName, rStyle, rLang);
}

std::shared_ptr<SvMemoryStream> ImageTree::getImageStream(OUString const & rName, OUString const & rStyle, OUString const & rLang)
{
    return mpImplImageTree->getImageStream(rName, rStyle, rLang);
}

bool ImageTree::loadImage(OUString const & rName, OUString const & rStyle,
                          BitmapEx & rBitmap, bool bLocalized,
                          sal_Int32 nScalePercentage,
                          const ImageLoadFlags eFlags)
{
    return mpImplImageTree->loadImage(rName, rStyle, rBitmap, bLocalized, eFlags, nScalePercentage);
}

bool ImageTree::loadImage(OUString const & rName, OUString const & rStyle,
                          BitmapEx & rBitmap, bool bLocalized,
                          const ImageLoadFlags eFlags)
{
    return loadImage(rName, rStyle, rBitmap, bLocalized, -1, eFlags);
}

css::uno::Reference<css::container::XNameAccess> const & ImageTree::getNameAccess()
{
    return mpImplImageTree->getNameAccess();
}

void ImageTree::shutdown()
{
    mpImplImageTree->shutdown();
}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
