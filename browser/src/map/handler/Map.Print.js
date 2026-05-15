/* -*- js-indent-level: 8 -*- */
/*
 * window.L.Map.Print is handling the print action
 */

/* global _ */

window.L.Map.mergeOptions({
	printHandler: true
});

window.L.Map.Print = window.L.Handler.extend({

	initialize: function (map) {
		this._map = map;
	},

	addHooks: function () {
		this._map.on('filedownloadready', this._onFileReady, this);
	},

	removeHooks: function () {
		this._map.off('filedownloadready', this._onFileReady, this);
	},

	_onFileReady: function (e) {
		// we need to load the pdf document and pass it to the iframe as an
		// object URL, because else we might have cross origin security problems
		var xmlHttp = new XMLHttpRequest();
		xmlHttp.onreadystatechange = window.L.bind(function () {
			if (xmlHttp.readyState === 4 && xmlHttp.status === 200) {
				this._onInitPrint(xmlHttp);
			}
		}, this);
		xmlHttp.open('GET', e.url, true);
		xmlHttp.responseType = 'blob';
		xmlHttp.send();
	},

	_onInitPrint: function (e) {
		var blob = new Blob([e.response], {type: 'application/pdf'});
		var url = URL.createObjectURL(blob);
		this._printIframe = window.L.DomUtil.create('iframe', '', document.body);
		this._printIframe.title = _('Print dialog');
		this._printIframe.onload = window.L.bind(this._onIframeLoaded, this);
		window.L.DomUtil.setStyle(this._printIframe, 'visibility', 'hidden');
		if (window.L.Browser.safari) {
			// In Safari, 'visibility: hidden' does not work.
			window.L.DomUtil.setStyle(this._printIframe, 'display', 'none');
		}
		window.L.DomUtil.setStyle(this._printIframe, 'position', 'fixed');
		window.L.DomUtil.setStyle(this._printIframe, 'right', '0');
		window.L.DomUtil.setStyle(this._printIframe, 'bottom', '0');
		this._printIframe.src = url;
	},

	_onIframeLoaded: function () {
		this._printIframe.contentWindow.focus(); // Required for IE
		if (window.L.Browser.safari) {
			// In Safari, we have to wait until the PDF iframe is rendered.
			// Wait 2 paint cycles to be safe.
			requestAnimationFrame(() => {
				requestAnimationFrame(() => {
					this._printIframe.contentWindow.print();
				});
			});
		} else {
			this._printIframe.contentWindow.print();
		}
		// couldn't find another way to remove it
		setTimeout(window.L.bind(this._closePrintIframe, this, this._printIframe), 300 * 1000);
	},

	_closePrintIframe: function (printIframe) {
		window.L.DomUtil.remove(printIframe);
		this._map.focus();
	}
});

window.L.Map.addInitHook('addHandler', 'printHandler', window.L.Map.Print);
