/* -*- Mode: C++; tab-width: 4; indent-tabs-mode: nil; c-basic-offset: 4 -*- */
/*
 * This file is part of the LibreOffice project.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
package org.libreoffice.impressremote;

import org.libreoffice.impressremote.communication.CommunicationService;

import android.content.Context;
import android.os.Handler;
import android.text.format.DateFormat;
import android.view.Menu;

/**
 * Used to manage the action bar whenever a presentation is running.
 *
 */
public class ActionBarManager {

	private Context mContext;
	private Menu mMenu;
	private CommunicationService mCommunicationService;

	/*
	 * True if the timer is being used as a timer, false if we are showing a
	 * clock.
	 */
	private boolean mTimerOn = false;

	public ActionBarManager(Context aContext, Menu aMenu,
	                CommunicationService aCommunicationService) {
		mContext = aContext;
		mMenu = aMenu;
		mCommunicationService = aCommunicationService;
		timerHandler.removeCallbacks(timerUpdateThread);
		timerHandler.postDelayed(timerUpdateThread, 50);
	}

	private Handler timerHandler = new Handler();

	private Thread timerUpdateThread = new Thread() {

		@Override
		public void run() {
			//			invalidateOptionsMenu();
			CharSequence aTimeString;
			long aTime = mCommunicationService.getSlideShow().getTimer()
			                .getTimeMillis();
			if (mTimerOn) {
				aTimeString = DateFormat.format(mContext.getResources()
				                .getString(R.string.actionbar_timerformat),
				                aTime);
			} else {
				aTimeString = DateFormat.format(mContext.getResources()
				                .getString(R.string.actionbar_timeformat),
				                System.currentTimeMillis());
			}
			// TODO: set the string
			timerHandler.postDelayed(this, 50);

		}

	};
}
/* vim:set shiftwidth=4 softtabstop=4 expandtab: */