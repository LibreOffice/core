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

// The LibreLogo sidebar panel: editor, command line, turtle preview and
// output. The program runs in a Web Worker (logo/worker.js); the resulting
// drawing goes into the document through kit.js.

import { KitBridge } from './kit.js';
import { LogoLocale } from './logo/l10n.js';
import { DEFAULT_PAGE } from './logo/turtle.js';
import { LIBRELOGO_NS } from './logo/svg.js';
import { EXAMPLES } from './logo/examples.js';

const $ = (id) => document.getElementById(id);

// The sidebar panel: wires the editor, command line, preview and options to the
// Web Worker that runs programs and the KitBridge that draws into the document.
class Panel {
	constructor() {
		this.bridge = new KitBridge();
		this.worker = null;
		this.running = false;
		this.state = null; // last turtle snapshot, restored after Stop
		this.lastFrame = null;
		this.dirty = false;
		this.table = null;
		this.locale = null;
		// The UI language localizes the panel's own chrome (buttons, messages).
		// The Logo language, which decides the keyword set the program is read
		// in, tracks the document instead and is resolved in init(); until then
		// it falls back to the UI language.
		this.uiLang = (window.cool && window.cool.lang) || navigator.language || 'en-US';
		this.logoLang = this.uiLang;
		this.page = DEFAULT_PAGE.slice();
		this.queued = [];
	}

	async init() {
		// Translations for the panel's own strings; the program language
		// tables are loaded separately below.
		await window.cool.l10n.ready;
		this.bindUi();
		this.populateExamples();
		const resp = await fetch('l10n/logo-l10n.json');
		this.table = await resp.json();
		await this.refreshLogoLanguage();
		await this.refreshPageSize();
		this.startWorker();
		// Commands from the Logo notebookbar tab / menu arrive from the host:
		window.cool.onCommand = (id) => this.command(id);
		this.flushQueued();
	}

	// Read the document's language and, if the Logo language changed, rebuild
	// the panel's locale and bring the worker up on the new keyword set. Called
	// at startup and before each full program run, so switching the document
	// language takes effect on the next run, as it does on the desktop.
	async refreshLogoLanguage() {
		let docLang = null;
		try {
			docLang = await this.bridge.documentLanguage();
		} catch (e) {
			// no document language available: keep the UI language
		}
		const lang = docLang || this.uiLang;
		const locale = new LogoLocale(this.table, lang);
		if (this.locale && locale.lang === this.locale.lang) return;
		this.logoLang = lang;
		this.locale = locale;
		this.localizeUi();
		if (this.worker) this.startWorker();
	}

	// Read the page size and, if it changed, update the preview shape and bring
	// the worker up on the new page. Called at startup and before each run, so
	// switching orientation or page size is picked up, like the language is.
	async refreshPageSize() {
		let size = null;
		try {
			size = await this.bridge.pageSize();
		} catch (e) {
			this.log('info', cool._('Using A4 page size: %1').replace('%1', e.message));
		}
		if (!Array.isArray(size) || size.length !== 2 || !(size[0] > 0) || !(size[1] > 0)) return;
		if (this.page[0] === size[0] && this.page[1] === size[1]) return;
		this.page = size;
		$('preview').style.aspectRatio = this.page[0] + ' / ' + this.page[1];
		if (this.worker) this.startWorker();
	}

	localizeUi() {
		const L = this.locale;
		const up = (key) => L.localName(key).toUpperCase();
		$('btn-fd').title = up('FORWARD') + ' 10';
		$('btn-bk').title = up('BACKWARD') + ' 10';
		$('btn-lt').title = up('TURNLEFT') + ' 15';
		$('btn-rt').title = up('TURNRIGHT') + ' 15';
		$('btn-home').title = up('HOME');
		$('btn-cs').title = up('CLEARSCREEN');
		$('commandline').placeholder = up('FORWARD') + ' 100 ' + up('TURNLEFT') + ' 90';
		$('status-lang').textContent = L.lang.replace('_', '-');
	}

	startWorker() {
		if (this.worker) this.worker.terminate();
		this.worker = new Worker('logo/worker.js', { type: 'module' });
		this.worker.onmessage = (e) => this.onWorkerMessage(e.data);
		this.worker.onerror = (e) => {
			this.log('error', cool._('Internal error: %1').replace('%1', e.message || e));
			this.setRunning(false);
		};
		this.worker.postMessage({ type: 'init', lang: this.logoLang, table: this.table, pageSize: this.page, state: this.state });
	}

	// --- UI wiring -------------------------------------------------------------

	bindUi() {
		$('btn-run').onclick = () => this.runProgram();
		$('btn-stop').onclick = () => this.stop();
		$('btn-fd').onclick = () => this.command('forward');
		$('btn-bk').onclick = () => this.command('back');
		$('btn-lt').onclick = () => this.command('left');
		$('btn-rt').onclick = () => this.command('right');
		$('btn-home').onclick = () => this.command('home');
		$('btn-cs').onclick = () => this.command('clearscreen');
		$('btn-cmd').onclick = () => this.runCommandLine();
		$('commandline').onkeydown = (e) => {
			if (e.key === 'Enter') {
				e.preventDefault();
				this.runCommandLine();
			}
		};
		$('editor').onkeydown = (e) => {
			if (e.key === 'Enter' && (e.ctrlKey || e.metaKey)) {
				e.preventDefault();
				this.runProgram();
			}
			if (e.key === 'Tab') {
				e.preventDefault();
				const ta = e.target;
				const s = ta.selectionStart;
				ta.setRangeText('\t', s, ta.selectionEnd, 'end');
			}
		};
		$('btn-load').onclick = () => this.loadFromDocument();
		$('btn-save').onclick = () => this.saveToDocument();
		$('examples').onchange = (e) => this.loadExample(e.target.value);
		$('btn-from-svg').onclick = (e) => {
			e.stopPropagation();
			$('svg-file').click();
		};
		$('svg-file').onchange = (e) => {
			const file = e.target.files[0];
			if (file) this.loadFromSvg(file);
			e.target.value = '';
		};
		const editor = $('editor');
		editor.ondragover = (e) => {
			if (e.dataTransfer && Array.from(e.dataTransfer.items || []).some((i) => i.kind === 'file')) {
				e.preventDefault();
				e.dataTransfer.dropEffect = 'copy';
			}
		};
		editor.ondrop = (e) => {
			const file = e.dataTransfer && e.dataTransfer.files[0];
			if (file && /\.svg$|svg/i.test(file.type || file.name)) {
				e.preventDefault();
				this.loadFromSvg(file);
			}
		};
		$('btn-input-ok').onclick = () => this.answerInput($('input-value').value);
		$('btn-input-cancel').onclick = () => this.answerInput(null);
		$('input-value').onkeydown = (e) => {
			if (e.key === 'Enter') this.answerInput($('input-value').value);
			if (e.key === 'Escape') this.answerInput(null);
		};
		$('btn-remove').onclick = async () => {
			try {
				const r = await this.bridge.removeDrawing();
				this.log('info', r && r[0] ? cool._('Drawing removed from the document.') : cool._('No LibreLogo drawing in the document.'));
			} catch (e) {
				this.log('error', e.message);
			}
		};
	}

	setRunning(on) {
		this.running = on;
		$('btn-run').disabled = on;
		$('btn-stop').disabled = !on;
		$('btn-cmd').disabled = on;
		document.body.classList.toggle('running', on);
	}

	log(kind, text) {
		const out = $('output');
		const line = document.createElement('div');
		if (kind) line.className = kind;
		line.textContent = text;
		out.appendChild(line);
		while (out.childElementCount > 500) out.removeChild(out.firstChild);
		out.scrollTop = out.scrollHeight;
	}

	// --- commands from the notebookbar, menus and toolbar -------------------------

	command(id) {
		if (!this.locale) {
			this.queued.push(id);
			return;
		}
		const up = (key) => this.locale.localName(key).toUpperCase();
		switch (id) {
		case 'run': this.runProgram(); break;
		case 'stop': this.stop(); break;
		case 'home': this.runSource(up('HOME'), true); break;
		case 'clearscreen': this.runSource(up('CLEARSCREEN'), true); break;
		case 'forward': this.runSource(up('FORWARD') + ' 10', true); break;
		case 'back': this.runSource(up('BACKWARD') + ' 10', true); break;
		case 'left': this.runSource(up('TURNLEFT') + ' 15', true); break;
		case 'right': this.runSource(up('TURNRIGHT') + ' 15', true); break;
		default: break;
		}
	}

	flushQueued() {
		const q = this.queued;
		this.queued = [];
		for (const id of q) this.command(id);
	}

	// --- running -------------------------------------------------------------------

	async runProgram() {
		if (this.running) return;
		await this.refreshLogoLanguage();
		await this.refreshPageSize();
		let source = $('editor').value;
		if (!source.trim()) {
			try {
				const r = await this.bridge.programText();
				source = r.text || '';
				if (!source.trim()) {
					this.log('info', cool._('The document is empty. Type a program in the editor or in the document.'));
					return;
				}
				this.log('info', r.selection ? cool._('Running the selected text.') : cool._('Running the document text.'));
			} catch (e) {
				this.log('error', e.message);
				return;
			}
		}
		this.runSource(source, false);
	}

	runCommandLine() {
		const cmd = $('commandline').value;
		if (!cmd.trim() || this.running) return;
		this.runSource(cmd, true);
	}

	runSource(source, isCommand) {
		if (this.running) return;
		this.setRunning(true);
		this.dirty = false;
		this.isCommand = isCommand;
		this.worker.postMessage({ type: 'run', source });
	}

	stop() {
		if (!this.running) return;
		this.log('info', this.locale.message('ERR_STOP'));
		this.hideInput();
		// Kill the worker and bring a fresh one up with the last known state.
		this.startWorker();
		this.setRunning(false);
		this.maybeInsert();
	}

	onWorkerMessage(msg) {
		switch (msg.type) {
		case 'ready':
			break;
		case 'print':
			this.log('', msg.text);
			break;
		case 'input':
			this.showInput(msg.prompt);
			break;
		case 'export':
			// A named ".svg" PICTURE: hand the file to the host to save the
			// platform way (download or WOPI host in Online, filesystem in CODA).
			this.exportSvg(msg.name, msg.svg);
			break;
		case 'frame':
			this.lastFrame = msg;
			this.state = msg.state;
			this.dirty = true;
			$('preview').innerHTML = msg.svg;
			if (msg.view) $('preview').style.aspectRatio = msg.view[2] + ' / ' + msg.view[3];
			$('status-pos').textContent = Math.round(msg.position[0]) + ', ' + Math.round(msg.position[1]) + ' ' + Math.round(msg.heading) + '°';
			break;
		case 'error':
			this.log('error', msg.message);
			if (msg.line && !this.isCommand) this.highlightLine(msg.line);
			break;
		case 'done':
			this.setRunning(false);
			this.hideInput();
			this.maybeInsert();
			break;
		default:
			break;
		}
	}

	highlightLine(line) {
		const ta = $('editor');
		if (!ta.value.trim()) return;
		const lines = ta.value.split('\n');
		let start = 0;
		for (let i = 0; i < line - 1 && i < lines.length; i++) start += lines[i].length + 1;
		const end = start + (lines[line - 1] || '').length;
		ta.focus();
		ta.setSelectionRange(start, end);
	}

	showInput(prompt) {
		$('input-prompt').textContent = prompt;
		$('input-value').value = '';
		$('input-row').hidden = false;
		$('input-value').focus();
	}

	hideInput() {
		$('input-row').hidden = true;
	}

	answerInput(value) {
		if ($('input-row').hidden) return;
		this.hideInput();
		this.worker.postMessage({ type: 'input', value });
	}

	// --- document -------------------------------------------------------------------

	async maybeInsert() {
		if (!$('opt-insert').checked || !this.dirty || !this.lastFrame) return;
		const frame = this.lastFrame;
		const bounds = frame.bounds;
		// The worker sends a turtle-less render of the final frame; use it when
		// the turtle should not appear in the inserted drawing.
		const svg = $('opt-turtle').checked ? frame.svg : (frame.svgNoTurtle || frame.svg);
		if (!bounds) {
			// nothing drawn: remove any previous drawing
			try {
				await this.bridge.removeDrawing();
			} catch (e) {
				this.log('error', e.message);
			}
			return;
		}
		try {
			await this.bridge.insertDrawing(svg, bounds, frame.page, $('opt-mode').value);
			this.dirty = false;
		} catch (e) {
			this.log('error', cool._('Could not insert the drawing: %1').replace('%1', e.message));
		}
	}

	// Save a PICTURE "name.svg" export. Only the base name is used; the host
	// decides where the file goes.
	async exportSvg(name, svg) {
		const base = String(name).replace(/^.*[\\/]/, '') || 'picture.svg';
		try {
			const how = await window.cool.saveFile(base, svg, 'image/svg+xml');
			this.log('info', how === 'download'
				? cool._('Downloaded %1').replace('%1', base)
				: cool._('Saved %1').replace('%1', base));
		} catch (e) {
			this.log('error', cool._('Could not save %1: %2').replace('%1', base).replace('%2', e.message));
		}
	}

	// --- examples -------------------------------------------------------------------

	populateExamples() {
		const sel = $('examples');
		for (const ex of EXAMPLES) {
			const opt = document.createElement('option');
			opt.value = ex.id;
			opt.textContent = this.exampleName(ex.id);
			sel.appendChild(opt);
		}
	}

	// Menu labels for the examples. Kept as literal cool._() calls so the pot
	// extractor picks them up; the sources themselves stay untranslated.
	exampleName(id) {
		switch (id) {
		case 'star': return cool._('Star');
		case 'spiral': return cool._('Square spiral');
		case 'rosette': return cool._('Rosette');
		case 'tree': return cool._('Tree');
		case 'animation': return cool._('Animation');
		case 'beaver': return cool._('Beaver');
		default: return id;
		}
	}

	// Load the chosen example into the editor and run it. The picker returns to
	// its placeholder so the same example can be loaded again.
	loadExample(id) {
		$('examples').value = '';
		if (!id || this.running) return;
		const ex = EXAMPLES.find((e) => e.id === id);
		if (!ex) return;
		$('editor').value = ex.source;
		$('editor-box').open = true;
		this.runProgram();
	}

	// Recover the program embedded in an exported picture: read the
	// librelogo:source metadata that render() writes and put it in the editor.
	async loadFromSvg(file) {
		try {
			const text = await file.text();
			const doc = new DOMParser().parseFromString(text, 'image/svg+xml');
			if (doc.getElementsByTagName('parsererror').length) {
				this.log('error', cool._('%1 is not a valid SVG file.').replace('%1', file.name));
				return;
			}
			const el = doc.getElementsByTagNameNS(LIBRELOGO_NS, 'source')[0];
			if (!el || !el.textContent.trim()) {
				this.log('info', cool._('%1 has no embedded LibreLogo program.').replace('%1', file.name));
				return;
			}
			$('editor').value = el.textContent;
			$('editor-box').open = true;
			this.log('info', cool._('Loaded the program from %1.').replace('%1', file.name));
		} catch (e) {
			this.log('error', cool._('Could not read %1: %2').replace('%1', file.name).replace('%2', e.message));
		}
	}

	async loadFromDocument() {
		try {
			const r = await this.bridge.programText();
			$('editor').value = r.text || '';
			this.log('info', r.selection ? cool._('Loaded the selected text.') : cool._('Loaded the document text.'));
		} catch (e) {
			this.log('error', e.message);
		}
	}

	async saveToDocument() {
		const text = $('editor').value;
		if (!text.trim()) return;
		try {
			await window.cool.callRemote(function (program) {
				const doc = cool.getActiveDocument();
				doc.getCursor().insertText(program);
			}, text);
			this.log('info', cool._('Program written to the document.'));
		} catch (e) {
			this.log('error', e.message);
		}
	}
}

const panel = new Panel();
panel.init().catch((e) => panel.log('error', cool._('Could not start LibreLogo: %1').replace('%1', e.message)));
