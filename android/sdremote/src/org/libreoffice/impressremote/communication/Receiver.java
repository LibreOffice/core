/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import java.util.ArrayList;

import android.os.Bundle;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Base64;

public class Receiver {

	private Messenger mActivityMessenger;

	private SlideShow mSlideShow = new SlideShow();

	public SlideShow getSlideShow() {
		return mSlideShow;
	}

	public void setActivityMessenger(Messenger aActivityMessenger) {
		mActivityMessenger = aActivityMessenger;
	}

	public void parseCommand(ArrayList<String> aCommand) {
		if (mActivityMessenger == null) {
			return;
		}
		String aInstruction = aCommand.get(0);
		if (aInstruction.equals("slideshow_started")) {
			int aSlideShowlength = Integer.parseInt(aCommand.get(1));
			int aCurrentSlide = Integer.parseInt(aCommand.get(2));
			mSlideShow.setLength(aSlideShowlength);
			mSlideShow.setCurrentSlide(aCurrentSlide);

			Message aMessage = Message.obtain(null,
			                CommunicationService.MSG_SLIDESHOW_STARTED);
			Bundle aData = new Bundle();
			aMessage.setData(aData);
			try {
				mActivityMessenger.send(aMessage);
			} catch (RemoteException e) {
				// Dead Handler -- i.e. Activity gone.
			}
		} else {
			if (mSlideShow == null)
				return;

			if (aInstruction.equals("slide_updated")) {

				int aSlideNumber = Integer.parseInt(aCommand.get(1));

				mSlideShow.setCurrentSlide(aSlideNumber);

				Message aMessage = Message.obtain(null,
				                CommunicationService.MSG_SLIDE_CHANGED);
				Bundle aData = new Bundle();
				aData.putInt("slide_number", aSlideNumber);
				aMessage.setData(aData);
				try {
					mActivityMessenger.send(aMessage);
				} catch (RemoteException e) {
					// Dead Handler -- i.e. Activity gone.
				}
			} else if (aInstruction.equals("slide_preview")) {
				int aSlideNumber = Integer.parseInt(aCommand.get(1));
				String aImageString = aCommand.get(2);
				byte[] aImage = Base64.decode(aImageString, Base64.DEFAULT);

				// Store image internally
				mSlideShow.putImage(aSlideNumber, aImage);

				// Notify the frontend
				Message aMessage = Message.obtain(null,
				                CommunicationService.MSG_SLIDE_PREVIEW);
				Bundle aData = new Bundle();
				aData.putInt("slide_number", aSlideNumber);
				aMessage.setData(aData);
				try {
					mActivityMessenger.send(aMessage);
				} catch (RemoteException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}

		}

	}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */