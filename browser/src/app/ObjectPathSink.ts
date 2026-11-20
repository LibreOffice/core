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
	The changes of the path of an object, sent to the engine. A move of points travels at once.
	Every other change names points by their index in the path the client has, so it is not sent
	while an undo or a redo is on its way, and point moves that still wait are sent first. Such a
	change can add or remove points and weights, so the keyboard leaves the points.
*/
class ObjectPathSink implements PathSink {
	public readonly wholeTwips: boolean = true;
	private objectId: number;
	private selection: SelectionSection;

	constructor(objectId: number, selection: SelectionSection) {
		this.objectId = objectId;
		this.selection = selection;
	}

	/*
		The object the points belong to and one item for every point or weight that moved. The
		points that did not move are not named at all, so a path of many thousand points still
		travels as the few numbers that changed, and the ones left alone come back exactly as they
		went out.
	*/
	public movePoints(moves: PathPointMove[]): void {
		if (!moves.length) return;

		app.socket.sendMessage(
			'setobjectpoints id=' +
				String(this.objectId) +
				' changes=' +
				ObjectPathSink.itemsOf(moves),
		);
	}

	/// Each point is "-polygon,point,0", where 0 names the point itself.
	public deletePoints(places: PathPlace[]): void {
		const items = places.map(
			(place: PathPlace) => '-' + [place.polygon, place.point, 0].join(','),
		);
		this.send('setobjectpoints', 'changes=' + items.join(';') + ' as=delete');
	}

	/// The object is the one selected, so it goes the way the selected objects are deleted.
	public deleteObject(): void {
		if (!this.readyToSend()) return;
		this.selection.travel.leave();
		GraphicSelection.deleteTheObjectsHere();
	}

	/*
		The new point goes first and the weights after it, which name their points by the places
		they have in the path with the new point. The points that do not change are not named, as
		for a move.
	*/
	public addPoint(addition: PathPointAddition): void {
		const point = [
			addition.polygon,
			addition.point,
			addition.x,
			addition.y,
		].join(',');
		const weights = addition.weights.length
			? ';' + ObjectPathSink.itemsOf(addition.weights)
			: '';
		this.send('setobjectpoints', 'changes=+' + point + weights + ' as=add');
	}

	/// The moves go first and the removals after them, so a weight that is removed stays
	/// removed even where a move makes the curve run smoothly through its point.
	public changeWeights(change: PathWeightsChange): void {
		const items = change.removals.map(
			(removal: PathWeightRemoval) =>
				'-' + [removal.polygon, removal.point, removal.part].join(','),
		);
		if (change.moves.length)
			items.unshift(ObjectPathSink.itemsOf(change.moves));
		this.send(
			'setobjectpoints',
			'changes=' + items.join(';') + ' as=' + change.step,
		);
	}

	/*
		The second object takes an id reserved for this view, where one is at hand. It then shows
		at once as its own object, drawn as its path in hairlines, and it is selected, so it can
		be moved away. The engine's report of it replaces it. Without a reserved id the second
		object shows when the engine reports it.
	*/
	public split(place: PathPlace, after: cool.ObjectPathPolygon[]): void {
		const newId = ReservedObjectIds.take();
		const sent = this.send(
			'splitobjectpath',
			ObjectPathSink.parametersOf(place) +
				(newId === undefined ? '' : ' newid=' + String(newId)),
		);
		if (sent && newId !== undefined) this.standInAsSplitOff(newId, after);
	}

	/// The second object of a split stands as created, under its id and selected, in the place
	/// of the first object among its group and layer.
	private standInAsSplitOff(
		newId: number,
		path: cool.ObjectPathPolygon[],
	): void {
		const part = RenderGeometrySection.currentPart();
		const first = RenderGeometrySection.objectOf(this.objectId);
		if (!part) return;

		const box = PathGeometry.rangeOf(path);
		const created: cool.SlideObject = {
			id: newId,
			parent: first?.parent,
			layer: first?.layer,
			objectKind: first?.objectKind,
			x: box.minX,
			y: box.minY,
			width: box.width,
			height: box.height,
			path: path,
			primitives: [
				{
					type: cool.PolygonHairlinePrimitive.type,
					color: HandleLook.outlineColour,
					path: PathGeometry.asSvg(path),
				} as cool.PolygonHairlinePrimitive,
			],
		};
		if (
			RenderManager.addProvisionalObject(
				part.partId,
				created,
				PathEditing.longestWait,
			)
		)
			GraphicSelection.selectAndShow([newId]);
	}

	/// Closing or opening names a state, so the outbox takes it. Opening at a point moves the
	/// points around, so the changes before it go first.
	public setClosed(closed: boolean, at?: PathPlace): void {
		this.send(
			'setobjectpathclosed',
			'closed=' +
				String(closed) +
				(at ? ' at=' + String(at.polygon) + ',' + String(at.point) : ''),
			at !== undefined,
		);
	}

	/// The weights go first, named by their places in the path as it is, and the points after
	/// them.
	public simplify(simplification: PathSimplification): void {
		const items = simplification.weights
			.map(
				(weight: PathWeightRemoval) =>
					'-' + [weight.polygon, weight.point, weight.part].join(','),
			)
			.concat(
				simplification.points.map(
					(place: PathPlace) => '-' + [place.polygon, place.point, 0].join(','),
				),
			);
		this.send('setobjectpoints', 'changes=' + items.join(';') + ' as=simplify');
	}

	public pathChangedHere(_path: cool.ObjectPathPolygon[]): void {
		// Every change goes to the engine by itself, as the change it is.
	}

	/// The handles stand on the path as shown, so they are worked out again.
	public shownPathChanged(): void {
		this.selection.refresh();
		app.sectionContainer?.requestReDraw();
	}

	/// The moves as the items of a setobjectpoints message.
	private static itemsOf(moves: PathPointMove[]): string {
		return moves
			.map((move: PathPointMove) =>
				[
					move.polygon,
					move.point,
					move.part,
					move.x,
					move.y,
					move.continuity,
				].join(','),
			)
			.join(';');
	}

	/// The parameters that name a point of the path to the engine.
	private static parametersOf(place: PathPlace): string {
		return 'polygon=' + String(place.polygon) + ' point=' + String(place.point);
	}

	/*
		Answers whether a change can go to the engine now. It cannot while an undo is on its way,
		and the hold back is shown then. Otherwise the keyboard leaves the handles, and with
		sendAhead the changes waiting in the outbox go first. A change that names a state goes
		into the outbox behind them, which keeps the order and lets a later state of the same
		thing take its place. The change can add or remove points and weights, so the keyboard
		keeps its place rather than its handle: it stays on the handle at that place, or goes to
		the nearest point where the handle went.
	*/
	private readyToSend(sendAhead: boolean = true): boolean {
		if (ChangeOutbox.undoOnItsWay()) {
			this.selection.showTheHoldBack();
			return false;
		}

		this.selection.travel.keepThePlace(PathEditing.namesAPoint);
		if (sendAhead) ChangeOutbox.sendEverythingAhead();
		return true;
	}

	/// Answers whether the message went out.
	private send(
		command: string,
		parameters: string,
		sendAhead: boolean = true,
	): boolean {
		if (!this.readyToSend(sendAhead)) return false;

		app.socket.sendMessage(
			command + ' id=' + String(this.objectId) + ' ' + parameters,
		);
		return true;
	}
}
