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
 * Socket to be intialized on opening the overview page in Admin console
 */

/* global DlgYesNo _ $ Util AdminSocketBase Admin */

function getCollapsibleClass(id) {
	var container = document.getElementById(id);
	var label = container.children[0];
	var checkBox = container.children[1];
	var list = container.children[2];
	return {
		'addItem': function(itemId, text) {
			var listItem = document.createElement('li');
			listItem.id = itemId;
			listItem.innerText = text;
			list.appendChild(listItem);
		},
		'toggle': function() {
			checkBox.checked = !checkBox.checked;
		},
		'expand': function() {
			checkBox.checked = true;
		},
		'collapse': function() {
			checkBox.checked = false;
		},
		'setText': function(text) {
			label.innerText = text;
		},
		'getText': function() {
			return label.innerText;
		},
		'checkbox': checkBox,
		'label': label,
		'list': list
	};
}

// Creates collapsable section with its elements. Requires mcollapsable CSS class. Once created, collapsable element runs without javascript.
function createCollapsable(parentNode, id, text) {
	var div  = document.createElement('div'); // One div to hold them all.
	div.id = id;
	// Let's make some magic with CSS.
	// This is our checkbox, but it looks like a label.
	var checkBox = document.createElement('input');
	checkBox.type = 'checkbox';
	checkBox.className = 'title is-4 mcollapsable'; // Class names come from Bulma.css (except for mcollapsable). We use that library for Admin console.
	checkBox.checked = false;
	checkBox.style.visibility = 'hidden';
	checkBox.id = id + 'check';

	var label = document.createElement('label');
	label.innerText = text;
	label.className = 'field-label is-5';
	label.setAttribute('for', id + 'check');
	label.style.cursor = 'pointer';
	label.style.textDecoration = 'underline';

	var list = document.createElement('ul');

	div.appendChild(label);
	div.appendChild(checkBox);
	div.appendChild(list);

	parentNode.appendChild(div);
	return getCollapsibleClass(id);
}

// This function takes the list of the users viewing a specific document. Creates an HTML element holding the list.
function createDocumentUserListElement(cell, doc) {
	var collapsable = createCollapsable(cell, 'ucontainer' + doc['pid'], String(doc['views'].length) + _(' user(s).'));
	for (var i = 0; i < doc['views'].length; i++) {
		collapsable.addItem('user' + doc['views'][i]['sessionid'], doc['views'][i]['userName']);
	}
}

function upsertDocsTable(doc, sName, socket, wopiHost) {
	var add = false;
	var row = document.getElementById('doc' + doc['pid']);
	if (row === undefined || row === null) {
		row = document.createElement('tr');
		row.id = 'doc' + doc['pid'];
		document.getElementById('doclist').appendChild(row);
		add = true;
	}

	var sessionCloseCell = document.createElement('td'); // This cell will open "Do you want to kill this session?" dialog.
	sessionCloseCell.innerText = '✖';
	sessionCloseCell.title = _('Kill session.');
	sessionCloseCell.className = 'has-text-centered';
	sessionCloseCell.style.cursor = 'pointer';
	if (add === true) { row.appendChild(sessionCloseCell); } else { row.cells[0] = sessionCloseCell; }
	sessionCloseCell.onclick = function() {
		var dialog = (new DlgYesNo())
			.title(_('Confirmation'))
			.text(_('Are you sure you want to terminate this session?'))
			.yesButtonText(_('OK'))
			.noButtonText(_('Cancel'))
			.type('warning')
			.yesFunction(function() {
				socket.send('kill ' + doc['pid']);
			});
		dialog.open();
	};

	var wopiHostCell = document.createElement('td');
	wopiHostCell.innerText = wopiHost;
	if (add === true) { row.appendChild(wopiHostCell); } else { row.cells[0] = wopiHostCell; }
	wopiHostCell.className = 'has-text-left';

	if (add === true) {
		var userInfoCell = document.createElement('td');
		userInfoCell.className = 'has-text-left';
		if (add === true) { row.appendChild(userInfoCell); } else { row.cells[1] = userInfoCell; }
		createDocumentUserListElement(userInfoCell, doc);
	}
	else {
		var collapsable = getCollapsibleClass('ucontainer' + doc['pid']);
		collapsable.addItem('user' + doc['views'][0]['sessionid'], doc['views'][0]['userName']);
		collapsable.setText(String(parseInt(collapsable.getText().split(' ')[0]) + 1) + _(' user(s).'));
	}

	var pidCell = document.createElement('td');
	pidCell.innerText = doc['pid'];
	if (add === true) { row.appendChild(pidCell); } else { row.cells[0] = pidCell; }
	pidCell.className = 'has-text-centered';

	var nameCell = document.createElement('td');
	nameCell.innerText = sName;
	if (add === true) { row.appendChild(nameCell); } else { row.cells[0] = nameCell; }
	nameCell.className = 'has-text-left';

	var memoryCell = document.createElement('td');
	memoryCell.id = 'docmem' + doc['pid'];
	memoryCell.innerText = Util.humanizeMem(parseInt(doc['memory']));
	if (add === true) { row.appendChild(memoryCell); } else { row.cells[0] = memoryCell; }
	memoryCell.className = 'has-text-centered';

	var eTimeCell = document.createElement('td');
	eTimeCell.innerText = Util.humanizeSecs(doc['elapsedTime']);
	if (add === true) { row.appendChild(eTimeCell); } else { row.cells[0] = eTimeCell; }
	eTimeCell.className = 'has-text-centered';

	var idleCell = document.createElement('td');
	idleCell.id = 'docidle' + doc['pid'];
	idleCell.innerText = Util.humanizeSecs(doc['idleTime']);
	if (add === true) { row.appendChild(idleCell); } else { row.cells[0] = idleCell; }
	idleCell.className = 'has-text-centered';

	var isModifiedCell = document.createElement('td');
	isModifiedCell.id = 'mod' + doc['pid'];
	isModifiedCell.innerText = doc['modified'];
	if (add === true) { row.appendChild(isModifiedCell); } else { row.cells[0] = isModifiedCell; }
	isModifiedCell.className = 'has-text-centered';

	var isUploadedCell = document.createElement('td');
	isUploadedCell.id = 'up' + doc['pid'];
	isUploadedCell.innerText = doc['uploaded'];
	if (add === true) {
		row.appendChild(isUploadedCell);
	} else {
		row.cells[0] = isUploadedCell;
	}
	isUploadedCell.className = 'has-text-centered';

	// TODO: Is activeViews always the same with viewer count? We will hide this for now. If they are not same, this will be added to Users column like: 1/2 active/user(s).
	if (add === true) {
		var viewsCell = document.createElement('td');
		viewsCell.id = 'docview' + doc['pid'];
		viewsCell.innerText = doc['activeViews'];
		//row.appendChild(viewsCell);
	}
	else {
		//document.getElementById('docview' + doc['pid']).innerText = String(parseInt(document.getElementById('docview' + doc['pid'])) + 1);
	}
}

function upsertUsersTable(docPid, sName, userList, wopiHost) {
	for (var i = 0; i < userList.length; i++) {
		var encodedUId = encodeURI(userList[i]['userId']);
		var row = document.getElementById('usr' + encodedUId);
		var collapsable;
		if (row === undefined || row === null) {
			row = document.createElement('tr');
			row.id = 'usr' + encodedUId;
			document.getElementById('userlist').appendChild(row);

			var wopiHostCell = document.createElement('td');
			wopiHostCell.innerText = wopiHost;
			row.appendChild(wopiHostCell);

			var userNameCell = document.createElement('td');
			userNameCell.innerText = userList[i]['userName'];
			row.appendChild(userNameCell);

			var docInfoCell = document.createElement('td');
			row.appendChild(docInfoCell);
			collapsable = createCollapsable(docInfoCell, 'docListContainer_' + encodedUId, '1' + ' document(s) open.');
			collapsable.addItem(userList[i]['sessionid'] + '_' + docPid, sName);
		}
		else {
			collapsable = getCollapsibleClass('docListContainer_' + encodedUId);
			collapsable.setText(String(parseInt(collapsable.getText()) + 1) + _(' document(s) open.'));
			collapsable.addItem(userList[i]['sessionid'] + '_' + docPid, sName);
		}
	}
}

var AdminSocketOverview = AdminSocketBase.extend({
	constructor: function(host) {
		this.base(host);
	},

	_basicStatsIntervalId: 0,

	_docElapsedTimeIntervalId: 0,

	_getBasicStats: function() {
		this.socket.send('mem_consumed');
		this.socket.send('active_docs_count');
		this.socket.send('active_users_count');
		this.socket.send('sent_bytes');
		this.socket.send('recv_bytes');
		this.socket.send('uptime');
	},

	onSocketOpen: function() {
		// Base class' onSocketOpen handles authentication
		this.base.call(this);

		this.socket.send('documents');
		this.socket.send('subscribe adddoc rmdoc resetidle propchange modifications uploaded');

		this._getBasicStats();
		var socketOverview = this;
		this._basicStatsIntervalId =
		setInterval(function() {
			return socketOverview._getBasicStats();
		}, 5000);

		this._docElapsedTimeIntervalId =
		setInterval(function() {
			$('td.elapsed_time').each(function() {
				var newSecs = parseInt($(this).val()) + 1;
				$(this).val(newSecs);
				$(this).html(Util.humanizeSecs(newSecs));
			});
			$('td.idle_time').each(function() {
				var newSecs = parseInt($(this).val()) + 1;
				$(this).val(newSecs);
				$(this).html(Util.humanizeSecs(newSecs));
			});
		}, 1000);
	},

	onSocketMessage: function(e) {
		var textMsg;
		if (typeof e.data === 'string') {
			textMsg = e.data;
		}
		else {
			textMsg = '';
		}

		var $doc, $a;
		var nTotalViews;
		var docProps, sPid, sName;
		if (textMsg.startsWith('documents')) {
			var jsonStart = textMsg.indexOf('{');
			var docList = JSON.parse(textMsg.substr(jsonStart).trim())['documents'];

			for (var i = 0; i < docList.length; i++) {
				sName = decodeURI(docList[i]['fileName']);
				upsertUsersTable(docList[i]['pid'], sName, docList[i]['views'], docList[i]['wopiHost']);
				upsertDocsTable(docList[i], sName, this.socket, docList[i]['wopiHost']);
			}
		}
		else if (textMsg.startsWith('resetidle')) {
			textMsg = textMsg.substring('resetidle'.length);
			sPid = textMsg.trim().split(' ')[0];
			document.getElementById('docidle' + sPid).innerText = Util.humanizeSecs(0);
		}
		else if (textMsg.startsWith('adddoc')) {
			textMsg = textMsg.substring('adddoc'.length);
			docProps = textMsg.trim().split(' ');
			docProps = {
				'pid': docProps[0],
				'sName': decodeURI(docProps[1]),
				'sessionid': docProps[2],
				'userName': decodeURI(docProps[3]),
				'encodedUId': encodeURI(docProps[4]),
				'userId': docProps[4],
				'memory': docProps[5],
				'wopiHost': docProps[6],
				'elapsedTime': '0',
				'idleTime': '0',
				'modified': 'No',
				'uploaded': 'Yes',
				'views': [{ 'sessionid': docProps[2], 'userName': decodeURI(docProps[3]) }]
			};

			if (typeof docProps['wopiHost'] === 'undefined') {
				docProps['wopiHost'] = '';
			}
			upsertDocsTable(docProps, docProps['sName'], this.socket, docProps['wopiHost']);
			upsertUsersTable(docProps['pid'], docProps['sName'], [docProps], docProps['wopiHost']);
			document.getElementById('active_docs_count').innerText = String(parseInt(document.getElementById('active_docs_count').innerText) + 1);
			document.getElementById('active_users_count').innerText = String(parseInt(document.getElementById('active_users_count').innerText) + 1);
		}
		else if (textMsg.startsWith('mem_consumed') ||
			textMsg.startsWith('active_docs_count') ||
			textMsg.startsWith('active_users_count') ||
			textMsg.startsWith('sent_bytes') ||
			textMsg.startsWith('recv_bytes') ||
			textMsg.startsWith('uptime'))
		{
			textMsg = textMsg.split(' ');
			var sCommand = textMsg[0];
			var nData = parseInt(textMsg[1]);

			if (sCommand === 'mem_consumed' ||
			    sCommand === 'sent_bytes' ||
			    sCommand === 'recv_bytes') {
				nData = Util.humanizeMem(nData);
			}
			else if (sCommand === 'uptime') {
				nData = Util.humanizeSecs(nData);
			}
			$(document.getElementById(sCommand)).text(nData);
		}
		else if (textMsg.startsWith('rmdoc')) {
			textMsg = textMsg.substring('rmdoc'.length);
			docProps = textMsg.trim().split(' ');
			sPid = docProps[0];
			var sessionid = docProps[1];

			var doc = document.getElementById('doc' + sPid);
			if (doc !== undefined && doc !== null) {
				var $user = $(document.getElementById('user' + sessionid));
				$user.remove();
				var collapsable = getCollapsibleClass('ucontainer' + sPid);
				var viewerCount = parseInt(collapsable.getText().split(' ')[0]) - 1;
				if (viewerCount === 0) {
					document.getElementById('docview').deleteRow(doc.rowIndex);
				}
				else {
					collapsable.setText(String(viewerCount) + _(' user(s).'));
				}
				$a = $(document.getElementById('active_users_count'));
				nTotalViews = parseInt($a.text());
				$a.text(nTotalViews - 1);
			}

			var docEntry = document.getElementById(sessionid + '_' + sPid);
			if (docEntry !== null) {
				var docCount = docEntry.parentNode.children.length;
				var userDocListCell = docEntry.parentNode.parentNode.parentNode;
				if (docCount === 1) {
					document.getElementById('userview').deleteRow(userDocListCell.parentNode.rowIndex);
				}
				else {
					docEntry.remove();
					var userCollapsable = getCollapsibleClass(userDocListCell.children[0].id);
					userCollapsable.setText(String(docCount - 1) + _(' document(s) open.'));
				}
			}
		}
		else if (textMsg.startsWith('propchange')) {
			textMsg = textMsg.substring('propchange'.length);
			docProps = textMsg.trim().split(' ');
			sPid = docProps[0];
			var sProp = docProps[1];
			var sValue = docProps[2];

			$doc = $('#doc' + sPid);
			if ($doc.length !== 0) {
				if (sProp == 'mem') {
					var $mem = $('#docmem' + sPid);
					$mem.text(Util.humanizeMem(parseInt(sValue)));
				}
			}
		}
		else if (textMsg.startsWith('modifications')) {
			textMsg = textMsg.substring('modifications'.length);
			docProps = textMsg.trim().split(' ');
			sPid = docProps[0];
			var value = docProps[1];

			var $mod = $(document.getElementById('mod' + sPid));
			$mod.text(value);
		}
		else if (textMsg.startsWith('uploaded')) {
			textMsg = textMsg.substring('uploaded'.length);
			docProps = textMsg.trim().split(' ');
			if (docProps.length === 2) {
				sPid = docProps[0];
				var value = docProps[1];

				var up = $(document.getElementById('up' + sPid));
				if (up !== undefined) {
					up.text(value);
				}
			}
		}
		else if (e.data == 'InvalidAuthToken' || e.data == 'NotAuthenticated') {
			var msg;
			if (window.location.protocol === 'http:')
			{
				// Browsers refuse to overwrite the jwt cookie in this case.
				msg =  _('Failed to set jwt authentication cookie over insecure connection');
			}
			else
			{
				msg =  _('Failed to authenticate this session over protocol {0}');
				msg = msg.replace('{0}', window.location.protocol);
			}

			var dialog = (new DlgYesNo())
				.title(_('Warning'))
				.text(_(msg))
				.yesButtonText(_('OK'))
				.noButtonText(_('Cancel'))
				.type('warning');
			this.pageWillBeRefreshed = true;
			dialog.open();
		}
	},

	onSocketClose: function() {
		clearInterval(this._basicStatsIntervalId);
		clearInterval(this._docElapsedTimeIntervalId);
		this.base.call(this);
	}
});

Admin.Overview = function(host) {
	return new AdminSocketOverview(host);
};
