/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote.communication;

import java.io.IOException;
import java.net.Socket;
import java.net.UnknownHostException;

import android.os.StrictMode;

/**
 * Standard Network client. Connects to a server using Sockets.
 *
 * @author Andrzej J.R. Hunt
 */
public class NetworkClient extends Client {

	private static final int PORT = 1599;

	private Socket mSocket;

	public NetworkClient(String ipAddress) {
		// FIXME: eventually networking will be fully threaded.
		StrictMode.ThreadPolicy policy = new StrictMode.ThreadPolicy.Builder()
		                .permitAll().build();
		StrictMode.setThreadPolicy(policy);
		try {
			mSocket = new Socket(ipAddress, PORT);
			mInputStream = mSocket.getInputStream();
			mOutputStream = mSocket.getOutputStream();
			startListening();
		} catch (UnknownHostException e) {
			// TODO Tell the user we have a problem
			e.printStackTrace();
		} catch (IOException e) {
			// TODO As above
			e.printStackTrace();
		}

	}

	@Override
	public void closeConnection() {
		try {
			if (mSocket != null)
				mSocket.close();
		} catch (IOException e) {
			// TODO Auto-generated catch block
			e.printStackTrace();
		}
	}

}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */