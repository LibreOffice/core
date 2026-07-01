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
 * CalcNotifications - calc-specific UI notifications from core
 * (snackbars, toasts), kept separate from CalcTileLayer which
 * handles tile/layer state.
 */

interface CalcRefErrorState {
	cellAddress: string;
}

interface CalcValidationStopState {
	title: string; // user-set title from the Error Alert tab; empty string when the user left it blank
	message: string;
}

class CalcNotifications {
	private map: any;

	constructor(map: any) {
		this.map = map;
		this.map.on('commandstatechanged', this.onStateChanged, this);
	}

	private onStateChanged(e: any): void {
		if (e.commandName === 'CalcRefError') {
			this.onCalcRefError(e.state);
		} else if (e.commandName === 'CalcRefErrorDismiss') {
			this.map.uiManager.closeSnackbar();
		} else if (e.commandName === 'CalcExpandRefsSkipped') {
			this.onCalcExpandRefsSkipped();
		} else if (e.commandName === 'CalcValidationStop') {
			this.onCalcValidationStop(e.state);
		}
	}

	private onCalcExpandRefsSkipped(): void {
		this.map.uiManager.showSnackbar(
			_('A formula range was not extended to include the inserted cells.'),
		);
	}

	private onCalcValidationStop(state: CalcValidationStopState): void {
		if (!state) return;
		const label = state.title
			? state.title + ': ' + state.message
			: state.message;
		this.map.uiManager.showSnackbar(label, null, null, 6000, false, true);
	}

	private onCalcRefError(state: CalcRefErrorState): void {
		if (!state || !state.cellAddress) return;
		const map = this.map;
		const cellAddr = state.cellAddress;
		map.uiManager.showSnackbar(
			_('Deletion created #REF! errors in other formulas. Undo to restore.'),
			_('Find First'),
			function () {
				map.sendUnoCommand('.uno:GoToCell', {
					ToPoint: { type: 'string', value: cellAddr },
				});
			},
			-1,
			false,
			true,
		);
	}
}
