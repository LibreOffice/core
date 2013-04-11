/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
function Communicator( aServerAddress ) {

    var mReceiveBuffer = "";
    var mCurrentMessage = [];
    var mSocket;

    // PUBLIC
    this.sendMessage = function( aMessage ) {
        mSocket.send( aMessage );
    }

    // PRIVATE
    function processMessage( aMessage ) {
        console.log( "Received message " + aMessage );
    }

    function dataReceived( aEvent ) {
        mReceiveBuffer += aEvent.data;
        var i;
        while ( ( i = mReceiveBuffer.indexOf( '\n' ) ) != -1 ) {
            var aLine = mReceiveBuffer.substring( 0, i );
            mReceiveBuffer = mReceiveBuffer.substring( i+1 );
            if ( aLine.length > 0 ) {
                mCurrentMessage.push( aLine );
            } else {
                processMessage( mCurrentMessage );
                mCurrentMessage = [];
            }
            aLine = "";
        }
    }

    // CONSTRUCTOR
    if(  navigator.mozTCPSocket ) {
        mSocket = navigator.mozTCPSocket.open( "localhost", 1599 );
        mSocket.onopen = function( aEvent ) {
            console.log( "Received onopen" );
            mSocket.send( "LO_SERVER_CLIENT_PAIR\nFirefox OS\n1234\n\n" );
        }
        mSocket.onerror = function( aEvent ) {
            console.log( "Received error: " + aEvent.data );
        }
        mSocket.ondata = dataReceived;
    } else {
        console.log( "Can't access socket." );
    }

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */