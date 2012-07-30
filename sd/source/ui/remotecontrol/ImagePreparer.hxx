/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _SD_IMPRESSREMOTE_IMAGEPREPARER_HXX
#define _SD_IMPRESSREMOTE_IMAGEPREPARER_HXX

#include <salhelper/thread.hxx>
#include <rtl/ref.hxx>

#include <com/sun/star/presentation/XSlideShowController.hpp>

#include "Transmitter.hxx"

namespace css = ::com::sun::star;

namespace sd
{

class ImagePreparer:
    public salhelper::Thread
{
public:
    ImagePreparer( const
        css::uno::Reference<css::presentation::XSlideShowController>&
        rxController, sd::Transmitter *aTransmitter,
        rtl::Reference<ImagePreparer>& rRef );
    ~ImagePreparer();

private:
    css::uno::Reference<css::presentation::XSlideShowController> xController;
    Transmitter *pTransmitter;
    rtl::Reference<ImagePreparer>& mRef;

    // Thread method
    void execute();

    void sendPreview( sal_uInt32 aSlideNumber );
    css::uno::Sequence<sal_Int8> preparePreview( sal_uInt32 aSlideNumber,
                sal_uInt32 aWidth, sal_uInt32 aHeight, sal_uInt64 &rSize );

    void sendNotes( sal_uInt32 aSlideNumber );
    rtl::OString prepareNotes( sal_uInt32 aSlideNumber );
    rtl::OString notesToHtml( sal_uInt32 aSlideNumber );
};

}


#endif // _SD_IMPRESSREMOTE_IMAGEPREPARER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
