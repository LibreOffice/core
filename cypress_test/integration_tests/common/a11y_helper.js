/* -*- js-indent-level: 8 -*- */
/* global cy Cypress expect require */

var ceHelper = require('./contenteditable_helper');
var desktopHelper = require('./desktop_helper');
var helper = require('./helper');

/**
 * Enable UICoverage tracking. Call this in the `before` hook after getting the frame window.
 * @param {Object} win - The frame window object
 */
function enableUICoverage(win) {
	const enableUICoverage = {
		'Track': { 'type': 'boolean', 'value': true }
	};
	win.app.map.sendUnoCommand('.uno:UICoverage', enableUICoverage);
}

/**
 * Report UICoverage and verify results. Call this in the `after` hook.
 * Returns the result object via a Cypress alias '@uicoverageResult' for additional assertions.
 * @param {Object} win - The frame window object
 * @param {boolean} hasLinguisticData - Whether linguistic data (thesaurus, etc.) is available
 */
function reportUICoverage(win, hasLinguisticData = true) {
	if (win.app.socket._onMessage.restore) {
		// if _onMessage is already wrapped by Sinon, do not create a new spy
		cy.wrap(win.app.socket._onMessage).as('onMessage');
	} else {
		cy.spy(win.app.socket, '_onMessage').as('onMessage').log(false);
	}

	cy.then(() => {
		const endUICoverage = {
			'LinguisticDataAvailable': { 'type': 'boolean', 'value': hasLinguisticData },
			'Report': { 'type': 'boolean', 'value': true },
			'Track': { 'type': 'boolean', 'value': false }
		};
		win.app.map.sendUnoCommand('.uno:UICoverage', endUICoverage);
	});

	var coverageResult = null;

	function findUICoverageCall(onMessage) {
		return onMessage.getCalls().find(call => {
			const evt = call.args && call.args[0]
			const textMsg = evt && evt.textMsg;
			if (!textMsg || !textMsg.startsWith('unocommandresult:')) {
				return false;
			}
			const jsonPart = textMsg.replace('unocommandresult:', '').trim();
			const data = JSON.parse(jsonPart);
			return data.commandName === '.uno:UICoverage';
		});
	}

	// Use should() for retry until the message arrives
	cy.get('@onMessage').should(onMessage => {
		const matchingCall = findUICoverageCall(onMessage);
		expect(matchingCall, '.uno:UICoverage result').to.be.an('object');

		const textMsg = matchingCall.args[0].textMsg;
		const jsonPart = textMsg.replace('unocommandresult:', '').trim();
		coverageResult = JSON.parse(jsonPart).result;

		Cypress.log({name: 'UICoverage Message: ', message: JSON.stringify(coverageResult)});
	}).then(() => {
		cy.wrap(coverageResult).as('uicoverageResult');
	});
}

/**
 * Reset document state after each test.
 */
function resetState() {
	cy.cGet('body').then($body => {
		const snackbarDismiss = $body.find('#snackbar-dismiss-button-button');
		if (snackbarDismiss.length > 0) {
			cy.wrap(snackbarDismiss).click({ force: true });
		}
	});
	desktopHelper.undoAll();
	cy.cGet('div.clipboard').as('clipboard');
	ceHelper.moveCaret('home', 'ctrl');
}

/**
 * Check for a11y errors in spy calls and throw if any found.
 * @param {Object} win - The frame window object
 * @param {Object} spy - Sinon spy on console.error
 */
function checkA11yErrors(win, spy) {
	cy.then(() => {
		const a11yValidatorExceptionText = win.app.A11yValidatorException.PREFIX;
		const a11yErrors = spy.getCalls().filter(call =>
			String(call.args[0]).includes(a11yValidatorExceptionText)
		);

		if (a11yErrors.length > 0) {
			const errorMessages = a11yErrors.map(call =>
				call.args.map(arg => String(arg)).join(' ')
			).join('\n\n');

			throw new Error(`Found A11y errors:\n${errorMessages}`);
		}
	});
}

/**
 * Run a11y validation via dispatcher command and check for errors.
 * @param {Object} win - The frame window object
 * @param {string} dispatchCommand - The dispatch command (e.g., 'validatedialogsa11y')
 */
function runA11yValidation(win, dispatchCommand) {
	cy.then(() => {
		var spy = Cypress.sinon.spy(win.console, 'error');
		win.app.dispatcher.dispatch(dispatchCommand);

		checkA11yErrors(win, spy);

		if (spy && spy.restore) {
			spy.restore();
		}
	});
}

/**
 * Get the active dialog at a specific nesting level.
 * @param {number} level - The dialog nesting level (1 for top-level dialog)
 * @returns {Cypress.Chainable} - Chainable that yields the dialog element
 */
function getActiveDialog(level) {
	return cy.cGet('.ui-dialog[role="dialog"]:not(.snackbar)')
		.should('have.length', level)
		.then($dialogs => cy.wrap($dialogs.last()));
}

/**
 * Close the active dialog at a specific nesting level.
 * @param {number} level - The dialog nesting level
 */
function closeActiveDialog(level) {
	getActiveDialog(level)
		.parents('.jsdialog-window')
		.invoke('attr', 'id')
		.then(dialogId => {
			const windowSelector = `#${CSS.escape(dialogId)}`;
			// A jsdialog update can rebuild the dialog contents at any moment,
			// which detaches a close button found earlier. Find the button and
			// click it in one step, and repeat until the dialog is gone.
			helper.retryUntil(
				function () {
					cy.cGet('body').then($body => {
						const $button = $body.find(windowSelector + ' .ui-dialog-titlebar-close');
						if ($button.length) $button[0].click();
					});
				},
				function () {
					return cy.cGet('body').then($body =>
						$body.find(windowSelector).length === 0);
				},
				{ errorMsg: 'dialog ' + dialogId + ' never closed' });
		});

	cy.cGet('.ui-dialog[role="dialog"]:not(.snackbar)').should('have.length', level - 1);
}

/**
 * Close the active warning dialog at a specific nesting level.
 * @param {number} level - The dialog nesting level
 * @param {string} buttonSelector - The button to close the dialog with.
 */
function closeActiveWarningDialog(level, buttonSelector = '#no-button') {
	getActiveDialog(level)
		.parents('.jsdialog-window')
		.invoke('attr', 'id')
		.then(dialogId => {
			cy.cGet(`#${CSS.escape(dialogId)} ` + buttonSelector)
				.click();
		});

	cy.cGet('.ui-dialog[role="dialog"]:not(.snackbar)').should('have.length', level - 1);
}

/**
 * Get the active tab panel for a given tab.
 * @param {jQuery} $container - The container element
 * @param {string} activeTabId - The ID of the active tab
 * @returns {jQuery|null} - The tab panel element or null
 */
function getActiveTabPanel($container, activeTabId) {
	const tabSelector = `#${CSS.escape(activeTabId)}`;
	const $activeTab = $container.find(tabSelector);

	if (!$activeTab.length) return null;

	const panelId = $activeTab.attr('aria-controls');
	if (!panelId) return null;

	const panelSelector = `#${CSS.escape(panelId)}[role="tabpanel"]`;
	return $container.find(panelSelector);
}

/**
 * Test the name dialog (used for adding hatching patterns, etc.) - Writer only.
 * @param {Object} win - The frame window object
 * @param {number} level - The current dialog nesting level
 */
function testNameDialog(win, level) {
	/* exercise the name dialog */
	getActiveDialog(level + 1)
		.then(() => {
			return helper.processToIdle(win);
		})
		.then(() => {
			runA11yValidation(win, 'validatedialogsa11y');
		})
		.then(() => {
			// save with default suggested name
			cy.cGet('[role="dialog"][aria-labelledby*="Name"] #ok-button').should('be.enabled').click();
			return helper.processToIdle(win);
		})
		.then(() => {
			cy.cGet('.ui-dialog[role="dialog"]:not(.snackbar)').should('have.length', level);
		});
	/* Then add the same name again so we get the warning subdialog */
	cy.cGet('button.ui-pushbutton[aria-label="Add"]:visible').should('not.be.disabled').click();
	getActiveDialog(level + 1)
		.then(() => {
			return helper.processToIdle(win);
		})
		.then(() => {
			// save with a name that exists to force the warning subdialog
			cy.cGet('#name_entry-input').type('{selectall}{backspace}Hatching 1');
			cy.cGet('[role="dialog"][aria-labelledby*="Name"] #ok-button').should('be.enabled').click();
			return helper.processToIdle(win);
		})
		.then(() => {
			// warning subdialog, default close will cancel
			handleDialog(win, level + 1);
		});
}

/**
 * Traverse tabs in a dialog and run a11y validation on each.
 * @param {Function} getContainer - Function that returns the container element
 * @param {Object} win - The frame window object
 * @param {number} level - The current dialog nesting level
 * @param {string} command - The uno command that opened the dialog
 * @param {boolean} isNested - Whether this is a nested tab traversal
 */
function traverseTabs(getContainer, win, level, command, isNested = false) {
	const TABLIST = '[role="tablist"]';
	const TAB = '[role="tab"]';

	return getContainer().then($container => {
		let $tabLists;

		if (!isNested) {
			// For top-level tabs, select only direct tab lists under #tabcontrol
			// to avoid picking up tab lists from nested tab-panels
			$tabLists = $container.find('#tabcontrol > ' + TABLIST);
		} else {
			// For nested tabs, the container is already the relevant tab-panel,
			// so select all tab lists within it
			$tabLists = $container.find(TABLIST);
		}

		if (!$tabLists.length) return;

		return Cypress._.reduce($tabLists, (chain, tabListEl, tabListIndex) => {
			return chain.then(() => {
				const $tabs = Cypress.$(tabListEl).find(TAB);

				const clickTabByIndex = (index) => {
					if (index >= $tabs.length) return cy.wrap(null);

					const $tab = $tabs.eq(index);
					const tabId = $tab.attr('id');
					const tabAriaControls = $tab.attr('aria-controls');

					return getContainer()
						.find(TABLIST).eq(tabListIndex)
						.find(TAB).eq(index)
						.click({ force: true })
						.then(() => {
							return helper.processToIdle(win);
						})
						.then(() => {
							runA11yValidation(win, 'validatedialogsa11y');
						})
						.then(() => {
							// Writer-specific tab subdialogs
							if (command == '.uno:SetDocumentProperties' && tabId == 'customprops') {
								cy.cGet('#durationbutton-button').click();
								handleDialog(win, level + 1);
							} else if (command == '.uno:SetDocumentProperties' && tabId == 'general') {
								cy.cGet('#changepass-button').should('not.be.disabled').click();
								handleDialog(win, level + 1);
							} else if (command == '.uno:InsertSection' && tabId == 'section') {
								// check protect to enable password dialog
								cy.cGet('#protect-input').check();
								cy.cGet('#selectpassword-button').should('not.be.disabled').click();
								handleDialog(win, level + 1);
								cy.cGet('#protect-input').uncheck();
								cy.cGet('#selectpassword-button').should('be.disabled');
							} else if (command == '.uno:HyperlinkDialog' && tabId == 'document') {
								cy.cGet('#browse-button').click();
								handleDialog(win, level + 1);
							} else if (command == '.uno:FontDialog' && tabId == 'font') {
								cy.cGet('#btnWestFeatures-button').click();
								handleDialog(win, level + 1);
							} else if ((command == '.uno:PageDialog' || command == '.uno:PageFormatDialog') && (tabId == 'header' || tabId == 'footer')) {
								// enable the header/footer to make the More... button sensitive
								const toggleId = tabId == 'header' ? '#checkHeaderOn-input' : '#checkFooterOn-input';
								cy.cGet(toggleId).check({ force: true });
								cy.cGet('[id^="buttonMore"][id$="-button"]').filter(':visible').first().should('be.enabled').click();
								handleDialog(win, level + 1);
							} else if ((command == '.uno:FormatArea' || command == '.uno:PageDialog' || command == '.uno:PageFormatDialog') && tabAriaControls == 'lbhatch') {
								cy.cGet('button.ui-pushbutton[aria-label="Add"]:visible').click();
								testNameDialog(win, level);
							}
						})
						.then(() => {
							return getContainer();
						})
						.then($ctx => {
							const $panel = getActiveTabPanel($ctx, tabId);

							if (!$panel || !$panel.length) return;

							const panelId = $panel.attr('id');
							const panelSelector = `#${CSS.escape(panelId)}`;

							return getContainer()
								.then(() => {
									const $nestedTablists = $panel.find(TABLIST);

									if (!isNested && $nestedTablists.length > 0) {
										return traverseTabs(
											() => getContainer().find(panelSelector),
											win, level, command, true
										);
									}
								});
						})
						.then(() => {
							return clickTabByIndex(index + 1);
						});
				};

				return clickTabByIndex(0);
			});
		}, cy.wrap(null));
	});
}

/**
 * Handle tabs in a dialog.
 * @param {Object} win - The frame window object
 * @param {number} level - The current dialog nesting level
 * @param {string} command - The uno command that opened the dialog
 */
function handleTabsInDialog(win, level, command) {
	traverseTabs(() => getActiveDialog(level), win, level, command);
}

/**
 * Handle a dialog - wait for it, validate a11y, handle subdialogs, traverse tabs, and close.
 * @param {Object} win - The frame window object
 * @param {number} level - The dialog nesting level
 * @param {string} command - The uno command that opened the dialog (optional)
 * @param {boolean} isWarningDialog - If this is a warning dialog
 * @param {string} warningButtonSelector - The button a warning dialog is closed with (optional)
 */
function handleDialog(win, level, command, isWarningDialog, warningButtonSelector) {
	getActiveDialog(level)
		.then(() => {
			return helper.processToIdle(win);
		})
		.then(() => {
			runA11yValidation(win, 'validatedialogsa11y');
		})
		.then(() => {
			// Writer-specific subdialogs
			if (command == '.uno:EditRegion' ||
			    command == '.uno:InsertCaptionDialog' ||
			    command == '.uno:SpellDialog' ||
			    command == '.uno:SpellingAndGrammarDialog' ||
			    command == '.uno:DataDataPilotRun:Field') {
				getActiveDialog(level).find('#options-button').click();
				handleDialog(win, level + 1);
			} else if (command == '.uno:InsertIndexesEntry') {
				cy.cGet('#new-button').click();
				handleDialog(win, level + 1);
			} else if (command == '.uno:ContentControlProperties') {
				cy.cGet('#add-button').click();
				handleDialog(win, level + 1);
			} else if (command == '.uno:ThemeDialog') {
				cy.cGet('#button_add-button').click();
				handleDialog(win, level + 1);
			} else if (command == '.uno:SearchDialog') {
				cy.cGet('.ui-expander-label').contains('Other options').should('be.visible').click();
				cy.cGet('#similarity-input').check();
				cy.cGet('#similaritybtn-button').should('be.enabled').click();
				handleDialog(win, level + 1);
				cy.cGet('#similarity-input').uncheck();
				cy.cGet('#soundslike-input').check();
				cy.cGet('#soundslikebtn-button').should('be.enabled').click();
				handleDialog(win, level + 1);
				cy.cGet('#soundslike-input').uncheck();
				// Format and Attributes search are writer-only and the
				// buttons are hidden in calc/draw.
				cy.cGet('body').then($body => {
					if ($body.find('#attributes-button:visible').length) {
						cy.cGet('#attributes-button').should('be.enabled').click();
						handleDialog(win, level + 1);
					}
				});
				cy.cGet('body').then($body => {
					if ($body.find('#format-button:visible').length) {
						cy.cGet('#format-button').should('be.enabled').click();
						handleDialog(win, level + 1);
					}
				});
			} else if (command == '.uno:Signature') {
				cy.cGet('#signatures .ui-treeview-entry > div:first-child').click();
				cy.cGet('#view-button').should('be.enabled').click();
				handleDialog(win, level + 1);
				cy.cGet('#sign-button').should('be.enabled').click();
				handleDialog(win, level + 1);
			} else if (command == '.uno:DataBarFormatDialog') {
				cy.cGet('#options-button').should('be.visible').click();
				handleDialog(win, level + 1);
			} else if (command == '.uno:DataDataPilotRun') {
				cy.cGet('#listbox-page .ui-treeview-entry > div:first-child').dblclick();
				handleDialog(win, level + 1, '.uno:DataDataPilotRun:Field');

				cy.cGet('#listbox-data .ui-treeview-entry > div:first-child').dblclick();
				handleDialog(win, level + 1, '.uno:DataDataPilotRun:Data');
			} else if (command == '.uno:InsertObjectChart') {
				cy.cGet('#next').click();
				helper.processToIdle(win);
				cy.cGet('#IB_RANGE-button').click();
				// At some point this might begin to behave as if the current dialog turned
				// into a cell selector, in which case the dialog will be the same level
				// not a level higher I imagine.
				handleDialog(win, level + 1);
			}

			handleTabsInDialog(win, level, command);
			if (isWarningDialog) {
				closeActiveWarningDialog(level, warningButtonSelector);
			} else {
				closeActiveDialog(level);
			}
		});
}

/**
 * Test a dialog by sending its uno command and handling it.
 * @param {Object} win - The frame window object
 * @param {string|Object} commandSpec - The uno command or {command, args} object
 */
function testDialog(win, commandSpec) {
	const command = typeof commandSpec === 'string' ? commandSpec : commandSpec.command;
	const args = typeof commandSpec === 'string' ? undefined : commandSpec.args;

	cy.then(() => {
		win.app.map.sendUnoCommand(command, args);
	});

	handleDialog(win, 1, command);
}

const allCommonDialogs = [
	// .uno:Signature must run first: it shows a "save before sign" prompt
	// when the document is modified, which aborts the dialog flow. Running
	// before any other dialog dirties the doc keeps that prompt out of the way.
	'.uno:Signature',
	'.uno:AcceptTrackedChanges',
	{ command: '.uno:ExportToPDF', args: { SynchronMode: { type: 'boolean', value: false } } },
	'.uno:FontworkGalleryFloater',
	'.uno:GotoPage',
	'.uno:HyperlinkDialog',
	'.uno:InsertQrCode',
	'.uno:InsertSymbol',
	'.uno:RunMacro',
	'.uno:SearchDialog',
	// modules/swriter/ui/seclabeldialog.ui. The command is writer-only, so
	// the calc and impress specs exclude it. With no SPIF policy provisioned
	// (the a11y harness has none) the dialog opens with its no-policy notice
	// and disabled inputs, which still exercises the .ui for coverage + a11y.
	'.uno:SecurityLabel',
	'.uno:SetDocumentProperties',
	'.uno:SpellDialog',
	'.uno:SpellingAndGrammarDialog',
	'.uno:SplitCell',
	'.uno:StyleNewByExample',
	'.uno:ThemeDialog',
	'.uno:ThesaurusDialog',
	'.uno:WidgetTestDialog'
];

const needLinguisticDataDialogs = [
	'.uno:SpellDialog',
	'.uno:SpellingAndGrammarDialog',
	'.uno:ThesaurusDialog',
];

/**
 * Generate test cases for all common dialogs.
 * @param {Object} options - Configuration options
 * @param {Function} options.it - The Mocha `it` function from the test file
 * @param {Function} options.getWin - Function that returns the frame window object
 * @param {Function} options.getHasLinguisticData - Function that returns whether linguistic data is available
 */
/**
 * Check if a dialog command requires linguistic data.
 * @param {string} command - The uno command
 * @returns {boolean} - Whether the dialog requires linguistic data
 */
function needsLinguisticData(command) {
	return needLinguisticDataDialogs.includes(command);
}

/**
 * Test the PDF export warning dialog by exporting with conflicting options.
 * @param {Object} win - The frame window object
 */
function testPDFExportWarningDialog(win) {
	cy.then(() => {
		const args = { SynchronMode: { type: 'boolean', value: false } };
		win.app.map.sendUnoCommand('.uno:ExportToPDF', args);
	});

	// The warning dialog opens at the same level (1) as the export options
	// dialog it replaces, so to tell them apart capture the export
	// dialog's window id and wait for that specific dialog to close after
	// OK before handling the warning.
	var exportDialogId;
	getActiveDialog(1)
		.then(($dialog) => {
			exportDialogId = $dialog.parents('.jsdialog-window').attr('id');
			return helper.processToIdle(win);
		})
		.then(() => {
			cy.cGet('#forms-input').check();
			cy.cGet('#pdf_version-input').select('PDF/A-1b (PDF 1.4 base)');
			cy.cGet('#ok-button').click();
			// pdf export dialog should dismiss
			cy.cGet('#' + CSS.escape(exportDialogId)).should('not.exist');
			// and the warning dialog we're interested in should appear
			handleDialog(win, 1);
		});
}

/**
 * Assert that keyboard focus is somewhere inside the element matched by
 * selector. Retries until it holds, so it is safe to call right after a
 * focus-moving key press.
 * @param {string} selector - cGet selector for the container element
 */
function assertFocusWithin(selector) {
	cy.cGet(selector).should(function ($element) {
		var win = $element[0].ownerDocument.defaultView;
		expect($element[0].contains(win.document.activeElement)).to.equal(true);
	});
}

/**
 * The tab order of the open sidebar deck: every tabbable descendant in DOM
 * order, which is the order the browser moves through with Tab as long as no
 * widget forces a position of its own with a positive tabindex.
 *
 * JSDialog.GetFocusableElements is deliberately not used to build it, so the
 * expectation stays independent of the helper under test.
 * @param {Object} win - The frame window object
 */
function sidebarTabOrder(win) {
	const wrapper = win.app.map.sidebar.wrapper;
	const candidates = wrapper.querySelectorAll(
		'a[href], button, input, select, textarea, [tabindex], [contenteditable="true"]');

	return Array.prototype.filter.call(candidates, function (element) {
		if (element.disabled) return false;
		const tabindex = element.getAttribute('tabindex');
		if (tabindex !== null && parseInt(tabindex, 10) < 0) return false;
		// visibilityProperty matters: a deck keeps the widgets of the styles
		// it does not show in place with visibility hidden, and those are not
		// tabbable even though the default checkVisibility() call says so.
		return element.checkVisibility({
			visibilityProperty: true,
			contentVisibilityAuto: true,
		});
	});
}

/**
 * Name an element for an assertion message.
 * @param {Element} element
 */
function describeFocusable(element) {
	if (!element) return 'nothing';
	return element.tagName + '#' + (element.id || '<no id>');
}

/**
 * Open the property deck if it is not up yet and let its focus grab settle,
 * so it cannot steal the focus back mid-test.
 * @param {function} getWin - Returns the frame window object
 */
function openSidebarPropertyDeck(getWin) {
	cy.cGet('#sidebar-dock-wrapper').then(function ($dock) {
		if (!$dock.is(':visible')) {
			cy.then(function () {
				getWin().app.map.sendUnoCommand('.uno:SidebarDeck.PropertyDeck');
				return helper.processToIdle(getWin());
			});
		}
	});
	cy.cGet('#sidebar-dock-wrapper').should('be.visible');

	cy.then(function () {
		helper.waitUntilLayoutingIsIdle(getWin());
		helper.waitForTimers(getWin(), 'sidebarstealfocus');
	});
}

/**
 * Bodies of the sidebar deck keyboard tests. The deck differs per module, so
 * every expectation is read from the DOM rather than naming widgets, and the
 * same checks hold for Writer, Calc, Impress and Draw. Each spec declares its
 * own it() blocks -- the tag preprocessor only rewrites spec files, so tests
 * declared from here never reach the runner.
 */
const sidebarKeyboard = {
	assertFocusedIs: function (win, getExpected, label) {
		cy.cGet('#sidebar-dock-wrapper').should(function () {
			const expected = getExpected();
			const active = win.document.activeElement;
			expect(describeFocusable(active), label)
				.to.equal(describeFocusable(expected));
			expect(active, label + ' (same element)').to.equal(expected);
		});
	},

	assertNoForcedTabPosition: function (getWin) {
		cy.then(function () {
			const order = sidebarTabOrder(getWin());
			expect(order.length, 'tabbable widgets of the deck').to.be.greaterThan(1);

			const forced = order.filter(function (element) {
				return parseInt(element.getAttribute('tabindex'), 10) > 0;
			}).map(describeFocusable);
			expect(forced, 'widgets with a positive tabindex').to.be.empty;
		});
	},

	// A deck keeps the widgets of the styles it does not show in place with
	// visibility hidden. A bare checkVisibility() does not look at the
	// visibility property, so the helper used to offer them as focusable.
	assertHelperSkipsInvisible: function (getWin) {
		cy.then(function () {
			const win = getWin();
			const focusables = win.JSDialog.GetFocusableElements(win.app.map.sidebar.wrapper);
			expect(focusables, 'focusables of the deck').to.not.be.empty;

			const invisible = focusables.filter(function (element) {
				return !element.checkVisibility({
					visibilityProperty: true,
					contentVisibilityAuto: true,
				});
			}).map(describeFocusable);
			expect(invisible, 'widgets laid out but not visible').to.be.empty;
		});
	},

	assertHelperReportsTabOrder: function (getWin) {
		cy.then(function () {
			const win = getWin();
			const reported = win.JSDialog.GetFocusableElements(win.app.map.sidebar.wrapper)
				.map(describeFocusable);
			expect(reported, 'widgets the helper reports')
				.to.deep.equal(sidebarTabOrder(win).map(describeFocusable));
		});
	},

	assertPredicateAgreesWithHelper: function (getWin) {
		cy.then(function () {
			const win = getWin();
			const wrapper = win.app.map.sidebar.wrapper;
			const hidden = Array.prototype.filter.call(
				wrapper.querySelectorAll(win.JSDialog.FocusableSelector),
				function (element) {
					return !element.checkVisibility({
						visibilityProperty: true,
						contentVisibilityAuto: true,
					});
				});

			expect(hidden.map(describeFocusable),
				'widgets of the deck hidden with visibility').to.not.be.empty;
			hidden.forEach(function (element) {
				expect(win.JSDialog.IsFocusable(element),
					describeFocusable(element) + ' is hidden').to.equal(false);
			});

			win.JSDialog.GetFocusableElements(wrapper).forEach(function (element) {
				expect(win.JSDialog.IsFocusable(element),
					describeFocusable(element) + ' is reported focusable').to.equal(true);
			});
		});
	},

	assertRingEntersOnFirstWidget: function (getWin) {
		cy.realPress('F6');
		sidebarKeyboard.assertFocusedIs(getWin(), function () {
			return sidebarTabOrder(getWin())[0];
		}, 'widget focused when the ring enters the sidebar');
	},

	assertTabWalksTheDeck: function (getWin) {
		cy.realPress('F6');

		cy.then(function () {
			const win = getWin();
			const order = sidebarTabOrder(win);
			const from = order.indexOf(win.document.activeElement);
			expect(from, 'the focused widget is part of the tab order')
				.to.be.greaterThan(-1);

			// walk to the end of the deck, then back to where we started
			for (let at = from + 1; at < order.length; at++) {
				cy.then(function () {
					cy.realPress('Tab');
					sidebarKeyboard.assertFocusedIs(win, function () {
						return sidebarTabOrder(win)[at];
					}, 'widget ' + at + ' of the deck');
				});
			}

			for (let at = order.length - 2; at >= from; at--) {
				cy.then(function () {
					cy.realPress(['Shift', 'Tab']);
					sidebarKeyboard.assertFocusedIs(win, function () {
						return sidebarTabOrder(win)[at];
					}, 'widget ' + at + ' of the deck, walking back');
				});
			}
		});
	},

	assertRingLeavesTheDeck: function (getWin) {
		cy.realPress('F6');
		cy.realPress('Tab');

		cy.cGet('#sidebar-dock-wrapper').should(function ($dock) {
			expect($dock[0].contains(getWin().document.activeElement),
				'focus is inside the deck').to.equal(true);
		});

		cy.realPress('F6');

		cy.cGet('#sidebar-dock-wrapper').should(function ($dock) {
			expect($dock[0].contains(getWin().document.activeElement),
				'focus left the deck').to.equal(false);
		});
	},
};

module.exports.assertFocusWithin = assertFocusWithin;
/**
 * The accessibility tree the browser exposes to assistive technology, read over
 * the Chrome DevTools Protocol. runA11yValidation checks the markup; this is
 * the name and role computed from it, so it catches a label that resolves to
 * nothing. Chromium-family browsers only.
 */
function cdp(command, params) {
	return Cypress.automation('remote:debugger:protocol', {
		command: command,
		params: params || {},
	});
}

function axTreeAvailable() {
	return Cypress.browser.family === 'chromium';
}

/// The document lives in its own frame, and getFullAXTree defaults to the top
/// one, which is the cypress runner.
const COOL_FRAME_ATTEMPTS = 20;

function coolFrameId(attempt) {
	const at = attempt || 1;

	return cy.then(function () {
		return cdp('Page.enable');
	}).then(function () {
		return cdp('Page.getFrameTree');
	}).then(function (tree) {
		const urls = [];
		(function walk(node) {
			urls.push(node.frame.url ? node.frame.url : '');
			if (node.frame.url && node.frame.url.indexOf('cool.html') !== -1)
				urls.cool = node.frame.id;
			(node.childFrames || []).forEach(walk);
		})(tree.frameTree);

		if (urls.cool) return urls.cool;

		// The tree can be asked before the document frame is in it. Retrying
		// keeps that from failing every accessibility assertion at once, with
		// a message that names none of it.
		if (at < COOL_FRAME_ATTEMPTS) {
			return cy.wait(250, { log: false }).then(function () {
				return coolFrameId(at + 1);
			});
		}

		expect(urls.join(', '),
			'no cool.html frame after ' + at + ' tries; the frames seen were')
			.to.contain('cool.html');
	});
}

function axNode(node) {
	const props = {};
	(node.properties || []).forEach(function (p) {
		props[p.name] = p.value && p.value.value;
	});
	return {
		role: node.role && node.role.value,
		roleType: node.role && node.role.type,
		name: (node.name && node.name.value) || '',
		description: (node.description && node.description.value) || '',
		ignored: node.ignored,
		backendDOMNodeId: node.backendDOMNodeId,
		properties: props,
	};
}

/// Every node of the whole document's accessibility tree, in one round trip.
/// getAXNodesWithin sweeps one container; getFocusedAXNode is a single widget.
function getAXNodes() {
	return coolFrameId().then(function (frameId) {
		return cdp('Accessibility.enable').then(function () {
			return cdp('Accessibility.getFullAXTree', { depth: -1, frameId: frameId });
		});
	}).then(function (res) {
		return ((res && res.nodes) || []).map(axNode);
	});
}

/// The document node of the cool frame, which is what a selector is resolved
/// against. DOM.getDocument returns the runner's document, and the cool one is
/// two frames down from it. Document nodes carry documentURL, not frameId --
/// that sits on the IFRAME element above them.
function coolDocumentNodeId() {
	return cy.then(function () {
		return cdp('DOM.enable');
	}).then(function () {
		return cdp('DOM.getDocument', { depth: -1, pierce: true });
	}).then(function (res) {
		let found = null;

		(function walk(node) {
			if (found !== null) return;
			if (node.nodeName === '#document' &&
					(node.documentURL || '').indexOf('cool.html') !== -1)
				found = node.nodeId;
			(node.children || []).forEach(walk);
			if (node.contentDocument) walk(node.contentDocument);
		})(res.root);

		expect(found, 'the cool.html document node in the DOM tree')
			.to.not.equal(null);
		return found;
	});
}

/// The accessibility subtree of one container, so an assertion names the
/// surface it actually read.
function getAXNodesWithin(selector) {
	return coolDocumentNodeId().then(function (documentNodeId) {
		return cdp('DOM.querySelector', {
			nodeId: documentNodeId,
			selector: selector,
		});
	}).then(function (res) {
		expect(res.nodeId, 'a node matching ' + selector).to.not.equal(0);

		return cdp('Accessibility.enable').then(function () {
			return cdp('Accessibility.queryAXTree', { nodeId: res.nodeId });
		});
	}).then(function (res) {
		return ((res && res.nodes) || []).map(axNode);
	});
}

/// role plus whatever identifies the element behind an unnamed node, so a
/// failure names something findable instead of a bare role.
function describeAXNode(node) {
	if (!node.backendDOMNodeId) return node.role;

	return cdp('DOM.describeNode', { backendNodeId: node.backendDOMNodeId })
		.then(function (res) {
			const attributes = (res.node && res.node.attributes) || [];
			const wanted = ['id', 'class'];
			const parts = [];

			for (let at = 0; at < attributes.length; at += 2) {
				if (wanted.indexOf(attributes[at]) !== -1 && attributes[at + 1])
					parts.push(attributes[at] + '="' + attributes[at + 1] + '"');
			}

			return node.role + (parts.length ? ' <' + parts.join(' ') + '>' : '');
		});
}

/// {role, name, ignored, properties} of the widget holding the focus, or null.
/// The document root reports itself as focused too, so take the deepest one.
function getFocusedAXNode() {
	return coolFrameId().then(function (frameId) {
		return cdp('Accessibility.enable').then(function () {
			return cdp('Accessibility.getFullAXTree', { depth: -1, frameId: frameId });
		});
	}).then(function (res) {
		const focused = (res.nodes || []).filter(function (n) {
			return (n.properties || []).some(function (p) {
				return p.name === 'focused' && p.value && p.value.value === true;
			});
		});
		const node = focused[focused.length - 1];
		if (!node) return null;
		const props = {};
		(node.properties || []).forEach(function (p) {
			props[p.name] = p.value && p.value.value;
		});
		return {
			role: node.role && node.role.value,
			roleType: node.role && node.role.type,
			name: (node.name && node.name.value) || '',
			nameSources: ((node.name && node.name.sources) || []).map(function (src) {
				return src.attribute || src.nativeSource || src.type;
			}),
			ignored: node.ignored,
			properties: props,
		};
	});
}

/**
 * A toolbutton shows it is on with the selected class; what a screen reader is
 * told is the pressed state of its node in the accessibility tree. Assert the
 * two never disagree over a container, reading the tree rather than the
 * attribute the browser built it from.
 */
function assertToggleStatesAgree(win, container, when) {
	const root = win.document.querySelector(container);
	expect(root, container + ' exists').to.not.equal(null);

	const shown = Array.prototype.map.call(
		root.querySelectorAll('button'),
		function (button) {
			return {
				id: button.id,
				name: (button.getAttribute('aria-label') ||
					button.textContent || '').trim(),
				looksPressed: button.classList.contains('selected'),
			};
		}).filter(function (button) {
			return button.name;
		});

	getAXNodes().then(function (nodes) {
		const byName = {};
		nodes.forEach(function (node) {
			if (node.role !== 'button') return;
			const key = node.name.trim();
			if (key) byName[key] = node;
		});

		const silent = [];
		const mismatched = [];

		shown.forEach(function (button) {
			const node = byName[button.name];
			if (!node) return;
			const says = node.properties.pressed;

			if (button.looksPressed && says === undefined) {
				silent.push(button.id + ' (' + button.name + ')');
			} else if (says !== undefined &&
					(says === 'true') !== button.looksPressed) {
				mismatched.push(button.id + ' looks ' +
					(button.looksPressed ? 'pressed' : 'unpressed') +
					' but the tree says ' + says);
			}
		});

		expect(silent, 'buttons that look pressed and announce nothing ' + when)
			.to.be.empty;
		expect(mismatched, 'buttons whose announced state disagrees ' + when)
			.to.be.empty;
	});
}

module.exports.enableUICoverage = enableUICoverage;
module.exports.reportUICoverage = reportUICoverage;
module.exports.resetState = resetState;
module.exports.checkA11yErrors = checkA11yErrors;
module.exports.runA11yValidation = runA11yValidation;
module.exports.getActiveDialog = getActiveDialog;
module.exports.testNameDialog = testNameDialog;
module.exports.closeActiveDialog = closeActiveDialog;
module.exports.getActiveTabPanel = getActiveTabPanel;
module.exports.traverseTabs = traverseTabs;
module.exports.handleTabsInDialog = handleTabsInDialog;
module.exports.handleDialog = handleDialog;
module.exports.testDialog = testDialog;
module.exports.allCommonDialogs = allCommonDialogs;
module.exports.needsLinguisticData = needsLinguisticData;
module.exports.testPDFExportWarningDialog = testPDFExportWarningDialog;
module.exports.sidebarTabOrder = sidebarTabOrder;
module.exports.describeFocusable = describeFocusable;
module.exports.openSidebarPropertyDeck = openSidebarPropertyDeck;
module.exports.sidebarKeyboard = sidebarKeyboard;
module.exports.axTreeAvailable = axTreeAvailable;
module.exports.getFocusedAXNode = getFocusedAXNode;
module.exports.assertToggleStatesAgree = assertToggleStatesAgree;
module.exports.getAXNodes = getAXNodes;
module.exports.getAXNodesWithin = getAXNodesWithin;
module.exports.describeAXNode = describeAXNode;
