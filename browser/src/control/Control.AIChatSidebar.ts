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
 * Control.AIChatSidebar - AI Chat sidebar rendered via JSDialog in the existing sidebar dock wrapper.
 *
 * Manages chat state (messages, processing), generates JSDialog widget JSON,
 * and renders using its own JSDialog builder instance with a custom callback.
 * Selected text is exchanged as markdown (text/markdown;charset=utf-8).
 */

/* global app JSDialog Autolinker marked */
declare var JSDialog: any;
declare var Autolinker: any;
declare var marked: any;

namespace cool {
	interface ChatMessage {
		role: 'user' | 'assistant';
		content: string;
		displayContent?: string;
		selectedText?: string;
		timestamp: number;
		isError?: boolean;
		imageData?: string;
		isApproval?: boolean;
		approvalType?: 'inspect' | 'modify';
	}

	export class AIChatSidebar {
		private messages: ChatMessage[] = [];
		private isProcessing: boolean = false;
		private currentRequestId: string = '';
		private inputText: string = '';
		private _isActive: boolean = false;
		private lastSentSelectedText: string = '';
		private hintText: string = '';
		private progressText: string = '';
		private pendingFormulaContext: string = '';

		private builder: any;
		private container: HTMLElement;
		private wrapper: HTMLElement;

		private readonly PROMPT_CARDS: { label: string; prompt: string }[] = [
			{
				label: _('Make the following text more concise'),
				prompt: 'Make it more concise',
			},
			{
				label: _('Make the tone more professional'),
				prompt: 'Make it more formal',
			},
			{
				label: _('Rewrite in a more casual, friendly tone'),
				prompt: 'Make it more casual',
			},
			{
				label: _('Summarize the key points of this text'),
				prompt: 'Summarize this text',
			},
			{
				label: _('Expand and add more detail to this text'),
				prompt: 'Expand this text',
			},
			{
				label: _('Fix grammar, spelling, and punctuation errors'),
				prompt: 'Fix grammar & spelling',
			},
		];

		private readonly INITIAL_CARDS_SHOWN: number = 3;
		private showAllCards: boolean = false;

		private readonly SELECTION_FETCH_TIMEOUT_MS = 5000;
		private readonly COPY_FEEDBACK_DURATION_MS = 1500;
		private readonly TEXTAREA_MAX_HEIGHT_PX = 120;
		private readonly FORMULA_FETCH_TIMEOUT_MS = 5000;
		private readonly MAX_API_MESSAGES = 50;
		private readonly MAX_MESSAGE_LENGTH = 100000; // 100K characters

		constructor() {
			this.container = document.getElementById('aichat-panel') as HTMLElement;
			this.wrapper = document.getElementById(
				'aichat-dock-wrapper',
			) as HTMLElement;
			this.createBuilder();
			this.setupCellRefNavigation();
			this.registerChatHandlers();
		}

		private createBuilder(): void {
			this.builder = new window.L.control.jsDialogBuilder({
				mobileWizard: this,
				map: app.map,
				windowId: -1,
				cssClass: 'jsdialog sidebar',
				useScrollAnimation: false,
				suffix: 'sidebar',
				callback: this.jsdialogCallback.bind(this),
			});
		}

		private registerChatHandlers(): void {
			app.map.on('aichatresult', this.onAIChatResult, this);
			app.map.on('aichatprogress', this.onAIChatProgress, this);
			app.map.on('aichatapproval', this.onAIChatApproval, this);
			app.map.on('docloaded', this.onDocLoaded, this);
			app.map.on('textselectionchange', this.onTextSelectionChange, this);
		}

		private onDocLoaded(e: any): void {
			if (e.status === false && this._isActive) {
				this.hide();
				this.clearConversation();
			}
		}

		private onTextSelectionChange(): void {
			if (!this._isActive) return;
			const selCtx = document.getElementById('aichat-selection-context');
			if (selCtx) {
				selCtx.style.display = TextSelections.isActive() ? '' : 'none';
			}
		}

		toggle(): void {
			if (this._isActive) {
				this.hide();
			} else {
				this.show();
			}
		}

		show(): void {
			this._isActive = true;
			this.render();
			this.wrapper.classList.add('visible');
			this.focusInput();
		}

		hide(): void {
			this._isActive = false;
			this.wrapper.classList.remove('visible');
			if (!app.map.editorHasFocus()) {
				app.map.fire('editorgotfocus');
				app.map.focus();
			}
		}

		isVisible(): boolean {
			return this._isActive;
		}

		private render(): void {
			if (!this._isActive) return;
			this.container.innerHTML = '';
			const data = this.getWidgetJSON();
			this.builder.build(this.container, [data], false);
			this.applyMessageStyles();
			this.applyInputStyles();
			this.scrollToBottom();
			this.attachContainerKeyboardHandler();
			if (!this.isProcessing) {
				this.focusInput();
			}
		}

		private updateMessagesArea(): void {
			this.builder.updateWidget(this.container, this.getMessagesAreaJSON());
			app.layoutingService.onDrain(() => {
				this.applyMessageStyles();
				this.scrollToBottom();
			});
		}

		private updateInputArea(): void {
			this.builder.updateWidget(this.container, this.getInputJSON());
			app.layoutingService.onDrain(() => {
				this.applyInputStyles();
				if (!this.isProcessing) {
					this.focusInput();
				}
			});
		}

		private updateHint(): void {
			this.builder.updateWidget(this.container, this.getHintJSON());
		}

		private updateHeader(): void {
			this.builder.updateWidget(this.container, this.getHeaderJSON());
		}

		private updateChatState(includeHeader: boolean = false): void {
			this.updateMessagesArea();
			this.updateInputArea();
			this.updateHint();
			if (includeHeader) {
				this.updateHeader();
			}
		}

		private appendMessage(msg: ChatMessage, index: number): void {
			const messagesList = document.getElementById('aichat-messages-list');
			if (!messagesList) {
				this.updateMessagesArea();
				return;
			}
			this.builder.build(messagesList, [this.getMessageJSON(msg, index)], true);
			this.applyStyleForMessage(index);
			this.scrollToBottom();
		}

		private updateLoadingDots(): void {
			this.builder.updateWidget(this.container, this.getLoadingDotsJSON());
			app.layoutingService.onDrain(() => {
				this.scrollToBottom();
			});
		}

		private applyStyleForMessage(index: number): void {
			const msg = this.messages[index];
			const el = document.getElementById('aichat-msg-' + index);
			if (!el || !msg) return;

			const cls =
				msg.role === 'user' ? 'aichat-msg-user' : 'aichat-msg-assistant';
			el.classList.add(cls);
			if (msg.isError) {
				el.classList.add('aichat-msg-error');
			}
			if (msg.isApproval) {
				el.classList.add('aichat-msg-approval');
				if (msg.approvalType === 'modify') {
					el.classList.add('aichat-msg-approval-modify');
				}
			}
			const label =
				msg.role === 'user'
					? _('Your message')
					: msg.isApproval
						? _('Action requiring approval')
						: msg.isError
							? _('Error message')
							: _('AI response');
			el.setAttribute('aria-label', label);
		}

		private applyMessageStyles(): void {
			for (let i = 0; i < this.messages.length; i++) {
				this.applyStyleForMessage(i);
			}
			this.applyCardStyles();
		}

		private applyCardStyles(): void {
			const chips = document.getElementById('aichat-chips');
			if (!chips) return;

			chips.setAttribute('role', 'list');
			const wrappers = chips.querySelectorAll('[id^="aichat-chip-"]');
			wrappers.forEach((w) => w.setAttribute('role', 'listitem'));

			const selCtx = document.getElementById('aichat-selection-context');
			if (selCtx) {
				selCtx.style.display = TextSelections.isActive() ? '' : 'none';
			}

			this.setupChipKeyboardNavigation();
		}

		private applyInputStyles(): void {
			const sendBtn = document.querySelector(
				'#aichat-send-btn button.ui-pushbutton',
			);
			if (sendBtn) {
				sendBtn.classList.toggle('aichat-stop-mode', this.isProcessing);
			}
		}

		private scrollToBottom(): void {
			requestAnimationFrame(() => {
				const messagesArea = document.getElementById('aichat-messages-area');
				if (messagesArea) {
					messagesArea.scrollTo({
						top: messagesArea.scrollHeight,
						behavior: 'smooth',
					});
				}
			});
		}

		private focusInput(): void {
			requestAnimationFrame(() => {
				const textarea = document.querySelector(
					'#aichat-input.ui-textarea',
				) as HTMLTextAreaElement | null;
				if (textarea) textarea.focus();
			});
		}

		private getWidgetJSON(): any {
			const children: any[] = [
				this.getHeaderJSON(),
				this.getMessagesAreaJSON(),
				this.getHintJSON(),
			];
			children.push(this.getInputJSON());
			return {
				id: 'aichat-main',
				type: 'container',
				vertical: true,
				children: children,
			};
		}

		private getEUNoticeJSON(): any {
			return {
				id: 'aichat-eu-notice',
				type: 'fixedtext',
				text: _(
					'Responses are generated by an external AI service. Content may be inaccurate. Review before use.',
				),
				enabled: true,
			};
		}

		private getHintJSON(): any {
			return {
				id: 'aichat-hint',
				type: 'fixedtext',
				text: this.hintText || '',
				enabled: true,
				visible: !!this.hintText,
				allyRole: 'status',
				ariaLive: 'polite' as const,
			};
		}

		// JSDialog builder only creates a wrapper div for containers
		// with >1 children; add a spacer to guarantee the div exists.
		private ensureContainerChildren(children: any[], idPrefix: string): void {
			if (children.length < 2) {
				children.push({
					id: idPrefix + '-spacer',
					type: 'fixedtext',
					text: '',
					enabled: true,
				});
			}
		}

		private readonly ICON_TRASH = 'lc_aichat_trash.svg';
		private readonly ICON_CLOSE = 'lc_aichat_close.svg';
		private readonly ICON_SEND = 'lc_aichat_send.svg';
		private readonly ICON_INSERT = 'lc_aichat_insert.svg';
		private readonly ICON_STOP = 'lc_aichat_stop.svg';

		// Keep as data URI: transient success indicator with semantic green color
		private readonly ICON_CHECK: string =
			"data:image/svg+xml,%3Csvg viewBox='0 0 24 24' xmlns='http://www.w3.org/2000/svg' " +
			"fill='none' stroke='%2322c55e' stroke-width='2.5' stroke-linecap='round' " +
			"stroke-linejoin='round'%3E%3Cpath d='M5 13l4 4L19 7'/%3E%3C/svg%3E";

		private getTitle(): string {
			const model = app.map.aiModelName || '';
			if (model) {
				return _('AI Assistant') + ': ' + model;
			}
			return _('AI Assistant');
		}

		private getHeaderJSON(): any {
			return {
				id: 'aichat-header',
				type: 'container',
				horizontal: true,
				children: [
					{
						id: 'aichat-title',
						type: 'fixedtext',
						text: this.getTitle(),
						enabled: true,
					},
					{
						id: 'aichat-clear-btn',
						type: 'pushbutton',
						image: this.ICON_TRASH,
						enabled: this.messages.length > 0,
						aria: { label: _('New conversation') },
					},
					{
						id: 'aichat-close-btn',
						type: 'pushbutton',
						image: this.ICON_CLOSE,
						enabled: true,
						aria: { label: _('Close') },
					},
				],
			};
		}

		private getMessagesListJSON(): any {
			const children: any[] = [];

			if (this.messages.length === 0) {
				children.push({
					id: 'aichat-empty-state',
					type: 'fixedtext',
					text: _('Ask AI anything about your document...'),
					enabled: true,
				});
				children.push(this.getPromptChipsJSON());
			} else {
				for (let i = 0; i < this.messages.length; i++) {
					children.push(this.getMessageJSON(this.messages[i], i));
				}
			}

			this.ensureContainerChildren(children, 'aichat-messages');

			return {
				id: 'aichat-messages-list',
				type: 'container',
				vertical: true,
				children: children,
			};
		}

		private getLoadingDotsJSON(): any {
			const dotsChildren: any[] = [
				{
					id: 'aichat-dot-1',
					type: 'fixedtext',
					text: '\u25CF',
					enabled: true,
				},
				{
					id: 'aichat-dot-2',
					type: 'fixedtext',
					text: '\u25CF',
					enabled: true,
				},
				{
					id: 'aichat-dot-3',
					type: 'fixedtext',
					text: '\u25CF',
					enabled: true,
				},
			];
			if (this.progressText) {
				dotsChildren.push({
					id: 'aichat-progress-text',
					type: 'fixedtext',
					text: this.progressText,
					enabled: true,
				});
			}
			return {
				id: 'aichat-loading-dots',
				type: 'container',
				horizontal: true,
				visible: this.isProcessing,
				allyRole: 'status',
				ariaLive: 'polite' as const,
				aria: { label: this.progressText || _('Loading') },
				children: dotsChildren,
			};
		}

		private getMessagesAreaJSON(): any {
			return {
				id: 'aichat-messages-area',
				type: 'container',
				vertical: true,
				children: [this.getMessagesListJSON(), this.getLoadingDotsJSON()],
				allyRole: 'log',
				ariaLive: 'polite' as const,
				aria: { label: _('Chat messages') },
			};
		}

		private getMessageJSON(msg: ChatMessage, index: number): any {
			const isUser = msg.role === 'user';
			const children: any[] = [];

			if (isUser && msg.selectedText) {
				children.push({
					id: 'aichat-context-' + index,
					type: 'fixedtext',
					html: app.LOUtil.sanitize(this.markdownToHtml(msg.selectedText)),
					enabled: true,
				});
			}

			if (msg.imageData) {
				// Image message
				children.push({
					id: `aichat-msg-text-${index}`,
					type: 'fixedtext',
					html: app.LOUtil.sanitize(
						'<img src="data:image/png;base64,' +
							msg.imageData +
							'" alt="' +
							_('AI generated image') +
							'" class="aichat-generated-image" />',
					),
					enabled: true,
				});
				children.push(this.getImageActionsJSON(index));
			} else {
				// Text message content - use displayContent for UI, content for API
				const displayText = msg.displayContent || msg.content;
				if (isUser || msg.isError) {
					children.push({
						id: `aichat-msg-text-${index}`,
						type: 'fixedtext',
						text: displayText,
						enabled: true,
					});
				} else {
					children.push({
						id: `aichat-msg-text-${index}`,
						type: 'fixedtext',
						html: app.LOUtil.sanitize(this.markdownToHtml(displayText)),
						enabled: true,
					});
				}

				// Action buttons for text assistant messages (skip approval messages)
				if (!isUser && !msg.isError && !msg.isApproval) {
					children.push(this.getActionsJSON(index, true));
				}

				// Retry button for error messages
				if (msg.isError) {
					children.push({
						id: `aichat-retry-${index}`,
						type: 'pushbutton',
						text: _('Retry'),
						enabled: true,
					});
				}
			}

			this.ensureContainerChildren(children, `aichat-msg-${index}`);

			return {
				id: `aichat-msg-${index}`,
				type: 'container',
				vertical: true,
				children: children,
			};
		}

		private getActionsJSON(index: number, showInsert: boolean): any {
			const children: any[] = [];

			if (showInsert) {
				children.push({
					id: `aichat-insert-text-${index}`,
					type: 'pushbutton',
					image: this.ICON_INSERT,
					aria: { label: _('Insert at cursor') },
				});
			}

			children.push({
				id: `aichat-copy-text-${index}`,
				type: 'pushbutton',
				image: 'lc_copy.svg',
				enabled: true,
				aria: { label: _('Copy to clipboard') },
			});

			this.ensureContainerChildren(children, `aichat-actions-${index}`);

			return {
				id: `aichat-actions-${index}`,
				type: 'container',
				horizontal: true,
				children: children,
			};
		}

		private getImageActionsJSON(index: number): any {
			return {
				id: `aichat-actions-${index}`,
				type: 'container',
				horizontal: true,
				children: [
					{
						id: `aichat-insert-img-${index}`,
						type: 'pushbutton',
						image: this.ICON_INSERT,
						enabled: true,
						aria: { label: _('Insert at cursor') },
					},
					{
						id: `aichat-copy-img-${index}`,
						type: 'pushbutton',
						image: 'lc_copy.svg',
						enabled: true,
						aria: { label: _('Copy to clipboard') },
					},
				],
			};
		}

		private getInputJSON(): any {
			return {
				id: 'aichat-input-area',
				type: 'container',
				vertical: true,
				children: [
					{
						id: 'aichat-input-row',
						type: 'container',
						horizontal: true,
						children: [
							{
								id: 'aichat-input',
								type: 'multilineedit',
								text: this.inputText,
								placeholder: _('Ask AI...'),
								cursor: true,
								enabled: !this.isProcessing,
								aria: { label: _('Message input') },
							},
							{
								id: 'aichat-send-btn',
								type: 'pushbutton',
								image: this.isProcessing ? this.ICON_STOP : this.ICON_SEND,
								enabled: this.isProcessing || this.inputText.trim().length > 0,
								aria: {
									label: this.isProcessing
										? _('Stop generating')
										: _('Send message'),
								},
							},
						],
					},
					this.getEUNoticeJSON(),
				],
			};
		}

		private getPromptChipsJSON(): any {
			const cardChildren: any[] = [];

			// Selection context indicator — shown/hidden in applyCardStyles()
			cardChildren.push({
				id: 'aichat-selection-context',
				type: 'fixedtext',
				text: '\u2726  ' + _('Using selected text as context'),
				enabled: true,
			});

			if (app.map.getDocType() === 'spreadsheet') {
				cardChildren.push({
					id: 'aichat-chip-formula-diagnosis',
					type: 'pushbutton',
					text: _('Diagnose formula error'),
					enabled: true,
				});
				cardChildren.push({
					id: 'aichat-chip-sanity-check',
					type: 'pushbutton',
					text: _('Sanity check data'),
					enabled: true,
				});
				cardChildren.push({
					id: 'aichat-chip-create-formula',
					type: 'pushbutton',
					text: _('Create a formula'),
					enabled: true,
				});
			}

			const cardsToShow = this.showAllCards
				? this.PROMPT_CARDS.length
				: this.INITIAL_CARDS_SHOWN;

			for (let i = 0; i < cardsToShow && i < this.PROMPT_CARDS.length; i++) {
				const card = this.PROMPT_CARDS[i];
				cardChildren.push({
					id: 'aichat-chip-' + i,
					type: 'pushbutton',
					text: card.label,
					enabled: true,
				});
			}

			// "See more" / "See less" toggle
			if (this.PROMPT_CARDS.length > this.INITIAL_CARDS_SHOWN) {
				cardChildren.push({
					id: 'aichat-see-more',
					type: 'pushbutton',
					text: this.showAllCards ? _('See less') : _('See more'),
					enabled: true,
				});
			}

			return {
				id: 'aichat-chips',
				type: 'container',
				vertical: true,
				children: cardChildren,
			};
		}

		private parseActionIndex(id: string, prefix: string): number {
			return parseInt(id.substring(prefix.length), 10);
		}

		private handleClick(id: string): void {
			// Exact-match actions
			const exactActions: Record<string, () => void> = {
				'aichat-send-btn': () => {
					if (this.isProcessing) {
						app.socket.sendMessage('aichatcancel: ' + this.currentRequestId);
						this.isProcessing = false;
						this.currentRequestId = '';
						this.hintText = '';
						this.progressText = '';
						this.updateLoadingDots();
						this.updateInputArea();
					} else {
						this.sendMessage();
					}
				},
				'aichat-close-btn': () => this.hide(),
				'aichat-clear-btn': () => this.clearConversation(),
				'aichat-chip-formula-diagnosis': () => this.diagnoseFormulaError(),
				'aichat-chip-sanity-check': () => this.sanityCheckData(),
				'aichat-chip-create-formula': () => this.createFormula(),
				'aichat-see-more': () => {
					this.showAllCards = !this.showAllCards;
					this.builder.updateWidget(this.container, this.getMessagesListJSON());
					app.layoutingService.onDrain(() => {
						this.applyCardStyles();
					});
				},
			};

			if (exactActions[id]) {
				exactActions[id]();
				return;
			}

			// Prefix-based actions (order matters: longer prefixes first)
			const prefixActions: {
				prefix: string;
				handler: (idx: number) => void;
			}[] = [
				{
					prefix: 'aichat-insert-img-',
					handler: (idx) => {
						const imgData = this.messages[idx]?.imageData;
						if (imgData) {
							this.insertImageAtCursor(imgData);
							this.showCopyFeedback(idx, 'aichat-insert-img-');
							this.showTransientHint(_('Image inserted at cursor'));
						}
					},
				},
				{
					prefix: 'aichat-copy-img-',
					handler: (idx) => {
						const imgData = this.messages[idx]?.imageData;
						if (imgData) {
							this.copyImageToClipboard(imgData, idx);
						}
					},
				},
				{
					prefix: 'aichat-insert-text-',
					handler: (idx) => {
						if (this.messages[idx]) {
							this.insertAtCursor(this.messages[idx].content);
							this.showCopyFeedback(idx, 'aichat-insert-text-');
							this.showTransientHint(_('Text inserted at cursor'));
						}
					},
				},
				{
					prefix: 'aichat-copy-text-',
					handler: (idx) => {
						if (this.messages[idx]) {
							this.copyToClipboard(this.messages[idx].content, idx);
						}
					},
				},
				{
					prefix: 'aichat-chip-',
					handler: (idx) => {
						if (this.PROMPT_CARDS[idx]) {
							this.inputText = this.PROMPT_CARDS[idx].prompt;
							this.sendMessage();
						}
					},
				},
				{
					prefix: 'aichat-retry-',
					handler: (idx) => {
						const userMsg = this.findPrecedingUserMessage(idx);
						if (userMsg) {
							this.messages.splice(idx, 1);
							const userIdx = this.messages.indexOf(userMsg);
							if (userIdx >= 0) this.messages.splice(userIdx, 1);
							this.inputText = userMsg.displayContent || userMsg.content;
							this.sendMessage();
						}
					},
				},
			];

			for (const { prefix, handler } of prefixActions) {
				if (id.startsWith(prefix)) {
					handler(this.parseActionIndex(id, prefix));
					return;
				}
			}
		}

		private handleInputChange(id: string, data: any): void {
			if (id !== 'aichat-input') return;

			const prevEmpty = this.inputText.trim().length === 0;
			this.inputText = data;
			const nowEmpty = this.inputText.trim().length === 0;

			if (prevEmpty !== nowEmpty) {
				const sendBtn = document.querySelector(
					'#aichat-send-btn button.ui-pushbutton',
				) as HTMLButtonElement | null;
				if (sendBtn) {
					if (nowEmpty) {
						sendBtn.setAttribute('disabled', 'true');
					} else {
						sendBtn.removeAttribute('disabled');
					}
				}
			}

			// Show warning when approaching message length limit
			if (this.inputText.length > this.MAX_MESSAGE_LENGTH * 0.9) {
				this.hintText = _('Approaching message length limit');
				this.updateHint();
			} else if (this.hintText && this.hintText.includes('length')) {
				this.hintText = '';
				this.updateHint();
			}

			const textarea = document.querySelector(
				'#aichat-input.ui-textarea',
			) as HTMLTextAreaElement | null;
			if (textarea) {
				textarea.style.height = 'auto';
				textarea.style.height =
					Math.min(textarea.scrollHeight, this.TEXTAREA_MAX_HEIGHT_PX) + 'px';
			}
		}

		private jsdialogCallback(
			objectType: string,
			eventType: string,
			object: any,
			data: any,
			_builder: any,
		): void {
			if (!object || !object.id) return;

			if (eventType === 'click') {
				this.handleClick(object.id);
			} else if (eventType === 'change') {
				this.handleInputChange(object.id, data);
			}
		}

		// Needed for JSDialog builder compatibility (mobileWizard interface)
		setTabs(): void {
			// no-op: required by JSDialog builder mobileWizard interface
		}
		selectedTab(): number {
			return 0;
		}

		private get requestTimeoutMs(): number {
			return (app.map.aiRequestTimeout ?? 120) * 1000;
		}

		private startRequestTimeout(
			requestId: string,
			ms: number,
			handler: (data: any) => void,
		): void {
			setTimeout(() => {
				if (this.isProcessing && this.currentRequestId === requestId) {
					handler({
						success: false,
						error: _('Request timeout'),
						requestId: requestId,
					});
				}
			}, ms);
		}

		private async buildUserMessage(text: string): Promise<ChatMessage | null> {
			let selectedText = '';
			if (TextSelections.isActive()) {
				try {
					selectedText = await this.fetchSelectedMarkdown();
				} catch (e: any) {
					if (e?.message === 'complexselection') {
						this.hintText = _(
							'The selection contains images or other non-text content that cannot be sent as context.',
						);
						this.updateHint();
						return null;
					}
				}

				if (selectedText && selectedText === this.lastSentSelectedText) {
					selectedText = '';
				}
				if (selectedText) {
					this.lastSentSelectedText = selectedText;
				}
			}

			let userContent = text;
			// If createFormula() captured selection context before the user typed,
			// use it when no fresh selection is available.
			if (!selectedText && this.pendingFormulaContext) {
				selectedText = '';
				userContent = this.pendingFormulaContext + text;
				this.pendingFormulaContext = '';
			} else if (selectedText) {
				this.pendingFormulaContext = '';
				userContent =
					'[Selected text from document:\n```\n' + selectedText + '\n```]\n\n';

				if (app.map.getDocType() === 'spreadsheet') {
					userContent += this.getCellReferenceInstructions() + '\n\n';
				}

				userContent += text;
			}

			return {
				role: 'user',
				content: userContent,
				displayContent: text,
				selectedText: selectedText || undefined,
				timestamp: Date.now(),
			};
		}

		private buildApiMessages(): { role: string; content: string }[] {
			const apiMessages: { role: string; content: string }[] = [];
			const textMessages = this.messages.filter(
				(m) => !m.imageData && !m.isError && !m.isApproval,
			);
			const recent = textMessages.slice(-this.MAX_API_MESSAGES);
			for (const msg of recent) {
				apiMessages.push({ role: msg.role, content: msg.content });
			}
			return apiMessages;
		}

		private dispatchRequest(): void {
			this.currentRequestId = this.generateRequestId();

			const payload = JSON.stringify({
				messages: this.buildApiMessages(),
				requestId: this.currentRequestId,
				docType: app.map.getDocType(),
			});
			app.socket.sendMessage('aichat: ' + payload);
			this.startRequestTimeout(
				this.currentRequestId,
				this.requestTimeoutMs,
				(d) => this.onAIChatResult(d),
			);
		}

		async sendMessage(): Promise<void> {
			const text = this.inputText.trim();
			if (!text || this.isProcessing) return;

			if (text.length > this.MAX_MESSAGE_LENGTH) {
				this.hintText = _(
					'Message is too long. Please shorten it and try again.',
				);
				this.updateHint();
				return;
			}

			this.hintText = '';
			this.inputText = '';
			// Clear the DOM value synchronously so that a pending keyup
			// event does not restore inputText from the stale DOM value.
			const textarea = document.querySelector(
				'#aichat-input.ui-textarea',
			) as HTMLTextAreaElement | null;
			if (textarea) textarea.value = '';

			const userMsg = await this.buildUserMessage(text);
			if (!userMsg) return;

			this.messages.push(userMsg);
			this.isProcessing = true;

			if (this.messages.length === 1) {
				// First message - full rebuild to transition from prompt cards
				this.updateChatState(true);
			} else {
				this.appendMessage(userMsg, this.messages.length - 1);
				this.updateLoadingDots();
				this.updateInputArea();
				this.updateHeader();
			}
			this.updateHint();

			this.dispatchRequest();
		}

		private handleAIResponse(
			data: any,
			buildSuccessMsg: (data: any) => ChatMessage,
			defaultError: string,
		): void {
			if (data.requestId !== this.currentRequestId) return;

			this.isProcessing = false;
			this.hintText = '';
			this.progressText = '';

			if (data.success) {
				this.messages.push(buildSuccessMsg(data));
			} else {
				this.messages.push({
					role: 'assistant',
					content: _('Error: ') + (data.error || defaultError),
					timestamp: Date.now(),
					isError: true,
				});
			}

			this.updateLoadingDots();
			this.appendMessage(
				this.messages[this.messages.length - 1],
				this.messages.length - 1,
			);
			this.updateInputArea();
			this.updateHeader();
		}

		private onAIChatResult(data: any): void {
			this.handleAIResponse(
				data,
				(d) => {
					if (d.imageData) {
						return {
							role: 'assistant',
							content: _('Generated image'),
							imageData: d.imageData,
							timestamp: Date.now(),
						};
					}
					return {
						role: 'assistant',
						content: d.content,
						timestamp: Date.now(),
					};
				},
				_('AI request failed'),
			);
		}

		private onAIChatProgress(data: any): void {
			if (data.requestId !== this.currentRequestId) return;
			this.progressText = data.status || '';
			this.updateLoadingDots();
			// Reset the request timeout so multi-round loops do not time out
			this.startRequestTimeout(
				this.currentRequestId,
				this.requestTimeoutMs,
				(d) => this.onAIChatResult(d),
			);
		}

		private getModelDisplayName(): string {
			return app.map.aiModelName || _('The AI assistant');
		}

		private onAIChatApproval(data: any): void {
			if (data.requestId !== this.currentRequestId) return;
			this.hintText = '';
			this.updateHint();

			const name = this.getModelDisplayName();
			let content: string;
			if (data.toolName === 'extract_document_structure') {
				const plan = data.summary;
				if (plan) {
					content = plan + '\n\n' + _('Do you want to proceed?');
				} else {
					content = _(
						'%1 wants to inspect your document. Do you want to proceed?',
					).replace('%1', name);
				}
			} else if (data.toolName === 'transform_document_structure') {
				const summary = data.summary || _('Modify document structure');
				content =
					_('%1 wants to modify your document:').replace('%1', name) +
					'\n\n' +
					summary +
					'\n\n' +
					_('Do you want to apply this change?');
			} else if (data.toolName === 'set_cell_formula') {
				const summary = data.summary || _('Set cell formula');
				content =
					_('%1 wants to set a formula in your spreadsheet:').replace(
						'%1',
						name,
					) +
					'\n\n' +
					summary +
					'\n\n' +
					_('Do you want to apply this change?');
			} else {
				content = _(
					'%1 wants to perform an action. Do you want to proceed?',
				).replace('%1', name);
			}

			// Show the approval UI as an assistant message
			const approvalMsg: ChatMessage = {
				role: 'assistant',
				content: content,
				timestamp: Date.now(),
				isApproval: true,
				approvalType:
					data.toolName === 'transform_document_structure' ||
					data.toolName === 'set_cell_formula'
						? 'modify'
						: 'inspect',
			};
			this.messages.push(approvalMsg);
			this.appendMessage(approvalMsg, this.messages.length - 1);

			// Insert approval buttons after the message is built
			app.layoutingService.onDrain(() => {
				const msgEl = document.getElementById(
					'aichat-msg-' + (this.messages.length - 1),
				);
				if (!msgEl) return;
				msgEl.classList.add('aichat-msg-assistant');

				const btnContainer = document.createElement('div');
				btnContainer.className = 'aichat-approval-buttons';

				const approveBtn = document.createElement('button');
				approveBtn.textContent = _('Approve');
				approveBtn.className = 'aichat-approve-btn';
				approveBtn.setAttribute('aria-label', _('Approve action'));
				approveBtn.onclick = () => {
					this.sendApprovalAction('approve');
					btnContainer.remove();
				};

				const rejectBtn = document.createElement('button');
				rejectBtn.textContent = _('Reject');
				rejectBtn.className = 'aichat-reject-btn';
				rejectBtn.setAttribute('aria-label', _('Reject action'));
				rejectBtn.onclick = () => {
					this.sendApprovalAction('reject');
					btnContainer.remove();
				};

				btnContainer.appendChild(approveBtn);
				btnContainer.appendChild(rejectBtn);
				msgEl.appendChild(btnContainer);
			});
		}

		private sendApprovalAction(action: string): void {
			const payload = JSON.stringify({
				requestId: this.currentRequestId,
				action: action,
			});
			app.socket.sendMessage('aichatapprove: ' + payload);
			if (action === 'approve') {
				this.progressText = _('Applying changes...');
				this.updateLoadingDots();
			} else {
				this.hintText = _('Change rejected');
				this.updateHint();
			}
			if (action === 'approve') {
				// Reset timeout for the remaining loop
				this.startRequestTimeout(
					this.currentRequestId,
					this.requestTimeoutMs,
					(d) => this.onAIChatResult(d),
				);
			}
		}

		clearConversation(): void {
			this.messages = [];
			this.isProcessing = false;
			this.currentRequestId = '';
			this.inputText = '';
			this.lastSentSelectedText = '';
			this.hintText = '';
			this.progressText = '';
			this.showAllCards = false;
			this._chipKeyNavAttached = false;
			this.render();
		}

		private async fetchSelectedMarkdown(): Promise<string> {
			// Use annotated markdown for Calc to include row/column
			// headers that give the LLM cell-address context.
			const isCalc = app.map.getDocType() === 'spreadsheet';
			const mimeType = isCalc
				? 'application/x-libreoffice-markdown-annotated'
				: 'text/markdown;charset=utf-8';

			return new Promise((resolve, reject) => {
				const cleanup = () => {
					clearTimeout(timeout);
					app.map.off('textselectioncontent', handleTextResponse);
					app.map.off('complexselection', handleComplexResponse);
				};

				const timeout = setTimeout(() => {
					cleanup();
					reject(new Error(_('Selection fetch timeout')));
				}, this.SELECTION_FETCH_TIMEOUT_MS);

				const handleTextResponse = (e: any) => {
					const textMsg = e.msg || '';
					if (textMsg.startsWith('textselectioncontent:')) {
						cleanup();

						const content = textMsg.substring('textselectioncontent:'.length);
						try {
							// If multiple MIME types, it comes as JSON
							if (content.startsWith('{')) {
								const json = JSON.parse(content);
								const markdown = json[mimeType] || '';
								resolve(markdown);
							} else {
								resolve(content);
							}
						} catch {
							reject(new Error(_('Failed to parse selection content')));
						}
					}
				};

				const handleComplexResponse = () => {
					cleanup();
					reject(new Error('complexselection'));
				};

				app.map.on('textselectioncontent', handleTextResponse);
				app.map.on('complexselection', handleComplexResponse);
				app.socket.sendMessage('gettextselection mimetype=' + mimeType);
			});
		}

		private insertAtCursor(markdownText: string): void {
			// Same paste mechanism - pastes at current cursor position
			const cleaned = this.stripCodeFences(markdownText);
			const blob = new Blob([
				'paste mimetype=text/markdown;charset=utf-8\n',
				cleaned,
			]);
			app.socket.sendMessage(blob);
			app.map.fire('editorgotfocus');
			app.map.focus();
		}

		private base64ToUint8Array(data: string): Uint8Array {
			const byteChars = atob(data);
			const bytes = new Uint8Array(byteChars.length);
			for (let i = 0; i < byteChars.length; i++) {
				bytes[i] = byteChars.charCodeAt(i);
			}
			return bytes;
		}

		private insertImageAtCursor(base64Data: string): void {
			const bytes = this.base64ToUint8Array(base64Data);
			const blob = new Blob(['paste mimetype=image/png\n', bytes.buffer]);
			app.socket.sendMessage(blob);
			app.map.fire('editorgotfocus');
			app.map.focus();
		}

		private copyImageToClipboard(base64Data: string, index: number): void {
			const imgBlob = new Blob([this.base64ToUint8Array(base64Data)], {
				type: 'image/png',
			});

			if (navigator.clipboard && navigator.clipboard.write) {
				navigator.clipboard
					.write([
						new ClipboardItem({
							'image/png': Promise.resolve(imgBlob),
						}),
					])
					.then(() => {
						this.showCopyFeedback(index, 'aichat-copy-img-');
					})
					.catch((e: any) => {
						window.console.error('Copy image failed:', e);
					});
			} else {
				window.console.error('Clipboard API not available');
			}
		}

		private _chipKeyNavAttached: boolean = false;
		private _keyboardHandlerAttached: boolean = false;

		// Custom keyboard navigation for prompt chips. We can't use
		// JSDialog.KeyboardListNavigation() because the pushbutton handler
		// wraps each button in a div.ui-pushbutton-wrapper, so
		// nextElementSibling from a <button> is null (no siblings inside
		// the wrapper). Modifying the shared utility would risk breaking
		// other components, so this small handler is safer.
		private setupChipKeyboardNavigation(): void {
			if (this._chipKeyNavAttached) return;
			const chips = document.getElementById('aichat-chips');
			if (!chips) return;
			this._chipKeyNavAttached = true;

			chips.addEventListener('keydown', (e: KeyboardEvent) => {
				if (e.key !== 'ArrowDown' && e.key !== 'ArrowUp') return;
				const buttons = Array.from(
					chips.querySelectorAll<HTMLButtonElement>(
						'[id^="aichat-chip-"] > button.ui-pushbutton',
					),
				);
				if (!buttons.length) return;
				const idx = buttons.indexOf(
					document.activeElement as HTMLButtonElement,
				);
				const next =
					e.key === 'ArrowDown'
						? idx < buttons.length - 1
							? idx + 1
							: 0
						: idx > 0
							? idx - 1
							: buttons.length - 1;
				e.preventDefault();
				buttons[next].focus();
			});
		}

		private attachContainerKeyboardHandler(): void {
			if (this._keyboardHandlerAttached) return;
			this._keyboardHandlerAttached = true;

			this.container.addEventListener('keydown', (e: KeyboardEvent) => {
				if (e.key === 'Escape') {
					e.preventDefault();
					this.hide();
					return;
				}
				if (e.key === 'Enter' && !e.shiftKey) {
					const target = e.target as HTMLElement;
					if (
						target &&
						target.classList.contains('ui-textarea') &&
						target.closest('#aichat-input')
					) {
						e.preventDefault();
						this.sendMessage();
					}
				}
			});
		}

		private showCopyFeedback(
			index: number,
			prefix: string = 'aichat-copy-text-',
		): void {
			const wrapper = document.getElementById(prefix + index);
			if (!wrapper) return;
			const img = wrapper.querySelector('button img') as HTMLImageElement;
			if (!img) return;

			const originalSrc = img.src;
			img.src = this.ICON_CHECK;

			setTimeout(function () {
				if (img.isConnected) {
					img.src = originalSrc;
				}
			}, this.COPY_FEEDBACK_DURATION_MS);
		}

		private showTransientHint(text: string): void {
			this.hintText = text;
			this.updateHint();
			setTimeout(() => {
				if (this.hintText === text) {
					this.hintText = '';
					this.updateHint();
				}
			}, this.COPY_FEEDBACK_DURATION_MS);
		}

		private copyToClipboard(text: string, index: number): void {
			if (navigator.clipboard && window.isSecureContext) {
				navigator.clipboard
					.writeText(text)
					.then(() => {
						this.showCopyFeedback(index);
					})
					.catch(() => {
						this.fallbackCopy(text, index);
					});
			} else {
				this.fallbackCopy(text, index);
			}
		}

		private fallbackCopy(text: string, index: number): void {
			const textArea = document.createElement('textarea');
			textArea.style.position = 'absolute';
			textArea.style.opacity = '0';
			textArea.value = text;
			document.body.appendChild(textArea);
			textArea.select();
			try {
				document.execCommand('copy');
				this.showCopyFeedback(index);
			} catch (e) {
				window.console.error('Copy failed:', e);
			} finally {
				document.body.removeChild(textArea);
			}
		}

		private markdownToHtml(text: string): string {
			let html: string;

			if (typeof marked !== 'undefined') {
				html = marked.parse(text.trim(), {
					gfm: true,
					breaks: true,
				});
			} else {
				// Fallback: plain text with basic escaping and line breaks
				html =
					'<p>' +
					text
						.replace(/&/g, '&amp;')
						.replace(/</g, '&lt;')
						.replace(/>/g, '&gt;')
						.replace(/\n/g, '<br>') +
					'</p>';
			}

			// Convert cell:// links to clickable spans before sanitization
			// Handles <a href="cell://A1">A1</a> produced by marked from [A1](cell://A1)
			html = html.replace(
				/<a\s+href="cell:\/\/([A-Z]{1,3}\d{1,7})"[^>]*>([^<]*)<\/a>/gi,
				(_, addr, label) =>
					'<span class="aichat-cell-ref" data-cell="' +
					addr.toUpperCase() +
					'" role="link" tabindex="0">' +
					label +
					'</span>',
			);

			// Autolink bare URLs
			if (typeof Autolinker !== 'undefined') {
				html = Autolinker.link(html);
			}

			return html;
		}

		private findPrecedingUserMessage(
			assistantMsgIndex: number,
		): ChatMessage | null {
			for (let i = assistantMsgIndex - 1; i >= 0; i--) {
				if (this.messages[i].role === 'user') {
					return this.messages[i];
				}
			}
			return null;
		}

		private stripCodeFences(text: string): string {
			// Strip markdown code fences that AI models often wrap responses in
			// Handles: ```markdown\n...\n``` and ```\n...\n```
			let result = text.trim();
			const fencePattern = /^```(?:markdown|md)?\s*\n([\s\S]*?)\n```\s*$/;
			const match = result.match(fencePattern);
			if (match) {
				result = match[1];
			}
			return result;
		}

		private fetchFormulaDependencyChain(): Promise<any> {
			return new Promise((resolve, reject) => {
				const timeout = setTimeout(() => {
					app.map.off('commandvalues', handleResponse);
					reject(new Error(_('Formula analysis timeout')));
				}, this.FORMULA_FETCH_TIMEOUT_MS);

				const handleResponse = (e: any) => {
					if (e.commandName === '.uno:FormulaDepChain') {
						clearTimeout(timeout);
						app.map.off('commandvalues', handleResponse);
						resolve(e.commandValues);
					}
				};

				app.map.on('commandvalues', handleResponse);
				app.socket.sendMessage('commandvalues command=.uno:FormulaDepChain');
			});
		}

		public async diagnoseFormulaError(): Promise<void> {
			if (this.isProcessing) return;

			this.progressText = _('Analyzing formula dependencies...');
			this.updateMessagesArea();

			let depChain: any;
			try {
				depChain = await this.fetchFormulaDependencyChain();
			} catch {
				this.progressText = '';
				this.hintText = _('Failed to analyze formula.');
				this.updateHint();
				return;
			}

			this.progressText = '';
			this.hintText = '';

			if (!depChain || !depChain.hasError) {
				if (depChain && depChain.reason === 'not_formula') {
					this.hintText = _('The selected cell does not contain a formula.');
				} else {
					this.hintText = _('The selected cell does not have a formula error.');
				}
				this.updateHint();
				return;
			}

			const content = this.buildFormulaDiagnosisPrompt(depChain);
			const cell = depChain.cell;
			const displayContent =
				_('Diagnose formula error in') +
				' ' +
				cell.address +
				': ' +
				cell.formula;
			this.messages.push({
				role: 'user',
				content: content,
				displayContent: displayContent,
				timestamp: Date.now(),
			});
			this.inputText = '';
			this.hintText = '';
			this.isProcessing = true;
			this.updateChatState(true);
			this.updateHint();
			this.dispatchRequest();
		}

		private buildFormulaDiagnosisPrompt(depChain: any): string {
			const cell = depChain.cell;
			let context = _('I have a formula error in my spreadsheet.') + '\n\n';
			context +=
				_('Cell') +
				' ' +
				cell.address +
				' (' +
				_('Sheet') +
				': ' +
				cell.sheet +
				') ' +
				_('contains') +
				':\n';
			context += '  ' + _('Formula') + ': ' + cell.formula + '\n';
			context += '  ' + _('Error') + ': ' + cell.error + '\n\n';

			if (depChain.dependencies && depChain.dependencies.length > 0) {
				context += _('Referenced cells') + ':\n';
				context += this.formatDependencies(depChain.dependencies, 1);
				context += '\n';
			}

			context +=
				_('Please diagnose why this formula produces the') +
				' ' +
				cell.error +
				' ' +
				_('error.') +
				' ';
			context += _('Explain the root cause and suggest how to fix it.');
			context +=
				' ' +
				_(
					'When referencing specific cells, format each cell address as a clickable link: [A1](cell://A1).',
				);

			return context;
		}

		private formatDependencies(deps: any[], indent: number): string {
			let result = '';
			let prefix = '';
			for (let p = 0; p < indent; p++) prefix += '  ';

			for (let i = 0; i < deps.length; i++) {
				const dep = deps[i];
				if (dep.type === 'formula') {
					result += prefix + '- ' + dep.address + ': ';
					result += _('formula') + ' ' + dep.formula;
					if (dep.error) {
						result += ' => ' + _('ERROR') + ': ' + dep.error;
					} else if (dep.value !== undefined) {
						result += ' => ' + dep.value;
					}
					result += '\n';
					if (dep.dependencies && dep.dependencies.length > 0) {
						result += this.formatDependencies(dep.dependencies, indent + 1);
					}
				} else if (dep.type === 'value') {
					result +=
						prefix +
						'- ' +
						dep.address +
						': ' +
						_('value') +
						' ' +
						dep.value +
						'\n';
				} else if (dep.type === 'string') {
					result +=
						prefix +
						'- ' +
						dep.address +
						': ' +
						_('text') +
						' "' +
						dep.value +
						'"\n';
				} else if (dep.type === 'empty') {
					result += prefix + '- ' + dep.address + ': (' + _('empty') + ')\n';
				} else if (dep.type === 'range') {
					result += prefix + '- ' + dep.address + ': ' + _('range');
					if (dep.errorCells) {
						result += ' (' + _('errors in') + ': ' + dep.errorCells + ')';
					}
					if (dep.scanLimited) {
						result += ' (' + _('only partially scanned') + ')';
					}
					result += '\n';
				} else if (dep.type === 'circular') {
					result +=
						prefix +
						'- ' +
						dep.address +
						': (' +
						_('circular reference') +
						')\n';
				}
			}
			return result;
		}

		private setupCellRefNavigation(): void {
			const handleCellRef = (target: HTMLElement) => {
				const cellAddr = target.getAttribute('data-cell');
				if (cellAddr) {
					this.navigateToCell(cellAddr);
				}
			};

			this.container.addEventListener('click', (e: MouseEvent) => {
				const target = (e.target as HTMLElement).closest(
					'.aichat-cell-ref',
				) as HTMLElement | null;
				if (target) {
					e.preventDefault();
					handleCellRef(target);
				}
			});

			this.container.addEventListener('keydown', (e: KeyboardEvent) => {
				if (e.key !== 'Enter' && e.key !== ' ') return;
				const target = e.target as HTMLElement;
				if (target && target.classList.contains('aichat-cell-ref')) {
					e.preventDefault();
					handleCellRef(target);
				}
			});
		}

		private navigateToCell(cellAddress: string): void {
			// Use the Name Box (address input, windowId -4) to navigate
			const msg = JSON.stringify({
				id: 'pos_window',
				cmd: 'change',
				data: cellAddress,
				type: 'combobox',
			});
			app.socket.sendMessage('dialogevent -4 ' + msg);
		}

		private async sanityCheckData(): Promise<void> {
			if (this.isProcessing) return;

			if (!TextSelections.isActive()) {
				this.hintText = _('Select the data you want to sanity-check first.');
				this.updateHint();
				return;
			}

			this.progressText = _('Analyzing selected data...');
			this.updateMessagesArea();

			let markdown: string;
			try {
				markdown = await this.fetchSelectedMarkdown();
			} catch (e: any) {
				this.progressText = '';
				if (e?.message === 'complexselection') {
					this.hintText = _(
						'The selection contains images or other non-text content that cannot be sent as context.',
					);
				} else {
					this.hintText = _('Failed to extract spreadsheet data.');
				}
				this.updateHint();
				return;
			}

			this.progressText = '';

			if (!markdown || !markdown.trim()) {
				this.hintText = _('No data found in selection.');
				this.updateHint();
				return;
			}

			const prompt = this.buildSanityCheckPrompt();
			const content =
				'[Selected text from document:\n```\n' +
				markdown +
				'\n```]\n\n' +
				prompt;
			this.messages.push({
				role: 'user',
				content: content,
				displayContent: prompt,
				selectedText: markdown,
				timestamp: Date.now(),
			});
			this.inputText = '';
			this.hintText = '';
			this.isProcessing = true;
			this.updateChatState(true);
			this.updateHint();
			this.dispatchRequest();
		}

		private async createFormula(): Promise<void> {
			if (this.isProcessing) return;

			let context = '';
			if (TextSelections.isActive()) {
				try {
					const markdown = await this.fetchSelectedMarkdown();
					if (markdown && markdown.trim()) {
						context =
							'[Selected text from document:\n```\n' +
							markdown +
							'\n```]\n\n' +
							this.getCellReferenceInstructions() +
							'\n\n';
					}
				} catch {
					// No selection context available, proceed without it
				}
			}

			this.hintText = _(
				'Describe what formula you need. For example: "Calculate the average of column B" or "Sum all values where column A is greater than 100".',
			);
			this.pendingFormulaContext = context;
			this.updateHint();
		}

		private getCellReferenceInstructions(): string {
			let instructions = '';
			instructions +=
				_(
					'Read row numbers from the "Row" column — do not count or estimate row positions.',
				) + ' ';
			instructions +=
				_(
					"To identify a cell's column, match the value to the column letter in the header row — do not guess.",
				) + ' ';
			instructions +=
				_(
					'Reference a cell by combining its column header with the row number (e.g., column B + row 2 = B2).',
				) + ' ';
			instructions += _(
				'When referencing specific cells, format each cell as a clickable link: [B2](cell://B2).',
			);
			return instructions;
		}

		private buildSanityCheckPrompt(): string {
			let prompt = _('Sanity check the following spreadsheet data.') + '\n\n';
			prompt += _('Please analyze this data for:') + '\n';
			prompt +=
				'1. ' +
				_(
					"Benford's Law violations — check if leading digit distribution of numeric values deviates significantly from the expected distribution, which may suggest data manipulation.",
				) +
				'\n';
			prompt +=
				'2. ' +
				_(
					'Statistical outliers — identify values that deviate significantly from their column or group.',
				) +
				'\n';
			prompt +=
				'3. ' +
				_(
					'General anomalies — suspicious patterns, unexpected duplicates, inconsistencies, or formatting issues.',
				) +
				'\n\n';
			prompt += this.getCellReferenceInstructions();
			return prompt;
		}

		private generateRequestId(): string {
			return (
				'aichat-' +
				Date.now().toString(36) +
				'-' +
				Math.random().toString(36).substring(2, 8)
			);
		}
	}
}

JSDialog.AIChatSidebar = null as cool.AIChatSidebar | null;

JSDialog.getAIChatSidebar = function (): cool.AIChatSidebar {
	if (!JSDialog.AIChatSidebar) {
		JSDialog.AIChatSidebar = new cool.AIChatSidebar();
	}
	return JSDialog.AIChatSidebar;
};
