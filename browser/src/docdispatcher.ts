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

/* global app _ */

/*
	app.dispatcher.dispatch() will be used to call some actions so we can share the code
	This is mostly used by keyboard shortcuts etc, which need a base (for simplicity) to call actions.
*/

class Dispatcher {
	private actionsMap: any = {};

	private addGeneralCommands() {
		this.actionsMap['save'] = function (source?: string) {
			// Save only when not read-only.
			if (!app.map.isReadOnlyMode() && !app.map['wopi'].HideSaveOption) {
				app.map.fire('postMessage', {
					msgId: 'UI_Save',
					args: { source: source || 'toolbar' },
				});
				if (!app.map._disableDefaultAction['UI_Save']) {
					app.map.save(
						false /* An explicit save should terminate cell edit */,
						false /* An explicit save should save it again */,
					);
				}
			}
		};

		this.actionsMap['closeapp'] = () => {
			if ((window as any).ThisIsAMobileApp) {
				window.postMobileMessage('BYE');
			} else {
				if (
					app.map &&
					app.map.formulabar &&
					(app.map.formulabar.hasFocus() || app.map.formulabar.isInEditMode())
				) {
					this.dispatch('acceptformula'); // save data from the edited cell on exit
				}

				window.prefs.sendPendingBrowserSettingsUpdate();
				app.map.fire('postMessage', {
					msgId: 'close',
					args: { EverModified: app.map._everModified, Deprecated: true },
				});
				app.map.fire('postMessage', {
					msgId: 'UI_Close',
					args: { EverModified: app.map._everModified },
				});
			}
			if (!app.map._disableDefaultAction['UI_Close']) app.map.remove();
		};

		this.actionsMap['userlist'] = () => {
			if (app.map.userList) app.map.userList.openDropdown();
		};

		this.actionsMap['print'] = function () {
			app.map.print();
		};
		this.actionsMap['print-notespages'] = function () {
			// To Export only notes with slides both should be true (ExportNotesPages, ExportOnlyNotesPages).
			// As ExportOnlyNotesPages is kind of child condition to ExportNotesPages.
			const options = {
				ExportNotesPages: {
					type: 'boolean',
					value: true,
				},
				ExportOnlyNotesPages: {
					type: 'boolean',
					value: true,
				},
			};
			const optionsString = JSON.stringify(options);
			app.map.print(optionsString);
		};
		this.actionsMap['repair'] = function () {
			app.socket.sendMessage('commandvalues command=.uno:DocumentRepair');
		};

		this.actionsMap['remotelink'] = function () {
			app.map.fire('postMessage', { msgId: 'UI_PickLink' });
		};
		this.actionsMap['remoteaicontent'] = function () {
			app.map.fire('postMessage', { msgId: 'UI_InsertAIContent' });
		};
		// TODO: deduplicate
		this.actionsMap['hyperlinkdialog'] = function () {
			app.map.sendUnoCommand('.uno:HyperlinkDialog');
		};
		this.actionsMap['inserthyperlink'] = () => {
			app.map.sendUnoCommand('.uno:HyperlinkDialog');
		};
		this.actionsMap['rev-history'] = function () {
			app.map.openRevisionHistory();
		};
		this.actionsMap['shareas'] = function () {
			app.map.openShare();
		};

		this.actionsMap['savecomments'] = function () {
			if (app.isCommentEditingAllowed()) {
				app.map.fire('postMessage', { msgId: 'UI_Save' });
				if (!app.map._disableDefaultAction['UI_Save']) {
					app.map.save(false, false);
				}
			}
		};

		this.actionsMap['insertmultimedia'] = function () {
			window.L.DomUtil.get('insertmultimedia').click();
		};
		this.actionsMap['remotemultimedia'] = function () {
			app.map.fire('postMessage', {
				msgId: 'UI_InsertFile',
				args: {
					callback: 'Action_InsertMultimedia',
					mimeTypeFilter: app.LOUtil.mediaMimeFilter,
				},
			});
		};

		this.actionsMap['localcomparedocuments'] = function () {
			window.L.DomUtil.get('comparedocuments').click();
		};
		this.actionsMap['remotecomparedocuments'] = function () {
			app.map.fire('postMessage', {
				msgId: 'UI_InsertFile',
				args: {
					callback: 'Action_CompareDocuments',
					mimeTypeFilter: app.LOUtil.documentMimeFilter,
				},
			});
		};

		this.actionsMap['charmapcontrol'] = function () {
			app.map.sendUnoCommand('.uno:InsertSymbol');
		};
		this.actionsMap['closetablet'] = function () {
			app.map.uiManager.enterReadonlyOrClose();
		};

		this.actionsMap['toggledarktheme'] = function () {
			app.map.uiManager.toggleDarkMode();
		};
		this.actionsMap['invertbackground'] = function () {
			app.map.uiManager.toggleInvert();
		};
		this.actionsMap['home-search'] = function () {
			app.map.uiManager.focusSearch();
		};
		this.actionsMap['renamedocument'] = function () {
			app.map.uiManager.renameDocument();
		};
		this.actionsMap['togglewasm'] = function () {
			app.map.uiManager.toggleWasm();
		};

		this.actionsMap['languagemenu'] = function () {
			app.map.fire('morelanguages');
		};
		this.actionsMap['morelanguages-selection'] = function () {
			app.map.fire('morelanguages', { applyto: 'selection' });
		};
		this.actionsMap['morelanguages-paragraph'] = function () {
			app.map.fire('morelanguages', { applyto: 'paragraph' });
		};
		this.actionsMap['morelanguages-all'] = function () {
			app.map.fire('morelanguages', { applyto: 'all' });
		};
		this.actionsMap['localgraphic'] = function () {
			window.L.DomUtil.get('insertgraphic').click();
		};
		this.actionsMap['remotegraphic'] = this.actionsMap['insertremotegraphic'] =
			function () {
				app.map.fire('postMessage', { msgId: 'UI_InsertGraphic' });
			};

		this.actionsMap['showhelp'] = function () {
			app.map.showHelp('online-help-content');
		};

		this.actionsMap['focustonotebookbar'] = function () {
			const tabsContainer = document.getElementsByClassName(
				'notebookbar-tabs-container',
			)[0].children[0];
			let elementToFocus: HTMLButtonElement;
			if (tabsContainer) {
				for (let i = 0; i < tabsContainer.children.length; i++) {
					if (tabsContainer.children[i].classList.contains('selected')) {
						elementToFocus = tabsContainer.children[i] as HTMLButtonElement;
						break;
					}
				}
			}
			if (!elementToFocus)
				elementToFocus = document.getElementById(
					'Home-tab-label',
				) as HTMLButtonElement;

			elementToFocus.focus();
		};

		this.actionsMap['saveas'] = function () {
			if (app.map && app.map.uiManager.getCurrentMode() === 'notebookbar') {
				app.map.openSaveAs(); // Opens save as dialog if integrator supports it.
			}
		};

		this.actionsMap['insertcomment'] = function () {
			app.map.insertComment();
		};

		this.actionsMap['insertthreadedcomment'] = function () {
			app.map.insertThreadedComment();
		};

		this.actionsMap['showcommentsnavigator'] = function (data?: any) {
			if (
				!document
					.getElementById('navigation-sidebar')
					.classList.contains('visible')
			)
				app.map.sendUnoCommand('.uno:Navigator');
			app.map.sendUnoCommand(
				'.uno:NavigatorSelectComment?CommentId:short=' +
					(data ? (data as number) : 0),
			);
		};

		this.actionsMap['zoomin'] = () => {
			app.map.zoomIn(1, null, true /* animate? */);
		};
		this.actionsMap['zoomout'] = () => {
			app.map.zoomOut(1, null, true /* animate? */);
		};
		this.actionsMap['zoomreset'] = () => {
			app.map.setZoom(app.map.options.zoom, null, true);
		};
		this.actionsMap['fitwidthzoom'] = () => {
			if (app.activeDocument.activeLayout)
				app.activeDocument.activeLayout.adjustViewZoomLevel();
		};

		this.actionsMap['searchprev'] = () => {
			app.searchService.searchPrevious();
		};
		this.actionsMap['searchnext'] = () => {
			app.searchService.searchNext();
		};
		this.actionsMap['cancelsearch'] = () => {
			app.map.cancelSearch();
		};
		this.actionsMap['showsearchbar'] = () => {
			$('#toolbar-down').hide();
			$('#showsearchbar').removeClass('over');
			$('#toolbar-search').show();
			if (!app.isReadOnly() && app.map.isReadOnlyMode())
				$('#mobile-edit-button').hide();
			window.L.DomUtil.get('search-input').focus();
		};
		this.actionsMap['hidesearchbar'] = () => {
			$('#toolbar-search').hide();
			if (app.map.isEditMode()) $('#toolbar-down').show();
			/** show edit button if only we are able to edit but in readonly mode */
			if (!app.isReadOnly() && app.map.isReadOnlyMode())
				$('#mobile-edit-button').css('display', 'flex');
		};

		this.actionsMap['prev'] = () => {
			if (app.map._docLayer._docType === 'text') app.map.goToPage('prev');
			else app.map.setPart('prev');
		};
		this.actionsMap['next'] = () => {
			if (app.map._docLayer._docType === 'text') app.map.goToPage('next');
			else app.map.setPart('next');
		};

		this.actionsMap['inserttextbox'] = () => {
			app.map.sendUnoCommand('.uno:Text?CreateDirectly:bool=true');
		};
		this.actionsMap['insertannotation'] = () => {
			app.map.insertComment();
		};

		this.actionsMap['fold'] = () => {
			app.map.uiManager.toggleMenubar();
		};

		this.actionsMap['close'] = this.actionsMap['closemobile'] = () => {
			app.map.uiManager.enterReadonlyOrClose();
		};

		this.actionsMap['serveraudit'] = () => {
			app.map.serverAuditDialog.open();
		};

		this.actionsMap['togglea11ystate'] = () => {
			if (app.map._lockAccessibilityOn) {
				return;
			}
			var prevAccessibilityState =
				window.prefs.getBoolean('accessibilityState');
			app.map.setAccessibilityState(!prevAccessibilityState);
		};

		this.actionsMap['toggleuimode'] = () => {
			if (app.map.uiManager.shouldUseNotebookbarMode()) {
				app.map.uiManager.onChangeUIMode({ mode: 'classic', force: true });
			} else {
				app.map.uiManager.onChangeUIMode({ mode: 'notebookbar', force: true });
			}
		};

		this.actionsMap['showruler'] = () => {
			app.map.uiManager.toggleRuler();
		};

		this.actionsMap['showstylelistdeck'] = () => {
			app.map.uiManager.showStyleListDeck();
		};

		this.actionsMap['showstatusbar'] = () => {
			app.map.uiManager.toggleStatusBar();
		};

		this.actionsMap['collapsenotebookbar'] = () => {
			app.map.uiManager.collapseNotebookbar();
		};

		this.actionsMap['validatedialogsa11y'] = () => {
			if (window.app.a11yValidator) {
				window.app.a11yValidator.validateAllOpenDialogs();
			} else {
				console.warn('A11yValidator not available');
			}
		};

		this.actionsMap['validatesidebara11y'] = () => {
			if (window.app.a11yValidator) {
				window.app.a11yValidator.validateSidebar();
			} else {
				console.warn('A11yValidator not available');
			}
		};

		this.actionsMap['validatenotebookbara11y'] = () => {
			if (window.app.a11yValidator) {
				window.app.a11yValidator.validateNotebookbar();
			} else {
				console.warn('A11yValidator not available');
			}
		};
	}

	private addAICommands() {
		this.actionsMap['aichat'] = function () {
			if (!app.map.isAIConfigured) {
				app.map.uiManager.showSnackbar(
					_(
						'AI is not configured. Go to File > Options > View Settings to set it up.',
					),
				);
				return;
			}
			const sidebar = JSDialog.getAIChatSidebar();
			sidebar.toggle();
		};

		this.actionsMap['helpfixformulaerror'] = function () {
			if (!app.map.isAIConfigured) {
				app.map.uiManager.showSnackbar(
					_(
						'AI is not configured. Go to File > Options > View Settings to set it up.',
					),
				);
				return;
			}
			const sidebar = JSDialog.getAIChatSidebar();
			if (!sidebar.isVisible()) sidebar.show();
			sidebar.diagnoseFormulaError();
		};
	}

	private addExportCommands() {
		this.actionsMap['exportpdf'] = function () {
			app.map.sendUnoCommand('.uno:ExportToPDF', {
				SynchronMode: {
					type: 'boolean',
					value: false,
				},
			});
		};

		this.actionsMap['exportdirectpdf'] = function () {
			app.map.sendUnoCommand('.uno:ExportDirectToPDF', {
				SynchronMode: {
					type: 'boolean',
					value: false,
				},
			});
		};

		this.actionsMap['exportepub'] = function () {
			app.map.sendUnoCommand('.uno:ExportToEPUB', {
				SynchronMode: {
					type: 'boolean',
					value: false,
				},
			});
		};
	}

	private addCalcCommands() {
		this.actionsMap['acceptformula'] = function () {
			if (window.mode.isSmallScreenDevice()) {
				app.map.focus();
				app.map._docLayer.postKeyboardEvent(
					'input',
					app.map.keyboard.keyCodes.enter,
					app.map.keyboard._toUNOKeyCode(app.map.keyboard.keyCodes.enter),
				);
			} else {
				app.map.sendUnoCommand('.uno:AcceptFormula');
			}

			app.map.onFormulaBarBlur();
			app.map.formulabarBlur();
			app.map.formulabarSetDirty();
		};

		this.actionsMap['cancelformula'] = function () {
			app.map.sendUnoCommand('.uno:Cancel');
			app.map.onFormulaBarBlur();
			app.map.formulabarBlur();
			app.map.formulabarSetDirty();
		};

		this.actionsMap['startformula'] = function () {
			app.map.sendUnoCommand('.uno:StartFormula');
			app.map.onFormulaBarFocus();
			app.map.formulabarFocus();
			app.map.formulabarSetDirty();
		};

		this.actionsMap['functiondialog'] = function () {
			if (window.mode.isSmallScreenDevice() && app.map._functionWizardData) {
				app.map._docLayer._closeMobileWizard();
				app.map._docLayer._openMobileWizard(app.map._functionWizardData);
				app.map.formulabarSetDirty();
			} else {
				app.map.sendUnoCommand('.uno:FunctionDialog');
			}
		};

		this.actionsMap['print-active-sheet'] = function () {
			const currentSheet = app.map._docLayer._selectedPart + 1;
			const options = {
				ExportFormFields: {
					type: 'boolean',
					value: false,
				},
				ExportNotes: {
					type: 'boolean',
					value: false,
				},
				SheetRange: {
					type: 'string',
					value: currentSheet + '-' + currentSheet,
				},
			};
			const optionsString = JSON.stringify(options);
			app.map.print(optionsString);
		};

		this.actionsMap['print-all-sheets'] = function () {
			app.map.print();
		};
		this.actionsMap['togglerelative'] = function () {
			app.map.sendUnoCommand('.uno:ToggleRelative');
		};
		this.actionsMap['focusonaddressinput'] = function () {
			document.getElementById('#addressInput input').focus();
		};

		// sheets toolbar
		this.actionsMap['insertsheet'] = function () {
			var nPos = $('#spreadsheet-tab-scroll')[0].childElementCount;
			app.map.insertPage(nPos);
			app.map.insertPage.scrollToEnd = true;
		};
		this.actionsMap['firstrecord'] = function () {
			$('#spreadsheet-tab-scroll').scrollLeft(0);
		};
		this.actionsMap['nextrecord'] = function () {
			// TODO: We should get visible tab's width instead of 60px
			$('#spreadsheet-tab-scroll').scrollLeft(
				$('#spreadsheet-tab-scroll').scrollLeft() + 60,
			);
		};
		this.actionsMap['prevrecord'] = function () {
			$('#spreadsheet-tab-scroll').scrollLeft(
				$('#spreadsheet-tab-scroll').scrollLeft() - 30,
			);
		};
		this.actionsMap['lastrecord'] = function () {
			// Set a very high value, so that scroll is set to the maximum possible value internally.
			// https://developer.mozilla.org/en-US/docs/Web/API/Element/scrollLeft
			window.L.DomUtil.get('spreadsheet-tab-scroll').scrollLeft = 100000;
		};
		this.actionsMap['columnrowhighlight'] = function () {
			var newState = !app.map.uiManager.getHighlightMode();
			app.map.uiManager.setHighlightMode(newState);

			if (newState) FocusCellSection.showFocusCellSection();
			else FocusCellSection.hideFocusCellSection();

			app.sectionContainer.requestReDraw();
		};

		this.actionsMap['defaultborderstyle'] = () => {
			app.map.sendUnoCommand(
				window.getBorderStyleUNOCommand(0, 0, 1, 0, 0, 0, 0),
			);
		};
	}

	private addImpressAndDrawCommands() {
		this.actionsMap['presentation'] = this.actionsMap[
			'fullscreen-presentation'
		] = () => {
			if ((window as any).canvasSlideshowEnabled)
				app.map.fire('newfullscreen', {
					isWelcomePresentation:
						window.coolParams.get('welcome') === 'true' ? true : false,
				});
			else app.map.fire('fullscreen');
		};

		this.actionsMap['presentation-currentslide'] = this.actionsMap[
			'presentation-currentslide'
		] = () => {
			if ((window as any).canvasSlideshowEnabled)
				app.map.fire('newfullscreen', {
					startSlideNumber: app.map.getCurrentPartNumber(),
				});
			else
				app.map.fire('fullscreen', {
					startSlideNumber: app.map.getCurrentPartNumber(),
				});
		};

		this.actionsMap['presentinwindow'] = this.actionsMap['present-in-window'] =
			() => {
				const welcomePresentation =
					window.coolParams.get('welcome') === 'true' ? true : false;
				if ((window as any).canvasSlideshowEnabled)
					app.map.fire('newpresentinwindow', {
						isWelcomePresentation: welcomePresentation,
					});
				else
					app.map.fire('presentinwindow', {
						isWelcomePresentation: welcomePresentation,
					});
			};

		this.actionsMap['followmepresentation'] = this.actionsMap[
			'presentation-follow-me'
		] = () => {
			app.map.slideShowPresenter.setLeader(true);
			app.map.fire('newpresentinwindow');
		};

		this.actionsMap['followpresentation'] = this.actionsMap[
			'presentation-follow'
		] = () => {
			app.map.slideShowPresenter.setLeader(false);
			app.map.slideShowPresenter.setFollower(true);
			app.map.slideShowPresenter.setFollowing(true);
			app.map.fire('newfollowmepresentation');
		};

		this.actionsMap['presenterconsole'] = () => {
			if ((window as any).canvasSlideshowEnabled)
				app.map.fire('newpresentinconsole');
		};

		this.actionsMap['fullscreen-drawing'] = () => {
			app.util.toggleFullScreen();
		};

		this.actionsMap['deletepage'] = function () {
			let msg: string;
			if (app.map.getDocType() === 'presentation') {
				msg = _('Are you sure you want to delete this slide?');
			} else {
				/* drawing */
				msg = _('Are you sure you want to delete this page?');
			}
			app.map.uiManager.showInfoModal(
				'deleteslide-modal',
				_('Delete'),
				msg,
				'',
				_('OK'),
				function () {
					app.map.deletePage();
				},
				true,
				'deleteslide-modal-response',
			);
		};

		this.actionsMap['previouspart'] = function () {
			app.map._docLayer._preview._scrollViewByDirection('prev');
		};

		this.actionsMap['nextpart'] = function () {
			app.map._docLayer._preview._scrollViewByDirection('next');
		};

		this.actionsMap['lastpart'] = function () {
			if (app && app.file.fileBasedView === true) {
				const partToSelect = app.map._docLayer._parts - 1;
				app.map._docLayer._preview._scrollViewToPartPosition(partToSelect);
			}
		};

		this.actionsMap['firstpart'] = function () {
			if (app && app.file.fileBasedView === true) {
				const partToSelect = 0;
				app.map._docLayer._preview._scrollViewToPartPosition(partToSelect);
			}
		};

		this.actionsMap['hideslide'] = function () {
			app.map.hideSlide();
		};
		this.actionsMap['showslide'] = function () {
			app.map.showSlide();
		};
		this.actionsMap['duplicatepage'] = function () {
			app.map.duplicatePage();
		};
		this.actionsMap['insertpage'] = function () {
			app.map.insertPage();
		};

		this.actionsMap['leftpara'] = function () {
			app.map.sendUnoCommand(
				(window as any).getUNOCommand({
					textCommand: '.uno:LeftPara',
					objectCommand: '.uno:ObjectAlignLeft',
					unosheet: '.uno:AlignLeft',
				}),
			);
		};
		this.actionsMap['centerpara'] = function () {
			app.map.sendUnoCommand(
				(window as any).getUNOCommand({
					textCommand: '.uno:CenterPara',
					objectCommand: '.uno:AlignCenter',
					unosheet: '.uno:AlignHorizontalCenter',
				}),
			);
		};
		this.actionsMap['rightpara'] = function () {
			app.map.sendUnoCommand(
				(window as any).getUNOCommand({
					textCommand: '.uno:RightPara',
					objectCommand: '.uno:ObjectAlignRight',
					unosheet: '.uno:AlignRight',
				}),
			);
		};

		this.actionsMap['selectbackground'] = function () {
			window.L.DomUtil.get('selectbackground').click();
		};

		this.actionsMap['notesmode'] = function () {
			if (app.impress.notesMode)
				app.map.sendUnoCommand('.uno:NormalMultiPaneGUI');
			else app.map.sendUnoCommand('.uno:NotesMode');
		};

		this.actionsMap['animationdeck'] = () => {
			app.map.sidebarFromNotebookbar.openAnimationsSidebar();
		};

		this.actionsMap['transitiondeck'] = () => {
			app.map.sidebarFromNotebookbar.openTransitionsSidebar();
		};
	}

	private addZoteroCommands() {
		this.actionsMap['zoteroaddeditcitation'] = function () {
			app.map.zotero.handleItemList();
		};
		this.actionsMap['zoterosetdocprefs'] = function () {
			app.map.zotero.handleStyleList();
		};
		this.actionsMap['zoteroaddeditbibliography'] = function () {
			app.map.zotero.insertBibliography();
		};
		this.actionsMap['zoteroaddnote'] = function () {
			app.map.zotero.handleInsertNote();
		};
		this.actionsMap['zoterorefresh'] = function () {
			app.map.zotero.refreshCitationsAndBib();
		};
		this.actionsMap['zoterounlink'] = function () {
			app.map.zotero.unlinkCitations();
		};
	}

	private addWriterCommands() {
		this.actionsMap['.uno:ShowResolvedAnnotations'] = function () {
			const items = app.map['stateChangeHandler'];
			let val = items.getItemValue('.uno:ShowResolvedAnnotations');
			val = val === 'true' || val === true;
			app.map.showResolvedComments(!val);
		};

		this.actionsMap['showannotations'] = function () {
			const items = app.map['stateChangeHandler'];
			let val = items.getItemValue('showannotations');
			val = val === 'true' || val === true;
			app.map.showComments(!val);
		};

		this.actionsMap['.uno:AcceptAllTrackedChanges'] = function () {
			app.map.sendUnoCommand('.uno:AcceptAllTrackedChanges');
			app.socket.sendMessage('commandvalues command=.uno:ViewAnnotations');
		};

		this.actionsMap['.uno:RejectAllTrackedChanges'] = function () {
			app.map.sendUnoCommand('.uno:RejectAllTrackedChanges');
			const commentSection = app.sectionContainer.getSectionWithName(
				app.CSections.CommentList.name,
			);
			commentSection.rejectAllTrackedCommentChanges();
		};

		this.actionsMap['toggletracking'] = () => {
			const TrackChangesCurrentState =
				app.map['stateChangeHandler'].getItemValue('.uno:TrackChanges');
			if (
				TrackChangesCurrentState === 'true' ||
				TrackChangesCurrentState === true
			)
				app.map.sendUnoCommand('.uno:TrackChanges?TrackChanges:bool=false');
			else app.map.sendUnoCommand('.uno:TrackChangesInAllViews');
		};

		this.actionsMap['acceptTrackedChangeToNext'] = function () {
			app.map.sendUnoCommand('.uno:AcceptTrackedChangeToNext');
		};

		this.actionsMap['rejectTrackedChangeToNext'] = function () {
			app.map.sendUnoCommand('.uno:RejectTrackedChangeToNext');
		};

		this.actionsMap['multipageview'] = function () {
			if (app.activeDocument && app.activeDocument.activeLayout) {
				let commandState = false;
				if (app.activeDocument.activeLayout.type === 'ViewLayoutMultiPage') {
					app.activeDocument.activeLayout = new ViewLayoutWriter();
					app.activeDocument.activeLayout.adjustViewZoomLevel();
				} else {
					app.activeDocument.activeLayout = new ViewLayoutMultiPage();
					commandState = true;
				}

				app.map.fire('commandstatechanged', {
					commandName: 'multipageview',
					state: commandState ? 'true' : 'false',
				});
				app.activeDocument.activeLayout.sendClientVisibleArea();
				app.sectionContainer.requestReDraw();
			}
		};

		this.actionsMap['comparechanges'] = function () {
			if (app.activeDocument && app.activeDocument.activeLayout) {
				Util.ensureValue(app.activeDocument);
				app.socket.sendMessage('uno .uno:RedlineRenderMode');

				const commandState =
					app.activeDocument.activeLayout.type === 'ViewLayoutCompareChanges';

				app.map.fire('commandstatechanged', {
					commandName: 'comparechanges',
					state: !commandState ? 'true' : 'false',
				});

				app.activeDocument.activeLayout = commandState
					? new ViewLayoutWriter()
					: new ViewLayoutCompareChanges();

				// Do this only if we are switching to Writer normal layout.
				// Try to handle this in constructor for compare-changes layout.
				if (commandState) {
					TileManager.redraw();
					app.map._docLayer._fitWidthZoom(null, null, true);
					app.activeDocument.activeLayout.sendClientVisibleArea();
					app.sectionContainer.requestReDraw();
				}
			}
		};

		// View Changes menu: radio-style actions (Inline / Side by Side / Hidden).
		// Each action activates its mode and deactivates the others.

		const updateViewChangesState = function (mode: string) {
			const states: Record<string, boolean> = {
				'viewchanges-inline': mode === 'inline',
				'viewchanges-sidebyside': mode === 'sidebyside',
				'viewchanges-hidden': mode === 'hidden',
				viewchanges: mode !== 'hidden',
			};

			for (const key in states) {
				const val = states[key] ? 'true' : 'false';
				app.map['stateChangeHandler'].setItemValue(key, val);
				app.map.fire('commandstatechanged', {
					commandName: key,
					state: val,
				});
			}
		};

		const switchToWriterLayout = function () {
			if (
				app.activeDocument?.activeLayout?.type === 'ViewLayoutCompareChanges'
			) {
				app.activeDocument.activeLayout = new ViewLayoutWriter();
				TileManager.redraw();
				app.map._docLayer._fitWidthZoom(null, null, true);
				app.activeDocument.activeLayout.sendClientVisibleArea();
				app.sectionContainer.requestReDraw();
			}
		};

		this.actionsMap['viewchanges-inline'] = function () {
			if (!app.activeDocument?.activeLayout) return;

			switchToWriterLayout();

			// Ensure inline tracked changes are visible.
			const showState = app.map['stateChangeHandler'].getItemValue(
				'.uno:ShowTrackedChanges',
			);
			if (showState !== 'true')
				app.map.sendUnoCommand('.uno:ShowTrackedChanges');

			updateViewChangesState('inline');
		};

		this.actionsMap['viewchanges-sidebyside'] = function () {
			if (!app.activeDocument?.activeLayout) return;

			Util.ensureValue(app.activeDocument);
			app.socket.sendMessage('uno .uno:RedlineRenderMode');

			if (app.activeDocument.activeLayout.type !== 'ViewLayoutCompareChanges')
				app.activeDocument.activeLayout = new ViewLayoutCompareChanges();

			updateViewChangesState('sidebyside');
		};

		this.actionsMap['viewchanges-hidden'] = function () {
			if (!app.activeDocument?.activeLayout) return;

			switchToWriterLayout();

			// Ensure inline tracked changes are hidden.
			const showState = app.map['stateChangeHandler'].getItemValue(
				'.uno:ShowTrackedChanges',
			);
			if (showState === 'true')
				app.map.sendUnoCommand('.uno:ShowTrackedChanges');

			updateViewChangesState('hidden');
		};
	}

	private addMobileCommands() {
		this.actionsMap['comment_wizard'] = function () {
			const configuration = window as any;
			if (configuration.commentWizard) {
				configuration.commentWizard = false;
				app.sectionContainer
					.getSectionWithName(app.CSections.CommentList.name)
					.removeHighlighters();
				app.map.fire('closemobilewizard');
				app.map.mobileTopBar.selectItem('comment_wizard', false);
			} else {
				if (configuration.insertionMobileWizard)
					app.dispatcher.dispatch('insertion_mobile_wizard');
				else if (configuration.mobileWizard)
					app.dispatcher.dispatch('mobile_wizard');
				configuration.commentWizard = true;
				var menuData = app.map._docLayer.getCommentWizardStructure();
				app.map.fire('mobilewizard', { data: menuData });
				app.map.mobileTopBar.selectItem('comment_wizard', true);
			}
		};
		this.actionsMap['mobile_wizard'] = () => {
			const configuration = window as any;
			if (configuration.mobileWizard) {
				configuration.mobileWizard = false;
				app.map.sendUnoCommand('.uno:SidebarHide');
				app.map.fire('closemobilewizard');
				app.map.mobileTopBar.selectItem('mobile_wizard', false);
			} else {
				if (configuration.insertionMobileWizard)
					app.dispatcher.dispatch('insertion_mobile_wizard');
				else if (configuration.commentWizard)
					app.dispatcher.dispatch('comment_wizard');
				configuration.mobileWizard = true;
				app.map.sendUnoCommand('.uno:SidebarShow');
				app.map.fire('showwizardsidebar');
				app.map.mobileTopBar.selectItem('mobile_wizard', true);
			}
		};
		this.actionsMap['insertion_mobile_wizard'] = () => {
			const configuration = window as any;
			if (configuration.insertionMobileWizard) {
				configuration.insertionMobileWizard = false;
				app.map.fire('closemobilewizard');
				app.map.mobileTopBar.selectItem('insertion_mobile_wizard', false);
			} else {
				if (configuration.mobileWizard)
					app.dispatcher.dispatch('mobile_wizard');
				else if (configuration.commentWizard)
					app.dispatcher.dispatch('comment_wizard');
				configuration.insertionMobileWizard = true;
				const menuData = app.map.menubar.generateInsertMenuStructure();
				app.map.fire('mobilewizard', { data: menuData });
				app.map.mobileTopBar.selectItem('insertion_mobile_wizard', true);
			}
		};

		this.actionsMap['fontcolor'] = () => {
			app.map.fire('mobilewizard', {
				data: (window as any).getColorPickerData('Font Color'),
			});
		};
		this.actionsMap['backcolor'] = () => {
			app.map.fire('mobilewizard', {
				data: (window as any).getColorPickerData('Highlight Color'),
			});
		};
		// TODO: leftover from mobile bottom bar
		// if (id === 'fontcolor' && typeof e.color !== 'undefined') {
		// 	onColorPick(id, e.color, e.themeData);
		// }
		// else if (id === 'backcolor' && typeof e.color !== 'undefined') {
		// 	onColorPick(id, e.color, e.themeData);
		// }
		// else if (id === 'backgroundcolor' && typeof e.color !== 'undefined') {
		// 	onColorPick(id, e.color, e.themeData);
		// }
	}

	/// optional docType specifies which commands should we load
	constructor(docType: string = undefined) {
		docType = docType ? docType : app.map._docLayer._docType;

		this.addGeneralCommands();
		this.addExportCommands();
		this.addAICommands();

		if (docType === 'text') {
			this.addWriterCommands();
			this.addZoteroCommands();
		} else if (docType === 'spreadsheet') {
			this.addCalcCommands();
		} else if (['presentation', 'drawing'].includes(docType)) {
			this.addImpressAndDrawCommands();
		}

		if (window.mode.isSmallScreenDevice()) this.addMobileCommands();
	}

	public dispatch(action: string, data?: any) {
		// Don't allow to execute new actions while any dialog is visible.
		// It prevents launching multiple instances of the same dialog.
		// Exception: validatedialogsa11y needs to run when dialogs are open.
		if (
			action !== 'validatedialogsa11y' &&
			(app.map.dialog.hasOpenedDialog() ||
				(app.map.jsdialog && app.map.jsdialog.hasDialogOpened()))
		) {
			app.map.dialog.blinkOpenDialog();
			console.debug('Cannot dispatch: ' + action + ' when dialog is opened.');
			return;
		}

		if (action.indexOf('saveas-') === 0) {
			const format = action.substring('saveas-'.length);
			app.map.openSaveAs(format);
			return;
		} else if (action.indexOf('downloadas-') === 0) {
			const format = action.substring('downloadas-'.length);
			let fileName = app.map['wopi'].BaseFileName;
			fileName = fileName.substr(0, fileName.lastIndexOf('.'));
			fileName = fileName === '' ? 'document' : fileName;
			app.map.downloadAs(fileName + '.' + format, format);
			return;
		}

		if (action.indexOf('exportas-') === 0) {
			const format = action.substring('exportas-'.length);
			app.map.openSaveAs(format);
			return;
		}

		if (
			action === '.uno:Copy' ||
			action === '.uno:Cut' ||
			action === '.uno:Paste' ||
			action === '.uno:PasteSpecial'
		) {
			app.map._clip.filterExecCopyPaste(action);
			return;
		}

		if (this.actionsMap[action] !== undefined) {
			this.actionsMap[action](data);
			return;
		}

		if (window.ThisIsTheWindowsApp && action.startsWith('new-')) {
			window.postMobileMessage(action);
			return;
		}

		console.error('unknown dispatch: "' + action + '"');
	}
}

app.definitions['dispatcher'] = Dispatcher;
