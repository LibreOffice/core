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

/*
 * Util.ModelState - will hold and update the state of a JSDialog model for a component
 *                   that will allow to be aware of previous updates even if we recreate
 *                   DOM nodes without any server communication (example: notebookbar switch)
 */

class JSDialogModelState {
	protected componentName: string;
	private model: WidgetJSON | null;

	constructor(componentName: string) {
		app.console.debug(
			'JSDialogModelState: created new for component: ' + componentName,
		);
		this.componentName = componentName;
		this.model = null;
	}

	private copyObject(obj: JSDialogJSON | WidgetJSON) {
		// use new API first
		if (window.structuredClone) {
			try {
				return window.structuredClone(obj);
			} catch (e) {
				app.console.debug('JSDialogModelState: ' + e);
			}
		}

		// try fallback
		try {
			app.console.debug('JSDialogModelState: fallback copy by JSON.parse');
			return JSON.parse(JSON.stringify(obj));
		} catch (e) {
			app.console.debug('JSDialogModelState: failed to clone the model: ' + e);
		}

		try {
			app.console.debug('JSDialogModelState: fallback copy by Object.assign');
			return Object.assign({}, obj);
		} catch (e) {
			app.console.debug('JSDialogModelState: ' + e);
		}

		return null;
	}

	public safeStringify(obj: any): string {
		try {
			return JSON.stringify(obj);
		} catch (e) {
			app.console.debug('JSDialogModelState: ' + e);
		}

		return 'Bad Object';
	}

	/// get model copy
	public getSnapshot() {
		if (this.model) {
			const snapshot = this.copyObject(this.model);
			if (snapshot) {
				return snapshot;
			} else {
				app.console.debug(
					'JSDialogModelState: cannot copy object, use original model',
				);
				return this.model;
			}
		}

		return null;
	}

	/// replaces complete state of a model
	public fullUpdate(data: JSDialogJSON) {
		app.console.debug(
			'JSDialogModelState: set model for component: ' + this.componentName,
		);

		this.model = data;
	}

	/// applies widget update to a model
	public applyUpdate(data: UpdateData) {
		if (!data || !data.control || !data.control.id) {
			app.console.error(
				'JSDialogModelState: bad syntax in widgetUpdate: ' +
					this.safeStringify(data),
			);
			return;
		}

		this.widgetUpdate(data.control);
	}

	/// applies widget update to a model
	public widgetUpdate(widget: WidgetJSON) {
		const found = this.getById(widget.id);
		if (found) {
			const id = found.id;
			const before = this.safeStringify(found);
			// data will no longer be used, we don't need deep copy
			Object.assign(found, widget);

			if (JSDialog.verbose) {
				app.console.debug(
					'JSDialogModelState: ' +
						this.componentName +
						' widgetUpdate\n\nBEFORE: ' +
						before +
						'\n\nAFTER: ' +
						this.safeStringify(this.getById(id)),
				);
			}
		}
	}

	/// applies action to a model
	public widgetAction(data: ActionData) {
		if (!data || !data.data || !data.data.control_id) {
			app.console.error(
				'JSDialogModelState: bad syntax in widgetAction: ' +
					this.safeStringify(data),
			);
			return;
		}

		const found = this.getById(data.data.control_id);
		if (found) {
			if (JSDialog.verbose) {
				app.console.debug(
					'JSDialogModelState: ' +
						this.componentName +
						' widgetAction ' +
						data.data.control_id,
				);
			}

			if (data.data.action_type === 'show') found.visible = true;
			else if (data.data.action_type === 'hide') found.visible = false;
			else if (data.data.action_type === 'rename' && data.data.new_id)
				found.id = data.data.new_id;
		}
	}

	/// returns current state of a widget with given id
	public getById(widgetId: string): WidgetJSON | null {
		if (!this.model) {
			app.console.debug(
				'JSDialogModelState: model missing in component: ' + this.componentName,
			);
			return null;
		}

		const found = JSDialogModelState.findWidgetById(widgetId, this.model);
		if (!found && JSDialog.verbose)
			app.console.debug(
				'JSDialogModelState: ' +
					this.componentName +
					' not found id: ' +
					widgetId,
			);
		return found;
	}

	private static findWidgetById(
		id: string,
		model: WidgetJSON,
	): WidgetJSON | null {
		if (model.id === id) {
			return model;
		} else if (model.children) {
			for (const i in model.children) {
				const found = JSDialogModelState.findWidgetById(id, model.children[i]);
				if (found) return found;
			}
		}

		return null;
	}
}
