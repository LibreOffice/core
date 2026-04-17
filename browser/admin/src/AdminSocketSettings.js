/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */
/*
 * Socket to be intialized on opening the settings page in Admin console
 */

/* global DlgYesNo $ AdminSocketBase Admin _ */

var AdminSocketSettings = AdminSocketBase.extend({
	constructor: function(host) {
		this.base(host);
		this._init();
	},

	_init: function() {
		var socketSettings = this.socket;
		$(document).ready(function() {
			$('#admin_settings').on('submit', function(e) {
				e.preventDefault();
				var memStatsSize = $('#mem_stats_size').val();
				var memStatsInterval = $('#mem_stats_interval').val();
				var cpuStatsSize = $('#cpu_stats_size').val();
				var cpuStatsInterval = $('#cpu_stats_interval').val();
				var command = 'set';
				command += ' mem_stats_size=' + memStatsSize;
				command += ' mem_stats_interval=' + memStatsInterval;
				command += ' cpu_stats_size=' + cpuStatsSize;
				command += ' cpu_stats_interval=' + cpuStatsInterval;
				command += ' limit_virt_mem_mb=' + $('#limit_virt_mem_mb').val();
				command += ' limit_stack_mem_kb=' + $('#limit_stack_mem_kb').val();
				command += ' limit_file_size_mb=' + $('#limit_file_size_mb').val();
				socketSettings.send(command);
			});

			document.getElementById('btnShutdown').onclick = function() {
				var dialog = (new DlgYesNo())
					.title(_('Confirmation'))
					.text(_('Are you sure you want to shut down the server?'))
					.yesButtonText(_('OK'))
					.noButtonText(_('Cancel'))
					.type('warning')
					.yesFunction(function() {
						socketSettings.send('shutdown maintenance');
					});
				dialog.open();
			};
		});
	},

	onSocketOpen: function() {
		// Base class' onSocketOpen handles authentication
		this.base.call(this);
		this.socket.send('subscribe settings');
		this.socket.send('settings');
		this.socket.send('version');
	},

	onSocketMessage: function(e) {
		var textMsg;
		if (typeof e.data === 'string') {
			textMsg = e.data;
		}
		else {
			textMsg = '';
		}

		if (textMsg.startsWith('settings')) {
			textMsg = textMsg.substring('settings '.length);
			var settings = textMsg.split(' ');
			for (var i = 0; i < settings.length; i++) {
				var setting = settings[i].split('=');
				var settingKey = setting[0];
				var settingVal = setting[1];
				var elem = document.getElementById(settingKey);
				if (elem) {
					elem.value = settingVal;
				}
			}
		}
		else if (textMsg.startsWith('coolserver ')) {
			// This must be the first message, unless we reconnect.
			var coolwsdVersionObj = JSON.parse(textMsg.substring(textMsg.indexOf('{')));
			var h = coolwsdVersionObj.Hash;
			if (parseInt(h,16).toString(16) === h.toLowerCase().replace(/^0+/, '')) {
				h = '<a target="_blank" href="https://gerrit.collaboraoffice.com/plugins/gitiles/online/+log/' + h + '">' + h + '</a>';
				$('#coolwsd-version').html(coolwsdVersionObj.Version + ' (git hash: ' + h + ')');
			}
			else {
				$('#coolwsd-version').text(coolwsdVersionObj.Version);
			}
			let buildConfig = coolwsdVersionObj.BuildConfig;
			if (coolwsdVersionObj.PocoVersion !== undefined) {
				buildConfig += ' (poco version: ' + coolwsdVersionObj.PocoVersion + ')';
			}
			$('#coolwsd-buildconfig').html(buildConfig);
		}
		else if (textMsg.startsWith('lokitversion ')) {
			var lokitVersionObj = JSON.parse(textMsg.substring(textMsg.indexOf('{')));
			$('#lokit-buildconfig').html(lokitVersionObj.BuildConfig);
		}
	},

	onSocketClose: function() {
		clearInterval(this._basicStatsIntervalId);
		this.base.call(this);
	}
});

Admin.Settings = function(host) {
	return new AdminSocketSettings(host);
};
