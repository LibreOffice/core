/* global describe it require cy beforeEach Cypress */
import "@cypress-audit/lighthouse/commands";

var helper = require('./helper');

function runLighthouseAccessibilityTest(docType, filePath) {
    describe(['taglighthouse'], `${docType} Lighthouse Accessibility Tests`, function () {

        beforeEach(function() {
            var newFilePath = helper.setupDocument(filePath);
            helper.loadDocument(newFilePath, /*skipDocumentChecks*/ true, false, undefined, 'vector=false');
            helper.documentChecks(/*skipInitializedCheck*/ true);
        });

        it('lighthouse accessibility score', function () {
            // Fail the test early if not running in chrome
            if (!Cypress.browser || Cypress.browser.name !== 'chrome') {
                throw new Error(`Unsupported browser: ${Cypress.browser.name}. Lighthouse only works with Chrome.`);
            }

            cy.lighthouse({
                accessibility: 100,
            });
        });
    });
}

module.exports.runLighthouseAccessibilityTest = runLighthouseAccessibilityTest;