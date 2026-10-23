/* -*- js-indent-level: 8; fill-column: 100 -*- */
/*
 * Copyright the Collabora Online contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

describe('AI chat sidebar', function () {
	let map: any;
	let sidebar: any;
	let errors: string[];
	const realError = console.error;

	beforeEach(function () {
		installFakeClock();
		(window as any).prefs.reset();
		JSDialog.AIChatSidebar = null;
		map = setUpAIChatPage();
		sidebar = new cool.AIChatSidebar();
		JSDialog.AIChatSidebar = sidebar;
		// The layouting service catches a failing task and only logs it.
		errors = [];
		console.error = (message: any) => errors.push(String(message));
	});

	afterEach(function () {
		console.error = realError;
		JSDialog.AIChatSidebar = null;
		tearDownAIChatPage();
		restoreClock();
		(window as any).prefs.reset();
		nodeassert.deepStrictEqual(errors, []);
	});

	function all(selector: string): HTMLElement[] {
		return Array.from(document.querySelectorAll(selector)) as HTMLElement[];
	}

	function one(selector: string): HTMLElement {
		const element = document.querySelector(selector) as HTMLElement;
		nodeassert.ok(element, 'nothing matches ' + selector);
		return element;
	}

	describe('outline', function () {
		const outline = {
			title: 'Renewable Energy',
			slides: [
				{
					part: 'opening',
					intent: 'title',
					title: 'Renewable Energy',
					gist: 'Set the stage for the whole deck.',
				},
				{
					part: 'body',
					intent: 'bullets',
					title: 'Solar Power',
					gist: 'Cover photovoltaic basics.',
				},
				{
					part: 'closing',
					intent: 'closing',
					title: 'Thank You',
					gist: 'Wrap up and invite questions.',
				},
			],
		};

		// A single-slide presentation is a fresh deck, so the server proposes an outline.
		async function requestOutline(): Promise<void> {
			map.docType = 'presentation';
			answerAIRequests({ outline: outline });
			await sendAIRequest(sidebar, 'Create a deck about renewable energy');
			one('.aichat-outline-card');
		}

		function row(index: number): HTMLElement {
			return all('.aichat-outline-row')[index];
		}

		function rowField(index: number, selector: string): HTMLInputElement {
			return row(index).querySelector(selector) as HTMLInputElement;
		}

		function approveButton(): HTMLButtonElement {
			return one('.aichat-approve-btn') as HTMLButtonElement;
		}

		it('the card shows one editable row per proposed slide', async function () {
			await requestOutline();

			nodeassert.strictEqual(all('.aichat-outline-row').length, 3);
			nodeassert.strictEqual(
				rowField(0, '.aichat-outline-num').textContent,
				'1.',
			);
			nodeassert.strictEqual(
				rowField(2, '.aichat-outline-num').textContent,
				'3.',
			);
			nodeassert.strictEqual(all('input.aichat-outline-title').length, 3);
			nodeassert.strictEqual(all('button.aichat-outline-delete').length, 3);
			nodeassert.strictEqual(
				rowField(0, '.aichat-outline-title').value,
				'Renewable Energy',
			);
			nodeassert.strictEqual(
				rowField(1, '.aichat-outline-title').value,
				'Solar Power',
			);
		});

		it('approving sends the edited outline with a deleted slide dropped', async function () {
			await requestOutline();

			const title = rowField(0, '.aichat-outline-title');
			clearField(title);
			typeInto(title, 'Clean Energy Today');
			rowField(1, '.aichat-outline-delete').click();
			nodeassert.strictEqual(all('.aichat-outline-row').length, 2);
			await clickOn('.aichat-approve-btn');

			const decisions = sentAIApprovals();
			nodeassert.strictEqual(decisions.length, 1);
			const decision = decisions[0];
			nodeassert.strictEqual(decision.action, 'approve');
			nodeassert.strictEqual(decision.outline.title, 'Renewable Energy');
			nodeassert.deepStrictEqual(
				decision.outline.slides.map((slide: any) => slide.title),
				['Clean Energy Today', 'Thank You'],
			);
			nodeassert.strictEqual(decision.outline.slides[0].part, 'opening');

			nodeassert.ok(
				aiMessagesText().includes('Outline approved - building 2 slides.'),
			);
			nodeassert.strictEqual(all('.aichat-outline-card').length, 0);
		});

		it('an emptied title falls back to the proposed title', async function () {
			await requestOutline();

			clearField(rowField(1, '.aichat-outline-title'));
			await clickOn('.aichat-approve-btn');

			nodeassert.strictEqual(
				sentAIApprovals()[0].outline.slides[1].title,
				'Solar Power',
			);
		});

		it('the gist of each slide is editable', async function () {
			await requestOutline();

			nodeassert.strictEqual(all('textarea.aichat-outline-gist').length, 3);
			nodeassert.strictEqual(
				rowField(0, '.aichat-outline-gist').value,
				'Set the stage for the whole deck.',
			);
		});

		it('an edited gist is sent with the approved outline', async function () {
			await requestOutline();

			const gist = rowField(0, '.aichat-outline-gist');
			clearField(gist);
			typeInto(gist, 'Frame why renewables matter now.');
			await clickOn('.aichat-approve-btn');

			const slides = sentAIApprovals()[0].outline.slides;
			nodeassert.strictEqual(
				slides[0].gist,
				'Frame why renewables matter now.',
			);
			nodeassert.strictEqual(slides[1].gist, 'Cover photovoltaic basics.');
		});

		it('an emptied gist falls back to the proposed description', async function () {
			await requestOutline();

			clearField(rowField(1, '.aichat-outline-gist'));
			await clickOn('.aichat-approve-btn');

			nodeassert.strictEqual(
				sentAIApprovals()[0].outline.slides[1].gist,
				'Cover photovoltaic basics.',
			);
		});

		it('rejecting sends the plain decision and collapses the card', async function () {
			await requestOutline();

			await clickOn('.aichat-reject-btn');

			const decisions = sentAIApprovals();
			nodeassert.strictEqual(decisions.length, 1);
			nodeassert.strictEqual(decisions[0].action, 'reject');
			nodeassert.strictEqual(decisions[0].outline, undefined);
			nodeassert.ok(aiMessagesText().includes('Outline rejected.'));
			nodeassert.strictEqual(all('.aichat-outline-card').length, 0);
		});

		it('progress updates while the deck builds, then the result lands', async function () {
			await requestOutline();

			await clickOn('.aichat-approve-btn');
			nodeassert.ok(
				one('#aichat-progress-text').textContent.includes(
					'Building slide 1 of 2',
				),
			);

			deliverOutlineResult();
			await settleAIChat();
			nodeassert.ok(aiMessagesText().includes('Mock AI response'));
		});

		it('adding a slide appends an empty numbered row and blocks approve', async function () {
			await requestOutline();
			nodeassert.strictEqual(approveButton().disabled, false);

			await clickOn('.aichat-outline-add');

			nodeassert.strictEqual(all('.aichat-outline-row').length, 4);
			nodeassert.strictEqual(
				rowField(3, '.aichat-outline-num').textContent,
				'4.',
			);
			const title = rowField(3, '.aichat-outline-title');
			nodeassert.strictEqual(title.value, '');
			nodeassert.strictEqual(title.getAttribute('placeholder'), 'Slide title');
			const gist = rowField(3, '.aichat-outline-gist');
			nodeassert.strictEqual(gist.value, '');
			nodeassert.strictEqual(
				gist.getAttribute('placeholder'),
				'What this slide should cover',
			);
			nodeassert.strictEqual(approveButton().disabled, true);
		});

		it('a titled added slide is sent at the end of the approved outline', async function () {
			await requestOutline();

			await clickOn('.aichat-outline-add');
			nodeassert.strictEqual(approveButton().disabled, true);

			typeInto(rowField(3, '.aichat-outline-title'), 'Wind Power');
			typeInto(
				rowField(3, '.aichat-outline-gist'),
				'Cover turbines and offshore farms.',
			);
			nodeassert.strictEqual(approveButton().disabled, false);
			await clickOn('.aichat-approve-btn');

			const slides = sentAIApprovals()[0].outline.slides;
			nodeassert.strictEqual(slides.length, 4);
			nodeassert.deepStrictEqual(slides[3], {
				part: 'body',
				intent: 'bullets',
				title: 'Wind Power',
				gist: 'Cover turbines and offshore farms.',
			});
		});

		it('deleting the untitled added slide lets the user approve again', async function () {
			await requestOutline();

			await clickOn('.aichat-outline-add');
			nodeassert.strictEqual(all('.aichat-outline-row').length, 4);
			nodeassert.strictEqual(approveButton().disabled, true);

			rowField(3, '.aichat-outline-delete').click();
			nodeassert.strictEqual(all('.aichat-outline-row').length, 3);
			nodeassert.strictEqual(approveButton().disabled, false);
		});

		it('the card and its edits come back after the sidebar is reopened', async function () {
			await requestOutline();

			const title = rowField(0, '.aichat-outline-title');
			clearField(title);
			typeInto(title, 'Clean Energy Today');
			const gist = rowField(0, '.aichat-outline-gist');
			clearField(gist);
			typeInto(gist, 'Frame why renewables matter now.');
			rowField(1, '.aichat-outline-delete').click();
			await clickOn('.aichat-outline-add');
			typeInto(rowField(2, '.aichat-outline-title'), 'Wind Power');
			nodeassert.strictEqual(all('.aichat-outline-row').length, 3);
			const card = one('.aichat-outline-card');

			await reopenAIChat(sidebar);

			nodeassert.notStrictEqual(one('.aichat-outline-card'), card);
			nodeassert.strictEqual(all('.aichat-outline-row').length, 3);
			nodeassert.strictEqual(
				rowField(0, '.aichat-outline-title').value,
				'Clean Energy Today',
			);
			nodeassert.strictEqual(
				rowField(0, '.aichat-outline-gist').value,
				'Frame why renewables matter now.',
			);
			nodeassert.strictEqual(
				rowField(1, '.aichat-outline-title').value,
				'Thank You',
			);
			nodeassert.strictEqual(
				rowField(2, '.aichat-outline-title').value,
				'Wind Power',
			);
			nodeassert.strictEqual(
				rowField(2, '.aichat-outline-num').textContent,
				'3.',
			);
			nodeassert.strictEqual(approveButton().disabled, false);

			await clickOn('.aichat-approve-btn');
			const decisions = sentAIApprovals();
			nodeassert.strictEqual(decisions.length, 1);
			nodeassert.strictEqual(decisions[0].action, 'approve');
			nodeassert.deepStrictEqual(
				decisions[0].outline.slides.map((slide: any) => slide.title),
				['Clean Energy Today', 'Thank You', 'Wind Power'],
			);
			nodeassert.strictEqual(
				decisions[0].outline.slides[0].gist,
				'Frame why renewables matter now.',
			);
		});

		it('a decided outline is not offered a second time after a reopen', async function () {
			await requestOutline();

			await clickOn('.aichat-approve-btn');
			nodeassert.strictEqual(all('.aichat-outline-card').length, 0);

			await reopenAIChat(sidebar);

			nodeassert.ok(
				aiMessagesText().includes('Outline approved - building 3 slides.'),
			);
			nodeassert.strictEqual(all('.aichat-outline-card').length, 0);
			nodeassert.strictEqual(all('.aichat-approve-btn').length, 0);
		});
	});

	describe('approval', function () {
		async function requestApproval(transform?: any): Promise<void> {
			map.docType = 'presentation';
			answerAIRequests({
				approvalToolName: 'transform_document_structure',
				approvalTransformJson: transform ? JSON.stringify(transform) : '',
			});
			await sendAIRequest(sidebar, 'Tidy up my deck');
		}

		function slideCommands(commands: any[]): any {
			return { Transforms: { SlideCommands: commands } };
		}

		it('the card counts the change from the commands, not the model summary', async function () {
			// The summary sounds harmless, but the commands also delete a slide.
			await requestApproval(
				slideCommands([
					{ InsertMasterSlide: 0 },
					{ 'SetText.0': 'Title' },
					{ 'SetText.1': 'Body' },
					{ DeleteSlide: 1 },
					{ ApplyTemplate: 'Mint' },
				]),
			);

			nodeassert.ok(
				aiMessagesText().includes(
					'This will: insert 1 slide, delete 1 slide, set text in 2 places, ' +
						"apply the design template 'Mint'.",
				),
			);
		});

		it('no count is shown when the approval carries no slide commands', async function () {
			await requestApproval();

			nodeassert.ok(aiMessagesText().includes('Mock change'));
			nodeassert.ok(!aiMessagesText().includes('This will:'));
		});

		it('a compiled deck transform is described like any other', async function () {
			// A themed deck starts with ApplyTemplate, then has a layout, texts and a
			// slide part per slide, and a GenerateImage for an image slide.
			await requestApproval(
				slideCommands([
					{ ApplyTemplate: 'Cobalt' },
					{ ChangeLayoutByName: 'AUTOLAYOUT_TITLE' },
					{ 'SetText.0': 'Renewable Energy' },
					{ 'SetText.1': 'An overview' },
					{ SetSlidePart: 'opening' },
					{ InsertMasterSlide: 0 },
					{ ChangeLayoutByName: 'AUTOLAYOUT_TITLE_CONTENT' },
					{ 'SetText.0': 'Sources' },
					{ 'SetText.1': 'Solar\nWind\nHydro' },
					{ SetSlidePart: 'body' },
					{ InsertMasterSlide: 0 },
					{ ChangeLayoutByName: 'AUTOLAYOUT_TITLE_CONTENT' },
					{ 'SetText.0': 'Growth' },
					{
						'GenerateImage.1': {
							prompt: 'a line chart of renewable capacity',
							alt: 'Line chart of renewable capacity',
						},
					},
					{ SetSlidePart: 'body' },
				]),
			);

			nodeassert.ok(
				aiMessagesText().includes(
					'This will: insert 2 slides, change 3 layouts, set text in 5 places, ' +
						'generate 1 image, run 3 SetSlidePart commands, ' +
						"apply the design template 'Cobalt'.",
				),
			);
		});

		it('an unknown command is reported by its raw name', async function () {
			await requestApproval(slideCommands([{ ReplaceEverything: 1 }]));

			nodeassert.ok(
				aiMessagesText().includes(
					'This will: run 1 ReplaceEverything command.',
				),
			);
		});

		it('the approve and reject buttons come back after the sidebar is reopened', async function () {
			await requestApproval();
			one('.aichat-approval-buttons');

			await reopenAIChat(sidebar);

			one('.aichat-approval-buttons');
			await clickOn('.aichat-approve-btn');
			nodeassert.strictEqual(all('.aichat-approval-buttons').length, 0);
		});

		it('a decided approval is not offered a second time after a reopen', async function () {
			await requestApproval();
			await clickOn('.aichat-reject-btn');
			nodeassert.strictEqual(all('.aichat-approval-buttons').length, 0);

			await reopenAIChat(sidebar);

			nodeassert.strictEqual(all('.aichat-approval-buttons').length, 0);
		});
	});
});
