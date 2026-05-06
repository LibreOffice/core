/* -*- js-indent-level: 8 -*- */
/* global cy Cypress expect */

// Maximum viewport height for consistent screenshots across environments.
// Some Chrome/Chromium headless configurations reserve space for UI (or
// something of that nature) so viewport must be <= 581px to fit within that
// max available space to produce identical screenshots.
// See cypress-io/cypress#27260.
const maxScreenshotableViewportHeight = 581;

/*
 * Prepares the test document by copying or uploading it
 * filePath: test document file path
 * returns new test document file path
 */
function setupDocument(filePath, copyCertificates = false) {
	cy.log('>> setupDocument - start');
	cy.log('Param - filePath: ' + filePath);

	var newFilePath;
	if (Cypress.env('INTEGRATION') === 'nextcloud') {
		upLoadFileToNextCloud(filePath);
		newFilePath = filePath;
	} else if (Cypress.env('SERVER') !== 'localhost') {
		newFilePath = filePath;
	} else {
		// Rename and copy file to use a clean test document for every test case.
		var randomText = (Math.random() + 1).toString(36).substring(2,7);
		// The filename is used in a URI query string where characters like
		// '/' and '+' have special meaning, and spaces are invalid. Replace
		// these with hyphens so the path survives URL parsing unchanged.
		var cypressTestName = Cypress.currentTest.title.replace(/[\/\\ \.+]/g, '-');

		// Check for extension. The '.' has to be in fileName specifically, not earlier in filePath
		if (getFileName(filePath).includes('.')) {
			// Add text to name before the extension
			newFilePath = filePath.substr(0, filePath.lastIndexOf('.'))
				+ '-' + cypressTestName + '-' + randomText
				+ '.' + filePath.substr(filePath.lastIndexOf('.')+1, filePath.length);
		} else {
			// Add text to name at the end
			newFilePath = filePath + '-' + cypressTestName + '-' + randomText;
		}

		copyFile(filePath, newFilePath);
		if (copyCertificates) {
			const suffix = '.wopi.json';
			copyFile(filePath + suffix, newFilePath + suffix);
		}
	}

	cy.log('<< setupDocument - end');
	return newFilePath;
}

/*
 * Opens the document and waits for it to be ready
 * filePath: test document path
 * skipDocumentChecks: Skips the document checks that wait for it to be ready.
 *   This is useful for documents that have an interaction before the
 *   document is loaded, such as clearing a warning about macros.
 * isMultiUser: Set to true for multiuser tests.
 */
function loadDocument(filePath, skipDocumentChecks, isMultiUser, lang) {
	cy.log('>> loadDocument - start');
	cy.log('Param - filePath: ' + filePath);
	if (skipDocumentChecks) {
		cy.log('Param - skipDocumentChecks: ' + skipDocumentChecks);
	}
	if (isMultiUser) {
		cy.log('Param - isMultiUser: ' + isMultiUser);
	}

	// Set viewport
	doIfOnMobile(function() {
		// could be any phone type
		cy.viewport('iphone-6');
	});
	if (isMultiUser) {
		cy.viewport(2000,660);
	}

	// Set active frame
	if (isMultiUser) {
		cy.cSetActiveFrame('#iframe1');
	} else {
		cy.cSetActiveFrame('#coolframe');
	}

	// Load document
	if (Cypress.env('INTEGRATION') === 'nextcloud') {
		loadDocumentNextcloud(filePath);
	} else {
		loadDocumentNoIntegration(filePath, isMultiUser, lang);
	}

	const isDraw = filePath.indexOf('draw') === 0;

	// Wait for and verify that document is loaded
	if (!skipDocumentChecks) {
		if (isMultiUser) {
			cy.cSetActiveFrame('#iframe1');
			documentChecks(isDraw);
			cy.cSetActiveFrame('#iframe2');
			documentChecks(true);
		} else {
			// frame set above
			documentChecks(isDraw);
		}
	}

	cy.log('<< loadDocument - end');
}

/*
 * Covers most use cases. For more flexibility,
 * call setupDocument and loadDocument directly
 * filePath: test document path, for example: 'calc/hello-world.ods'
 */
function setupAndLoadDocument(filePath, isMultiUser = false, copyCertificates = false, lang = undefined) {
	cy.log('>> setupAndLoadDocument - start');

	var newFilePath = setupDocument(filePath, copyCertificates);
	if (isMultiUser) {
		loadDocument(newFilePath, undefined, isMultiUser, lang);
	} else {
		loadDocument(newFilePath, undefined, undefined, lang);
	}

	cy.log('<< setupAndLoadDocument - end');
	return newFilePath;
}

/*
 * Covers most use cases. For more flexibility,
 * call closeDocument and loadDocument directly
 */
function reloadDocument(filePath) {
	cy.log('>> reloadDocument - start');

	closeDocument(filePath);
	loadDocument(filePath, /*skipDocumentChecks*/ true);

	// loadDocument skips full documentChecks on reload, but we still
	// need to wait for the document canvas to be visible before
	// callers start asserting on document content.
	cy.cGet('#document-canvas', {timeout: Cypress.config('defaultCommandTimeout') * 2.0})
		.should('be.visible');

	cy.log('<< reloadDocument - end');
}

function copyFile(filePath, newFilePath) {
	// subFolder can be '', if filePath does not have a slash
	var subFolder = getSubFolder(filePath);
	var newSubFolder = getSubFolder(newFilePath);
	var fileName = getFileName(filePath);
	var newFileName = getFileName(newFilePath);

	cy.task('copyFile', {
		sourceDir: Cypress.env('DATA_FOLDER') + subFolder + '/',
		destDir: Cypress.env('DATA_WORKDIR') + newSubFolder + '/',
		fileName: fileName,
		destFileName: newFileName,
	});
}

function logError(event) {
	Cypress.log({ name:'error:', message: (event.error.message ? event.error.message : 'no message')
		      + '\n' + (event.error.stack ? event.error.stack : 'no stack') });
}

/*
 * Loads the test document directly in Collabora Online.
 */
function loadDocumentNoIntegration(filePath, isMultiUser, lang) {
	cy.log('>> loadDocumentNoIntegration - start');

	var URI = '';

	if (Cypress.env('INTEGRATION') === 'php-proxy') {
		URI += 'http://' + Cypress.env('SERVER') + '/proxy.php?req=';
	}

	URI += '/browser/' + Cypress.env('WSD_VERSION_HASH') + '/debug.html'
		+ '?lang=' + (lang || 'en-US')
		+ '&file_path=' + Cypress.env('DATA_WORKDIR') + filePath;

	if (Cypress.env('INTEGRATION') === 'php-proxy') {
		const serverPort = Cypress.env('SERVER_PORT');
		if (serverPort)
			URI += '&wopiPort=' + serverPort;
	}

	if (isMultiUser) {
		URI = URI.replace('debug.html', 'cypress-multiuser.html');
	}

	cy.visit(URI, {
		onBeforeLoad: function(win) {
			win.addEventListener('error', logError);
			win.addEventListener('DOMContentLoaded', function () {
				for (var i = 0; i < win.frames.length; i++) {
					win.frames[i].addEventListener('error', logError);
				}
			});
		}
	}).then(function() {
		if (Cypress.config('logServerResponse')) {
			cy.getFrameWindow()
				.its('L', {log: false})
				.then(function(L) {
					cy.stub(L.initial, '_stubMessage')
						.log(false)
						.callsFake(function(e){
							Cypress.log({name: 'server response =>', message: e});
						});
				});
		}
	});

	cy.log('<< loadDocumentNoIntegration - end');
}

/*
 * Loads the test document inside a Nextcloud integration
 */
function loadDocumentNextcloud(filePath) {
	cy.log('>> loadDocumentNextcloud - start');
	cy.log('Param - filePath: ' + filePath);

	var fileName = getFileName(filePath);

	// Open test document
	cy.cGet('tr[data-file=\'' + fileName + '\']').click();

	cy.cGet('iframe#richdocumentsframe').should('be.visible', {timeout : Cypress.config('defaultCommandTimeout') * 2.0});

	cy.wait(10000);

	// We create global aliases for iframes, so it's faster to reach them.
	cy.cGet('iframe#richdocumentsframe')
		.its('0.contentDocument').should('exist')
		.its('body').should('not.be.undefined')
		.then(cy.wrap).as('richdocumentsIFrameGlobal');

	cy.cGet('@richdocumentsIFrameGlobal')
		.find('iframe#coolframe')
		.its('0.contentDocument').should('exist')
		.its('body').should('not.be.undefined')
		.then(cy.wrap).as('loleafletIFrameGlobal');

	// The IFRAME_LEVEL environment variable will indicate
	// in which iframe we have.
	cy.cGet('iframe#richdocumentsframe')
		.then(function() {
			Cypress.env('IFRAME_LEVEL', '2');
		});

	cy.log('<< loadDocumentNextcloud - end');
}

// Hide NC's first run wizard, which is opened by the first run of
// nextcloud. When we run cypress in headless mode, NC don't detect
// that we already used it and so it always opens this wizard.
function hideNCFirstRunWizard() {
	cy.log('>> hideNCFirstRunWizard - start');

	// Hide first run wizard if it's there
	cy.wait(2000); // Wait some time to the wizard become visible, if it's there.
	cy.cGet('body')
		.then(function(body) {
			if (body.find('#firstrunwizard').length !== 0) {
				cy.cGet('#firstrunwizard')
					.then(function(wizard) {
						wizard.hide();
					});
			}
		});

	cy.log('<< hideNCFirstRunWizard - end');
}

// Upload a test document into Nextcloud and open it.
// Parameters:
// filePath - test document file path
// subsequentLoad - whether we load a test document for the first time in the
//                  test case or not. It's important because we need to sign in
//                  with the username + password only for the first time.
function upLoadFileToNextCloud(filePath, subsequentLoad) {
	cy.log('>> upLoadFileToNextCloud - start');
	cy.log('Param - filePath: ' + filePath);
	cy.log('Param - subsequentLoad: ' + subsequentLoad);

	// TODO: subsequentLoad appears to be unused
	// TODO: see if cy.session can handle login

	var fileName = getFileName(filePath);

	// Open local nextcloud installation
	var url = 'http://' + Cypress.env('SERVER') + 'nextcloud/index.php/apps/files';
	cy.visit(url);

	// Log in with cypress test user / password (if this is the first time)
	if (subsequentLoad !== true) {
		cy.cGet('input#user').clear().type('cypress_test');

		cy.cGet('input#password').clear().type('cypress_test');

		cy.cGet('input#submit-form').click();

		cy.cGet('.button.new').should('be.visible');

		// Wait for free space calculation before uploading document
		cy.cGet('#free_space').should('not.have.attr', 'value', '');

		hideNCFirstRunWizard();

		// Remove all existing file, so we make sure the test document is removed
		// and then we can upload a new one.
		cy.cGet('#fileList')
			.then(function(filelist) {
				if (filelist.find('tr').length !== 0) {
					cy.waitUntil(function() {
						cy.cGet('#fileList tr:nth-of-type(1) .action-menu.permanent')
							.click();

						cy.cGet('.menuitem.action.action-delete.permanent')	.click();

						cy.cGet('#uploadprogressbar').should('not.be.visible');

						return cy.cGet('#fileList')
							.then(function(filelist) {
								return filelist.find('tr').length === 0;
							});
					}, {timeout: 60000});
				}
			});
	} else {
		// Wait for free space calculation before uploading document
		cy.cGet('#free_space').should('not.have.attr', 'value', '');

		hideNCFirstRunWizard();
	}

	cy.cGet('tr[data-file=\'' + fileName + '\']').should('not.exist');

	// Upload test document
	doIfOnDesktop(function() {
		cy.cGet('input#file_upload_start')
			.attachFile({ filePath: 'desktop/' + filePath, encoding: 'binary' });
	});
	doIfOnMobile(function() {
		cy.cGet('input#file_upload_start')
			.attachFile({ filePath: 'mobile/' + filePath, encoding: 'binary' });
	});

	cy.cGet('#uploadprogressbar')
		.should('not.be.visible');

	cy.cGet('tr[data-file=\'' + fileName + '\']')
		.should('be.visible');

	cy.log('<< upLoadFileToNextCloud - end');
}

// Used for interference testing. We wait until the interfering user loads
// its instance of the document and starts its interfering actions.
// So we can be sure the interference actions are made during the test
// user does the actual test steps.
function waitForInterferingUser() {
	cy.log('>> waitForInterferingUser - start');

	cy.cGet('#toolbar-up #userlist', { timeout: Cypress.config('defaultCommandTimeout') * 2.0 })
		.should('be.visible');

	cy.wait(10000);

	cy.log('<< waitForInterferingUser - end');
}

function documentChecks(skipInitializedCheck = false) {
	cy.log('>> documentChecks - start');

	var canvasCheck = cy.cGet('#document-canvas', {timeout : Cypress.config('defaultCommandTimeout') * 2.0});
	if (!skipInitializedCheck) {
		canvasCheck.should('be.visible');
		cy.cGet('#map', {timeout : Cypress.config('defaultCommandTimeout') * 2.0})
			.should('have.class', 'initialized');
	}

	// With php-proxy the client is irresponsive for some seconds after load, because of the incoming messages.
	if (Cypress.env('INTEGRATION') === 'php-proxy') {
		cy.wait(10000);
	}

	if (!skipInitializedCheck /* TODO: if notebookbar mode */) {
		doIfOnDesktop(() => {
			cy.cGet('.notebookbar-scroll-wrapper', {timeout : Cypress.config('defaultCommandTimeout') * 2.0})
				.should('have.class', 'initialized');
		});
	}

	// Wait for the sidebar to open.
	if (Cypress.env('INTEGRATION') !== 'nextcloud') {
		doIfOnDesktop(function() {
			if (Cypress.env('pdf-view') !== true)
				cy.cGet('#sidebar-panel', { timeout: 20000 }).should('be.visible').should('not.be.empty');

			// Check that the document does not take the whole window width.
			cy.window()
				.then(function(win) {
					cy.cframe().find('#document-container')
						.should(function(doc) {
							expect(doc).to.have.lengthOf(1);
							if (Cypress.env('pdf-view') !== true)
								expect(doc[0].getBoundingClientRect().right).to.be.lessThan(win.innerWidth * 0.95);
						});
				});
		});

		// In Writer wait for styles to appear in notebookbar
		doIfOnDesktop(() => {
			doIfInWriter(() => {
				cy.cGet('#stylesview.notebookbar .icon-view-item-container img')
					.should('exist');
			});
		});

		// Check also that the inputbar is drawn in Calc.
		doIfInCalc(() => {
			cy.cGet('#sc_input_window.formulabar').should('exist');
			cy.cGet('#pos_window-input-address.addressInput').should('exist');
			//cy.cGet('#pos_window-input-address.addressInput').should('not.be.empty');
		});
	}

	// Ensure the busypopup overlay is gone so it doesn't swallow clicks.
	if (!skipInitializedCheck) {
		cy.cGet('#busypopup-overlay').should('not.exist');
	}

	if (Cypress.env('INTERFERENCE_TEST') === true) {
		waitForInterferingUser();
	}

	cy.log('<< documentChecks - end');
}

// Assert that NO keyboard input is accepted (i.e. keyboard should be HIDDEN).
function assertNoKeyboardInput() {
	cy.log('>> assertNoKeyboardInput - start');

	cy.cGet('div.clipboard').should('have.attr', 'data-accept-input', 'false');

	cy.log('<< assertNoKeyboardInput - end');
}

// Assert that keyboard input is accepted (i.e. keyboard should be VISIBLE).
function assertHaveKeyboardInput() {
	cy.log('>> assertHaveKeyboardInput - start');

	cy.cGet('div.clipboard').should('have.attr', 'data-accept-input', 'true');

	cy.log('<< assertHaveKeyboardInput - end');
}

// Assert that we have cursor and focus on the text area of the document.
function assertCursorAndFocus() {
	cy.log('>> assertCursorAndFocus - start');

	if (Cypress.env('INTEGRATION') !== 'nextcloud') {
		// Active element must be the textarea named clipboard.
		assertFocus('className', 'clipboard');
	}

	// In edit mode, we should have the blinking cursor.
	cy.cGet('.leaflet-cursor.blinking-cursor').should('exist');
	cy.cGet('.leaflet-cursor-container').should('exist');

	assertHaveKeyboardInput();

	cy.log('<< assertCursorAndFocus - end');
}

// Select all text via CTRL+A shortcut.
// options.isTable: when true, use multiple ctrl+a to select all text beyond current table
function selectAllText(options) {
	var isTable = options && options.isTable;

	cy.log('>> selectAllText - start');

	if (isTable) {
		typeIntoDocument('{ctrl}a');
		typeIntoDocument('{ctrl}a');
		typeIntoDocument('{ctrl}a');
	} else {
		typeIntoDocument('{ctrl}a');
	}

	textSelectionShouldExist();

	cy.log('<< selectAllText - end');
}

// Clear all text by selecting all and deleting.
// options.isTable: when true, use multiple ctrl+a to select all text beyond current table
function clearAllText(options) {
	cy.log('>> clearAllText - start');

	//assertCursorAndFocus();

	// Trigger select all
	selectAllText(options);

	// Then remove
	typeIntoDocument('{backspace}');

	textSelectionShouldNotExist();

	cy.log('<< clearAllText - end');
}

// Check that the clipboard text matches with the specified text.
// Parameters:
// expectedPlainText - a string, the clipboard container should have.
function expectTextForClipboard(expectedPlainText) {
	cy.log('>> expectTextForClipboard - start');

	cy.log('Text:' + expectedPlainText);

	// FIXME: create explicit function for Writer and other modules
	// "p" and "p font" are for Writer only, pre for Calc and Impress
	cy.cGet('#copy-paste-container').then(function(pItem) {
		if (pItem.children('font').length !== 0) {
			cy.cGet('#copy-paste-container p font').should('have.text', expectedPlainText);
		} else {
			cy.cGet('#copy-paste-container p, #copy-paste-container pre').should('have.text', expectedPlainText);
		}
	});

	cy.log('<< expectTextForClipboard - end');
}

// Check that the clipboard text matches with the
// passed regular expression.
// Parameters:
// regexp - a regular expression to match the content with.
//          https://docs.cypress.io/api/commands/contains.html#Regular-Expression
function matchClipboardText(regexp) {
	cy.log('>> matchClipboardText - start');

	// FIXME: create explicit function for Writer and other modules
	// "p" and "p font" are for Writer only, pre for Calc and Impress
	cy.cGet('#copy-paste-container').then(function(pItem) {
		if (pItem.children('font').length !== 0) {
			cy.cGet('body').contains('#copy-paste-container p font', regexp).should('exist');
		} else {
			cy.cGet('body').contains('#copy-paste-container pre', regexp).should('exist');
		}
	});

	cy.log('<< matchClipboardText - end');
}

function clipboardTextShouldBeDifferentThan(text) {
	cy.log('>> clipboardTextShouldBeDifferentThan - start');

	// FIXME: create explicit function for Writer and other modules
	// "p" and "p font" are for Writer only, pre for Calc and Impress
	cy.cGet('#copy-paste-container').then(function(pItem) {
		if (pItem.children('font').length !== 0) {
			cy.cGet('body').contains('#copy-paste-container p font', text).should('not.exist');
		} else {
			cy.cGet('body').contains('#copy-paste-container pre', text).should('not.exist');
		}
	});

	cy.log('<< clipboardTextShouldBeDifferentThan - end');
}

// This method is intended to call after each test case.
// We use this method to close the document, before step
// on to the next test case.
// Parameters:
// filePath - test document path (we can check it on the admin console).
// testState - whether the test passed or failed before this method was called.
function closeDocument(filePath) {
	cy.log('>> closeDocument - start');

	var fileName = getFileName(filePath);

	if (Cypress.env('INTEGRATION') === 'nextcloud') {
		if (Cypress.env('IFRAME_LEVEL') === '2') {
			// Close the document, with the close button.
			doIfOnMobile(function() {
				cy.cGet('#toolbar-mobile-back').click();
				cy.cGet('#mobile-edit-button').should('be.visible');
				cy.cGet('#toolbar-mobile-back').then(function(item) {
						cy.wrap(item).click();
						Cypress.env('IFRAME_LEVEL', '');
					});
			});
			doIfOnDesktop(function() {
				cy.cGet('#closebutton').then(function(item) {
						cy.wrap(item).click();
						Cypress.env('IFRAME_LEVEL', '');
					});
			});

			cy.cGet('#filestable').should('be.visible');
			cy.cGet('#filestable').should('not.have.class', 'hidden');

			cy.wait(3000);

			// Remove the document
			cy.cGet('tr[data-file=\'' + fileName + '\'] .action-menu.permanent').click();
			cy.cGet('.menuitem.action.action-delete.permanent').click();
			cy.cGet('tr[data-file=\'' + fileName + '\']').should('not.exist');

		}
	// For php-proxy admin console does not work, so we just open
	// localhost and wait some time for the test document to be closed.
	} else if (Cypress.env('INTEGRATION') === 'php-proxy') {
		cy.visit('http://' + Cypress.env('SERVER') + '/', {failOnStatusCode: false});

		cy.wait(5000);
	} else {
		if (Cypress.env('INTERFERENCE_TEST') === true) {
			typeIntoDocument('{ctrl}s');

			cy.wait(2000);
		}

		// Make sure that the document is closed
		cy.visit('http://admin:admin@' + Cypress.env('SERVER') + ':' +
			Cypress.env('SERVER_PORT') +
			'/browser/dist/admin/admin.html');

		// We have all lines of document infos as one long string.
		// We have PID number before the file names, with matching
		// also on the PID number we can make sure to match on the
		// whole file name, not on a suffix of a file name.
		var regex = new RegExp('[0-9]' + fileName);
		cy.get('#doclist').invoke('text').should('not.match', regex);
	}

	cy.log('<< closeDocument - end');
}

// Initialize an alias to a negative number value. It can be useful
// when we use an alias as a variable and later we intend to set it
// to a non-negative value.
// Parameters:
// aliasName - a string, expected to be used as alias.
function initAliasToNegative(aliasName) {
	cy.log('>> initAliasToNegative - start');
	cy.log('Param - aliasName: ' + aliasName);

	cy.cGet('#copy-paste-container')
		.invoke('offset')
		.its('top')
		.as(aliasName);

	cy.get('@' + aliasName).should('be.lessThan', 0);

	cy.log('<< initAliasToNegative - end');
}

// Wait for attribute which appears after specialized UI is loaded
function waitForDocType() {
		cy.cGet('body', {log: false})
		.should('have.attr', 'data-docType');
}

// Run a callback if docType is matching provided value
function checkDocTypeAndRun(docType, matching, callback) {
	waitForDocType();

	cy.cframe().find('body', {log: false})
		.then((bodyElm) => {
			cy.log('>> doIf ' + docType + ' - start');

			const isMatching = bodyElm.get(0).getAttribute('data-docType') == docType;
			if (isMatching == matching) {
				cy.log('>> doIf ' + docType + ' - TRUE');
				callback();
				return;
			}

			cy.log('>> doIf ' + docType + ' - FALSE');
		});
}

// Run a code snippet if we are inside Calc.
function doIfInCalc(callback) {
	checkDocTypeAndRun('spreadsheet', true, callback);
}

// Run a code snippet if we are *NOT* inside Calc.
function doIfNotInCalc(callback) {
	checkDocTypeAndRun('spreadsheet', false, callback);
}

// Run a code snippet if we are inside Impress.
function doIfInImpress(callback) {
	checkDocTypeAndRun('presentation', true, callback);
}

// Run a code snippet if we are *NOT* inside Impress.
function doIfNotInImpress(callback) {
	checkDocTypeAndRun('presentation', false, callback);
}

// Run a code snippet if we are inside Writer.
function doIfInWriter(callback) {
	checkDocTypeAndRun('text', true, callback);
}

// Run a code snippet if we are *NOT* inside Writer.
function doIfNotInWriter(callback) {
	checkDocTypeAndRun('text', false, callback);
}

// Types text into elem with a delay in between characters.
// Sometimes cy.type results in random character insertion,
// this avoids that, which is not clear why it happens.
// Parameters:
// selector - a CSS selector to query a DOM element to type in.
// text - a text, what we'll type char-by-char.
// delayMs - delay in ms between the characters.
function typeText(selector, text, delayMs = 0) {
	cy.log('>> typeText - start');

	for (var i = 0; i < text.length; i++) {
		cy.cGet(selector).type(text.charAt(i));
		if (delayMs > 0)
			cy.wait(delayMs);
	}

	cy.log('<< typeText - end');
}

// Check whether an img DOM element has only white colored pixels or not.
function isImageWhite(selector, expectWhite = true) {
	cy.log('>> isImageWhite - start');

	expect(selector).to.have.string('img');

	cy.cGet(selector)
		.should(function(images) {
			var img = images[0];

			// Ensure the image has actual rendered content, not a
			// placeholder (e.g. SVG loading spinner for slide previews).
			expect(img.complete).to.be.true;
			expect(img.naturalWidth).to.be.greaterThan(0);

			// Create an offscreen canvas to check the image's pixels
			var canvas = document.createElement('canvas');
			canvas.width = img.width;
			canvas.height = img.height;
			canvas.getContext('2d').drawImage(img, 0, 0, img.width, img.height);
			var context = canvas.getContext('2d');

			// Ignore a small zone on the edges, to ignore border.
			var ignoredPixels = 2;
			var pixelData = context.getImageData(ignoredPixels, ignoredPixels,
				img.width - 2 * ignoredPixels,
				img.height - 2 * ignoredPixels).data;

			var result = true;
			for (var i = 0; i < pixelData.length; ++i) {
				if (pixelData[i] !== 255) {
					result = false;
					break;
				}
			}
			if (expectWhite)
				expect(result).to.be.true;
			else
				expect(result).to.be.false;
		});

	cy.log('<< isImageWhite - end');
}

function isCanvasWhite(expectWhite = true) {
	cy.log('>> isCanvasWhite - start');

	cy.wait(300);
	cy.cGet('#document-canvas').should('exist').then(function(canvas) {
		var result = true;
		var context = canvas[0].getContext('2d');
		var pixelData = context.getImageData(0, 0, canvas[0].width, canvas[0].height).data;
		for (var i = 0; i < pixelData.length; i++) {
			if (pixelData[i] !== 255) {
				result = false;
				break;
			}
		}

		if (expectWhite)
			expect(result).to.be.true;
		else
			expect(result).to.be.false;
	});

	cy.log('<< isCanvasWhite - end');
}

// Run a code snippet if we are in a mobile test.
function doIfOnMobile(callback) {
	cy.window({log: false})
		.then(function(win) {
			if (win.navigator.userAgent === 'cypress-mobile') {
				callback();
			}
		});
}

// Run a code snippet if we are in a desktop test.
function doIfOnDesktop(callback) {
	cy.window({log: false})
		.then(function(win) {
			if (win.navigator.userAgent === 'cypress') {
				callback();
			}
		});
}

// Move the cursor in the given direction and wait until it moves.
// Parameters:
// direction - the direction the cursor should be moved.
//			   possible values: up, down, left, right, home, end
// modifier - a modifier to the cursor movement keys (e.g. 'shift' or 'ctrl').
// checkCursorVis - whether to check the cursor visibility after movement.
// cursorSelector - selector for the cursor DOM element (document cursor is the default).
function moveCursor(direction, modifier,
	checkCursorVis = true,
	cursorSelector = '.cursor-overlay .blinking-cursor') {
	cy.log('>> moveCursor - start');
	cy.log('Param - direction: ' + direction);
	cy.log('Param - modifier: ' + modifier);
	cy.log('Param - checkCursorVis: ' + checkCursorVis);
	cy.log('Param - cursorSelector: ' + cursorSelector);

	// Get the original cursor position.
	var origCursorPos = 0;
	cy.cGet(cursorSelector)
		.should(function(cursor) {
			if (direction === 'up' ||
				direction === 'down' ||
				(direction === 'home' && modifier === 'ctrl') ||
				(direction === 'end' && modifier === 'ctrl')) {
				origCursorPos = cursor.offset().top;
			} else if (direction === 'left' ||
				direction === 'right' ||
				direction === 'home' ||
				direction === 'end') {
				origCursorPos = cursor.offset().left;
			}
			expect(origCursorPos).to.not.equal(0);
		});

	// Move the cursor using keyboard input.
	var key = '';
	if (modifier === 'ctrl') {
		key = '{ctrl}';
	} else if (modifier === 'shift') {
		key = '{shift}';
	}

	if (direction === 'up') {
		key += '{uparrow}';
	} else if (direction === 'down') {
		key += '{downarrow}';
	} else if (direction === 'left') {
		key += '{leftarrow}';
	} else if (direction === 'right') {
		key += '{rightarrow}';
	} else if (direction === 'home') {
		key += '{home}';
	} else if (direction === 'end') {
		key += '{end}';
	}

	typeIntoDocument(key);

	// Make sure the cursor position was changed.
	cy.cGet(cursorSelector)
		.should(function(cursor) {
			if (direction === 'up' ||
				direction === 'down' ||
				(direction === 'home' && modifier === 'ctrl') ||
				(direction === 'end' && modifier === 'ctrl')) {
				expect(cursor.offset().top).to.not.equal(origCursorPos);
			} else if (direction === 'left' ||
					direction === 'right' ||
					direction === 'end' ||
					direction === 'home') {
				expect(cursor.offset().left).to.not.equal(origCursorPos);
			}
		});

	// Cursor should be visible after move, because the view always follows it.
	if (checkCursorVis === true) {
		cy.cGet(cursorSelector).should('be.visible');
	}

	cy.log('<< moveCursor - end');
}

// Type something into the document. It can be some text or special characters too.
function typeIntoDocument(text) {
	cy.log('>> typeIntoDocument - start');

	cy.cGet('div.clipboard').type(text, {force: true});

	cy.log('<< typeIntoDocument - end');
}

// Get cursor's current position.
// Parameters:
// offsetProperty - which offset position we need (e.g. 'left' or 'top').
// aliasName - we create an alias with the queried position.
// cursorSelector - selector to find the correct cursor element in the DOM.
function getCursorPos(offsetProperty, aliasName, cursorSelector = '.cursor-overlay .blinking-cursor') {
	cy.log('>> getCursorPos - start');

	initAliasToNegative(aliasName);

	cy.cGet(cursorSelector)
		.invoke('offset')
		.its(offsetProperty)
		.as(aliasName);

	cy.get('@' + aliasName).then(aliasValue => {
		var value = aliasValue;
		cy.wrap(value).as(aliasName);
	});

	cy.get('@' + aliasName)
		.should('be.greaterThan', 0);

	cy.log('<< getCursorPos - end');
}

// We make sure we have a text selection..
function textSelectionShouldExist() {
	cy.log('>> textSelectionShouldExist - start');

	cy.cGet('.text-selection-handle-start').should('exist');
	cy.cGet('.text-selection-handle-end').should('exist');

	// One of the marker should be visible at least (if not both).
	doIfOnMobile(function() {
		cy.cGet('.text-selection-handle-start, .text-selection-handle-end').should('be.visible');
	})

	cy.log('<< textSelectionShouldExist - end');
}

// We make sure we don't have a text selection..
function textSelectionShouldNotExist() {
	cy.log('>> textSelectionShouldNotExist - start');

	cy.cGet('.text-selection-handle-start').should('not.be.visible');
	cy.cGet('.text-selection-handle-end').should('not.be.visible');

	cy.log('<< textSelectionShouldNotExist - end');
}

// Used to represent the bounds of overlays like cell-cursor, document selections etc.
class Bounds {
	constructor(top, left, width, height) {
		this.set(top, left, width, height);
	}

	set(top, left, width, height) {
		/** @type {number} */
		this.top = top;
		/** @type {number} */
		this.left = left;
		/** @type {number} */
		this.width = width;
		/** @type {number} */
		this.height = height;
	}

	isValid() {
		return (this.top !== undefined
			&& this.left !== undefined
			&& this.width !== undefined
			&& this.height !== undefined);
	}

	/**
	 * Checks whether "other" bounds lies within self bounds.
	 * @param {Bounds} other
	 */
	contains(other) {
		return (other.top >= this.top && other.bottom <= this.bottom &&
			other.left >= this.left && other.right <= this.right);
	}

	get right() {
		return this.left + this.width;
	}

	get bottom() {
		return this.top + this.height;
	}

	static parseBoundsJson(boundsJsonString) {
		var jsonObject = JSON.parse(boundsJsonString);
		return new Bounds(jsonObject.top, jsonObject.left, jsonObject.width, jsonObject.height);
	}

	parseSetJson(boundsJsonString) {
		var jsonObject = JSON.parse(boundsJsonString);
		this.set(jsonObject.top, jsonObject.left, jsonObject.width, jsonObject.height);
	}

	toString() {
		return '{"top":' + this.top + ',"left":' + this.left
			+ ',"width":' + this.width + ',"height":' + this.height + '}';
	}
}

// Used to get the bounds of canvas section/overlay items from the JSON text inside its
// test div element.
// Parameters:
// itemDivId - The id of the test div element corresponding to the overlay item.
// bounds - A Bounds object in which this function stores the bounds of the overlay item.
//          The bounds unit is core pixels in document coordinates.
function getItemBounds(itemDivId, bounds) {
	cy.log('>> getItemBounds - start');

	cy.cGet(itemDivId)
		.should(function (itemDiv) {
			bounds.parseSetJson(itemDiv.text());
			expect(bounds.isValid()).to.be.true;
		});

	cy.log('<< getItemBounds - end');
}

var getOverlayItemBounds = getItemBounds;

// This ensures that the overlay item has the expected bounds via its test div element.
// Parameters:
// itemDivId - The id of the test div element corresponding to the overlay item.
// bounds - A Bounds object with the expected bounds data.
//          The bounds unit should be core pixels in document coordinates.
function overlayItemHasBounds(itemDivId, expectedBounds) {
	cy.log('>> overlayItemHasBounds - start');

	cy.cGet(itemDivId)
		.should(function (elem) {
			expect(Bounds.parseBoundsJson(elem.text()))
				.to.deep.equal(expectedBounds, 'Bounds of ' + itemDivId);
		});

	cy.log('<< overlayItemHasBounds - end');
}

// This ensures that the overlay item has different bounds from the given one
// via its test div element.
// Parameters:
// itemDivId - The id of the test div element corresponding to the overlay item.
// bounds - A Bounds object with the bounds data to compare.
function overlayItemHasDifferentBoundsThan(itemDivId, bounds) {
	cy.log('>> overlayItemHasDifferentBoundsThan - start');
	cy.log(bounds.toString());

	cy.cGet(itemDivId)
		.should(function (elem) {
			expect(elem.text()).to.not.equal(bounds.toString());
		});

	cy.log('<< overlayItemHasDifferentBoundsThan - end');
}

// Type some text into an input DOM item.
// Parameters:
// selector - selector to find the correct input item in the DOM.
// text - string to type in (can contain cypress command strings).
// clearBefore - whether clear the existing content or not.
function typeIntoInputField(selector, text, clearBefore = true)
{
	cy.log('>> typeIntoInputField - start');

	cy.wait(600);
	cy.cGet(selector).type((clearBefore ? '{selectall}{backspace}' : '') + text + '{enter}');
	cy.wait(600);
	cy.cGet(selector).should('have.value', text);

	cy.log('<< typeIntoInputField - end');
}

function getVisibleBounds(domRect) {
	return new Bounds(
		Math.max(0, domRect.top),
		Math.max(0, domRect.left),
		domRect.width,
		domRect.height);
}

function assertFocus(selectorType, selector) {
	cy.log('>> assertFocus - start');

	cy.cGet().its('activeElement.'+selectorType).should('be.eq', selector);

	cy.log('<< assertFocus - end');
}

// Create an alias to a point whose coordinate are the middle point of the blinking cursor
// It should be used with clickAt (see function below)
function getBlinkingCursorPosition(aliasName) {
	cy.log('>> getBlinkingCursorPosition - start');

	var cursorSelector = '.cursor-overlay .blinking-cursor';
	cy.cGet(cursorSelector).then(function(cursor) {
		var boundRect = cursor[0].getBoundingClientRect();
		var xPos = (boundRect.left + boundRect.right) / 2;
		var yPos = (boundRect.top + boundRect.bottom) / 2;
		cy.wrap({x: xPos, y: yPos}).as(aliasName);
	});

	cy.get('@' + aliasName).then(point => {
		expect(point.x).to.be.greaterThan(0);
		expect(point.y).to.be.greaterThan(0);
	});

	cy.log('<< getBlinkingCursorPosition - end');
}

// Simulate a click at the point referenced by the passed alias.
// If the 'double' parameter is true, a double click is simulated.
// To be used in pair with getBlinkingCursorPosition (see function above)
function clickAt(aliasName, double = false) {
	cy.log('>> clickAt - start');

	cy.get('@' + aliasName).then(point => {
		expect(point.x).to.be.greaterThan(0);
		expect(point.y).to.be.greaterThan(0);
		if (double) {
			cy.cGet('body').dblclick(point.x, point.y);
		} else {
			cy.cGet('body').click(point.x, point.y);
		}
	});

	cy.log('<< clickAt - end');
}

// Replaces the system clipboard with a dummy one for copy purposes. The copy content for the
// specified type will be injected into the DOM, so the caller can assert it.
function setDummyClipboardForCopy(type) {
	if (type === undefined) {
		type = 'text/html';
	}
	cy.window().then(win => {
		const app = win['0'].app;
		const clipboard = app.map._clip;
		clipboard._dummyClipboard = {
			write: function(clipboardItems) {
				const clipboardItem = clipboardItems[0];
				clipboardItem.getType(type).then(blob => blob.text())
				.then(function (text) {
					if (type === 'text/html') {
						clipboard._dummyDiv.innerHTML = text;
					} else if (type == 'text/plain') {
						clipboard._dummyPlainDiv.innerHTML = text;
					}
				});
				return {
					then: function(resolve/*, reject*/) {
						resolve();
					},
				};
			},
		};
	});
}

// Clicks the Copy button on the UI.
function copy() {
	cy.log('helper.copy()');
	return cy.window({log: false}).then(win => {
		const app = win['0'].app;
		const clipboard = app.map._clip;
		clipboard.filterExecCopyPaste('.uno:Copy');
	});
}

/*
 * Get 'hello.ods' from 'calc/hello.ods'
 */
function getFileName(filePath) {
	var fileName;
	if (filePath.includes('/')) {
		fileName = filePath.substr(filePath.lastIndexOf('/')+1, filePath.length);
	} else {
		fileName = filePath;
	}
	return fileName;
}

/*
 * Get 'calc' from 'calc/hello.ods'
 */
function getSubFolder(filePath) {
	var subFolder;
	if (filePath.includes('/')) {
		subFolder = filePath.substr(0, filePath.lastIndexOf('/'));
	} else {
		subFolder = '';
	}
	return subFolder;
}

// Return the center of the shape SVG overlay in coordinates suitable for
// clicking on #document-canvas.  The outer SVG (created by
// ShapeHandlesSection.setSVG) carries left/top relative to
// #canvas-container; width/height may be on the inner SVG (when a viewBox
// is present) or on the outer SVG.
function getShapeSVGCenter() {
	return cy.cGet('#canvas-container > svg')
		.then(function(outerSvg) {
			var outer = outerSvg[0];
			var inner = outer.querySelector('svg');
			var width = parseInt(inner && inner.style.width) || parseInt(outer.style.width);
			var height = parseInt(inner && inner.style.height) || parseInt(outer.style.height);
			expect(width).to.be.greaterThan(0);
			expect(height).to.be.greaterThan(0);
			return {
				x: parseInt(outer.style.left) + width / 2,
				y: parseInt(outer.style.top) + height / 2,
			};
		});
}

/*
 * Assert image svg
 */
function assertImageSize(expectedWidth, expectedHeight) {
	cy.log('>> assertImageSize - start');

	cy.cGet('#canvas-container > svg', {timeout: 1000})
		.then(function (element) {
			expect(element).to.have.length(1);
			const actualWidth = parseInt(element[0].style.width.replace('px', ''));
			const actualHeight = parseInt(element[0].style.height.replace('px', ''));

			expect(actualWidth).to.be.closeTo(expectedWidth, 10);
			expect(actualHeight).to.be.closeTo(expectedHeight, 10);
		});

	// wait for above async result
	cy.wait(3000);

	cy.log('<< assertImageSize - end');
}

function containsFocusElement(container, doesContain) {
	cy.getFrameWindow().then(function(win) {
		cy.expect(container.contains(win.document.activeElement)).to.equal(doesContain);
	});
}

function getMenuEntry(index) {
	cy.log('>> getMenuEntry - ' + index);
	return cy.cGet('.ui-dialog-content div.ui-combobox-entry span').eq(index);
}

var idleCounter = 0;

function waitUntilCoreIsIdle(win) {
	var expectedIdleID = String(++idleCounter);
	var spy;
	var ownSpy = false;

	cy.then(function() {
		// Check if _onMessage is already wrapped/spied
		if (win.app.socket._onMessage.restore) {
			// Already wrapped, use the existing spy
			spy = win.app.socket._onMessage;
		} else {
			// Create our own spy
			spy = Cypress.sinon.spy(win.app.socket, '_onMessage');
			ownSpy = true;
		}

		var idleArgs = {
			'idleID': { 'type': 'string', 'value': expectedIdleID }
		};
		// Use sendMessage directly to avoid side effects from sendUnoCommand
		// (e.g. re-enabling following state)
		win.app.socket.sendMessage('uno .uno:ReportWhenIdle ' + JSON.stringify(idleArgs));
	});

	cy.wrap(null, { timeout: Cypress.config('defaultCommandTimeout') * 2.0 }).should(function() {
		var matchingCall = spy.getCalls().find(function(call) {
			var evt = call.args && call.args[0];
			var textMsg = evt && evt.textMsg;
			if (!textMsg || !textMsg.startsWith('unocommandresult:')) {
				return false;
			}
			var jsonPart = textMsg.replace('unocommandresult:', '').trim();
			var data = JSON.parse(jsonPart);
			if (data.commandName !== '.uno:ReportWhenIdle') {
				return false;
			}
			return data.idleID === expectedIdleID;
		});

		expect(matchingCall, '.uno:ReportWhenIdle result with idleID ' + expectedIdleID).to.be.an('object');
	});

	return cy.then(function() {
		// Only restore if we created our own spy
		if (ownSpy && spy && spy.restore) {
			spy.restore();
		}
	});
}

function waitUntilLayoutingIsIdle(win) {
	return cy.then(function() {
		return new Cypress.Promise(function(resolve) {
			win.app.layoutingService.onDrain(function() {
				resolve();
			});
		});
	});
}

// Wait for any pending ResizeObserver callbacks to fire.  After a UI
// mode switch the document-container div changes size via CSS; the
// ResizeObserver in CanvasTileLayer (which calls _syncTileContainerSize
// and then sectionContainer.onResize) will fire asynchronously, and
// we want to wait until the onResize has fired and sectionContainer
// is updated.
//
// Two requestAnimationFrame calls are needed because of the order
// within a single rendering frame:
//   1. Run requestAnimationFrame callbacks
//   2. Recalculate styles / layout
//   3. Run ResizeObserver callbacks
//   4. Paint
// The first RAF runs at step 1 - before the ResizeObserver callback
// at step 3.  From there it schedules a second RAF which runs at
// step 1 of the next frame, by which time step 3 of the previous
// frame has completed and sectionContainer.onResize has executed.
function waitForResizeObserver(win) {
	return cy.then(function() {
		return new Cypress.Promise(function(resolve) {
			win.requestAnimationFrame(function() {
				win.requestAnimationFrame(function() {
					resolve();
				});
			});
		});
	});
}

function processToIdle(win) {
	return waitUntilCoreIsIdle(win).then(function() {
		return waitForTimers(win, 'jsdialog-deferred');
	}).then(function() {
		return waitUntilLayoutingIsIdle(win);
	}).then(function() {
		return waitForResizeObserver(win);
	});
}

// Wait until no timers of the given tag exist.
// If no timers with the tag exist at call time resolve immediately.
function waitForTimers(win, tag) {
	return cy.waitUntil(function() {
		return !win.app.timerRegistry.hasActive(tag);
	}, { timeout: Cypress.config('defaultCommandTimeout'), interval: 50 });
}

// Waits for a map stateChangeHandler item to reach the expected value.
// Useful after sending uno commands where the state change message from
// core may arrive asynchronously based on a state change timer from core
function waitForMapState(command, expectedValue) {
	cy.log('>> waitForMapState - start');
	cy.log('Param - command: ' + command + ', expectedValue: ' + expectedValue);
	cy.getFrameWindow().should(win => {
		expect(win.app.map['stateChangeHandler'].getItemValue(command)).to.equal(expectedValue);
	});
	cy.log('<< waitForMapState - end');
}

// cy.realPress('Enter') via CDP bypasses preventDefault on keydown,
// which causes implicit form submission and spurious button clicks
// that don't happen with real keyboard input. This helper blocks
// those side effects for a single keypress.
function realPressInDialog(key) {
	cy.cGet('.jsdialog-window form').then($form => {
		// Block implicit form submission for this keypress
		function blockSubmit(e) {
			e.preventDefault();
			e.stopImmediatePropagation();
		}
		$form[0].addEventListener('submit', blockSubmit, true);

		// Block the close button from being clicked
		var closeBtn = $form[0].querySelector('.ui-dialog-titlebar-close');
		var origOnclick = closeBtn ? closeBtn.onclick : null;
		if (closeBtn) {
			closeBtn.onclick = function(e) {
				e.preventDefault();
				e.stopImmediatePropagation();
			};
		}

		cy.realPress(key).then(() => {
			$form[0].removeEventListener('submit', blockSubmit, true);
			if (closeBtn) closeBtn.onclick = origOnclick;
		});
	});
}

// Useful to get an item in the context-menu (currently only for document-area
// context menus). This context menu uses jsdialog dropdown as the
// implementation.
function getContextMenuItem(menuItemText) {
	return cy.cGet('#jsd-context-menu-dropdown-overlay')
		.contains('.ui-combobox-entry.jsdialog.ui-grid-cell span',
			menuItemText);
}

function getContextMenuItemList() {
	return cy.cGet('#jsd-context-menu-dropdown-overlay').find('.ui-combobox-entry.jsdialog.ui-grid-cell span');
}

module.exports.setupDocument = setupDocument;
module.exports.loadDocument = loadDocument;
module.exports.setupAndLoadDocument = setupAndLoadDocument;
module.exports.reloadDocument = reloadDocument;
module.exports.documentChecks = documentChecks;
module.exports.assertCursorAndFocus = assertCursorAndFocus;
module.exports.assertNoKeyboardInput = assertNoKeyboardInput;
module.exports.assertHaveKeyboardInput = assertHaveKeyboardInput;
module.exports.selectAllText = selectAllText;
module.exports.clearAllText = clearAllText;
module.exports.expectTextForClipboard = expectTextForClipboard;
module.exports.matchClipboardText = matchClipboardText;
module.exports.clipboardTextShouldBeDifferentThan = clipboardTextShouldBeDifferentThan;
module.exports.closeDocument = closeDocument;
module.exports.initAliasToNegative = initAliasToNegative;
module.exports.doIfInCalc = doIfInCalc;
module.exports.doIfInImpress = doIfInImpress;
module.exports.doIfInWriter = doIfInWriter;
module.exports.doIfNotInCalc = doIfNotInCalc;
module.exports.doIfNotInImpress = doIfNotInImpress;
module.exports.doIfNotInWriter = doIfNotInWriter;
module.exports.typeText = typeText;
module.exports.isImageWhite = isImageWhite;
module.exports.isCanvasWhite = isCanvasWhite;
module.exports.doIfOnMobile = doIfOnMobile;
module.exports.doIfOnDesktop = doIfOnDesktop;
module.exports.moveCursor = moveCursor;
module.exports.typeIntoDocument = typeIntoDocument;
module.exports.upLoadFileToNextCloud = upLoadFileToNextCloud;
module.exports.getCursorPos = getCursorPos;
module.exports.textSelectionShouldExist = textSelectionShouldExist;
module.exports.textSelectionShouldNotExist = textSelectionShouldNotExist;
module.exports.Bounds = Bounds;
module.exports.getItemBounds = getItemBounds;
module.exports.getOverlayItemBounds = getOverlayItemBounds;
module.exports.overlayItemHasBounds = overlayItemHasBounds;
module.exports.overlayItemHasDifferentBoundsThan = overlayItemHasDifferentBoundsThan;
module.exports.typeIntoInputField = typeIntoInputField;
module.exports.getVisibleBounds = getVisibleBounds;
module.exports.assertFocus = assertFocus;
module.exports.getBlinkingCursorPosition = getBlinkingCursorPosition;
module.exports.clickAt = clickAt;
module.exports.setDummyClipboardForCopy = setDummyClipboardForCopy;
module.exports.copy = copy;
module.exports.getFileName = getFileName;
module.exports.getSubFolder = getSubFolder;
module.exports.addressInputSelector = "#addressInput input";
module.exports.assertImageSize = assertImageSize;
module.exports.containsFocusElement = containsFocusElement;
module.exports.getMenuEntry = getMenuEntry;
module.exports.waitUntilCoreIsIdle = waitUntilCoreIsIdle;
module.exports.waitUntilLayoutingIsIdle = waitUntilLayoutingIsIdle;
module.exports.processToIdle = processToIdle;
module.exports.waitForTimers = waitForTimers;
module.exports.waitForMapState = waitForMapState;
module.exports.maxScreenshotableViewportHeight = maxScreenshotableViewportHeight;
module.exports.getContextMenuItem = getContextMenuItem;
module.exports.getContextMenuItemList = getContextMenuItemList;
module.exports.getMatchedCSSRules = getMatchedCSSRules;
module.exports.realPressInDialog = realPressInDialog;
module.exports.getShapeSVGCenter = getShapeSVGCenter;

// Find all author CSS rules that match an element, with the source
// stylesheet filename and the full selector for each matching rule.
// Usage:
//   helper.getMatchedCSSRules('.spinfieldbutton-up').then(function(output) {
//       console.error(output);
//   });
function getMatchedCSSRules(selector) {
	return cy.cGet(selector).first().then(function($el) {
		var el = $el[0];
		var doc = el.ownerDocument;
		var lines = [];

		for (var s = 0; s < doc.styleSheets.length; s++) {
			var sheet = doc.styleSheets[s];
			var href = sheet.href || 'inline';
			if (href !== 'inline') {
				var parts = href.split('/');
				href = parts[parts.length - 1];
			}

			var rules;
			try { rules = sheet.cssRules; } catch (e) { continue; }

			for (var r = 0; r < rules.length; r++) {
				var rule = rules[r];
				if (!rule.selectorText) continue;
				try {
					if (!el.matches(rule.selectorText)) continue;
				} catch (e) { continue; }

				var props = [];
				for (var p = 0; p < rule.style.length; p++) {
					var name = rule.style[p];
					var val = rule.style.getPropertyValue(name);
					var priority = rule.style.getPropertyPriority(name);
					props.push('  ' + name + ': ' + val + (priority ? ' !' + priority : ''));
				}
				lines.push(href + ' | ' + rule.selectorText);
				lines.push(props.join('\n'));
			}
		}
		return lines.join('\n');
	});
}
