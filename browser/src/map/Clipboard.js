/* -*- js-indent-level: 8; fill-column: 100 -*- */
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
 * window.L.Clipboard is used to abstract our storage and management of
 * local & remote clipboard data.
 */

/* global app DocUtil _ brandProductName $ ClipboardItem Promise GraphicSelection cool JSDialog */

// Get all interesting clipboard related events here, and handle
// download logic in one place ...
// We keep track of the current selection content if it is simple
// So we can do synchronous copy/paste in the callback if possible.
window.L.Clipboard = window.L.Class.extend({
	initialize: function(map) {
		this._map = map;
		this._selectionContent = '';
		this._selectionPlainTextContent = '';
		this._selectionType = null;
		this._accessKey = [ '', '' ];
		this._clipboardSerial = 0; // incremented on each operation
		this._failedTimer = null;
		this._unoCommandForCopyCutPaste = null;
		// Tracks if we're in paste special mode for the navigator.clipboard case
		this._navigatorClipboardPasteSpecial = false;
		// Is handling an 'Action_Copy' in progress?
		this._isActionCopy = false;

		var div = document.createElement('div');
		this._dummyDiv = div;
		this._dummyPlainDiv = null;
		this._dummyClipboard = {};

		// Tracks waiting for UNO commands to complete
		this._commandCompletion = [];
		this._map.on('commandresult', this._onCommandResult, this);
		this._map.on('clipboardchanged', this._onCommandResult, this);

		div.setAttribute('id', 'copy-paste-container');
		div.style.userSelect = 'text !important';
		div.style.opacity = '0';
		div.setAttribute('contenteditable', 'true');
		div.setAttribute('type', 'text');
		div.style.position = 'fixed';
		div.style.left = '0px';
		div.style.top = '-200px';
		div.style.width = '15000px';
		div.style.height = '200px';
		div.style.overflow = 'hidden';
		div.style.zIndex = '-1000';
		div.style['-webkit-user-select'] = 'text !important';
		div.style.display = 'block';
		div.style.fontSize = '6pt';

		// so we get events to where we want them.
		var parent = document.getElementById('map');
		parent.appendChild(div);

		if (window.L.Browser.cypressTest) {
			this._dummyPlainDiv = document.createElement('div');
			this._dummyPlainDiv.id = 'copy-plain-container';
			this._dummyPlainDiv.style = 'position: fixed; left: 0px; top: -400px; width: 15000px; height: 200px; ' +
				'overflow: hidden; z-index: -1000; -webkit-user-select: text !important; display: block; ' +
				'font-size: 6pt';
			parent.appendChild(this._dummyPlainDiv);
		}

		// sensible default content.
		this._resetDiv();

		var that = this;
		var beforeSelect = function(ev) { return that._beforeSelect(ev); };

		if (window.ThisIsTheWindowsApp)
		{
			// We can have very trivial implementations, native code does everything
			document.oncut = function(ev) {
				if (ev.srcElement['id'] === 'copy-paste-container')
					window.postMobileMessage('CUT');
			};
			document.oncopy = function(ev) {
				if (ev.srcElement['id'] === 'copy-paste-container')
					window.postMobileMessage('COPY');
			};
			document.onpaste = function(ev) {
				if (ev.srcElement['id'] === 'pre-space' || ev.srcElement['id'] === 'clipboard-area') {
					ev.preventDefault();
					if (ev.clipboardData.types.length == 1 && ev.clipboardData.types[0] === 'text/plain')
						window.postMobileMessage('PASTEUNFORMATTED');
					else
						window.postMobileMessage('PASTE');
				}
			};
		} else {
			document.oncut = function(ev)   { return that.cut(ev); };
			document.oncopy = function(ev)  { return that.copy(ev); };
			document.onpaste = function(ev) { return that.paste(ev); };
		}
		document.onbeforecut = beforeSelect;
		document.onbeforecopy = beforeSelect;
		document.onbeforepaste = beforeSelect;
	},

	// Decides if `html` effectively contains just an image.
	isHtmlImage: function(html) {
		const startsWithMeta = html.substring(0, 5) == '<meta';
		if (startsWithMeta) {
			// Ignore leading <meta>.
			const metaEnd = html.indexOf('>');
			if (metaEnd != -1) {
				// Start after '>'.
				html = html.substring(metaEnd + 1);
			}
		}

		// Starts with an <img> element.
		if (html.substring(0, 4) === '<img') {
			return true;
		}

		return false;
	},


	setKey: function(key) {
		if (this._accessKey[0] === key)
			return;
		this._accessKey[1] = this._accessKey[0];
		this._accessKey[0] = key;
	},

	getMetaBase: function() {
		if (window.ThisIsAMobileApp) {
			return 'collabora-online-mobile'; // makeHttpUrl does not work with the file:// protocol used in mobile apps...
		}
		return window.makeHttpUrl('');
	},

	getMetaPath: function(idx) {
		if (!idx)
			idx = 0;
		if (this._accessKey[idx] === '')
			return '';

		var metaPath = '/cool/clipboard?WOPISrc=' + encodeURIComponent(this._map.options.doc) +
			'&ServerId=' + app.socket.WSDServer.Id +
			'&ViewId=' + this._map._docLayer._viewId +
			'&Tag=' + this._accessKey[idx];

		if (window.routeToken !== '')
			metaPath += '&RouteToken='+window.routeToken;

		return metaPath;
	},

	getMetaURL: function(idx) {
		return this.getMetaBase() + this.getMetaPath(idx);
	},

	// Returns the marker used to identify stub messages.
	_getHtmlStubMarker: function() {
		return '<title>Stub HTML Message</title>';
	},

	// Returns true if the argument is a stub html.
	_isStubHtml: function(text) {
		return text.indexOf(this._getHtmlStubMarker()) > 0;
	},

	// wrap some content with our stub magic
	_originWrapBody: function(body, isStub) {
		var lang = 'en_US'; // FIXME: l10n
		var encodedOrigin = encodeURIComponent(this.getMetaURL());
		var text =  '<!DOCTYPE HTML PUBLIC "-//W3C//DTD HTML 4.0 Transitional//EN">\n' +
		            '<html>\n' +
		            '  <head>\n';
		if (isStub)
			text += '    ' + this._getHtmlStubMarker() + '\n';
		text +=     '    <meta http-equiv="content-type" content="text/html; charset=utf-8"/>\n' +
			    '  </head>\n' +
			    '  <body lang="' + lang + '" dir="ltr"><div id="meta-origin" data-coolorigin="' + encodedOrigin + '">\n' +
			    body +
			    '  </div></body>\n' +
			'</html>';
		return text;
	},

	// what an empty clipboard has on it
	_getStubHtml: function() {
		return this._substProductName(this._originWrapBody(
		    '    <p>' + _('To paste outside {productname}, please first click the \'download\' button') + '</p>\n',
		    true
		));
	},

	// used for DisableCopy mode to fill the clipboard
	_getDisabledCopyStubHtml: function() {
		return this._substProductName(this._originWrapBody(
		    '    <p>' + _('Copying from the document disabled') + '</p>\n',
		    true
		));
	},

	getMetaOrigin: function (html) {
		return this._getMetaOrigin(html, '<div id="meta-origin" data-coolorigin="');
	},

	_getMetaOrigin: function (html, prefix) {
		var start = html.indexOf(prefix);
		if (start < 0) {
			return '';
		}
		var end = html.indexOf('"', start + prefix.length);
		if (end < 0) {
			return '';
		}
		var meta = html.substring(start + prefix.length, end);

		// quick sanity checks that it one of ours.
		if (meta.indexOf('%2Fclipboard%3FWOPISrc%3D') >= 0 &&
		    meta.indexOf('%26ServerId%3D') > 0 &&
		    meta.indexOf('%26ViewId%3D') > 0 &&
		    meta.indexOf('%26Tag%3D') > 0)
			return decodeURIComponent(meta);
		else
			window.app.console.log('Mis-understood foreign origin: "' + meta + '"');
		return '';
	},

	_encodeHtmlToBlob: function(text) {
		var content = [];
		var data = new Blob([text]);
		content.push('text/html\n');
		content.push(data.size.toString(16) + '\n');
		content.push(data);
		content.push('\n');
		return new Blob(content);
	},

	_readContentSyncToBlob: function(dataTransfer) {
		var content = [];
		var types = dataTransfer.types;
		for (var t = 0; t < types.length; ++t) {
			if (types[t] === 'Files')
				continue; // images handled elsewhere.
			var dataStr = dataTransfer.getData(types[t]);
			// Avoid types that has no content.
			if (!dataStr.length)
				continue;
			var data = new Blob([dataStr]);
			window.app.console.log('type ' + types[t] + ' length ' + data.size +
				    ' -> 0x' + data.size.toString(16) + '\n');
			content.push((types[t] === 'text' ? 'text/plain' : types[t]) + '\n');
			content.push(data.size.toString(16) + '\n');
			content.push(data);
			content.push('\n');
		}
		if (content.length > 0)
			return new Blob(content, {type : 'application/octet-stream', endings: 'transparent'});
		else
			return null;
	},

	// Abstract async post & download for our progress wrappers
	// type: GET or POST
	// url:  where to get / send the data
	// optionalFormData: used for POST for form data
	// forClipboard: a boolean telling if we need the "Confirm copy to clipboard" link in the end
	// completeFn: called on completion - with response.
	// progressFn: allows splitting the progress bar up.
	_doAsyncDownload: async function(type,url,optionalFormData,forClipboard,progressFn,) {
		var request = new XMLHttpRequest();

		// avoid to invoke the following code if the download widget depends on user interaction
		if (!this._downloadProgress || this._downloadProgress.isClosed()) {
			this._startProgress(false);
			this._downloadProgress.startProgressMode();
		}

		return await new Promise((resolve, reject) => {
			request.onload = () => {
				this._downloadProgress._onComplete();
				if (!forClipboard) {
					this._downloadProgress._onClose();
				}

				// For some reason 400 error from the server doesn't
				// invoke onerror callback, but we do get here with
				// size==0, which signifies no response from the server.
				// So we check the status code instead.
				if (request.status == 200) {
					resolve(request.response);
				} else {
					reject(request.response);
				}
			};
			request.onerror = (error) => {
				reject(error);
				this._downloadProgress._onComplete();
				this._downloadProgress._onClose();
			};

			request.ontimeout = () => {
				this._map.uiManager.showSnackbar(_('warning: copy/paste request timed out'));
				this._downloadProgress._onClose();
				reject('request timed out');
			};

			request.upload.addEventListener('progress', (e) => {
				if (e.lengthComputable) {
					var percent = progressFn(e.loaded / e.total * 100);
					var progress = { statusType: 'setvalue', value: percent };
					this._downloadProgress._onUpdateProgress(progress);
				}
			}, false);

			if ('processCoolUrl' in window) {
				url = window.processCoolUrl({ url: url, type: 'clipboard' });
			}

			request.open(type, url, true /* isAsync */);
			request.timeout = 30 * 1000; // 30 secs ...
			request.responseType = 'blob';
			if (optionalFormData !== null)
				request.send(optionalFormData);
			else
				request.send();
		});
	},

	// Suck the data from one server to another asynchronously ...
	_dataTransferDownloadAndPasteAsync: async function(src, fallbackHtml) {
		// FIXME: add a timestamp in the links (?) ignore old / un-responsive servers (?)
		let response;
		const errorMessage = _('Failed to download clipboard, please re-copy');
		try {
			response = await this._doAsyncDownload(
				'GET', src, null, false,
				function(progress) { return progress/2; },
			);
		} catch (_error) {
			window.app.console.log('failed to download clipboard using fallback html');

			// If it's the stub, avoid pasting.
			if (this._isStubHtml(fallbackHtml))
			{
				// Let the user know they haven't really copied document content.
				window.app.console.error('Clipboard: failed to download - ' + errorMessage);
				this._map.uiManager.showInfoModal('data-transfer-warning', '', errorMessage);
				return;
			}

			var formData = new FormData();
			let commandName = null;
			if (this._checkAndDisablePasteSpecial()) {
				commandName = '.uno:PasteSpecial';
			} else {
				commandName = '.uno:Paste';
			}
			const data = JSON.stringify({
				url: src,
				commandName: commandName,
			});
			formData.append('data', new Blob([data]), 'clipboard');
			try {
				await this._doAsyncDownload(
					'POST', this.getMetaURL(), formData, false,
					function(progress) { return 50 + progress/2; },
				);
			} catch (_error) {
				await this.dataTransferToDocumentFallback(null, fallbackHtml);
			}
			return;
		}

		window.app.console.log('download done - response ' + response);
		var formData = new FormData();
		formData.append('data', response, 'clipboard');

		try {
			await this._doAsyncDownload(
				'POST', this.getMetaURL(), formData, false,
				function(progress) { return 50 + progress/2; }
			);

			if (this._checkAndDisablePasteSpecial()) {
				window.app.console.log('up-load done, now paste special');
				app.socket.sendMessage('uno .uno:PasteSpecial');
			} else {
				window.app.console.log('up-load done, now paste');
				app.socket.sendMessage('uno .uno:Paste');
			}
		} catch (_error) {
			window.app.console.error('Clipboard: failed to download - error');
			this._map.uiManager.showInfoModal('data-transfer-warning', '', errorMessage);
		}
	},

	_onImageLoadFunc: function (file) {
		var that = this;
		return function(e) {
			that._pasteTypedBlob(file.type, e.target.result);
		};
	},

	// Sends a paste event with the specified mime type and content
	_pasteTypedBlob: function(fileType, fileBlob) {
		var header = 'paste mimetype=' + fileType + '\n';
		var blob;
		if (window.ThisIsTheQtApp || window.ThisIsTheWindowsApp) {
			// To work around a qtwebchannel "Could not convert argument
			// QJsonValue(object, QJsonObject()) to target type QString ." bug, send the
			// payload as a base64-encoded string rather than as an ArrayBuffer blob
			// (and decode it in ChildSession::paste in kit/ChildSession.cpp):
			blob = header + window.btoa(
				Array.from(new Uint8Array(fileBlob), (b) => String.fromCodePoint(b))
				.join(''));
                } else {
			blob = new Blob([header, fileBlob]);
		}
		app.socket.sendMessage(blob);
	},

	_asyncReadPasteFile: function (file) {
		if (file.type.match(/image.*/)) {
			return this._asyncReadPasteImage(file);
		}
		if (file.type.match(/audio.*/) || file.type.match(/video.*/)) {
			return this._asyncReadPasteAVMedia(file);
		}
		return false;
	},

	_asyncReadPasteImage: function (file) {
		var reader = new FileReader();
		reader.onload = this._onImageLoadFunc(file);
		reader.readAsArrayBuffer(file);
		return true;
	},

	_asyncReadPasteAVMedia: function (file) {
		this._map.insertMultimedia(file);
		return true;
	},

	// Returns true if it finished synchronously, and false if it has started an async operation
	// that will likely end at a later time (required to avoid closing progress bar in paste(ev))
	// FIXME: This comment is a lie if dataTransferToDocumentFallback is called, as it calls _doAsyncDownload
	dataTransferToDocument: function (dataTransfer, preferInternal, htmlText, usePasteKeyEvent) {
		// Look for our HTML meta magic.
		//   cf. ClientSession.cpp /textselectioncontent:/

		var meta = this._getMetaOrigin(htmlText, '<div id="meta-origin" data-coolorigin="');
		var id = this.getMetaPath(0);
		var idOld = this.getMetaPath(1);

		// for the paste, we always prefer the internal COKit's copy/paste
		if (preferInternal === true &&
			((id !== '' && meta.indexOf(id) >= 0) || (idOld !== '' && meta.indexOf(idOld) >= 0)))
		{
			// Home from home: short-circuit internally.
			window.app.console.log('short-circuit, internal paste');
			this._doInternalPaste(this._map, usePasteKeyEvent);
			return true;
		}

		// Do we have a remote Online we can suck rich data from ?
		if (meta !== '')
		{
			window.app.console.log('Transfer between servers\n\t"' + meta + '" vs. \n\t"' + id + '"');
			this._dataTransferDownloadAndPasteAsync(meta, htmlText);
			return false; // just started async operation - did not finish yet
		}

		// Fallback.
		this.dataTransferToDocumentFallback(dataTransfer, htmlText, usePasteKeyEvent);
		return true;
	},

	_sendToInternalClipboard: async function (content) {
		if (window.ThisIsTheiOSApp || window.ThisIsTheMacOSApp) {
			await window.webkit.messageHandlers.clipboard.postMessage(`sendToInternal ${await content.text()}`); // no need to base64 in this direction...
		} else if (window.ThisIsTheWindowsApp) {
			await window.postMobileMessage(`CLIPBOARDSET ${await content.text()}`);
		} else {
			var formData = new FormData();
			formData.append('file', content);

			return await this._doAsyncDownload('POST', this.getMetaURL(), formData, false,
				function(progress) { return progress; }
			);
		}
	},

	dataTransferToDocumentFallback: async function(dataTransfer, htmlText, usePasteKeyEvent) {
		var content;
		if (dataTransfer) {
			// Suck HTML content out of dataTransfer now while it feels like working.
			content = this._readContentSyncToBlob(dataTransfer);
		}

		// Fallback on the html.
		if (!content && htmlText != '') {
			content = this._encodeHtmlToBlob(htmlText);
		}

		// FIXME: do we want this section ?

		// Images get a look in only if we have no content and are async (used in the Ctrl-V
		// case)
		if (((content == null && htmlText === '') || this.isHtmlImage(htmlText)) && dataTransfer != null)
		{
			var types = dataTransfer.types;

			window.app.console.log('Attempting to paste image(s)');

			// first try to transfer images
			// TODO if we have both Files and a normal mimetype, should we handle
			// both, or prefer one or the other?
			for (var t = 0; t < types.length; ++t) {
				window.app.console.log('\ttype' + types[t]);
				if (types[t] === 'Files') {
					var files = dataTransfer.files;
					if (files !== null)
					{
						for (var f = 0; f < files.length; ++f)
							this._asyncReadPasteFile(files[f]);
					} // IE / Edge
					else this._asyncReadPasteFile(dataTransfer.items[t].getAsFile());
				}
			}

			// If any paste special dialog is open, close it here, because we won't call
			// _doInternalPaste() that would do the closing.
			this._checkAndDisablePasteSpecial();

			return;
		}

		if (content == null) {
			window.app.console.log('Nothing we can paste on the clipboard');
			return;
		}

		window.app.console.log('Normal HTML, so smart paste not possible');

		await this._sendToInternalClipboard(content);

		window.app.console.log('clipboard: Sent ' + content.size + ' bytes successfully');

		this._doInternalPaste(this._map, usePasteKeyEvent);
	},

	_checkSelection: function() {
		var checkSelect = document.getSelection();
		if (checkSelect && checkSelect.isCollapsed)
			window.app.console.log('Error: collapsed selection - cannot copy/paste');
	},

	_getHtmlForClipboard: function() {
		var text;

		if (this._selectionType === 'complex' || GraphicSelection.hasActiveSelection()) {
			window.app.console.log('Copy/Cut with complex/graphical selection');
			if (this._selectionType === 'text' && this._selectionContent !== '')
			{ // back here again having downloaded it ...
				text = this._selectionContent; // Not sure if we hit these lines. Last else block seems to catch the downloaded content (selection type is not "complex" while copying to clipboard).
				window.app.console.log('Use downloaded selection.');
			}
			else
			{
				window.app.console.log('Downloaded that selection.');
				text = this._getStubHtml();
				this._onDownloadOnLargeCopyPaste();
				this._downloadProgress.setURI( // richer, bigger HTML ...
					this.getMetaURL() + '&MimeType=text/html,text/plain;charset=utf-8');
			}
		} else if (this._selectionType === null) {
			window.app.console.log('Copy/Cut with no selection!');
			text = this._getStubHtml();
		} else {
			window.app.console.log('Copy/Cut with simple text selection');
			text = this._selectionContent;
		}
		return text;
	},

	// returns whether we should stop processing the event
	populateClipboard: function(ev) {
		// If the copy paste API is not supported, we download the content as a fallback method.
		var text = this._getHtmlForClipboard();

		var plainText = DocUtil.stripHTML(text);
		if (text == this._selectionContent && this._selectionPlainTextContent != '') {
			plainText = this._selectionPlainTextContent;
		}
		if (ev.clipboardData) { // Standard
			if (this._unoCommandForCopyCutPaste === '.uno:CopyHyperlinkLocation') {
				var ess = 's';
				var re = new RegExp('^(.*)(<a href=")([^"]+)(">.*</a>)(</p>\n</body>\n</html>)$', ess);
				var match = re.exec(text);
				if (match !== null && match.length === 6) {
					text = match[1] + match[3] + match[5];
					plainText = DocUtil.stripHTML(text);
				}
			}
			// if copied content is graphical then plainText is null and it does not work on mobile.
			ev.clipboardData.setData('text/plain', plainText ? plainText: ' ');
			ev.clipboardData.setData('text/html', text);
			window.app.console.log('Put "' + text + '" on the clipboard');
			this._clipboardSerial++;
		}
	},

	_isAnyInputFieldSelected: function(forCopy = false) {
		if ($('#search-input').is(':focus'))
			return true;

		if ($('.ui-edit').is(':focus'))
			return true;

		if ($('.ui-textarea').is(':focus'))
			return true;

		if ($('input.ui-combobox-content').is(':focus'))
			return true;

		if (this._map.uiManager.isAnyDialogOpen()
			&& !this.isCopyPasteDialogReadyForCopy()
			&& !this.isPasteSpecialDialogOpen())
			return true;

		if (cool.Comment.isAnyFocus())
		    return true;

		if (forCopy) {
			let selection = window.getSelection();
			selection = selection && selection.toString();
			if (selection && selection.length !== 0)
				return true;
		}

		return false;
	},

	_isFormulabarSelected: function() {
		if ($('#sc_input_window').is(':focus'))
			return true;
		return false;
	},

	// Does the selection of text before an event comes in
	_beforeSelect: function(ev) {
		window.app.console.log('Got event ' + ev.type + ' setting up selection');

		if (this._isAnyInputFieldSelected(ev.type === 'beforecopy'))
			return;

		this._beforeSelectImpl();
	},

	_beforeSelectImpl: function() {
		if (this._selectionType === 'slide')
			return;

		// We need some spaces in there ...
		this._resetDiv();

		var sel = document.getSelection();
		if (!sel)
			return;

		var selected = false;
		var selectRange;

		if (!selected)
		{
			sel.removeAllRanges();
			selectRange = document.createRange();
			selectRange.selectNodeContents(this._dummyDiv);
			sel.addRange(selectRange);

			var checkSelect = document.getSelection();
			if (checkSelect.isCollapsed)
				window.app.console.log('Error: failed to select - cannot copy/paste');
		}

		return false;
	},

	_resetDiv: function() {
		// cleanup the content:
		this._dummyDiv.replaceChildren();

		let bElement = document.createElement('b');
		bElement.style.fontWeight = 'normal';
		bElement.style.backgroundColor = 'transparent';
		bElement.style.color = 'transparent';

		let span = document.createElement('span');
		span.textContent = '  ';

		bElement.appendChild(span);
		this._dummyDiv.appendChild(bElement);
	},

	// Try-harder fallbacks for emitting cut/copy/paste events.
	_execOnElement: function(operation) {
		var serial = this._clipboardSerial;

		this._resetDiv();

		var success = false;
		var active = null;

		// selection can change focus.
		active = document.activeElement;

		success = (document.execCommand(operation) &&
			   serial !== this._clipboardSerial);

		// try to restore focus if we need to.
		if (active !== null && active !== document.activeElement)
			active.focus();

		window.app.console.log('fallback ' + operation + ' ' + (success?'success':'fail'));

		return success;
	},

	// Encourage browser(s) to actually execute the command
	_execCopyCutPaste: function(operation, cmd, params) {
		var serial = this._clipboardSerial;

		this._unoCommandForCopyCutPaste = cmd;

		if (operation !== 'paste' && cmd !== undefined && this._navigatorClipboardWrite(params)) {
			// This is the codepath where an UNO command initiates the clipboard
			// operation.
			return;
		}

		if (!window.ThisIsTheiOSApp && // in mobile apps, we want to drop straight to navigatorClipboardRead as execCommand will require user interaction...
			!window.ThisIsTheMacOSApp &&
			!window.ThisIsTheWindowsApp &&
			!window.ThisIsTheQtApp &&
			document.execCommand(operation) &&
			serial !== this._clipboardSerial) {
			window.app.console.log('copied successfully');
			this._unoCommandForCopyCutPaste = null;
			return;
		}

		// execCommand('paste') may fire the paste event synchronously and
		// return false; in that case paste() has already dispatched a
		// .uno:Paste, so a second async navigator-clipboard read would
		// duplicate the content.
		if (operation == 'paste' && serial !== this._clipboardSerial) {
			this._unoCommandForCopyCutPaste = null;
			return;
		}

		if (operation == 'paste' && this._navigatorClipboardRead(false)) {
			// execCommand(paste) failed, the new clipboard API is available, tried that
			// way.
			return;
		}

		this._afterCopyCutPaste(operation);
	},

	_afterCopyCutPaste: function(operation) {
		var serial = this._clipboardSerial;
		this._unoCommandForCopyCutPaste = null;

		// try a hidden div
		if (this._execOnElement(operation)) {
			window.app.console.log('copied on element successfully');
			return;
		}

		// see if we have help for paste
		if (operation === 'paste')
		{
			try {
				window.app.console.warn('Asked parent for a paste event');
				this._map.fire('postMessage', {msgId: 'UI_Paste'});
			} catch (error) {
				window.app.console.warn('Failed to post-message: ' + error);
			}
		}

		// wait and see if we get some help
		var that = this;
		clearTimeout(this._failedTimer);
		setTimeout(function() {
			if (that._clipboardSerial !== serial)
			{
				window.app.console.log('successful ' + operation);
				if (operation === 'paste')
					that._stopHideDownload();
			}
			else
			{
				window.app.console.log('help did not arrive for ' + operation);
				that._warnCopyPaste();
			}
		}, 150 /* ms */);
	},

	// ClipboardContent.getType() callback: used with the Paste button
	_navigatorClipboardGetTypeCallback: async function(clipboardContent, blob, type) {
		if (type == 'image/png') {
			this._pasteTypedBlob(type, blob);
			return;
		}

		let text;
		try {
			text = await blob.text();
		} catch (error) {
			window.app.console.log('blob.text() failed: ' + error.message);
			return;
		}

		if (type !== 'text/html' || !this.isHtmlImage(text)) {
			this._navigatorClipboardTextCallback(text, type);
			return;
		}

		// Got an image, work with that directly.
		let image;
		try {
			image = await clipboardContent.getType('image/png');
		} catch (error) {
			window.app.console.log('clipboardContent.getType(image/png) failed: ' + error.message);
			return;
		}

		this._navigatorClipboardGetTypeCallback(clipboardContent, image, 'image/png');
	},

	// Clipboard blob text() callback for the text/html and text/plain cases
	_navigatorClipboardTextCallback: function(text, textType) {
		// paste() wants to work with a paste event, so construct one.
		var ev = {
			clipboardData: {
				// Used early by paste().
				getData: function(type) {
					if (type === textType) {
						return text;
					}

					return '';
				},
				// Used by _readContentSyncToBlob().
				types: [textType],
			},
			preventDefault: function() {
			},
		};

		// Invoke paste(), which knows how to recognize our HTML vs external HTML.
		this.paste(ev);
	},

	// Gets status of a copy/paste command from the remote Kit
    _onCommandResult: function(e) {
        if (e.commandName === '.uno:Copy' || e.commandName === '.uno:Cut' || e.commandName === '.uno:CopyHyperlinkLocation' || e.commandName === '.uno:CopySlide') {
			window.app.console.log('Resolve clipboard command promise ' + e.commandName
				+ ' with queue length: ' + this._commandCompletion.length);
			while (this._commandCompletion.length > 0)
			{
				let a = this._commandCompletion.shift();
				a.resolve();
			}
		}
	},

	_sendCommandAndWaitForCompletion: function(command, params) {
		if (command !== '.uno:Copy' && command !== '.uno:Cut' && command !== '.uno:CopyHyperlinkLocation' && command !== '.uno:CopySlide') {
			console.error(`_sendCommandAndWaitForCompletion was called with '${command}', but anything except Copy or Cut will never complete`);
			return null;
		}

		if (this._commandCompletion.length > 0) {
			console.warn('Already have ' + this._commandCompletion.length + ' pending clipboard command(s)');
			return null;
		}

		if (!params) app.socket.sendMessage('uno ' + command);
		else app.map.sendUnoCommand(command, params);

		return new Promise((resolve, reject) => {
			window.app.console.log('New ' + command + ' promise');
			// FIXME: add a timeout cleanup too ...
			this._commandCompletion.push({
				resolve: resolve,
				reject: reject,
			});
		});
	},

	_parseClipboardFetchResult: async function(text, mimetype, shorttype) {
		const content = this.parseClipboard(await text)[shorttype];
		const blob = new Blob([content], { 'type': mimetype });
		console.log('Generate blob of type ' + mimetype + ' from ' + shorttype + ' text: ' + content);
		return blob;
	},

	// Executes the navigator.clipboard.write() call, if it's available.
	_navigatorClipboardWrite: function(params) {
		if (!window.L.Browser.clipboardApiAvailable && !window.ThisIsTheiOSApp && !window.ThisIsTheMacOSApp && !window.ThisIsTheWindowsApp && !window.ThisIsTheQtApp) {
			return false;
		}

		if (this._selectionType !== 'text' && this._selectionType !== 'slide') {
			return false;
		}

		this._asyncAttemptNavigatorClipboardWrite(params);
		return true;
	},

	_asyncAttemptNavigatorClipboardWrite: async function(params) {
		const command = this._unoCommandForCopyCutPaste;

		if (window.ThisIsTheQtApp) {
			// Qt handles UNO command and clipboard sync only via COPY/CUT/COPYSLIDE messages.
			if (command === '.uno:Cut')
				window.postMobileMessage('CUT');
			else if (command === '.uno:CopySlide')
				window.postMobileMessage('COPYSLIDE');
			else
				window.postMobileMessage('COPY');
			return;
		}

		const check_ = this._sendCommandAndWaitForCompletion(command, params);

		// I strongly disrecommend awaiting before the clipboard.write line in the
		// non-iOS-app path

		// It turns out there are some rather precarious conditions for copy/paste to be
		// allowed in Safari on mobile - and awaiting seems to tip us over into "too late to
		// copy/paste"

		// Deferring like this is kinda horrible - it certainly looks gross in places - but
		// it's absolutely necessary to avoid errors on the clipboard.write line

		// I don't like it either :). If you change this make sure to thoroughly test
		// cross-browser and cross-device!

		if (window.ThisIsTheiOSApp || window.ThisIsTheMacOSApp) {
			// This is sent down the fakewebsocket which can race with the
			// native message - so first step is to wait for the result of
			// that command so we are sure the clipboard is set before
			// fetching it.
			if (await check_ === null)
				return; // Either wrong command or a pending event.

			await window.webkit.messageHandlers.clipboard.postMessage(`write`);
		} else if (window.ThisIsTheWindowsApp) {
			// As above.
			if (await check_ === null)
				return;
			await window.postMobileMessage(`CLIPBOARDWRITE`);
		} else {
			const url = this.getMetaURL() + '&MimeType=text/html,text/plain;charset=utf-8';

			// It's important in DisableCopy to write something to the clipboard so that future paste actions can trigger an internal paste
			const text = (this._map['wopi'].DisableCopy ? this._getDisabledCopyStubHtml() : (async () => {
				if (await check_ === null)
					throw new Error('Failed check, either wrong command or pending event');
					// We need to throw an error here rather than just returning so that a failure halts copying the ClipboardItem to the clipboard

				const result = await fetch(url);
				return await result.text();
			})());

			const clipboardItem = new ClipboardItem({
				'text/html': this._parseClipboardFetchResult(text, 'text/html', 'html'),
				'text/plain': this._parseClipboardFetchResult(text, 'text/plain', 'plain'),
			});
			// Again, despite fetch(url), this._parseClipboardFetchResult(...) and check_ all being promises, we need to let browser internals await them after we have safely succeeded in calling clipboard.write
			// We throw an error if our checks fail before returning our text to cause these promises to reject - that way everything can be deferred for later, with failures causing the clipboard write to fail later
			// We define the text promise outside to allow us to reuse the fetch rather than fetching twice (as in Ic23f7f817cc855ff08f25a2afefcd73d6fc3472b)

			let clipboard = navigator.clipboard;
			if (window.L.Browser.cypressTest) {
				clipboard = this._dummyClipboard;
			}

			try {
				await clipboard.write([clipboardItem]);
			} catch (error) {
				// When document is not focused, writing to clipboard is not allowed. But this error shouldn't stop the usage of clipboard API.
				if (!document.hasFocus()) {
					window.app.console.warn('navigator.clipboard.write() failed: ' + error.message);
					// The user switched to another tab before the async clipboard write completed.
					// Schedule a one-shot retry when this window regains focus so the system
					// clipboard is updated with the latest copied content, enabling correct
					// cross-tab paste behaviour.
					// Remove any previous pending retry - only the latest copy matters.
					if (this._pendingClipboardRetryHandler) {
						window.removeEventListener('focus', this._pendingClipboardRetryHandler);
					}
					var retryClipboardItem = clipboardItem;
					var retryClipboard = clipboard;
					var self = this;
					var retryHandler = function() {
						window.removeEventListener('focus', retryHandler);
						self._pendingClipboardRetryHandler = null;
						retryClipboard.write([retryClipboardItem]).catch(function(retryError) {
							window.app.console.warn('navigator.clipboard.write() retry failed: ' + retryError.message);
						});
					};
					this._pendingClipboardRetryHandler = retryHandler;
					window.addEventListener('focus', retryHandler);
					return;
				}

				// Similarly, we'll get an error that is identical to the permission error if our `text` promise rejects
				// But this is really a check failure - if we can see that check failed we don't need to act on the bogus permission error
				if (await check_ === null) {
					 window.app.console.warn('navigator.clipboard.write() failed due to a failing check');
					 return;
				}

				window.app.console.error('navigator.clipboard.write() failed: ' + error.message);
				// Warn that the copy failed.
				this._warnCopyPaste();
				// Prefetch selection, so next time copy will work with the keyboard.
				app.socket.sendMessage('gettextselection mimetype=text/html,text/plain;charset=utf-8');
			}
		}
	},

	// Parses the result from the clipboard endpoint into HTML and plain text.
	parseClipboard: function(text) {
		let textHtml;
		let textPlain = '';
		if (text.startsWith('{')) {
			let textJson = JSON.parse(text);
			textHtml = textJson['text/html'];
			textPlain = textJson['text/plain;charset=utf-8'];
		} else {
			var idx = text.indexOf('<!DOCTYPE HTML');
			if (idx === -1) {
				idx = text.indexOf('<!DOCTYPE html');
			}
			if (idx > 0)
				text = text.substring(idx, text.length);
			textHtml = text;
		}

		if (!app.sectionContainer.testing)
			textHtml = DocUtil.stripStyle(textHtml);

		return {
			'html': textHtml,
			'plain': textPlain
		};
	},

	// Executes the navigator.clipboard.read() call, if it's available.
	_navigatorClipboardRead: function(isSpecial) {
		if (!window.L.Browser.clipboardApiAvailable && !window.ThisIsTheiOSApp && !window.ThisIsTheMacOSApp && !window.ThisIsTheWindowsApp) {
			return false;
		}

		this._asyncAttemptNavigatorClipboardRead(isSpecial);
		return true;
	},

	_MobileAppReadClipboard: function(encodedClipboardData) {
		if (encodedClipboardData === "(internal)") {
			return null;
		}

		const clipboardData = Array.from(
			encodedClipboardData.split(' '),
		).map((encoded) =>
			(encoded === '(null)' ? '' : window.b64d(encoded)),
		);

		const dataByMimeType = {};

		if (clipboardData[0]) {
			dataByMimeType['text/plain'] = new Blob([clipboardData[0]]);
		}

		if (clipboardData[1]) {
			dataByMimeType['text/html'] = new Blob([clipboardData[1]]);
		}

		if (Object.keys(dataByMimeType).length === 0) {
			return [];
		}

		return [new ClipboardItem(dataByMimeType)];
	},

	_iOSReadClipboard: async function() {
		const encodedClipboardData = await window.webkit.messageHandlers.clipboard.postMessage('read');
		return this._MobileAppReadClipboard(encodedClipboardData);
	},

	_WindowsReadClipboard: async function() {
		// FIXME: Unclear whether this function ever is invoked and whether it actually
		// would do anything sane if invoked. Especially the expectation that
		// window.postMobileMessage() would return some value is surely wrong. The
		// CLIPBOARDREAD handling in CODA.cpp certainly does not attempt to return any
		// value, and I don't see how one would even do that in the WebView2 API.
		const encodedClipboardData = await window.postMobileMessage('CLIPBOARDREAD');
		// FIXME: Is the same code as for iOS OK? Will see.
		return this._MobileAppReadClipboard(encodedClipboardData);
	},

	_asyncAttemptNavigatorClipboardRead: async function(isSpecial) {
		var clipboard = navigator.clipboard;
		if (window.L.Browser.cypressTest) {
			clipboard = this._dummyClipboard;
		}
		let clipboardContents;
		try {
			if (window.ThisIsTheiOSApp || window.ThisIsTheMacOSApp)
				clipboardContents = await this._iOSReadClipboard();
			else if (window.ThisIsTheWindowsApp)
				clipboardContents = await this._WindowsReadClipboard();
			else
				clipboardContents = await clipboard.read();

			if (clipboardContents === null) {
				this._doInternalPaste(this._map, false);
				return; // Internal paste, skip the rest of the browser paste code
			}
		} catch (error) {
			window.app.console.log('navigator.clipboard.read() failed: ' + error.message);
			if (isSpecial) {
				// Fallback to the old code, as in filterExecCopyPaste().
				this._openPasteSpecialPopup();
			} else {
				// Fallback to the old code, as in _execCopyCutPaste().
				this._afterCopyCutPaste('paste');
			}
			return;
		}

		if (isSpecial) {
			this._navigatorClipboardPasteSpecial = true;
		}

		if (clipboardContents.length < 1) {
			window.app.console.log('clipboard has no items');
			return;
		}

		var clipboardContent = clipboardContents[0];

		if (clipboardContent.types.includes('text/html')) {
			let blob;
			try {
				blob = await clipboardContent.getType('text/html');
			} catch (error) {
				window.app.console.log('clipboardContent.getType(text/html) failed: ' + error.message);
				return;
			}
			this._navigatorClipboardGetTypeCallback(clipboardContent, blob, 'text/html');
		} else if (clipboardContent.types.includes('text/plain')) {
			let blob;
			try {
				blob = await clipboardContent.getType('text/plain');
			} catch (error) {
				window.app.console.log('clipboardContent.getType(text/plain) failed: ' + error.message);
				return;
			}
			this._navigatorClipboardGetTypeCallback(clipboardContent, blob, 'text/plain');
		} else if (clipboardContent.types.includes('image/png')) {
			let blob;
			try {
				blob = await clipboardContent.getType('image/png');
			} catch (error) {
				window.app.console.log('clipboardContent.getType(image/png) failed: ' + error.message);
				return;
			}
			this._navigatorClipboardGetTypeCallback(clipboardContent, blob, 'image/png');
		} else {
			window.app.console.log('navigator.clipboard has no text/html or text/plain');
			return;
		}
	},

	// Pull UNO clipboard commands out from menus and normal user input.
	// We try to massage and re-emit these, to get good security event / credentials.
	filterExecCopyPaste: function(cmd, params) {
		if (window.ThisIsTheAndroidApp) {
			// perform internal operations
			app.socket.sendMessage('uno ' + cmd);
			return true;
		}

		if (window.ThisIsTheWindowsApp) {
			// Here, too, just let native code handle it
			if (cmd === '.uno:Cut') {
				window.postMobileMessage('CUT');
				return true;
			} else if (cmd === '.uno:Copy') {
				window.postMobileMessage('COPY');
				return true;
			} else if (cmd === '.uno:Paste') {
				window.postMobileMessage('PASTE');
				return true;
			}
		}

		if (window.ThisIsTheQtApp) {
			if (cmd === '.uno:Cut') {
				window.postMobileMessage('CUT');
				return true;
			} else if (cmd === '.uno:Copy') {
				window.postMobileMessage('COPY');
				return true;
			} else if (cmd === '.uno:CopySlide') {
				window.postMobileMessage('COPYSLIDE');
				return true;
			} else if (cmd === '.uno:Paste') {
				window.postMobileMessage('PASTE');
				return true;
			} else if (cmd === '.uno:PasteSpecial') {
				window.postMobileMessage('PASTESPECIAL');
				return true;
			}
		}

		if (cmd === '.uno:Copy' || cmd === '.uno:CopyHyperlinkLocation' || cmd === '.uno:CopySlide') {
			this._execCopyCutPaste('copy', cmd, params);
		} else if (cmd === '.uno:Cut') {
			this._execCopyCutPaste('cut', cmd);
		} else if (cmd === '.uno:Paste') {
			this._execCopyCutPaste('paste', cmd);
		} else if (cmd === '.uno:PasteSpecial') {
			if (this._navigatorClipboardRead(true)) {
				return true;
			}
			this._openPasteSpecialPopup();
		} else {
			return false;
		}
		window.app.console.log('filtered uno command ' + cmd);
		return true;
	},

	_doCopyCut: function(ev, unoName) {
		if (this._selectionType === 'slide')
			unoName = 'CopySlide';
		window.app.console.log(unoName);

		if (this._isAnyInputFieldSelected(unoName === 'Copy'))
			return;

		if (this._downloadProgressStatus() === 'downloadButton')
			this._stopHideDownload(); // Terminate pending confirmation

		var preventDefault = true;

		if (this._map['wopi'].DisableCopy === true)
		{
			app.socket.sendMessage('uno .uno:' + unoName);
			var text = this._getDisabledCopyStubHtml();
			var plainText = DocUtil.stripHTML(text);
			if (ev.clipboardData) {
				window.app.console.log('Copying disabled: put stub message on the clipboard');
				ev.clipboardData.setData('text/plain', plainText ? plainText: ' ');
				ev.clipboardData.setData('text/html', text);
				this._clipboardSerial++;
			}
		} else {
			this._unoCommandForCopyCutPaste = `.uno:${unoName}`;
			this._checkSelection();

			// This is the codepath (_navigatorClipboardWrite) where the browser initiates the clipboard operation, e.g. the keyboard is used.
			if (!this._navigatorClipboardWrite()) {
				app.socket.sendMessage('uno .uno:' + unoName);
				this.populateClipboard(ev);
			}
		}

		if (ev.clipboardData && unoName === 'Cut') {
			// Cut text is not removed from the editable area,
			// so we need to request the focused paragraph.
			this._map._textInput._abortComposition(ev);
		}

		if (preventDefault) {
			ev.preventDefault();
			return false;
		}
	},

	_doInternalPaste: function(map, usePasteKeyEvent) {
		if (usePasteKeyEvent) {
			// paste into dialog
			var KEY_PASTE = 1299;
			map._textInput._sendKeyEvent(0, KEY_PASTE);
		} else if (this._checkAndDisablePasteSpecial()) {
			app.socket.sendMessage('uno .uno:PasteSpecial');
		} else {
			// paste into document
			app.socket.sendMessage('uno .uno:Paste');
		}
	},

	cut:  function(ev) { return this._doCopyCut(ev, 'Cut'); },

	copy: function(ev) { return this._doCopyCut(ev, 'Copy'); },

	paste: function(ev) {
		if (this._map.isReadOnlyMode())
			return;

		window.app.console.log('Paste');

		if (this._isAnyInputFieldSelected() && !this._isFormulabarSelected())
			return;

		// If the focus is in the search box, paste there.
		if (this._map.isSearching())
			return;

		if (this._downloadProgressStatus() === 'downloadButton')
			this._stopHideDownload(); // Terminate pending confirmation

		if (this._map._activeDialog)
			ev.usePasteKeyEvent = true;

		if (ev.clipboardData) {
			ev.preventDefault();
			this._map._textInput._abortComposition(ev);
			this._clipboardSerial++;

			if (window.ThisIsTheQtApp) {
				// Like Windows: native code handles clipboard sync + paste entirely.
				window.postMobileMessage('PASTE');
				return false;
			}

			var usePasteKeyEvent = ev.usePasteKeyEvent;
			// Always capture the html content separate as we may lose it when we
			// pass the clipboard data to a different context (async calls, f.e.).
			var htmlText = ev.clipboardData.getData('text/html');
			var hasFinished = this.dataTransferToDocument(ev.clipboardData, /* preferInternal = */ true, htmlText, usePasteKeyEvent);
			if (hasFinished)
				this._stopHideDownload();
		}
		return false;
	},

	clearSelection: function() {
		this._selectionContent = '';
		this._selectionPlainTextContent = '';
		this._selectionType = null;
		this._scheduleHideDownload();
	},

	// textselectioncontent: message
	setTextSelectionHTML: function(html, plainText = '') {
		this._selectionType = 'text';
		this._selectionContent = html;
		this._selectionPlainTextContent = plainText;
		if (window.L.Browser.cypressTest) {
			this._dummyDiv.innerHTML = html;
			this._dummyPlainDiv.innerText = plainText;
		}
		this._scheduleHideDownload();
	},

	// Sets the selection type without having the selection content (async clipboard).
	setTextSelectionType: function(selectionType) {
		this._selectionType = selectionType;
	},

	// sets the selection to some (cell formula) text)
	setTextSelectionText: function(text) {
		// Usually 'text' is what we see in the formulabar
		// In case of actual formula we don't wish to put formula into client clipboard
		// Putting formula in clipboard means user will paste formula outside of online
		// Pasting inside online is handled by internal paste
		if (this._map.getDocType() === 'spreadsheet' && text.startsWith('=')) {
			app.socket.sendMessage('gettextselection mimetype=text/html');
			return;
		}
		this._selectionType = 'text';
		this._selectionContent = this._originWrapBody(text);
		this._selectionPlainTextContent = text;
		this._scheduleHideDownload();
	},

	setActionCopy: function(isActionCopy) {
		this._isActionCopy = isActionCopy;
	},

	isActionCopy: function() {
		return this._isActionCopy;
	},

	// complexselection: message
	onComplexSelection: function (/*text*/) {
		// Mark this selection as complex.
		this._selectionType = 'complex';
		this._scheduleHideDownload();
	},

	_startProgress: function(isLargeCopy) {
		if (!this._downloadProgress) {
			this._downloadProgress = window.L.control.downloadProgress();
			this._map.addControl(this._downloadProgress);
		}
		this._downloadProgress.show(isLargeCopy);
	},

	_onDownloadOnLargeCopyPaste: function () {
		if (this._downloadProgress && this._downloadProgress.isStarted()) {
			// Need to show this only when a download is really in progress and we block it.
			// Otherwise, it's easier to flash the widget or something.
			this._warnLargeCopyPasteAlreadyStarted();
		} else {
			this._startProgress(true);
		}
	},

	_downloadProgressStatus: function() {
		if (this._downloadProgress)
			return this._downloadProgress.currentStatus();
	},

	// Download button is still shown after selection changed -> user has changed their mind...
	_scheduleHideDownload: function() {
		if (!this._downloadProgress || this._downloadProgress.isClosed())
			return;

		if (['downloadButton', 'confirmPasteButton'].includes(this._downloadProgressStatus()))
			this._stopHideDownload();
	},

	// useful if we did an internal paste already and don't want that.
	_stopHideDownload: function() {
		if (!this._downloadProgress || this._downloadProgress.isClosed())
			return;
		this._downloadProgress._onClose();
	},

	_warnCopyPaste: function() {
		var id = 'copy_paste_warning';
		if (!JSDialog.shouldShowAgain(id))
			return;

		this._map.uiManager.showYesNoButton(
				id + '-box',
				/*title=*/'',
				/*message=*/'',
				/*yesButtonText=*/_('OK'),
				/*noButtonText=*/_('Don’t show this again'),
				/*yesFunction=*/null,
				/*noFunction=*/function () {JSDialog.setShowAgain(id, false);},
				/*cancellable=*/true);
		this._warnCopyPasteImpl(id);
	},

	_warnCopyPasteImpl: function (id) {
		var box = document.getElementById(id + '-box');

		// TODO: do it JSDialog native...
		if (!box) {
			setTimeout(() => { this._warnCopyPasteImpl(id) }, 10);
			return;
		}

		var innerDiv = window.L.DomUtil.create('div', '', null);
		box.insertBefore(innerDiv, box.firstChild);

		if (window.mode.isSmallScreenDevice() || window.mode.isTablet()) {
			const p = document.createElement('p');
			p.textContent = _('Your browser has very limited access to the clipboard, so please use the paste buttons on your on-screen keyboard instead.');
			innerDiv.appendChild(p);
		}
		else {
			const ctrlText = app.util.replaceCtrlAltInMac('Ctrl');
			const p = document.createElement('p');
			p.textContent = _('Your browser has very limited access to the clipboard, so use these keyboard shortcuts:');
			innerDiv.appendChild(p);

			const table = document.createElement('table');
			table.className = 'warn-copy-paste';
			innerDiv.appendChild(table);

			let row = document.createElement('tr');
			table.appendChild(row);

			// Add three cells for copy & cut & paste.
			for (let i = 0; i < 3; i++) {
				const cell = document.createElement('td');
				row.appendChild(cell);

				let kbd = document.createElement('kbd');
				kbd.textContent = ctrlText;
				cell.appendChild(kbd);

				const span = document.createElement('span');
				span.textContent = '+';
				span.className = 'kbd--plus';
				cell.appendChild(span);

				kbd = document.createElement('kbd');
				kbd.textContent = i === 0 ? 'C': (i === 1 ? 'X': 'V');
				cell.appendChild(kbd);
			}

			// Add table headers as second row.
			row = document.createElement('tr');
			table.appendChild(row);
			for (let i = 0; i < 3; i++) {
				const cell = document.createElement('td');
				cell.textContent = i === 0 ? _('Copy'): (i === 1 ? _('Cut'): _('Paste'));
				row.appendChild(cell);
			}
		}
	},

	_substProductName: function (msg) {
		var productName = (typeof brandProductName !== 'undefined') ? brandProductName : 'Collabora Online Development Edition (unbranded)';
		return msg.replace('{productname}', productName);
	},

	_warnLargeCopyPasteAlreadyStarted: function () {
		this._map.uiManager.showInfoModal('large copy paste started warning');
		const container = document.getElementById('large copy paste started warning');
		container.replaceChildren();
		const p = document.createElement('p');
		p.textContent = _('A download due to a large copy/paste operation has already started. Please, wait for the current download or cancel it before starting a new one');
		container.appendChild(p);
	},

	isPasteSpecialDialogOpen: function() {
		if (!this.pasteSpecialDialogId)
			return false;
		else {
			var result = document.getElementById(this.pasteSpecialDialogId);
			return result !== undefined && result !== null ? true: false;
		}
	},

	isCopyPasteDialogReadyForCopy: function () {
		return this._downloadProgress && this._downloadProgress.isComplete();
	},

	_openPasteSpecialPopup: function () {
		if (window.ThisIsTheWindowsApp) {
			// No warning dialog necessary, just do it
			app.socket.sendMessage('uno .uno:PasteSpecial');
			return;
		}

		// We will use this for closing the dialog.
		this.pasteSpecialDialogId = this._map.uiManager.generateModalId('paste_special_dialog') + '-box';

		var id = 'paste_special_dialog';
		this._map.uiManager.showYesNoButton(id + '-box', /*title=*/'', /*message=*/'', /*yesButtonText=*/_('Paste from this document'), /*noButtonText=*/_('Cancel paste special'), /*yesFunction=*/function() {
			app.socket.sendMessage('uno .uno:PasteSpecial');
		}, /*noFunction=*/null, /*cancellable=*/true);

		this._openPasteSpecialPopupImpl(id);
	},

	_openPasteSpecialPopupImpl: function (id) {
		var box = document.getElementById(id + '-box');

		// TODO: do it JSDialog native...
		if (!box) {
			setTimeout(() => { this._openPasteSpecialPopupImpl(id) }, 10);
			return;
		}

		var innerDiv = window.L.DomUtil.create('div', '', null);
		box.insertBefore(innerDiv, box.firstChild);

		const ctrlText = app.util.replaceCtrlAltInMac('Ctrl');

		let p = document.createElement('p');
		p.textContent = _('Your browser has very limited access to the clipboard');
		innerDiv.appendChild(p);
		p = document.createElement('p');
		innerDiv.appendChild(p);
		const bold = document.createElement('b');
		bold.textContent = _('Please use following combination to see more options:');
		p.appendChild(bold);

		p = document.createElement('p');
		innerDiv.appendChild(p);
		let kbd = document.createElement('kbd');
		kbd.textContent = ctrlText;
		p.appendChild(kbd);
		const span = document.createElement('span');
		span.className = 'kbd--plus';
		span.textContent = '+';
		p.appendChild(span);
		kbd = document.createElement('kbd');
		kbd.textContent = 'V';
		p.appendChild(kbd);

		p = document.createElement('p');
		innerDiv.appendChild(p);
		p.textContent = _('Close popup to ignore paste special');

		// Drop the not wanted whitespace between the dialog body and the button row at the
		// bottom.
		var label = document.getElementById('modal-dialog-' + id + '-box-label');
		label.style.display = 'none';
	},

	// Check if the paste special mode is enabled, and if so disable it.
	_checkAndDisablePasteSpecial: function() {
		if (this._navigatorClipboardPasteSpecial) {
			this._navigatorClipboardPasteSpecial = false;
			return true;
		}

		if (this.isPasteSpecialDialogOpen()) {
			this._map.jsdialog.closeDialog(this.pasteSpecialDialogId, false);
			return true;
		}

		return false;
	},
});

window.L.clipboard = function(map) {
	if (window.ThisIsTheAndroidApp)
		window.app.console.log('======> Assertion failed!? No window.L.Clipboard object should be needed in the Android app');
	return new window.L.Clipboard(map);
};
