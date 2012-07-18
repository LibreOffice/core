/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef _SD_IMPRESSREMOTE_RECEIVER_HXX
#define _SD_IMPRESSREMOTE_RECEIVER_HXX

#include <com/sun/star/presentation/XSlideShowListener.hpp>
#include <com/sun/star/presentation/XSlideShowController.hpp>
#include <com/sun/star/presentation/XPresentationSupplier.hpp>
#include <com/sun/star/presentation/XPresentation.hpp>
#include <com/sun/star/presentation/XPresentation2.hpp>
#include <osl/socket.hxx>
#include <stdlib.h>

#include <vector>

#include "Transmitter.hxx"

namespace css = ::com::sun::star;

namespace sd
{

class Receiver
{
public:
    Receiver( Transmitter *aTransmitter );
    ~Receiver();
    void parseCommand( std::vector<rtl::OString> aCommand );

private:
    Transmitter *pTransmitter;
};

}

css::uno::Sequence<sal_Int8> preparePreview(sal_uInt32 aSlideNumber, const css::uno::Reference<css::presentation::XSlideShowController>& xSlideShowController, sal_uInt32 aWidth, sal_uInt32 aHeight, sal_uInt64 &aSize );

void sendPreview(sal_uInt32 aSlideNumber, const css::uno::Reference<
        css::presentation::XSlideShowController>& xSlideShowController, sd::Transmitter *aTransmitter );

#endif // _SD_IMPRESSREMOTE_RECEIVER_HXX
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
