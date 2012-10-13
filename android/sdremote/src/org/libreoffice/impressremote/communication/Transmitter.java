/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import android.graphics.Color;

/**
 * Interface to send commands to the server.
 */
public class Transmitter {

	private Client mClient;

	public Transmitter(Client aClient) {
		mClient = aClient;
	}

	public void nextTransition() {
		mClient.sendCommand("transition_next\n\n");
	}

	public void previousTransition() {

		mClient.sendCommand("transition_previous\n\n");
	}

	public void gotoSlide(int slide) {
		mClient.sendCommand("goto_slide\n" + slide + "\n\n");
	}

	/**
	 * Blank the screen to the default colour (set server-side), which is
	 * generally black. This is slightly faster than using
	 * <code> blankScreen( colour ) </code>.
	 */
	public void blankScreen() {
		mClient.sendCommand("presentation_blank_screen\n\n");
	}

	/**
	 * Set the screen to a specific colour. Only use if a non default colour is
	 * needed.
	 *
	 * @param aColor
	 */
	public void blankScreen(Color aColor) {
		// FIXME: check how to get colour in integer form.
		mClient.sendCommand("presentation_blank_screen\n" + aColor + "\n\n");
	}

	public void resume() {
		mClient.sendCommand("presentation_resume\n\n");
	}

	public void startPresentation() {
		mClient.sendCommand("presentation_start\n\n");
	}

	public void stopPresentation() {
		mClient.sendCommand("presentation_stop\n\n");
	}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */
