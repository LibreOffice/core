// @ts-strict-ignore
/* -*- js-indent-level: 8 -*- */
/*
 * Copyright the Collabora Office contributors.
 *
 * SPDX-License-Identifier: MPL-2.0
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/.
 */

/*
 * Evented is a base class that Map/Layer classes inherit from to
 * handle custom events.
 */

interface EventBaseType {
	type: string;
	target: any;
	[name: string]: any;
}

type CEventListener = (e: EventBaseType) => void;

interface EventMapObject {
	[name: string]: CEventListener;
}

interface CallbackWithContext {
	fn: CEventListener;
	ctx: any;
}

interface CallbackWithoutContext {
	fn: CEventListener;
}

class Evented extends BaseClass {
	// Stores the handlers which have'current' Evented object as context.
	private _eventsAuto: Map<string, Array<CallbackWithoutContext>>;

	// Stores the handlers that have 'foreign' objects as contexts.
	private _eventsExt: Map<string, Map<string, CallbackWithContext>>;
	// Cache the handler count for each event-name.
	private _numEvents: Map<string, number>;

	// Parents of this Evented object.
	private _eventParents: Map<number, Evented>;

	// This points to the outer window.L.Evented object.
	// This is needed till we completely get rid of window.L.Evented.
	private _outerObject: any;

	constructor(outerObject?: any) {
		super();
		this._eventsAuto = new Map();
		this._eventsExt = new Map();
		this._numEvents = new Map();
		this._eventParents = new Map();
		this._outerObject = outerObject ? outerObject : this;
	}

	public on(
		types: Map<string, CEventListener> | string | EventMapObject,
		fn: CEventListener | any,
		context?: any,
	): Evented {
		if (typeof types === 'string') {
			// types can be a list of event names strings delimited by spaces.
			const parts: string[] = Util.splitWords(types);

			for (var i = 0, len = parts.length; i < len; i++) {
				this._addEventHandlerImpl(parts[i], fn, context);
			}
		}
		// types can be a map of types/handlers
		else if (types instanceof Map) {
			for (const [type, listener] of types) {
				this._addEventHandlerImpl(type, listener, fn);
			}
		}
		// types can also be a generic object with type-name as properties and
		// handlers as their values.
		else {
			const typeNames = Object.keys(types);
			for (let i = 0, len = typeNames.length; i < len; ++i) {
				const typeName = typeNames[i];
				const listener = types[typeName];
				this._addEventHandlerImpl(typeName, listener, fn);
			}
		}
		return this;
	}

	public off(
		types?: Map<string, CEventListener> | string | EventMapObject,
		fn?: CEventListener | any,
		context?: any,
	): Evented {
		if (!types) {
			// clear all handler maps if called without arguments.
			this._eventsExt.clear();
			this._eventsAuto.clear();
			this._numEvents.clear();
		} else if (typeof types === 'string') {
			const parts: string[] = Util.splitWords(types);

			for (var i = 0, len = parts.length; i < len; i++) {
				this._removeEventHandlerImpl(parts[i], fn, context);
			}
		} else if (types instanceof Map) {
			for (const [type, listener] of types) {
				this._removeEventHandlerImpl(type, listener, fn);
			}
		} else {
			const typeNames = Object.keys(types);
			for (let i = 0, len = typeNames.length; i < len; ++i) {
				const typeName = typeNames[i];
				const listener = types[typeName];
				this._removeEventHandlerImpl(typeName, listener, fn);
			}
		}

		return this;
	}

	private _addHandlerForeignCtxt(
		foreignCtxtId: number,
		type: string,
		fn: CEventListener,
		context: any,
	): void {
		// Store handlers with 'foreign' contexts and their count in separate maps.
		if (!this._eventsExt.has(type)) {
			this._eventsExt.set(type, new Map());
		}
		const typeIndex = this._eventsExt.get(type);
		const id: string =
			Util.stamp(fn as unknown as IDAble) + '_' + foreignCtxtId;

		if (!typeIndex.has(id)) {
			typeIndex.set(id, { fn: fn, ctx: context });

			// keep track of the number of keys in the index to quickly check if it's empty
			const count = this._numEvents.has(type) ? this._numEvents.get(type) : 0;

			this._numEvents.set(type, count + 1);
		}
	}

	private _addEventHandlerImpl(
		type: string,
		fn: CEventListener,
		context: any,
	): void {
		let foreignCtxtId = 0;
		// After killing window.L.Evented, the following check should change to
		// context !== this.
		if (context && context !== this._outerObject) {
			foreignCtxtId = Util.stamp(context);
		}

		if (foreignCtxtId) {
			this._addHandlerForeignCtxt(foreignCtxtId, type, fn, context);
		} else {
			// Context is the current Evented object itself.
			// Append just the handler to 'auto' map (context is implicitly known).

			// eslint-disable-next-line no-lonely-if
			if (!this._eventsAuto.has(type)) {
				this._eventsAuto.set(type, [{ fn: fn }]);
			} else {
				this._eventsAuto.get(type).push({ fn: fn });
			}
		}
	}

	private _removeHandlerForeignCtxt(
		foreignCtxtId: number,
		type: string,
		fn: CEventListener,
	): CallbackWithContext {
		let listener: CallbackWithContext = null;
		const id: string =
			Util.stamp(fn as unknown as IDAble) + '_' + foreignCtxtId;
		const listeners = this._eventsExt.get(type);

		if (listeners && listeners.get(id)) {
			listener = listeners.get(id);
			listeners.delete(id);
			this._numEvents.set(type, this._numEvents.get(type) - 1);
		}

		return listener;
	}

	private _removeHandlerSelfCtxt(
		type: string,
		fn: CEventListener,
	): CallbackWithoutContext {
		let listener: CallbackWithoutContext = null;
		const listeners = this._eventsAuto.get(type);

		if (listeners) {
			const len = listeners.length;
			for (let i = 0; i < len; i++) {
				if (listeners[i].fn === fn) {
					listener = listeners[i];
					listeners.splice(i, 1);
					break;
				}
			}
		}

		return listener;
	}

	private _removeEventHandlerImpl(
		type: string,
		fn: CEventListener,
		context: any,
	): void {
		if (!this._eventsExt.size && !this._eventsAuto.size) {
			return;
		}

		if (!fn) {
			// clear all handler maps for the type if handler isn't specified.
			this._eventsExt.delete(type);
			this._numEvents.delete(type);
			this._eventsAuto.delete(type);
			return;
		}

		let foreignCtxtId = 0;
		// After killing window.L.Evented, the following check should change to
		// context !== this.
		if (context && context !== this._outerObject) {
			foreignCtxtId = Util.stamp(context);
		}

		let listener: CallbackWithContext | CallbackWithoutContext;
		if (foreignCtxtId) {
			listener = this._removeHandlerForeignCtxt(foreignCtxtId, type, fn);
		} else {
			listener = this._removeHandlerSelfCtxt(type, fn);
		}

		// ensure the removed handler is not called if remove happens in fire
		if (listener) {
			listener.fn = Util.falseFn;
		}
	}

	public fire(type: string, data?: any, propagate?: boolean): Evented {
		if (!this.listens(type, propagate)) {
			return this;
		}

		const event = { ...data, type: type, target: this._outerObject };

		if (this._eventsExt.size || this._eventsAuto.size) {
			const typeIndex = this._eventsExt.get(type);
			const simpleListeners = this._eventsAuto.get(type);

			if (simpleListeners) {
				// Don't cause an infinite loop due to nested .on()/.off() calls.
				// Make a copy of the array to ensure this.
				const listeners = simpleListeners.slice();

				for (let i = 0, len = listeners.length; i < len; i++) {
					listeners[i].fn.call(this._outerObject, event);
				}
			}

			if (typeIndex) {
				// fire event using foreign context handlers as well.
				for (const [_, value] of typeIndex) {
					value.fn.call(value.ctx, event);
				}
			}
		}

		if (propagate) {
			// Call fire() on this instance's parents.
			this._propagateEvent(event);
		}

		return this;
	}

	public addEventParent(obj: Evented): Evented {
		this._eventParents.set(Util.stamp(obj), obj);
		return this;
	}

	public removeEventParent(obj: Evented): Evented {
		if (this._eventParents.size) {
			this._eventParents.delete(Util.stamp(obj));
		}
		return this;
	}

	private hasContextListeners(type: string): boolean {
		if (!this._eventsExt.size) {
			return false;
		}

		const mp = this._eventsExt.get(type);
		if (!mp || !mp.size) {
			return false;
		}

		return true;
	}

	private hasNoContextListener(type: string): boolean {
		if (!this._eventsAuto.size) {
			return false;
		}

		const arr = this._eventsAuto.get(type);
		if (!arr || !arr.length) {
			return false;
		}

		return true;
	}

	public listens(type: string, propagate?: boolean): boolean {
		if (this.hasContextListeners(type) || this.hasNoContextListener(type)) {
			return true;
		}

		if (propagate) {
			// also see if parents are listening in this case.
			for (const [_, parent] of this._eventParents) {
				if (parent.listens(type, propagate)) {
					return true;
				}
			}
		}
		return false;
	}

	public once(
		types: Map<string, CEventListener> | string | EventMapObject,
		fn: CEventListener | any,
		context?: any,
	): Evented {
		if (typeof types == 'string') {
			var handler = function () {
				this.off(types, fn, context).off(types, handler, context);
			}.bind(this);

			// add a handler which is executed once and removed soon after.
			return this.on(types, fn, context).on(types, handler, context);
		} else if (types instanceof Map) {
			for (const [type, listener] of types) {
				this.once(type, listener, fn);
			}
			return this;
		} else {
			const typeNames = Object.keys(types);
			for (let i = 0, len = typeNames.length; i < len; ++i) {
				const typeName = typeNames[i];
				const listener = types[typeName];
				this.once(typeName, listener, fn);
			}
			return this;
		}
	}

	private _propagateEvent(e: EventBaseType): void {
		for (const [_, parent] of this._eventParents) {
			parent.fire(e.type, { layer: e.target, ...e }, true);
		}
	}
}

app.Evented = Evented;
