/* global describe it cy require beforeEach */

var helper = require('../../common/helper');
var impressHelper = require('../../common/impress_helper');
var { getSlideShow, getSlideShowContent, getSlideShowCanvas } = require('../../common/impress_helper');

describe(['tagmultiuser'], 'Follow me slide show', function() {
    var win1, win2;

    beforeEach(function() {
            helper.setupAndLoadDocument('impress/follow.odp',true);

            cy.getFrameWindow('#iframe1').then((win) => { win1 = win; });
            cy.getFrameWindow('#iframe2').then((win) => { win2 = win; });
	    cy.viewport(2000, helper.maxScreenshotableViewportHeight);

            cy.cSetActiveFrame('#iframe1');
            cy.cGet('.notebookbar #Slideshow-tab-label').click();

            cy.cSetActiveFrame('#iframe2');
            cy.cGet('.notebookbar #Slideshow-tab-label').click();
        });

    it('Start Follow Me Slideshow', function () {
        cy.cSetActiveFrame('#iframe1');

        cy.cGet('.notebookbar #slide-presentation-follow').should('be.not.visible');
        cy.cGet('.notebookbar #slide-presentation-follow-me').should('be.visible');
        cy.cGet('.notebookbar #slide-presentation-follow-me').click();
        getSlideShow().should('be.visible');

        cy.cSetActiveFrame('#iframe2');
        getSlideShow().should('be.visible');
        impressHelper.clickSlideShowNav(win2, '#endshow');
        getSlideShow().should('not.exist');
        cy.cGet('.notebookbar #slide-presentation-follow-me').should('be.not.visible');
        cy.cGet('.notebookbar #slide-presentation-follow').should('be.visible');
    });

    it('Go to next effect', function () {
        cy.cSetActiveFrame('#iframe2');
        cy.cGet('.notebookbar #slide-presentation-follow-me').click();
        getSlideShow().should('be.visible');
        impressHelper.waitForSlideShowIdle(win2);
        getSlideShowContent().find('#slideshow-canvas').click();
        impressHelper.waitForSlideShowIdle(win2);

        cy.cSetActiveFrame('#iframe1');
        getSlideShow().should('be.visible');
        impressHelper.waitForSlideShowIdle(win1);
        getSlideShowCanvas().compareSnapshot('effect1', 0.05);
    });

    it('Go to previous effect and slide', function () {
        cy.cSetActiveFrame('#iframe2');
        cy.cGet('.notebookbar #slide-presentation-follow-me').click();
        getSlideShow().should('be.visible');
        impressHelper.waitForSlideShowIdle(win2);
        impressHelper.waitForSlideShowIdle(win1);

        //move to nextslide last effect
        for (let i = 0; i < 5; i++) {
            impressHelper.clickSlideShowNav(win2, '#next');
            impressHelper.waitForSlideShowIdle(win2);
            impressHelper.waitForSlideShowIdle(win1);
        }
        getSlideShowCanvas().compareSnapshot('slide2_effect3', 0.05);

        cy.cSetActiveFrame('#iframe1');
        getSlideShow().should('be.visible');
        impressHelper.waitForSlideShowIdle(win1);
        getSlideShowCanvas().compareSnapshot('slide2_effect3', 0.05);

        // go to previous effect
        cy.cSetActiveFrame('#iframe2');
        impressHelper.clickSlideShowNav(win2, '#previous');
        impressHelper.waitForSlideShowIdle(win2);
        impressHelper.waitForSlideShowIdle(win1);
        getSlideShowCanvas().compareSnapshot('slide2_effect2', 0.05);
        cy.cSetActiveFrame('#iframe1');
        impressHelper.waitForSlideShowIdle(win1);
        getSlideShowCanvas().compareSnapshot('slide2_effect2', 0.05);

        //go to previous effect
        cy.cSetActiveFrame('#iframe2');
        for (let i = 0; i < 3; i++) {
            impressHelper.clickSlideShowNav(win2, '#previous');
            impressHelper.waitForSlideShowIdle(win2);
            impressHelper.waitForSlideShowIdle(win1);
        }
        getSlideShowCanvas().compareSnapshot('effect1', 0.05);
        cy.cSetActiveFrame('#iframe1');
        impressHelper.waitForSlideShowIdle(win1);
        getSlideShowCanvas().compareSnapshot('effect1', 0.05);
    });

    it('Follow and unfollow', function () {
        cy.cSetActiveFrame('#iframe2');
        cy.cGet('.notebookbar #slide-presentation-follow-me').click();
        getSlideShow().should('be.visible');
        impressHelper.waitForSlideShowIdle(win2);
        impressHelper.waitForSlideShowIdle(win1);

        //move to nextslide last effect
        for (let i = 0; i < 4; i++) {
            impressHelper.clickSlideShowNav(win2, '#next');
            impressHelper.waitForSlideShowIdle(win2);
            impressHelper.waitForSlideShowIdle(win1);
        }
        getSlideShowCanvas().compareSnapshot('slide2_effect2', 0.05);

        cy.cSetActiveFrame('#iframe1');
        getSlideShow().should('be.visible');
        impressHelper.waitForSlideShowIdle(win1);
        getSlideShowCanvas().compareSnapshot('slide2_effect2', 0.05);

        //unfollow by going 1 slide backward
        for (let i = 0; i < 3; i++) {
            impressHelper.clickSlideShowNav(win1, '#previous');
            impressHelper.waitForSlideShowIdle(win1);
        }
        // higher tolerance due to difference in state of navigation buttons
        getSlideShowCanvas().compareSnapshot('effect1', 0.07);

        //start following again
        cy.cSetActiveFrame('#iframe1');
        impressHelper.waitForSlideShowIdle(win1);
        getSlideShowContent().find("#follow").click();
        impressHelper.waitForSlideShowIdle(win1);
        getSlideShowCanvas().compareSnapshot('slide2_effect2', 0.07);

        cy.cSetActiveFrame('#iframe2');
        impressHelper.clickSlideShowNav(win2, '#next');
        impressHelper.waitForSlideShowIdle(win2);
        impressHelper.waitForSlideShowIdle(win1);
        getSlideShowCanvas().compareSnapshot('slide2_effect3', 0.07);
    });

    it('Rejoin', function () {
        cy.cSetActiveFrame('#iframe2');
        cy.cGet('.notebookbar #slide-presentation-follow-me').click();
        getSlideShow().should('be.visible');
        impressHelper.waitForSlideShowIdle(win2);
        impressHelper.waitForSlideShowIdle(win1);

        //move to nextslide last effect
        for (let i = 0; i < 4; i++) {
            impressHelper.clickSlideShowNav(win2, '#next');
            impressHelper.waitForSlideShowIdle(win2);
            impressHelper.waitForSlideShowIdle(win1);
        }
        getSlideShowCanvas().compareSnapshot('slide2_effect2', 0.05);

        cy.cSetActiveFrame('#iframe1');
        impressHelper.waitForSlideShowIdle(win1);
        impressHelper.clickSlideShowNav(win1, '#endshow');
        getSlideShow().should('not.exist');
        // Wait for the _windowCloseInterval cleanup to complete before clicking follow.
        helper.waitForTimers(win1, 'slideshowwindowclose');
        cy.cGet('.notebookbar #slide-presentation-follow').should('be.visible');
        cy.cGet('#slide-presentation-follow').click();
        getSlideShow().should('be.visible');
        impressHelper.waitForSlideShowIdle(win1);
        getSlideShowCanvas().compareSnapshot('slide2_effect2', 0.05);
    });

    // Dispatch a keydown event on the exact document node where the
    // SlideShowNavigator keydown handler is registered. We go through
    // the presenter's _slideShowWindowProxy to get the same object
    // reference that addEventListener was called on.
    function sendSlideshowKey(win, keyCode) {
        cy.then(function() {
            var presenter = win.app.map.slideShowPresenter;
            var proxyWin = presenter._slideShowWindowProxy.contentWindow;
            var doc = proxyWin.document;
            doc.dispatchEvent(new proxyWin.KeyboardEvent('keydown', {
                code: keyCode,
                key: keyCode,
                bubbles: true,
                cancelable: true,
            }));
        });
    }

    it('Follower cannot advance when at leader position', function () {
        // Leader (iframe2) starts followme and advances 1 effect
        cy.cSetActiveFrame('#iframe2');
        cy.cGet('.notebookbar #slide-presentation-follow-me').click();
        getSlideShow().should('be.visible');
        impressHelper.waitForSlideShowIdle(win2);
        impressHelper.waitForSlideShowIdle(win1);
        impressHelper.clickSlideShowNav(win2, '#next');
        impressHelper.waitForSlideShowIdle(win2);
        impressHelper.waitForSlideShowIdle(win1);

        // Follower (iframe1) auto-follows to effect1
        cy.cSetActiveFrame('#iframe1');
        // getSlideShow().should('be.visible');
        impressHelper.waitForSlideShowIdle(win1);
        getSlideShowCanvas().compareSnapshot('effect1', 0.05);

        // Follower presses ArrowRight - blocked by canUserAdvanceEffect
        sendSlideshowKey(win1, 'ArrowRight');
        impressHelper.waitForSlideShowIdle(win1);
        // Still at effect1, not advanced to slide 2
        getSlideShowCanvas().compareSnapshot('effect1', 0.05);
    });

    it('Follower: skip to last reachable effect of leader slide', function () {
        // Leader (iframe2) starts followme and advances 4 clicks to slide2_effect2
        cy.cSetActiveFrame('#iframe2');
        cy.cGet('.notebookbar #slide-presentation-follow-me').click();
        getSlideShow().should('be.visible');
        impressHelper.waitForSlideShowIdle(win2);
        impressHelper.waitForSlideShowIdle(win1);
        for (let i = 0; i < 4; i++) {
            impressHelper.clickSlideShowNav(win2, '#next');
            impressHelper.waitForSlideShowIdle(win2);
            impressHelper.waitForSlideShowIdle(win1);
        }
        getSlideShowCanvas().compareSnapshot('slide2_effect2', 0.05);

        // Follower (iframe1) auto-follows to slide2_effect2
        cy.cSetActiveFrame('#iframe1');
        getSlideShow().should('be.visible');
        impressHelper.waitForSlideShowIdle(win1);

        // Follower goes backward 2 times on slide 2 to rewind effects
        impressHelper.clickSlideShowNav(win1, '#previous');
        impressHelper.waitForSlideShowIdle(win1);
        impressHelper.clickSlideShowNav(win1, '#previous');
        impressHelper.waitForSlideShowIdle(win1);

        // Follower presses PageDown - should skip only to leader's 2 effects, not all 3
        sendSlideshowKey(win1, 'PageDown');
        impressHelper.waitForSlideShowIdle(win1);
        // Should be at slide2_effect2 (leader position), not slide2_effect3
        getSlideShowCanvas().compareSnapshot('slide2_effect2', 0.07);
    });

    it('Exit', function () {
        cy.cSetActiveFrame('#iframe2');
        cy.cGet('.notebookbar #slide-presentation-follow-me').click();
        getSlideShow().should('be.visible');
        impressHelper.waitForSlideShowIdle(win2);
        impressHelper.waitForSlideShowIdle(win1);
        impressHelper.clickSlideShowNav(win2, '#next');
        impressHelper.waitForSlideShowIdle(win2);
        impressHelper.waitForSlideShowIdle(win1);

        getSlideShowCanvas().compareSnapshot('effect1', 0.05);

        cy.cSetActiveFrame('#iframe1');
        impressHelper.waitForSlideShowIdle(win1);
        getSlideShowCanvas().compareSnapshot('effect1', 0.05);

        cy.cSetActiveFrame('#iframe2');
        impressHelper.clickSlideShowNav(win2, '#endshow');
        getSlideShow().should('not.exist');
        cy.cSetActiveFrame('#iframe1');
        getSlideShow().should('not.exist');
    });
});
