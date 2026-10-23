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

	describe('conversation', function () {
		const input = '#aichat-input.ui-textarea';

		function isOpen(): boolean {
			return !!document.querySelector('#aichat-dock-wrapper.visible');
		}

		function isDisabled(selector: string): boolean {
			return one(selector).hasAttribute('disabled');
		}

		async function openWithAnswer(options: any = {}): Promise<void> {
			answerAIRequests(options);
			await toggleAIChat(sidebar);
		}

		async function send(text: string): Promise<void> {
			typeInto(one(input), text);
			await clickOn('#aichat-send-btn button');
		}

		it('the aichat command opens the sidebar when AI is configured', async function () {
			answerAIRequests();
			new Dispatcher('text').dispatch('aichat');
			await settleAIChat();

			nodeassert.ok(isOpen());
			nodeassert.ok(one('#aichat-panel').children.length > 0);
		});

		it('a second toggle closes the sidebar', async function () {
			await openWithAnswer();
			nodeassert.ok(isOpen());

			await toggleAIChat(sidebar);

			nodeassert.ok(!isOpen());
		});

		it('the close button closes the sidebar', async function () {
			await openWithAnswer();

			await clickOn('#aichat-close-btn button');

			nodeassert.ok(!isOpen());
		});

		it('the aichat command offers the AI settings when AI is not configured', async function () {
			map.isAIConfigured = false;
			const prefs = (window as any).prefs;
			const realCanPersist = prefs.canPersist;
			const realSettingBaseUrl = (window as any).wopiSettingBaseUrl;
			prefs.canPersist = true;
			(window as any).wopiSettingBaseUrl = 'https://settings.test';
			try {
				new Dispatcher('text').dispatch('aichat');
				await settleAIChat();
			} finally {
				prefs.canPersist = realCanPersist;
				(window as any).wopiSettingBaseUrl = realSettingBaseUrl;
			}

			nodeassert.deepStrictEqual(map.settingsDialogCalls, [['ai-section']]);
			nodeassert.ok(!isOpen());
		});

		it('an empty conversation asks what to do with the document', async function () {
			await openWithAnswer();

			nodeassert.ok(
				one('#aichat-empty-state').textContent.includes(
					'What would you like to do with this document?',
				),
			);
		});

		it('an empty conversation greets the user for the local time of day', async function () {
			await openWithAnswer();

			const hour = new Date().getHours();
			const partOfDay =
				hour >= 5 && hour < 12
					? 'morning'
					: hour >= 12 && hour < 18
						? 'afternoon'
						: 'evening';
			nodeassert.ok(
				one('#aichat-empty-state-greeting').textContent.includes(
					'Good ' + partOfDay,
				),
			);
		});

		it('an empty conversation offers four text suggestions and no See more', async function () {
			await openWithAnswer();

			for (let index = 0; index < 4; index++) one('#aichat-chip-' + index);
			nodeassert.strictEqual(all('#aichat-see-more').length, 0);
		});

		it('the header shows the title', async function () {
			await openWithAnswer();

			nodeassert.ok(one('#aichat-title').textContent.includes('AI Assistant'));
		});

		it('clear is disabled while the conversation is empty', async function () {
			await openWithAnswer();

			nodeassert.ok(isDisabled('#aichat-clear-btn button'));
		});

		it('send is disabled while the input is empty', async function () {
			await openWithAnswer();

			nodeassert.ok(isDisabled('#aichat-send-btn button'));
		});

		it('send is enabled once the input has text', async function () {
			await openWithAnswer();

			typeInto(one(input), 'Hello');
			await settleAIChat();

			nodeassert.ok(!isDisabled('#aichat-send-btn button'));
		});

		it('send is disabled again when the input is cleared', async function () {
			await openWithAnswer();
			typeInto(one(input), 'Hello');
			await settleAIChat();
			nodeassert.ok(!isDisabled('#aichat-send-btn button'));

			clearField(one(input));
			typeInto(one(input), '');
			await settleAIChat();

			nodeassert.ok(isDisabled('#aichat-send-btn button'));
		});

		it('the input shows a placeholder', async function () {
			await openWithAnswer();

			nodeassert.strictEqual(
				one(input).getAttribute('placeholder'),
				'Ask AI...',
			);
		});

		it('a sent message appears as a user message', async function () {
			await openWithAnswer({ content: 'Hello from AI' });

			await send('Hello');

			const message = one('#aichat-msg-0');
			nodeassert.ok(message.classList.contains('aichat-msg-user'));
			nodeassert.ok(message.textContent.includes('Hello'));
		});

		it('the answer appears as an assistant message', async function () {
			await openWithAnswer({ content: 'Hello from AI' });

			await send('Hello');

			const message = one('#aichat-msg-1');
			nodeassert.ok(message.classList.contains('aichat-msg-assistant'));
			nodeassert.ok(message.textContent.includes('Hello from AI'));
		});

		it('the input is empty after a send', async function () {
			await openWithAnswer({ content: 'Hello from AI' });

			await send('Hello');

			one('#aichat-msg-0');
			nodeassert.strictEqual((one(input) as HTMLTextAreaElement).value, '');
		});

		it('clear is enabled once the conversation has messages', async function () {
			await openWithAnswer({ content: 'Hello from AI' });

			await send('Hello');

			one('#aichat-msg-1');
			nodeassert.ok(!isDisabled('#aichat-clear-btn button'));
		});

		it('a suggestion sends its prompt', async function () {
			await openWithAnswer({ content: 'Chip response' });

			await clickOn('#aichat-chip-0 button');

			const message = one('#aichat-msg-0');
			nodeassert.ok(message.classList.contains('aichat-msg-user'));
			nodeassert.ok(message.textContent.includes('Make it more concise'));
			const requests = sentAIRequests();
			nodeassert.strictEqual(requests.length, 1);
			nodeassert.ok(
				requests[0].messages[0].content.includes('Make it more concise'),
			);
		});

		it('the suggestions and the empty state go after a send', async function () {
			await openWithAnswer({ content: 'Chip response' });

			await clickOn('#aichat-chip-0 button');

			one('#aichat-msg-0');
			nodeassert.strictEqual(all('#aichat-chips').length, 0);
			nodeassert.strictEqual(all('#aichat-empty-state').length, 0);
		});

		it('clear brings back the empty state', async function () {
			await openWithAnswer({ content: 'Response' });
			await send('Hello');
			one('#aichat-msg-1');

			await clickOn('#aichat-clear-btn button');

			one('#aichat-empty-state');
			one('#aichat-chips');
			nodeassert.strictEqual(all('#aichat-msg-0').length, 0);
			nodeassert.ok(isDisabled('#aichat-clear-btn button'));
		});

		it('an assistant message offers copy and insert, a user message offers neither', async function () {
			await openWithAnswer({ content: 'Copyable text' });

			await send('Hello');

			one('#aichat-copy-text-1');
			one('#aichat-insert-text-1');
			nodeassert.strictEqual(all('#aichat-copy-text-0').length, 0);
			nodeassert.strictEqual(all('#aichat-insert-text-0').length, 0);
		});

		it('a failed answer shows as an error message with a retry button', async function () {
			await openWithAnswer({ success: false, error: 'Something went wrong' });

			await send('Hello');

			const message = one('#aichat-msg-1');
			nodeassert.ok(message.classList.contains('aichat-msg-error'));
			nodeassert.ok(message.textContent.includes('Error:'));
			one('#aichat-retry-1');
		});

		it('retry sends the failed message again as one user message', async function () {
			await openWithAnswer({ success: false, error: 'boom' });
			// The failed first turn stays, so the list is updated in place.
			await send('First');
			await send('Second');
			one('#aichat-msg-3');

			await clickOn('#aichat-retry-3 button');

			nodeassert.strictEqual(all('[id="aichat-msg-2"]').length, 1);
			nodeassert.ok(one('#aichat-msg-2').classList.contains('aichat-msg-user'));
			const requests = sentAIRequests();
			nodeassert.strictEqual(requests.length, 3);
			const resent = requests[2].messages.filter(
				(message: any) => message.content === 'Second',
			);
			nodeassert.strictEqual(resent.length, 1);
		});

		it('retry after an approved request failed asks for approval again', async function () {
			await openWithAnswer({
				approvalToolName: 'extract_document_structure',
				success: false,
				error: 'boom',
			});
			await send('Summarize');
			nodeassert.ok(
				one('#aichat-msg-1').classList.contains('aichat-msg-approval'),
			);
			await clickOn('.aichat-approve-btn');
			nodeassert.ok(
				one('#aichat-msg-2').classList.contains('aichat-msg-error'),
			);

			await clickOn('#aichat-retry-2 button');

			nodeassert.ok(one('#aichat-msg-0').classList.contains('aichat-msg-user'));
			nodeassert.ok(
				one('#aichat-msg-1').classList.contains('aichat-msg-approval'),
			);
			nodeassert.strictEqual(
				all('#aichat-messages-list > [id^="aichat-msg-"]').length,
				2,
			);
			nodeassert.strictEqual(all('.aichat-msg-error').length, 0);
		});

		it('Escape in the input closes the sidebar', async function () {
			await openWithAnswer();

			await pressKey(input, 'Escape');

			nodeassert.ok(!isOpen());
		});

		it('Enter in the input sends the message', async function () {
			await openWithAnswer({ content: 'Keyboard response' });
			typeInto(one(input), 'Hello');

			await pressKey(input, 'Enter');

			nodeassert.ok(one('#aichat-msg-0').classList.contains('aichat-msg-user'));
			const requests = sentAIRequests();
			nodeassert.strictEqual(requests.length, 1);
			nodeassert.deepStrictEqual(requests[0].messages, [
				{ role: 'user', content: 'Hello' },
			]);
		});

		it('Shift+Enter in the input does not send', async function () {
			await openWithAnswer({ content: 'Keyboard response' });
			typeInto(one(input), 'Hello');

			await pressKey(input, 'Enter', true);

			nodeassert.strictEqual(all('#aichat-msg-0').length, 0);
			nodeassert.strictEqual(
				(one(input) as HTMLTextAreaElement).value,
				'Hello',
			);
		});
	});

	describe('tone of voice', function () {
		const input = '#aichat-input.ui-textarea';
		const pickerOpen = '#aichat-tone-picker.aichat-tone-picker-open';
		const prefs = () => (window as any).prefs;

		function pressed(selector: string): string {
			return one(selector).getAttribute('aria-pressed');
		}

		async function openPicker(): Promise<void> {
			await clickOn('#aichat-tone-chip button.ui-pushbutton');
			one(pickerOpen);
		}

		async function addCustomTone(): Promise<void> {
			await clickOn('#aichat-tone-add button');
			typeInto(one('#aichat-tone-form-name input'), 'Curt');
			typeInto(one('#aichat-tone-form-desc.ui-textarea'), 'Be terse');
			await settleAIChat();
			await clickOn('#aichat-tone-form-save button');
		}

		async function openEmojiSearch(keyword: string): Promise<void> {
			await clickOn('#aichat-tone-add button');
			await clickOn('#aichat-tone-form-emoji-more button');
			typeInto(
				one('#aichat-tone-form-emoji-picker .ui-emoji-picker-search'),
				keyword,
			);
			await settleAIChat();
		}

		// The smiling face with smiling eyes is keyworded "smile happy blush". A new voice
		// starts with it as its icon.
		const smile = '\u{1F60A}';
		// The winking face is keyworded "wink flirt".
		const wink = '\u{1F609}';

		function emojiCell(emoji: string): HTMLElement {
			return all('#aichat-tone-form-emoji-picker .ui-emoji-picker-cell').find(
				(cell) => cell.textContent.includes(emoji),
			);
		}

		beforeEach(async function () {
			answerAIRequests({ content: 'Reply' });
			await toggleAIChat(sidebar);
		});

		it('the picker offers the six preset tones', async function () {
			await openPicker();

			for (const tone of [
				'natural',
				'formal',
				'short',
				'friendly',
				'professional',
				'casual',
			])
				one('#aichat-tone-preset-' + tone);
		});

		it('choosing a preset closes the picker and names it on the chip', async function () {
			await openPicker();

			await clickOn('#aichat-tone-preset-formal .ui-chip-main');

			nodeassert.strictEqual(all(pickerOpen).length, 0);
			nodeassert.ok(
				one('#aichat-tone-chip button.ui-pushbutton').textContent.includes(
					'Formal',
				),
			);
		});

		it('the chosen preset is kept in the preferences', async function () {
			await openPicker();

			await clickOn('#aichat-tone-preset-friendly .ui-chip-main');

			nodeassert.strictEqual(prefs().get('aichat.selectedTone'), 'friendly');
		});

		it('the chosen preset is shown as pressed in the picker', async function () {
			await openPicker();
			await clickOn('#aichat-tone-preset-professional .ui-chip-main');

			await openPicker();

			nodeassert.strictEqual(
				pressed('#aichat-tone-preset-professional .ui-chip-main'),
				'true',
			);
			nodeassert.strictEqual(
				pressed('#aichat-tone-preset-natural .ui-chip-main'),
				'false',
			);
		});

		it('Emojify toggles its pressed state and is kept in the preferences', async function () {
			await openPicker();
			const emojify = '#aichat-tone-emojify button.ui-pushbutton';
			nodeassert.strictEqual(pressed(emojify), 'false');

			await clickOn(emojify);

			nodeassert.strictEqual(pressed(emojify), 'true');
			nodeassert.strictEqual(prefs().get('aichat.emojify'), 'true');
		});

		it('a document that fails to load keeps the saved tone', async function () {
			await openPicker();
			await clickOn('#aichat-tone-preset-formal .ui-chip-main');
			nodeassert.strictEqual(prefs().get('aichat.selectedTone'), 'formal');

			map.fire('docloaded', { status: false });
			await settleAIChat();

			nodeassert.ok(!sidebar.isVisible());
			nodeassert.strictEqual(prefs().get('aichat.selectedTone'), 'formal');
		});

		it('Add my own voice opens an empty form that cannot be saved yet', async function () {
			await openPicker();

			await clickOn('#aichat-tone-add button');

			one('#aichat-tone-form');
			one('#aichat-tone-form-name input');
			nodeassert.ok(
				one('#aichat-tone-form-save button').hasAttribute('disabled'),
			);
		});

		it('save is enabled once the name and the description are filled in', async function () {
			await openPicker();
			await clickOn('#aichat-tone-add button');

			typeInto(one('#aichat-tone-form-name input'), 'Curt');
			typeInto(one('#aichat-tone-form-desc.ui-textarea'), 'Be terse');
			await settleAIChat();

			nodeassert.ok(
				!one('#aichat-tone-form-save button').hasAttribute('disabled'),
			);
		});

		it('a saved voice is added as a chip and chosen', async function () {
			await openPicker();

			await addCustomTone();

			nodeassert.strictEqual(
				all('.ui-chip[id^="aichat-tone-custom-"]').length,
				1,
			);
			nodeassert.strictEqual(
				pressed('.ui-chip[id^="aichat-tone-custom-"] .ui-chip-main'),
				'true',
			);
		});

		it('editing a saved voice opens the form with its values', async function () {
			await openPicker();
			await addCustomTone();

			await clickOn('[id^="aichat-tone-edit-"]');

			nodeassert.strictEqual(
				(one('#aichat-tone-form-name input') as HTMLInputElement).value,
				'Curt',
			);
			nodeassert.strictEqual(
				(one('#aichat-tone-form-desc.ui-textarea') as HTMLTextAreaElement)
					.value,
				'Be terse',
			);
			one('#aichat-tone-form-delete button');
		});

		it('a confirmed delete removes the saved voice', async function () {
			await openPicker();
			await addCustomTone();
			await clickOn('[id^="aichat-tone-edit-"]');

			await clickOn('#aichat-tone-form-delete button');
			one('#aichat-tone-delete-confirm');
			await clickOn('#aichat-tone-delete-confirm-ok button');

			nodeassert.strictEqual(
				all('.ui-chip[id^="aichat-tone-custom-"]').length,
				0,
			);
		});

		it('cancelling the delete goes back to the edit form', async function () {
			await openPicker();
			await addCustomTone();
			await clickOn('[id^="aichat-tone-edit-"]');
			await clickOn('#aichat-tone-form-delete button');

			await clickOn('#aichat-tone-delete-confirm-cancel button');

			nodeassert.strictEqual(all('#aichat-tone-delete-confirm').length, 0);
			one('#aichat-tone-form-delete button');
			nodeassert.strictEqual(
				(one('#aichat-tone-form-name input') as HTMLInputElement).value,
				'Curt',
			);
		});

		it('the plus button opens the emoji picker with a search field', async function () {
			await openPicker();
			await clickOn('#aichat-tone-add button');

			await clickOn('#aichat-tone-form-emoji-more button');

			one('#aichat-tone-form-emoji-picker .ui-emoji-picker-search');
		});

		it('the emoji search finds an emoji by its keyword', async function () {
			await openPicker();

			await openEmojiSearch('smile');

			nodeassert.ok(emojiCell(smile));
			nodeassert.strictEqual(emojiCell(wink), undefined);
		});

		it('a picked emoji closes the picker and fills the first icon slot', async function () {
			await openPicker();
			await openEmojiSearch('wink');

			emojiCell(wink).click();
			await settleAIChat();

			nodeassert.strictEqual(all('#aichat-tone-form-emoji-picker').length, 0);
			const slot = '#aichat-tone-form-icon-0 button.ui-pushbutton';
			nodeassert.strictEqual(pressed(slot), 'true');
			nodeassert.ok(one(slot).textContent.includes(wink));
		});

		it('a request carries the chosen preset', async function () {
			await openPicker();
			await clickOn('#aichat-tone-preset-formal .ui-chip-main');

			typeInto(one(input), 'Hello');
			await clickOn('#aichat-send-btn button');

			const requests = sentAIRequests();
			const last = requests[requests.length - 1];
			nodeassert.strictEqual(last.tone, 'formal');
			nodeassert.strictEqual(last.customToneDescription, undefined);
			nodeassert.strictEqual(last.emojify, false);
		});

		it('a request with a saved voice carries its description', async function () {
			await openPicker();
			await addCustomTone();

			typeInto(one(input), 'Hello');
			await clickOn('#aichat-send-btn button');

			const requests = sentAIRequests();
			const last = requests[requests.length - 1];
			nodeassert.strictEqual(last.tone, 'custom');
			nodeassert.strictEqual(last.customToneDescription, 'Be terse');
		});

		it('Escape closes the picker and leaves the sidebar open', async function () {
			await openPicker();

			await pressKey(input, 'Escape');

			nodeassert.strictEqual(all(pickerOpen).length, 0);
			one('#aichat-dock-wrapper.visible');
		});

		it('each Escape closes one layer: emoji picker, form, picker, sidebar', async function () {
			await openPicker();
			await clickOn('#aichat-tone-add button');
			await clickOn('#aichat-tone-form-emoji-more button');
			one('#aichat-tone-form-emoji-picker');

			await pressKey(input, 'Escape');
			nodeassert.strictEqual(all('#aichat-tone-form-emoji-picker').length, 0);
			one('#aichat-tone-form');

			await pressKey(input, 'Escape');
			nodeassert.strictEqual(all('#aichat-tone-form').length, 0);
			one(pickerOpen);

			await pressKey(input, 'Escape');
			nodeassert.strictEqual(all(pickerOpen).length, 0);
			one('#aichat-dock-wrapper.visible');

			await pressKey(input, 'Escape');
			nodeassert.strictEqual(all('#aichat-dock-wrapper.visible').length, 0);
		});
	});

	describe('formula help', function () {
		beforeEach(function () {
			map.docType = 'spreadsheet';
		});

		async function requestFormula(): Promise<void> {
			answerAIRequests({ approvalToolName: 'set_cell_formula' });
			await sendAIRequest(sidebar, 'Average column A');
		}

		it('the card says the spreadsheet is about to change', async function () {
			await requestFormula();

			nodeassert.ok(
				aiMessagesText().includes('wants to set a formula in your spreadsheet'),
			);
		});

		it('approving the card answers approve', async function () {
			await requestFormula();

			await clickOn('.aichat-approve-btn');

			const decisions = sentAIApprovals();
			nodeassert.strictEqual(decisions.length, 1);
			nodeassert.strictEqual(decisions[0].action, 'approve');
		});

		it('rejecting the card answers reject', async function () {
			await requestFormula();

			await clickOn('.aichat-reject-btn');

			const decisions = sentAIApprovals();
			nodeassert.strictEqual(decisions.length, 1);
			nodeassert.strictEqual(decisions[0].action, 'reject');
		});

		it('four suggestions show, and See more shows the rest', async function () {
			answerAIRequests();
			await toggleAIChat(sidebar);

			nodeassert.ok(
				one('#aichat-empty-state').textContent.includes('this spreadsheet'),
			);
			one('#aichat-chip-formula-diagnosis');
			one('#aichat-chip-sanity-check');
			one('#aichat-chip-create-formula');
			one('#aichat-chip-0');
			nodeassert.strictEqual(all('#aichat-chip-3').length, 0);

			await clickOn('#aichat-see-more button');

			one('#aichat-chip-3');
			nodeassert.ok(one('#aichat-see-more').textContent.includes('See less'));
		});

		it('See less hides the extra suggestions again', async function () {
			answerAIRequests();
			await toggleAIChat(sidebar);
			await clickOn('#aichat-see-more button');
			one('#aichat-chip-3');

			await clickOn('#aichat-see-more button');

			nodeassert.strictEqual(all('#aichat-chip-3').length, 0);
			nodeassert.ok(one('#aichat-see-more').textContent.includes('See more'));
		});
	});

	describe('template picker', function () {
		// A single-slide presentation is a fresh deck, where a design can still be chosen.
		beforeEach(function () {
			map.docType = 'presentation';
		});

		it('a question goes out with no design picker in the way', async function () {
			answerAIRequests();

			await sendAIRequest(sidebar, 'What does this slide say?');

			nodeassert.ok(aiMessagesText().includes('Mock AI response'));
			const requests = sentAIRequests();
			nodeassert.strictEqual(requests.length, 1);
			nodeassert.strictEqual(requests[0].designTemplate, undefined);
			nodeassert.strictEqual(all('.aichat-design-section').length, 0);
		});

		it('no design is the default and the approval carries none', async function () {
			answerAIRequests({
				outline: {
					title: 'Cats',
					slides: [
						{
							part: 'opening',
							intent: 'title',
							title: 'Cats',
							gist: 'Set the stage.',
						},
						{
							part: 'closing',
							intent: 'closing',
							title: 'Thanks',
							gist: 'Wrap up.',
						},
					],
				},
			});
			await sendAIRequest(sidebar, 'Create a deck about cats');
			one('.aichat-outline-card .aichat-design-section');
			nodeassert.ok(
				aiChatSentMessages.includes(
					'commandvalues command=.uno:GetDesignTemplates',
				),
			);
			map.fire('commandvalues', {
				commandName: '.uno:GetDesignTemplates',
				commandValues: { templates: [{ name: 'Cobalt' }] },
			});
			await settleAIChat();

			nodeassert.strictEqual(
				one('.aichat-template-skip').getAttribute('aria-pressed'),
				'true',
			);
			nodeassert.strictEqual(
				one('.aichat-template-card[title="Cobalt"]').getAttribute(
					'aria-pressed',
				),
				'false',
			);

			await clickOn('.aichat-approve-btn');

			const decision = sentAIApprovals()[0];
			nodeassert.strictEqual(decision.action, 'approve');
			nodeassert.strictEqual(decision.designTemplate, undefined);
		});

		it('an edit approval offers no designs', async function () {
			answerAIRequests({
				approvalToolName: 'transform_document_structure',
				approvalTransformJson: JSON.stringify({
					Transforms: { SlideCommands: [{ DeleteSlide: 1 }] },
				}),
			});

			await sendAIRequest(sidebar, 'Delete the second slide');

			one('.aichat-approve-btn');
			nodeassert.strictEqual(all('.aichat-design-section').length, 0);
		});
	});
});
