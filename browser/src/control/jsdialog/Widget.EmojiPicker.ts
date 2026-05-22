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
 * JSDialog.EmojiPicker - inline emoji picker with search, categories,
 * and a persisted "Recently used" row.
 *
 * Example JSON:
 * {
 *     id: 'my-emoji-picker',
 *     type: 'emojipicker',
 *     recentsPrefsKey: 'myFeature.emojiRecents', // optional
 *     maxRecents: 16,                            // optional
 *     searchPlaceholder: 'Search emojis',        // optional
 * }
 *
 * Fires builder.callback('emojipicker', 'change', container, emoji, builder)
 * when the user picks an emoji. The picked emoji is also pushed to the head
 * of the recents list (deduped, capped to maxRecents) and persisted via
 * window.prefs.
 */

declare var JSDialog: any;

interface EmojiEntry {
	e: string; // the emoji codepoint
	k: string; // space-separated search keywords (all lowercase)
}

interface EmojiCategory {
	id: string;
	label: string;
	emojis: EmojiEntry[];
}

class EmojiPickerWidget {
	private static readonly DEFAULT_RECENTS_KEY = 'emojiPickerRecents';
	private static readonly DEFAULT_MAX_RECENTS = 16;

	// Curated catalogue. Order within each category roughly follows
	// common-use frequency rather than Unicode order, so the most likely
	// picks are visible first. Each entry carries lowercase search keywords.
	private static readonly CATEGORIES: EmojiCategory[] = [
		{
			id: 'smileys',
			label: _('Smileys'),
			emojis: [
				{ e: '😊', k: 'smile happy blush' },
				{ e: '😀', k: 'grin smile happy face' },
				{ e: '😁', k: 'grin smile happy beam' },
				{ e: '😂', k: 'laugh cry tears joy lol' },
				{ e: '🤣', k: 'rofl laugh roll floor' },
				{ e: '😅', k: 'sweat smile relief nervous' },
				{ e: '😉', k: 'wink flirt' },
				{ e: '😍', k: 'love heart eyes adore' },
				{ e: '🥰', k: 'love hearts smile adore' },
				{ e: '😘', k: 'kiss love' },
				{ e: '😎', k: 'cool sunglasses shades' },
				{ e: '🤩', k: 'star eyes excited wow' },
				{ e: '🤔', k: 'think thinking hmm' },
				{ e: '😏', k: 'smirk sly' },
				{ e: '😌', k: 'relieved calm content' },
				{ e: '😴', k: 'sleep tired zzz' },
				{ e: '🤤', k: 'drool yum food' },
				{ e: '🙃', k: 'upside down silly' },
				{ e: '🫠', k: 'melt melting overwhelmed' },
				{ e: '😇', k: 'angel halo innocent' },
				{ e: '🥳', k: 'party celebrate birthday' },
				{ e: '🥺', k: 'pleading please beg' },
				{ e: '😢', k: 'cry sad tear' },
				{ e: '😭', k: 'sob cry sad loud' },
				{ e: '😤', k: 'huff steam frustrated' },
				{ e: '😡', k: 'angry mad red' },
				{ e: '🤬', k: 'angry swear curse' },
				{ e: '😱', k: 'scream shock fear' },
				{ e: '🥶', k: 'cold freeze ice' },
				{ e: '🥵', k: 'hot sweat heat' },
				{ e: '🤯', k: 'mind blown exploding' },
				{ e: '😬', k: 'grimace awkward' },
				{ e: '🙄', k: 'roll eyes annoyed' },
				{ e: '😑', k: 'expressionless meh blank' },
				{ e: '😐', k: 'neutral meh poker' },
				{ e: '😶', k: 'silent no mouth blank' },
				{ e: '🤐', k: 'zip mouth quiet' },
				{ e: '🤫', k: 'shh quiet hush' },
				{ e: '🤥', k: 'lying pinocchio' },
				{ e: '😷', k: 'sick mask flu' },
			],
		},
		{
			id: 'people',
			label: _('People & gestures'),
			emojis: [
				{ e: '👋', k: 'wave hi hello bye' },
				{ e: '🤝', k: 'handshake deal agreement' },
				{ e: '👍', k: 'thumbs up like yes good' },
				{ e: '👎', k: 'thumbs down dislike no bad' },
				{ e: '👏', k: 'clap applause bravo' },
				{ e: '🙌', k: 'raise hands praise celebrate' },
				{ e: '🙏', k: 'pray please thanks namaste' },
				{ e: '💪', k: 'muscle strong flex' },
				{ e: '✍️', k: 'write writing pen' },
				{ e: '🤞', k: 'fingers crossed luck hope' },
				{ e: '✌️', k: 'peace victory' },
				{ e: '🤘', k: 'rock metal horns' },
				{ e: '🤟', k: 'love you sign hand' },
				{ e: '👌', k: 'ok okay perfect' },
				{ e: '🤌', k: 'pinched fingers italian' },
				{ e: '🫰', k: 'finger heart love money' },
				{ e: '🫶', k: 'heart hands love' },
				{ e: '🫡', k: 'salute respect' },
				{ e: '🤲', k: 'open hands palms cup' },
				{ e: '👀', k: 'eyes look watch see' },
				{ e: '🧠', k: 'brain smart mind think' },
				{ e: '👶', k: 'baby infant' },
				{ e: '🧒', k: 'child kid' },
				{ e: '👦', k: 'boy child' },
				{ e: '👧', k: 'girl child' },
				{ e: '🧑', k: 'person adult' },
				{ e: '👨', k: 'man male' },
				{ e: '👩', k: 'woman female' },
				{ e: '🧓', k: 'older person elder' },
				{ e: '👴', k: 'old man grandpa elder' },
			],
		},
		{
			id: 'objects',
			label: _('Objects'),
			emojis: [
				{ e: '💼', k: 'briefcase work business job' },
				{ e: '📝', k: 'note memo write document' },
				{ e: '📚', k: 'books study library learn' },
				{ e: '📖', k: 'book open read' },
				{ e: '📰', k: 'newspaper news' },
				{ e: '✏️', k: 'pencil write edit' },
				{ e: '✒️', k: 'pen nib write ink' },
				{ e: '🖊️', k: 'pen write' },
				{ e: '🖋️', k: 'fountain pen write ink' },
				{ e: '📌', k: 'pin pushpin' },
				{ e: '📎', k: 'paperclip attach' },
				{ e: '🖇️', k: 'paperclips link attach' },
				{ e: '📂', k: 'folder open files' },
				{ e: '📁', k: 'folder files' },
				{ e: '🗂️', k: 'dividers organize files' },
				{ e: '📅', k: 'calendar date day' },
				{ e: '📆', k: 'calendar tear off date' },
				{ e: '🗒️', k: 'spiral notepad notes' },
				{ e: '🗓️', k: 'spiral calendar' },
				{ e: '📊', k: 'bar chart stats data' },
				{ e: '📈', k: 'chart up trending growth' },
				{ e: '📉', k: 'chart down trending loss' },
				{ e: '💻', k: 'laptop computer code' },
				{ e: '🖥️', k: 'desktop computer monitor' },
				{ e: '⌨️', k: 'keyboard type' },
				{ e: '🖱️', k: 'mouse computer click' },
				{ e: '🖨️', k: 'printer print' },
				{ e: '📞', k: 'phone call telephone' },
				{ e: '📱', k: 'mobile phone cell smartphone' },
				{ e: '💾', k: 'save floppy disk' },
				{ e: '💡', k: 'idea bulb light' },
				{ e: '🔦', k: 'flashlight torch light' },
				{ e: '🔑', k: 'key lock' },
				{ e: '🔒', k: 'lock secure private' },
				{ e: '🔓', k: 'unlock open' },
				{ e: '🔨', k: 'hammer build tool' },
				{ e: '🛠️', k: 'tools hammer wrench fix build' },
				{ e: '⚙️', k: 'gear settings config' },
				{ e: '🧰', k: 'toolbox tools' },
				{ e: '⏰', k: 'clock alarm time wake' },
			],
		},
		{
			id: 'symbols',
			label: _('Symbols'),
			emojis: [
				{ e: '✨', k: 'sparkles shine magic new' },
				{ e: '⭐', k: 'star favorite' },
				{ e: '🌟', k: 'glowing star sparkle' },
				{ e: '💫', k: 'dizzy stars sparkle' },
				{ e: '🔥', k: 'fire hot flame lit' },
				{ e: '💥', k: 'boom explosion bang' },
				{ e: '💯', k: 'hundred perfect best' },
				{ e: '✅', k: 'check tick yes done' },
				{ e: '❌', k: 'cross x no wrong' },
				{ e: '⚠️', k: 'warning caution alert' },
				{ e: '❗', k: 'exclamation important' },
				{ e: '❓', k: 'question mark help' },
				{ e: '💬', k: 'speech chat message bubble talk' },
				{ e: '💭', k: 'thought bubble think' },
				{ e: '🗨️', k: 'speech left bubble' },
				{ e: '🗯️', k: 'anger right speech bubble' },
				{ e: '♥️', k: 'heart love red' },
				{ e: '💜', k: 'purple heart' },
				{ e: '💙', k: 'blue heart' },
				{ e: '💚', k: 'green heart' },
				{ e: '💛', k: 'yellow heart' },
				{ e: '🧡', k: 'orange heart' },
				{ e: '🤍', k: 'white heart' },
				{ e: '🤎', k: 'brown heart' },
				{ e: '🖤', k: 'black heart' },
				{ e: '♻️', k: 'recycle reuse' },
				{ e: '🆗', k: 'ok button' },
				{ e: '🆕', k: 'new button' },
				{ e: '🆒', k: 'cool button' },
				{ e: '🎯', k: 'target bullseye dart goal aim' },
			],
		},
		{
			id: 'nature',
			label: _('Nature & food'),
			emojis: [
				{ e: '🌱', k: 'seedling plant grow' },
				{ e: '🌿', k: 'herb leaf plant' },
				{ e: '🍀', k: 'four leaf clover luck' },
				{ e: '🌳', k: 'tree deciduous' },
				{ e: '🌲', k: 'evergreen tree pine' },
				{ e: '🌴', k: 'palm tree tropical' },
				{ e: '🌵', k: 'cactus desert' },
				{ e: '🌷', k: 'tulip flower' },
				{ e: '🌸', k: 'cherry blossom flower' },
				{ e: '🌹', k: 'rose flower red' },
				{ e: '🌻', k: 'sunflower flower yellow' },
				{ e: '🌼', k: 'blossom flower yellow' },
				{ e: '🌍', k: 'earth globe europe africa' },
				{ e: '🌎', k: 'earth globe americas' },
				{ e: '🌏', k: 'earth globe asia australia' },
				{ e: '☀️', k: 'sun sunny day' },
				{ e: '🌙', k: 'moon night crescent' },
				{ e: '⛅', k: 'cloud sun partly' },
				{ e: '🌧️', k: 'rain cloud weather' },
				{ e: '❄️', k: 'snowflake cold winter' },
				{ e: '🍎', k: 'apple red fruit' },
				{ e: '🍊', k: 'orange tangerine fruit' },
				{ e: '🍋', k: 'lemon yellow fruit' },
				{ e: '🍌', k: 'banana yellow fruit' },
				{ e: '🍉', k: 'watermelon fruit' },
				{ e: '🍇', k: 'grapes fruit' },
				{ e: '🍓', k: 'strawberry fruit red' },
				{ e: '🍒', k: 'cherries fruit red' },
				{ e: '🍕', k: 'pizza food' },
				{ e: '🍔', k: 'burger hamburger food' },
			],
		},
	];

	protected parentContainer: HTMLElement;
	protected data: EmojiPickerWidgetJSON;
	protected builder: JSBuilder;

	protected container!: HTMLElement;
	protected search!: HTMLInputElement;
	protected grid!: HTMLElement;
	protected query: string = '';
	protected recents: string[] = [];

	constructor(
		parentContainer: HTMLElement,
		data: EmojiPickerWidgetJSON,
		builder: JSBuilder,
		_callback: JSDialogCallback,
	) {
		this.parentContainer = parentContainer;
		this.data = data;
		this.builder = builder;
	}

	private get recentsKey(): string {
		return this.data.recentsPrefsKey || EmojiPickerWidget.DEFAULT_RECENTS_KEY;
	}

	private get maxRecents(): number {
		return this.data.maxRecents || EmojiPickerWidget.DEFAULT_MAX_RECENTS;
	}

	private loadRecents(): void {
		try {
			const arr = JSON.parse(window.prefs.get(this.recentsKey, '[]'));
			this.recents = Array.isArray(arr)
				? arr.filter((s: any) => typeof s === 'string')
				: [];
		} catch {
			this.recents = [];
		}
	}

	private saveRecents(): void {
		window.prefs.set(this.recentsKey, JSON.stringify(this.recents));
	}

	private onPick(emoji: string): void {
		this.recents = [emoji, ...this.recents.filter((e) => e !== emoji)].slice(
			0,
			this.maxRecents,
		);
		this.saveRecents();
		this.builder.callback(
			'emojipicker',
			'change',
			this.container,
			emoji,
			this.builder,
		);
	}

	// Look up the first search keyword for an emoji glyph. Used for
	// aria-label so screen readers announce e.g. "smile" rather than
	// stumbling on the raw codepoint. Returns '' if not in the catalogue.
	private firstKeyword(emoji: string): string {
		for (const cat of EmojiPickerWidget.CATEGORIES) {
			for (const entry of cat.emojis) {
				if (entry.e === emoji) {
					const space = entry.k.indexOf(' ');
					return space === -1 ? entry.k : entry.k.slice(0, space);
				}
			}
		}
		return '';
	}

	private appendSection(title: string, emojis: string[]): void {
		const heading = window.L.DomUtil.create(
			'div',
			'ui-emoji-picker-heading',
			this.grid,
		);
		heading.setAttribute('role', 'heading');
		heading.setAttribute('aria-level', '3');
		heading.textContent = title;

		const row = window.L.DomUtil.create(
			'div',
			'ui-emoji-picker-row',
			this.grid,
		);
		for (const emoji of emojis) {
			const btn = window.L.DomUtil.create(
				'button',
				'ui-emoji-picker-cell',
				row,
			) as HTMLButtonElement;
			btn.type = 'button';
			btn.textContent = emoji;
			btn.setAttribute('aria-label', this.firstKeyword(emoji) || emoji);
			btn.addEventListener('click', () => this.onPick(emoji));
		}
	}

	private matchesEntry(entry: EmojiEntry): boolean {
		if (!this.query) return true;
		return entry.e === this.query || entry.k.indexOf(this.query) !== -1;
	}

	private renderGrid(): void {
		this.grid.innerHTML = '';
		if (!this.query && this.recents.length > 0) {
			this.appendSection(_('Recently used'), this.recents);
		}
		let anyShown = false;
		for (const cat of EmojiPickerWidget.CATEGORIES) {
			const filtered = this.query
				? cat.emojis.filter((entry) => this.matchesEntry(entry))
				: cat.emojis;
			if (filtered.length === 0) continue;
			anyShown = true;
			this.appendSection(
				cat.label,
				filtered.map((entry) => entry.e),
			);
		}
		if (!anyShown && this.query) {
			const empty = window.L.DomUtil.create(
				'div',
				'ui-emoji-picker-empty',
				this.grid,
			);
			empty.textContent = _('No emojis match "{0}"').replace('{0}', this.query);
		}
	}

	public build(): boolean {
		this.loadRecents();

		this.container = window.L.DomUtil.create(
			'div',
			'ui-emoji-picker ' + this.builder.options.cssClass,
			this.parentContainer,
		);
		this.container.id = this.data.id;
		this.container.setAttribute('role', 'dialog');
		if (this.data.aria && this.data.aria.label) {
			this.container.setAttribute('aria-label', this.data.aria.label);
		} else {
			this.container.setAttribute('aria-label', _('Choose an emoji'));
		}

		this.search = window.L.DomUtil.create(
			'input',
			'ui-emoji-picker-search ' + this.builder.options.cssClass,
			this.container,
		) as HTMLInputElement;
		this.search.type = 'search';
		this.search.placeholder = this.data.searchPlaceholder || _('Search emojis');
		this.search.setAttribute('aria-label', this.search.placeholder);
		this.search.addEventListener('input', () => {
			this.query = this.search.value.trim().toLowerCase();
			this.renderGrid();
		});

		this.grid = window.L.DomUtil.create(
			'div',
			'ui-emoji-picker-grid',
			this.container,
		);

		this.renderGrid();

		requestAnimationFrame(() => this.search.focus());

		return false;
	}
}

JSDialog.emojiPicker = function (
	parentContainer: HTMLElement,
	data: EmojiPickerWidgetJSON,
	builder: JSBuilder,
	callback: JSDialogCallback,
) {
	const widget = new EmojiPickerWidget(
		parentContainer,
		data,
		builder,
		callback,
	);
	return widget.build();
};
