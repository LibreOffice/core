/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
#ifndef INCLUDED_SD_SOURCE_UI_REMOTECONTROL_RECEIVER_HXX
#define INCLUDED_SD_SOURCE_UI_REMOTECONTROL_RECEIVER_HXX

#include <rtl/string.hxx>
#include <vcl/timer.hxx>

#include <vector>
#include <deque>

namespace sd { class Transmitter; }

namespace sd
{

// Timer is protected by the solar mutex => so are we.
class Receiver : private Timer
{
    std::deque< std::vector< OString > > maExecQueue;
public:
    explicit Receiver( Transmitter *aTransmitter );
    virtual ~Receiver() override;
    virtual void Invoke() override;
    void pushCommand( const std::vector<OString> &rCommand );
    static void executeCommand( const std::vector<OString> &aCommand );

private:
    Transmitter *pTransmitter;
};

}
#endif // INCLUDED_SD_SOURCE_UI_REMOTECONTROL_RECEIVER_HXX
