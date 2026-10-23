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
	host: any;
	serviceRoot: any;
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
		host: (window as any).host,
		serviceRoot: (window as any).serviceRoot,
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
	// With no host and no service root, image URLs are relative, as in the desktop app.
	(window as any).host = '';
	(window as any).serviceRoot = '';

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
	(window as any).host = aiChatSaved.host;
	(window as any).serviceRoot = aiChatSaved.serviceRoot;
	aiChatSaved = null;
}

interface AIServerOptions {
	content?: string;
	success?: boolean;
	error?: string;
	// With a tool name, each request first asks for approval to run that tool.
	approvalToolName?: string;
	approvalTransformJson?: string;
	// With an outline, each request is answered with that deck outline.
	outline?: { title: string; slides: any[] };
	progress?: string;
}

// Lands the final result of an approved outline. Set when the outline is approved.
let deliverOutlineResult: () => void = () => {};

// Answers the aichat: requests the way the AI server does. A reply arrives after the layout
// drains, and the cancel messages are dropped.
function answerAIRequests(options: AIServerOptions = {}): void {
	const content = options.content || 'Mock AI response';
	const success = options.success !== undefined ? options.success : true;
	const later = (fn: () => void) => app.layoutingService.onDrain(fn);
	let pendingRequestId = '';
	deliverOutlineResult = () => {};

	const fireResult = (requestId: string) => {
		const result: any = { requestId: requestId, success: success };
		if (success) result.content = content;
		else result.error = options.error || 'Mock error';
		app.map.fire('aichatresult', result);
	};

	app.socket.sendMessage = (message: string) => {
		aiChatSentMessages.push(message);
		if (message.startsWith('aichat: ')) {
			const requestId = JSON.parse(
				message.substring('aichat: '.length),
			).requestId;
			pendingRequestId = requestId;
			later(() => {
				if (options.outline)
					app.map.fire('aichatoutline', {
						requestId: requestId,
						title: options.outline.title,
						slides: options.outline.slides,
					});
				else if (options.approvalToolName)
					app.map.fire('aichatapproval', {
						requestId: requestId,
						toolName: options.approvalToolName,
						summary: 'Mock change',
						transformJson: options.approvalTransformJson || '',
					});
				else fireResult(requestId);
			});
		} else if (message.startsWith('aichatapprove: ')) {
			const decision = JSON.parse(message.substring('aichatapprove: '.length));
			const requestId = pendingRequestId;
			if (!options.outline) later(() => fireResult(requestId));
			else if (decision.action === 'approve') {
				later(() =>
					app.map.fire('aichatprogress', {
						requestId: requestId,
						status: options.progress || 'Building slide 1 of 2...',
					}),
				);
				deliverOutlineResult = () => fireResult(requestId);
			}
		}
	};
}

// Opens or closes the sidebar, as the aichat dispatch does.
async function toggleAIChat(sidebar: any): Promise<void> {
	sidebar.toggle();
	await settleAIChat();
}

// Closes the open sidebar and opens it again. The dock is checked to be off the screen in
// between.
async function reopenAIChat(sidebar: any): Promise<void> {
	await toggleAIChat(sidebar);
	nodeassert.strictEqual(
		document.querySelectorAll('#aichat-dock-wrapper.visible').length,
		0,
	);
	await toggleAIChat(sidebar);
	nodeassert.strictEqual(
		document.querySelectorAll('#aichat-dock-wrapper.visible').length,
		1,
	);
}

// Types into an input or a textarea as a user does: the text is added to the value, and the
// input, keyup and change events follow.
function typeInto(element: Element, text: string): void {
	const field = element as HTMLInputElement;
	field.value += text;
	for (const type of ['input', 'keyup', 'change'])
		field.dispatchEvent(new (window as any).Event(type, { bubbles: true }));
}

function clearField(element: Element): void {
	const field = element as HTMLInputElement;
	field.value = '';
	field.dispatchEvent(new (window as any).Event('input', { bubbles: true }));
}

async function clickOn(selector: string): Promise<void> {
	const element = document.querySelector(selector) as HTMLElement;
	if (!element) throw new Error('nothing matches ' + selector);
	element.click();
	await settleAIChat();
}

// Opens the sidebar, types the request and sends it.
async function sendAIRequest(sidebar: any, text: string): Promise<void> {
	if (!sidebar.isVisible()) await toggleAIChat(sidebar);
	typeInto(document.querySelector('#aichat-input.ui-textarea'), text);
	await clickOn('#aichat-send-btn button');
}

function aiMessagesText(): string {
	const list = document.getElementById('aichat-messages-list');
	return list ? list.textContent : '';
}

// Presses a key in the element, as keydown, and lets the sidebar react.
async function pressKey(
	selector: string,
	key: string,
	shiftKey: boolean = false,
): Promise<void> {
	const element = document.querySelector(selector);
	if (!element) throw new Error('nothing matches ' + selector);
	element.dispatchEvent(
		new (window as any).KeyboardEvent('keydown', {
			key: key,
			shiftKey: shiftKey,
			bubbles: true,
			cancelable: true,
		}),
	);
	await settleAIChat();
}
