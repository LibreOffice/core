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

namespace cool {
	/// One planar face of a 3D primitive, in the space of the primitive: its polygons, the outer
	/// one first, then the holes. Answers whether the walk goes on.
	export type Face3DSink = (face: PolyPolygon3D) => boolean;

	/// A 3D object of a scene with the mapping of its primitive's space into the view, where x
	/// and y run over the unit square the scene is seen on and z is the depth.
	interface Scene3DMember {
		id: number;
		primitive: Primitive3D;
		toView: Matrix3D;
		toScene: Matrix3D;
	}

	/// The outline of an extrusion or a body of rotation placed in 3D: a lid that closes the
	/// front or the back, or one of the regular slices between them.
	interface Slice3D {
		kind: 'front' | 'regular' | 'back';
		outline: PolyPolygon3D;
	}

	/*
		A 3D scene as the client works it out from its 3D objects: the faces of each kind, the hit
		of a point on the page as the engine finds it, and the outline that follows the shape. The
		faces are made on the fly and handed out one by one, nothing of them is kept.
	*/
	export class Scene3D {
		static readonly sceneKind = 202;

		// the kinds whose faces we make here, the others leave the scene to its picture
		private static readonly kindsMade = [
			'cube',
			'sphere',
			'polyPolygon',
			'extrude',
			'lathe',
		];

		// neighbouring faces that turn by more than this, in degrees, meet in an edge one sees
		private static readonly creaseAngle = 30;

		private static outlineKept: {
			key: string;
			outline: Path2D | undefined;
		} | null = null;

		/// Whether we work that scene out from its 3D objects: it is seen through a camera of its
		/// own and holds 3D objects of the kinds made here only.
		static takes(data: VectorPrimitivesData, sceneId: number): boolean {
			const scene = data.objects.get(sceneId);
			if (scene?.objectKind !== Scene3D.sceneKind || !scene.scene3D)
				return false;

			const members = Scene3D.membersOf(data, sceneId);
			return (
				members !== null &&
				members.length > 0 &&
				members.every((member: Scene3DMember) =>
					Scene3D.kindsMade.includes(member.primitive.type),
				)
			);
		}

		/// The 3D object of the scene a point on the page, in twips, hits first, the one in front,
		/// with its depth. Nothing where the point meets none of them.
		static hitAt(
			data: VectorPrimitivesData,
			sceneId: number,
			x: number,
			y: number,
		): { id: number; depth: number } | undefined {
			return Scene3D.hitsAt(data, sceneId, x, y)[0];
		}

		/*
			Every 3D object of the scene a point on the page, in twips, hits, each once with the
			depth of its nearest face there, from the front to the back. The depth runs from 0 at
			the front of the view to 1 at its back. Empty where the point meets none of them.
		*/
		static hitsAt(
			data: VectorPrimitivesData,
			sceneId: number,
			x: number,
			y: number,
		): { id: number; depth: number }[] {
			const toPage = Matrix2D.fromArray(
				data.objects.get(sceneId)?.scene3D?.placement,
			);
			const fromPage = toPage?.invert();
			if (!toPage || !fromPage) return [];

			// as the engine does: outside the unit square of the scene nothing is hit
			const onTheScene = fromPage.apply(x, y);
			if (
				onTheScene.x < 0 ||
				onTheScene.x > 1 ||
				onTheScene.y < 0 ||
				onTheScene.y > 1
			)
				return [];

			const hits: { id: number; depth: number }[] = [];
			for (const member of Scene3D.membersOf(data, sceneId) ?? []) {
				const depth = Scene3D.depthOfTheHit(member, toPage, onTheScene, x, y);
				if (depth !== undefined) hits.push({ id: member.id, depth: depth });
			}
			return hits.sort((one, other) => one.depth - other.depth);
		}

		/*
			What shows the shape of a 3D scene, or of one 3D object or scene inside one, on the
			page in twips: for each 3D object the lines that read best for its kind, none hidden
			by what lies in front of it, joined into lines where they run on.
		*/
		static outlineOf(
			data: VectorPrimitivesData,
			objectId: number,
		): Path2D | undefined {
			const key = data.partId + ':' + objectId + ':' + data.version;
			if (Scene3D.outlineKept?.key === key) return Scene3D.outlineKept.outline;

			const sceneId = Scene3D.sceneOf(data, objectId);
			const placement = Matrix2D.fromArray(
				sceneId === undefined ? undefined : (
					data.objects.get(sceneId)?.scene3D?.placement
				),
			);
			let outline: Path2D | undefined;
			if (sceneId !== undefined && placement) {
				outline = new Path2D();
				for (const member of Scene3D.membersOf(data, sceneId) ?? [])
					if (Scene3D.liesIn(data, member.id, objectId))
						for (const line of Scene3D.linesOfTheShape(member, placement))
							line.addTo(outline);
			}

			Scene3D.outlineKept = { key: key, outline: outline };
			return outline;
		}

		/// The 3D scene, sitting in no other scene, that object is or lies in. Nothing for an
		/// object that is no part of a 3D scene.
		static sceneOf(
			data: VectorPrimitivesData,
			objectId: number,
		): number | undefined {
			for (
				let at: number | undefined = objectId, guard = 0;
				at !== undefined && guard < 100;
				at = data.objects.get(at)?.parent, ++guard
			)
				if (data.objects.get(at)?.scene3D) return at;
			return undefined;
		}

		/// Whether the object is that one or lies inside it, at any depth.
		private static liesIn(
			data: VectorPrimitivesData,
			objectId: number,
			holderId: number,
		): boolean {
			for (
				let at: number | undefined = objectId, guard = 0;
				at !== undefined && guard < 100;
				at = data.objects.get(at)?.parent, ++guard
			)
				if (at === holderId) return true;
			return false;
		}

		/// Hands out the faces of a primitive of a kind made here, in the space of the primitive,
		/// as the engine makes them. Nothing for any other kind.
		static forEachFace(primitive: Primitive3D, sink: Face3DSink): void {
			if (primitive.type === 'cube') Scene3D.cubeFaces(sink);
			else if (primitive.type === 'sphere')
				Scene3D.sphereFaces(primitive, sink);
			else if (primitive.type === 'polyPolygon')
				Scene3D.polyPolygonFace(primitive, sink);
			else if (primitive.type === 'extrude')
				Scene3D.facesOfTheSlices(Scene3D.extrudeSlices(primitive), false, sink);
			else if (primitive.type === 'lathe')
				Scene3D.facesOfTheSlices(
					Scene3D.latheSlices(primitive),
					Scene3D.turnsAllRound(primitive),
					sink,
				);
		}

		/// The 3D objects inside the scene, those of the scenes inside it included, each with the
		/// way of its primitive into the view. Null where the scene has no camera.
		private static membersOf(
			data: VectorPrimitivesData,
			sceneId: number,
		): Scene3DMember[] | null {
			const view = data.objects.get(sceneId)?.scene3D;
			if (!view) return null;

			// the engine's view: the scene's own transformation, the camera, the projection and
			// the step to the unit square, in that order
			const sceneSteps = [view.orientation, view.projection, view.deviceToView];
			let sceneToView = Matrix3D.fromArray(view.transformation);
			for (const step of sceneSteps) {
				const next = Matrix3D.fromArray(step);
				if (!sceneToView || !next) return null;
				sceneToView = sceneToView.then(next);
			}
			if (!sceneToView) return null;

			const members: Scene3DMember[] = [];
			for (const id of data.order) {
				const object = data.objects.get(id);
				const primitive = object?.primitives3D?.[0];
				if (!object || !primitive) continue;

				// from the object up through the scenes in between to the one seen
				let toScene =
					Matrix3D.fromArray(primitive.transform) ?? Matrix3D.IDENTITY;
				let at: SlideObject | undefined = object;
				let guard = 0;
				while (at && at.id !== sceneId && guard++ < 100) {
					toScene = toScene.then(
						Matrix3D.fromArray(at.transform3D) ?? Matrix3D.IDENTITY,
					);
					at = at.parent ? data.objects.get(at.parent) : undefined;
				}
				if (!at) continue;

				members.push({
					id: id,
					primitive: primitive,
					toView: toScene.then(sceneToView),
					toScene: toScene.then(
						Matrix3D.fromArray(view.transformation) ?? Matrix3D.IDENTITY,
					),
				});
			}
			return members;
		}

		/*
			The depth of the nearest face of the object at a point on the page, in twips, nothing
			where the point meets none. Each face is laid onto the page as one sees it and asked
			as a fill is; a plane stays a plane in the view, so the depth at the point is the one
			of the face's plane there.
		*/
		private static depthOfTheHit(
			member: Scene3DMember,
			toPage: Matrix2D,
			onTheScene: Point,
			x: number,
			y: number,
		): number | undefined {
			let nearest: number | undefined;
			Scene3D.forEachFace(member.primitive, (face: PolyPolygon3D) => {
				const seen = face.transformed(member.toView);
				const onThePage = seen.flattened().transformed(toPage);

				const box = onThePage.polygons[0].range();
				if (x < box.minX || x > box.maxX || y < box.minY || y > box.maxY)
					return true;
				if (!onThePage.isInside(x, y)) return true;

				// a face seen on its edge has no depth of its own at a point
				const normal = seen.polygons[0].normal();
				if (Math.abs(normal.z) < 1e-12) return true;
				const corner = seen.polygons[0].points[0];
				const depth =
					corner.z -
					(normal.x * (onTheScene.x - corner.x) +
						normal.y * (onTheScene.y - corner.y)) /
						normal.z;
				if (nearest === undefined || depth < nearest) nearest = depth;
				return true;
			});
			return nearest;
		}

		/*
			The lines that show the shape of one 3D object, on the page in twips, in the way that
			reads best for its kind: a body of rotation by its profile at each eighth of the turn
			and the ends of it, a sphere by a few of its rings and meridians, and any other kind
			by its silhouette and the edges where its faces meet at an angle.
		*/
		private static linesOfTheShape(
			member: Scene3DMember,
			placement: Matrix2D,
		): Polygon2D[] {
			const onThePage = (line: Polygon3D): Polygon2D =>
				line.transformed(member.toView).flattened().transformed(placement);
			if (member.primitive.type === 'lathe')
				return Scene3D.linesOfALathe(member.primitive).map(onThePage);
			if (member.primitive.type === 'sphere')
				return Scene3D.linesOfASphere(member.primitive).map(onThePage);
			return Scene3D.edgesOf(member).map(onThePage);
		}

		/*
			The lines of a body of rotation: a profile for each eighth of a full turn, as the
			engine draws the slices; where the profile is open, the rings its two ends turn,
			unless an end stays on the axis; and where it does not go all round, the outlines of
			its lids.
		*/
		private static linesOfALathe(primitive: Primitive3D): Polygon3D[] {
			const slices = Scene3D.latheSlices(primitive);
			const regular = slices.filter(
				(slice: Slice3D) => slice.kind === 'regular',
			);
			const lines: Polygon3D[] = [];
			for (const slice of slices)
				if (slice.kind !== 'regular') lines.push(...slice.outline.polygons);

			// a profile for each eighth of a full turn, spread evenly over the turn; one that
			// does not go all round keeps its first and its last
			const allRound = Scene3D.turnsAllRound(primitive);
			const eighths = Math.max(
				1,
				Math.round(Math.abs(primitive.rotation ?? 0) / (Math.PI / 4)),
			);
			if (allRound) {
				const profiles = Math.min(regular.length, eighths);
				for (let profile = 0; profile < profiles; ++profile)
					lines.push(
						...regular[
							Math.round((profile * regular.length) / profiles) % regular.length
						].outline.polygons,
					);
			} else if (regular.length) {
				const steps = Math.min(regular.length - 1, eighths);
				for (let profile = 0; profile <= steps; ++profile)
					lines.push(
						...regular[
							steps ? Math.round((profile * (regular.length - 1)) / steps) : 0
						].outline.polygons,
					);
			}

			const profiles = regular.length ? regular[0].outline.polygons : [];
			profiles.forEach((profile: Polygon3D, polygon: number) => {
				if (profile.closed) return;
				for (const end of [0, profile.points.length - 1]) {
					const ring = new Polygon3D(
						regular.map(
							(slice: Slice3D) => slice.outline.polygons[polygon].points[end],
						),
						allRound,
					);
					if (ring.longestEdge() > 1e-9) lines.push(ring);
				}
			});
			return lines;
		}

		/// The lines of a sphere: at most eight of its meridians and the rings between at most six
		/// bands, through the points of its own faces, so they lie on what the engine draws.
		private static linesOfASphere(primitive: Primitive3D): Polygon3D[] {
			const around = Scene3D.segmentsOf(
				primitive.horizontalSegments,
				2 * Math.PI,
			);
			const down = Scene3D.segmentsOf(primitive.verticalSegments, Math.PI);
			const meridians = Math.min(around, 8);
			const bands = Math.min(down, 6);

			const lines: Polygon3D[] = [];
			for (let meridian = 0; meridian < meridians; ++meridian) {
				const piece = Math.round((meridian * around) / meridians);
				const points: Point3D[] = [];
				for (let ring = 0; ring <= down; ++ring)
					points.push(Scene3D.onTheSphere(piece, around, ring, down));
				lines.push(new Polygon3D(points, false));
			}
			for (let band = 1; band < bands; ++band) {
				const ring = Math.round((band * down) / bands);
				const points: Point3D[] = [];
				for (let piece = 0; piece < around; ++piece)
					points.push(Scene3D.onTheSphere(piece, around, ring, down));
				lines.push(new Polygon3D(points, true));
			}
			return lines;
		}

		/*
			The silhouette of one 3D object and the edges where its faces meet at an angle, in
			the space of its primitive: the edges where a face turned towards us meets one turned
			away, and those where neighbouring faces turn by more than the crease angle. An edge
			that only one face has, as on a single polygon, is taken too. Each edge is taken once,
			none is hidden by what lies in front of it, and they come joined into chains.
		*/
		private static edgesOf(member: Scene3DMember): Polygon3D[] {
			// the same corner of two faces is one point, so each edge is found once
			const pointIds = new Map<string, number>();
			const points: Point3D[] = [];
			const idOf = (point: Point3D): number => {
				const key =
					point.x.toFixed(9) +
					',' +
					point.y.toFixed(9) +
					',' +
					point.z.toFixed(9);
				let id = pointIds.get(key);
				if (id === undefined) {
					id = points.length;
					points.push(point);
					pointIds.set(key, id);
				}
				return id;
			};

			const faces: { area: number; depth: number; normal: Point3D }[] = [];
			const edges = new Map<
				number,
				{ from: number; to: number; faces: number[] }
			>();
			const pointLimit = 1 << 20;
			Scene3D.forEachFace(member.primitive, (face: PolyPolygon3D) => {
				// a face whose corners all lie on one line has no area and no edges to show
				const inTheScene = face.polygons[0].transformed(member.toScene);
				if (inTheScene.isFlat()) return true;

				// the turn of the face as we see it; which way is towards us is found below
				const seen = face.polygons[0].transformed(member.toView);
				faces.push({
					area: seen.normal().z,
					depth:
						seen.points.reduce((sum: number, one: Point3D) => sum + one.z, 0) /
						seen.points.length,
					normal: inTheScene.normal(),
				});

				const index = faces.length - 1;
				for (const polygon of face.polygons) {
					const ids = polygon.points.map(idOf);
					for (let at = 0; at < ids.length; ++at) {
						const from = ids[at];
						const to = ids[(at + 1) % ids.length];
						if (from === to) continue;
						const key = Math.min(from, to) * pointLimit + Math.max(from, to);
						const edge = edges.get(key);
						if (edge) edge.faces.push(index);
						else edges.set(key, { from: from, to: to, faces: [index] });
					}
				}
				return true;
			});
			if (!faces.length) return [];

			// the nearest face is surely turned towards us, the faces turned like it are too
			const nearest = faces.reduce((one, other) =>
				other.depth < one.depth ? other : one,
			);
			const towardsUs = (face: number): boolean =>
				faces[face].area * nearest.area > 0;
			const creaseCosine = Math.cos((Scene3D.creaseAngle * Math.PI) / 180);

			const drawn: [number, number][] = [];
			for (const edge of edges.values()) {
				let shown = edge.faces.length === 1;
				if (edge.faces.length >= 2) {
					const [one, other] = edge.faces;
					const a = faces[one].normal;
					const b = faces[other].normal;
					const lengths = a.length() * b.length();
					shown =
						towardsUs(one) !== towardsUs(other) ||
						(lengths > 0 && a.dot(b) / lengths < creaseCosine);
				}
				if (shown) drawn.push([edge.from, edge.to]);
			}

			return Scene3D.chainsOf(drawn, (point: number) => points[point]).map(
				(chain: Point3D[]) => {
					const closed =
						chain.length > 3 && chain[0] === chain[chain.length - 1];
					return new Polygon3D(closed ? chain.slice(0, -1) : chain, closed);
				},
			);
		}

		/*
			The edges joined into chains of edges that meet end to end, so a dashed line runs on
			evenly along a curve made of many short edges. At a point where several edges meet,
			a chain goes on along the one that goes on the straightest. A chain that comes back to
			where it started ends on its first point again. Each edge is given by its two points.
		*/
		private static chainsOf(
			edges: [number, number][],
			pointOf: (point: number) => Point3D,
		): Point3D[][] {
			const edgesAt = new Map<number, number[]>();
			edges.forEach(([from, to], index: number) => {
				for (const point of [from, to]) {
					const there = edgesAt.get(point);
					if (there) there.push(index);
					else edgesAt.set(point, [index]);
				}
			});
			const used = edges.map(() => false);
			const otherEnd = (index: number, point: number): number =>
				edges[index][0] === point ? edges[index][1] : edges[index][0];

			// of the edges not taken yet at a point, the one that goes on the straightest from the
			// way the chain came
			const nextAt = (point: number, from: number): number | undefined => {
				const here = pointOf(point);
				const came = here.minus(pointOf(from));
				let best: number | undefined;
				let straightest = -Infinity;
				for (const index of edgesAt.get(point) ?? []) {
					if (used[index]) continue;
					const goes = pointOf(otherEnd(index, point)).minus(here);
					const lengths = came.length() * goes.length() || 1;
					const straight = came.dot(goes) / lengths;
					if (straight > straightest) {
						straightest = straight;
						best = index;
					}
				}
				return best;
			};

			const chains: Point3D[][] = [];
			edges.forEach(([from, to], first: number) => {
				if (used[first]) return;
				used[first] = true;

				// on from the end, then back from the start, as long as edges meet there
				const ahead = [from, to];
				for (
					let index = nextAt(to, from);
					index !== undefined;
					index = nextAt(ahead[ahead.length - 1], ahead[ahead.length - 2])
				) {
					used[index] = true;
					ahead.push(otherEnd(index, ahead[ahead.length - 1]));
				}
				const behind = [to, from];
				for (
					let index = nextAt(from, to);
					index !== undefined;
					index = nextAt(behind[behind.length - 1], behind[behind.length - 2])
				) {
					used[index] = true;
					behind.push(otherEnd(index, behind[behind.length - 1]));
				}
				chains.push(behind.slice(2).reverse().concat(ahead).map(pointOf));
			});
			return chains;
		}

		/// The six faces of the unit cube, from 0 to 1, each turned as the engine makes it.
		private static cubeFaces(sink: Face3DSink): void {
			const corner = (x: number, y: number, z: number) => new Point3D(x, y, z);
			const a = corner(0, 0, 0);
			const b = corner(0, 1, 0);
			const c = corner(1, 1, 0);
			const d = corner(1, 0, 0);
			const e = corner(0, 0, 1);
			const f = corner(0, 1, 1);
			const g = corner(1, 1, 1);
			const h = corner(1, 0, 1);
			const faces = [
				[d, a, e, h],
				[b, a, d, c],
				[e, a, b, f],
				[c, g, f, b],
				[h, g, c, d],
				[f, g, h, e],
			];
			for (const face of faces)
				if (!sink(new PolyPolygon3D([new Polygon3D(face)]))) return;
		}

		/// The faces of the sphere, ring by ring from the top, each a four cornered piece, the
		/// unit sphere brought from -1 to 1 into 0 to 1, as the engine makes them.
		private static sphereFaces(primitive: Primitive3D, sink: Face3DSink): void {
			const around = Scene3D.segmentsOf(
				primitive.horizontalSegments,
				2 * Math.PI,
			);
			const down = Scene3D.segmentsOf(primitive.verticalSegments, Math.PI);
			for (let ring = 0; ring < down; ++ring)
				for (let piece = 0; piece < around; ++piece) {
					const face = new Polygon3D([
						Scene3D.onTheSphere(piece, around, ring, down),
						Scene3D.onTheSphere(piece + 1, around, ring, down),
						Scene3D.onTheSphere(piece + 1, around, ring + 1, down),
						Scene3D.onTheSphere(piece, around, ring + 1, down),
					]);
					if (!sink(new PolyPolygon3D([face]))) return;
				}
		}

		/// How many segments a sphere has over that span: as given, or one for each fifteen
		/// degrees where none is given, between 1 and 512, as the engine takes them.
		private static segmentsOf(given: number | undefined, span: number): number {
			return Math.min(
				512,
				Math.max(1, given ? given : Math.round(span / (Math.PI / 12))),
			);
		}

		/// The point of the unit sphere, brought from -1 to 1 into 0 to 1, at that piece of the
		/// way round and that ring of the way down from the top.
		private static onTheSphere(
			piece: number,
			around: number,
			ring: number,
			down: number,
		): Point3D {
			const horizontal = (2 * Math.PI * piece) / around;
			const vertical = Math.PI / 2 - (Math.PI * ring) / down;
			const cosVertical = Math.cos(vertical);
			return new Point3D(
				(cosVertical * Math.cos(horizontal) + 1) / 2,
				(Math.sin(vertical) + 1) / 2,
				(-cosVertical * Math.sin(horizontal) + 1) / 2,
			);
		}

		/// The one face of a polygon object: all its polygons together, as the engine fills them.
		private static polyPolygonFace(
			primitive: Primitive3D,
			sink: Face3DSink,
		): void {
			const face = new PolyPolygon3D(
				(primitive.polygons ?? []).map((polygon: Primitive3DPolygon) =>
					Polygon3D.fromFlat(polygon.points, polygon.closed),
				),
			);
			if (face.polygons.length && face.polygons[0].points.length >= 3)
				sink(face);
		}

		/*
			The faces of an extrusion or a body of rotation, from its slices as the engine makes
			them: between each two regular slices a four cornered face for every edge of the
			outline, a lid with its holes where a slice closes an end, the front one turned
			round, and the faces of the bevel between a lid and the slice next to it. A body
			turned all round goes on from its last slice to its first.
		*/
		private static facesOfTheSlices(
			slices: Slice3D[],
			allRound: boolean,
			sink: Face3DSink,
		): void {
			const count = slices.length;
			if (!count) return;

			const pairs = allRound ? count : count - 1;
			if (!pairs) {
				sink(slices[0].outline.reversed());
				return;
			}

			for (let at = 0; at < pairs; ++at) {
				const one = slices[at];
				const other = slices[(at + 1) % count];
				if (one.kind !== 'regular' || other.kind !== 'regular') continue;

				const before = slices[(at + count - 1) % count];
				if (before.kind === 'front') {
					if (
						!one.outline.equals(before.outline, 1e-9) &&
						!Scene3D.sides(one, before, sink)
					)
						return;
					if (!sink(before.outline.reversed())) return;
				}

				const after = slices[(at + 2) % count];
				if (after.kind === 'back') {
					if (
						!other.outline.equals(after.outline, 1e-9) &&
						!Scene3D.sides(after, other, sink)
					)
						return;
					if (!sink(after.outline)) return;
				}

				if (!Scene3D.sides(other, one, sink)) return;
			}
		}

		/// The four cornered faces between two slices, one for each edge of their polygons.
		/// Answers whether the walk goes on.
		private static sides(
			one: Slice3D,
			other: Slice3D,
			sink: Face3DSink,
		): boolean {
			const polygons = Math.min(
				one.outline.polygons.length,
				other.outline.polygons.length,
			);
			for (let polygon = 0; polygon < polygons; ++polygon) {
				const a = one.outline.polygons[polygon];
				const b = other.outline.polygons[polygon];
				const points = Math.min(a.points.length, b.points.length);
				if (!points) continue;
				const edges = a.closed ? points : points - 1;
				for (let edge = 0; edge < edges; ++edge) {
					const next = (edge + 1) % points;
					const face = new Polygon3D([
						a.points[edge],
						b.points[edge],
						b.points[next],
						a.points[next],
					]);
					if (!sink(new PolyPolygon3D([face]))) return false;
				}
			}
			return true;
		}

		/// The slices of an extrusion: the outline at the front and at the back of its depth,
		/// and a lid at each end that is closed.
		private static extrudeSlices(primitive: Primitive3D): Slice3D[] {
			const source = Scene3D.outlineOf2D(primitive);
			const depth = primitive.depth ?? 0;
			if (Math.abs(depth) < 1e-12)
				return [Scene3D.slice(source, Matrix3D.IDENTITY, 'regular')];

			const diagonal = primitive.diagonal ?? 0;
			const characters = primitive.characterMode === true;
			let front = source;
			let back = source;
			let frontDepth = depth;
			let backDepth = 0;
			const backScale = Scene3D.backScaleOf(primitive);
			if (backScale !== 1) front = front.scaledOnTheCenter(backScale);

			const slices: Slice3D[] = [];
			if (primitive.closeFront) {
				const offset = depth * diagonal * 0.5;
				frontDepth = depth - offset;
				const lid = Scene3D.withTheBevel(front, offset, characters);
				front = lid.body;
				slices.push(
					Scene3D.slice(lid.lid, Matrix3D.translation(0, 0, depth), 'front'),
				);
			}

			let backLid: PolyPolygon2D | undefined;
			if (primitive.closeBack) {
				const offset = depth * diagonal * 0.5;
				backDepth = offset;
				const lid = Scene3D.withTheBevel(back, offset, characters);
				back = lid.body;
				backLid = lid.lid;
			}

			slices.push(
				Scene3D.slice(front, Matrix3D.translation(0, 0, frontDepth), 'regular'),
			);
			slices.push(
				Scene3D.slice(back, Matrix3D.translation(0, 0, backDepth), 'regular'),
			);
			if (backLid)
				slices.push(Scene3D.slice(backLid, Matrix3D.IDENTITY, 'back'));
			return slices;
		}

		/// The slices of a body of rotation: the outline turned about the y axis in its steps,
		/// and a lid at each end that is closed, where it does not go all round.
		private static latheSlices(primitive: Primitive3D): Slice3D[] {
			const source = Scene3D.outlineOf2D(primitive);
			const rotation = primitive.rotation ?? 0;
			const steps = primitive.horizontalSegments ?? 0;
			if (Math.abs(rotation) < 1e-12 || !steps)
				return [Scene3D.slice(source, Matrix3D.IDENTITY, 'regular')];

			const backScale = Scene3D.backScaleOf(primitive);
			const allRound = Scene3D.turnsAllRound(primitive);
			const closeFront = primitive.closeFront === true && !allRound;
			const closeBack = primitive.closeBack === true && !allRound;
			const diagonal = primitive.diagonal ?? 0;
			const characters = primitive.characterMode === true;
			let front = source;
			let back = source;
			if (backScale !== 1) back = back.scaledOnTheCenter(backScale);

			const slices: Slice3D[] = [];
			let backLid: Slice3D | undefined;
			if (closeFront || closeBack) {
				// the bevel is a twelfth of the way the middle of the outline goes round
				const box = front.range();
				const offset = (((box.maxX + box.minX) * rotation) / 2 / 12) * diagonal;
				if (closeFront) {
					const lid = Scene3D.withTheBevel(front, offset, characters);
					front = lid.body;
					slices.push(
						Scene3D.slice(lid.lid, Matrix3D.translation(0, 0, offset), 'front'),
					);
				}
				if (closeBack) {
					const lid = Scene3D.withTheBevel(back, offset, characters);
					back = lid.body;
					backLid = Scene3D.slice(
						lid.lid,
						Matrix3D.translation(0, 0, -offset).then(
							Matrix3D.turnAboutY(rotation),
						),
						'back',
					);
				}
			}

			if (!allRound)
				slices.push(Scene3D.slice(front, Matrix3D.IDENTITY, 'regular'));
			for (let step = 1; step <= steps; ++step) {
				const share = step / steps;
				slices.push(
					Scene3D.slice(
						backScale !== 1 ? front.interpolated(back, share) : front,
						Matrix3D.turnAboutY(rotation * share),
						'regular',
					),
				);
			}
			if (backLid) slices.push(backLid);
			return slices;
		}

		/// Whether the body of rotation goes all round with no back scale, so its last slice
		/// meets its first.
		private static turnsAllRound(primitive: Primitive3D): boolean {
			return (
				Scene3D.backScaleOf(primitive) === 1 &&
				!!primitive.horizontalSegments &&
				Math.abs((primitive.rotation ?? 0) - 2 * Math.PI) < 1e-9
			);
		}

		/// The scale of the back against the front, one where they are alike; a scale of
		/// nothing is a very small one, as the engine takes it.
		private static backScaleOf(primitive: Primitive3D): number {
			const scale = primitive.backScale ?? 1;
			if (Math.abs(scale - 1) < 1e-9) return 1;
			return Math.abs(scale) < 1e-12 ? 0.000001 : scale;
		}

		/// The 2D outline an extrusion or a body of rotation is made from, as the engine prepared
		/// it.
		private static outlineOf2D(primitive: Primitive3D): PolyPolygon2D {
			return new PolyPolygon2D(
				(primitive.outline ?? []).map((polygon) =>
					Polygon2D.fromFlat(polygon.points, polygon.closed),
				),
			);
		}

		private static slice(
			outline: PolyPolygon2D,
			place: Matrix3D,
			kind: 'front' | 'regular' | 'back',
		): Slice3D {
			return { kind: kind, outline: outline.inSpace(place) };
		}

		/*
			The lid of an end with a bevel of that width, and the outline the body goes on with
			there. As the engine does it, the lid is the outline grown inwards; with letters the
			outline grows outwards instead and both are scaled back to its old size, so a thin
			letter does not cross itself.
		*/
		private static withTheBevel(
			outline: PolyPolygon2D,
			width: number,
			characters: boolean,
		): { body: PolyPolygon2D; lid: PolyPolygon2D } {
			if (width <= 0 || Math.abs(width) < 1e-12)
				return { body: outline, lid: outline };
			if (!characters)
				return { body: outline, lid: outline.grownInNormalDirection(-width) };

			const box = outline.range();
			const grown = outline.grownInNormalDirection(width);
			const grownBox = grown.range();
			const scaleX =
				Math.abs(grownBox.width) < 1e-12 ? 1 : box.width / grownBox.width;
			const scaleY =
				Math.abs(grownBox.height) < 1e-12 ? 1 : box.height / grownBox.height;
			const back = Matrix2D.IDENTITY.translate(-grownBox.minX, -grownBox.minY)
				.scale(scaleX, scaleY)
				.translate(box.minX, box.minY);
			return { body: grown.transformed(back), lid: outline.transformed(back) };
		}
	}
}
