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

describe('CleanupSidebar', function () {
	const assert = require('assert').strict;

	// In the browser window is the global object, so the _n() that plural.js installs is where
	// a bare call finds it. Under mocha the two are separate objects, so hand it to the global
	// the panel's own call is resolved against.
	before(function () {
		(globalThis as any)._n = (window as any)._n;
	});

	describe('a byte figure written out', function () {
		it('counts whole bytes under a kilobyte', function () {
			assert.equal(formatByteSize(0), '0 bytes');
			assert.equal(formatByteSize(1), '1 byte');
			assert.equal(formatByteSize(1023), '1023 bytes');
		});

		it('rounds to whole kilobytes under a megabyte', function () {
			assert.equal(formatByteSize(1024), '1 KB');
			assert.equal(formatByteSize(1536), '2 KB');
			assert.equal(formatByteSize(1024 * 1024 - 1), '1024 KB');
		});

		it('gives a megabyte figure one decimal place with a period', function () {
			assert.equal(formatByteSize(1024 * 1024), '1.0 MB');
			assert.equal(formatByteSize(1024 * 1024 * 2.25), '2.3 MB');
		});
	});

	describe('a finding the panel is asked to draw', function () {
		const finding = {
			id: 3,
			category: 'image',
			slide: 1,
			imageNumber: 0,
			dpi: 857,
			hiddenPercent: 0,
			name: '',
			currentBytes: 921600,
			saving: 400000,
			measured: true,
			canGoTo: true,
			canFix: true,
		};

		it('is accepted when every figure is there', function () {
			assert.equal(isCleanupRow(finding), true);
		});

		it('is refused when a figure is missing', function () {
			const noNumber: any = Object.assign({}, finding);
			delete noNumber.imageNumber;
			assert.equal(isCleanupRow(noNumber), false);

			const noSaving: any = Object.assign({}, finding);
			delete noSaving.saving;
			assert.equal(isCleanupRow(noSaving), false);

			const noId: any = Object.assign({}, finding);
			delete noId.id;
			assert.equal(isCleanupRow(noId), false);
		});

		it('is refused when the kind of finding is not one the panel knows', function () {
			const unknown = Object.assign({}, finding, { category: 'video' });
			assert.equal(isCleanupRow(unknown), false);
		});

		it('is accepted when the finding is a cropped image', function () {
			const cropped = Object.assign({}, finding, {
				category: 'croppedImage',
			});
			assert.equal(isCleanupRow(cropped), true);
		});

		it('is refused when it is not an object at all', function () {
			assert.equal(isCleanupRow(null), false);
			assert.equal(isCleanupRow('Slide 2'), false);
		});
	});

	describe('a patch of a row already on screen', function () {
		it('needs no more than the id of the row', function () {
			assert.equal(isCleanupRowPatch({ id: 7 }), true);
		});

		it('is accepted when the figures it carries are of the right kind', function () {
			assert.equal(
				isCleanupRowPatch({ id: 7, imageNumber: 2, saving: 12 }),
				true,
			);
		});

		it('is refused when a figure arrives as something else', function () {
			assert.equal(isCleanupRowPatch({ id: 7, saving: '12' }), false);
			assert.equal(isCleanupRowPatch({ id: 7, imageNumber: '2' }), false);
		});

		it('is refused when it names no row', function () {
			assert.equal(isCleanupRowPatch({ imageNumber: 2 }), false);
		});
	});

	describe('a list of rows or of ids', function () {
		it('comes out empty when the payload carries no list', function () {
			assert.deepEqual(asArray(undefined), []);
			assert.deepEqual(asArray('Slide 2'), []);
			assert.deepEqual(asArray({ id: 3 }), []);
		});

		it('keeps the entries of a list as they arrived', function () {
			assert.deepEqual(asArray([{ id: 3 }, 7]), [{ id: 3 }, 7]);
		});

		it('holds only the ids that are numbers', function () {
			assert.deepEqual(asNumberArray([1, '2', null, 3]), [1, 3]);
			assert.deepEqual(asNumberArray('1'), []);
		});
	});

	describe('the values a scan is run with', function () {
		const options = { resolution: 150, quality: 80, forPublication: false };

		it('are accepted when both figures and the choice are there', function () {
			assert.equal(isCleanupOptions(options), true);
		});

		it('are refused when a field is missing', function () {
			const noQuality: any = Object.assign({}, options);
			delete noQuality.quality;
			assert.equal(isCleanupOptions(noQuality), false);
		});

		it('are refused when a field arrives as something else', function () {
			assert.equal(
				isCleanupOptions(Object.assign({}, options, { resolution: '150' })),
				false,
			);
			assert.equal(
				isCleanupOptions(Object.assign({}, options, { forPublication: 'yes' })),
				false,
			);
			assert.equal(isCleanupOptions(null), false);
		});
	});

	describe('how far the measuring has come', function () {
		it('is the pair of counts the kit sends', function () {
			assert.deepEqual(asMeasuredProgress({ done: 2, total: 5 }), {
				done: 2,
				total: 5,
			});
		});

		it('counts as nothing measured when a count is missing or of another kind', function () {
			assert.deepEqual(asMeasuredProgress(undefined), {
				done: 0,
				total: 0,
			});
			assert.deepEqual(asMeasuredProgress({ done: '2', total: 5 }), {
				done: 0,
				total: 5,
			});
		});
	});

	describe('a request the kit turns away', function () {
		it('ends the busy state for the requests that started one', function () {
			assert.equal(refusalEndsBusyState('scan'), true);
			assert.equal(refusalEndsBusyState('fix'), true);
			assert.equal(refusalEndsBusyState('fixAll'), true);
		});

		it('leaves a run going when the refused request was a goTo or a list', function () {
			assert.equal(refusalEndsBusyState('goTo'), false);
			assert.equal(refusalEndsBusyState('list'), false);
			assert.equal(refusalEndsBusyState(undefined), false);
		});
	});

	// The deck is a function of the panel state, so what it shows is read off the JSON it
	// makes.
	function findWidget(root: any, id: string): any {
		if (root.id === id) return root;
		for (const child of root.children || []) {
			const found = findWidget(child, id);
			if (found) return found;
		}
		return null;
	}

	function isShown(widget: any): boolean {
		return widget.visible !== false;
	}

	function stateWithRows(rows: any[]): CleanupPanelState {
		const state = newCleanupPanelState();
		for (const row of rows) state.rows.set(row.id, row);
		state.hasList = true;
		state.rowCount = rows.length;
		state.fixableCount = rows.filter((row) => row.canFix).length;
		return state;
	}

	const imageRow = {
		id: 3,
		category: 'image',
		slide: 1,
		imageNumber: 0,
		dpi: 857,
		hiddenPercent: 0,
		name: '',
		currentBytes: 921600,
		saving: 0,
		measured: false,
		canGoTo: true,
		canFix: true,
	};

	const notesRow = {
		id: 4,
		category: 'notes',
		slide: 2,
		imageNumber: 0,
		dpi: 0,
		hiddenPercent: 0,
		name: '',
		currentBytes: 1200,
		saving: 1200,
		measured: true,
		canGoTo: false,
		canFix: false,
	};

	const croppedRow = {
		id: 5,
		category: 'croppedImage',
		slide: 2,
		imageNumber: 0,
		dpi: 0,
		hiddenPercent: 62,
		name: '',
		currentBytes: 921600,
		saving: 400000,
		measured: true,
		canGoTo: true,
		canFix: true,
	};

	describe('the kinds of finding a scan covers', function () {
		it('are the two slide checks whatever the scan is run with', function () {
			const scanned = scannedFromOptions({
				resolution: 0,
				quality: 80,
				forPublication: false,
			});
			assert.deepEqual(scanned, ['hiddenSlide', 'unusedMaster']);
		});

		it('bring the image checks in once a target resolution is asked for', function () {
			const scanned = scannedFromOptions({
				resolution: 150,
				quality: 80,
				forPublication: false,
			});
			assert.equal(scanned.indexOf('image') !== -1, true);
			assert.equal(scanned.indexOf('croppedImage') !== -1, true);
		});

		it('bring the sharing checks in for a deck being prepared to hand out', function () {
			const scanned = scannedFromOptions({
				resolution: 0,
				quality: 80,
				forPublication: true,
			});
			assert.equal(scanned.indexOf('notes') !== -1, true);
			assert.equal(scanned.indexOf('embeddedObject') !== -1, true);
		});

		it('are read off the list, with anything the panel does not know left out', function () {
			assert.deepEqual(asCategoryArray(['image', 'video', 'notes']), [
				'image',
				'notes',
			]);
			assert.deepEqual(asCategoryArray(undefined), []);
		});
	});

	describe('what a check has to report', function () {
		it('is what it found while it holds findings', function () {
			assert.equal(checkStateFor(2, true), 'found');
			assert.equal(checkStateFor(2, false), 'found');
		});

		it('is none once a scan has covered it and turned nothing up', function () {
			assert.equal(checkStateFor(0, true), 'none');
		});

		it('is that it was not checked while no scan has covered it', function () {
			assert.equal(checkStateFor(0, false), 'unchecked');
		});
	});

	describe('the one line the scan values say while they are folded away', function () {
		it('names the resolution and the quality', function () {
			assert.equal(
				imageOptionsSummary({
					resolution: 150,
					quality: 80,
					forPublication: false,
				}),
				'150 DPI, quality 80 %',
			);
		});

		it('says the images are left alone when no resolution is asked for', function () {
			assert.equal(
				imageOptionsSummary({
					resolution: 0,
					quality: 80,
					forPublication: false,
				}),
				'images left alone',
			);
		});
	});

	describe('what the band says the list comes to', function () {
		it('gives the share of the file the saving would free up', function () {
			assert.equal(
				cleanupTotalText(250000, 1000000),
				'244 KB (25%) can be cleaned up',
			);
		});

		it('leaves the share out when the size of the file is not known', function () {
			assert.equal(cleanupTotalText(250000, 0), '244 KB can be cleaned up');
		});

		it('leaves the share out when it would not round to a whole percent', function () {
			assert.equal(
				cleanupTotalText(100, 1000000),
				'100 bytes can be cleaned up',
			);
		});
	});

	describe('the word on the button of a row', function () {
		it('offers to remove the speaker notes and to fix everything else', function () {
			assert.equal(fixButtonText('notes'), 'Remove');
			assert.equal(fixButtonText('image'), 'Fix');
		});
	});

	describe('the deck before a scan', function () {
		const deck = cleanupDeckJSON(newCleanupPanelState());

		it('offers the options and the scan button and nothing of a run', function () {
			assert.equal(isShown(findWidget(deck, 'cleanup-options')), true);
			assert.equal(isShown(findWidget(deck, 'cleanup-scan')), true);
			assert.equal(isShown(findWidget(deck, 'cleanup-status')), false);
			assert.equal(isShown(findWidget(deck, 'cleanup-summary')), false);
		});

		it('says what the deck is for', function () {
			const message = findWidget(deck, 'cleanup-message');
			assert.equal(isShown(message), true);
			assert.equal(message.text.indexOf('clean it up here') !== -1, true);
		});

		it('keeps every group off the screen', function () {
			for (const category of [
				'image',
				'hiddenSlide',
				'unusedMaster',
				'notes',
				'embeddedObject',
			])
				assert.equal(
					isShown(findWidget(deck, groupWidgetId(category as CleanupCategory))),
					false,
				);
		});

		it('shows the options the controls hold', function () {
			const resolution = findWidget(deck, 'cleanup-resolution');
			assert.deepEqual(resolution.selectedEntries, ['2']);
			assert.equal(findWidget(deck, 'cleanup-quality').value, 80);
			assert.equal(findWidget(deck, 'cleanup-sharing').checked, false);
		});
	});

	describe('the deck with a list', function () {
		const deck = cleanupDeckJSON(stateWithRows([imageRow, notesRow]));

		it('shows the checks that hold a row, with the count beside the name', function () {
			const images = findWidget(deck, 'cleanup-group-image');
			assert.equal(isShown(images), true);
			assert.equal(images.children[0].text, 'High-resolution images');
			assert.equal(images.secondaryText, '1');
			assert.equal(isShown(findWidget(deck, 'cleanup-group-notes')), true);
			assert.equal(
				isShown(findWidget(deck, 'cleanup-group-hidden-slide')),
				false,
			);
		});

		it('leaves the flat row of a check that holds a row off the screen', function () {
			assert.equal(isShown(findWidget(deck, 'cleanup-check-image')), false);
			assert.equal(
				isShown(findWidget(deck, 'cleanup-check-hidden-slide')),
				true,
			);
		});

		it('draws a row as a link when the view can go there and as text otherwise', function () {
			assert.equal(findWidget(deck, 'cleanup-row-3-goto').type, 'linkbutton');
			assert.equal(findWidget(deck, 'cleanup-row-3-text'), null);
			assert.equal(findWidget(deck, 'cleanup-row-4-text').type, 'fixedtext');
			assert.equal(findWidget(deck, 'cleanup-row-4-goto'), null);
		});

		it('gives a Fix button only to a row that offers a cleanup', function () {
			assert.equal(findWidget(deck, 'cleanup-row-3-fix').type, 'pushbutton');
			assert.equal(findWidget(deck, 'cleanup-row-4-fix'), null);
		});

		it('writes the saving on a row once it is measured', function () {
			assert.equal(findWidget(deck, 'cleanup-row-3-saving'), null);
			assert.equal(findWidget(deck, 'cleanup-row-4-saving').text, 'saves 1 KB');
		});

		it('leaves the detail line out of a row that has none', function () {
			assert.equal(
				findWidget(deck, 'cleanup-row-3-detail').text,
				'900 KB at 857 DPI',
			);
			assert.equal(findWidget(deck, 'cleanup-row-4-detail'), null);
		});

		it('writes the line that names a finding from the figures it carries', function () {
			assert.equal(findWidget(deck, 'cleanup-row-3-goto').text, 'Slide 2');
			assert.equal(
				findWidget(deck, 'cleanup-row-4-text').text,
				'Slide 3 has speaker notes',
			);
		});

		it('adds the list up in the band and offers Fix all', function () {
			const summary = findWidget(deck, 'cleanup-summary');
			assert.equal(isShown(summary), true);
			assert.equal(
				findWidget(deck, 'cleanup-total').text,
				'1 thing to clean up',
			);
			assert.equal(isShown(findWidget(deck, 'cleanup-fix-all')), true);
			assert.equal(isShown(findWidget(deck, 'cleanup-message')), false);
		});
	});

	describe('the detail line of a cropped image', function () {
		it('says what the picture takes and how much of it is hidden', function () {
			const deck = cleanupDeckJSON(stateWithRows([croppedRow]));
			assert.equal(
				findWidget(deck, 'cleanup-row-5-detail').text,
				'900 KB, 62% cropped away',
			);
		});

		it('says the share alone while the picture has no size of its own', function () {
			const noBytes = Object.assign({}, croppedRow, { currentBytes: 0 });
			const deck = cleanupDeckJSON(stateWithRows([noBytes]));
			assert.equal(
				findWidget(deck, 'cleanup-row-5-detail').text,
				'62% cropped away',
			);
		});
	});

	describe('the deck while a run is going', function () {
		const state = stateWithRows([imageRow]);
		state.busy = true;
		state.statusName = 'measuring';
		state.measuredDone = 2;
		state.measuredTotal = 5;
		const deck = cleanupDeckJSON(state);

		it('swaps the scan button for the status of the run', function () {
			assert.equal(isShown(findWidget(deck, 'cleanup-scan')), false);
			assert.equal(isShown(findWidget(deck, 'cleanup-status')), true);
			assert.equal(
				findWidget(deck, 'cleanup-status-text').text,
				'Measuring images...',
			);
		});

		it('puts the counts on the bar', function () {
			const progress = findWidget(deck, 'cleanup-progress');
			assert.equal(progress.value, 2);
			assert.equal(progress.maxValue, 5);
			assert.equal(progress.infinite, false);
			assert.equal(findWidget(deck, 'cleanup-progress-figures').text, '2 of 5');
		});

		it('turns the options and the row buttons off and hides the band', function () {
			assert.equal(findWidget(deck, 'cleanup-resolution').enabled, false);
			assert.equal(findWidget(deck, 'cleanup-quality').enabled, false);
			assert.equal(findWidget(deck, 'cleanup-row-3-fix').enabled, false);
			assert.equal(isShown(findWidget(deck, 'cleanup-summary')), false);
		});

		it('lets the bar move on its own while the whole is not known', function () {
			state.measuredTotal = 0;
			const progress = findWidget(cleanupDeckJSON(state), 'cleanup-progress');
			assert.equal(progress.infinite, true);
		});
	});

	describe('a row a cleanup dealt with', function () {
		it('waits dimmed for the scan that follows', function () {
			const state = stateWithRows([imageRow]);
			state.goneRows.add(3);
			const row = cleanupRegionJSON(state, 'cleanup-row-3');
			assert.equal(row.cssClass.indexOf('cleanup-row-gone') !== -1, true);
		});
	});

	describe('the deck of a document that cannot be changed', function () {
		const state = newCleanupPanelState();
		state.readOnly = true;
		const deck = cleanupDeckJSON(state);

		it('offers no options, no run and no band', function () {
			assert.equal(isShown(findWidget(deck, 'cleanup-options')), false);
			assert.equal(isShown(findWidget(deck, 'cleanup-scan')), false);
			assert.equal(isShown(findWidget(deck, 'cleanup-summary')), false);
			assert.equal(isShown(findWidget(deck, 'cleanup-separator')), false);
		});

		it('says why the list is empty', function () {
			const message = findWidget(deck, 'cleanup-message');
			assert.equal(isShown(message), true);
			assert.equal(message.text.indexOf('cannot be changed') !== -1, true);
		});
	});

	describe('a region asked for on its own', function () {
		it('is the same widget the whole deck holds', function () {
			const state = stateWithRows([imageRow, notesRow]);
			const deck = cleanupDeckJSON(state);
			for (const id of [
				'cleanup-options',
				'cleanup-options-body',
				'cleanup-run',
				'cleanup-summary',
				'cleanup-message',
				'cleanup-tool-images',
				'cleanup-check-notes',
				'cleanup-group-image',
				'cleanup-row-4',
			])
				assert.deepEqual(cleanupRegionJSON(state, id), findWidget(deck, id));
		});

		it('is nothing for an id the deck does not know', function () {
			const state = stateWithRows([imageRow]);
			assert.equal(cleanupRegionJSON(state, 'cleanup-row-9'), null);
			assert.equal(cleanupRegionJSON(state, 'cleanup-row-3-fix'), null);
			assert.equal(cleanupRegionJSON(state, 'somethingelse'), null);
		});
	});

	describe('how the options row looks', function () {
		it('stays as it was while a run turns its fields off, and the fields change instead', function () {
			const idle = stateWithRows([imageRow]);
			const busy = stateWithRows([imageRow]);
			busy.busy = true;

			const look = (state: CleanupPanelState, id: string) =>
				regionLook(cleanupRegionJSON(state, id));

			assert.equal(
				look(idle, 'cleanup-options'),
				look(busy, 'cleanup-options'),
			);
			assert.notEqual(
				look(idle, 'cleanup-options-body'),
				look(busy, 'cleanup-options-body'),
			);
		});
	});

	describe('how a group looks', function () {
		it('changes when a row joins or leaves it or its heading changes', function () {
			const one = regionLook(
				cleanupRegionJSON(stateWithRows([imageRow]), 'cleanup-group-image'),
			);
			const two = regionLook(
				cleanupRegionJSON(
					stateWithRows([imageRow, Object.assign({}, imageRow, { id: 5 })]),
					'cleanup-group-image',
				),
			);
			assert.notEqual(one, two);
		});

		it('stays as it was when only what a row says changes', function () {
			const before = regionLook(
				cleanupRegionJSON(stateWithRows([imageRow]), 'cleanup-group-image'),
			);
			const after = regionLook(
				cleanupRegionJSON(
					stateWithRows([Object.assign({}, imageRow, { saving: 400000 })]),
					'cleanup-group-image',
				),
			);
			assert.equal(before, after);
		});
	});

	describe('the widget ids of the deck', function () {
		it('name the row of a row widget', function () {
			assert.equal(rowOfWidgetId('cleanup-row-12-fix', '-fix'), 12);
			assert.equal(rowOfWidgetId('cleanup-row-12-goto', '-goto'), 12);
			assert.equal(rowOfWidgetId('cleanup-row-12', ''), 12);
		});

		it('name no row for any other widget', function () {
			assert.equal(rowOfWidgetId('cleanup-row-12-goto', '-fix'), null);
			assert.equal(rowOfWidgetId('cleanup-scan', '-fix'), null);
			assert.equal(rowOfWidgetId('cleanup-row-x-fix', '-fix'), null);
		});

		it('name the category of a group widget', function () {
			assert.equal(
				categoryOfGroupWidgetId('cleanup-group-hidden-slide'),
				'hiddenSlide',
			);
			assert.equal(
				categoryOfGroupWidgetId('cleanup-group-image-heading'),
				null,
			);
		});

		it('tell the controls a run turns off', function () {
			assert.equal(goesDeadWhenBusy('cleanup-scan-button'), true);
			assert.equal(goesDeadWhenBusy('cleanup-quality-input'), true);
			assert.equal(goesDeadWhenBusy('cleanup-row-3-fix-button'), true);
			assert.equal(goesDeadWhenBusy('cleanup-stop-button'), false);
			assert.equal(goesDeadWhenBusy('cleanup-row-3-goto'), false);
		});
	});

	describe('the values the option controls report', function () {
		it('are taken from the widget events of the deck', function () {
			const options = { resolution: 150, quality: 80, forPublication: false };
			assert.equal(
				applyOptionEvent(
					options,
					'combobox',
					'selected',
					'cleanup-resolution',
					'1;96 DPI',
				),
				true,
			);
			assert.equal(options.resolution, 96);
			assert.equal(
				applyOptionEvent(
					options,
					'spinfield',
					'change',
					'cleanup-quality',
					'65',
				),
				true,
			);
			assert.equal(options.quality, 65);
			assert.equal(
				applyOptionEvent(
					options,
					'checkbox',
					'change',
					'cleanup-sharing',
					true,
				),
				true,
			);
			assert.equal(options.forPublication, true);
		});

		it('keep a figure inside the range the control offers', function () {
			const options = { resolution: 150, quality: 80, forPublication: false };
			applyOptionEvent(
				options,
				'spinfield',
				'change',
				'cleanup-quality',
				'250',
			);
			assert.equal(options.quality, 100);
			applyOptionEvent(
				options,
				'spinfield',
				'change',
				'cleanup-quality',
				'abc',
			);
			assert.equal(options.quality, 100);
			applyOptionEvent(
				options,
				'combobox',
				'selected',
				'cleanup-resolution',
				'9;',
			);
			assert.equal(options.resolution, 150);
		});

		it('leave any other event alone', function () {
			const options = { resolution: 150, quality: 80, forPublication: false };
			assert.equal(
				applyOptionEvent(
					options,
					'pushbutton',
					'click',
					'cleanup-scan',
					undefined,
				),
				false,
			);
			assert.equal(
				applyOptionEvent(
					options,
					'spinfield',
					'value',
					'cleanup-quality',
					'65',
				),
				false,
			);
			assert.deepEqual(options, {
				resolution: 150,
				quality: 80,
				forPublication: false,
			});
		});
	});

	describe('an event carrying a run number', function () {
		it('is applied when it names the list the panel shows', function () {
			assert.equal(
				isStaleEvent({ event: 'progress', run: 4 }, 4, false),
				false,
			);
		});

		it('is dropped when it names another list', function () {
			assert.equal(isStaleEvent({ event: 'progress', run: 3 }, 4, false), true);
		});

		it('is dropped after the panel has let its list go', function () {
			assert.equal(
				isStaleEvent({ event: 'measured', run: 3 }, null, false),
				true,
			);
		});

		it('is applied while the panel is waiting to hear which list it now has', function () {
			assert.equal(
				isStaleEvent({ event: 'measured', run: 3 }, null, true),
				false,
			);
		});

		it('is applied when it names no run at all', function () {
			assert.equal(isStaleEvent({ event: 'finished' }, 4, false), false);
		});
	});
});
