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

class A11yValidatorException extends Error {
	static readonly PREFIX: string = 'A11yValidatorException';

	constructor(message: string) {
		super(message);
		this.name = A11yValidatorException.PREFIX;
		// Fix prototype chain for TypeScript extending built-in classes
		Object.setPrototypeOf(this, A11yValidatorException.prototype);
	}
}

class A11yValidator {
	private checks: Array<(type: string, element: HTMLElement) => void> = [];
	private _directlyValidatedElements: Set<Element> | null = null;

	constructor() {
		this.setupChecks();
	}

	private setupChecks(): void {
		this.checks.push(this.checkNativeButtonElement.bind(this));
		this.checks.push(this.checkImageAltAttribute.bind(this));
		this.checks.push(this.checkLabelElement.bind(this));
		this.checks.push(this.checkElementHasLabel.bind(this));
		this.checks.push(this.checkAriaControls.bind(this));
		this.checks.push(this.checkFrameOnlyDecorativeImages.bind(this));
	}

	checkWidget(type: string, element: HTMLElement): void {
		for (const check of this.checks) {
			try {
				check(type, element);
			} catch (error) {
				if (error instanceof A11yValidatorException) console.error(error);
				throw error;
			}
		}
	}

	private checkNativeButtonElement(type: string, element: HTMLElement): void {
		if (
			element.tagName !== 'BUTTON' &&
			element.getAttribute('role') === 'button'
		) {
			throw new A11yValidatorException(
				`In '${this.getDialogTitle(element)}' at '${this.getElementPath(element)}': widget of type '${type}' has ${element.tagName} element with role="button". It should use native <button> element instead.`,
			);
		}

		for (let i = 0; i < element.children.length; i++) {
			const child = element.children[i];
			if (this.shouldCheckChild(child)) {
				this.checkNativeButtonElement(type, child as HTMLElement);
			}
		}
	}

	private checkImageAltAttribute(type: string, element: HTMLElement): void {
		const images = element.querySelectorAll('img');

		images.forEach((img, index) => {
			const hasAlt = img.hasAttribute('alt');
			const altValue = img.getAttribute('alt');

			if (!this.isVisible(img)) return; // skip hidden images

			if (!hasAlt) {
				throw new A11yValidatorException(
					`In '${this.getDialogTitle(element)}' at '${this.getElementPath(img)}': image in widget of type '${type}' is missing alt attribute`,
				);
			}

			const parent = img.parentElement;
			const span =
				parent && (parent.querySelector('span.unolabel') as HTMLSpanElement);
			const explicitLabel = span && span.innerText.trim().length > 0;
			const visibleLabel =
				parent && document.querySelector(`label[for="${parent.id}"]`);

			const parentHasLabel =
				parent &&
				(parent.hasAttribute('aria-label') ||
					parent.hasAttribute('aria-labelledby') ||
					visibleLabel ||
					explicitLabel);

			const isFocusable = img.tabIndex === 0;

			if (altValue === '' && parent) {
				const isDecorativeImg = img.classList.contains('ui-decorative-image'); // exclude ui-decorative-image decorative images - they can have empty alt

				if (isFocusable) {
					throw new A11yValidatorException(
						`In '${this.getDialogTitle(element)}' at '${this.getElementPath(img)}': focusable image in widget of type '${type}' has empty alt attribute (screen readers need alt text for focusable images) or make it decorative by removing tabIndex`,
					);
				}

				if (!parentHasLabel && !isDecorativeImg) {
					throw new A11yValidatorException(
						`In '${this.getDialogTitle(element)}' at '${this.getElementPath(img)}': image in widget of type '${type}' has empty alt attribute but parent element lacks label`,
					);
				}
			}

			if (altValue !== '' && parentHasLabel && !isFocusable) {
				throw new A11yValidatorException(
					`In '${this.getDialogTitle(element)}' at '${this.getElementPath(img)}': image in widget of type '${type}' has non-empty alt attribute but parent element also has label (should not duplicate)`,
				);
			}
		});
	}

	private checkElementHasLabel(type: string, element: HTMLElement): void {
		if (element.hasAttribute('aria-labelledby')) {
			const labelledbyValue = element.getAttribute('aria-labelledby') as string;
			const labelledbyIds = labelledbyValue.trim().split(/\s+/);

			for (const labelledbyId of labelledbyIds) {
				const referencedElement = document.getElementById(labelledbyId);

				if (!referencedElement) {
					throw new A11yValidatorException(
						`In '${this.getDialogTitle(element)}' at '${this.getElementPath(element)}': element in widget of type '${type}' has aria-labelledby attribute pointing to non-existing element with id: '${labelledbyId}'`,
					);
				}

				// Only flag htmlFor duplication for single-label case.
				// Multi-label (grid pattern) legitimately needs both
				// htmlFor (click-to-focus) and aria-labelledby
				// (composite accessible name).
				if (labelledbyIds.length === 1) {
					const labelHasHtmlFor =
						referencedElement.tagName === 'LABEL' &&
						(referencedElement as HTMLLabelElement).htmlFor;

					const htmlForPointsToThisElement =
						labelHasHtmlFor &&
						(referencedElement as HTMLLabelElement).htmlFor === element.id;

					if (htmlForPointsToThisElement) {
						throw new A11yValidatorException(
							`In '${this.getDialogTitle(element)}' at '${this.getElementPath(element)}': element in widget of type '${type}' has aria-labelledby attribute pointing to label element with id: '${labelledbyId}', but that label also has htmlFor attribute pointing to this element. Should not duplicate labelling.`,
						);
					}
				}
			}
		} else {
			const visibleLabel = document.querySelector(`label[for="${element.id}"]`);
			if (!visibleLabel) {
				const ariaLabel = element.getAttribute('aria-label') ?? '';
				const hasAriaLabel = ariaLabel.trim() !== '';

				if (
					JSDialog.GetFormControlTypesInCO().has(element.tagName) &&
					!hasAriaLabel
				) {
					throw new A11yValidatorException(
						`In '${this.getDialogTitle(element)}' at '${this.getElementPath(element)}': element in widget of type '${type}' is missing label: it should have either <label>, aria-labelledby or aria-label attribute.`,
					);
				}
			}
		}

		for (let i = 0; i < element.children.length; i++) {
			const child = element.children[i];
			if (this.shouldCheckChild(child)) {
				this.checkElementHasLabel(type, child as HTMLElement);
			}
		}
	}

	private checkLabelElement(type: string, element: HTMLElement): void {
		if (element.tagName === 'LABEL') {
			const htmlFor = (element as HTMLLabelElement).htmlFor?.trim();
			if (htmlFor) {
				const referencedElement = document.getElementById(htmlFor);
				if (!referencedElement) {
					throw new A11yValidatorException(
						`In '${this.getDialogTitle(element)}' at '${this.getElementPath(element)}': label element in widget of type '${type}' has htmlFor attribute pointing to non-existing element with id '${htmlFor}'`,
					);
				} else if (
					!JSDialog.GetFormControlTypesInCO().has(referencedElement.tagName)
				) {
					throw new A11yValidatorException(
						`In '${this.getDialogTitle(element)}' at '${this.getElementPath(element)}': label element in widget of type '${type}' references non-labelable element <${referencedElement.tagName.toLowerCase()}> via htmlFor attribute. Try using aria-labelledby on the referenced element instead.`,
					);
				} else if (
					referencedElement.hasAttribute('aria-labelledby') ||
					referencedElement.hasAttribute('aria-label')
				) {
					// Allow when the target has multi-label aria-labelledby
					// (space-separated IDs indicate a grid-layout pattern
					// where htmlFor provides click-to-focus and
					// aria-labelledby provides the composite accessible name).
					const ariaLabelledBy =
						referencedElement.getAttribute('aria-labelledby');
					const isMultiLabel =
						ariaLabelledBy && ariaLabelledBy.trim().split(/\s+/).length > 1;

					if (!isMultiLabel) {
						throw new A11yValidatorException(
							`In '${this.getDialogTitle(element)}' at '${this.getElementPath(element)}': label element in widget of type '${type}' is associated with element with id: '${htmlFor}' via htmlFor, but that element also has aria-label or aria-labelledby attribute. Should not duplicate labelling.`,
						);
					}
				}
			} else {
				const referencedElement = document.querySelector(
					`[aria-labelledby~="${element.id}"]`,
				);
				if (!referencedElement) {
					throw new A11yValidatorException(
						`In '${this.getDialogTitle(element)}' at '${this.getElementPath(element)}': label element in widget of type '${type}' is not associated with any element via htmlFor or aria-labelledby. Should this element really be a label? If it just represent static text then try converting it into a <span> element instead.`,
					);
				}
			}
		}
	}

	// TODO: there are some elements on which aria-controls only added
	// when the relevant element exist in DOM. Need to handle that case as well.
	private checkAriaControls(type: string, element: HTMLElement): void {
		const controlledElementId = element.getAttribute('aria-controls') || '';
		if (controlledElementId.trim() !== '') {
			const referencedElement = document.getElementById(controlledElementId);

			if (!referencedElement) {
				throw new A11yValidatorException(
					`In '${this.getDialogTitle(element)}' at '${this.getElementPath(element)}': element is widget of type '${type}' has aria-control attribute but mentioned element does not exist in DOM. Only add this attribute when mentioned element exist in DOM.`,
				);
			}
		}

		for (let i = 0; i < element.children.length; i++) {
			const child = element.children[i];
			if (this.shouldCheckChild(child)) {
				this.checkAriaControls(type, child as HTMLElement);
			}
		}
	}

	private checkFrameOnlyDecorativeImages(
		type: string,
		element: HTMLElement,
	): void {
		if (
			!element.classList.contains('ui-frame-container') ||
			!element.classList.contains('ui-fieldset')
		) {
			return;
		}

		const content = element.querySelector('.ui-expander-content');
		if (!content) return;

		const decorativeImages = content.querySelectorAll(
			'img.ui-decorative-image',
		);
		if (decorativeImages.length === 0) return;

		// If there is any focusable element, the frame has accessible content
		if (JSDialog.FindFocusableWithin(content, 'next')) return;

		// If there are any form controls (even disabled), the frame has
		// real content rather than only decorative images
		const formControlTags = JSDialog.GetFormControlTypesInCO();
		for (const tag of formControlTags) {
			if (content.querySelector(tag)) return;
		}

		throw new A11yValidatorException(
			`In '${this.getDialogTitle(element)}' at '${this.getElementPath(element)}': frame '${type}' contains only decorative images with no accessible content. Remove the frame so its label does not mislead users into expecting meaningful content.`,
		);
	}

	private checkInitialFocusNotCloseButton(dialogElement: HTMLElement): number {
		const active = document.activeElement;
		if (!active || !dialogElement.contains(active)) return 0;

		if (active.classList.contains('ui-dialog-titlebar-close')) {
			console.error(
				new A11yValidatorException(
					`In '${this.getDialogTitle(active as HTMLElement)}': initial keyboard focus is on the close (X) button in the titlebar. Focus should be on a control inside the dialog body.`,
				),
			);
			return 1;
		}
		return 0;
	}

	private checkDuplicateButtonLabels(container: HTMLElement): number {
		const buttons = container.querySelectorAll('button[aria-labelledby]');
		const labelMap = new Map<string, HTMLElement[]>();

		buttons.forEach((btn) => {
			const labelledBy = btn.getAttribute('aria-labelledby')?.trim();
			if (!labelledBy) return;

			// Skip hidden buttons (e.g. inside collapsed sidebar panels).
			// They are not reachable by users or screen readers.
			if (!this.isVisible(btn as HTMLElement)) return;

			if (!labelMap.has(labelledBy)) {
				labelMap.set(labelledBy, []);
			}
			labelMap.get(labelledBy)?.push(btn as HTMLElement);
		});

		let errorCount = 0;

		for (const [labelId, btns] of labelMap) {
			if (btns.length > 1) {
				const ids = btns.map((b) => b.id || '(no id)').join(', ');
				console.error(
					new A11yValidatorException(
						`In '${this.getDialogTitle(container)}': buttons [${ids}] share the same aria-labelledby="${labelId}". Each button must have a distinct accessible name to clearly convey its function.`,
					),
				);
				errorCount++;
			}
		}

		return errorCount;
	}

	private shouldCheckChild(child: Element): boolean {
		return (
			child instanceof HTMLElement &&
			!this._directlyValidatedElements?.has(child)
		);
	}

	private isVisible(element: HTMLElement): boolean {
		const style = getComputedStyle(element);
		if (style.visibility === 'hidden') return false;

		return element.getClientRects().length > 0;
	}

	private getDialogTitle(element: HTMLElement): string {
		const dialog = element.closest('.ui-dialog');
		if (!dialog) return 'unknown dialog';

		const title = dialog.querySelector('h2.ui-dialog-title');
		return title?.textContent?.trim() || 'untitled dialog';
	}

	private getElementPath(element: HTMLElement): string {
		const ids: string[] = [];
		let current: HTMLElement | null = element;
		const dialog = element.closest('.ui-dialog');

		while (current && current !== dialog) {
			if (current.id) {
				ids.unshift(current.id);
			}
			current = current.parentElement;
		}

		return ids.length > 0 ? ids.join(' > ') : '(no ids in path)';
	}

	validateContainer(
		dialogElement: HTMLElement,
		extraElement?: HTMLElement,
	): number {
		// Find all widgets in the dialog that have an id
		const widgets = dialogElement.querySelectorAll('[id]');
		let errorCount = 0;

		// Build a set of widget elements so that the recursive checks
		// (checkNativeButtonElement, checkElementHasLabel) can skip
		// children that will be validated individually.  Without this,
		// every parent re-walks all descendants → O(n²).
		this._directlyValidatedElements = new Set(Array.from(widgets));

		widgets.forEach((widget) => {
			if (widget instanceof HTMLElement) {
				const widgetType = widget.getAttribute('id') || 'unknown';
				try {
					this.checkWidget(widgetType, widget);
				} catch (error) {
					errorCount++;
					// Error already logged in checkWidget
				}
			}
		});

		if (extraElement && !this._directlyValidatedElements.has(extraElement)) {
			try {
				this.checkWidget('dialog-content', extraElement);
			} catch (error) {
				errorCount++;
			}
		}

		errorCount += this.checkDuplicateButtonLabels(dialogElement);

		this._directlyValidatedElements = null;
		return errorCount;
	}

	validateDialog(dialogElement: HTMLElement): void {
		const content = dialogElement.querySelector('.ui-dialog-content');

		const errorCount =
			this.validateContainer(
				dialogElement,
				content instanceof HTMLElement ? content : undefined,
			) + this.checkInitialFocusNotCloseButton(dialogElement);

		if (errorCount === 0) {
			console.error('A11yValidator: dialog passed all checks');
		} else {
			console.error(
				`A11yValidator: dialog has ${errorCount} accessibility issues`,
			);
		}
	}

	validateIframeDialog(container: HTMLElement): void {
		const iframe = container.querySelector('iframe');
		if (!iframe || !iframe.contentDocument || !iframe.contentDocument.body)
			return;

		this.validateDialog(iframe.contentDocument.body);
	}

	validateAllOpenDialogs(): void {
		const jsdialog = app.map?.jsdialog;
		if (!jsdialog || !jsdialog.dialogs) {
			console.error('A11yValidator: no jsdialog manager found');
			return;
		}

		const dialogIds = Object.keys(jsdialog.dialogs);
		if (dialogIds.length === 0) {
			console.error('A11yValidator: no open dialogs to validate');
			return;
		}

		for (const dialogId of dialogIds) {
			const dialogInfo = jsdialog.dialogs[dialogId];
			if (dialogInfo && dialogInfo.container) {
				this.validateDialog(dialogInfo.container);
			}
		}
	}

	validateSidebar(): void {
		const currentSidebar = app.map?.sidebar;
		if (!currentSidebar) {
			console.error('A11yValidator: no open sidebar to validate');
			return;
		}

		const container = currentSidebar.getContainer();
		Util.ensureValue(container);
		const errorCount = this.validateContainer(container);

		if (errorCount === 0) {
			console.error('A11yValidator: sidebar passed all checks');
		} else {
			console.error(
				`A11yValidator: sidebar has ${errorCount} accessibility issues`,
			);
		}
	}

	validateNotebookbar(): void {
		const notebookbar = app.map?.uiManager?.notebookbar;
		if (!notebookbar) {
			console.error('A11yValidator: no notebookbar to validate');
			return;
		}

		let errorCount = this.validateContainer(notebookbar.container);
		errorCount += this.checkTabContainerConsistency(notebookbar);
		errorCount += this.checkOverflowGroupChildIds(notebookbar);

		if (errorCount === 0) {
			console.error('A11yValidator: notebookbar passed all checks');
		} else {
			console.error(
				`A11yValidator: notebookbar has ${errorCount} accessibility issues`,
			);
		}
	}

	private checkTabContainerConsistency(notebookbar: any): number {
		const selectedTab = document.querySelector(
			'.ui-tab.notebookbar.selected',
		) as HTMLElement;
		if (!selectedTab || !selectedTab.id) return 0;

		const tabName = selectedTab.id.split('-')[0];
		const tabs = notebookbar.getTabs();
		const tabDef = tabs?.find((t: any) => t.id === selectedTab.id);

		let errorCount = 0;

		if (tabDef && tabDef.name && tabDef.name !== tabName) {
			console.error(
				new A11yValidatorException(
					`Tab '${tabDef.id}' has name '${tabDef.name}' which does not match its id prefix '${tabName}'. The name must match so that accessibility shortcuts are assigned to its buttons.`,
				),
			);
			errorCount++;
		}

		const container = notebookbar.container.querySelector(
			'#' + tabName + '-container',
		);
		if (!container) {
			console.error(
				new A11yValidatorException(
					`Selected tab '${selectedTab.id}' has no matching container '#${tabName}-container' in the DOM. Accessibility shortcuts for this tab's buttons will not work.`,
				),
			);
			errorCount++;
		}

		errorCount += this.checkDuplicateShortcuts(selectedTab.id);

		return errorCount;
	}

	private collectAllCombinations(
		node: WidgetJSON | WidgetJSON[],
		items: Array<{ id: string; combination: string }>,
		language: string | null,
	): void {
		if (!node) return;

		if (Array.isArray(node)) {
			for (const child of node) {
				this.collectAllCombinations(child, items, language);
			}
			return;
		}

		const isOverflow = node.type === 'overflowgroup';

		if (!isOverflow && node.accessibility && node.accessibility.combination) {
			const combo =
				language && node.accessibility[language]
					? (node.accessibility[language] as string)
					: node.accessibility.combination;
			const id = node.id || 'unknown';
			items.push({ id: id, combination: combo });
		}

		if (node.children && Array.isArray(node.children)) {
			for (const child of node.children) {
				this.collectAllCombinations(child, items, language);
			}
		}
	}

	private checkDuplicateShortcuts(selectedTabId: string): number {
		const notebookbar = (window as any).app?.map?.uiManager?.notebookbar;
		if (!notebookbar) return 0;

		const tabs = notebookbar.getTabs();
		const rawDefinitions = notebookbar.getFullJSON();
		if (!tabs || !rawDefinitions) return 0;

		// Find the ContextContainer
		let node = rawDefinitions;
		let contextContainer = null;
		while (
			node.children &&
			Array.isArray(node.children) &&
			node.children[0] &&
			!contextContainer
		) {
			if (node.children[0].id === 'ContextContainer')
				contextContainer = node.children[0];
			else node = node.children[0];
		}
		if (!contextContainer) return 0;

		// Find the selected tab's raw content
		const tabName = selectedTabId.split('-')[0];
		let rawContentList = null;
		for (const child of contextContainer.children) {
			if (
				child.children &&
				child.children[0] &&
				child.children[0].id === tabName + '-container'
			) {
				rawContentList = child.children[0].children;
				break;
			}
		}
		if (!rawContentList) return 0;

		const NbaDefs = (window as any).NotebookbarAccessibilityDefinitions;
		const language = NbaDefs ? new NbaDefs().getLanguage() : null;

		const items: Array<{ id: string; combination: string }> = [];
		this.collectAllCombinations(rawContentList, items, language);

		let errorCount = 0;

		for (let i = 0; i < items.length; i++) {
			for (let j = i + 1; j < items.length; j++) {
				const a = items[i];
				const b = items[j];
				if (a.combination === b.combination) {
					console.error(
						new A11yValidatorException(
							`Tab '${selectedTabId}' has duplicate shortcut '${a.combination}' on '${a.id}' and '${b.id}'. Each shortcut within a tab must be unique.`,
						),
					);
					errorCount++;
				} else if (a.combination.startsWith(b.combination)) {
					console.error(
						new A11yValidatorException(
							`Tab '${selectedTabId}': shortcut '${b.combination}' on '${b.id}' is a prefix of '${a.combination}' on '${a.id}', making '${a.id}' unreachable.`,
						),
					);
					errorCount++;
				} else if (b.combination.startsWith(a.combination)) {
					console.error(
						new A11yValidatorException(
							`Tab '${selectedTabId}': shortcut '${a.combination}' on '${a.id}' is a prefix of '${b.combination}' on '${b.id}', making '${b.id}' unreachable.`,
						),
					);
					errorCount++;
				}
			}
		}

		return errorCount;
	}

	private checkOverflowGroupChildIds(notebookbar: any): number {
		const selectedTab = document.querySelector(
			'.ui-tab.notebookbar.selected',
		) as HTMLElement;
		if (!selectedTab || !selectedTab.id) return 0;

		const tabName = selectedTab.id.split('-')[0];
		const containerId = tabName + '-container';

		const fullJSON = notebookbar.getFullJSON();
		const tabJSON = this.findJSONNodeById(fullJSON, containerId);
		if (!tabJSON) return 0;

		let errorCount = 0;

		const walk = (node: any): void => {
			if (!node || !node.children || !Array.isArray(node.children)) return;

			for (const child of node.children) {
				if (child.type === 'overflowgroup' && child.id) {
					this.findDuplicateIdInChildren(
						child.id,
						child.children,
						(dupId: string) => {
							console.error(
								new A11yValidatorException(
									`Overflow group '${dupId}' contains a child with the same id. This breaks accessibility shortcut resolution because querySelector matches the parent instead of the child.`,
								),
							);
							errorCount++;
						},
					);
				}
				walk(child);
			}
		};

		walk(tabJSON);
		return errorCount;
	}

	private findJSONNodeById(node: any, id: string): any {
		if (!node) return null;
		if (node.id === id) return node;
		if (node.children && Array.isArray(node.children)) {
			for (const child of node.children) {
				const found = this.findJSONNodeById(child, id);
				if (found) return found;
			}
		}
		return null;
	}

	private findDuplicateIdInChildren(
		parentId: string,
		children: any[],
		onDuplicate: (id: string) => void,
	): void {
		if (!children || !Array.isArray(children)) return;

		for (const child of children) {
			if (child.id === parentId) {
				onDuplicate(parentId);
				return;
			}
			if (child.children) {
				this.findDuplicateIdInChildren(parentId, child.children, onDuplicate);
			}
		}
	}
}

window.app.a11yValidator = new A11yValidator();
window.app.A11yValidatorException = A11yValidatorException;
