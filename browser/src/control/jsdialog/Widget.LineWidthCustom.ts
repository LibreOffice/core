/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/**
 * Widget.LineWidthCustom.ts
 *
 * LineWidthCustom is the tail of the line thickness dropdown: the last custom
 * thickness the user typed. LineWidthField is the field next to the dropdown
 * to type a new one. The sidebar Line panel offers both in its own popup
 * (svx/ui/floatinglineproperty.ui); this is the notebookbar counterpart, so
 * the eight presets are not all that is on offer there either.
 *
 * .uno:LineWidth takes and reports 1/100 mm, the field works in points.
 */

declare var JSDialog: any;

const LAST_CUSTOM_PREF = 'lineWidthLastCustom';

const HMM_PER_POINT = 2540 / 72;

const PREVIEW_PX_PER_POINT = 2.4;
const PREVIEW_MAX_PX = 20;

function pointsToHMM(points: number): number {
	return Math.round(points * HMM_PER_POINT);
}

function hmmToPoints(hmm: number): number {
	return Math.round((hmm / HMM_PER_POINT) * 10) / 10;
}

function formatPoints(points: number): string {
	return points.toFixed(1) + ' pt';
}

function getLastCustomWidth(): number | null {
	const points = parseFloat(window.prefs.get(LAST_CUSTOM_PREF, ''));
	return isNaN(points) || points <= 0 ? null : points;
}

function getCurrentLineWidth(builder: any): number | null {
	const state = builder.map.stateChangeHandler.getItemValue('.uno:LineWidth');
	const hmm = parseInt(state, 10);
	return isNaN(hmm) || hmm < 0 ? null : hmmToPoints(hmm);
}

function applyLineWidth(builder: any, points: number): number {
	const hmm = pointsToHMM(points);
	builder.map.sendUnoCommand('.uno:LineWidth?LineWidth:long=' + hmm);
	return hmm;
}

function setWidthPreview(entry: HTMLElement, points: number) {
	const height = Math.min(
		PREVIEW_MAX_PX,
		Math.max(1, Math.round(points * PREVIEW_PX_PER_POINT)),
	);
	entry.style.setProperty('--ui-linewidth-preview-height', height + 'px');
}

function createLastCustomEntry(
	parentContainer: Element,
	data: any,
	builder: any,
	applyWidth: (points: number) => void,
) {
	const lastCustom = getLastCustomWidth();
	const name = _('Last Custom Value');

	const entry = window.L.DomUtil.create(
		'div',
		'ui-combobox-entry ui-linewidth-lastcustom ' + builder.options.cssClass,
		parentContainer,
	);
	entry.id = data.id + '-lastcustom';
	entry.setAttribute('role', 'option');
	entry.setAttribute('tabindex', '-1');
	entry.setAttribute('aria-selected', 'false');

	if (lastCustom !== null) setWidthPreview(entry, lastCustom);

	const text = window.L.DomUtil.create('span', '', entry);
	text.innerText = lastCustom !== null ? formatPoints(lastCustom) : name;

	if (lastCustom === null) {
		window.L.DomUtil.addClass(entry, 'disabled');
		entry.setAttribute('disabled', '');
		entry.setAttribute('aria-disabled', 'true');
	} else {
		entry.setAttribute('aria-label', name + ': ' + formatPoints(lastCustom));
	}

	const activate = function () {
		if (lastCustom === null) return;
		applyWidth(lastCustom);
		JSDialog.CloseAllDropdowns();
	};

	entry.addEventListener('click', activate);
	entry.addEventListener('keydown', function (event: KeyboardEvent) {
		if (event.key === 'Enter' || event.key === ' ') {
			activate();
			event.preventDefault();
		} else if (event.key === 'Tab') {
			JSDialog.CloseAllDropdowns();
			event.preventDefault();
		}
	});
}

JSDialog.LineWidthCustom = function (
	parentContainer: Element,
	data: any,
	builder: any,
): boolean {
	const applyWidth = function (points: number) {
		applyLineWidth(builder, points);
	};

	window.L.DomUtil.create(
		'hr',
		'jsdialog ui-separator horizontal ui-linewidth-section-separator',
		parentContainer,
	);

	createLastCustomEntry(parentContainer, data, builder, applyWidth);

	return false;
};

JSDialog.LineWidthField = function (
	parentContainer: Element,
	data: any,
	builder: any,
): boolean {
	const current = getCurrentLineWidth(builder);
	data.text = current !== null ? formatPoints(current) : '';

	let sent: ((value: number) => void) | null = null;

	JSDialog.spinfieldControl(
		parentContainer,
		data,
		builder,
		function (
			objectType: string,
			eventType: string,
			object: unknown,
			value: string,
		) {
			const points = parseFloat(value);
			if (isNaN(points) || points < 0) return;

			if (points > 0) window.prefs.set(LAST_CUSTOM_PREF, points);
			const hmm = applyLineWidth(builder, points);
			if (sent) sent(hmm);
		},
	);

	const container = parentContainer.querySelector('#' + data.id) as any;
	const spinfield = container
		? (container.querySelector('input') as HTMLInputElement)
		: null;
	if (!spinfield) return false;

	sent = JSDialog.followCommandState(
		builder,
		container,
		'.uno:LineWidth',
		function (state: string) {
			const hmm = parseInt(state, 10);
			if (isNaN(hmm) || hmm < 0) return;

			const points = hmmToPoints(hmm);
			JSDialog._setSpinFieldValue(
				spinfield,
				JSDialog._formatSpinFieldValue(points.toFixed(1), container._unit),
				points,
			);
		},
	);

	return false;
};
