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

describe('Events', function () {
	describe('Evented: Register handler with event names as a string', function () {
		const contextTypes = ['self', 'foreign'];
		const callTypes = ['eventlist-string', 'event+handler-object'];
		for (const contextType of contextTypes) {
			for (const callType of callTypes) {
				runTestsForContextType(contextType, callType);
			}
		}
	});

	function runTestsForContextType(contextType: string, callType: string) {
		const obj = new DerivedEvented();
		const foreignContext = new ForeignContext();
		let context: DerivedEvented | ForeignContext = null;
		function getContext() {
			return contextType === 'self' ? obj : foreignContext;
		}

		function getEventRegObject(regData: EventRegistrationData[]): any {
			const object: any = {};
			for (const item of regData) {
				const parts = Util.splitWords(item.eventList);
				for (const evt of parts) {
					object[evt] = item.handler;
				}
			}

			return object;
		}

		function registerEvents(
			eventObj: Evented,
			regData: EventRegistrationData[],
			thisContext: any,
		) {
			if (callType == 'eventlist-string') {
				for (const item of regData) {
					eventObj.on(item.eventList, item.handler, thisContext);
				}
			} else if (callType == 'event+handler-object') {
				const object: any = getEventRegObject(regData);
				eventObj.on(object, thisContext);
			} else {
				nodeassert.fail('Invalid callType "' + callType + '"');
			}
		}

		function unregisterEvents(
			eventObj: Evented,
			regData: EventRegistrationData[],
			thisContext: any,
		) {
			if (callType == 'eventlist-string') {
				for (const item of regData) {
					eventObj.off(item.eventList, item.handler, thisContext);
				}
			} else if (callType == 'event+handler-object') {
				const object: any = getEventRegObject(regData);
				eventObj.off(object, thisContext);
			} else {
				nodeassert.fail('Invalid callType "' + callType + '"');
			}
		}

		function registerForOnce(
			eventObj: Evented,
			regData: EventRegistrationData[],
			thisContext: any,
		) {
			if (callType == 'eventlist-string') {
				for (const item of regData) {
					eventObj.once(item.eventList, item.handler, thisContext);
				}
			} else if (callType == 'event+handler-object') {
				const object: any = getEventRegObject(regData);
				eventObj.once(object, thisContext);
			} else {
				nodeassert.fail('Invalid callType "' + callType + '"');
			}
		}

		describe(
			'with contextType = ' + contextType + ' callType = ' + callType,
			function () {
				beforeEach(function () {
					obj.reset();
					obj.off();
					foreignContext.reset();
					context = getContext();
				});

				it('no fire() => no handler calls', function () {
					const regData: EventRegistrationData[] = [
						{ eventList: 'e1 e2', handler: context.handler1 },
						{ eventList: 'e3 e4', handler: context.handler2 },
					];

					registerEvents(obj, regData, context);

					unregisterEvents(obj, regData, context);

					nodeassert.equal(0, context.first.numCalls);
					nodeassert.equal(null, context.first.event);

					nodeassert.equal(0, context.second.numCalls);
					nodeassert.equal(null, context.second.event);
				});

				it('fire() called after off() must have no effect', function () {
					const regData: EventRegistrationData[] = [
						{ eventList: 'e1 e2', handler: context.handler1 },
						{ eventList: 'e3 e4', handler: context.handler2 },
					];

					registerEvents(obj, regData, context);

					unregisterEvents(obj, regData, context);

					obj.fire('e1');
					obj.fire('e4');

					nodeassert.equal(0, context.first.numCalls);
					nodeassert.equal(null, context.first.event);

					nodeassert.equal(0, context.second.numCalls);
					nodeassert.equal(null, context.second.event);
				});

				it('fire() e1 and e4', function () {
					const regData: EventRegistrationData[] = [
						{ eventList: 'e1 e2', handler: context.handler1 },
						{ eventList: 'e3 e4', handler: context.handler2 },
					];

					registerEvents(obj, regData, context);

					obj.fire('e1');
					obj.fire('e4');

					unregisterEvents(obj, regData, context);

					nodeassert.equal(1, context.first.numCalls);
					nodeassert.equal('e1', context.first.event.type);
					nodeassert.equal(obj, context.first.event.target);

					nodeassert.equal(1, context.second.numCalls);
					nodeassert.equal('e4', context.second.event.type);
					nodeassert.equal(obj, context.second.event.target);
				});

				it('fire() all four events', function () {
					const regData: EventRegistrationData[] = [
						{ eventList: 'e1 e2', handler: context.handler1 },
						{ eventList: 'e3 e4', handler: context.handler2 },
					];

					registerEvents(obj, regData, context);

					obj.fire('e1');
					obj.fire('e2');
					obj.fire('e3');
					obj.fire('e4');

					unregisterEvents(obj, regData, context);

					nodeassert.equal(2, context.first.numCalls);
					nodeassert.equal('e2', context.first.event.type);
					nodeassert.equal(obj, context.first.event.target);

					nodeassert.equal(2, context.second.numCalls);
					nodeassert.equal('e4', context.second.event.type);
					nodeassert.equal(obj, context.second.event.target);
				});

				it('fire() e4 multiple times', function () {
					const regData: EventRegistrationData[] = [
						{ eventList: 'e1 e2', handler: context.handler1 },
						{ eventList: 'e3 e4', handler: context.handler2 },
					];

					registerEvents(obj, regData, context);

					obj.fire('e4');
					obj.fire('e4');
					obj.fire('e4');

					unregisterEvents(obj, regData, context);

					nodeassert.equal(0, context.first.numCalls);
					nodeassert.equal(null, context.first.event);

					nodeassert.equal(3, context.second.numCalls);
					nodeassert.equal('e4', context.second.event.type);
					nodeassert.equal(obj, context.second.event.target);
				});

				it('fire() e2 then e1, ensure order of handler calls', function () {
					const regData: EventRegistrationData[] = [
						{ eventList: 'e1 e2', handler: context.handler1 },
						{ eventList: 'e3 e4', handler: context.handler2 },
					];

					registerEvents(obj, regData, context);

					obj.fire('e2');
					obj.fire('e1');

					unregisterEvents(obj, regData, context);

					nodeassert.equal(2, context.first.numCalls);
					// last called event type is captured.
					nodeassert.equal('e1', context.first.event.type);
					nodeassert.equal(obj, context.first.event.target);

					nodeassert.equal(0, context.second.numCalls);
					nodeassert.equal(null, context.second.event);
				});

				it('fire() with data object', function () {
					const regData: EventRegistrationData[] = [
						{ eventList: 'e1', handler: context.handler1 },
					];

					registerEvents(obj, regData, context);

					obj.fire('e1', { key1: 42, key2: { inner: 'innerValue' } });
					unregisterEvents(obj, regData, context);

					nodeassert.equal(1, context.first.numCalls);
					nodeassert.equal('e1', context.first.event.type);
					nodeassert.equal(obj, context.first.event.target);
					const eventData = context.first.event as any;
					nodeassert.equal(42, eventData.key1);
					nodeassert.equal('innerValue', eventData.key2.inner);

					nodeassert.equal(0, context.second.numCalls);
					nodeassert.equal(null, context.second.event);
				});

				it('fire() with no propagate but has parent object', function () {
					const parentObj = new DerivedEvented();

					const regDataChild: EventRegistrationData[] = [
						{ eventList: 'e1', handler: context.handler1 },
					];
					const regDataParent: EventRegistrationData[] = [
						{ eventList: 'e1', handler: parentObj.handler1 },
					];

					registerEvents(obj, regDataChild, context);
					registerEvents(parentObj, regDataParent, parentObj);

					// Register parent object.
					obj.addEventParent(parentObj);

					obj.fire('e1', { key1: 42, key2: { inner: 'innerValue' } });

					unregisterEvents(obj, regDataChild, context);
					unregisterEvents(parentObj, regDataParent, parentObj);

					nodeassert.equal(1, context.first.numCalls);
					nodeassert.equal('e1', context.first.event.type);
					nodeassert.equal(obj, context.first.event.target);
					const eventData = context.first.event as any;
					nodeassert.equal(42, eventData.key1);
					nodeassert.equal('innerValue', eventData.key2.inner);

					nodeassert.equal(0, context.second.numCalls);
					nodeassert.equal(null, context.second.event);

					// No calls should have been made to parent object handlers.
					nodeassert.equal(0, parentObj.first.numCalls);
					nodeassert.equal(null, parentObj.first.event);

					nodeassert.equal(0, parentObj.second.numCalls);
					nodeassert.equal(null, parentObj.second.event);

					// Deregister parent object.
					obj.removeEventParent(parentObj);
				});

				it('fire() with propagate and has parent object', function () {
					const parentObj = new DerivedEvented();

					const regDataChild: EventRegistrationData[] = [
						{ eventList: 'e1', handler: context.handler1 },
					];
					const regDataParent: EventRegistrationData[] = [
						{ eventList: 'e1', handler: parentObj.handler1 },
					];

					registerEvents(obj, regDataChild, context);
					registerEvents(parentObj, regDataParent, parentObj);

					// Register parent object.
					obj.addEventParent(parentObj);

					obj.fire('e1', { key1: 42, key2: { inner: 'innerValue' } }, true);

					unregisterEvents(obj, regDataChild, context);
					unregisterEvents(parentObj, regDataParent, parentObj);

					nodeassert.equal(1, context.first.numCalls);
					nodeassert.equal('e1', context.first.event.type);
					const eventData = context.first.event as any;
					nodeassert.equal(42, eventData.key1);
					nodeassert.equal('innerValue', eventData.key2.inner);

					nodeassert.equal(0, context.second.numCalls);
					nodeassert.equal(null, context.second.event);

					// Parent object must also have received the call.
					nodeassert.equal(1, parentObj.first.numCalls);
					nodeassert.equal('e1', parentObj.first.event.type);
					const eventData2 = parentObj.first.event as any;
					nodeassert.equal(42, eventData2.key1);
					nodeassert.equal('innerValue', eventData2.key2.inner);
					nodeassert.deepEqual(obj, eventData2.layer);

					nodeassert.equal(0, parentObj.second.numCalls);
					nodeassert.equal(null, parentObj.second.event);

					// Deregister parent object.
					obj.removeEventParent(parentObj);
				});

				it('fire() with propagate but with removed parent object', function () {
					const parentObj = new DerivedEvented();

					const regDataChild: EventRegistrationData[] = [
						{ eventList: 'e1', handler: context.handler1 },
					];
					const regDataParent: EventRegistrationData[] = [
						{ eventList: 'e1', handler: parentObj.handler1 },
					];

					registerEvents(obj, regDataChild, context);
					registerEvents(parentObj, regDataParent, parentObj);

					// Register parent object.
					obj.addEventParent(parentObj);
					// Deregister the same parent.
					obj.removeEventParent(parentObj);

					obj.fire('e1', { key1: 42, key2: { inner: 'innerValue' } }, true);

					unregisterEvents(obj, regDataChild, context);
					unregisterEvents(parentObj, regDataParent, parentObj);

					nodeassert.equal(1, context.first.numCalls);
					nodeassert.equal('e1', context.first.event.type);
					nodeassert.equal(obj, context.first.event.target);
					const eventData = context.first.event as any;
					nodeassert.equal(42, eventData.key1);
					nodeassert.equal('innerValue', eventData.key2.inner);

					nodeassert.equal(0, context.second.numCalls);
					nodeassert.equal(null, context.second.event);

					// No calls should have been made to parent object handlers.
					nodeassert.equal(0, parentObj.first.numCalls);
					nodeassert.equal(null, parentObj.first.event);

					nodeassert.equal(0, parentObj.second.numCalls);
					nodeassert.equal(null, parentObj.second.event);
				});

				it('listens() when object is not listening', function () {
					const regData: EventRegistrationData[] = [
						{ eventList: 'e1', handler: context.handler1 },
					];

					registerEvents(obj, regData, context);

					nodeassert.equal(false, obj.listens('e2'));

					unregisterEvents(obj, regData, context);
				});

				it('listens() when object stopped listening', function () {
					const regData: EventRegistrationData[] = [
						{ eventList: 'e1', handler: context.handler1 },
					];

					registerEvents(obj, regData, context);
					unregisterEvents(obj, regData, context);

					nodeassert.equal(false, obj.listens('e1'));
				});

				it('listens() when object is listening', function () {
					const regData: EventRegistrationData[] = [
						{ eventList: 'e1', handler: context.handler1 },
					];

					registerEvents(obj, regData, context);

					nodeassert.equal(true, obj.listens('e1'));

					unregisterEvents(obj, regData, context);
				});

				it('once() without firing', function () {
					const regData: EventRegistrationData[] = [
						{ eventList: 'e1', handler: context.handler1 },
					];

					registerForOnce(obj, regData, context);

					unregisterEvents(obj, regData, context);

					nodeassert.equal(0, context.first.numCalls);
					nodeassert.equal(null, context.first.event);

					nodeassert.equal(0, context.second.numCalls);
					nodeassert.equal(null, context.second.event);
				});

				it('once() with single fire()', function () {
					const regData: EventRegistrationData[] = [
						{ eventList: 'e1', handler: context.handler1 },
					];

					registerForOnce(obj, regData, context);

					obj.fire('e1', { key1: 42, key2: { inner: 'innerValue' } });

					nodeassert.equal(1, context.first.numCalls);
					nodeassert.equal('e1', context.first.event.type);
					nodeassert.equal(obj, context.first.event.target);
					const eventData = context.first.event as any;
					nodeassert.equal(42, eventData.key1);
					nodeassert.equal('innerValue', eventData.key2.inner);
					// off() should have been called automatically
					// so listens() should return false.
					nodeassert.equal(false, obj.listens('e1'));

					nodeassert.equal(0, context.second.numCalls);
					nodeassert.equal(null, context.second.event);
				});

				it('once() with multiple fire()', function () {
					const regData: EventRegistrationData[] = [
						{ eventList: 'e1', handler: context.handler1 },
					];

					registerForOnce(obj, regData, context);

					obj.fire('e1', { key1: 42, key2: { inner: 'innerValue' } });
					obj.fire('e1', { key1: 4242, key2: { inner: 'innerValue' } });
					obj.fire('e1', { key1: 424242, key2: { inner: 'innerValue' } });

					// despite the multiple fire() calls, handler must have been
					// called just once.
					nodeassert.equal(1, context.first.numCalls);
					nodeassert.equal('e1', context.first.event.type);
					nodeassert.equal(obj, context.first.event.target);
					const eventData = context.first.event as any;
					nodeassert.equal(42, eventData.key1);
					nodeassert.equal('innerValue', eventData.key2.inner);
					// off() should have been called automatically
					// so listens() should return false.
					nodeassert.equal(false, obj.listens('e1'));

					nodeassert.equal(0, context.second.numCalls);
					nodeassert.equal(null, context.second.event);
				});
			},
		);
	}
});
