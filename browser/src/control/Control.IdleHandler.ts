// @ts-strict-ignore
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
 * Class for idle handling of the view.
 */

/* global app L TileManager */

// FIXME: These should surely be somewhere else and not in this random .ts file.
declare var mode: any;
declare var ThisIsTheAndroidApp: any;
declare var postMobileMessage: any;
declare var postMobileCall: any;
declare var idleTimeoutSecs: number;
declare var outOfFocusTimeoutSecs: number;

/**/

app.idleHandlerId = 'inactive_user_message';

class IdleHandler {
    _serverRecycling: boolean = false;
    _documentIdle: boolean = false;
	_lastActivity: number = Date.now();
	_inactivityTimer: ReturnType<typeof setTimeout> = null;
	_outOfFocusTimer: ReturnType<typeof setTimeout> = null;
    _active: boolean = true;
    map: any;
	dimId: string = app.idleHandlerId;

	getIdleMessage(): string {
		if (this.map['wopi'] && this.map['wopi'].DisableInactiveMessages) {
			return '';
		} else if (window.mode.isDesktop()) {
			return _('Idle document - please click to reload and resume editing');
		} else {
			return _('Idle document - please tap to reload and resume editing');
		}
	}

	isDimActive(): boolean {
		return !!document.getElementById(this.map.uiManager.generateModalId(this.dimId));
	}

	// time from the last activity in [s]
	getElapsedFromActivity(): number {
		return (Date.now() - this._lastActivity) / 1000;
	}

	refreshAnnotations() {
		var docLayer = this.map._docLayer;
		if (docLayer.isCalc() && docLayer.options.sheetGeometryDataEnabled) {
			docLayer.requestSheetGeometryData();
		}
		app.socket.sendMessage('commandvalues command=.uno:ViewAnnotations');
	}

	_activate() {
		window.app.console.debug('IdleHandler: _activate()');

		if (this._serverRecycling || this._documentIdle) {
			return false;
		}

		this._startInactiveTimer();
		this._stopOutOfFocusTimer();

		if (!this._active) {
			// Only activate when we are connected.
			if (app.socket.connected()) {
				app.socket.sendMessage('useractive');
				this._active = true;

				/*
				  If we have the docLayer then refresh annotations now. If not then
				  postpone until we do have the docLayer so we know if this is calc
				  or not, because for calc we have to ensure we have the sheet
				  geometry before requesting annotations otherwise we will lack the
				  requirements to position them.
				*/
				if (this.map._docLayer) {
					this.map._docLayer.allowDrawing();
					this.refreshAnnotations();
				}
				else {
					this.map.once('doclayerinit', this.refreshAnnotations, this);
				}

				if (this.isDimActive()) {
					this.map.jsdialog.closeDialog(this.dimId, false);
					return true;
				}
			} else {
				this.map.loadDocument();
			}
		}

		// Ideally instead of separate isAnyEdit check here, we could check isAnyEdit inside isAnyDialogOpen,
		// but unfortunately that causes problem in _deactivate and unnecessary 'userinactive' message is sent
		if (window.mode.isDesktop()
		&& !this.map.uiManager.isAnyDialogOpen()
		&& !cool.Comment.isAnyEdit()
		&& (this.map.formulabar && !this.map.formulabar.hasFocus())
		&& $('input:focus').length === 0) {
			this.map.focus();
		}

		return false;
	}

	_startInactiveTimer() {
		if (this._serverRecycling || this._documentIdle || !this.map._docLoaded) {
			return;
		}

		clearTimeout(this._inactivityTimer);

		this._inactivityTimer = setTimeout(() => {
			this._dimIfInactive();
		}, (window.L.Browser.cypressTest ? 1000 : 1 * 60 * 1000)); // Check once a minute
	}

	_startOutOfFocusTimer() {
		if (this._serverRecycling || this._documentIdle || !this.map._docLoaded) {
			return;
		}

		this._stopOutOfFocusTimer();

		this._outOfFocusTimer = setTimeout(() => {
			this._dim();
		}, window.outOfFocusTimeoutSecs * 1000);
	}

	_stopOutOfFocusTimer() {
		clearTimeout(this._outOfFocusTimer);
	}

	_dimIfInactive() {
		if (this.map._docLoaded && (this.getElapsedFromActivity() >= window.idleTimeoutSecs)) {
			this._dim();
		} else {
			this._startInactiveTimer();
		}
	}

	_dim() {
		if (this.map.slideShowPresenter && this.map.slideShowPresenter._checkAlreadyPresenting())
			return; // do not stop presentation

		this.map.fire('closealldialogs');
		const message = this.getIdleMessage();

		window.app.console.debug('IdleHandler: _dim()');

		if (document.getElementById(this.dimId))
			return;

		this._active = false;
		var map = this.map;

		var restartConnectionFn = () => {
			if (app.idleHandler._documentIdle)
			{
				window.app.console.debug('idleness: reactivating');
				map.fire('postMessage', {msgId: 'User_Active'});
				app.idleHandler._documentIdle = false;
				app.setCursorVisibility(true);
			}
			return app.idleHandler._activate();
		};

		this.map._textInput.hideCursor();

		const uiManager = this.map.uiManager;
		const dialogId = uiManager.generateModalId(this.dimId);
		uiManager.showInfoModal(this.dimId);

		app.layoutingService.appendLayoutingTask(() => {
			const dimNode = document.getElementById(this.dimId);
			if (!dimNode)
				return;

			dimNode.textContent = message;

			const restartConnection = () => { restartConnectionFn(); };

			if (message === '') {
				const dialogNode = document.getElementById(dialogId);
				if (dialogNode) dialogNode.style.display = 'none';
				app.LOUtil.onRemoveHTMLElement(dimNode, restartConnection);
			} else {
				const overlayId = dialogId + '-overlay';
				const overlay = document.getElementById(overlayId);
				if (overlay) overlay.onmouseover = () => { restartConnection(); uiManager.closeModal(dialogId); };
				app.LOUtil.onRemoveHTMLElement(overlay, restartConnection);
			}
		});

		this._sendInactiveMessage();

		TileManager.clearPreFetch();
	}

	notifyActive() {
		this._lastActivity = Date.now();

		if (window.ThisIsTheAndroidApp) {
			window.postMobileMessage('LIGHT_SCREEN');
		}
	}

	_sendInactiveMessage() {
		this.map._doclayer && this.map._docLayer._onMessage('textselection:', null);
		this.map.fire('postMessage', {msgId: 'User_Idle'});
		if (app.socket.connected()) {
			app.socket.sendMessage('userinactive');
		}
	}

	_deactivate() {
		window.app.console.debug('IdleHandler: _deactivate()');

		if (this._serverRecycling || this._documentIdle || !this.map._docLoaded) {
			return;
		}

		if (window.mode.isDesktop() && (!this._active || this.isDimActive())) {
			// A dialog is already dimming the screen and probably
			// shows an error message. Leave it alone.
			this._active = false;
			this._sendInactiveMessage();

			return;
		}

		if (app.map && app.map.formulabar &&
			(app.map.formulabar.hasFocus() || app.map.formulabar.isInEditMode()))
			app.dispatcher.dispatch('acceptformula'); // save data from the edited cell on exit

		this._startOutOfFocusTimer();
	}
}

// Initiate the class.
app.idleHandler = new IdleHandler();
