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
	export type Face3DSink = (polygons: Point3D[][]) => boolean;

	/// A 3D object of a scene with the mapping of its primitive's space into the view, where x
	/// and y run over the unit square the scene is seen on and z is the depth.
	interface Scene3DMember {
		id: number;
		primitive: Primitive3D;
		toView: Matrix3D;
		toScene: Matrix3D;
	}

	/// One polygon of the 2D outline an extrusion or a body of rotation is made from.
	interface OutlinePolygon2D {
		closed: boolean;
		points: Point[];
	}

	type Outline2D = OutlinePolygon2D[];

	/// The outline of an extrusion or a body of rotation placed in 3D: a lid that closes the
	/// front or the back, or one of the regular slices between them.
	interface Slice3D {
		kind: 'front' | 'regular' | 'back';
		polygons: { closed: boolean; points: Point3D[] }[];
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
						Scene3D.addTheShape(member, placement, outline);
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
			Scene3D.forEachFace(member.primitive, (polygons: Point3D[][]) => {
				const seen = polygons.map((polygon: Point3D[]) =>
					polygon.map((one: Point3D) => member.toView.apply(one)),
				);
				const onThePage = seen.map((polygon: Point3D[]) =>
					polygon.map((one: Point3D) => toPage.apply(one.x, one.y)),
				);

				const box = Range2D.fromPoints(onThePage[0]);
				if (x < box.minX || x > box.maxX || y < box.minY || y > box.maxY)
					return true;

				const path = new Path2D();
				for (const polygon of onThePage) {
					polygon.forEach((one: Point, at: number) =>
						at ? path.lineTo(one.x, one.y) : path.moveTo(one.x, one.y),
					);
					path.closePath();
				}
				if (!VectorHitTest.isInside(path, x, y)) return true;

				// a face seen on its edge has no depth of its own at a point
				const normal = Scene3D.normalOf(seen[0]);
				if (Math.abs(normal.z) < 1e-12) return true;
				const corner = seen[0][0];
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

		/// The normal of a polygon by Newell's way, which holds for any planar polygon, convex or
		/// not.
		private static normalOf(polygon: Point3D[]): Point3D {
			let x = 0;
			let y = 0;
			let z = 0;
			for (let at = 0; at < polygon.length; ++at) {
				const one = polygon[at];
				const next = polygon[(at + 1) % polygon.length];
				x += (one.y - next.y) * (one.z + next.z);
				y += (one.z - next.z) * (one.x + next.x);
				z += (one.x - next.x) * (one.y + next.y);
			}
			return new Point3D(x, y, z);
		}

		/*
			Adds what shows the shape of one 3D object to the path, on the page in twips, in the
			way that reads best for its kind: a body of rotation by its profile at each step of
			the turn and the ends of it, a sphere by a few of its rings and meridians, and any
			other kind by its silhouette and the edges where its faces meet at an angle.
		*/
		private static addTheShape(
			member: Scene3DMember,
			placement: Matrix2D,
			outline: Path2D,
		): void {
			const onThePage = (point: Point3D): Point => {
				const seen = member.toView.apply(point);
				return placement.apply(seen.x, seen.y);
			};
			if (member.primitive.type === 'lathe')
				Scene3D.addTheLines(
					Scene3D.linesOfALathe(member.primitive),
					outline,
					onThePage,
				);
			else if (member.primitive.type === 'sphere')
				Scene3D.addTheLines(
					Scene3D.linesOfASphere(member.primitive),
					outline,
					onThePage,
				);
			else Scene3D.addTheEdges(member, placement, outline);
		}

		/// Adds lines given in the space of the primitive to the path, each as one line, closed
		/// where it says so.
		private static addTheLines(
			lines: { closed: boolean; points: Point3D[] }[],
			outline: Path2D,
			onThePage: (point: Point3D) => Point,
		): void {
			for (const line of lines) {
				if (line.points.length < 2) continue;
				line.points.forEach((point: Point3D, at: number) => {
					const place = onThePage(point);
					if (at) outline.lineTo(place.x, place.y);
					else outline.moveTo(place.x, place.y);
				});
				if (line.closed) outline.closePath();
			}
		}

		/*
			The lines of a body of rotation: a profile for each eighth of a full turn, as the
			engine draws the slices; where the profile is open, the rings its two ends turn,
			unless an end stays on the axis; and where it does not go all round, the outlines of
			its lids.
		*/
		private static linesOfALathe(
			primitive: Primitive3D,
		): { closed: boolean; points: Point3D[] }[] {
			const slices = Scene3D.latheSlices(primitive);
			const regular = slices.filter(
				(slice: Slice3D) => slice.kind === 'regular',
			);
			const lines: { closed: boolean; points: Point3D[] }[] = [];
			for (const slice of slices)
				if (slice.kind !== 'regular') lines.push(...slice.polygons);

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
						].polygons,
					);
			} else if (regular.length) {
				const steps = Math.min(regular.length - 1, eighths);
				for (let profile = 0; profile <= steps; ++profile)
					lines.push(
						...regular[
							steps ? Math.round((profile * (regular.length - 1)) / steps) : 0
						].polygons,
					);
			}
			const polygons = regular.length ? regular[0].polygons.length : 0;
			for (let polygon = 0; polygon < polygons; ++polygon) {
				if (regular[0].polygons[polygon].closed) continue;
				const last = regular[0].polygons[polygon].points.length - 1;
				for (const end of [0, last]) {
					const ring = regular.map(
						(slice: Slice3D) => slice.polygons[polygon].points[end],
					);
					if (
						ring.every((point: Point3D) => point.minus(ring[0]).length() < 1e-9)
					)
						continue;
					lines.push({ closed: allRound, points: ring });
				}
			}
			return lines;
		}

		/// The lines of a sphere: at most eight of its meridians and the rings between at most six
		/// bands, through the points of its own faces, so they lie on what the engine draws.
		private static linesOfASphere(
			primitive: Primitive3D,
		): { closed: boolean; points: Point3D[] }[] {
			const around = Scene3D.segmentsOf(
				primitive.horizontalSegments,
				2 * Math.PI,
			);
			const down = Scene3D.segmentsOf(primitive.verticalSegments, Math.PI);
			const meridians = Math.min(around, 8);
			const bands = Math.min(down, 6);

			const lines: { closed: boolean; points: Point3D[] }[] = [];
			for (let meridian = 0; meridian < meridians; ++meridian) {
				const piece = Math.round((meridian * around) / meridians);
				const points: Point3D[] = [];
				for (let ring = 0; ring <= down; ++ring)
					points.push(Scene3D.onTheSphere(piece, around, ring, down));
				lines.push({ closed: false, points: points });
			}
			for (let band = 1; band < bands; ++band) {
				const ring = Math.round((band * down) / bands);
				const points: Point3D[] = [];
				for (let piece = 0; piece < around; ++piece)
					points.push(Scene3D.onTheSphere(piece, around, ring, down));
				lines.push({ closed: true, points: points });
			}
			return lines;
		}

		/*
			Adds the silhouette of one 3D object and the edges where its faces meet at an angle
			to the path, on the page in twips: the edges where a face turned towards us meets one
			turned away, and those where neighbouring faces turn by more than the crease angle.
			An edge that only one face has, as on a single polygon, is drawn too. Each edge is
			drawn once, none is hidden by what lies in front of it.
		*/
		private static addTheEdges(
			member: Scene3DMember,
			placement: Matrix2D,
			outline: Path2D,
		): void {
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
			Scene3D.forEachFace(member.primitive, (polygons: Point3D[][]) => {
				// a face whose corners all lie on one line has no area and no edges to show
				const inTheScene = polygons[0].map((one: Point3D) =>
					member.toScene.apply(one),
				);
				const normal = Scene3D.normalOf(inTheScene);
				let longest = 0;
				for (let at = 0; at < inTheScene.length; ++at)
					longest = Math.max(
						longest,
						inTheScene[at]
							.minus(inTheScene[(at + 1) % inTheScene.length])
							.length(),
					);
				if (normal.length() <= 1e-9 * longest * longest) return true;

				// the turn of the face as we see it; which way is towards us is found below
				const face = faces.length;
				const seen = polygons[0].map((one: Point3D) =>
					member.toView.apply(one),
				);
				let area = 0;
				for (let at = 0; at < seen.length; ++at) {
					const next = seen[(at + 1) % seen.length];
					area += seen[at].x * next.y - next.x * seen[at].y;
				}
				faces.push({
					area: area,
					depth:
						seen.reduce((sum: number, one: Point3D) => sum + one.z, 0) /
						seen.length,
					normal: normal,
				});

				for (const polygon of polygons) {
					const ids = polygon.map(idOf);
					for (let at = 0; at < ids.length; ++at) {
						const from = ids[at];
						const to = ids[(at + 1) % ids.length];
						if (from === to) continue;
						const key = Math.min(from, to) * pointLimit + Math.max(from, to);
						const edge = edges.get(key);
						if (edge) edge.faces.push(face);
						else edges.set(key, { from: from, to: to, faces: [face] });
					}
				}
				return true;
			});
			if (!faces.length) return;

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

			Scene3D.addTheChains(drawn, outline, (point: number) => {
				const seen = member.toView.apply(points[point]);
				return placement.apply(seen.x, seen.y);
			});
		}

		/*
			Adds the edges to the path as chains of edges that meet end to end, each drawn as one
			line, so a dashed line runs on evenly along a curve made of many short edges. A
			chain that comes back to where it started is closed. Each edge is given by its two
			points, where the page places a point is asked once.
		*/
		private static addTheChains(
			edges: [number, number][],
			outline: Path2D,
			onThePage: (point: number) => Point,
		): void {
			const edgesAt = new Map<number, number[]>();
			edges.forEach(([from, to], index: number) => {
				for (const point of [from, to]) {
					const there = edgesAt.get(point);
					if (there) there.push(index);
					else edgesAt.set(point, [index]);
				}
			});
			const used = edges.map(() => false);
			const placed = new Map<number, Point>();
			const placeOf = (point: number): Point => {
				let place = placed.get(point);
				if (!place) {
					place = onThePage(point);
					placed.set(point, place);
				}
				return place;
			};

			// of the edges not drawn yet at a point, the one that goes on the straightest from
			// the way the chain came, so a ring or a meridian runs on as one line
			const nextAt = (point: number, from: number): number | undefined => {
				const here = placeOf(point);
				const came = placeOf(from);
				const inX = here.x - came.x;
				const inY = here.y - came.y;
				const inLength = Math.hypot(inX, inY) || 1;
				let best: number | undefined;
				let straightest = -Infinity;
				for (const index of edgesAt.get(point) ?? []) {
					if (used[index]) continue;
					const there = placeOf(otherEnd(index, point));
					const outX = there.x - here.x;
					const outY = there.y - here.y;
					const outLength = Math.hypot(outX, outY) || 1;
					const straight = (inX * outX + inY * outY) / (inLength * outLength);
					if (straight > straightest) {
						straightest = straight;
						best = index;
					}
				}
				return best;
			};
			const otherEnd = (index: number, point: number): number =>
				edges[index][0] === point ? edges[index][1] : edges[index][0];

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
				const chain = behind.slice(2).reverse().concat(ahead);

				const closed = chain.length > 3 && chain[0] === chain[chain.length - 1];
				const last = closed ? chain.length - 1 : chain.length;
				const start = placeOf(chain[0]);
				outline.moveTo(start.x, start.y);
				for (let at = 1; at < last; ++at) {
					const next = placeOf(chain[at]);
					outline.lineTo(next.x, next.y);
				}
				if (closed) outline.closePath();
			});
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
			for (const face of faces) if (!sink([face])) return;
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
					const face = [
						Scene3D.onTheSphere(piece, around, ring, down),
						Scene3D.onTheSphere(piece + 1, around, ring, down),
						Scene3D.onTheSphere(piece + 1, around, ring + 1, down),
						Scene3D.onTheSphere(piece, around, ring + 1, down),
					];
					if (!sink([face])) return;
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
			const polygons = (primitive.polygons ?? []).map(
				(polygon: Primitive3DPolygon) => {
					const points: Point3D[] = [];
					for (let at = 0; at + 2 < polygon.points.length; at += 3)
						points.push(
							new Point3D(
								polygon.points[at],
								polygon.points[at + 1],
								polygon.points[at + 2],
							),
						);
					return points;
				},
			);
			if (polygons.length && polygons[0].length >= 3) sink(polygons);
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
			const flipped = (slice: Slice3D): Point3D[][] =>
				slice.polygons.map((polygon) => polygon.points.slice().reverse());
			const whole = (slice: Slice3D): Point3D[][] =>
				slice.polygons.map((polygon) => polygon.points);

			const pairs = allRound ? count : count - 1;
			if (!pairs) {
				sink(flipped(slices[0]));
				return;
			}

			for (let at = 0; at < pairs; ++at) {
				const one = slices[at];
				const other = slices[(at + 1) % count];
				if (one.kind !== 'regular' || other.kind !== 'regular') continue;

				const before = slices[(at + count - 1) % count];
				if (before.kind === 'front') {
					if (
						!Scene3D.sameSlices(one, before) &&
						!Scene3D.sides(one, before, sink)
					)
						return;
					if (!sink(flipped(before))) return;
				}

				const after = slices[(at + 2) % count];
				if (after.kind === 'back') {
					if (
						!Scene3D.sameSlices(other, after) &&
						!Scene3D.sides(after, other, sink)
					)
						return;
					if (!sink(whole(after))) return;
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
			const polygons = Math.min(one.polygons.length, other.polygons.length);
			for (let polygon = 0; polygon < polygons; ++polygon) {
				const a = one.polygons[polygon];
				const b = other.polygons[polygon];
				const points = Math.min(a.points.length, b.points.length);
				if (!points) continue;
				const edges = a.closed ? points : points - 1;
				for (let edge = 0; edge < edges; ++edge) {
					const next = (edge + 1) % points;
					const face = [
						a.points[edge],
						b.points[edge],
						b.points[next],
						a.points[next],
					];
					if (!sink([face])) return false;
				}
			}
			return true;
		}

		/// Whether two slices lie on the same places, as where a lid has no bevel.
		private static sameSlices(one: Slice3D, other: Slice3D): boolean {
			return (
				one.polygons.length === other.polygons.length &&
				one.polygons.every((polygon, at) => {
					const theirs = other.polygons[at].points;
					return (
						polygon.points.length === theirs.length &&
						polygon.points.every(
							(point, index) => point.minus(theirs[index]).length() < 1e-9,
						)
					);
				})
			);
		}

		/// The slices of an extrusion: the outline at the front and at the back of its depth,
		/// and a lid at each end that is closed.
		private static extrudeSlices(primitive: Primitive3D): Slice3D[] {
			const source = Scene3D.outlineToTurn(primitive);
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
			if (backScale !== 1) front = Scene3D.scaledOnTheCenter(front, backScale);

			const slices: Slice3D[] = [];
			if (primitive.closeFront) {
				const offset = depth * diagonal * 0.5;
				frontDepth = depth - offset;
				const lid = Scene3D.withTheBevel(front, offset, characters);
				front = lid.body;
				slices.push(Scene3D.slice(lid.lid, Scene3D.moved(depth), 'front'));
			}

			let backLid: Outline2D | undefined;
			if (primitive.closeBack) {
				const offset = depth * diagonal * 0.5;
				backDepth = offset;
				const lid = Scene3D.withTheBevel(back, offset, characters);
				back = lid.body;
				backLid = lid.lid;
			}

			slices.push(Scene3D.slice(front, Scene3D.moved(frontDepth), 'regular'));
			slices.push(Scene3D.slice(back, Scene3D.moved(backDepth), 'regular'));
			if (backLid)
				slices.push(Scene3D.slice(backLid, Matrix3D.IDENTITY, 'back'));
			return slices;
		}

		/// The slices of a body of rotation: the outline turned about the y axis in its steps,
		/// and a lid at each end that is closed, where it does not go all round.
		private static latheSlices(primitive: Primitive3D): Slice3D[] {
			const source = Scene3D.outlineToTurn(primitive);
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
			if (backScale !== 1) back = Scene3D.scaledOnTheCenter(back, backScale);

			const slices: Slice3D[] = [];
			let backLid: Slice3D | undefined;
			if (closeFront || closeBack) {
				// the bevel is a twelfth of the way the middle of the outline goes round
				const box = Range2D.fromPoints(
					front.flatMap((polygon: OutlinePolygon2D) => polygon.points),
				);
				const offset = (((box.maxX + box.minX) * rotation) / 2 / 12) * diagonal;
				if (closeFront) {
					const lid = Scene3D.withTheBevel(front, offset, characters);
					front = lid.body;
					slices.push(Scene3D.slice(lid.lid, Scene3D.moved(offset), 'front'));
				}
				if (closeBack) {
					const lid = Scene3D.withTheBevel(back, offset, characters);
					back = lid.body;
					backLid = Scene3D.slice(
						lid.lid,
						Scene3D.moved(-offset).then(Scene3D.turned(rotation)),
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
						backScale !== 1 ? Scene3D.between(front, back, share) : front,
						Scene3D.turned(rotation * share),
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

		/// The 2D outline the body is made from, as the engine prepared it.
		private static outlineToTurn(primitive: Primitive3D): Outline2D {
			return (primitive.outline ?? []).map((polygon) => {
				const points: Point[] = [];
				for (let at = 0; at + 1 < polygon.points.length; at += 2)
					points.push(new Point(polygon.points[at], polygon.points[at + 1]));
				return { closed: polygon.closed, points: points };
			});
		}

		private static slice(
			outline: Outline2D,
			place: Matrix3D,
			kind: 'front' | 'regular' | 'back',
		): Slice3D {
			return {
				kind: kind,
				polygons: outline.map((polygon: OutlinePolygon2D) => ({
					closed: polygon.closed,
					points: polygon.points.map((point: Point) =>
						place.apply(new Point3D(point.x, point.y, 0)),
					),
				})),
			};
		}

		private static moved(depth: number): Matrix3D {
			return Matrix3D.fromArray([
				1,
				0,
				0,
				0,
				0,
				1,
				0,
				0,
				0,
				0,
				1,
				depth,
				0,
				0,
				0,
				1,
			]) as Matrix3D;
		}

		/// A turn about the y axis, turned the way the drawing layer turns.
		private static turned(angle: number): Matrix3D {
			const cos = Math.cos(angle);
			const sin = Math.sin(angle);
			return Matrix3D.fromArray([
				cos,
				0,
				sin,
				0,
				0,
				1,
				0,
				0,
				-sin,
				0,
				cos,
				0,
				0,
				0,
				0,
				1,
			]) as Matrix3D;
		}

		private static scaledOnTheCenter(
			outline: Outline2D,
			scale: number,
		): Outline2D {
			const box = Range2D.fromPoints(
				outline.flatMap((polygon: OutlinePolygon2D) => polygon.points),
			);
			const x = (box.minX + box.maxX) / 2;
			const y = (box.minY + box.maxY) / 2;
			return outline.map((polygon: OutlinePolygon2D) => ({
				closed: polygon.closed,
				points: polygon.points.map(
					(point: Point) =>
						new Point(x + (point.x - x) * scale, y + (point.y - y) * scale),
				),
			}));
		}

		private static between(
			one: Outline2D,
			other: Outline2D,
			share: number,
		): Outline2D {
			return one.map((polygon: OutlinePolygon2D, at: number) => ({
				closed: polygon.closed,
				points: polygon.points.map((point: Point, index: number) => {
					const theirs = other[at].points[index];
					return new Point(
						point.x + (theirs.x - point.x) * share,
						point.y + (theirs.y - point.y) * share,
					);
				}),
			}));
		}

		/*
			The lid of an end with a bevel of that width, and the outline the body goes on with
			there. As the engine does it, the lid is the outline grown inwards; with letters the
			outline grows outwards instead and both are scaled back to its old size, so a thin
			letter does not cross itself.
		*/
		private static withTheBevel(
			outline: Outline2D,
			width: number,
			characters: boolean,
		): { body: Outline2D; lid: Outline2D } {
			if (width <= 0 || Math.abs(width) < 1e-12)
				return { body: outline, lid: outline };
			if (!characters)
				return { body: outline, lid: Scene3D.grown(outline, -width) };

			const box = Range2D.fromPoints(
				outline.flatMap((polygon: OutlinePolygon2D) => polygon.points),
			);
			const grown = Scene3D.grown(outline, width);
			const grownBox = Range2D.fromPoints(
				grown.flatMap((polygon: OutlinePolygon2D) => polygon.points),
			);
			const scaleX =
				Math.abs(grownBox.width) < 1e-12 ? 1 : box.width / grownBox.width;
			const scaleY =
				Math.abs(grownBox.height) < 1e-12 ? 1 : box.height / grownBox.height;
			const back = (polygons: Outline2D): Outline2D =>
				polygons.map((polygon: OutlinePolygon2D) => ({
					closed: polygon.closed,
					points: polygon.points.map(
						(point: Point) =>
							new Point(
								(point.x - grownBox.minX) * scaleX + box.minX,
								(point.y - grownBox.minY) * scaleY + box.minY,
							),
					),
				}));
			return { body: back(grown), lid: back(outline) };
		}

		/// Each point moved that far along the way between the normals of its two edges, as the
		/// drawing layer grows a polygon.
		private static grown(outline: Outline2D, width: number): Outline2D {
			const across = (x: number, y: number): Point => {
				const length = Math.hypot(x, y);
				return length ? new Point(-y / length, x / length) : new Point(0, 0);
			};
			return outline.map((polygon: OutlinePolygon2D) => {
				const points = polygon.points;
				return {
					closed: polygon.closed,
					points: points.map((current: Point, at: number) => {
						const before = points[(at + points.length - 1) % points.length];
						const next = points[(at + 1) % points.length];
						const back = across(before.x - current.x, before.y - current.y);
						const forth = across(next.x - current.x, next.y - current.y);
						let x = back.x - forth.x;
						let y = back.y - forth.y;
						const length = Math.hypot(x, y);
						if (length) {
							x = (x / length) * width;
							y = (y / length) * width;
						}
						return new Point(current.x + x, current.y + y);
					}),
				};
			});
		}
	}
}
