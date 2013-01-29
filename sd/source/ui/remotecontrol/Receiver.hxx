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
#include <vcl/timer.hxx>
#include <vcl/svapp.hxx>

#include <vector>

#include "Transmitter.hxx"

namespace sd
{

// Timer is protected by the solar mutex => so are we.
class Receiver : Timer
{
    std::deque< std::vector< rtl::OString > > maExecQueue;
public:
    Receiver( Transmitter *aTransmitter );
    ~Receiver();
    virtual void Timeout();
    void pushCommand( const std::vector<rtl::OString> &rCommand );
    void executeCommand( const std::vector<rtl::OString> &aCommand );

private:
    Transmitter *pTransmitter;
};

}
#endif // _SD_IMPRESSREMOTE_RECEIVER_HXX
