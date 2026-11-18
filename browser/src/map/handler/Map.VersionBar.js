/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/*
 * window.L.Map.VersionBar.
 */
/* global app _ */

window.L.Map.VersionBar = window.L.Handler.extend({
	onAdd: function () {
		this._map.on('adminuser', this.onUpdateInfo, this);
		this._map.on('versionbar', this.onversionbar, this);
	},

	onRemove: function () {
		this._map.off('versionbar', this.onversionbar, this);
		this._map.off('adminuser', this.onUpdateInfo, this);
	},

	onUpdateInfo: function () {
		if (app.isAdminUser) {
			var laterDate = new Date();
			var currentDate = new Date();
			var timeValue = window.prefs.getNumber('InfoBarLaterDate');
			if (isNaN(timeValue)) {
				/* - 5 seconds */
				laterDate.setTime(currentDate.getTime() - 5000);
			} else {
				/* + 5 days (432,000,000 Milliseconds) */
				timeValue = Number(timeValue);
				laterDate.setTime(timeValue + 432000000);
			}

			if (currentDate > laterDate)
				app.socket.sendMessage('versionbar');
		}
	},

	onversionbar: function (e) {
		if (e && e.coolwsd_version) {
			var latestVersion = e.coolwsd_version.split('.');
			var currentVersion = app.socket.WSDServer.Version.split('.');
			var snackbarMessage = _('Your Collabora Online server needs updating. Version {0} is available.');
			var length = Math.max(latestVersion.length, currentVersion.length);
			for (var i = 0; i < length; i++) {
				var v1 = i < latestVersion.length ? parseInt(latestVersion[i]) : 0;
				var v2 = i < currentVersion.length ? parseInt(currentVersion[i]) : 0;

				if (v1 > v2) {
					var currentDate = new Date();
					window.prefs.set('InfoBarLaterDate', currentDate.getTime());
					snackbarMessage = snackbarMessage.replace('{0}', e.coolwsd_version);
					this._map.uiManager.showSnackbar(snackbarMessage);
					break;
				}
				if (v1 < v2) {
					break;
				}
			}
		}
	}
});

if (window.prefs.canPersist) {
	window.L.Map.versionBar = new window.L.Map.VersionBar();
}
