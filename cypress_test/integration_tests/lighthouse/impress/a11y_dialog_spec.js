/* global describe expect it cy before after afterEach require */

var helper = require('../../common/helper');
var desktopHelper = require('../../common/desktop_helper');
var impressHelper = require('../../common/impress_helper');
var a11yHelper = require('../../common/a11y_helper');

const allImpressDialogs = [
    '.uno:HeaderAndFooter',
    '.uno:InsertTable',
    '.uno:PageSetup',
    '.uno:TableDialog',
];

// 'common' dialogs that impress specifically does not support
const excludedCommonDialogs = [
    '.uno:AcceptTrackedChanges',
    // Not exercised in this ODF a11y run: the Security Label rides in the OOXML
    // customXml part, so the command is not meaningful on these fixtures.
    '.uno:SecurityLabel',
    '.uno:SpellingAndGrammarDialog', // does not open in impress, SpellDialog is the equivalent
    '.uno:StyleNewByExample', // command dispatches but does not surface a dialog in impress
];

describe(['taglighthouse'], 'Accessibility Impress Dialog Tests', { testIsolation: false }, function () {
    let win;
    let hasLinguisticData = false;

    before(function () {
        helper.setupAndLoadDocument('impress/help_dialog.odp', /*isMultiUser=*/false, /*copyCertificates=*/true);

        // to make insertImage use the correct buttons
        desktopHelper.switchUIToNotebookbar();

        helper.setDummyClipboardForCopy();

        cy.getFrameWindow().then(function (frameWindow) {
            win = frameWindow;
            a11yHelper.enableUICoverage(win);

            // a copy of a table selection falls back and offers this notice
            win.JSDialog.setShowAgain('copy_paste_warning', false);
        });

        cy.cGet('.jsdialog-window').should('not.exist');

        cy.then(() => {
            return helper.processToIdle(win);
        }).then(() => {
            // help_dialog.odp has a table on slide 1. Click into it so the
            // default context is a text insertion point - lots of common
            // dialogs (InsertSymbol, ThesaurusDialog etc.) only work that way.
            impressHelper.selectTableInTheCenter(win);
        });

        cy.then(() => {
            const thesaurusState = win.app.map.stateChangeHandler.getItemValue('.uno:ThesaurusDialog');
            hasLinguisticData = (thesaurusState === 'enabled');
        });
    });

    after(function () {
        cy.then(() => {
            // leave the clipboard notice on for whatever runs next
            win.prefs.remove('UIShowAgain_copy_paste_warning');
        });

        a11yHelper.reportUICoverage(win, hasLinguisticData);

        cy.get('@uicoverageResult').then(result => {
            expect(result.used, `used .ui files`).to.not.be.empty;
            expect(result.CompleteImpressDialogCoverage,
                `complete impress dialog coverage; missing: ${JSON.stringify(result.MissingImpressDialogCoverage)}`).to.be.true;
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
        cy.cGet('.jsdialog-window:not(.ui-overflow-group-popup):not(.snackbar)').should('not.exist');

        a11yHelper.resetState();

        // drop any lingering text-edit or shape selection so the click
        // sequence in selectTableInTheCenter starts from slide level.
        helper.typeIntoDocument('{esc}{esc}');

        // re-establish the cursor in the first table cell after a test that
        // moved focus elsewhere (selecting a shape, escaping etc.)
        impressHelper.selectTableInTheCenter(win);
    });

    a11yHelper.allCommonDialogs.forEach(function (commandSpec) {
        const command = typeof commandSpec === 'string' ? commandSpec : commandSpec.command;
        if (excludedCommonDialogs.includes(command)) {
            // silently skip the common dialogs that writer doesn't have
            return;
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

    it('Transform dialog', function () {
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:BasicShapes.octagon');
        });

        cy.cGet('#test-div-shapeHandlesSection').should('exist');

        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:TransformDialog');
        });
        a11yHelper.handleDialog(win, 1, '.uno:TransformDialog');

        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:FormatArea');
        });
        a11yHelper.handleDialog(win, 1, '.uno:FormatArea');

        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:FormatLine');
        });
        a11yHelper.handleDialog(win, 1, '.uno:FormatLine');

        // exit shape mode
        helper.typeIntoDocument('{esc}');
    });

    it('PasteSpecial Dialog', function () {
        // The table cell holding the caret is empty, so there is nothing to select.
        helper.typeIntoDocument('Lorem ipsum');

        helper.selectAllText();

        // Copying an empty selection raises the limited-clipboard modal, whose overlay
        // then covers the dialog.
        cy.wrap(null).should(() => {
            expect(win.app.map._clip._selectionType, 'clipboard selection type').to.equal('text');
        });

        helper.copy().then(() => {
            return helper.processToIdle(win);
        })
        .then(() => {
            win.app.map.sendUnoCommand('.uno:PasteSpecial');
        });
        a11yHelper.handleDialog(win, 1);
    });

    it('PDF export warning dialog', function () {
        a11yHelper.testPDFExportWarningDialog(win);
    });

    // TODO - create allImpressDialogs
    allImpressDialogs.forEach(function (commandSpec) {
        const command = typeof commandSpec === 'string' ? commandSpec : commandSpec.command;
        it(`Impress Dialog ${command}`, function () {
            a11yHelper.testDialog(win, commandSpec);
        });
    });

    it('Graphic dialog', function () {
        helper.typeIntoDocument('{esc}{esc}');
        helper.processToIdle(win);
        desktopHelper.insertImage();
        cy.cGet('#test-div-shapeHandlesSection').should('exist');
        helper.processToIdle(win);
        a11yHelper.testDialog(win, '.uno:CompressGraphic');
        // remove the inserted image so subsequent tests can reach the table
        helper.typeIntoDocument('{del}');
    });

    it('Hatch Add name dialog and duplicate warning', function () {
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
                cy.cGet('.ui-dialog [role="tab"]').then($tabs => {
                    var areaTab = $tabs.filter((_i, el) => {
                        const label = (el.getAttribute('aria-label') || el.textContent || '').trim();
                        return el.getAttribute('aria-controls') === 'RID_SVXPAGE_AREA' || label === 'Area';
                    }).get(0);
                    expect(areaTab, 'Area tab').to.not.be.undefined;
                    areaTab.click();
                });
                return helper.processToIdle(win);
            })
            .then(() => {
                cy.cGet('#moreeffects-button').click();
                return a11yHelper.getActiveDialog(2);
            })
            .then(() => helper.processToIdle(win))
            .then(() => {
                cy.cGet('.ui-dialog [role="tab"][aria-controls="hatch"]').click();
                return helper.processToIdle(win);
            })
            .then(() => {
                cy.cGet('button.ui-pushbutton[aria-label="Add"]:visible').click();
                return helper.processToIdle(win);
            });

        a11yHelper.testNameDialog(win, 2);

        a11yHelper.closeActiveDialog(2);
        a11yHelper.closeActiveDialog(1);
        helper.typeIntoDocument('{esc}');
    });

    it('Paragraph dialog', function () {
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:ParagraphDialog');
        });
        a11yHelper.handleDialog(win, 1, '.uno:ParagraphDialog');
    });

    it('Bullets and Numbering dialog', function () {
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:OutlineBullet');
        });
        a11yHelper.handleDialog(win, 1, '.uno:OutlineBullet');
    });

    it('Font dialog', function () {
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:FontDialog');
        });
        a11yHelper.handleDialog(win, 1, '.uno:FontDialog');
    });

    it('Object dialog', function () {
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:BasicShapes.octagon');
        });
        cy.cGet('#test-div-shapeHandlesSection').should('exist');

        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:NameGroup');
        });
        a11yHelper.handleDialog(win, 1, '.uno:NameGroup');

        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:ObjectTitleDescription');
        });
        a11yHelper.handleDialog(win, 1, '.uno:ObjectTitleDescription');

        helper.typeIntoDocument('{esc}');
    });

    it('Dimension line dialog', function () {
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:MeasureLine');
        });
        cy.cGet('#test-div-shapeHandlesSection').should('exist');

        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:MeasureAttributes');
        });
        a11yHelper.handleDialog(win, 1, '.uno:MeasureAttributes');

        helper.typeIntoDocument('{esc}');
    });

    it('Duplicate dialog', function () {
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:BasicShapes.octagon');
        });
        cy.cGet('#test-div-shapeHandlesSection').should('exist');

        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:CopyObjects');
        });
        a11yHelper.handleDialog(win, 1, '.uno:CopyObjects');

        helper.typeIntoDocument('{esc}');
    });

    it('Callout dialog', function () {
        // a legacy callout is created with its text edit running; ending an edit without text
        // deletes it, so it gets a word before Escape leaves it selected
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:DrawCaption');
        });
        cy.cGet('#test-div-shapeHandlesSection').should('exist');
        helper.typeIntoDocument('Callout{esc}');
        // the typed keys are handled after a command sent now would be
        cy.then(() => helper.processToIdle(win));
        cy.cGet('#test-div-shapeHandlesSection').should('exist');

        // Position and Size opens the callout dialog for a legacy callout
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:TransformDialog');
        });
        a11yHelper.handleDialog(win, 1, '.uno:TransformDialog');

        helper.typeIntoDocument('{esc}');
    });

    it('Connector dialog', function () {
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:Connector');
        });
        cy.cGet('#test-div-shapeHandlesSection').should('exist');

        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:ConnectorAttributes');
        });
        a11yHelper.handleDialog(win, 1, '.uno:ConnectorAttributes');

        helper.typeIntoDocument('{esc}');
    });

    it('Interaction dialog', function () {
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:BasicShapes.octagon');
        });
        cy.cGet('#test-div-shapeHandlesSection').should('exist');

        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:AnimationEffects');
        });
        a11yHelper.handleDialog(win, 1, '.uno:AnimationEffects');

        helper.typeIntoDocument('{esc}');
    });

    it('Extrusion Depth dialog', function () {
        a11yHelper.testExtrusionDepthDialog(win);
    });

    it('Custom Animation dialog', function () {
        // afterEach left us inside a table cell on slide 1. Escape out so
        // BasicShapes.octagon adds a shape at slide level rather than
        // inside the table.
        helper.typeIntoDocument('{esc}{esc}{esc}');

        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:BasicShapes.octagon');
        });
        cy.cGet('#test-div-shapeHandlesSection').should('exist');
        cy.then(() => helper.processToIdle(win));

        // Enter the shape and add some text so the dialog adds its
        // text-animation tab (customanimationtexttab.ui), then escape back
        // to shape selection so Add Effect remains applicable.
        helper.typeIntoDocument('{enter}');
        helper.typeIntoDocument('text');
        helper.typeIntoDocument('{esc}');
        cy.cGet('#test-div-shapeHandlesSection').should('exist');
        cy.then(() => helper.processToIdle(win));

        // Open the Custom Animation sidebar deck. .uno:CustomAnimation
        // is the SidebarController binding for SdCustomAnimationDeck.
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:CustomAnimation');
        });
        cy.cGet('#sidebar-dock-wrapper').should('be.visible');
        cy.cGet('#add_effect').should('be.visible');
        cy.then(() => helper.processToIdle(win));

        function clickSidebarPushButton(id) {
            cy.cGet('#' + id).scrollIntoView().should('be.visible');
            cy.cGet('#' + id + ' button').should('not.be.disabled').click();
        }

        clickSidebarPushButton('add_effect');
        cy.then(() => helper.processToIdle(win));

        clickSidebarPushButton('more_properties');
        a11yHelper.handleDialog(win, 1, '.uno:CustomAnimationDialog');

        // restore the default impress sidebar deck and drop the shape selection
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:ModifyPage');
        });
        helper.typeIntoDocument('{esc}');
    });

    // form controls are created and selected only in design mode, and the document opens its
    // forms in alive mode
    function switchControlDesignMode(on) {
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:SwitchControlDesignMode', {
                SwitchControlDesignMode: { type: 'boolean', value: on }
            });
        });
    }

    // the ids of the objects the client knows as selected
    function selectedObjectIds() {
        return win.app.definitions.graphicSelection.selectedObjectIDs;
    }

    // a form control of the given kind in the middle of the slide, selected
    function insertFormControl(command) {
        // nothing is selected, so the object selected next is the new control; a form control
        // gets no handles in the tiled view, so the selection itself is what we wait for
        helper.typeIntoDocument('{esc}{esc}');
        cy.wrap(null).should(() => {
            expect(selectedObjectIds(), 'selected objects').to.have.length(0);
        });
        cy.then(() => {
            // Ctrl creates the control at once, at its default size
            win.app.map.sendUnoCommand(command, {
                KeyModifier: { type: 'unsigned short', value: 8192 }
            });
        });
        cy.wrap(null).should(() => {
            expect(selectedObjectIds(), 'selected objects').to.have.length(1);
        });
        cy.then(() => {
            win.app.map.sendUnoCommand('.uno:ControlProperties');
        });
        a11yHelper.getActiveDialog(1);
        // the dialog opens on the tab used last, the lines below are on General
        selectPropertyTab('General');
    }

    // The property dialog keeps rebuilding its tabs and lines for a while after it opens, after a
    // tab is switched and after one of its own dialogs closes. Each lookup waits until that is
    // done, so the element it finds is the one that stays on the page.
    function selectPropertyTab(name) {
        cy.then(() => helper.processToIdle(win));
        cy.cGet('#FormPropertyDialog .ui-tab').contains(name).click();
        cy.then(() => helper.processToIdle(win));
    }

    // the property line with that title
    function propertyLine(title) {
        cy.then(() => helper.processToIdle(win));
        return cy.cGet('#FormPropertyDialog [id^="BrowserLine"]')
            .filter(function (index, line) {
                const label = line.firstElementChild;
                return label && label.textContent.replace(/\.+$/, '').trim() === title;
            });
    }

    // the button at the end of the property line with that title, which opens its dialog
    function clickPropertyLineButton(title) {
        cy.then(() => helper.processToIdle(win));
        propertyLine(title)
            .find('[id^="browse"]:not(.hidden) button')
            .first()
            .scrollIntoView()
            .click();
    }

    // the entries of a list box, typed into the editor of several lines behind the arrow; a list
    // without entries offers no default selection
    function setListEntries(entries) {
        propertyLine('List entries').find('.menubutton').first().scrollIntoView().click();
        cy.cGet('.jsdialog-window.modalpopup textarea#textview').type(entries.join('{enter}'));
        cy.cGet('.jsdialog-window.modalpopup #ok-button').click();
        cy.cGet('.jsdialog-window.modalpopup').should('not.exist');
    }

    it('Form control properties dialogs', function () {
        switchControlDesignMode(true);
        insertFormControl('.uno:Pushbutton');
        clickPropertyLineButton('Font');
        a11yHelper.handleDialog(win, 2);
        clickPropertyLineButton('Tab order');
        a11yHelper.handleDialog(win, 2);
        selectPropertyTab('Events');
        clickPropertyLineButton('Execute action');
        a11yHelper.handleDialog(win, 2);
        a11yHelper.handleDialog(win, 1, '.uno:ControlProperties');

        insertFormControl('.uno:ListBox');
        setListEntries(['One', 'Two']);
        clickPropertyLineButton('Label Field');
        a11yHelper.handleDialog(win, 2);
        clickPropertyLineButton('Default selection');
        a11yHelper.handleDialog(win, 2);
        a11yHelper.handleDialog(win, 1, '.uno:ControlProperties');

        // these show the fields for dates, times and formatted numbers in their lines
        insertFormControl('.uno:DateField');
        a11yHelper.handleDialog(win, 1, '.uno:ControlProperties');
        insertFormControl('.uno:TimeField');
        a11yHelper.handleDialog(win, 1, '.uno:ControlProperties');
        insertFormControl('.uno:FormattedField');
        a11yHelper.handleDialog(win, 1, '.uno:ControlProperties');

        helper.typeIntoDocument('{esc}');
        switchControlDesignMode(false);
    });

});
