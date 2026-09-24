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
 * Socket to be intialized on opening the log page in Admin console
 */

/* global Admin AdminSocketBase */

var AdminSocketLog = AdminSocketBase.extend({
	_logLines: '',

	constructor: function(host) {
		this.base(host);
	},

	refreshLog: function() {
		this.socket.send('log_lines');
	},

	pullChannelList: function() {
		this.socket.send('channel_list');
	},

	sendChannelListLogLevels: function(e) {
		e.stopPropagation();

		// We change the colour of the button when we send the data and change it back when the task is done (in function applyChannelList). But it is happening too fast.
		document.getElementById('update-log-levels').classList.add('is-warning');
		document.getElementById('update-log-levels').classList.remove('is-info');

		// Get the form.
		var form = document.getElementById('form-channel-list');

		// Get channel select elements.
		var selectList = form.querySelectorAll('select');

		// Prepare the statement.
		var textToSend = 'update-log-levels';
		for (var i = 0; i < selectList.length; i++) {
			textToSend += ' ' + selectList[i].getAttribute('name').replace('channel-', '') + '=' + selectList[i].value;
		}

		this.socket.send(textToSend);
		document.getElementById('channel-list-modal').classList.remove('is-active');
	},

	onSocketOpen: function() {
		// Base class' onSocketOpen handles authentication
		this.base.call(this);

		document.getElementById('refresh-log').onclick = this.refreshLog.bind(this);
		document.getElementById('update-log-levels').onclick = this.sendChannelListLogLevels.bind(this);
		document.getElementById('log-channel-filter').onchange = this.applyChannelFilter.bind(this);

		this.pullChannelList();
		this.refreshLog();
	},

	applyChannelList: function(channelListStr) {
		var channelListArr = channelListStr.split(' '); // Every item holds: channel name + = + log level.

		// Here we have the log channel list and their respective log levels.
		// We will create items for them. User will be able to set the log level for each channel.
		var channelForm = document.getElementById('form-channel-list');
		channelForm.replaceChildren(); // Clear and refill it.
		var optionList = Array('none', 'fatal', 'critical', 'error', 'warning', 'notice', 'information', 'debug', 'trace');
		var innerHTML = ''; // Of select elements.
		for (var i = 0; i < optionList.length; i++) {
			innerHTML += '<option value="' + optionList[i] + '">' + optionList[i] + '</option>';
		}

		for (i = 0; i < channelListArr.length; i++) {
			if (channelListArr[i].split('=').length === 2) {
				var channelName = channelListArr[i].split('=')[0];
				var channelLogLevel = channelListArr[i].split('=')[1];

				var newDiv = document.createElement('div');
				newDiv.className = 'content';

				var newLabel = document.createElement('label');
				newLabel.className = 'label is-normal';
				newLabel.setAttribute('for', 'channel-' + channelName);
				newLabel.innerText = channelName;

				var newSubDivision = document.createElement('div');
				newSubDivision.className = 'select';

				var newSelectElement = document.createElement('select');
				newSelectElement.name = 'channel-' + channelName;
				newSelectElement.id = 'channel-' + channelName;
				// eslint-disable-next-line no-restricted-syntax -- option elements built from the fixed level names
				newSelectElement.innerHTML = innerHTML;
				newSelectElement.value = channelLogLevel;
				newSelectElement.style.width = '160px';
				newSelectElement.className = 'form-control';

				channelForm.appendChild(newDiv);
				newDiv.appendChild(newLabel);
				newDiv.appendChild(newSubDivision);
				newSubDivision.appendChild(newSelectElement);
			}
		}

		document.getElementById('update-log-levels').classList.remove('is-warning');
		document.getElementById('update-log-levels').classList.add('is-info');
	},

	applyChannelFilter: function() {
		if (document.getElementById('log-channel-filter').selectedIndex !== 0) {
			var filteredChannel = document.getElementById('log-channel-filter').value;
			var filteredLines = '';
			var lineList = this._logLines.split('\n');
			for (var i = 0; i < lineList.length; i++) {
				if (lineList[i].split('[').length > 1) {
					if (lineList[i].split('[')[1].split(']')[0].trim() === filteredChannel)
						filteredLines += lineList[i] + '\n';
				}
			}
			document.getElementById('log-lines').value = filteredLines;
		}
		else {
			document.getElementById('log-lines').value = this._logLines;
		}
	},

	refreshChannelFilter: function() {
		var lineList = this._logLines.split('\n');
		var channelList = new Array(0);
		for (var i = 0; i < lineList.length; i++) {
			if (lineList[i].trim() !== '' && lineList[i].split('[').length > 1) {
				var channelName = lineList[i].split('[')[1].split(']')[0].trim();
				if (!channelList.includes(channelName))
					channelList.push(channelName);
			}
		}

		var currentFilteredChannel = document.getElementById('log-channel-filter').value;

		// Remove previous channels.
		for (i = document.getElementById('log-channel-filter').options.length - 1; i > 0; i--)
			document.getElementById('log-channel-filter').remove(i);

		// Now add new ones.
		for (i = 0; i < channelList.length; i++) {
			var option = document.createElement('option');
			option.text = channelList[i];
			document.getElementById('log-channel-filter').add(option);
		}

		// Does previously selected channel name still exist? If not, we will re-set filter.
		if (!channelList.includes(currentFilteredChannel))
			document.getElementById('log-channel-filter').selectedIndex = 0;
		else
			document.getElementById('log-channel-filter').selectedIndex = channelList.findIndex(function(item) {return item === currentFilteredChannel;}) + 1;

		// Now, we will apply filter.
		this.applyChannelFilter();
	},

	onSocketMessage: function(e) {
		if (e.data.startsWith('log_lines')) {
			var result = e.data;
			result = result.substring(10, result.length);
			this._logLines = result;
			this.refreshChannelFilter();
		}
		else if (e.data.startsWith('channel_list')) {
			var channelListStr = e.data.substring(13, e.data.length);
			this.applyChannelList(channelListStr);
		}
	},

	onSocketClose: function() {
		this.base.call(this);
	}
});

Admin.Log = function(host) {
	return new AdminSocketLog(host);
};
