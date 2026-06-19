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
	},

	removeHooks: function () {
		this._map.off('insertgraphic', this._onInsertGraphic, this);
		this._map.off('insertmultimedia', this._onInsertMultimedia, this);
		this._map.off('inserturl', this._onInsertURL, this);
		this._map.off('childid', this._onChildIdMsg, this);
		this._map.off('selectbackground', this._onSelectBackground, this);
		this._map.off('comparedocuments', this._onCompareDocuments, this);
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
	},

	_sendFile: async function (name, file, type) {
		var socket = app.socket;
		var map = this._map;
		var sectionContainer = app.sectionContainer;
		var url = this.getWopiUrl(map);

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
					width: app.activeDocument.fileSize.cX * map.getZoomScale(10),
					height: app.activeDocument.fileSize.cY * map.getZoomScale(10),
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
			app.socket.sendMessage('insertfile name=' + encodeURIComponent(e.url) + ' type=' + e.urltype);
		}
	}
});

window.L.Map.addInitHook('addHandler', 'fileInserter', window.L.Map.FileInserter);
