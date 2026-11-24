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

	/*
		A 3D scene as the client works it out from its 3D objects: the faces of each kind, the hit
		of a point on the page as the engine finds it, and the outline that follows the shape. The
		faces are made on the fly and handed out one by one, nothing of them is kept.
	*/
	export class Scene3D {
		static readonly sceneKind = 202;

		// the kinds whose faces we make here, the others leave the scene to its picture
		private static readonly kindsMade = ['cube', 'sphere', 'polyPolygon'];

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
			page in twips: the edges where a face turned towards us meets one turned away, the
			silhouette, and the edges where faces meet at an angle, of which at least one is
			turned towards us. An edge that only one face has, as on a single polygon, is drawn
			too. Each edge is drawn once.
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
						Scene3D.addTheEdges(member, placement, outline);
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

		/// Adds the edges that show the shape of one 3D object to the path, on the page in twips.
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
				const face = faces.length;
				const seen = polygons[0].map((one: Point3D) =>
					member.toView.apply(one),
				);
				// the turn of the face as we see it; which way is towards us is found below
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
					normal: Scene3D.normalOf(
						polygons[0].map((one: Point3D) => member.toScene.apply(one)),
					),
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

			for (const edge of edges.values()) {
				let shown = edge.faces.length === 1;
				if (edge.faces.length >= 2) {
					const [one, other] = edge.faces;
					if (towardsUs(one) !== towardsUs(other)) shown = true;
					else if (towardsUs(one)) {
						const a = faces[one].normal;
						const b = faces[other].normal;
						const lengths = a.length() * b.length();
						shown = lengths > 0 && a.dot(b) / lengths < creaseCosine;
					}
				}
				if (!shown) continue;

				const from = member.toView.apply(points[edge.from]);
				const to = member.toView.apply(points[edge.to]);
				const start = placement.apply(from.x, from.y);
				const end = placement.apply(to.x, to.y);
				outline.moveTo(start.x, start.y);
				outline.lineTo(end.x, end.y);
			}
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
			const segments = (given: number | undefined, span: number): number =>
				Math.min(
					512,
					Math.max(1, given ? given : Math.round(span / (Math.PI / 12))),
				);
			const around = segments(primitive.horizontalSegments, 2 * Math.PI);
			const down = segments(primitive.verticalSegments, Math.PI);

			const pointAt = (horizontal: number, vertical: number): Point3D => {
				const cosVertical = Math.cos(vertical);
				return new Point3D(
					(cosVertical * Math.cos(horizontal) + 1) / 2,
					(Math.sin(vertical) + 1) / 2,
					(-cosVertical * Math.sin(horizontal) + 1) / 2,
				);
			};

			for (let ring = 0; ring < down; ++ring) {
				const top = Math.PI / 2 - (Math.PI * ring) / down;
				const bottom = Math.PI / 2 - (Math.PI * (ring + 1)) / down;
				for (let piece = 0; piece < around; ++piece) {
					const left = (2 * Math.PI * piece) / around;
					const right = (2 * Math.PI * (piece + 1)) / around;
					const face = [
						pointAt(left, top),
						pointAt(right, top),
						pointAt(right, bottom),
						pointAt(left, bottom),
					];
					if (!sink([face])) return;
				}
			}
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
	}
}
