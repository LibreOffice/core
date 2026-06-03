/* global describe expect it cy Cypress before after afterEach require */

var helper = require('../../common/helper');
var calcHelper = require('../../common/calc_helper');
var desktopHelper = require('../../common/desktop_helper');
var a11yHelper = require('../../common/a11y_helper');

const allCalcDialogs = [
    '.uno:AddName',
    '.uno:AnalysisOfVarianceDialog',
    '.uno:ChiSquareTestDialog',
    '.uno:ColumnWidth',
    '.uno:ColorScaleFormatDialog',
    '.uno:ConditionalFormatDialog',
    '.uno:ConditionalFormatEasy?FormatRule:short=0',
    '.uno:ConditionalFormatManagerDialog',
    '.uno:DataBarFormatDialog',
    '.uno:IconSetFormatDialog',
    '.uno:CorrelationDialog',
    '.uno:CovarianceDialog',
    '.uno:DataSort',
    '.uno:DataFilterSpecialFilter',
    '.uno:DataFilterStandardFilter',
    '.uno:DefineName',
    '.uno:DefineDBName',
    '.uno:DeleteCell',
    '.uno:Delete',
    '.uno:DescriptiveStatisticsDialog',
    '.uno:EditHeaderAndFooter',
    '.uno:EditPrintArea',
    '.uno:EditStyle?Param:string=Heading&Family:short=2',
    '.uno:ExponentialSmoothingDialog',
    '.uno:FormatCellDialog',
    '.uno:FourierAnalysisDialog',
    '.uno:FunctionDialog',
    '.uno:FTestDialog',
    '.uno:GoalSeekDialog',
    '.uno:Group',
    '.uno:InsertCell',
    '.uno:InsertObjectChart',
    '.uno:InsertSparkline',
    '.uno:JumpToTable',
    '.uno:Move?FromContextMenu:bool=true&MoveOrCopySheetDialog:bool=true&ContextMenuIndex=0',
    '.uno:MovingAverageDialog',
    '.uno:PageFormatDialog',
    '.uno:Protect',
    '.uno:RegressionDialog',
    '.uno:RowHeight',
    '.uno:SamplingDialog',
    '.uno:SelectDB',
    '.uno:SelectSheetView',
    '.uno:SetOptimalColumnWidth',
    '.uno:SetOptimalRowHeight',
    '.uno:SolverDialog',
    '.uno:TTestDialog',
    '.uno:Validation',
    '.uno:ZTestDialog',
];

// 'common' dialogs that calc specifically does not support
const excludedCommonDialogs = [
    '.uno:AcceptTrackedChanges',
    '.uno:GotoPage',
    '.uno:SpellingAndGrammarDialog',
    '.uno:SplitCell',
];

// don't pass yet
const buggyCalcDialogs = [
];

describe(['tagdesktop'], 'Accessibility Calc Dialog Tests', { testIsolation: false }, function () {
    let win;
    let hasLinguisticData = false;

    before(function () {
        helper.setupAndLoadDocument('calc/help_dialog.ods', /*isMultiUser=*/false, /*copyCertificates=*/true);

        // to make insertImage use the correct buttons
        desktopHelper.switchUIToNotebookbar();

        helper.setDummyClipboardForCopy();

        cy.getFrameWindow().then(function (frameWindow) {
            win = frameWindow;
            a11yHelper.enableUICoverage(win);
        });

        cy.cGet('.jsdialog-window').should('not.exist');


        cy.then(() => {
            // Go to a cell with text to enable thesaurus (if there is linguistic data available).
            calcHelper.enterCellAddressAndConfirm(win, 'C5');
        }).then(() => {
            const thesaurusState = win.app.map.stateChangeHandler.getItemValue('.uno:ThesaurusDialog');
            hasLinguisticData = (thesaurusState === 'enabled');
        });
    });

    after(function () {
        a11yHelper.reportUICoverage(win, hasLinguisticData);

        cy.get('@uicoverageResult').then(result => {
            expect(result.used, `used .ui files`).to.not.be.empty;
            expect(result.CompleteCalcDialogCoverage,
                `complete calc dialog coverage; missing: ${JSON.stringify(result.MissingCalcDialogCoverage)}`).to.be.true;
            expect(result.CompleteCommonDialogCoverage,
                `complete common dialog coverage; missing: ${JSON.stringify(result.MissingCommonDialogCoverage)}`).to.be.true;
        });
    });

    afterEach(function () {
        // Close any dialogs that might still be open after a test failure
        cy.cGet('body').then($body => {
            const dialogs = $body.find('.jsdialog-window .ui-dialog-titlebar-close');
            if (dialogs.length > 0) {
                // Close dialogs from innermost to outermost
                for (let i = dialogs.length - 1; i >= 0; i--) {
                    cy.wrap(dialogs[i]).click({ force: true });
                }
            }
        });
        cy.cGet('.jsdialog-window:not(.ui-overflow-group-popup)').should('not.exist');

        a11yHelper.resetState();

        // make C5 the home cell for all tests
        calcHelper.enterCellAddressAndConfirm(win, 'C5');
    });

    a11yHelper.allCommonDialogs.forEach(function (commandSpec) {
        const command = typeof commandSpec === 'string' ? commandSpec : commandSpec.command;
        if (excludedCommonDialogs.includes(command)) {
            // silently skip the common dialogs that calc doesn't have
            return;
        } else if (buggyCalcDialogs.includes(command)) {
            it.skip(`Dialog ${command} (buggy)`, function () {});
        } else {
            it(`Common Dialog ${command}`, function () {
                if (!hasLinguisticData && a11yHelper.needsLinguisticData(command)) {
                    this._runnable.title += ' (skipped: missing linguistic data)';
                    this.skip();
                }
                a11yHelper.testDialog(win, commandSpec);
            });
        }
    });

    allCalcDialogs.forEach(function (commandSpec) {
        const command = typeof commandSpec === 'string' ? commandSpec : commandSpec.command;
        if (buggyCalcDialogs.includes(command)) {
            it.skip(`Dialog ${command} (buggy)`, function () {});
        } else {
            it(`Calc Dialog ${command}`, function () {
                a11yHelper.testDialog(win, commandSpec);
            });
        }
    });

    it('Graphic dialog', function () {
        cy.viewport(1920,1080);
        helper.processToIdle(win);

        desktopHelper.insertImage('calc');

        a11yHelper.testDialog(win, '.uno:CompressGraphic');
        a11yHelper.testDialog(win, '.uno:TransformDialog');
        a11yHelper.testDialog(win, '.uno:FormatArea');
        a11yHelper.testDialog(win, '.uno:FormatLine');

        // exit shape mode
        helper.typeIntoDocument('{esc}');

        cy.viewport(Cypress.config('viewportWidth'), Cypress.config('viewportHeight'));
        desktopHelper.selectZoomLevel('100', false);

        // exit shape mode
        helper.typeIntoDocument('{esc}');

        cy.viewport(Cypress.config('viewportWidth'), Cypress.config('viewportHeight'));
        desktopHelper.selectZoomLevel('100', false);
    });

    it('Shape paragraph dialog', function () {
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:BasicShapes.octagon');
        });

        cy.cGet('#test-div-shapeHandlesSection').should('exist');

        helper.typeIntoDocument('{enter}');

        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:ParagraphDialog');
        });
        a11yHelper.handleDialog(win, 1);

        // exit shape mode
        helper.typeIntoDocument('{esc}');
    });

    it('Sparkline data range dialog', function () {
        cy.cGet('#spreadsheet-tab4').click();
        helper.typeIntoInputField(helper.addressInputSelector, 'B1');
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:EditSparkline');
        });
        a11yHelper.handleDialog(win, 1, '.uno:EditSparkline');
        cy.cGet('#spreadsheet-tab0').click();
    });

    it('Hatch Add name dialog and duplicate warning', function () {
        // The shared traverseTabs lbhatch handler covers this for writer but
        // doesn't fire here, so trigger the cui/ui/namedialog.ui +
        // cui/ui/queryduplicatedialog.ui path explicitly.
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:BasicShapes.octagon');
        });
        cy.cGet('#test-div-shapeHandlesSection').should('exist');

        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:FormatArea');
        });

        a11yHelper.getActiveDialog(1)
            .then(() => helper.processToIdle(win))
            .then(() => {
                // Click the inner Hatch sub-tab in the Area tab page. The
                // rendered .ui-tabs container is visibility:hidden until the
                // outer Area tab is selected, so click the DOM node directly.
                cy.cGet('.ui-dialog [role="tab"]').then($tabs => {
                    let areaTab = null;
                    let hatchTab = null;
                    $tabs.each((_i, el) => {
                        const controls = el.getAttribute('aria-controls') || '';
                        const label = (el.getAttribute('aria-label') || el.textContent || '').trim();
                        if (controls === 'RID_SVXPAGE_AREA' || (!areaTab && label === 'Area' && controls !== 'lbhatch'))
                            areaTab = el;
                        if (controls === 'lbhatch')
                            hatchTab = el;
                    });
                    expect(areaTab, 'outer Area tab').to.not.be.null;
                    expect(hatchTab, 'inner Hatch tab').to.not.be.null;
                    areaTab.click();
                    hatchTab.click();
                });
                return helper.processToIdle(win);
            })
            .then(() => {
                cy.cGet('button.ui-pushbutton[aria-label="Add"]:visible').click();
                return helper.processToIdle(win);
            });

        // First Add: accept the default suggested name "Hatching 1".
        a11yHelper.getActiveDialog(2)
            .then(() => helper.processToIdle(win))
            .then(() => {
                a11yHelper.runA11yValidation(win, 'validatedialogsa11y');
                cy.cGet('[role="dialog"][aria-labelledby*="Name"] #ok-button')
                    .should('be.enabled').click();
                return helper.processToIdle(win);
            });

        // Add again with a duplicate name to force the warning subdialog.
        cy.cGet('button.ui-pushbutton[aria-label="Add"]:visible').click();
        a11yHelper.getActiveDialog(2)
            .then(() => helper.processToIdle(win))
            .then(() => {
                cy.cGet('#name_entry-input').type('{selectall}{backspace}Hatching 1');
                cy.cGet('[role="dialog"][aria-labelledby*="Name"] #ok-button')
                    .should('be.enabled').click();
                return helper.processToIdle(win);
            });

        // queryduplicatedialog replaces namedialog at the same level.
        a11yHelper.handleDialog(win, 2);

        a11yHelper.closeActiveDialog(1);
        helper.typeIntoDocument('{esc}');
    });

    it('PasteSpecial Dialog', function () {
        helper.setDummyClipboardForCopy('text/html');
        // Select some text
        helper.selectAllText();

        helper.copy().then(() => {
            return helper.processToIdle(win);
        })
        .then(() => {
            win.app.map.sendUnoCommand('.uno:PasteSpecial');
        });
        a11yHelper.handleDialog(win, 1);
    });

    it('Text Import Dialog', function () {
        helper.setDummyClipboardForCopy('text/plain');
        // Select some text
        helper.selectAllText();

        helper.copy().then(() => {
            return helper.processToIdle(win);
        })
        .then(() => {
            win.app.map.sendUnoCommand('.uno:PasteTextImportDialog');
        });
        a11yHelper.handleDialog(win, 1, '.uno:PasteTextImportDialog');
    });

    it('Font Dialog', function () {
        calcHelper.dblClickOnFirstCell();

        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:FontDialog');
        });

        a11yHelper.handleDialog(win, 1, ".uno:FontDialog");

        helper.typeIntoDocument('{esc}');
    });

    it('Merge Cells Dialog', function () {
        helper.typeIntoInputField(helper.addressInputSelector, 'A1:A3')

        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:MergeCells');
        });

        a11yHelper.handleDialog(win, 1, ".uno:MergeCells");
    });

    it('Select Source Dialog', function () {
        helper.typeIntoInputField(helper.addressInputSelector, 'A1:A3')
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:DataDataPilotRun');
        });
        // This is just the 'select source' dialog, not the pivot table dialog
        a11yHelper.handleDialog(win, 1);
    });

    it('Pivot Table Dialog', function () {
        cy.cGet('#spreadsheet-tab3').click();
        helper.typeIntoInputField(helper.addressInputSelector, 'A1:B1')
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:DataDataPilotRun');
        });
        a11yHelper.handleDialog(win, 1, ".uno:DataDataPilotRun");
        cy.cGet('#spreadsheet-tab0').click();
    });

    it('Pivot Calculated Field Dialog', function () {
        cy.cGet('#spreadsheet-tab3').click();
        helper.typeIntoInputField(helper.addressInputSelector, 'A1');
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:CalculatedFieldRun');
        });
        a11yHelper.handleDialog(win, 1);
        cy.cGet('#spreadsheet-tab0').click();
    });

    it('AutoCorrect Warning Dialog', function () {
        helper.typeIntoDocument('=2x3{enter}');
        a11yHelper.handleDialog(win, 1, '', true);
    });

    it('PDF export warning dialog', function () {
        cy.cGet('#spreadsheet-tab0').click();
        a11yHelper.testPDFExportWarningDialog(win);
    });

    it('Settings dialog', function () {
        cy.then(() => {
            win.app.map.settings.showSettingsDialog();
        });

        cy.cGet('.iframe-settings-wrap').should('be.visible').then(() => {
            var spy = Cypress.sinon.spy(win.console, 'error');
            var container = win.document.querySelector('.iframe-settings-wrap');
            win.app.a11yValidator.validateIframeDialog(container);
            a11yHelper.checkA11yErrors(win, spy);
            spy.restore();
        });

        // Close the settings dialog
        cy.cGet('.iframe-settings-wrap .ui-dialog-titlebar-close').click();
        cy.cGet('.iframe-settings-wrap').should('not.exist');
    });
});
