/* -*- js-indent-level: 8 -*- */
/* global describe it cy require beforeEach expect */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');

describe(['tagdesktop', 'tagnextcloud', 'tagproxy'], 'Slide pane context menu', function() {

	beforeEach(function() {
		helper.setupAndLoadDocument('impress/slideshow.odp', false, false, undefined, 'vector=false');
		cy.getFrameWindow().then((win) => {
			this.win = win;
		});
	});

	function slideImage(position) {
		return cy.cGet('#slide-sorter .preview-frame:not(#first-drop-site) .preview-img')
			.eq(position);
	}

	function openSlideContextMenu(position) {
		slideImage(position).rightclick();
		cy.cGet('#slide-img-menu-dropdown').should('be.visible');
	}

	function clickContextMenuItem(text) {
		cy.cGet('#slide-img-menu-dropdown').contains('.ui-combobox-entry', text).click();
	}

	it('Layout submenu changes the layout of the slide', function() {
		openSlideContextMenu(0);
		clickContextMenuItem('Layout');

		cy.cGet('[id$="-dropdown"]:visible #slidelayoutgrid').should('be.visible');
		// Layout 18 is "Title, 4 Content".
		cy.cGet('[id$="-dropdown"]:visible #slidelayoutgrid [class*="WhatLayout:long=18"] button')
			.click();
		cy.cGet('#slide-img-menu-dropdown').should('not.exist');
		helper.processToIdle(this.win);

		cy.window().should(function() {
			var state = this.win.app.map['stateChangeHandler'].getItemValue('.uno:AssignLayout');
			expect(String(state)).to.equal('18');
		}.bind(this));
	});

	it('Layout submenu marks the layout of the current slide', function() {
		var layoutCell = function(layout) {
			return cy.cGet('[id$="-dropdown"]:visible #slidelayoutgrid [class*="WhatLayout:long=' + layout + '"]');
		};

		cy.window().should(function() {
			var state = this.win.app.map['stateChangeHandler'].getItemValue('.uno:AssignLayout');
			expect(state).to.match(/^\d+$/);
		}.bind(this)).then(function() {
			var layout = this.win.app.map['stateChangeHandler'].getItemValue('.uno:AssignLayout');
			openSlideContextMenu(0);
			clickContextMenuItem('Layout');
			layoutCell(layout).should('have.class', 'selected');
			cy.cGet('[id$="-dropdown"]:visible #slidelayoutgrid div.selected').should('have.length', 1);
		}.bind(this));

		// Layout 18 is "Title, 4 Content".
		layoutCell(18).find('button').click();
		cy.cGet('#slide-img-menu-dropdown').should('not.exist');
		helper.processToIdle(this.win);

		openSlideContextMenu(0);
		clickContextMenuItem('Layout');
		layoutCell(18).should('have.class', 'selected');
		cy.cGet('[id$="-dropdown"]:visible #slidelayoutgrid div.selected').should('have.length', 1);
	});

	it('Layout submenu stops listening for state changes once closed', function() {
		var listenerCount = function(win) {
			var evented = win.app.map._evented;
			var withoutContext = evented._eventsAuto.get('commandstatechanged');
			var withContext = evented._eventsExt.get('commandstatechanged');
			return (withoutContext ? withoutContext.length : 0) + (withContext ? withContext.size : 0);
		};
		var countWhileOpen;
		var cellCount;

		openSlideContextMenu(0);
		clickContextMenuItem('Layout');
		cy.cGet('[id$="-dropdown"]:visible #slidelayoutgrid .unotoolbutton').then(function(cells) {
			cellCount = cells.length;
		});
		helper.processToIdle(this.win);
		cy.window().then(function() {
			countWhileOpen = listenerCount(this.win);
			this.win.JSDialog.CloseAllDropdowns();
		}.bind(this));
		cy.cGet('#slidelayoutgrid').should('not.exist');

		// The buttons of the closed submenu notice they are gone at the next state change. Other
		// buttons that left the document go with them, so at least the cells of the grid go.
		cy.window().then(function() {
			this.win.app.map.fire('commandstatechanged', { commandName: '.uno:None', state: '' });
		}.bind(this));
		cy.window().should(function() {
			expect(cellCount).to.equal(16);
			expect(listenerCount(this.win)).to.be.at.most(countWhileOpen - cellCount);
		}.bind(this));
	});

	// The slide pane shows the masters as soon as the view reports the master view context, so
	// this returns then, before the state of the master view command arrives.
	function enterMasterView(win) {
		cy.then(function() {
			win.app.map.sendUnoCommand('.uno:SlideMasterPage');
		});
		cy.window().should(function() {
			expect(win.app.map.context.context).to.equal('MasterPage');
		});
	}

	it('Master view offers the master slide menu on every right-click', function() {
		// Right after the switch, the way a user right-clicks a master straight away.
		enterMasterView(this.win);

		for (var i = 0; i < 2; i++) {
			openSlideContextMenu(0);
			cy.cGet('#slide-img-menu-dropdown .ui-combobox-entry').then(function(items) {
				var texts = Array.from(items).map(function(item) { return item.textContent.trim(); });
				expect(texts).to.include.members(['New Master', 'Rename Master', 'Close Master View']);
				expect(texts).not.to.include('New Slide');
			});
			cy.then(function() {
				this.win.JSDialog.CloseAllDropdowns();
			}.bind(this));
			cy.cGet('#slide-img-menu-dropdown').should('not.exist');
		}
	});

	it('Master view menu adds a master with New Master', function() {
		enterMasterView(this.win);
		helper.processToIdle(this.win);
		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 1);

		openSlideContextMenu(0);
		clickContextMenuItem('New Master');
		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 2);
	});

	it('Master view menu pastes a master below the clicked master', function() {
		helper.setDummyClipboardForCopy();
		enterMasterView(this.win);
		helper.processToIdle(this.win);
		openSlideContextMenu(0);
		clickContextMenuItem('New Master');
		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 2);
		var secondMaster;
		cy.then(function() {
			secondMaster = this.win.app.impress.partList[1].name;
			expect(secondMaster).to.be.a('string').and.not.be.empty;
		}.bind(this));

		openSlideContextMenu(0);
		clickContextMenuItem('Copy');
		// The copy lands on the dummy clipboard, and the paste reads it back from there.
		cy.cGet('#copy-paste-container').should(function(container) {
			expect(container[0].innerHTML).to.contain('meta-origin');
		}).then(function(container) {
			helper.setDummyClipboardForPaste(container[0].innerHTML, '');
		});

		openSlideContextMenu(0);
		clickContextMenuItem('Paste');
		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 3);
		cy.window().should(function() {
			expect(this.win.app.impress.partList[2].name).to.equal(secondMaster);
		}.bind(this));

		// The paste added a master and no slide.
		cy.then(function() {
			this.win.app.map.sendUnoCommand('.uno:CloseMasterView');
		}.bind(this));
		cy.window().should(function() {
			expect(this.win.app.map.context.context).to.equal('DrawPage');
		}.bind(this));
		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 1);
	});

	it('Start Slideshow from This Slide presents from the clicked slide', function() {
		openSlideContextMenu(0);
		clickContextMenuItem('New Slide');
		impressHelper.assertSlidePreviewCountAfterIdle(this.win, 2);

		// Select both slides with the first one current, so the clicked slide is a
		// selected slide that is not the current one.
		slideImage(0).click();
		slideImage(1).click({ ctrlKey: true });
		helper.processToIdle(this.win);
		cy.window().should(function() {
			expect(this.win.app.map.getCurrentPartNumber()).to.equal(0);
			expect(this.win.app.impress.isSlideSelected(1)).to.equal(true);
		}.bind(this));

		openSlideContextMenu(1);
		clickContextMenuItem('Start Slideshow from This Slide');

		cy.cGet('#slideshow-cypress-iframe').should('be.visible');
		impressHelper.waitForSlideShowIdle(this.win);
		cy.window().should(function() {
			var navigator = this.win.app.map.slideShowPresenter._slideShowNavigator;
			expect(navigator.currentSlideIndex).to.equal(1);
		}.bind(this));
	});
});
