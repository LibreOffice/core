/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#pragma once

#include <vcl/timer.hxx>
#include <com/sun/star/uno/Reference.hxx>

namespace com::sun::star::presentation { class XSlideShowController; }
namespace sd { class Transmitter; }

namespace sd
{

class ImagePreparer : private Timer
{
    sal_uInt32 mnSendingSlide;
public:
    ImagePreparer( css::uno::Reference<css::presentation::XSlideShowController> xController,
                   sd::Transmitter *aTransmitter );
    virtual ~ImagePreparer() override;

private:
    css::uno::Reference<css::presentation::XSlideShowController> xController;
    Transmitter *pTransmitter;

    virtual void Invoke() override;

    void sendPreview( sal_uInt32 aSlideNumber );
    css::uno::Sequence<sal_Int8> preparePreview( sal_uInt32 aSlideNumber,
                sal_uInt32 aWidth, sal_uInt32 aHeight, sal_uInt64 &rSize );

    void sendNotes( sal_uInt32 aSlideNumber );
    OString prepareNotes( sal_uInt32 aSlideNumber );
};

}

/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
