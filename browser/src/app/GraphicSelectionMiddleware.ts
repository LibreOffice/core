// @ts-strict-ignore
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

declare var JSDialog: any;

class GraphicSelection {
	/*
		The drawing objects the selection stands on, by the unique ids the engine gives them, empty
		while nothing is selected. They all sit at the same level: directly on the page, or in one
		and the same group.
	*/
	public static selectedObjectIDs: number[] = [];

	/*
		True when the selection that arrived last was made somewhere other than here - the
		keyboard, an undo, another user - and false when it is the one this client asked for.
	*/
	public static selectionCameFromElsewhere: boolean = false;
	public static rectangle: cool.SimpleRectangle | null = null;
	public static extraInfo: any = null;
	public static selectionAngle: number = 0;
	public static inPlaceActive: boolean = false;
	public static tileTwipsOffset: number[] = [0, 0];
	public static inPlaceArea: number[] | null = null;
	public static inPlaceMath: boolean = false;
	public static inPlaceTileOffset: number[] = [0, 0];
	private static selectionEdges: number[] | null = null;
	public static handlesSection: ShapeHandlesSection = null;
	public static chartContextToolbarSelectStyle: ChartContextButtonSection =
		null;
	public static chartContextToolbarSaveStyle: ChartContextButtonSection = null;
	public static diagramButton: DiagramButtonSection = null;
	/// The object the dark overlay was last laid out for.
	public static darkOverlayRectangle: cool.SimpleRectangle | null = null;

	/// The player of a selected video or audio object while the client draws the selection
	/// itself, null while none is selected or the tiled sections host it.
	public static mediaPlayer: MediaPlayerSection | null = null;

	/*
		Whether anything is selected. While the document is drawn from objects that is what the
		ids say, and the rectangle the engine sends is not needed for it; otherwise it is the
		rectangle, since that is all there is.
	*/
	public static hasActiveSelection() {
		if (RenderManager.isVectorRendering())
			return this.selectedObjectIDs.length > 0;

		return this.rectangle !== null;
	}

	/// The one selected object, or nothing where none or several are selected.
	private static theOneSelected(): cool.SlideObject | undefined {
		return this.selectedObjectIDs.length === 1
			? RenderGeometrySection.objectOf(this.selectedObjectIDs[0])
			: undefined;
	}

	/// The file name ending of the picture what is selected holds, or nothing where it holds
	/// none. Asked the same two ways.
	public static graphicExtension(): string | undefined {
		if (RenderManager.isVectorRendering())
			return this.theOneSelected()?.graphicExtension || undefined;

		return this.extraInfo?.graphicExtension || undefined;
	}

	/*
		Whether the crop marks of a picture are shown and may be taken hold of. It is the crop
		switch of the toolbar, kept here while the client draws the selection itself: the engine
		is never asked to enter its crop mode, since that would have it send a drag mode of its
		own that only cropping and turning are offered in.
	*/
	public static cropping: boolean = false;

	/// Whether the crop switch is the client's to take: it is while the client draws the
	/// selection itself.
	public static takesTheCropSwitch(): boolean {
		return RenderManager.isVectorRendering();
	}

	public static toggleCropping(): void {
		this.cropping = !this.cropping;

		// The switch is shown pressed or not by the state every other command has.
		app.map.fire('commandstatechanged', {
			commandName: '.uno:Crop',
			state: this.cropping ? 'true' : 'false',
		});

		this.selectionSection?.refresh();
		app.sectionContainer?.requestReDraw();
	}

	/// Whether the document is being signed, in which case what is selected may be moved and
	/// resized although the document is read-only. Asked the same two ways.
	public static isBeingSigned(): boolean {
		if (RenderManager.isVectorRendering())
			return this.theOneSelected()?.signing === true;

		return this.extraInfo?.isSignature === true;
	}

	public static onUpdatePermission() {
		this.rectangle = null;
		this.updateGraphicSelection();
	}

	/// True while the client named something to the engine that is no place to stand in, a
	/// diagram alone, for a command to work on: the report that comes of it is taken as it is.
	private static nextReportAsItIs: boolean = false;

	/// Takes the engine's next report of the selection as it comes.
	public static takeTheNextReportAsItIs(): void {
		this.nextReportAsItIs = true;
	}

	/// The state of each of those last given, so a state is told only when it changes.
	private static statesGiven: { [command: string]: string } = {};

	/*
		Whether the state of that command is given here, so the engine's is left aside. One the
		engine sends before or after the view draws from the objects reaches the widgets, and what
		was given here for it last is forgotten, so the next state given here is told whatever it
		is.
	*/
	public static givesTheState(commandName: string): boolean {
		if (!Interaction.statesGivenHere.includes(commandName)) return false;
		if (RenderManager.isVectorRendering()) return true;

		delete GraphicSelection.statesGiven[commandName];
		return false;
	}

	/*
		Tells the states this view gives itself, as the engine tells a state: to the client's
		widgets, and to the application around it where there is one. Only a state that changed
		is told.
	*/
	public static tellTheStates(): void {
		if (!RenderManager.isVectorRendering()) return;

		// The states are worked out from the selected objects, so they wait until every one of
		// them is there: an update of the objects may take one away for a moment.
		if (
			this.selectedObjectIDs.some(
				(objectId: number) => !RenderGeometrySection.objectOf(objectId),
			)
		)
			return;

		const states = Interaction.context().commandStates();
		for (const commandName of Interaction.statesGivenHere) {
			const state = states[commandName] ? 'enabled' : 'disabled';
			if (GraphicSelection.statesGiven[commandName] === state) continue;
			GraphicSelection.statesGiven[commandName] = state;

			const json = { commandName: commandName, state: state };
			app.map.fire('commandstatechanged', json);
			const host = window as unknown as {
				ThisIsTheMacOSApp?: boolean;
				ThisIsTheQtApp?: boolean;
				ThisIsTheWindowsApp?: boolean;
				postMobileMessage?: (message: string) => void;
			};
			if (
				host.ThisIsTheMacOSApp ||
				host.ThisIsTheQtApp ||
				host.ThisIsTheWindowsApp
			)
				host.postMobileMessage?.('COMMANDSTATECHANGED ' + JSON.stringify(json));
		}
	}

	/*
		Runs a command whose meaning a view that draws from the objects gives itself: entering and
		leaving a group, which are the context's, as Enter, F3 and Ctrl+F3 are, and taking apart
		what is selected, which in a diagram is the diagram. Answers whether it was run here; one
		that is not goes to the engine.
	*/
	public static takesTheCommand(command: string): boolean {
		if (!RenderManager.isVectorRendering()) return false;
		if (command === '.uno:EnterGroup') return Interaction.context().enterKey();
		if (command === '.uno:LeaveGroup') return Interaction.context().leaveKey();
		if (command === '.uno:FormatUngroup')
			return Interaction.context().ungroup();
		return false;
	}

	/*
		Marks the drawing objects with the given unique ids, and nothing else. The engine is told
		which objects they are rather than where the click was, so what the client hit and what the
		engine marks are one thing. An empty list asks for no selection at all.
	*/
	public static selectObjects(objectIds: number[]) {
		this.selectedObjectIDs = objectIds.slice();
		SelectionSection.selectedHere(objectIds);
		// Nothing selected by asking for it is where one meant to be; nothing is to follow.
		this.rememberWhereItStands(objectIds);
		if (!objectIds.length) this.stoodAt = null;
		app.socket.sendMessage('selectobjects ids=' + objectIds.join(','));
		this.tellTheStates();
	}

	/*
		The selection as it stood last with something in it, and the groups above it, from its
		parent up. The engine drops objects that are deleted from what it has marked and reports
		what is left, possibly nothing, before the objects of the page say they are gone; this is
		what the selection is put right from once they do.
	*/
	private static stoodAt: {
		selected: number[];
		groupsAbove: number[];
		place: InteractionPlace;
	} | null = null;

	/// Whether the update of the objects is listened to, which is once.
	private static followingDeletions: boolean = false;

	/// Notes where a selection with something in it stands, in a view that draws from objects.
	private static rememberWhereItStands(objectIds: number[]): void {
		if (!objectIds.length || !RenderManager.isVectorRendering()) return;

		this.stoodAt = {
			selected: objectIds.slice(),
			groupsAbove: Interaction.groupsAbove(objectIds),
			place: Interaction.placeOf(objectIds),
		};

		if (this.followingDeletions) return;
		this.followingDeletions = true;
		RenderManager.onVectorChanged(() => {
			GraphicSelection.followDeletions();
			GraphicSelection.followTheTextEdit();
		});
	}

	/*
		Puts the selection right where objects it stood on were deleted, by this view or another:
		the context it stood in says what to take instead. Where nothing it stood on is gone, the
		selection stands as the engine reports it.
	*/
	private static followDeletions(): void {
		const stoodAt = this.stoodAt;
		if (!stoodAt || !RenderManager.isVectorRendering()) return;

		// The objects of the page are dropped from memory at times, when the window loses the
		// focus among others, and fetched again; while they are away they say nothing about what
		// was deleted.
		if (!RenderGeometrySection.currentPart()) return;

		const gone = stoodAt.selected.some(
			(objectId: number) =>
				RenderGeometrySection.objectOf(objectId) === undefined,
		);
		if (!gone) {
			if (!this.selectedObjectIDs.length) this.stoodAt = null;
			return;
		}

		const wanted = Interaction.contextAt(stoodAt.place).whatIsLeft(
			stoodAt.selected,
			stoodAt.groupsAbove,
		);
		const same =
			wanted.length === this.selectedObjectIDs.length &&
			wanted.every(
				(objectId: number, index: number) =>
					objectId === this.selectedObjectIDs[index],
			);

		// The selection is what is left already; with nothing left there is nothing to follow.
		if (same) {
			if (wanted.length) this.rememberWhereItStands(wanted);
			else this.stoodAt = null;
			return;
		}

		this.selectAndShow(wanted);
	}

	/// The engine was asked to enter the selected group; the context says what is selected then.
	public static takeTheFirstOfTheGroup(): void {
		if (!RenderManager.isVectorRendering()) return;
		Interaction.context().groupEntered();
	}

	/*
		Takes up the selection the engine reports and says whether it is another one than the
		client holds. It is the same one whenever the client asked for it, since the answer names
		the objects it named itself.
	*/
	public static selectionChanged(objectIds: number[]): boolean {
		const changed =
			objectIds.length !== this.selectedObjectIDs.length ||
			objectIds.some(
				(objectId: number, index: number) =>
					objectId !== this.selectedObjectIDs[index],
			);

		// The keyboard works on a handle of what is selected, so another selection leaves it.
		if (changed) GraphicSelection.travel()?.leave();

		this.selectedObjectIDs = objectIds.slice();
		this.selectionCameFromElsewhere = changed;
		this.rememberWhereItStands(objectIds);
		this.tellTheStates();

		// A selection the engine made that is no place to stand, a diagram alone for one, is
		// taken as the context says, and the engine is told - unless the client named it to the
		// engine on purpose and waits for what comes of it.
		const asItIs = this.nextReportAsItIs;
		this.nextReportAsItIs = false;
		if (RenderManager.isVectorRendering() && !asItIs) {
			const wanted = Interaction.context().enteredBy(objectIds);
			if (
				wanted.length !== objectIds.length ||
				wanted.some(
					(objectId: number, index: number) => objectId !== objectIds[index],
				)
			)
				this.selectObjects(wanted);
		}

		return changed;
	}

	/// The objects the engine says are selected, in the order it marked them.
	private static selectedObjectIDsOf(extraInfo: any): number[] {
		const objectIds = extraInfo?.uniqueIds;
		return Array.isArray(objectIds) ? objectIds : [];
	}

	/*
		The boxes of the other objects of the page, in pixels, for a drag to snap to. While the
		document is drawn from objects the client works them out from what it holds; otherwise they
		are the ones the engine sends along with the selection. What is being dragged is left out,
		since a drag does not snap to itself.
	*/
	public static snapRectangles(): number[][] {
		if (RenderManager.isVectorRendering()) {
			// The boxes the client holds are twips already, where the ones the engine sends are
			// hundredths of a millimetre and are corrected on their way in.
			const scale = app.twipsToPixels;
			const rectangles: number[][] = [];

			RenderGeometrySection.objectBoxes().forEach(
				(box: number[], objectId: number) => {
					if (this.selectedObjectIDs.includes(objectId)) return;
					rectangles.push(box.map((value: number) => value * scale));
				},
			);

			return rectangles;
		}

		const rectangles = this.extraInfo?.ObjectRectangles;
		if (!Array.isArray(rectangles)) return [];

		const ordNum = this.extraInfo.OrdNum;
		return rectangles.filter((rectangle: number[]) => rectangle[4] !== ordNum);
	}

	/*
		Works the handles out again from the objects as they stand now. The engine reports a
		selection as soon as it changes, while the objects it stands on arrive with the update
		that follows, so the handles of a shape that was just dragged are worked out once more when
		that update lands. Without it they would show where the shape was before.
	*/
	public static refreshLocalHandles(): void {
		// The section that draws the selection works its handles out from the objects, so it is
		// told whenever they change, whatever kind of handles they are.
		this.selectionSection?.refresh();

		// The buttons beside the bands follow the objects wherever they take them, as does the
		// player of a video. Which buttons there are is asked of the objects too, which may
		// arrive after the selection.
		this.updateGroupButtons();
		this.followThePlayer();

		// The states depend on the objects as well as on the selection: their order at the level,
		// and what each allows.
		this.tellTheStates();
	}

	/*
		Takes in that the engine answered something for that object which was not predicted here
		- or, given no object, that anything may have changed. Where it touches the selection, the
		selection's square is dropped, so that the handles are laid on what is really there. Where
		it is one of the selected objects, changed by something other than this view, such as an
		undo, the selection is brought on screen as it now stands.
	*/
	public static unpredictedAnswer(objectId?: number): void {
		const section = this.selectionSection;
		if (!section) return;

		section.theEngineAnswered(objectId);
		if (
			objectId === undefined ||
			RenderGeometrySection.theOnesAChangeTouches(
				section.selectedObjects(),
			).includes(objectId)
		)
			section.forgetTheSquare();

		if (
			objectId !== undefined &&
			!section.isDragged() &&
			RenderGeometrySection.theOnesThatDraw(
				RenderGeometrySection.standingFor(section.selectedObjects()),
			).includes(objectId)
		)
			GraphicSelection.showTheObjects(this.selectedObjectIDs);
	}

	/// The section that draws what is selected while the client draws it itself, null while it
	/// draws from tiles or while nothing is selected.
	public static selectionSection: SelectionSection | null = null;

	/// The keyboard on the handles of what is selected, where the client holds them itself.
	public static travel(): HandleTravel | undefined {
		return GraphicSelection.selectionSection?.travel;
	}

	/// The way each of the four arrow keys moves what is selected.
	private static readonly theWayOf: { [key: string]: number[] } = {
		ArrowUp: [0, -1],
		ArrowDown: [0, 1],
		ArrowLeft: [-1, 0],
		ArrowRight: [1, 0],
	};

	/*
		What the keyboard does to what is selected, while the client draws the selection itself.
		A handle takes the key first, where the keyboard is on one. An arrow key with the
		keyboard on no handle moves the objects, and that is done here and sent as the one change
		it is, never as a key: the engine would move them by a pixel of its own view, which is
		not the pixel seen here and depends on the zoom, and it asks only the objects that are
		marked, not the ones they hold, so it would move a group whose member is held fast.
	*/
	public static handleKeyboard(event: KeyboardEvent): boolean {
		// While text is being edited every key belongs to the text.
		if (this.textIsBeingEdited()) return false;

		if (!this.hasActiveSelection()) {
			if (!RenderManager.isVectorRendering()) return false;
			const way = GraphicSelection.theWayOf[event.key];
			if (way) {
				return Interaction.context().arrowKey(
					new cool.Point(way[0], way[1]),
					event,
				);
			}
			return this.travelByKey(event);
		}

		// A drag by the mouse owns the selection while it runs, so a key does nothing to it then
		// and is taken here. Escape goes on to call the drag off, and the selection stays.
		if (this.selectionSection?.isDragged()) return event.key !== 'Escape';

		// What a key changes goes to the engine once the keys are let go, as the last state of
		// each thing it changed. It is shown here at once all the same.
		return ChangeOutbox.byAKey(event, () => {
			if (GraphicSelection.travel()?.keyboard(event)) return true;

			if (!RenderManager.isVectorRendering()) return false;

			if (event.key === 'Escape') return Interaction.context().escape();
			if (this.travelByKey(event)) return true;
			if (this.selectionSection?.resetByKey(event)) return true;
			if (
				event.key === 'Enter' &&
				!event.shiftKey &&
				!event.ctrlKey &&
				!event.altKey &&
				!event.metaKey
			)
				return Interaction.context().enterKey();

			const way = GraphicSelection.theWayOf[event.key];
			if (!way) return false;

			return Interaction.context().arrowKey(
				new cool.Point(way[0], way[1]),
				event,
			);
		});
	}

	/*
		The keys that go from object to object and in and out of groups: Tab and Shift+Tab,
		Ctrl+Home and Ctrl+End, F3 and Ctrl+F3. Answers whether the context used the key.
	*/
	private static travelByKey(event: KeyboardEvent): boolean {
		const context = Interaction.context();
		const ctrl = event.ctrlKey || event.metaKey;

		if (event.key === 'Tab' && !ctrl && !event.altKey)
			return context.tabKey(!event.shiftKey);
		if (
			(event.key === 'Home' || event.key === 'End') &&
			ctrl &&
			!event.shiftKey
		)
			return context.toAnEnd(event.key === 'Home');
		if (event.key === 'F3' && !event.shiftKey && !event.altKey)
			return ctrl ? context.leaveKey() : context.enterKey();
		return false;
	}

	/*
		Whether this view is editing text: the text cursor shows, or, in a view that draws from the
		objects, the page holds a text edit entry of this view.
	*/
	private static textIsBeingEdited(): boolean {
		if (app.file.textCursor.visible) return true;
		return this.objectUnderTextEdit() !== undefined;
	}

	/// The object this view edits the text of, in a view that draws from the objects: the one its
	/// text edit entry belongs to. Nothing while it edits none.
	private static objectUnderTextEdit(): number | undefined {
		if (!RenderManager.isVectorRendering()) return undefined;

		const data = RenderGeometrySection.currentPart();
		const ownViewId = (
			app.map?._docLayer as unknown as cool.CanvasTileLayerInterface | undefined
		)?._viewId;
		if (!data || ownViewId === undefined) return undefined;

		for (const object of data.objects.values())
			if (object.kind === 'texteditoverlay' && object.viewId === ownViewId)
				return object.parent;
		return undefined;
	}

	/*
		Takes the object this view edits the text of as the selection, where the edit moved on to
		another one: a press during a text edit goes to the engine, which in a diagram edits the
		object pressed without marking anything new, so no report says so. The engine works on that
		object already, and is not told again, which would end the edit.
	*/
	private static followTheTextEdit(): void {
		const edited = this.objectUnderTextEdit();
		if (!edited || this.selectedObjectIDs.includes(edited)) return;
		if (!RenderGeometrySection.objectOf(edited)) return;

		this.selectedObjectIDs = [edited];
		this.rememberWhereItStands([edited]);
		this.updateSelectionSection();
		this.tellTheStates();
	}

	/*
		The view switched between drawing from tiles and drawing from the objects. The selection
		ends, the engine's included, with all that shows it: the handles drawn from the engine's
		message, the section that draws it, the button of a diagram and the buttons beside the
		bands.
	*/
	public static drawingSwitched(): void {
		if (this.handlesSection) {
			this.handlesSection.removeSubSections();
			app.sectionContainer.removeSection(this.handlesSection.name);
			this.handlesSection = null;
		}
		if (this.diagramButton) {
			app.sectionContainer.removeSection(this.diagramButton.name);
			this.diagramButton = null;
		}
		this.rectangle = null;
		this.selectObjects([]);
		this.updateSelectionSection();
		this.updateGroupButtons();
	}

	/*
		Makes the section that draws what is selected, or takes it away again. A single object
		has a section of its own, which knows the object by its id; several of them are held
		together by one section that frames them all.
	*/
	public static updateSelectionSection(): void {
		this.makeSelectionSection();

		// The buttons stand beside the bands the section draws, so they are placed once the
		// section stands for the selection as it is now.
		this.updateGroupButtons();
	}

	/// The buttons beside the bands of groups and diagrams, by name.
	private static groupButtons: Map<string, GroupButtonSection> = new Map();

	/*
		Makes the columns of buttons beside the bands of groups and diagrams match the selection:
		which there are, whether each may be pressed, and where it stands now. A column runs down
		from the top of its band and no further than the band's bottom, so the buttons that do not
		fit are left out from the bottom up. A view that draws from the tiles has none.
	*/
	public static updateGroupButtons(): void {
		const wanted = new Map<
			string,
			{ objectId: number; role: GroupButtonRole; slot: number }
		>();
		const section = this.selectionSection;

		if (RenderManager.isVectorRendering() && section) {
			for (const objectId of section.bandsWithButtons()) {
				const band = section.bandBox(objectId);
				if (!band) continue;

				// A column of n buttons takes n steps but the space under the last one.
				const room =
					band.height +
					HandleLook.reachOfASurround() +
					GroupButtonSection.spaceBetween * app.dpiScale;
				const fit = Math.floor(room / GroupButtonSection.step());

				GraphicSelection.buttonsBeside(objectId)
					.slice(0, fit)
					.forEach((role: GroupButtonRole, slot: number) =>
						wanted.set(GroupButtonSection.nameFor(objectId, role), {
							objectId,
							role,
							slot,
						}),
					);
			}
		}

		for (const [name, button] of this.groupButtons)
			if (!wanted.has(name)) {
				app.sectionContainer.removeSection(button.name);
				this.groupButtons.delete(name);
			}

		for (const [name, one] of wanted) {
			let button = this.groupButtons.get(name);
			if (!button) {
				button = new GroupButtonSection(one.objectId, one.role, one.slot);
				app.sectionContainer.addSection(button);
				this.groupButtons.set(name, button);
			}
			button.standAt(
				one.slot,
				GraphicSelection.groupButtonIsOn(one.objectId, one.role),
			);
		}
	}

	/// Moves the buttons beside the bands to where their bands are now.
	public static followTheGroupButtons(): void {
		for (const button of this.groupButtons.values()) button.follow();
	}

	/*
		The buttons beside the band of that group or diagram, from the top down. A diagram takes
		the one that opens it and Ungroup. A group takes Enter, Leave and Ungroup, and the one
		that opens a diagram above them where it holds a diagram at any depth, so that its column
		keeps one shape however the selection moves inside it.
	*/
	private static buttonsBeside(objectId: number): GroupButtonRole[] {
		if (RenderGeometrySection.objectOf(objectId)?.isDiagram)
			return ['diagram', 'ungroup'];

		const roles: GroupButtonRole[] = ['enter', 'leave', 'ungroup'];
		return GraphicSelection.holdsADiagram(objectId)
			? ['diagram', ...roles]
			: roles;
	}

	/// Whether that group holds a diagram at any depth.
	private static holdsADiagram(objectId: number, depth: number = 0): boolean {
		return RenderGeometrySection.heldBy(objectId).some(
			(held: number) =>
				RenderGeometrySection.objectOf(held)?.isDiagram === true ||
				(depth < 100 && GraphicSelection.holdsADiagram(held, depth + 1)),
		);
	}

	/*
		Whether a button beside the band of that group or diagram may be pressed. Where that one is
		selected itself, alone or among others, the buttons act on it: Ungroup, Enter for a group
		and the one that opens a diagram for a diagram. Where the selection sits inside it, they
		act on the selection as the menu does, with the menu's states, and the one that opens a
		diagram where the selection stands in a diagram.
	*/
	private static groupButtonIsOn(
		objectId: number,
		role: GroupButtonRole,
	): boolean {
		const selected = this.selectedObjectIDs;
		if (selected.includes(objectId)) {
			const aDiagram =
				RenderGeometrySection.objectOf(objectId)?.isDiagram === true;
			if (role === 'ungroup') return true;
			return role === 'diagram' ? aDiagram : role === 'enter' && !aDiagram;
		}

		switch (role) {
			case 'diagram':
				return Interaction.placeOf(selected).kind === 'diagram';
			case 'enter':
				return (
					Interaction.context().commandStates()['.uno:EnterGroup'] === true
				);
			case 'leave':
				return (
					Interaction.context().commandStates()['.uno:LeaveGroup'] === true
				);
			case 'ungroup':
				return (
					Interaction.context().commandStates()['.uno:FormatUngroup'] === true
				);
		}
	}

	/*
		A press on a button beside the band of that group or diagram. Where that one is selected
		itself, the press acts on it alone: it is selected on its own and opened, entered or taken
		apart, and taking it apart leaves the others selected. Where the selection sits inside it,
		the press is the menu's command for the selection.
	*/
	public static pressTheGroupButton(
		objectId: number,
		role: GroupButtonRole,
	): void {
		const itself = this.selectedObjectIDs.includes(objectId);

		switch (role) {
			case 'diagram':
				if (itself) {
					this.selectObjects(Interaction.context().enteredBy([objectId]));
					this.updateSelectionSection();
				}
				app.map.sendUnoCommand('.uno:EditDiagram');
				return;
			case 'enter':
				if (!itself) {
					app.map.sendUnoCommand('.uno:EnterGroup');
					return;
				}
				this.selectObjects([objectId]);
				Interaction.context().enterKey();
				return;
			case 'leave':
				app.map.sendUnoCommand('.uno:LeaveGroup');
				return;
			case 'ungroup':
				if (itself) this.takeApart(objectId);
				else app.map.sendUnoCommand('.uno:FormatUngroup');
				return;
		}
	}

	/*
		Takes that group or diagram apart on its own. The engine is named the object itself first,
		which for a diagram is no place for the client to stand in, so the client's selection
		stays as it is until the engine reports what taking it apart left selected.
	*/
	public static takeApart(objectId: number): void {
		this.takeTheNextReportAsItIs();
		app.socket.sendMessage('selectobjects ids=' + String(objectId));
		app.socket.sendMessage('uno .uno:FormatUngroup');
	}

	private static makeSelectionSection(): void {
		const objectIds = this.selectedObjectIDs;
		const several = objectIds.length > 1;
		const wanted =
			RenderManager.isVectorRendering() && objectIds.length
				? several
					? MultiSelectionSection.sectionName
					: ObjectSelectionSection.nameFor(objectIds[0])
				: null;

		if (this.selectionSection && this.selectionSection.name !== wanted) {
			app.sectionContainer.removeSection(this.selectionSection.name);
			this.selectionSection = null;
			RenderManager.drawnElsewhere([]);
			this.dropThePlayer();
		}

		if (wanted === null) return;

		if (!this.selectionSection) {
			this.selectionSection = several
				? new MultiSelectionSection(objectIds)
				: new ObjectSelectionSection(objectIds[0]);

			// A section takes its place among the others only once it is in the container, so
			// the handles are worked out after it has been added and not before.
			app.sectionContainer.addSection(this.selectionSection as any);
			this.selectionSection.refresh();
			return;
		}

		if (several)
			(this.selectionSection as MultiSelectionSection).setObjects(objectIds);
		else this.selectionSection.refresh();
	}

	/*
		How far the view scrolls along one axis to show a stretch of the document that starts at
		start and is length long, where the view shows viewStart and is viewLength long. Returns
		the new start of the view.

		A stretch that fits in the view is scrolled to in whole steps of the free space, the view
		minus the stretch, so that there is room left between the stretch and the edge it came in
		over. A stretch longer than the view is scrolled to by half a view, and only once it lies
		outside the middle part of the view, which leaves the reader something to recognise. The
		view only scrolls, it never zooms.
	*/
	private static scrolledStart(
		viewStart: number,
		viewLength: number,
		start: number,
		length: number,
	): number {
		const free = Math.min(viewLength - length, length);

		if (viewLength < length) {
			// A fifteenth-hundredth part of the view on each side is the middle part.
			const border = Math.round((viewLength * 30) / 200);

			if (viewStart + border > start + length)
				return viewStart - Math.round(viewLength / 2);

			if (viewStart + viewLength - border < start)
				return viewStart + Math.round(viewLength / 2);

			return viewStart;
		}

		if (free <= 0) return viewStart;

		let scrolled = viewStart;

		const beyondEnd = start + length - scrolled - viewLength;
		if (beyondEnd > 0) scrolled += (Math.floor(beyondEnd / free) + 1) * free;

		const beforeStart = scrolled - start;
		if (beforeStart > 0)
			scrolled -= (Math.floor(beforeStart / free) + 1) * free;

		return scrolled;
	}

	/*
		Selects those objects and brings them on screen, for a selection that does not come from
		pressing at a point: a key, a command, or what is left after a deletion. A press always
		lands on something in view, and the view stays put for it.
	*/
	public static selectAndShow(objectIds: number[]): void {
		this.selectObjects(objectIds);
		this.updateSelectionSection();
		this.showTheObjects(objectIds);
	}

	/// Brings those objects on screen, with the room around them their handles are drawn in.
	/// Answers whether the client holds any of them to do so.
	public static showTheObjects(objectIds: number[]): boolean {
		let box: cool.Range2D | undefined;
		for (const objectId of objectIds) {
			const own = RenderGeometrySection.boxOf(objectId);
			if (own) box = box ? box.union(own) : own;
		}
		if (!box) return false;

		// The room a handle is drawn in goes round the box, so the handles are on screen too.
		const room = 1.5 * ShapeHandlesSection.handleSize() * app.pixelsToTwips;
		const shown = box.expand(room, room);

		GraphicSelection.scrollRectangleIntoView(
			new cool.SimpleRectangle(
				shown.minX,
				shown.minY,
				shown.width,
				shown.height,
			),
		);
		return true;
	}

	/*
		Brings a point on screen, with the room around it a handle is drawn in, so that whatever
		sits there is never up against an edge of the view.
	*/
	public static scrollPointIntoView(point: cool.SimplePoint): void {
		const size = ShapeHandlesSection.handleSize() * app.pixelsToTwips;

		GraphicSelection.scrollRectangleIntoView(
			new cool.SimpleRectangle(
				point.x - 1.5 * size,
				point.y - 1.5 * size,
				3 * size,
				3 * size,
			),
		);
	}

	/// Scrolls so that the whole rectangle is seen, the way a presentation scrolls to what it
	/// marks. A rectangle that is seen already leaves the view where it is.
	public static scrollRectangleIntoView(rectangle: cool.SimpleRectangle): void {
		const viewed = app.activeDocument?.activeLayout?.viewedRectangle;
		if (!viewed || viewed.containsRectangle(rectangle.toArray())) return;

		const x = GraphicSelection.scrolledStart(
			viewed.x1,
			viewed.width,
			rectangle.x1,
			rectangle.width,
		);

		const y = GraphicSelection.scrolledStart(
			viewed.y1,
			viewed.height,
			rectangle.y1,
			rectangle.height,
		);

		if (x === viewed.x1 && y === viewed.y1) return;

		app.map._docLayer.scrollByPoint(
			new cool.SimplePoint(x - viewed.x1, y - viewed.y1),
		);
	}

	/*
		What tells one selection from another: the objects it stands on, so a selection of two
		shapes differs from a selection of one of them. The engine names every marked object, and
		the first of them again on its own, which is what a payload from an older engine carries.
	*/
	private static selectionKeyOf(extraInfo: any): string {
		const objectIds = GraphicSelection.selectedObjectIDsOf(extraInfo);
		return objectIds.length ? String(objectIds) : String(extraInfo?.id);
	}

	static resetSelectionRanges() {
		this.selectionChanged([]);
		GraphicSelection.updateSelectionSection();
		GraphicSelection.travel()?.leave();
		this.rectangle = null;
		this.extraInfo = null;
		this.inPlaceActive = false;

		if (this.handlesSection) {
			this.handlesSection.removeSubSections();
			app.sectionContainer.removeSection(this.handlesSection.name);
			this.handlesSection = null;
		}
	}

	/*
		The player of a selected video or audio object, where the client draws the selection
		itself: laid on the box of the selection and kept while the same object is selected
		again, so a running video is not started over by a click on it.
	*/
	private static showThePlayer(description: any): void {
		const section = this.selectionSection;
		if (!section) return;

		if (this.mediaPlayer && this.mediaPlayer.url !== description.url)
			this.dropThePlayer();

		if (!this.mediaPlayer) {
			this.mediaPlayer = new MediaPlayerSection(
				description,
				section.selectionCorner(),
				section.selectionSize(),
			);
			app.sectionContainer.addSection(this.mediaPlayer as any);
		}

		this.followThePlayer();
	}

	public static followThePlayer(): void {
		const section = this.selectionSection;
		if (!this.mediaPlayer || !section) return;

		this.mediaPlayer.follow(section.selectionCorner(), section.selectionSize());
	}

	private static dropThePlayer(): void {
		if (!this.mediaPlayer) return;

		app.sectionContainer.removeSection(this.mediaPlayer.name);
		this.mediaPlayer = null;
	}

	// shows the video inside current selection marker
	static onEmbeddedVideoContent(textMsg: string) {
		var videoDesc = JSON.parse(textMsg);

		if (RenderManager.isVectorRendering()) {
			this.showThePlayer(videoDesc);
			return;
		}

		if (!this.handlesSection) return;

		if (this.hasActiveSelection()) {
			videoDesc.width = this.rectangle.cWidth;
			videoDesc.height = this.rectangle.cHeight;
		}

		// proxy cannot identify RouteToken if it is encoded
		var routeTokenIndex = videoDesc.url.indexOf('%26RouteToken=');
		if (routeTokenIndex != -1) {
			videoDesc.url = videoDesc.url.replace(
				'%26RouteToken=',
				'&amp;RouteToken=',
			);
		}

		var videoToInsert =
			'<?xml version="1.0" encoding="UTF-8"?>\
		<svg xmlns="http://www.w3.org/2000/svg" width="' +
			videoDesc.width +
			'" height="' +
			videoDesc.height +
			'">\
		<foreignObject overflow="visible" width="' +
			videoDesc.width +
			'" height="' +
			videoDesc.height +
			'">\
			<body xmlns="http://www.w3.org/1999/xhtml">\
				<video controls="controls" width="' +
			videoDesc.width +
			'" height="' +
			videoDesc.height +
			'">\
					<source src="' +
			videoDesc.url +
			'" type="' +
			videoDesc.mimeType +
			'"/>\
					<track src="' +
			videoDesc.srt +
			'" kind="subtitles"' +
			'"/>\
				</video>\
			</body>\
		</foreignObject>\
		</svg>';

		this.handlesSection.addEmbeddedVideo(videoToInsert);
	}

	static renderDarkOverlay(rectangle: cool.SimpleRectangle = this.rectangle) {
		this.darkOverlayRectangle = rectangle;

		const anchor = app.activeDocument.activeLayout.documentAnchorPosition;
		var topLeft = new cool.Point(
			rectangle.v1X - anchor[0],
			rectangle.v1Y - anchor[1],
		);
		var bottomRight = new cool.Point(
			rectangle.v4X - anchor[0],
			rectangle.v4Y - anchor[1],
		);

		var bounds = new cool.Bounds(topLeft, bottomRight);

		app.map._docLayer._oleCSelections.setPointSet(CPointSet.fromBounds(bounds));
	}

	static refreshDarkOverlay() {
		if (this.darkOverlayRectangle && this.hasDarkOverlay())
			this.renderDarkOverlay(this.darkOverlayRectangle);
	}

	static hasDarkOverlay(): boolean {
		return !app.map._docLayer._oleCSelections.empty();
	}

	// When a shape is selected, the rectangles of other shapes are also sent from the core side.
	// They are in twips units.
	static convertObjectRectangleTwipsToPixels() {
		if (this.extraInfo && this.extraInfo.ObjectRectangles) {
			for (let i = 0; i < this.extraInfo.ObjectRectangles.length; i++) {
				for (let j = 0; j < 4; j++)
					this.extraInfo.ObjectRectangles[i][j] *=
						app.twipsToPixels * app.impress.twipsCorrection;
			}
		}
	}

	static inPlaceExtraInfo(messageJSON: any): any {
		const x1 = messageJSON[0];
		const y1 = messageJSON[1];
		const x2 = x1 + messageJSON[2];
		const y2 = y1 + messageJSON[3];
		const xs = [x1, (x1 + x2) / 2, x2];
		const ys = [y1, (y1 + y2) / 2, y2];
		const columns = [0, 1, 2, 0, 2, 0, 1, 2];
		const rows = [0, 0, 0, 1, 1, 2, 2, 2];

		const rectangle: any = {};
		for (let i = 0; i < 8; i++) {
			rectangle[String(i + 1)] = [
				{
					id: String(i),
					kind: String(i + 1),
					point: { x: xs[columns[i]], y: ys[rows[i]] },
				},
			];
		}

		return {
			id: 'inplace',
			handles: { kinds: { rectangle: rectangle } },
			isDraggable: true,
			isResizable: !this.inPlaceMath,
			isRotatable: false,
			isMathObject: this.inPlaceMath,
		};
	}

	static parseMessage(textMsg: string): any[] {
		return JSON.parse('[' + textMsg.substr('graphicselection:'.length) + ']');
	}

	static isMathSelection(msgData: any[]): boolean {
		return msgData.length > 5 && msgData[5]?.isMathObject === true;
	}

	// Core mirrors a left edge in a right-to-left sheet, and mirroring it again undoes that.
	static coreLeft(left: number, width: number): number {
		return app.map._docLayer.isCalcRTL() ? -(left + width) : left;
	}

	static selectsInPlaceObject(textMsg: string): boolean {
		try {
			const msgData = this.parseMessage(textMsg);
			if (msgData.length <= 5) return false;

			if (msgData[5]?.isChartPage === true) return true;

			const area = this.inPlaceArea;
			return (
				msgData[0] === area[0] &&
				msgData[1] === area[1] &&
				msgData[2] === area[2] &&
				msgData[3] === area[3]
			);
		} catch (error) {
			return false;
		}
	}

	static showInPlaceFrame() {
		const msgData: any[] = this.inPlaceArea.slice();
		msgData[4] = 0;
		msgData[5] = this.inPlaceExtraInfo(msgData);
		this.transformGraphicSelection(msgData);
		this.extractAndSetGraphicSelection(msgData);
		this.inPlaceTileOffset = this.tileTwipsOffset.slice();
		this.selectionAngle = 0;
		this.inPlaceActive = true;
		if (this.handlesSection) this.handlesSection.interactable = true;
		if (this.rectangle && this.hasDarkOverlay()) this.renderDarkOverlay();
	}

	private static toInPlaceEdges(
		rectangle: cool.SimpleRectangle,
		insideObject: boolean = false,
	): number[] {
		const docLayer = app.map._docLayer;
		const verticalOffset = docLayer.getFiledBasedViewVerticalOffset() || 0;
		let left = rectangle.x1;
		let top = rectangle.y1 - verticalOffset;
		let right = rectangle.x2;
		let bottom = rectangle.y2 - verticalOffset;

		if (docLayer._docType === 'spreadsheet' && insideObject) {
			const offset = this.inPlaceTileOffset;
			left = Math.abs(left) - offset[0];
			top -= offset[1];
			right = Math.abs(right) - offset[0];
			bottom -= offset[1];
		} else if (docLayer._docType === 'spreadsheet' && docLayer.sheetGeometry) {
			const topLeft = docLayer.sheetGeometry.getPrintTwipsPointFromTile(
				new cool.Point(Math.abs(left), top),
			);
			const bottomRight = docLayer.sheetGeometry.getPrintTwipsPointFromTile(
				new cool.Point(Math.abs(right), bottom),
			);
			left = topLeft.x;
			top = topLeft.y;
			right = bottomRight.x;
			bottom = bottomRight.y;
		}

		if (right < left) [left, right] = [right, left];
		if (bottom < top) [top, bottom] = [bottom, top];

		return [left, top, right, bottom];
	}

	private static keepUnmovedInPlaceEdges(edges: number[]): number[] {
		const area = this.inPlaceArea;
		const areaWidth = Math.abs(area[2]);
		const areaLeft = this.coreLeft(area[0], areaWidth);
		return this.keepUnmovedEdges(edges, [
			areaLeft,
			area[1],
			areaLeft + areaWidth,
			area[1] + area[3],
		]);
	}

	private static keepUnmovedEdges(
		edges: number[],
		core: number[],
		insideObject: boolean = false,
	): number[] {
		const frame = this.toInPlaceEdges(this.rectangle, insideObject);

		return edges.map((edge, index) => {
			const axis = index % 2;
			for (const frameIndex of [axis, axis + 2]) {
				if (Math.abs(edge - frame[frameIndex]) <= 2 * app.pixelsToTwips)
					return core[frameIndex];
			}
			return edge;
		});
	}

	public static sendElementArea(rectangle: cool.SimpleRectangle): void {
		let [left, top, right, bottom] = this.toInPlaceEdges(rectangle, true);
		if (this.selectionEdges && this.rectangle)
			[left, top, right, bottom] = this.keepUnmovedEdges(
				[left, top, right, bottom],
				this.selectionEdges,
				true,
			);
		const width = Math.round(right - left);
		const height = Math.round(bottom - top);
		if (width <= 0 || height <= 0) return;

		app.map.sendUnoCommand('.uno:TransformDialog', {
			TransformPosX: { type: 'long', value: Math.round(left) },
			TransformPosY: { type: 'long', value: Math.round(top) },
			TransformWidth: { type: 'long', value: width },
			TransformHeight: { type: 'long', value: height },
		});
	}

	public static sendInPlaceArea(
		rectangle: cool.SimpleRectangle,
		keepSize: boolean = false,
	): void {
		let [left, top, right, bottom] = this.toInPlaceEdges(rectangle);
		if (!keepSize && !this.inPlaceMath && this.inPlaceArea && this.rectangle)
			[left, top, right, bottom] = this.keepUnmovedInPlaceEdges([
				left,
				top,
				right,
				bottom,
			]);

		let width = Math.round(right - left);
		let height = Math.round(bottom - top);
		if (keepSize && this.inPlaceArea) {
			width = Math.abs(this.inPlaceArea[2]);
			height = this.inPlaceArea[3];
		}
		if (this.inPlaceMath && this.inPlaceArea) left = this.inPlaceArea[0];
		if (width <= 0 || height <= 0) return;

		left = this.coreLeft(left, width);

		app.socket.sendMessage(
			'inplaceobjectarea x=' +
				Math.round(left) +
				' y=' +
				Math.round(top) +
				' width=' +
				width +
				' height=' +
				height,
		);

		this.updateGraphicSelection();
	}

	static extractAndSetGraphicSelection(messageJSON: any) {
		var hasExtraInfo = messageJSON.length > 5;
		var hasGridOffset = false;
		var extraInfo = null;
		if (hasExtraInfo) {
			extraInfo = messageJSON[5];
			if (extraInfo.gridOffsetX || extraInfo.gridOffsetY) {
				app.map._docLayer._shapeGridOffset = new cool.SimplePoint(
					extraInfo.gridOffsetX,
					extraInfo.gridOffsetY,
				);
				hasGridOffset = true;
			}
		}

		const rawWidth = Math.abs(messageJSON[2]);
		// Calc RTL reports shapes with a negative X as its own sign-encoding convention.
		// Everywhere else (Impress, Writer, non-RTL Calc), a negative X is an ordinary
		// coordinate - for example a shape dragged partly off the left edge of a slide -
		// and must be kept as-is.
		const rectX =
			app.map._docLayer.isCalcRTL() && messageJSON[0] < 0
				? Math.abs(messageJSON[0]) - rawWidth
				: messageJSON[0];

		this.rectangle = new cool.SimpleRectangle(
			rectX,
			messageJSON[1],
			rawWidth,
			messageJSON[3],
		);
		this.selectionEdges = [
			rectX,
			messageJSON[1],
			rectX + rawWidth,
			messageJSON[1] + messageJSON[3],
		];

		if (hasGridOffset) {
			this.tileTwipsOffset = [
				app.map._docLayer._shapeGridOffset.x,
				app.map._docLayer._shapeGridOffset.y,
			];
			this.rectangle.moveBy(this.tileTwipsOffset);
		} else if (app.map._docLayer._docType === 'spreadsheet') {
			const tl = new cool.SimplePoint(
				Math.abs(this.rectangle.x1),
				this.rectangle.y1,
			);
			// What is inside an open object is shown where the object is.
			if (this.inPlaceArea && extraInfo?.id !== 'inplace') {
				tl.x += this.inPlaceTileOffset[0];
				tl.y += this.inPlaceTileOffset[1];
			} else app.map._docLayer.sheetGeometry.convertToTileTwips(tl);
			this.tileTwipsOffset = [
				tl.x - Math.abs(this.rectangle.x1),
				tl.y - this.rectangle.y1,
			];
			this.rectangle.moveTo(tl.toArray());
		} else {
			this.tileTwipsOffset = [0, 0];
		}

		this.extraInfo = extraInfo;

		if (app.map._docLayer._docType === 'presentation')
			this.convertObjectRectangleTwipsToPixels();
	}

	/// Push down the graphic selection on non-first pages of scrolling PDF view.
	static transformGraphicSelection(messageJSON: any) {
		const docLayer = app.map._docLayer;
		const verticalOffset = docLayer.getFiledBasedViewVerticalOffset();
		if (!verticalOffset) {
			return;
		}

		// y
		messageJSON[1] += verticalOffset;

		const extraInfo = messageJSON[5];
		const rectangle = extraInfo?.handles?.kinds?.rectangle;
		if (!rectangle) {
			return;
		}

		for (const key of ['1', '2', '3', '4', '5', '6', '7', '8']) {
			const y = parseInt(rectangle[key][0].point.y);
			rectangle[key][0].point.y = y + verticalOffset;
		}
	}

	public static updateGraphicSelection() {
		if (this.hasActiveSelection()) {
			// Hide the keyboard on graphic selection, unless cursor is visible.
			// Keep the focus in a dialog input or the search bar.
			if (!JSDialog.IsAnyInputFocused() && !app.map.isSearching())
				app.map.focus(app.file.textCursor.visible);

			let editMode = app.map.isEditMode();
			if (!editMode) {
				// If the just added signature line shape is selected, show the
				// graphic selection.
				editMode = GraphicSelection.isBeingSigned();
			}

			if (
				!editMode &&
				!this.extraInfo?.mimeType?.startsWith('video/') &&
				!this.extraInfo?.mimeType?.startsWith('audio/')
			) {
				return;
			}

			// Where the client draws the selection itself, the sections that draw it from the
			// message are not built at all. The selection section is all there is.
			if (RenderManager.isVectorRendering()) {
				GraphicSelection.updateSelectionSection();
				this.selectionSection?.setShowSection(editMode);
				app.sectionContainer.requestReDraw();
				app.map._docLayer._updateCursorAndOverlay();
				return;
			}

			var extraInfo = this.extraInfo;
			let addHandlesSection = false;

			if (!this.handlesSection) addHandlesSection = true;
			else if (
				GraphicSelection.selectionKeyOf(extraInfo) !==
				GraphicSelection.selectionKeyOf(
					this.handlesSection.sectionProperties.info,
				)
			) {
				// Another shape is selected.
				this.handlesSection.removeSubSections();
				app.sectionContainer.removeSection(this.handlesSection.name);
				this.handlesSection = null;
				addHandlesSection = true;
			}

			if (addHandlesSection) {
				this.handlesSection = new app.definitions.shapeHandlesSection({});
				app.sectionContainer.addSection(this.handlesSection);
			}

			this.handlesSection.setPosition(this.rectangle.pX1, this.rectangle.pY1);

			extraInfo.hasTableSelection =
				app.activeDocument.tableMiddleware.hasTableSelection(); // scaleSouthAndEastOnly

			this.handlesSection.refreshInfo(this.extraInfo);
			this.handlesSection.setShowSection(editMode);
			GraphicSelection.updateSelectionSection();
			this.selectionSection?.setShowSection(editMode);
			app.sectionContainer.requestReDraw();
		} else if (
			this.handlesSection &&
			app.sectionContainer.doesSectionExist(this.handlesSection.name)
		) {
			this.handlesSection.removeSubSections();
			app.sectionContainer.removeSection(this.handlesSection.name);
			this.handlesSection = null;
		}
		app.map._docLayer._updateCursorAndOverlay();
	}

	public static onShapeSelectionContent(textMsg: string) {
		textMsg = textMsg.substring('shapeselectioncontent:'.length + 1);

		var extraInfo = this.extraInfo;
		if (extraInfo && extraInfo.id) {
			app.map._cacheSVG[extraInfo.id] = textMsg;
		}

		// video is handled in _onEmbeddedVideoContent
		if (this.handlesSection) {
			if (this.handlesSection.sectionProperties.hasVideo)
				app.map._cacheSVG[extraInfo.id] = undefined;
			else this.handlesSection.setSVG(textMsg);

			if (!app.file.textCursor.visible) this.handlesSection.interactable = true;
		}
	}

	// Preview geometry sent by core while a shape handle is being dragged.
	public static onShapeDragPreview(textMsg: string) {
		if (!this.handlesSection) return;

		textMsg = textMsg.substring('shapedragpreview:'.length);
		try {
			this.handlesSection.onShapeDragPreview(JSON.parse(textMsg));
		} catch (error) {
			window.app.console.warn('cannot parse shapedragpreview message');
		}
	}

	private static checkDiagramData() {
		// A view that draws from the objects has the buttons beside the bands instead.
		if (RenderManager.isVectorRendering()) {
			GraphicSelection.updateGroupButtons();
			return;
		}

		if (GraphicSelection.extraInfo && GraphicSelection.extraInfo.isDiagram) {
			if (!GraphicSelection.diagramButton) {
				// need to create DiagramButtonSection
				const subSection = this.handlesSection
					? app.sectionContainer.getSectionWithName(
							this.handlesSection.sectionProperties.subSectionPrefix +
								'diagram',
						)
					: null;
				var halfWidth: number = 0;

				if (subSection && subSection.sectionProperties) {
					// get distance that frame occupies from frame object
					halfWidth = subSection.sectionProperties.ownInfo.halfWidth;
				}

				GraphicSelection.diagramButton = new DiagramButtonSection(
					halfWidth * app.twipsToPixels,
				);
				GraphicSelection.diagramButton.forceNextReposition();
				app.sectionContainer.addSection(GraphicSelection.diagramButton);
			}

			GraphicSelection.diagramButton.updatePosition();
		} else if (GraphicSelection.diagramButton) {
			app.sectionContainer.removeSection(GraphicSelection.diagramButton.name);
			GraphicSelection.diagramButton = null;
		}
	}

	private static checkChartData() {
		// Chart Context Buttons are disabled now.
		// we will probably enable it when chart styles will work fine.
		var disabled = true;
		if (disabled) return;
		if (
			GraphicSelection.extraInfo &&
			GraphicSelection.extraInfo.isChartPage &&
			GraphicSelection.extraInfo.isChartPage === true
		) {
			if (!GraphicSelection.chartContextToolbarSelectStyle) {
				GraphicSelection.chartContextToolbarSelectStyle =
					new ChartContextButtonSection(0);
				GraphicSelection.chartContextToolbarSaveStyle =
					new ChartContextButtonSection(1);
				GraphicSelection.chartContextToolbarSelectStyle.forceNextReposition();
				GraphicSelection.chartContextToolbarSaveStyle.forceNextReposition();
				app.sectionContainer.addSection(
					GraphicSelection.chartContextToolbarSelectStyle,
				);
				app.sectionContainer.addSection(
					GraphicSelection.chartContextToolbarSaveStyle,
				);
			}
			GraphicSelection.chartContextToolbarSelectStyle.updatePosition();
			GraphicSelection.chartContextToolbarSaveStyle.updatePosition();

			if (GraphicSelection.chartContextToolbarSelectStyle) {
				GraphicSelection.chartContextToolbarSelectStyle.showChartContextToolbar();
				GraphicSelection.chartContextToolbarSaveStyle.showChartContextToolbar();
			}
		} else if (GraphicSelection.chartContextToolbarSelectStyle) {
			app.sectionContainer.removeSection(
				GraphicSelection.chartContextToolbarSelectStyle.name,
			);
			app.sectionContainer.removeSection(
				GraphicSelection.chartContextToolbarSaveStyle.name,
			);
			GraphicSelection.chartContextToolbarSelectStyle = null;
			GraphicSelection.chartContextToolbarSaveStyle = null;
		}
	}

	public static onMessage(textMsg: string) {
		URLPopUpSection.closeURLPopUp();

		if (textMsg.match('EMPTY')) {
			this.resetSelectionRanges();
			if (this.inPlaceArea) this.showInPlaceFrame();
		} else if (textMsg.match('INPLACE EXIT')) {
			app.map._docLayer._oleCSelections.clear();
			this.darkOverlayRectangle = null;
			this.inPlaceActive = false;
			this.inPlaceArea = null;
			this.inPlaceMath = false;
		} else if (textMsg.match('INPLACE')) {
			try {
				var msgData = this.parseMessage(textMsg);
				const isMathObject = this.isMathSelection(msgData);
				if (isMathObject && app.map._docLayer._docType !== 'text') {
					this.inPlaceArea = null;
					this.inPlaceActive = false;
					this.extractAndSetGraphicSelection(msgData.slice(0, 4));
				} else if (msgData.length > 1) {
					this.inPlaceMath = isMathObject;
					this.inPlaceArea = msgData.slice(0, 4);
					this.showInPlaceFrame();
				}
			} catch (error) {
				window.app.console.warn('cannot parse graphicselection command');
			}

			// The frame comes in twips and the dark overlay holds it in pixels. A
			// message replayed for a new zoom level has to redo that conversion.
			if (this.rectangle) this.renderDarkOverlay();

			if (!this.inPlaceArea) this.rectangle = null;
		} else if (this.inPlaceArea && this.selectsInPlaceObject(textMsg)) {
			this.showInPlaceFrame();
			if (!window.ThisIsTheiOSApp && !this.inPlaceMath)
				app.socket.sendMessage('rendershapeselection mimetype=image/svg+xml');
		} else {
			this.inPlaceActive = false;
			msgData = this.parseMessage(textMsg);

			const isMathObject = this.isMathSelection(msgData);

			this.transformGraphicSelection(msgData);

			this.extractAndSetGraphicSelection(msgData);

			/*
				Which objects the selection is about. A selection the client asked for names the
				objects it named itself, so it arrives unchanged; one that differs was made
				elsewhere, by the keyboard, an undo or another user. Everything below is set up
				from the message either way, until the client builds it from what it holds.
			*/
			const changed = this.selectionChanged(
				GraphicSelection.selectedObjectIDsOf(
					msgData.length > 5 ? msgData[5] : null,
				),
			);

			// Update the dark overlay on zooming & scrolling
			if (!app.map._docLayer._oleCSelections.empty()) {
				// A formula is its own frame and changes shape as it is typed, so the
				// overlay follows the selection
				const framed = isMathObject
					? this.rectangle
					: (this.darkOverlayRectangle ?? this.rectangle);
				app.map._docLayer._oleCSelections.clear();
				this.renderDarkOverlay(framed);
			}

			this.selectionAngle = msgData.length > 4 ? msgData[4] : 0;

			if (this.extraInfo) {
				var dragInfo = this.extraInfo.dragInfo;
				if (dragInfo && dragInfo.dragMethod === 'PieSegmentDragging') {
					dragInfo.initialOffset /= 100.0;
					var dragDir = dragInfo.dragDirection;
					dragInfo.dragDirection = app.map._docLayer._twipsToPixels(
						new cool.Point(dragDir[0], dragDir[1]),
					);
					dragDir = dragInfo.dragDirection;
					dragInfo.range2 = dragDir.x * dragDir.x + dragDir.y * dragDir.y;
				}
			}

			// defaults
			var extraInfo = this.extraInfo;
			if (extraInfo) {
				if (extraInfo.isDraggable === undefined) extraInfo.isDraggable = true;
				if (extraInfo.isResizable === undefined) extraInfo.isResizable = true;
				if (extraInfo.isRotatable === undefined) extraInfo.isRotatable = true;
			}

			/*
				A picture of the selection to show while it is dragged. A view that draws from
				objects draws that picture from the objects themselves and asks for none.

				Workaround for tdf#123874. For some reason the handling of the
				shapeselectioncontent messages that we get back causes the WebKit process
				to crash on iOS.
			*/
			if (
				!RenderManager.isVectorRendering() &&
				!window.ThisIsTheiOSApp &&
				this.extraInfo.isDraggable &&
				!this.extraInfo.svg
			) {
				app.socket.sendMessage('rendershapeselection mimetype=image/svg+xml');
			}

			/*
				Scroll to the object that is selected now, where it does not lie in what is on
				screen. The view only scrolls, it never zooms, which is what the office does when
				Tab walks from one object to the next. The two conditions below both hold the view
				still in a case where the object then stays off screen, so a view that draws from
				objects reads them differently.
			*/
			const drawsFromObjects = RenderManager.isVectorRendering();

			/*
				In the other view a jump waits while a selection is complex, which is about a
				selection of text that a jump would tear the reader away from. A selection of an
				object is marked complex a few lines below, so from the second selection on it is
				complex before it is even looked at; a view that draws from objects does not wait.
			*/
			const mayJump = !drawsFromObjects && app.map._docLayer._allowViewJump();

			/*
				A jump also waits while the view follows a cursor, so that it stays with the person
				being watched. A single person editing follows their own view, and there is nobody
				else to stay with then.
			*/
			const followsAnotherView = drawsFromObjects
				? !app.isFollowingOff() &&
					Number(app.getFollowedViewId()) !== Number(app.map._docLayer._viewId)
				: app.isFollowingEditor() || app.isFollowingUser();

			/*
				A view that draws from objects scrolls only for a selection that is new, not for the
				same one reported again after it was changed, and none for a selection it asked for
				itself, which it brought on screen when it made it. It scrolls no further than it has
				to, so that the whole of what is selected lands on screen with its handles, and it
				measures that from the boxes of the objects it holds. Only where they have not
				arrived yet, as right after a paste, it goes by the rectangle the message brings.
			*/
			if (drawsFromObjects) {
				if (
					changed &&
					!TextSelections.getEndRectangle() &&
					!followsAnotherView &&
					!GraphicSelection.showTheObjects(this.selectedObjectIDs) &&
					this.rectangle
				)
					GraphicSelection.scrollRectangleIntoView(this.rectangle);
			} else if (!app.map._docLayer.isWriter() && this.rectangle && mayJump) {
				// The other view has the corner it holds put in the middle of the view.
				if (
					(!app.isPointVisibleInTheDisplayedArea([
						this.rectangle.x1,
						this.rectangle.y1,
					]) ||
						!app.isPointVisibleInTheDisplayedArea([
							this.rectangle.x2,
							this.rectangle.y2,
						])) &&
					!TextSelections.getEndRectangle() &&
					!followsAnotherView &&
					!app.map.calcInputBarHasFocus()
				)
					app.map._docLayer.scrollToPos(
						new cool.SimplePoint(this.rectangle.x1, this.rectangle.y1),
					);
			}
		}

		// Graphics are by default complex selections, unless Core tells us otherwise.
		if (app.map._clip) app.map._clip.onComplexSelection('');

		// Reset text selection - important for textboxes in Impress
		if (app.map._docLayer._selectionContentRequest)
			clearTimeout(app.map._docLayer._selectionContentRequest);
		app.map._docLayer._onMessage('textselectioncontent:');

		this.updateGraphicSelection();

		if (msgData && msgData.length > 5) {
			var extraInfo = msgData[5];
			if (
				extraInfo.mimeType?.startsWith('video/') ||
				extraInfo.mimeType?.startsWith('audio/')
			) {
				this.onEmbeddedVideoContent(JSON.stringify(extraInfo));
			}
		}

		GraphicSelection.checkChartData();
		GraphicSelection.checkDiagramData();
	}

	/*
		Some keywords: mousecontrol, _postMouseEvent, core side.
		Explanation:
			* User can edit a shape's text.
			* We need to send the mouse events to core in that case (for text selection, double click etc).
			* When cursor is visible, we assume text editing started.
			* And we set the "interactable" property of the shape handler section to false.
			* When "interactable" is false, canvas section container events pass through the section.
			* In our case, ShapeHandlesSection begins to be ignored.
			* And MouseControl catches all the events.
			* Users can select text, click, double click, triple click, quadruple click etc.
	*/
	public static onTextCursorVisibility(event: any) {
		// Text is being edited, so the keyboard belongs to the text and no longer to a handle.
		if (event.detail.visible) GraphicSelection.travel()?.leave();

		if (this.hasActiveSelection() && this.handlesSection) {
			this.handlesSection.interactable =
				!event.detail.visible || this.inPlaceActive;
		}
	}
}

app.events.on(
	'updatepermission',
	GraphicSelection.onUpdatePermission.bind(GraphicSelection),
);

app.events.on(
	'TextCursorVisibility',
	GraphicSelection.onTextCursorVisibility.bind(GraphicSelection),
);

app.definitions.graphicSelection = GraphicSelection;
