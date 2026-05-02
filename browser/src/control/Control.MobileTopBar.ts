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
 * JSDialog.MobileTopBar - component of top bar on mobile
 */

class MobileTopBar extends JSDialog.Toolbar {
	constructor(map: any) {
		super(map, 'MobileTopBar', 'toolbar-up');

		app.events.on('updatepermission', this.onUpdatePermission.bind(this));
		map.on('commandstatechanged', this.onCommandStateChanged, this);
	}

	private getToolItems() {
		if (this.docType == 'text') {
			return [
				{type: 'toolitem', id: 'signstatus', command: '.uno:Signature', w2icon: '', text: _UNO('.uno:Signature'), visible: false},
				{type: 'toolitem',  id: 'undo', text: _UNO('.uno:Undo'), command: '.uno:Undo', enabled: false},
				{type: 'toolitem',  id: 'redo', text: _UNO('.uno:Redo'), command: '.uno:Redo', enabled: false},
				{type: 'spacer', id: 'before-permissionmode'},
				this._generateHtmlItem('permissionmode'),
				{type: 'spacer', id: 'after-permissionmode'},
				{type: 'customtoolitem',  id: 'mobile_wizard', command: 'mobile_wizard'},
				{type: 'customtoolitem',  id: 'insertion_mobile_wizard', command: 'insertion_mobile_wizard'},
				{type: 'customtoolitem',  id: 'comment_wizard', command: 'comment_wizard', w2icon: 'viewcomments'},
				{type: 'menubutton', id: 'userlist:UsersListMenu', visible: false},
			];
		} else if (this.docType == 'spreadsheet') {
			return [
				{type: 'toolitem', id: 'signstatus', command: '.uno:Signature', w2icon: '', text: _UNO('.uno:Signature'), visible: false},
				{type: 'toolitem',  id: 'undo', text: _UNO('.uno:Undo'), command: '.uno:Undo', enabled: false},
				{type: 'toolitem',  id: 'redo', text: _UNO('.uno:Redo'), command: '.uno:Redo', enabled: false},
				{type: 'customtoolitem', visible: false, id: 'acceptformula', command: 'acceptformula', text: _('Accept')},
				{type: 'customtoolitem', visible: false, id: 'cancelformula', command: 'cancelformula', text: _('Cancel')},
				{type: 'spacer', id: 'before-PermissionMode'},
				this._generateHtmlItem('permissionmode'),
				{type: 'spacer', id: 'after-PermissionMode'},
				{type: 'customtoolitem',  id: 'mobile_wizard', command: 'mobile_wizard'},
				{type: 'customtoolitem',  id: 'insertion_mobile_wizard', command: 'insertion_mobile_wizard'},
				{type: 'customtoolitem',  id: 'comment_wizard', command: 'comment_wizard', w2icon: 'viewcomments'},
				{type: 'menubutton', id: 'userlist:UsersListMenu', visible: false},
			];
		} else if (this.docType == 'presentation') {
			return [
				{type: 'toolitem', id: 'signstatus', command: '.uno:Signature', w2icon: '', text: _UNO('.uno:Signature'), visible: false},
				{type: 'toolitem',  id: 'undo', text: _UNO('.uno:Undo'), command: '.uno:Undo', enabled: false},
				{type: 'toolitem',  id: 'redo', text: _UNO('.uno:Redo'), command: '.uno:Redo', enabled: false},
				{type: 'spacer', id: 'before-permissionmode'},
				this._generateHtmlItem('permissionmode'),
				{type: 'spacer', id: 'after-permissionmode'},
				{type: 'customtoolitem',  id: 'mobile_wizard', command: 'mobile_wizard'},
				{type: 'customtoolitem',  id: 'insertion_mobile_wizard', command: 'insertion_mobile_wizard'},
				{type: 'customtoolitem',  id: 'comment_wizard', command: 'comment_wizard', w2icon: 'viewcomments'},
				{type: 'customtoolitem', id: 'fullscreen-' + this.docType + '-toolbar-mobile', text: _UNO('.uno:FullScreen', this.docType)},
				{type: 'menubutton', id: 'userlist:UsersListMenu', visible: false},
			];
		} else if (this.docType == 'drawing') {
			return [
				{type: 'toolitem', id: 'signstatus', command: '.uno:Signature', w2icon: '', text: _UNO('.uno:Signature'), visible: false},
				{type: 'toolitem',  id: 'undo', text: _UNO('.uno:Undo'), command: '.uno:Undo', enabled: false},
				{type: 'toolitem',  id: 'redo', text: _UNO('.uno:Redo'), command: '.uno:Redo', enabled: false},
				{type: 'spacer', id: 'before-PermissionMode'},
				this._generateHtmlItem('permissionmode'),
				{type: 'spacer', id: 'after-PermissionMode'},
				{type: 'customtoolitem',  id: 'mobile_wizard', command: 'mobile_wizard'},
				{type: 'customtoolitem',  id: 'insertion_mobile_wizard', command: 'insertion_mobile_wizard'},
				{type: 'customtoolitem',  id: 'comment_wizard', command: 'comment_wizard', w2icon: 'viewcomments'},
				{type: 'menubutton', id: 'userlist:UsersListMenu', visible: false},
			];
		}
	}

	create() {
		const items = this.getToolItems();
		this.builder.build(this.parentContainer, items);
	}

	onUpdatePermission(e: any) {
		const toolbarButtons: string[] = ['undo', 'redo', 'mobile_wizard', 'insertion_mobile_wizard', 'comment_wizard'];
		if (e.detail.perm === 'edit') {
			toolbarButtons.forEach((id) => {
				this.showItem(id, true);
			});
			this.showItem('PermissionMode', false);
		} else {
			toolbarButtons.forEach((id) => {
				this.showItem(id, false);
			});
			this.showItem('comment_wizard', true);
			if ($('#mobile-edit-button').is(':hidden')) {
				this.showItem('PermissionMode', true);
			}
		}
	}

	onCommandStateChanged(e: any) {
		const commandName: string = e.commandName;
		const state: string = e.state;

		if (this.map.isEditMode() && (state === 'enabled' || state === 'disabled')) {
			const id: string = (window as any).unoCmdToToolbarId(commandName);

			if (state === 'enabled') {
				this.enableItem(id, true);
			} else {
				//this.uncheck(id);
				this.enableItem(id, false);
			}
		}
	}

	_generateHtmlItem(id: string) {
		const isReadOnlyMode: boolean = app.map ? app.isReadOnly() : true;
		const canUserWrite: boolean = !app.isReadOnly();

		return {
			type: 'container',
			id: id + '-container',
			children: [
				{type: 'htmlcontent', id: id, htmlId: id, text: '&nbsp;&nbsp;&nbsp;&nbsp;&nbsp', isReadOnlyMode: isReadOnlyMode, canUserWrite: canUserWrite, visible: false},
				{type: 'spacer', id: id + '-break'}
			],
			vertical: false,
		};
	}
}

JSDialog.MobileTopBar = function (map: any) {
	return new MobileTopBar(map);
};
