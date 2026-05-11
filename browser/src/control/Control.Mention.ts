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
 * Control.Mention
 */

/* global app cool */

interface MentionUserData {
	username: string;
	profile: string;
	label?: string;
}

class Mention extends AutoCompletePopup {
	declare map: any;
	declare newPopupData: PopupData;
	users: Array<MentionUserData>;
	filteredUsers: Array<MentionUserData>;
	declare data: MessageEvent<any>;
	debouceTimeoutId: NodeJS.Timeout;
	partialMention: Array<string>;
	typingMention: boolean;
	lastTypedChar: string;
	cursorPosAtStart: Point;

	constructor(map: any) {
		super(AutoCompleteDialogId.Mention, map);
	}

	onAdd() {
		this.newPopupData.isAutoCompletePopup = true;
		this.typingMention = false;
		this.partialMention = [];
		this.cursorPosAtStart = { x: 0, y: 0 } as Point;
	}

	sendMentionPostMsg(partialText: string) {
		if (this.debouceTimeoutId) clearTimeout(this.debouceTimeoutId);

		// happens when user deletes last character before '@'
		// if we send empty string to the WOPIHost. They might return us list
		// with thousand of users
		if (partialText === '') {
			this.closeMentionPopup(true);
			return;
		}

		this.debouceTimeoutId = setTimeout(() => {
			this.map.fire('postMessage', {
				msgId: 'UI_Mention',
				args: { type: 'autocomplete', text: partialText },
			});
		}, 300);
	}

	/* @ts-expect-error: Implementation is done before types are defined. Now there is a conflicting implementation. I need to keep it as is since this commit implies no functional changes. */
	getPopupEntries(users: Array<MentionUserData>): any[] {
		const entries: Array<TreeEntryJSON> = [];
		if (users === null) return entries;

		const text = this.getPartialMention();

		// filterout the users from list according to the text
		if (text.length > 1) {
			this.filteredUsers = users.filter((element: any) => {
				const uid = element.label ?? element.username;

				// case insensitive
				return uid.toLowerCase().includes(text.toLowerCase());
			});
		} else {
			this.filteredUsers = users;
		}

		if (this.filteredUsers.length !== 0) {
			for (const i in this.filteredUsers) {
				const currentUser = this.filteredUsers[i];
				const entry = {
					text: currentUser.label ?? currentUser.username,
					columns: [
						{
							text: currentUser.label ?? currentUser.username,
						},
					],
					row: i.toString(),
				} as TreeEntryJSON;
				entries.push(entry);
			}
		}
		return entries;
	}

	openMentionPopup(users: Array<MentionUserData>) {
		if (!this.typingMention) return;

		const entries = this.getPopupEntries(users);
		const commentSection = app.sectionContainer.getSectionWithName(
			app.CSections.CommentList.name,
		);

		const isMobileCommentActive = commentSection?.isMobileCommentActive();
		const mobileCommentModalId = commentSection?.getMobileCommentModalId();
		if (
			entries.length === 0 &&
			this.isSmallScreenDevice &&
			isMobileCommentActive
		) {
			const control = this.getTreeJSON();
			control.hideIfEmpty = true;
			const data = this.getPopupJSON(control, { x: 0, y: 0 });
			data.id = mobileCommentModalId;
			(data.control as TreeWidgetJSON).entries = [];
			this.sendUpdate(data);
			return;
		}

		// change start position if cursor Y position changes.
		// It happens when user is typing mention at the end where there is no
		// horizontal space and whole '@mention' goes to new line
		const currentPos = this.getCursorPosition();
		let cursorPos = { ...this.cursorPosAtStart }; // Make a copy so changes to cursorPos don’t affect the original position
		if (this.cursorPosAtStart.y !== currentPos.y) {
			cursorPos = currentPos;
			this.cursorPosAtStart = currentPos;
		}
		// popup mention should have total top margin of navigation bar + if toolbar present then toolbar height
		var canvasEl = this.map._docLayer._canvas.getBoundingClientRect();
		cursorPos.y += canvasEl.top;
		if (entries.length === 0) {
			// If the key pressed was a space, and there are no matches, then just
			// dismiss the popup.
			const noMatchOnSpace = this.getPartialMention().indexOf(' ');
			if (noMatchOnSpace !== -1) {
				this.closeMentionPopup(false);
				return;
			}
			const control = this.getSimpleTextJSON();
			if (window.L.DomUtil.get(this.popupId + 'fixedtext')) {
				const data = this.getPopupJSON(control, cursorPos);
				this.sendUpdate(data);
				return;
			}
			if (window.L.DomUtil.get(this.popupId)) this.closeMentionPopup(true);
			const data = this.newPopupData;
			data.children[0].children[0] = control;
			data.posx = cursorPos.x;
			data.posy = cursorPos.y;
			this.sendJSON(data);
			return;
		}

		const control = this.getTreeJSON();
		if (isMobileCommentActive) control.hideIfEmpty = true;
		if (window.L.DomUtil.get(this.popupId + 'List')) {
			const data = this.getPopupJSON(control, cursorPos);
			if (isMobileCommentActive) data.id = mobileCommentModalId;
			(data.control as TreeWidgetJSON).entries = entries;
			this.sendUpdate(data);
			return;
		}

		if (window.L.DomUtil.get(this.popupId)) this.closeMentionPopup(true);
		const data = this.newPopupData;
		data.children[0].children[0] = control;
		(data.children[0].children[0] as TreeWidgetJSON).entries = entries;
		data.posx = cursorPos.x;
		data.posy = cursorPos.y;
		this.sendJSON(data);
	}

	private getSimpleTextJSON(): TextWidget {
		return {
			id: this.popupId + 'fixedtext',
			type: 'fixedtext',
			text: _('No search results found!'),
			enabled: true,
		} as TextWidget;
	}

	closeMentionPopup(typingMention: boolean): void {
		this.typingMention = typingMention;
		if (!typingMention) this.partialMention = [];

		const mentionPopup =
			window.L.DomUtil.get(this.popupId) ||
			window.L.DomUtil.get(this.popupId + 'List') ||
			window.L.DomUtil.get(this.popupId + 'fixedtext');
		if (!mentionPopup) return;

		this.map.jsdialog.focusToLastElement(this.popupId);
		if (this.isSmallScreenDevice) {
			const commentSection = app.sectionContainer.getSectionWithName(
				app.CSections.CommentList.name,
			);
			const isMobileCommentActive = commentSection?.isMobileCommentActive();
			const mobileCommentModalId = commentSection?.getMobileCommentModalId();

			if (isMobileCommentActive) {
				const control = this.getTreeJSON();
				control.hideIfEmpty = true;
				const data = this.getPopupJSON(control, { x: 0, y: 0 });
				data.id = mobileCommentModalId;
				(data.control as TreeWidgetJSON).entries = [];
				this.sendUpdate(data);
			} else {
				this.map.fire('closemobilewizard');
			}
		} else this.map.jsdialog.clearDialog(this.popupId);
	}

	// get partialMention excluding '@'
	getPartialMention(): string {
		return this.partialMention.join('').substring(1);
	}

	isTypingMention(): boolean {
		return this.typingMention;
	}

	handleMentionInput(ev: any, newPara: boolean) {
		if (!this.typingMention) {
			const isAtSymbol = ev.data === '@';
			const isLastCharAtOrSpace =
				this.lastTypedChar === ' ' || this.lastTypedChar === '@';
			if ((newPara && isAtSymbol) || (isAtSymbol && isLastCharAtOrSpace)) {
				this.partialMention.push(ev.data);
				this.typingMention = true;
				this.cursorPosAtStart = this.getCursorPosition();
				return;
			}
			this.lastTypedChar = ev.data;
			return;
		}

		const deleteEvent =
			ev.inputType === 'deleteContentBackward' ||
			ev.inputType === 'deleteContentForward';
		if (deleteEvent) {
			const ch = this.partialMention.pop();
			if (ch === '@') this.closeMentionPopup(false);
			else this.sendMentionPostMsg(this.getPartialMention());
			return;
		}

		if (ev.data === '@' && this.partialMention.length === 1) {
			return;
		}

		const regEx = /^[0-9a-zA-Z ]+$/;
		if (ev.data && ev.data.match(regEx)) {
			this.partialMention.push(ev.data);
			this.sendMentionPostMsg(this.getPartialMention());
		} else {
			this.closeMentionPopup(false);
		}
	}

	getMentionUserData(index: number): MentionUserData {
		if (index >= this.filteredUsers.length)
			return { username: '', profile: '', label: null } as MentionUserData;
		return this.filteredUsers[index];
	}

	private sendHyperlinkUnoCommand(
		uid: string,
		profile: string,
		replacement: string,
	) {
		var command = {
			'Hyperlink.Text': {
				type: 'string',
				value: '@' + uid,
			},
			'Hyperlink.URL': {
				type: 'string',
				value: profile,
			},
			'Hyperlink.ReplacementText': {
				type: 'string',
				value: replacement,
			},
		};
		this.map.sendUnoCommand('.uno:SetHyperlink', command, true);
	}

	callback(objectType: any, eventType: any, object: any, index: number) {
		const commentSection = app.sectionContainer.getSectionWithName(
			app.CSections.CommentList.name,
		);
		const comment = commentSection?.getActiveEdit();
		if (eventType === 'close') {
			this.closeMentionPopup(false);
		} else if (eventType === 'select' || eventType === 'activate') {
			const username = this.filteredUsers[index].username;
			const profileLink = this.filteredUsers[index].profile;
			const label = this.filteredUsers[index].label;
			const replacement = '@' + this.getPartialMention();

			if (comment) {
				comment.autoCompleteMention(
					label ?? username,
					profileLink,
					replacement,
				);
			} else {
				this.sendHyperlinkUnoCommand(
					label ?? username,
					profileLink,
					replacement,
				);
				this.map._textInput._sendText(' ');
			}
			this.map.fire('postMessage', {
				msgId: 'UI_Mention',
				args: { type: 'selected', username: username, label: label },
			});
			this.closeMentionPopup(false);
		} else if (eventType === 'keydown') {
			if (object.key !== 'Tab' && object.key !== 'Shift') {
				if (comment) comment.focus();
				else this.map.focus();
				return true;
			}
		}
		return false;
	}
}
