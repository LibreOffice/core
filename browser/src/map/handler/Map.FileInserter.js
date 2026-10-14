/* -*- js-indent-level: 8 -*- */
/*
 * window.L.Map.FileInserter is handling the fileInserter action
 */

/* global app _ Uint8Array errorMessages */

window.L.Map.mergeOptions({
	fileInserter: true
});

window.L.Map.FileInserter = window.L.Handler.extend({

	initialize: function (map) {
		this._map = map;
		this._childId = null;
		this._toInsertGraphic = {};
		this._toInsertMultimedia = {};
		this._toInsertURL = {};
		this._toInsertBackground = {};
		this._toCompareDocuments = {};
		this._toFilePicker = {};
		// The id of the engine's file request the next picked file answers, or null.
		this._filePickerRequestId = null;
		this._onFilePickerChange = this._onFilePickerChange.bind(this);
		this._onFilePickerCancel = this._onFilePickerCancel.bind(this);
		var parser = document.createElement('a');
		parser.href = window.host;
	},

	getWopiUrl: function (map) {
		return window.makeHttpUrlWopiSrc('/' + map.options.urlPrefix + '/', map.options.doc, '/insertfile');
	},

	addHooks: function () {
		this._map.on('insertgraphic', this._onInsertGraphic, this);
		this._map.on('insertmultimedia', this._onInsertMultimedia, this);
		this._map.on('inserturl', this._onInsertURL, this);
		this._map.on('childid', this._onChildIdMsg, this);
		this._map.on('selectbackground', this._onSelectBackground, this);
		this._map.on('comparedocuments', this._onCompareDocuments, this);
		this._map.on('filepicker', this._onFilePickerRequest, this);
		const input = document.getElementById('filepicker');
		if (input) {
			input.addEventListener('change', this._onFilePickerChange);
			input.addEventListener('cancel', this._onFilePickerCancel);
		}
	},

	removeHooks: function () {
		this._map.off('insertgraphic', this._onInsertGraphic, this);
		this._map.off('insertmultimedia', this._onInsertMultimedia, this);
		this._map.off('inserturl', this._onInsertURL, this);
		this._map.off('childid', this._onChildIdMsg, this);
		this._map.off('selectbackground', this._onSelectBackground, this);
		this._map.off('comparedocuments', this._onCompareDocuments, this);
		this._map.off('filepicker', this._onFilePickerRequest, this);
		const input = document.getElementById('filepicker');
		if (input) {
			input.removeEventListener('change', this._onFilePickerChange);
			input.removeEventListener('cancel', this._onFilePickerCancel);
		}
	},

	// The engine needs a file, for example an image for a dialog. The picked
	// file goes up through the insertfile POST like an inserted image, and the
	// engine gets its name in a filepickerresult message with the request id.
	_onFilePickerRequest: function (e) {
		const input = document.getElementById('filepicker');
		if (!input) {
			this._answerFilePicker(e.id, null);
			return;
		}

		// One file type filter is "*.png;*.jpg"; the input takes ".png,.jpg".
		const extensions = [];
		for (const filter of e.filters || []) {
			for (const wildcard of filter.split(';')) {
				const trimmed = wildcard.trim();
				if (trimmed.startsWith('*.') && trimmed.length > 2 && trimmed !== '*.*')
					extensions.push(trimmed.substring(1));
			}
		}
		input.accept = extensions.join(',');

		this._filePickerRequestId = e.id;
		input.click();
	},

	_onFilePickerChange: function () {
		const input = document.getElementById('filepicker');
		const file = input.files && input.files.length > 0 ? input.files[0] : null;
		// Clear the value so picking the same file again fires change again.
		input.value = null;

		if (!file) {
			this._onFilePickerCancel();
			return;
		}

		if (!this._childId) {
			app.socket.sendMessage('getchildid');
			this._toFilePicker[Date.now()] = file;
		}
		else {
			this._sendFile(Date.now(), file, 'filepicker');
		}
	},

	_onFilePickerCancel: function () {
		this._answerFilePicker(this._filePickerRequestId, null);
	},

	// Tells the engine which uploaded file answers its request, or that the
	// request ends with no file when name is null.
	_answerFilePicker: function (requestId, name) {
		if (requestId === null || requestId === undefined)
			return;
		if (requestId === this._filePickerRequestId)
			this._filePickerRequestId = null;
		app.socket.sendMessage('filepickerresult id=' + requestId + (name ? ' name=' + name : ''));
	},

	_onInsertGraphic: function (e) {
		if (!this._childId) {
			app.socket.sendMessage('getchildid');
			this._toInsertGraphic[Date.now()] = e.file;
		}
		else {
			this._sendFile(Date.now(), e.file, 'graphic');
		}
	},

	_onInsertMultimedia: function (e) {
		// An image picked as media, such as an animated GIF, goes in as a
		// picture, the way a pasted or dropped image does. A picture keeps
		// its animation.
		if (e.file && e.file.type && e.file.type.startsWith('image/')) {
			this._onInsertGraphic(e);
			return;
		}

		if (!this._childId) {
			app.socket.sendMessage('getchildid');
			this._toInsertMultimedia[Date.now()] = e.file;
		}
		else {
			this._sendFile(Date.now(), e.file, 'multimedia');
		}
	},

	_onInsertURL: function (e) {
		if (!this._childId) {
			app.socket.sendMessage('getchildid');
			this._toInsertURL[Date.now()] = e;
		}
		else {
			this._sendURL(Date.now(), e);
		}
	},

	_onSelectBackground: function (e) {
		if (!this._childId) {
			app.socket.sendMessage('getchildid');
			this._toInsertBackground[Date.now()] = e.file;
		}
		else {
			this._sendFile(Date.now(), e.file, 'selectbackground');
		}
	},

	_onCompareDocuments: function (e) {
		if (!this._childId) {
			app.socket.sendMessage('getchildid');
			this._toCompareDocuments[Date.now()] = e.file;
		} else {
			this._sendFile(Date.now(), e.file, 'comparedocuments');
		}
	},

	_onChildIdMsg: function (e) {
		// When childId is not created (usually when we insert file/URL very first time), we send message to get child ID
		// and store the file(s) into respective arrays (look at _onInsertGraphic, _onInsertMultimedia, _onInsertURL, _onSelectBackground)
		// When we receive the childId we empty all the array and insert respective file/URL from here

		this._childId = e.id;
		for (var name in this._toInsertGraphic) {
			this._sendFile(name, this._toInsertGraphic[name], 'graphic');
		}
		this._toInsertGraphic = {};

		for (var name in this._toInsertMultimedia) {
			this._sendFile(name, this._toInsertMultimedia[name], 'multimedia');
		}
		this._toInsertMultimedia = {};

		for (name in this._toInsertURL) {
			this._sendURL(name, this._toInsertURL[name]);
		}
		this._toInsertURL = {};

		for (name in this._toInsertBackground) {
			this._sendFile(name, this._toInsertBackground[name], 'selectbackground');
		}
		this._toInsertBackground = {};

		for (name in this._toCompareDocuments) {
			this._sendFile(name, this._toCompareDocuments[name], 'comparedocuments');
		}
		this._toCompareDocuments = {};

		for (name in this._toFilePicker) {
			this._sendFile(name, this._toFilePicker[name], 'filepicker');
		}
		this._toFilePicker = {};
	},

	_sendFile: async function (name, file, type) {
		var socket = app.socket;
		var map = this._map;
		var sectionContainer = app.sectionContainer;
		var url = this.getWopiUrl(map);
		const fileInserter = this;
		// The request this upload answers; the engine hears about a failed
		// upload too, so its request does not stay open.
		const filePickerRequestId = type === 'filepicker' ? this._filePickerRequestId : null;

		var size;

		if (type === 'multimedia') {
			const videoElement = document.createElement('video');
			const objectURL = window.URL.createObjectURL(file);
			videoElement.src = objectURL;

			const videoLoadPromise = new Promise((resolve, reject) => {
				videoElement.addEventListener("loadedmetadata", resolve);
				videoElement.addEventListener("error", reject);
			});

			videoElement.load();

			let videoLoaded = false;

			try {
				await videoLoadPromise;
				videoLoaded = true;
			} catch (_error) {
				size = {
					width: 0,
					height: 0,
				}; // 0, 0 will make core pick the minimum size - which was the behavior before we checked size like this
			}

			if (videoLoaded) {
				size = {
					width: videoElement.videoWidth,
					height: videoElement.videoHeight,
				};

				const maxSize = {
					width: app.activeDocument.fileSize.cX * app.activeDocument.getZoomScale(10),
					height: app.activeDocument.fileSize.cY * app.activeDocument.getZoomScale(10),
				};

				const shrinkToFitFactor = Math.min(
					1,
					maxSize.width / size.width,
					maxSize.height / size.height
				);

				size.width *= shrinkToFitFactor;
				size.height *= shrinkToFitFactor;
			}

			videoElement.src = undefined;
			window.URL.revokeObjectURL(objectURL);
		}

		if ('processCoolUrl' in window) {
			url = window.processCoolUrl({ url: url, type: 'insertfile' });
		}

		if (!(file.filename && file.url) && (file.name === '' || file.size === 0)) {
			var errMsg = window.mode.isCODesktop()
				? _('The file of type: {0} cannot be inserted since the file has no name')
				: _('The file of type: {0} cannot be uploaded to server since the file has no name');
			if (file.size === 0)
				errMsg = window.mode.isCODesktop()
					? _('The file of type: {0} cannot be inserted since the file is empty')
					: _('The file of type: {0} cannot be uploaded to server since the file is empty');
			errMsg = errMsg.replace('{0}', file.type);
			map.fire('error', {msg: errMsg, critical: false});
			this._answerFilePicker(filePickerRequestId, null);
			return;
		}

		this._toInsertBackground = {};

		if (window.ThisIsAMobileApp) {
			// Pass the file contents as a base64-encoded parameter in an insertfile message
			var reader = new FileReader();
			reader.onload = function(e) {
				var byteBuffer = new Uint8Array(e.target.result);
				var strBytes = '';
				for (var i = 0; i < byteBuffer.length; i++) {
					strBytes += String.fromCharCode(byteBuffer[i]);
				}

				if (type === 'multimedia') {
					window.postMobileMessage('insertfile name=' + name + ' type=' + type +
										       ' data=' + window.btoa(strBytes) +
										       ' width=' + size.width + ' height=' + size.height);
				} else {
					window.postMobileMessage('insertfile name=' + name + ' type=' + type +
										       ' data=' + window.btoa(strBytes));
				}
			};
			reader.onerror = function(e) {
				window.postMobileError('Error when reading file: ' + e);
			};
			reader.onprogress = function(e) {
				window.postMobileDebug('FileReader progress: ' + Math.round(e.loaded*100 / e.total) + '%');
			};
			reader.readAsArrayBuffer(file);
		} else {
			var xmlHttp = new XMLHttpRequest();
			this._map.showBusy(_('Uploading...'), false);
			xmlHttp.onreadystatechange = function () {
				if (xmlHttp.readyState === 4) {
					map.hideBusy();
					if (xmlHttp.status === 200) {
						var sectionName = app.CSections.ContentControl.name;
						var section;
						if (sectionContainer.doesSectionExist(sectionName)) {
							section = sectionContainer.getSectionWithName(sectionName);
						}
						if (section && section.sectionProperties.picturePicker && type === 'graphic') {
							socket.sendMessage('contentcontrolevent type=picture' + ' name=' + name);
						} else if (type === 'filepicker') {
							fileInserter._answerFilePicker(filePickerRequestId, name);
						} else if (type === 'multimedia') {
							socket.sendMessage('insertfile name=' + name + ' type=' + type + ' width=' + size.width + ' height=' + size.height);
						} else {
							socket.sendMessage('insertfile name=' + name + ' type=' + type);
						}
					}
					else if (xmlHttp.status === 404) {
						map.fire('error', {msg: errorMessages.uploadfile.notfound, critical: false});
					}
					else if (xmlHttp.status === 413) {
						map.fire('error', {msg: errorMessages.uploadfile.toolarge, critical: false});
					}
					else {
						var msg = _('Uploading file to server failed with status: {0}');
						msg = msg.replace('{0}', xmlHttp.status);
						map.fire('error', {msg: msg, critical: false});
					}
					if (xmlHttp.status !== 200)
						fileInserter._answerFilePicker(filePickerRequestId, null);
				}
			};
			xmlHttp.open('POST', url, true);
			var formData = new FormData();
			formData.append('name', name);
			formData.append('childid', this._childId);
			if (file.filename && file.url) {
				formData.append('url', file.url);
				formData.append('filename', file.filename);
			} else {
				// Read file into memory first to handle content:// URIs
				// (e.g. files from Google Drive on Android WebView)
				// that may become inaccessible during XHR upload.
				try {
					let fileData = await new Promise(function(resolve, reject) {
						let reader = new FileReader();
						reader.onload = function(e) { resolve(e.target.result); };
						reader.onerror = function(e) { reject(e); };
						reader.readAsArrayBuffer(file);
					});
					formData.append('file', new Blob([fileData], {type: file.type}), file.name);
				} catch (e) {
					map.hideBusy();
					map.fire('error', {msg: _('Failed to read the selected file.'), critical: false});
					this._answerFilePicker(filePickerRequestId, null);
					return;
				}
			}
			xmlHttp.send(formData);

			// Set it to null in case server restarts/shuts down or the user reconnects after being idle
			// these change the childId but it would be cached already with the old one if a previous insertfile is made.
			// in that case we would get http error 400 because of the wrong childId.
			// when it's null we ask for a new childId before uploading.
			this._childId = null;
		}
	},

	_sendURL: function (name, e) {
		var sectionName = app.CSections.ContentControl.name;
		var section;
		if (app.sectionContainer.doesSectionExist(sectionName)) {
			section = app.sectionContainer.getSectionWithName(sectionName);
		}

		if (e.urltype == "graphicurl" && section && section.sectionProperties.picturePicker) {
			// The order argument is important
			app.socket.sendMessage('contentcontrolevent type=pictureurl name=' + encodeURIComponent(e.url));
		} else {
			// URL media is fetched by the kit, with no browser upload
			// step, so show the progress indicator from here.
			if (e.urltype === 'multimediaurl') {
				this._map.showBusy(_('Uploading...'), false);
			}
			app.socket.sendMessage('insertfile name=' + encodeURIComponent(e.url) + ' type=' + e.urltype);
		}
	}
});

window.L.Map.addInitHook('addHandler', 'fileInserter', window.L.Map.FileInserter);
