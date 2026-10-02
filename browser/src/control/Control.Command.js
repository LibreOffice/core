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
/*
 * Feature blocking handler
 */

/* global $ _ app UnlockPopup */

window.L.Map.include({

	Locking: {
		isLockedUser: false,
		isLockReadOnly: false,
		lockedCommandList: [],
		unlockTitle: '',
		unlockLink: '',
		unlockDescription: '',
		writerHighlights: '',
		calcHighlights: '',
		impressHighlights: '',
		drawHighlights: '',
	},

	_setLockProps: function(lockInfo) {
		this.Locking.isLockedUser = !!lockInfo['IsLockedUser'];
		this.Locking.isLockReadOnly = !!lockInfo['IsLockReadOnly'];
		this.Locking.lockedCommandList = lockInfo['LockedCommandList'];
		this.Locking.unlockTitle = _(lockInfo['UnlockTitle']);
		this.Locking.unlockLink = _(lockInfo['UnlockLink']);
		this.Locking.unlockDescription = _(lockInfo['UnlockDescription']);
		this.Locking.writerHighlights = _(lockInfo['WriterHighlights']);
		this.Locking.calcHighlights = _(lockInfo['CalcHighlights']);
		this.Locking.impressHighlights = _(lockInfo['ImpressHighlights']);
		this.Locking.drawHighlights = _(lockInfo['DrawHighlights']);
		this.Locking.unlockImageUrlPath = lockInfo['UnlockImageUrlPath'];
	},

	// We mark the element disabled for the feature locking
	// and add overlay on the element
	disableLockedItem: function(item, DOMParentElement, buttonToDisable) {
		if (this.isLockedUser() && this.isLockedItem(item)) {
			$(DOMParentElement).data('locked', true);
			$(DOMParentElement).addClass('locking-disabled');
			$(buttonToDisable).off('click');

			var that = this;

			if (window.mode.isSmallScreenDevice()) {
				var overlay = window.L.DomUtil.create('div', 'locking-overlay', DOMParentElement);
				var lock = window.L.DomUtil.create('img', 'locking-overlay-lock', overlay);
				app.LOUtil.setImage(lock, 'lc_lock.svg', this);
			}

			$(DOMParentElement).click(function(event) {
				event.stopPropagation();
				that.openUnlockPopup('');
			});
		}
	},

	openUnlockPopup: function(cmd) {
		if ((this.isRestrictedUser() && this.isRestrictedItem(cmd)) || this.uiManager.isAnyDialogOpen())
			return;
		const modalId = 'unlock-features-popup';
                this.uiManager.showInfoModal(modalId, null, ' ', ' ', _('Unlock'), () => {
                        window.open(this.Locking.unlockLink, '_blank');
                        this.uiManager.closeModal(this.uiManager.generateModalId('unlock-features-popup'));
                }, true);

		app.layoutingService.appendLayoutingTask(() => {
			let modal = document.getElementById(modalId);
			const imageUrl = this.Locking.unlockImageUrlPath
				? 'remote/static' + this.Locking.unlockImageUrlPath
				: 'images/lock-illustration.svg';
			const highlights = [
				this.Locking.writerHighlights,
				this.Locking.calcHighlights,
				this.Locking.impressHighlights,
				this.Locking.drawHighlights,
			].filter(Boolean);
			const content = UnlockPopup.build(imageUrl, this.Locking.unlockTitle,
				this.Locking.unlockDescription, highlights);
			const paraTag = modal.querySelector('p');
			if (paraTag)
				paraTag.replaceWith(content);
			else
				modal.insertBefore(content, modal.firstChild);
		});
	},

	isLockedItem: function(item) {
		var commands = this._extractCommand(item);

		for (var i in commands) {
			if (this.Locking.lockedCommandList.indexOf(commands[i]) >= 0)
				return true;
		}
		return false;
	},

	isLockedUser: function() {
		return this.Locking.isLockedUser;
	},

	Restriction: {
		isRestrictedUser: false,
		restrictedCommandList: [],
	},

	_setRestrictions: function(restrictionInfo) {
		this.Restriction.isRestrictedUser = !!restrictionInfo['IsRestrictedUser'];
		this.Restriction.restrictedCommandList = restrictionInfo['RestrictedCommandList'];
	},

	isRestrictedUser: function() {
		return this.Restriction.isRestrictedUser;
	},

	hideRestrictedItems: function(item, DOMParentElement, buttonToDisable) {
		if (this.isRestrictedUser() && this.isRestrictedItem(item)) {
			$(buttonToDisable).addClass('restricted-item');
			window.app.console.log();
		}

	},

	isRestrictedItem: function(item) {
		var commands = this._extractCommand(item);

		for (var i in commands) {
			if (this.Restriction.restrictedCommandList.indexOf(commands[i]) >= 0)
				return true;
		}
		return false;
	},

	isLockReadOnly: function() {
		return this.Locking.isLockReadOnly;
	},

	isLockedReadOnlyUser: function() {
		return this.Locking.isLockedUser && this.Locking.isLockReadOnly;
	},

	_extractCommand: function(item) {
		if (!item)
			return '';

		var commandArray = [];
		if (item.lockUno || item.uno) { // in classic mode uno commands are stored as uno in menus
			var uno = item.lockUno ? item.lockUno : item.uno;
			if (typeof uno === 'string')
				commandArray.push(uno);
			else { // some unos have multiple commands
				commandArray.push(uno.textCommand);
				commandArray.push(uno.objectCommand);
			}
		}
		else if (item.command) // in notebookbar uno commands are stored as command
			commandArray.push(item.command);
		else if (item.id)
			commandArray.push(item.id);
		else if (typeof item === 'string')
			commandArray.push(item);

		for (var command in commandArray) {
			if (!commandArray[command].startsWith('.uno:'))
				commandArray[command] = '.uno:' + commandArray[command];
		}
		return commandArray;
	}
});
