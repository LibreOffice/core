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

/// The built page, dist/cool.html, loaded into its own jsdom window with all its scripts running
/// as a browser runs them. The server is a stand-in socket: what the page sends is recorded in
/// sent, and serverSays() delivers a message from the server.
interface DocumentPage {
	window: any;
	map: any;
	sent: string[];
	errors: string[];
	serverSays(message: string): void;
	waitFor<T>(test: () => T, what: string, timeoutMs?: number): Promise<T>;
	close(): void;
}

function documentPageDist(): string {
	const path = require('path');
	return process.env.COOL_DIST_DIR || path.join(__dirname, '..', '..', 'dist');
}

/// The page as wsd/FileServer.cpp fills it in for a plain browser load.
function documentPageHtml(dist: string): string {
	const fs = require('fs');
	const path = require('path');
	let html: string = fs.readFileSync(path.join(dist, 'cool.html'), 'utf8');
	html = html.replace(/%SERVICE_ROOT%/g, '').replace(/%VERSION%/g, 'dist');
	html = html.replace('<!--%CSS_VARIABLES%-->', '');
	html = html.replace('%UI_DEFAULTS%', Buffer.from('{}').toString('base64'));
	return html.replace(/%[A-Z_]+%/g, '');
}

const documentPagePrefix = '/browser/dist/';

function documentPageFile(dist: string, url: URL): string {
	const path = require('path');
	return path.join(
		dist,
		decodeURIComponent(url.pathname.slice(documentPagePrefix.length)),
	);
}

/// Serves the page's scripts from dist. Stylesheets are served empty.
function documentPageLoader(dist: string, errors: string[]): any {
	const fs = require('fs');
	const jsdom = require('jsdom');
	const loader = new jsdom.ResourceLoader();
	loader.fetch = (address: string) => {
		const url = new URL(address);
		if (
			!url.pathname.startsWith(documentPagePrefix) ||
			url.pathname.endsWith('.css')
		)
			return Promise.resolve(Buffer.from(''));
		try {
			return Promise.resolve(fs.readFileSync(documentPageFile(dist, url)));
		} catch (e) {
			errors.push('not in dist: ' + url.pathname);
			return Promise.resolve(Buffer.from(''));
		}
	};
	return loader;
}

/// The page's requests for files in dist are answered from dist, as the server would answer them.
/// Other requests go to the jsdom XMLHttpRequest.
function documentPageRequests(pageWindow: any, dist: string): void {
	const fs = require('fs');
	const NetworkRequest = pageWindow.XMLHttpRequest;
	pageWindow.XMLHttpRequest = function (this: any) {
		this.readyState = 0;
		this.status = 0;
		this.responseText = '';
		this.response = '';
		this.onreadystatechange = null;
		this.onload = null;
		this.network = null;
	};
	const request = pageWindow.XMLHttpRequest.prototype;
	request.open = function (method: string, address: string, async?: boolean) {
		const url = new URL(address, pageWindow.location.href);
		if (method === 'GET' && url.pathname.startsWith(documentPagePrefix)) {
			this.file = documentPageFile(dist, url);
			this.async = async !== false;
			this.readyState = 1;
			return;
		}
		this.network = new NetworkRequest();
		this.network.onreadystatechange = () => {
			for (const name of ['readyState', 'status', 'responseText', 'response'])
				this[name] = this.network[name];
			if (this.onreadystatechange) this.onreadystatechange();
			if (this.network.readyState === 4 && this.onload) this.onload();
		};
		this.network.open(method, address, async);
	};
	request.setRequestHeader = function (name: string, value: string) {
		if (this.network) this.network.setRequestHeader(name, value);
	};
	request.overrideMimeType = function () {};
	request.getResponseHeader = function (): null {
		return null;
	};
	request.abort = function () {
		if (this.network) this.network.abort();
	};
	request.send = function (body: any) {
		if (this.network) return this.network.send(body);
		const answer = () => {
			try {
				this.responseText = this.response = fs.readFileSync(this.file, 'utf8');
				this.status = 200;
			} catch (e) {
				this.status = 404;
			}
			this.readyState = 4;
			if (this.onreadystatechange) this.onreadystatechange();
			if (this.onload) this.onload();
		};
		if (this.async) pageWindow.setTimeout(answer, 0);
		else answer();
	};
}

/// A canvas draws nothing. Its 2D context takes every call and every setting.
function documentPageContext(pageWindow: any, canvas: any): any {
	const settings: any = {};
	return new Proxy(settings, {
		get(target, name) {
			if (name === 'canvas') return canvas;
			if (name in target) return target[name];
			if (typeof name === 'symbol') return undefined;
			if (name === 'measureText')
				return () => ({
					width: 0,
					actualBoundingBoxAscent: 0,
					actualBoundingBoxDescent: 0,
				});
			if (name === 'getImageData' || name === 'createImageData')
				return (x: number, y: number, width: number, height: number) => ({
					width: width || 0,
					height: height || 0,
					data: new Uint8ClampedArray(4 * (width || 0) * (height || 0)),
				});
			if (name === 'getTransform') return () => new pageWindow.DOMMatrix();
			if (name === 'getLineDash') return (): number[] => [];
			return () => ({ addColorStop() {} });
		},
		set(target, name, value) {
			target[name] = value;
			return true;
		},
	});
}

/// What jsdom does not have, each with the plainest behaviour a browser could show.
function documentPageGaps(pageWindow: any): void {
	pageWindow.matchMedia = () => ({
		matches: false,
		addEventListener() {},
		removeEventListener() {},
		addListener() {},
		removeListener() {},
	});
	pageWindow.ResizeObserver = function () {
		this.observe = function () {};
		this.unobserve = function () {};
		this.disconnect = function () {};
	};
	pageWindow.CSS = {
		escape: (value: any) => String(value).replace(/[^a-zA-Z0-9_ -￿-]/g, '\\$&'),
		supports: () => false,
	};
	pageWindow.HTMLCanvasElement.prototype.getContext = function () {
		return documentPageContext(pageWindow, this);
	};
	// An element counts as shown unless it or a parent has display none. The page has no
	// stylesheets, so only inline styles hide an element.
	pageWindow.HTMLElement.prototype.checkVisibility = function () {
		for (let element = this; element; element = element.parentElement)
			if (pageWindow.getComputedStyle(element).display === 'none') return false;
		return true;
	};
	// innerText is the text content, without the line breaks a browser adds from the layout.
	Object.defineProperty(pageWindow.HTMLElement.prototype, 'innerText', {
		get() {
			return this.textContent;
		},
		set(value: string) {
			this.textContent = value;
		},
		configurable: true,
	});
	pageWindow.HTMLElement.prototype.scrollIntoView = () => {};
}

function documentPageWait<T>(
	test: () => T,
	what: string,
	timeoutMs = 5000,
): Promise<T> {
	return new Promise((resolve, reject) => {
		const start = Date.now();
		const poll = () => {
			const value = test();
			if (value) return resolve(value);
			if (Date.now() - start > timeoutMs)
				return reject(new Error('timed out waiting for ' + what));
			setTimeout(poll, 10);
		};
		poll();
	});
}

/// The status of a loaded document with one page, sheet or slide, with the fields the kit sends
/// for that document type.
function documentPageStatus(docType: string): any {
	const status: any = {
		type: docType,
		partscount: 1,
		selectedpart: '0',
		width: 12240,
		height: 15840,
		viewid: 1,
		readonly: false,
		mode: 0,
		partHasComments: false,
	};
	if (docType === 'text') {
		status.pagerectangles = [[0, 0, 12240, 15840]];
	} else if (docType === 'spreadsheet') {
		status.lastcolumn = 0;
		status.lastrow = 0;
		status.parts = [
			{
				visible: 1,
				rtllayout: 0,
				protected: 0,
				selected: 1,
				name: 'Sheet1',
				hash: '0',
				lastcolumn: 0,
				lastrow: 0,
			},
		];
	} else {
		// A slide is named by its part identifier, both in selectedpart and in its own part
		// info.
		status.selectedpart = 'slide-1';
		status.parts = [
			{
				masterPageCount: 1,
				mode: 0,
				gridSnapEnabled: false,
				gridVisible: false,
				visible: 1,
				selected: 1,
				width: 12240,
				height: 15840,
				leftBorder: 0,
				rightBorder: 0,
				upperBorder: 0,
				lowerBorder: 0,
				name: 'Slide 1',
				realName: '',
				part: 'slide-1',
			},
		];
	}
	return status;
}

/// Loads the page for a document of the given type, as getDocType() names it, and plays the
/// server's side of the load. Resolves once the map has its document layer.
async function openDocumentPage(docType: string): Promise<DocumentPage> {
	const jsdom = require('jsdom');
	const dist = documentPageDist();
	const errors: string[] = [];
	const sent: string[] = [];
	let socket: any = null;

	const virtualConsole = new jsdom.VirtualConsole();
	virtualConsole.on('jsdomError', (error: Error) => {
		// jsdom reports the browser features it lacks, such as navigation, as not implemented.
		if (/Not implemented/.test(error.message)) return;
		errors.push(error.message);
	});

	const dom = new jsdom.JSDOM(documentPageHtml(dist), {
		url: 'http://localhost:9980/browser/dist/cool.html?file_path=/test/document&lang=en-US',
		runScripts: 'dangerously',
		resources: documentPageLoader(dist, errors),
		pretendToBeVisual: true,
		virtualConsole,
		beforeParse(pageWindow: any) {
			documentPageGaps(pageWindow);
			documentPageRequests(pageWindow, dist);
			pageWindow.WebSocket = function (this: any, url: string) {
				this.url = url;
				this.readyState = 0;
				this.send = (message: any) => {
					sent.push(typeof message === 'string' ? message : '<binary>');
				};
				this.close = () => {
					this.readyState = 3;
				};
				socket = this;
			};
			pageWindow.WebSocket.OPEN = 1;
		},
	});
	const pageWindow = dom.window;

	// A page that fails to load is closed, so its timers stop and the test run can end.
	try {
		await new Promise((resolve) =>
			pageWindow.addEventListener('load', resolve),
		);
		await documentPageWait(() => socket, 'the page to open its socket');

		const serverSays = (message: string) => socket.onmessage({ data: message });
		socket.readyState = 1;
		socket.onopen({});
		serverSays(
			'coolserver {"Version":"test","Hash":"0","Protocol":"0.1","Id":"test"}',
		);
		serverSays(
			'lokitversion {"ProductName":"test","ProductVersion":"0","ProductExtension":"","BuildId":"0"}',
		);
		serverSays('status: ' + JSON.stringify(documentPageStatus(docType)));
		serverSays(
			'viewinfo: [{"id":1,"userid":"test","username":"Test User","readonly":"0","parked":"false","color":0}]',
		);
		await documentPageWait(
			() => pageWindow.app.map._docLayer,
			'the document layer',
		);

		return {
			window: pageWindow,
			map: pageWindow.app.map,
			sent,
			errors,
			serverSays,
			waitFor: documentPageWait,
			close: () => pageWindow.close(),
		};
	} catch (error) {
		pageWindow.close();
		if (errors.length)
			error.message += '\nThe page reported:\n' + errors.join('\n');
		throw error;
	}
}
