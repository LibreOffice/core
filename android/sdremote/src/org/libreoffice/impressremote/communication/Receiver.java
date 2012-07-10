/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import org.json.JSONException;
import org.json.JSONObject;

import android.os.Bundle;
import android.os.Message;
import android.os.Messenger;
import android.os.RemoteException;
import android.util.Base64;

public class Receiver {

	private Messenger mActivityMessenger;

	public void setActivityMessenger(Messenger aActivityMessenger) {
		mActivityMessenger = aActivityMessenger;
	}

	public void parseCommand(String aJSONCommandString) {
		if (mActivityMessenger == null) {
			return;
		}
		try {
			JSONObject aJSONCommand = new JSONObject(aJSONCommandString);
			String aInstruction = aJSONCommand.getString("command");
			if (aInstruction.equals("slide_updated")) {
				int aSlideNumber = aJSONCommand.getInt("slide_number");
				Message aMessage = Message.obtain(null,
						CommunicationService.MSG_SLIDE_CHANGED);
				Bundle aData = new Bundle();
				aData.putInt("slide_number", aSlideNumber);
				aMessage.setData(aData);
				try {
					mActivityMessenger.send(aMessage);
				} catch (RemoteException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			} else if (aInstruction.equals("slide_preview")) {
				int aSlideNumber = aJSONCommand.getInt("slide_number");
				String aImageString = aJSONCommand.getString("image");
				byte[] aImage = Base64.decode(aImageString, Base64.DEFAULT);
				Message aMessage = Message.obtain(null,
						CommunicationService.MSG_SLIDE_PREVIEW);
				Bundle aData = new Bundle();
				aData.putInt("slide_number", aSlideNumber);
				aData.putByteArray("preview_image", aImage);
				aMessage.setData(aData);
				try {
					mActivityMessenger.send(aMessage);
				} catch (RemoteException e) {
					// TODO Auto-generated catch block
					e.printStackTrace();
				}
			}

		} catch (JSONException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}

	}
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */