/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
function Transmitter( aClient ) {

    var mClient = aClient;

    this.nextTransition = function() {
        mClient.sendMessage( "transition_next\n\n" );
    }

    this.previousTransition = function() {
        mClient.sendMessage( "transition_previous\n\n" );
    }

    this.gotoSlide = function( aSlide ) {
        mClient.sendMessage( "goto_slide\n" + aSlide + "\n\n" );
    }

    this.blankScreen = function() {
        mClient.sendMessage( "presentation_blank_screen\n\n" );
    }

    this.blankScreen = function( aColor ) {
        mClient.sendMessage( "presentation_blank_screen\n" + aColor + "\n\n" );
    }

    this.resume = function() {
        mClient.sendMessage( "presentation_resume\n\n" );
    }

    this.startPresentation = function() {
        mClient.sendMessage( "presentation_start\n\n" );
    }

    this.stopPresentation = function() {
        mClient.sendMessage( "presentation_stop\n\n" );
    }
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */