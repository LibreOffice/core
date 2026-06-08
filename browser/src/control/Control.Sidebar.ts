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
 * JSDialog.Sidebar
 */

// /* global app */
class Sidebar extends SidebarBase {
	targetDeckCommand: string;
	isUserRequest: boolean; /// automatic or user request to show the sidebar
	sidebarShownTheFirstTime: boolean = true;

	constructor(map: MapInterface) {
		super(map, SidebarType.Sidebar);
		this.isUserRequest = false;
	}

	onAdd(map: MapInterface) {
		super.onAdd(map);
		this.map.on('sidebar', this.onSidebar, this);
	}

	onRemove() {
		super.onRemove();
		this.map.off('sidebar');
	}

	setAsInitialized() {
		this.isUserRequest = false;
	}

	updateSidebarPrefs(currentDeck: string) {
		// No longer used:
		// 'SdSlideTransitionDeck'
		// 'SdMasterPagesDeck',

		const decks = [
			'PropertyDeck',
			'SdCustomAnimationDeck',
			'SdMasterPagesDeck',
			'StyleListDeck',
			'A11yCheckDeck',
		];

		const deckPref: { [key: string]: string } = {};
		decks.forEach((deck: string) => {
			deckPref[deck] = currentDeck === deck ? 'true' : 'false';
		});
		this.map.uiManager.setDocTypeMultiplePrefs(deckPref);
	}

	/// The presentation sidebar/panel toolbar buttons are mutually exclusive, so
	/// only the button of the active deck stays highlighted. Core reports the
	/// state of its own decks, but the transitions and animations panels live in
	/// the notebookbar and core is never told they took over the sidebar. Their
	/// highlight, and clearing the core deck button they replace, is driven here.
	updatePresentationDeckHighlight(currentDeck: string) {
		if (this.map.getDocType() !== 'presentation') return;

		const panelCommandForDeck: { [key: string]: string } = {
			'transitions-deck': 'transitiondeck',
			'animations-deck': 'animationdeck',
		};
		const coreDeckCommands = [
			'.uno:SidebarDeck.PropertyDeck',
			'.uno:ModifyPage',
			'.uno:CustomAnimation',
			'.uno:MasterSlidesPanel',
		];

		const activePanelCommand = panelCommandForDeck[currentDeck];
		const stateHandler = this.map['stateChangeHandler'];

		const setHighlight = (command: string, active: boolean) => {
			const value = active ? 'true' : 'false';
			if (stateHandler && stateHandler.getItemValue(command) === value) return;
			this.map.fire('commandstatechanged', {
				commandName: command,
				state: value,
			});
		};

		Object.values(panelCommandForDeck).forEach((command) =>
			setHighlight(command, command === activePanelCommand),
		);

		// A notebookbar panel has taken over the sidebar: core still thinks its
		// last deck is open and keeps reporting it active, so clear those buttons.
		if (activePanelCommand)
			coreDeckCommands.forEach((command) => setHighlight(command, false));
	}

	commandForDeck(deckId: string): string {
		// No longer used:
		// if (deckId === 'SdSlideTransitionDeck') return '.uno:SlideChangeWindow';
		// else if (deckId === 'SdMasterPagesDeck') return '.uno:MasterSlidesPanel';

		if (deckId === 'PropertyDeck') return '.uno:SidebarDeck.PropertyDeck';
		else if (deckId === 'SdCustomAnimationDeck') return '.uno:CustomAnimation';
		else if (deckId === 'SdMasterPagesDeck') return '.uno:MasterSlidesPanel';
		else if (deckId === 'StyleListDeck')
			return '.uno:SidebarDeck.StyleListDeck';
		else if (deckId === 'A11yCheckDeck')
			return '.uno:SidebarDeck.A11yCheckDeck';
		return '';
	}

	setupTargetDeck(unoCommand: string | null) {
		this.targetDeckCommand = unoCommand;
	}

	getTargetDeck(): string {
		return this.targetDeckCommand;
	}

	changeDeck(unoCommand: string | null) {
		if (unoCommand !== null && unoCommand !== undefined)
			app.socket.sendMessage('uno ' + unoCommand);
		this.setupTargetDeck(unoCommand);
	}

	onSidebar(data: FireEvent) {
		var sidebarData = data.data;

		if (
			sidebarData.action === 'close' ||
			window.app.file.disableSidebar ||
			this.map.isReadOnlyMode()
		) {
			this.closeSidebar();
		} else if (sidebarData.children) {
			for (var i = sidebarData.children.length - 1; i >= 0; i--) {
				if (sidebarData.children[i].type !== 'deck') {
					sidebarData.children.splice(i, 1);
					continue;
				}

				if (
					typeof sidebarData.children[i].id === 'string' &&
					sidebarData.children[i].id.startsWith('Navigator')
				) {
					this.markNavigatorTreeView(sidebarData);
				}
			}

			if (sidebarData.children.length) {
				this.onResize();

				if (
					sidebarData.children &&
					sidebarData.children[0] &&
					sidebarData.children[0].id
				) {
					var currentDeck = sidebarData.children[0].id;
					this.updateSidebarPrefs(currentDeck);
					this.updatePresentationDeckHighlight(currentDeck);
					if (this.targetDeckCommand) {
						var stateHandler = this.map['stateChangeHandler'];
						var isCurrent = stateHandler
							? stateHandler.getItemValue(this.targetDeckCommand)
							: false;
						// just to be sure check with other method
						if (isCurrent === 'false' || !isCurrent)
							isCurrent =
								this.targetDeckCommand === this.commandForDeck(currentDeck);
						if (this.targetDeckCommand && (isCurrent === 'false' || !isCurrent))
							this.changeDeck(this.targetDeckCommand);
					} else {
						this.changeDeck(this.targetDeckCommand);
					}
				}

				this.model.fullUpdate(sidebarData as JSDialogJSON);

				const documentFragment = new DocumentFragment(); // do not modify dom yet
				const tempContainer = window.L.DomUtil.create(
					'div',
					'',
					documentFragment,
				);

				this.builder.build(tempContainer, [this.model.getSnapshot()], false);

				if (!this.isVisible()) {
					this.showSidebar();

					// on initial load of file do not focus automatically
					if (!this.sidebarShownTheFirstTime) this.isUserRequest = true;
				}

				this.map.uiManager.setDocTypePref('ShowSidebar', true);

				// cache - check happens in task and we will update value later in this function
				const wasUserRequest = this.isUserRequest;

				app.layoutingService.appendLayoutingTask(() => {
					// now attach to the DOM built content
					this.container.replaceChildren(tempContainer.firstChild);

					// schedule focus after animation so it will not shift the browser page
					if (wasUserRequest) {
						app.timerRegistry.setTimeout(
							'sidebarstealfocus',
							() => {
								app.layoutingService.appendLayoutingTask(() => {
									if (
										this.map.dialog.hasOpenedDialog() ||
										(this.map.jsdialog && this.map.jsdialog.hasDialogOpened())
									)
										return;
									const focusables = JSDialog.GetFocusableElements(
										this.container,
									);
									if (focusables && focusables.length) {
										focusables[0].focus();
									}
								});
							},
							250,
						); // see animation time in #sidebar-dock-wrapper.visible
					}

					if (this.sidebarShownTheFirstTime) {
						app.serverConnectionService.onShowSidebar();
						this.sidebarShownTheFirstTime = false;
					}
				});

				this.isUserRequest = false;
			} else {
				this.closeSidebar();
				this.isUserRequest = true;
			}
		}
	}
}

JSDialog.Sidebar = function (map: MapInterface) {
	return new Sidebar(map);
};
