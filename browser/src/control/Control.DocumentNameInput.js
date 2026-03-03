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
 * window.L.Control.DocumentNameInput
 */

/* global $ _ */
window.L.Control.DocumentNameInput = window.L.Control.extend({

	onAdd: function (map) {
		this.map = map;
		if (window.mode.isSmallScreenDevice())
			this.progressBar = document.getElementById('mobile-progress-bar');
		else
			this.progressBar = document.getElementById('document-name-input-progress-bar');

		var label = document.querySelector('label[for="document-name-input"]');
		if (label)
			label.textContent = _('Document name');

		map.on('doclayerinit', this.onDocLayerInit, this);
		map.on('wopiprops', this.onWopiProps, this);
	},

	documentNameConfirm: function(value) {
		if (value !== null && value != '' && value != this.map['wopi'].BaseFileName) {
			this._renaming = true;
			if (this.map['wopi'].UserCanRename && this.map['wopi'].SupportsRename) {
				if (value.lastIndexOf('.') > 0) {
					var fname = this.map['wopi'].BaseFileName;
					var ext = fname.substr(fname.lastIndexOf('.')+1, fname.length);
					// check format conversion
					if (ext != value.substr(value.lastIndexOf('.')+1, value.length)) {
						this.map.saveAs(value);
					} else {
						// same extension, just rename the file
						// file name must be without the extension for rename
						value = value.substr(0, value.lastIndexOf('.'));
						this.map.renameFile(value);
					}
				}
				else {
					// when user doesn't specify any extension
					this.map.renameFile(value);
				}
			} else {
				// saveAs for rename
				this.map.saveAs(value);
			}
		}
		this.map._onGotFocus();
	},

	documentNameCancel: function(blur) {
		if (this._renaming)
			return;

		$('#document-name-input').val(this.map['wopi'].BreadcrumbDocName);
		if (blur !== true) {
			this.map._onGotFocus();
		}
	},

	disableDocumentNameInput : function() {
		$('#document-name-input').prop('disabled', true);
		$('#document-name-input').removeClass('editable');
		$('#document-name-input').off('keypress', this.onDocumentNameKeyPress);
	},

	enableDocumentNameInput : function() {
		$('#document-name-input').prop('disabled', false);
		$('#document-name-input').addClass('editable');
		$('#document-name-input').off('keypress', this.onDocumentNameKeyPress).on('keypress', this.onDocumentNameKeyPress.bind(this));
		$('#document-name-input').off('focus', this.onDocumentNameFocus).on('focus', this.onDocumentNameFocus.bind(this));
		$('#document-name-input').off('blur', this.documentNameCancel).on('blur', this.documentNameCancel.bind(this, true));
	},

	onDocumentNameKeyPress: function(e) {
		if (e.keyCode === 13) { // Enter key
			var value = $('#document-name-input').val();
			this.documentNameConfirm(value);
		} else if (e.keyCode === 27) { // Escape key
			this.documentNameCancel();
		}
	},

	onDocumentNameFocus: function() {
		// hide the caret in the main document
		delete this._renaming;
		this.map._onLostFocus();
		var name = this.map['wopi'].BaseFileName;
		var extn = name.lastIndexOf('.');
		if (extn < 0)
			extn = name.length;
		$('#document-name-input').val(name);
		$('#document-name-input')[0].setSelectionRange(0, extn);
	},

	onDocLayerInit: function() {

		var el = $('#document-name-input');

		try {
			var fileNameFullPath = new URL(
				new URLSearchParams(window.location.search).get('WOPISrc')
			)
				.pathname
				.replace('/wopi/files', '');

			var basePath = fileNameFullPath.replace(this.map['wopi'].BaseFileName , '').replace(/\/$/, '');
			var title = this.map['wopi'].BaseFileName + '\n' + _('Path') + ': ' + basePath;

			el.prop('title', title);
		} catch (e) {
			// purposely ignore the error for legacy browsers
		}

		// FIXME: Android app would display a temporary filename, not the actual filename
		if (window.ThisIsTheAndroidApp) {
			el.hide();
		} else {
			el.show();
		}

		if (window.ThisIsAMobileApp) {
			// We can now set the document name in the menu bar
			el.prop('disabled', false);
			el.removeClass('editable');
			el.focus(function() { $(this).blur(); });
			// Call decodeURIComponent twice: Reverse both our encoding and the encoding of
			// the name in the file system.
			el.val(window.ThisIsTheEmscriptenApp ? this.map['wopi'].BaseFileName : decodeURIComponent(decodeURIComponent(this.map.options.doc.replace(/.*\//, '')))
							  // To conveniently see the initial visualViewport scale and size, un-comment the following line.
							  // + ' (' + window.visualViewport.scale + '*' + window.visualViewport.width + 'x' + window.visualViewport.height + ')'
							  // TODO: Yes, it would be better to see it change as you rotate the device or invoke Split View.
							 );
		}

		if (this.map.isReadOnlyMode()) {
			this.disableDocumentNameInput();
		}
	},

	onWopiProps: function(e) {
		if (e.BaseFileName !== null) {
			// set the document name into the name field
			$('#document-name-input').val(e.BreadcrumbDocName !== undefined ? e.BreadcrumbDocName : e.BaseFileName);
			var input = window.L.DomUtil.get('document-name-input');
			input.setAttribute('data-cooltip', input.value);
			window.L.control.attachTooltipEventListener(input, this.map);
		}
		if (!e.UserCanNotWriteRelative && !this.map.isReadOnlyMode()) {
			// Save As allowed
			this.enableDocumentNameInput();
		} else {
			this.disableDocumentNameInput();
		}
	},

	showProgressBar: function() {
		this.disableDocumentNameInput();
		this.progressBar.style.display = 'block';
	},

	hideProgressBar: function() {
		this.enableDocumentNameInput();
		this.progressBar.style.display = 'none';
	},

	setProgressBarValue: function(value) {
		this.progressBar.value = value;
	},

	showLoadingAnimation : function() {
		this.disableDocumentNameInput();
		$('#document-name-input-loading-bar').css('display', 'block');
	},

	hideLoadingAnimation : function() {
		this.enableDocumentNameInput();
		$('#document-name-input-loading-bar').css('display', 'none');
	},

	_getMaxAvailableWidth: function() {
		var x = $('#document-titlebar').prop('offsetLeft') + $('.document-title').prop('offsetLeft') + $('#document-name-input').prop('offsetLeft');
		var containerWidth = parseInt($('.main-nav').css('width'));
		var maxWidth = Math.max(containerWidth - x - 30, 0);
		maxWidth = Math.max(maxWidth, 300); // input field at least 300px
		return maxWidth;
	},

});

window.L.control.documentNameInput = function () {
	return new window.L.Control.DocumentNameInput();
};
