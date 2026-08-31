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

interface ExtensionDeckOwner {
	closeDeck(): void;
}

interface ExtensionDeckSlot {
	owner: ExtensionDeckOwner;
	/// false while the switch to the properties deck we asked for is still ongoing
	settled: boolean;
}

/// All the deck-navigation state the sidebar keeps, held as one value.
interface SidebarDeckState {
	activeDeckId: string | null; /// deck the last message showed, null while closed
	targetDeckCommand: string | null; /// deck command last asked for
	/// deck showing when the current deck was requested, and the command that requested it
	openedFrom: { deckId: string | null; forCommand: string } | null;
}

class Sidebar extends SidebarBase {
	isUserRequest: boolean; /// automatic or user request to show the sidebar
	sidebarShownTheFirstTime: boolean = true;

	/// The whole deck-navigation state, changed only through setDeckState. Keeping it in one
	/// value with one writer means the sidebar has a single place its state can move.
	deckState: SidebarDeckState = {
		activeDeckId: null,
		targetDeckCommand: null,
		openedFrom: null,
	};

	/// the extension currently showing, if any
	private extensionDeck: ExtensionDeckSlot | null = null;

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

	/// The single point where deck-navigation state changes.
	setDeckState(patch: Partial<SidebarDeckState>) {
		this.deckState = { ...this.deckState, ...patch };
	}

	closeSidebar() {
		if (this.extensionDeck) this.extensionDeck.owner.closeDeck();
		super.closeSidebar();
		this.setDeckState({ activeDeckId: null });
	}

	hasExtensionDeck(owner: ExtensionDeckOwner): boolean {
		return !!this.extensionDeck && this.extensionDeck.owner === owner;
	}

	takeExtensionDeckSlot(owner: ExtensionDeckOwner) {
		const sidebarPanel = document.getElementById(`${this.type}-panel`);
		if (!sidebarPanel) return;

		const previous = this.extensionDeck;
		if (previous && previous.owner !== owner) previous.owner.closeDeck();

		const wasVisible = this.isVisible();
		const onProperties =
			wasVisible && this.deckState.activeDeckId === 'PropertyDeck';
		this.extensionDeck = { owner, settled: onProperties };
		sidebarPanel.classList.add('extension-deck-shown');

		if (!onProperties) this.map.sendUnoCommand('.uno:SidebarDeck.PropertyDeck');
		if (!wasVisible) this.showSidebar();
	}

	releaseExtensionDeckSlot(owner: ExtensionDeckOwner) {
		if (!this.hasExtensionDeck(owner)) return;
		this.extensionDeck = null;
		const sidebarPanel = document.getElementById(`${this.type}-panel`);
		if (sidebarPanel) sidebarPanel.classList.remove('extension-deck-shown');
	}

	private updateExtensionDeckForCoreDeck(activeDeckId: string) {
		const slot = this.extensionDeck;
		if (!slot) return;
		if (activeDeckId === 'PropertyDeck') slot.settled = true;
		else if (slot.settled) slot.owner.closeDeck();
	}

	/// Records where a deck request was sent from, tagged with the command it was sent for.
	/// Runs when the request is sent, before any deck switching starts. Consulted in
	/// onSidebar only when the deck that arrives matches forCommand, so the transient deck
	/// changes core makes while carrying the request out cannot misapply the origin.
	noteDeckRequestOrigin(command: string) {
		this.setDeckState({
			openedFrom: {
				deckId: this.isVisible() ? this.deckState.activeDeckId : null,
				forCommand: command,
			},
		});
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
		else if (deckId === 'ElementsDeck') return '.uno:SidebarDeck.ElementsDeck';
		return '';
	}

	/// Adds the close button command to the first panel of the shown overlay deck.
	/// The properties deck is the main sidebar; every other deck is an overlay and carries
	/// one close button, in the header of its first panel. Entered from the properties deck,
	/// closing returns there; opened while the sidebar was closed, the deck's own command
	/// toggles it off, which closes the whole sidebar. The origin is trusted only when the
	/// arriving deck matches the command it was recorded for, so a core-driven switch cannot
	/// inherit it. A deck with no known command, like the navigator, is left alone.
	markOverlayDeckCloseButton(sidebarData: any, activeDeckId: string) {
		if (activeDeckId === 'PropertyDeck') return;

		const deckCommand = this.commandForDeck(activeDeckId);
		if (!deckCommand) return;

		const activeDeck = sidebarData.children.find(
			(deck: any) => deck.id === activeDeckId,
		);
		const firstPanel = (activeDeck.children || []).find(
			(child: any) => child.type === 'panel',
		);
		if (!firstPanel) return;

		const origin = this.deckState.openedFrom;
		const openedFromProperty =
			origin &&
			origin.forCommand === deckCommand &&
			origin.deckId === 'PropertyDeck';
		firstPanel.closeCommand = openedFromProperty
			? '.uno:SidebarDeck.PropertyDeck'
			: deckCommand;
	}

	setupTargetDeck(unoCommand: string | null) {
		this.setDeckState({ targetDeckCommand: unoCommand });
	}

	getTargetDeck(): string {
		return this.deckState.targetDeckCommand;
	}

	changeDeck(unoCommand: string | null) {
		if (unoCommand !== null && unoCommand !== undefined)
			app.socket.sendMessage('uno ' + unoCommand);
		this.setupTargetDeck(unoCommand);
	}

	onSidebar(data: FireEvent) {
		var sidebarData = data.data;

		if (window.app.file.disableSidebar || this.map.isReadOnlyMode()) {
			this.closeSidebar();
		} else if (sidebarData.action === 'close') {
			// While a panel deck owns the dock content, closing the core decks only
			// drops the core deck state and the dock stays as the panel left it.
			if (this.ownsContainerContent()) this.closeSidebar();
			else this.setDeckState({ activeDeckId: null });
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
					// The message lists every deck and marks the inactive ones with
					// visible false, so the deck being shown is the one left visible.
					const activeDeckId = (
						sidebarData.children.find((deck: any) => deck.visible !== false) ||
						sidebarData.children[0]
					).id;
					this.setDeckState({ activeDeckId });
					this.updateExtensionDeckForCoreDeck(activeDeckId);
					this.updateSidebarPrefs(activeDeckId);
					this.updatePresentationDeckHighlight(activeDeckId);

					this.markOverlayDeckCloseButton(sidebarData, activeDeckId);

					const target = this.deckState.targetDeckCommand;
					if (target) {
						var stateHandler = this.map['stateChangeHandler'];
						var isCurrent = stateHandler
							? stateHandler.getItemValue(target)
							: false;
						// just to be sure check with other method
						if (isCurrent === 'false' || !isCurrent)
							isCurrent = target === this.commandForDeck(activeDeckId);
						if (target && (isCurrent === 'false' || !isCurrent))
							this.changeDeck(target);
					} else {
						this.changeDeck(target);
					}
				}

				this.model.fullUpdate(sidebarData as JSDialogJSON);
				this.markContainerContentOwner();

				const documentFragment = new DocumentFragment(); // do not modify dom yet
				const tempContainer = window.L.DomUtil.create(
					'div',
					'',
					documentFragment,
				);

				this.builder.build(tempContainer, [this.model.getSnapshot()], false);

				if (!this.isVisible()) {
					this.showSidebar();

					if (this.sidebarShownTheFirstTime) {
						// The resize that the new width triggers is what refits the
						// zoom, so ask before that resize is handled.
						app.serverConnectionService.onShowSidebar();
					} else {
						// on initial load of file do not focus automatically
						this.isUserRequest = true;
					}
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

					this.sidebarShownTheFirstTime = false;
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
