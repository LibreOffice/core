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

// A map with what the AI chat sidebar and the JSDialog builder read from it. The recorders keep
// their calls in settingsDialogCalls and snackbarCalls.
function makeAIChatMap(): any {
	const map: any = new Evented();
	map.docType = 'text';
	map.numberOfParts = 1;
	map.isAIConfigured = true;
	map.aiModelName = '';
	map.aiRequestTimeout = 0;
	map.aiEthicalRating = '';
	map.getDocType = () => map.docType;
	map.getNumberOfParts = () => map.numberOfParts;
	map.editorHasFocus = () => true;
	map.focus = () => {};
	map.hideRestrictedItems = () => {};
	map.disableLockedItem = () => {};
	map.dialog = {
		hasOpenedDialog: () => false,
		blinkOpenDialog: () => {},
	};
	map.settingsDialogCalls = [] as any[];
	map.settings = {
		showSettingsDialog: (...args: any[]) => map.settingsDialogCalls.push(args),
	};
	map.snackbarCalls = [] as any[];
	map.uiManager = {
		showSnackbar: (...args: any[]) => map.snackbarCalls.push(args),
	};
	map._debug = null;
	return map;
}

// The messages sent to the server, in order.
let aiChatSentMessages: string[] = [];

// The aichat: requests sent so far, parsed.
function sentAIRequests(): any[] {
	return aiChatSentMessages
		.filter((message) => message.startsWith('aichat: '))
		.map((message) => JSON.parse(message.substring('aichat: '.length)));
}

// The aichatapprove: answers sent so far, parsed.
function sentAIApprovals(): any[] {
	return aiChatSentMessages
		.filter((message) => message.startsWith('aichatapprove: '))
		.map((message) => JSON.parse(message.substring('aichatapprove: '.length)));
}

// Runs every layouting task and every drain callback, until none is left.
function drainLayout(): void {
	const service: any = app.layoutingService;
	for (let round = 0; round < 100; round++) {
		while (service.hasTasksPending()) service.runTheTopTask();
		if (!service._drainCallbacks.length) return;
		service.triggerDrainCallbacks();
	}
	throw new Error('layout does not settle');
}

// Lets the pending promise callbacks run, then drains the layout.
async function settleAIChat(): Promise<void> {
	for (let round = 0; round < 3; round++) {
		for (let tick = 0; tick < 10; tick++) await Promise.resolve();
		drainLayout();
	}
}

interface AIChatPageState {
	map: any;
	layoutingService: any;
	localeService: any;
	socket: any;
	setImage: any;
	requestAnimationFrame: any;
	cancelAnimationFrame: any;
}

let aiChatSaved: AIChatPageState | null = null;

// Puts the sidebar panel in the page and a fresh map, socket and layout service on app.
// requestAnimationFrame runs on the timers, so the fake clock decides when a frame runs.
function setUpAIChatPage(): any {
	aiChatSaved = {
		map: app.map,
		layoutingService: app.layoutingService,
		localeService: app.localeService,
		socket: app.socket,
		setImage: app.LOUtil.setImage,
		requestAnimationFrame: (globalThis as any).requestAnimationFrame,
		cancelAnimationFrame: (globalThis as any).cancelAnimationFrame,
	};

	const map = makeAIChatMap();
	app.map = map;
	app.layoutingService = new LayoutingService();
	app.localeService = { initializeNumberFormatting: () => {} } as any;
	aiChatSentMessages = [];
	app.socket = {
		sendMessage: (message: string) => aiChatSentMessages.push(message),
	} as any;
	app.LOUtil.setImage = () => {};
	(globalThis as any).requestAnimationFrame = (callback: any) =>
		setTimeout(() => callback(0), 16);
	(globalThis as any).cancelAnimationFrame = (id: any) => clearTimeout(id);

	const wrapper = document.createElement('div');
	wrapper.id = 'aichat-dock-wrapper';
	const panel = document.createElement('div');
	panel.id = 'aichat-panel';
	wrapper.appendChild(panel);
	document.body.appendChild(wrapper);

	return map;
}

function tearDownAIChatPage(): void {
	document.getElementById('aichat-dock-wrapper')?.remove();
	if (!aiChatSaved) return;
	app.map = aiChatSaved.map;
	app.layoutingService = aiChatSaved.layoutingService;
	app.localeService = aiChatSaved.localeService;
	app.socket = aiChatSaved.socket;
	app.LOUtil.setImage = aiChatSaved.setImage;
	(globalThis as any).requestAnimationFrame = aiChatSaved.requestAnimationFrame;
	(globalThis as any).cancelAnimationFrame = aiChatSaved.cancelAnimationFrame;
	aiChatSaved = null;
}
