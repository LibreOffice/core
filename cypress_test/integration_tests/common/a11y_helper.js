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
	return cy.cGet('.ui-dialog[role="dialog"]')
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
			cy.cGet(`#${CSS.escape(dialogId)} .ui-dialog-titlebar-close`)
				.click();
		});

	cy.cGet('.ui-dialog[role="dialog"]').should('have.length', level - 1);
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

	cy.cGet('.ui-dialog[role="dialog"]').should('have.length', level - 1);
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
			cy.cGet('.ui-dialog[role="dialog"]').should('have.length', level);
		});
	/* Then add the same name again so we get the warning subdialog */
	cy.cGet('button.ui-pushbutton[aria-label="Add"]:visible').click();
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
							if (command == '.uno:SetDocumentProperties' && tabAriaControls == 'customprops') {
								cy.cGet('#durationbutton-button').click();
								handleDialog(win, level + 1);
							} else if (command == '.uno:SetDocumentProperties' && tabAriaControls == 'general') {
								cy.cGet('#changepass-button').should('not.be.disabled').click();
								handleDialog(win, level + 1);
							} else if (command == '.uno:InsertSection' && tabAriaControls == 'section') {
								// check protect to enable password dialog
								cy.cGet('#protect-input').check();
								cy.cGet('#selectpassword-button').should('not.be.disabled').click();
								handleDialog(win, level + 1);
								cy.cGet('#protect-input').uncheck();
								cy.cGet('#selectpassword-button').should('be.disabled');
							} else if (command == '.uno:HyperlinkDialog' && tabAriaControls == '~Document') {
								cy.cGet('#browse-button').click();
								handleDialog(win, level + 1);
							} else if (command == '.uno:FontDialog' && tabAriaControls == 'font') {
								cy.cGet('#btnWestFeatures-button').click();
								handleDialog(win, level + 1);
							} else if ((command == '.uno:PageDialog' || command == '.uno:PageFormatDialog') && tabAriaControls == 'Footer') {
								cy.cGet('button.ui-pushbutton[aria-label="More..."]:visible').click();
								handleDialog(win, level + 1);
							} else if (command == '.uno:FormatArea' && tabAriaControls == 'lbhatch') {
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
 */
function handleDialog(win, level, command, isWarningDialog) {
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
				cy.cGet('#options-button').click();
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
			} else if (command == '.uno:Signature') {
				cy.cGet('#signatures .ui-treeview-entry > div:first-child').click();
				cy.cGet('#view-button').should('be.enabled').click();
				handleDialog(win, level + 1);
				cy.cGet('#sign-button').should('be.enabled').click();
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
				closeActiveWarningDialog(level);
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

	getActiveDialog(1)
		.then(() => {
			return helper.processToIdle(win);
		})
		.then(() => {
			cy.cGet('#forms-input').check();
			cy.cGet('#pdf_version-input').select('PDF/A-1b (PDF 1.4 base)');
			cy.cGet('#ok-button').click();
		})
		.then(() => {
			// pdf export dialog should dismiss and a warning dialog should appear
			return helper.processToIdle(win);
		})
		.then(() => {
			// and the warning dialog we're interested in should appear
			handleDialog(win, 1);
		});
}

module.exports.enableUICoverage = enableUICoverage;
module.exports.reportUICoverage = reportUICoverage;
module.exports.resetState = resetState;
module.exports.checkA11yErrors = checkA11yErrors;
module.exports.runA11yValidation = runA11yValidation;
module.exports.getActiveDialog = getActiveDialog;
module.exports.closeActiveDialog = closeActiveDialog;
module.exports.getActiveTabPanel = getActiveTabPanel;
module.exports.traverseTabs = traverseTabs;
module.exports.handleTabsInDialog = handleTabsInDialog;
module.exports.handleDialog = handleDialog;
module.exports.testDialog = testDialog;
module.exports.allCommonDialogs = allCommonDialogs;
module.exports.needsLinguisticData = needsLinguisticData;
module.exports.testPDFExportWarningDialog = testPDFExportWarningDialog;
